package goipbus

import (
	"bytes"
	"fmt"
	"math"
	"testing"
)

func TestMin(t *testing.T) {
	var v float64
	v = math.Min(1., 2.)
	if v != 1. {
		t.Error("Expected 1, got ", v)
	}
}

func TestHeader(t *testing.T) {
	var size uint8 = 1
	var id IPbusTransactionID = 1

	// Read transaction Example
	// Ver = 0x20 PacketID = 0x0000 Byte-Order= 0x0 Packet TypeID = 0x0
	// 0x2 000 00 0 f
	pheader := IPbusPacketHeader(0x20000000)
	pID := IPbusPacketHeader(packetID) << 8
	order := IPbusPacketHeader(0xf) << 4
	ptypeID := IPbusPacketHeader(0x0)
	pheader = pheader | pID | order | ptypeID

	//
	var pword IPbusPacketHeader
	pword, err := packetHeader(0xf, ControlPacket)
	if err != nil {
		t.Errorf("Error %v\n", err)
	}
	if pheader != pword {
		t.Errorf("Expected header %x, generated %x\n", pheader, pword)
	}
	fmt.Printf("Generated header %x\n", pheader)

	// Read transaction Example
	// Ver = 0x20 PacketID = 0x0000 Byte-Order= 0x0 Packet TypeID = 0x0
	// 0x2 000 00 0 f
	pheader = IPbusPacketHeader(0x20000000)
	pID = IPbusPacketHeader(packetID) << 8
	order = IPbusPacketHeader(0xf) << 4
	ptypeID = IPbusPacketHeader(0x0)
	pheader = pheader | pID | order | ptypeID

	pword, err = packetHeader(0xf, ControlPacket)
	if err != nil {
		t.Errorf("Error %v\n", err)
	}
	if pheader != pword {
		t.Errorf("Expected header %x, generated %x\n", pheader, pword)
	}
	fmt.Printf("Generated header %x\n", pheader)

	fmt.Println("Starting Test Header")
	err = setTransactionID(id)
	if err != nil {
		t.Error("Error %v", err)
	}

	// Read transaction Example
	// Ver = 0x2 TransID = 0x000 Words= 0x00 TypeID = 0x0, InfoCode = 0xf
	// 0x2 000 00 0 f
	header := IPbusTransactionHeader(0x2000000f)
	trans := IPbusTransactionHeader(transactionID) << 16
	words := IPbusTransactionHeader(size) << 8
	typeID := IPbusTransactionHeader(0x0) << 4
	infoCode := IPbusTransactionHeader(0xf)
	header = header | trans | words | typeID | infoCode

	//
	var word0 IPbusTransactionHeader
	word0, err = transactionHeader(ReadTypeID, size)
	if err != nil {
		t.Errorf("Error %v\n", err)
	}
	if header != word0 {
		t.Errorf("Expected header %x, generated %x\n", header, word0)
	}
	fmt.Printf("Generated header %x\n", header)

	// Write transaction Example
	// Ver = 0x2 TransID = 0x000 Words= 0x00 TypeID = 0x1, InfoCode = 0xf
	// 0x2 000 00 1 f
	header = IPbusTransactionHeader(0x20000000)
	trans = IPbusTransactionHeader(transactionID) << 16
	words = IPbusTransactionHeader(size) << 8
	typeID = IPbusTransactionHeader(0x1) << 4
	infoCode = IPbusTransactionHeader(0xf)
	header = header | trans | words | typeID | infoCode

	word0, err = transactionHeader(WriteTypeID, size)
	if err != nil {
		t.Errorf("Error %v", err)
	}
	if header != word0 {
		t.Errorf("Expected header %x, generated %x\n", header, word0)
	}
	fmt.Printf("Generated header %x\n", header)

	// Read transaction Example
	// Ver = 0x2 TransID = 0x000 Words= 0x00 TypeID = 0x0, InfoCode = 0xf
	// 0x2 000 00 0 f
	header = IPbusTransactionHeader(0x2000000f)
	trans = IPbusTransactionHeader(transactionID) << 16
	words = IPbusTransactionHeader(size) << 8
	typeID = IPbusTransactionHeader(0x0) << 4
	infoCode = IPbusTransactionHeader(0xf)
	header = header | trans | words | typeID | infoCode

	word0, err = readHeader(size)
	if err != nil {
		t.Errorf("Error %v\n", err)
	}
	if header != word0 {
		t.Errorf("Expected header %x, generated %x\n", header, word0)
	}
	fmt.Printf("Generated header %x\n", header)

	// Write transaction Example
	// Ver = 0x2 TransID = 0x000 Words= 0x00 TypeID = 0x1, InfoCode = 0xf
	// 0x2 000 00 1 f
	header = IPbusTransactionHeader(0x20000000)
	trans = IPbusTransactionHeader(transactionID) << 16
	words = IPbusTransactionHeader(0x8) << 8
	typeID = IPbusTransactionHeader(0x1) << 4
	infoCode = IPbusTransactionHeader(0xf)
	header = header | trans | words | typeID | infoCode

	data := []IPbusWord{1, 2, 3, 4, 5, 6, 7, 8}
	word0, err = writeHeader(data)
	if err != nil {
		t.Errorf("Error %v", err)
	}
	if header != word0 {
		t.Errorf("Expected header %x, generated %x\n", header, word0)
	}
	fmt.Printf("Generated header %x\n", header)
}

func TestControlPackage(t *testing.T) {
	var size uint8 = 0xA
	var id IPbusTransactionID = 1
	var addr BaseAddress = 0xEFB

	// Set Transaction ID
	err := setTransactionID(id)
	if err != nil {
		t.Errorf("Error %v\n", err)
	}

	// Set Transaction Base Address
	err = setBaseAddress(addr)
	if err != nil {
		t.Errorf("Error %v\n", err)
	}

	// Packet Header: 		20 0002 f 0
	// Transaction Header:	2 001 0a 0 f
	// Base Address:		00000efb
	var bt = []byte{
		0x20, 0x00, 0x02, 0xf0,
		0x20, 0x01, 0x0a, 0x0f,
		0x00, 0x00, 0x0e, 0xfb,
	}
	b, err := readRequest(addr, size)
	if !bytes.Equal(bt, b) {
		t.Errorf("Expected buffer %x, generated %x\n", bt, b)
	}
	fmt.Printf("Generated request %x\n", b)

	// 20 0003 f 0
	// 2 002 0b 0 f
	// 00000efb
	bt = []byte{
		0x20, 0x00, 0x03, 0xf0,
		0x20, 0x02, 0x0b, 0x0f,
		0x00, 0x00, 0x0e, 0xfb,
	}
	b, err = readRequest(addr, size+1)
	if !bytes.Equal(bt, b) {
		t.Errorf("Expected buffer %x, generated %x\n", bt, b)
	}
	fmt.Printf("Generated request %x\n", b)

	// 20 0004 f 0
	// 2 002 0b 0 f
	// 00000efb
	bt = []byte{
		0x20, 0x00, 0x04, 0xf0,
		0x20, 0x03, 0x0a, 0x0f,
		0x00, 0x00, 0x0e, 0xfb,
	}
	//
	rq := new(IPbusRequest)
	rq.addr = baseAddress
	rq.data = nil
	rq.words = size
	if err != nil {
		t.Errorf("Error %v\n", err)
	}
	b = make([]byte, 12)
	p0 := new(IPbusControlPacket)
	p0.reqs[0] = *rq
	n, err := p0.Encode(b)
	if !bytes.Equal(bt, b) {
		t.Errorf("Expected buffer 0x%x, generated 0x%x\n", bt, b)
	}
	fmt.Printf("Generated %v bytes, request 0x%x\n", n, b)
}

func TestRequest(t *testing.T) {
	var size uint8 = 0xA
	var id IPbusTransactionID = 1
	var addr BaseAddress = 0xEFB

	// Set Transaction ID
	err := setTransactionID(id)
	if err != nil {
		t.Errorf("Error %v\n", err)
	}

	// Set Transaction Base Address
	err = setBaseAddress(addr)
	if err != nil {
		t.Errorf("Error %v\n", err)
	}

	// Define Transaction request
	rq := new(IPbusRequest)
	rq.typeId = ReadTypeID
	rq.addr = baseAddress
	rq.data = nil
	rq.words = size

	// Encode method
	n, err := rq.Encode()
	// 20 0001 f 0
	// 2 002 0b 0 f
	// 00000efb
	bt := []byte{
		0x20, 0x01, 0x0a, 0x0f,
		0x00, 0x00, 0x0e, 0xfb,
	}
	if !bytes.Equal(bt, rq.b) {
		t.Errorf("Expected buffer 0x%x, generated 0x%x\n", bt, rq.b)
	}
	fmt.Printf("Generated %v bytes, request 0x%x\n", n, rq.b)

	// Read set transaction header and
	n, err = rq.Read(rq.addr, rq.words)
	// 20 0002 f 0
	// 2 002 0b 0 f
	// 00000efb
	bt = []byte{
		0x20, 0x02, 0x0a, 0x0f,
		0x00, 0x00, 0x0e, 0xfb,
	}
	if !bytes.Equal(bt, rq.b) {
		t.Errorf("Expected buffer 0x%x, generated 0x%x\n", bt, rq.b)
	}
	fmt.Printf("Generated %v bytes, request 0x%x\n", n, rq.b)

	// 20 0003 f 0
	// 2 002 0b 0 f
	// 00000efb
	bt = []byte{
		0x20, 0x03, 0x0a, 0x0f,
		0x00, 0x00, 0x0e, 0xfb,
	}
	n, err = rq.Encode()
	if !bytes.Equal(bt, rq.b) {
		t.Errorf("Expected buffer 0x%x, generated 0x%x\n", bt, rq.b)
	}
	fmt.Printf("Generated %v bytes, request 0x%x\n", n, rq.b)

}

func TestNewRequests(t *testing.T) {
	var id IPbusTransactionID = 5

	// Set Transaction ID
	err := setTransactionID(id)
	if err != nil {
		t.Errorf("Error %v\n", err)
	}
	rq := NewReadRequest(0xefb, 0xe)
	rq.Encode()
	bt := []byte{0x20, 0x05, 0x0e, 0x0f, 0x00, 0x00, 0x0e, 0xfb}
	if !bytes.Equal(bt, rq.b) {
		t.Errorf("Expected buffer 0x%x, generated 0x%x\n", bt, rq.b)
	}
	fmt.Printf("Generated request 0x%x\n", rq.b)

	rq = NewNonIncrementalReadRequest(0xefb, 0xa)
	rq.Encode()
	bt = []byte{0x20, 0x06, 0x0a, 0x2f, 0x00, 0x00, 0x0e, 0xfb}
	if !bytes.Equal(bt, rq.b) {
		t.Errorf("Expected buffer 0x%x, generated 0x%x\n", bt, rq.b)
	}
	fmt.Printf("Generated request 0x%x\n", rq.b)

	rq = NewWriteRequest(0xefb, []IPbusWord{0x3, 0x4, 0x5})
	rq.Encode()
	bt = []byte{0x20, 0x07, 0x03, 0x1f,
		0x00, 0x00, 0x0e, 0xfb,
		0x00, 0x00, 0x00, 0x03,
		0x00, 0x00, 0x00, 0x04,
		0x00, 0x00, 0x00, 0x05,
	}
	if !bytes.Equal(bt, rq.b) {
		t.Errorf("Expected buffer 0x%x, generated 0x%x\n", bt, rq.b)
	}
	fmt.Printf("Generated request 0x%x\n", rq.b)

	rq = NewNonIncrementalWriteRequest(0xefb, []IPbusWord{0x6, 0x7, 0x8})
	rq.Encode()
	bt = []byte{0x20, 0x08, 0x03, 0x3f, 0x00, 0x00, 0x0e, 0xfb, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x08}
	if !bytes.Equal(bt, rq.b) {
		t.Errorf("Expected buffer 0x%x, generated 0x%x\n", bt, rq.b)
	}
	fmt.Printf("Generated request 0x%x\n", rq.b)

	rq = NewRMWsumRequest(0xefb, 0xf)
	rq.Encode()
	bt = []byte{0x20, 0x09, 0x01, 0x5f, 0x00, 0x00, 0x0e, 0xfb, 0x00, 0x00, 0x00, 0x0f}
	if !bytes.Equal(bt, rq.b) {
		t.Errorf("Expected buffer 0x%x, generated 0x%x\n", bt, rq.b)
	}
	fmt.Printf("Generated request 0x%x\n", rq.b)

	rq = NewRMWbitsRequest(0xefb, 0x00FF, 0xFF00)
	rq.Encode()
	bt = []byte{0x20, 0x0a, 0x01, 0x4f, 0x00, 0x00, 0x0e, 0xfb, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0x00}
	if !bytes.Equal(bt, rq.b) {
		t.Errorf("Expected buffer 0x%x, generated 0x%x\n", bt, rq.b)
	}
	fmt.Printf("Generated request 0x%x\n", rq.b)

}

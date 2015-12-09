package goipbus

import (
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

	fmt.Println("Starting Test Header")
	err := setTransactionID(id)
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

func TestTransaction(t *testing.T) {
	// Set Transaction Base Address
	var addr BaseAddress = 0xEFB

	err := setBaseAddress(addr)
	if err != nil {
		t.Errorf("Error %v\n", err)
	}

	//	var dummy IPbusWord = 0xffff

}

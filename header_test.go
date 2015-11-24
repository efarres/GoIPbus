package goipbus

import (
	"testing"
	"time"
)

func TestHeader(*testing.T) {
	var size int8 = 24
	var word IPbusWord
	var id IPbusTransactionID

	err := setTransactionID(id)
	if err != nil {
		t.Error("Error %v", err)
	}

	// Read transaction Example
	// TypeID = 0x0, InfoCode = 0xf
	// 0x2 000 00 0 f
	word = 0x2000000f
	word = word & (transactionID << 16) & (size << 8)

	word0, err = transactionHeader(ReadTypeID, size)
	if err != nil {
		t.Error("Error %v", err)
	}
	if word != word0 {
		t.Error("Expected header %b, generated %b ", word, word0)
	}

	var addr BaseAddress = 0xEFB
	var dummy IPbusWord = 0xffff

	// Write transaction Example
	// TypeID = 0x1, InfoCode = 0xf
	// 0x2 000 00 1 f
	word = 0x2000001f
	word = word & (transactionID << 16) & (size << 8)
	word0, err := transactionHeader(WriteTypeID, size)
	if err != nil {
		t.Error("Error %v", err)
	}
	if word != word0 {
		t.Error("Expected header %b, generated %b ", word, word0)
	}

}

// softipbus
// cactus project
// trunk/cactuscore/softipbus

// extract usefull implementation maintaining the cactus definitions

package goipbus

const (
	IPBUS_CONTROL_PKT = 0x0 // IPbus control packets are how data is sent/recieved
	IPBUS_STATUS_PKT  = 0x1 // These two packet types are for data reliability over UDP
	IPBUS_RESEND_PKT  = 0x2
	IPBUS_PKT_ERR     = 0x3 // not in the protocol
)

// Info over the IPbus transaction status
const (
	IPBUS_INFO_SUCCESS          = 0x0
	IPBUS_INFO_BADHEADER        = 0x1
	IPBUS_INFO_BUSERROR_READ    = 0x4
	IPBUS_INFO_BUSERROR_WRITE   = 0x5
	IPBUS_INFO_BUSTIMEOUT_READ  = 0x6
	IPBUS_INFO_BUSTIMEOUT_WRITE = 0x7
	IPBUS_INFO_REQUEST          = 0xf
)

// Input stream states
const (
	IPBUS_ISTREAM_EMPTY          = 0x0
	IPBUS_ISTREAM_PACKET         = 0x1 // new packet header is in the buffer
	IPBUS_ISTREAM_PACKET_SWP_ORD = 0x2 // new packet header with opposite endianness is in the buffer
	IPBUS_ISTREAM_PARTIAL_TRANS  = 0x3 // a partial transaction is in the buffer
	IPBUS_ISTREAM_FULL_TRANS     = 0x4 // a full transaction is ready in the buffer
	IPBUS_ISTREAM_ERR            = 0xF // something went wrong dude
)

// Process a TCP/IP packet with content stored in buffer coming from the given
// client. The buffer in the client will be consumed.  Returns number of bytes
// processed.  The response will be written to the client's output buffer.
func processInputStream(b []byte) (n IPbusTransactionHeader, err error) {

	return n, err
}

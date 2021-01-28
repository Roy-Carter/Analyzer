#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ws2tcpip.h>
#define NEW_FILE_NAME "receive.lua"
#define PCAP_NAME "test.pcap"
#define SRC_ADDR inet_addr("1.2.3.4")
#define DEST_ADDR inet_addr("1.2.3.5")
#define BUFFER_SIZE 4096
#define READ_SIZE 50
#define C_PORT 5400
#define PYTHON_PORT 2222
#define LOCAL_HOST "127.0.0.1"
#define EXIT_MSG "exit"
#define INT8 "uint8_t"
#define INT16 "uint16_t"
#define SPACE " "



typedef struct field_view {
	char *name;
	char *size;
	int letters;//number of hexa letters to check
}field;


typedef struct data_s {
	unsigned char payload[BUFFER_SIZE];
}data;

typedef struct pcap_hdr_s {
	uint32_t magic_number;   /* magic number */
	uint16_t version_major;  /* major version number */
	uint16_t version_minor;  /* minor version number */
	uint32_t  thiszone;       /* GMT to local correction */
	uint32_t sigfigs;        /* accuracy of timestamps */
	uint32_t snaplen;        /* max length of captured packets, in octets */
	uint32_t network;        /* data link type */
} pcap_hdr_t;

struct pcap_timeval {
	uint32_t tv_sec;       /* seconds */
	uint32_t tv_usec;      /* microseconds */
};

typedef struct pcaprec_hdr_s {
	struct pcap_timeval ts; /* time stamp */
	uint32_t incl_len;       /* number of octets of packet saved in file */
	uint32_t orig_len;       /* actual length of packet */
} pcaprec_hdr_t;


//Ethernet Header
typedef struct ethernet_header
{
	UCHAR dest[6]; //Total 48 bits
	UCHAR source[6]; //Total 48 bits
	USHORT type; //16 bits
}   ETHER_HDR;

typedef struct ip_hdr
{
	unsigned char  ip_header_len : 4;  // 4-bit header length (in 32-bit words) normally=5 (Means 20 Bytes may be 24 also)
	unsigned char  ip_version : 4;  // 4-bit IPv4 version
	unsigned char  ip_tos;           // IP type of service
	unsigned short ip_total_length;  // Total length
	unsigned short ip_id;            // Unique identifier 

	unsigned char  ip_frag_offset : 5;        // Fragment offset field

	unsigned char  ip_more_fragment : 1;
	unsigned char  ip_dont_fragment : 1;
	unsigned char  ip_reserved_zero : 1;

	unsigned char  ip_frag_offset1;    //fragment offset

	unsigned char  ip_ttl;           // Time to live
	unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)
	unsigned short ip_checksum;      // IP checksum
	unsigned int   ip_srcaddr;       // Source address
	unsigned int   ip_destaddr;      // Source address
}   IPV4_HDR;

// TCP header
typedef struct tcp_header
{
	unsigned short source_port;   // source port
	unsigned short dest_port;     // destination port
	unsigned int sequence;        // sequence number - 32 bits
	unsigned int acknowledge;     // acknowledgement number - 32 bits

	unsigned char ns : 1;          //Nonce Sum Flag Added in RFC 3540.
	unsigned char reserved_part1 : 3; //according to rfc
	unsigned char data_offset : 4;    /*The number of 32-bit words
									  in the TCP header.
									  This indicates where the data begins.
									  The length of the TCP header
									  is always a multiple
									  of 32 bits.*/

	unsigned char fin : 1; //Finish Flag
	unsigned char syn : 1; //Synchronise Flag
	unsigned char rst : 1; //Reset Flag
	unsigned char psh : 1; //Push Flag
	unsigned char ack : 1; //Acknowledgement Flag
	unsigned char urg : 1; //Urgent Flag

	unsigned char ecn : 1; //ECN-Echo Flag
	unsigned char cwr : 1; //Congestion Window Reduced Flag

	////////////////////////////////

	unsigned short window; // window
	unsigned short checksum; // checksum
	unsigned short urgent_pointer; // urgent pointer
} TCP_HDR;



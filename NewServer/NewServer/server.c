#include "server.h"
SOCKET init_server();
pcap_hdr_t create_global_header();
pcaprec_hdr_t create_packet_header(data*);
void server_create_tcp(TCP_HDR*);
void server_create_ip(IPV4_HDR*, int);
void server_create_eth(ETHER_HDR*);
bool write_to_lua(SOCKET , FILE *);
bool recv_file_len(SOCKET , long*);
bool recv_raw(SOCKET , void*, int);
void write_to_file(FILE*, pcaprec_hdr_t, ETHER_HDR*, IPV4_HDR*, TCP_HDR*, data*);
bool handle_server(SOCKET, char*, FILE*, field*);
bool handle_lua_file(SOCKET, field*);
bool check_socket_buffer(int);
int lines_in_lua();
void copy_file_fields(field *, int, char *, int);
void check_protocol_sizes(field *, int);
bool fill_fields(int, field *);
void print_field_description(int, field*);
SOCKET open_socket(int, char*);
bool send_file(SOCKET, FILE*);
bool send_file_length(SOCKET , long);
bool send_raw(SOCKET , void*, int);
bool handle_python(SOCKET, FILE*, FILE *);

/*
============================================
General : function is responsible for sending the length of data to the server
Parameters : sock - socket connection between client and server
			 *buf - holds a pointer to the data that needs to be sent
			 bufsize - the length of the data pointer

Return Value : returns TRUE when the length of the data was sent correctly.
returns FALSE when there was a socket error.
============================================
*/
bool send_raw(SOCKET sock, void* buf, int bufsize)
{
	unsigned char* pbuf = (unsigned char*)buf;
	while (bufsize > 0) {
		int num = send(sock, pbuf, bufsize, 0);
		if (num == SOCKET_ERROR) { return false; }
		pbuf += num;
		bufsize -= num;
	}
	return true;
}

/*
============================================
General : function is responsible for sending the length of the file
to the server in network byte order
Parameters : sock - socket connection between client and server
			 filesize - the value of the file size

Return Value : returns TRUE when the length of the data was sent correctly.
returns FALSE when there was a socket error.
============================================
*/

bool send_file_length(SOCKET sock, long filesize)
{
	filesize = htonl(filesize);
	return send_raw(sock, &filesize, sizeof(filesize));
}

/*
============================================
General : function is responsible of sending the new lua
file to the server
Parameters : sock - socket between the client and the server
			 f - file that needs to be sent to the server

Return Value : returns TRUE when the file was sent correctly
returns FALSE when the file is empty or when there was a socket error
============================================
*/
bool send_file(SOCKET sock, FILE* f)
{
	char buffer[BUFFER_SIZE];
	size_t num;
	if (fseek(f, 0, SEEK_END) != 0) { return false; }
	long filesize = ftell(f);
	rewind(f);
	if (filesize == -1L) { return false; }
	printf("file size -> %d\n", filesize);
	if (!send_file_length(sock, filesize)) { return false; }
	if (filesize > 0) {
		do {
			num = fread(buffer, 1, min(filesize, BUFFER_SIZE), f);
			if (num < 1) { return false; }
			if (!send_raw(sock, buffer, num)) { return false; }
			filesize -= num;
		} while (filesize > 0);
	}
	return true;
}
/*
===================================================
General : the function initalize the server socket.
Parameters : None.
Return Value : SOCKET , returns the socket created for
the client connection.
===================================================
*/
SOCKET init_server()
{
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		printf("winsock failed\n");
		return;
	}
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		printf("server socket creation failed");
		return;
	}
	SOCKADDR_IN hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(C_PORT);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (SOCKADDR*)& hint, sizeof(hint));
	listen(listening, SOMAXCONN);

	SOCKADDR_IN client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (SOCKADDR*)& client, &clientSize);

	char host[NI_MAXHOST];//client remote name
	char service[NI_MAXHOST];// service the client is connected on

	if (getnameinfo((SOCKADDR*)& client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		printf("connected on host :%s and service port :%s\n", host, service);
	}
	closesocket(listening);
	return clientSocket;
}

/*
===================================================
General : the function initalizes the TCP hdr.
Parameters : *tcphdr - represents tcp header of a packet
Return Value : None.
===================================================
*/

void server_create_tcp(TCP_HDR* tcphdr)
{
	tcphdr->source_port = htons(1024);
	tcphdr->dest_port = htons(1025);
	tcphdr->sequence = 0;
	tcphdr->acknowledge = 0;
	tcphdr->reserved_part1 = 0;
	tcphdr->data_offset = 5;
	tcphdr->fin = 0;
	tcphdr->syn = 1;
	tcphdr->rst = 0;
	tcphdr->psh = 0;
	tcphdr->ack = 0;
	tcphdr->urg = 0;
	tcphdr->ecn = 0;
	tcphdr->cwr = 0;
	tcphdr->window = htons(64240);
	tcphdr->checksum = 0;
	tcphdr->urgent_pointer = 0;
}

/*
===================================================
General : the function initalizes the IP hdr.
Parameters : *iphdr - represents the ip header of a packet
			  payload_size - represents the size of my payload
Return Value : None.
===================================================
*/

void server_create_ip(IPV4_HDR* iphdr, int payload_size) {
	iphdr->ip_version = 4;
	iphdr->ip_header_len = 5; //In double words thats 4 bytes
	iphdr->ip_tos = 0;
	iphdr->ip_total_length = htons(sizeof(IPV4_HDR) + sizeof(TCP_HDR) + payload_size);
	iphdr->ip_id = htons(2);
	iphdr->ip_frag_offset = 0;
	iphdr->ip_reserved_zero = 0;
	iphdr->ip_dont_fragment = 1;
	iphdr->ip_more_fragment = 0;
	iphdr->ip_frag_offset1 = 0;
	iphdr->ip_ttl = 64;
	iphdr->ip_protocol = IPPROTO_TCP;
	iphdr->ip_srcaddr = SRC_ADDR;
	iphdr->ip_destaddr = DEST_ADDR;
	iphdr->ip_checksum = 0;
}

/*
===================================================
General : the function initalizes the eth hdr.
Parameters : *ethdr - represents ethdr header of a packet
Return Value : None.
===================================================
*/
void server_create_eth(ETHER_HDR* ehdr)
{
	memset(ehdr->source, 0, 6); //Source Mac address
	memset(ehdr->dest, 0, 6); //Destination MAC address
	ehdr->type = htons(0x0800); //IP Frames
}

/*
============================================
General : function is responsible for receiving a length of data from the client
Parameters : sock - client socket to receive the data from
			 *buf - holds a pointer to the buffer that needs to update
			 bufsize - the length of the buffer

Return Value : returns TRUE when the data is read correctly
			   else, FALSE when there was a socket error or no bytes are received.
============================================
*/
bool recv_raw(SOCKET sock, void* buf, int bufsize)
{
	unsigned char* pbuf = (unsigned char*)buf;
	while (bufsize > 0) {
		int num = recv(sock, pbuf, bufsize, 0);
		if (num <= 0) { return false; }
		pbuf += num;
		bufsize -= num;
	}
	return true;
}

/*
===================================================
General : receives the length of the file and updates it

Parameters : sock - client socket to receive the data from
			 *filesize - holds a pointer to the size of the buffer that needs to update
			 filesize_len - the length of the file size pointer
Return Value : returns TRUE when the size is read correctly
			   else, FALSE when there was a socket error or no bytes are received.
===================================================
*/
bool recv_file_len(SOCKET sock, long* filesize)
{
	if (!recv_raw(sock, filesize, sizeof(*filesize))) { return false;}
	*filesize = ntohl(*filesize);
	return true;
}

/*
===================================================
General : writes to the lua file the data from the file
that was received in the socket
Parameters : sock - the socket between the client and server
			 *f - the file to write the data received to
Return Value : returns TRUE when everything was written to the file.
returns FALSE if there's no data received or detected a socket problem.
===================================================
*/
bool write_to_lua(SOCKET sock, FILE *f)
{
	long filesize;//size of address
	if (!recv_file_len(sock, &filesize)) { return false; }
	printf("file size (From C client) : %ld\n", filesize);
	if (filesize > 0)
	{
		char buffer[BUFFER_SIZE];
		//ZeroMemory(&buffer, BUFFER_SIZE);
		do {
			int num = min(filesize, BUFFER_SIZE);
			if (!recv_raw(sock, buffer, num)) {
				return false;
			}
			int offset = 0;
			do
			{
				size_t written = fwrite(&buffer[offset], 1, num - offset, f);
				if (written < 1) { return false; }
				offset += written;
			} while (offset < num);
			filesize -= num;
		} while (filesize > 0);
	}
	return true;
}

/*
===================================================
General : writes to the pcap file.
Parameters : *fd - file to write to
			 packet_header - holds the header for a packet
			 *ehdr - holds a pointer to the ethernet header
			 *iphdr - holds a pointer to the ip header
			 *tcphdr - holds a pointer to the tcp header
			 *data - holds a pointer to roy's protocol data
Return Value : None.
===================================================
*/
void write_to_file(FILE* fd, pcaprec_hdr_t packet_header, ETHER_HDR *ehdr, IPV4_HDR *iphdr, TCP_HDR* tcphdr, data* data)
{
	fwrite(&packet_header, sizeof(pcaprec_hdr_t), 1, fd);
	fwrite(ehdr, sizeof(ETHER_HDR), 1, fd);
	fwrite(iphdr, sizeof(IPV4_HDR), 1, fd);
	fwrite(tcphdr, sizeof(TCP_HDR), 1, fd);
	fwrite(data, strlen(data->payload), 1, fd);
}

/*
===================================================
General : checks the message request that was received from
the client.
Parameters : byteReceived - the number of bytes received from the client
			 *buf - a pointer to the data received
Return Value : return TRUE if there were no errors in the functions.
returns FALSE when there was a socket error or no bytes
received 
===================================================
*/
bool check_socket_buffer(int byteReceived)
{
	bool retval = false;
	if (byteReceived == SOCKET_ERROR)
	{
		printf("error in recv\n");
	}
	else if (byteReceived == 0)
	{
		printf("no bytes recvived\n");
	}
	else// no socket errors
	{
		retval = true;
	}
	return retval;

}

/*
===================================================
General : responsible of running the server and summons
all the functions for the server during the loop.
Parameters : clientSocket - the client socket
			 *buf - pointer to hold the data
			 *fd - holds a pointer to the pcap file
			 *fields - array of field structs that describe the protocol

Return Value : returns TRUE when the server when the server was handled with no problems , false otherwise
===================================================
*/
bool handle_server(SOCKET clientSocket, char*buf, FILE*fd, field* fields)
{
	ETHER_HDR ehdr;
	IPV4_HDR iphdr;
	TCP_HDR tcphdr;
	data packet_data;
	pcaprec_hdr_t packet_header;
	server_create_eth(&ehdr);
	server_create_tcp(&tcphdr);
	char *hex_str = NULL;
	bool condition = true;
	bool retval = false;
	int port = 1025;
	while (condition && !retval)
	{
		ZeroMemory(buf, BUFFER_SIZE);
		int byteReceived = recv(clientSocket, buf, BUFFER_SIZE, 0);
		hex_str = malloc(strlen(buf) * 2 + 1);
		strncpy(packet_data.payload, buf, BUFFER_SIZE);
		if (!check_socket_buffer(byteReceived))
		{
			condition = false;
		}
		if (strcmp(buf, EXIT_MSG) == 0)
		{
			printf("C Client has left the Server!\n");
			retval = true;
		}
		if (condition && !retval)
		{
			server_create_ip(&iphdr, strlen(packet_data.payload));
			packet_header = create_packet_header(&packet_data);
			write_to_file(fd, packet_header, &ehdr, &iphdr, &tcphdr, &packet_data);
			tcphdr.source_port = htons(port + 1);
			tcphdr.dest_port = htons(port);
			port++;
		}
		send(clientSocket, buf, byteReceived + 1, 0);//echo's back
	}
	if (hex_str)
	{
		free(hex_str);
	}
	return retval;
}

/*
===================================================
General : creates and puts parameters in the global header
Parameters : None.
Return Value : returns pcap_hdr_t with all the assigned parameters
===================================================
*/
pcap_hdr_t create_global_header()
{
	pcap_hdr_t header;
	header.magic_number = 0xa1b2c3d4;
	header.version_major = 2;
	header.version_minor = 4;
	header.thiszone = 0;
	header.sigfigs = 0;
	header.snaplen = 65535;
	header.network = 1;//data link layer = ethernet
	return header;
}

/*
===================================================
General : responsible on opening and writing to the lua file + writing the info to the fields struct
Parameters : clientSocket - the socket connecting between the server and the client
Return Value : returns TRUE if the file was written correctly and the fields were filled correctly
otherwise returns FALSE;
===================================================
*/
bool handle_lua_file(SOCKET clientSocket, field*fields)
{
	bool retval = false;
	FILE* filehandle = fopen(NEW_FILE_NAME, "wb");
	if (filehandle != NULL)
	{
		bool ok = write_to_lua(clientSocket, filehandle);
		fclose(filehandle);
		int lines = lines_in_lua();
		if (ok)
		{
			retval = fill_fields(lines, fields);
		}
	}
	return retval;
}

/*
===================================================
General : creates a specific packet header .
Parameters : *data - receives a pointer to the data part of my packet.
Return Value : pcaprec_hdr_t which represents a packet header type.
===================================================
*/
pcaprec_hdr_t create_packet_header(data* data)
{
	pcaprec_hdr_t packet_header;
	packet_header.ts = (struct pcap_timeval) { 0 };
	packet_header.incl_len = sizeof(ETHER_HDR) + sizeof(IPV4_HDR) + sizeof(TCP_HDR) + strlen(data->payload);
	packet_header.orig_len = sizeof(ETHER_HDR) + sizeof(IPV4_HDR) + sizeof(TCP_HDR) + strlen(data->payload);
	return packet_header;
}

/*
===================================================
General : Fills all the fields in the lua file that can be encountered
Parameters : lua_lines - number of lines in the lua file
Return Value : Returns TRUE when the struct was updated correctly , will
return FALSE when the file was not opened correctly / something in the array update went wrong
===================================================
*/
bool fill_fields(int lua_lines, field *fields)
{
	int i;
	bool retval = false;
	char str[READ_SIZE];
	int size;

	FILE* f = fopen(NEW_FILE_NAME, "rb");
	if (f == NULL)
	{
		printf("failed to open file f in fill_fields\n");
	}
	else
	{
		for (i = 0; i < lua_lines; i++)
		{
			memset(fields[i].name, 0, PARAM_MAX_SIZE);
			memset(fields[i].size, 0, PARAM_MAX_SIZE);
			memset(fields[i].opcode, 0, PARAM_MAX_SIZE);
			fgets(str, READ_SIZE, f);
			copy_file_fields(fields, lua_lines, str, i);
			check_protocol_sizes(fields, i);
		}
		fclose(f);
		print_field_description(lua_lines, fields);
		retval = true;
	}
	return retval;
}

/*
===================================================
General : Prints a description of what got into the field struct of pointers array
Parameters : lua_lines - number of lines in the lua file
			*fields - array of field structs that describe the protocol
Return Value : None.
===================================================
*/
void print_field_description(int lua_lines, field* fields)
{
	int i;
	for (i = 0; i < lua_lines; i++)
	{
		printf("Name[%d] %s\n", i, fields[i].name);
		printf("Size[%d] %s\n", i, fields[i].size);
		printf("Letters[%d] %d\n", i, fields[i].letters);
		printf("Opcode[%d] %s\n", i, fields[i].opcode); // no need for /n cuz when i read from the file it reads wit the \n
	}
}

/*
===================================================
General : Sets a size that can be defined in c for a struct
Parameters : *fields - array of field structs that describe the protocol
			 offset - the index of the current spot
Return Value : None.
===================================================
*/
void check_protocol_sizes(field * fields, int offset)
{
	if (strstr(fields[offset].size, "8") != NULL)
	{
		strncpy(fields[offset].size, "uint8_t", strlen(INT8));
		fields[offset].letters = 2;

	}
	if (strstr(fields[offset].size, "16") != NULL)
	{
		strncpy(fields[offset].size, "uint16_t", strlen(INT16));
		fields[offset].letters = 4;
	}

}

/*
===================================================
General : copies the name and the size of a field to the fields struct array
Parameters : *fields - array of field structs that describe the protocol
			 length - the length of the lua file
			 *str - the line that was read from the file
			 offset - the index of the current row
Return Value : Returns the number of lines in the lua file
===================================================
*/
void copy_file_fields(field * fields, int length, char * str, int offset)
{
	// Returns first token  
	char *token = strtok(str, SPACE);
	strncpy(fields[offset].name, token, strlen(token));
	token = strtok(NULL, SPACE);
	strncpy(fields[offset].size, token, strlen(token));
	token = strtok(NULL, SPACE);
	strncpy(fields[offset].opcode, token, strlen(token));


}

/*
===================================================
General : Calculates the number of lines in the lua file.
Parameters : None.
Return Value : Returns the number of lines in the lua file
===================================================
*/
int lines_in_lua()
{
	FILE* f = fopen(NEW_FILE_NAME, "rb");
	if (f == NULL)
	{
		printf("failed to open file f in lines_in_lua\n");
		return 0;
	}
	int lines = 0;
	char c;
	while ((c = fgetc(f)) != EOF)
	{
		if (c == '\n')
		{
			lines++;
		}
	}
	fclose(f);
	return lines;

}

/*
============================================
General : opens the server as a client socket in order to
connect to the python server.
Parameters : port - server port
			 ipAddress - servers ip address

Return Value : returns the socket that was opened
for the client in order to connect to the server.
============================================
*/
SOCKET open_socket(int port, char *ipAddress)
{
	WSADATA data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		printf("client can't start winsock\n");
		return;
	}
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		printf("can't create socket\n");
		WSACleanup();
		return;
	}
	SOCKADDR_IN hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress, &hint.sin_addr);

	//connect to server
	int connResult = connect(sock, (SOCKADDR*)& hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		printf("can't connect to a server\n");
		closesocket(sock);
		WSACleanup();
		return;
	}
	return sock;

}

/*
===================================================
General : responsible of running the connection between the C server and 
Python Server.
Parameters : sock - the connection socket
			 *pcap - pointer for the pcap file
			 *lua- pointer for the lua file 

Return Value : Will return True when the connection between the servers was finished
correctly , if there was an error in sending the file will return false;
===================================================
*/
bool handle_python(SOCKET sock, FILE *pcap,FILE *lua)
{
	bool condition = send_file(sock, pcap);
	bool retval = false;
	if (condition)
	{
		condition = send_file(sock, lua);
	}
	char input[BUFFER_SIZE];
	while (condition)
	{
		ZeroMemory(input, BUFFER_SIZE);
		scanf("%s", &input);
		if (strcmp(input, "exit") == 0)
		{
			condition = false;
			retval = true;
		}
	}
	return retval;
}

void main()
{
	SOCKET clientSocket = init_server();
	int lua_lines = lines_in_lua();
	field* fields = malloc(sizeof(field) * lua_lines);
	bool check = handle_lua_file(clientSocket, fields);
	char buf[BUFFER_SIZE];
	FILE* fd = fopen(PCAP_NAME, "wb");
	if (fd == NULL)
	{
		printf("file fd failed to open in main!\n");
	}
	else
	{
		pcap_hdr_t header = create_global_header();
		fwrite(&header, sizeof(header), 1, fd);
		//handles the C client - C server connection
		bool correct_handle = handle_server(clientSocket, buf, fd, fields);
		int i;
		fclose(fd);//finished with creating the pcap file , gotta send this to the python server
		if (correct_handle)
		{
			FILE *pcap = fopen(PCAP_NAME, "rb");
			FILE *lua = fopen(NEW_FILE_NAME, "rb");
			SOCKET pythonSocket = open_socket(PYTHON_PORT, LOCAL_HOST);
			bool pyret = handle_python(pythonSocket, pcap, lua);
			fclose(lua);
			fclose(pcap);
			closesocket(pythonSocket);
		}
	}
	free(fields);
	closesocket(clientSocket);
	WSACleanup();
	system("pause");
}
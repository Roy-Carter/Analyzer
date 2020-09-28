#include "client.h"

SOCKET open_socket(int, char*);
bool search_proto_in_file(char*, char*);
int find_char(char*, char);
bool handle_client(char*, char*, SOCKET);
bool send_file_length(SOCKET, long*, int);
bool convert_size(SOCKET, long);
bool send_file(SOCKET, FILE*);

/*
============================================
General : the function returns the index of the character in the string
Parameters : *str - the string to check
			 c - the letter to return the index of
Return Value : returns the index of the letter in the str
============================================
*/
int find_char(char* str, char c)
{
	int index = 0;
	while (*str != c)
	{
		index++;
		str++;
	}
	return index;
}
/*
============================================
General : searches for the fields in the lua file
and writes it to a new file
Parameters : *fname - pointer to the file name
			 *str - the string to search in the file
		
Return Value : returns TRUE if the string was found 
and a new file was created.
returns FALSE when one of the files was not opened correctly
============================================
*/
bool search_proto_in_file(char* fname, char* str) 
{
	bool retval = true;
	FILE* fp = fopen(fname, "rb");//read from file
	if (fp == NULL)
	{
		retval = false;
		printf("fp failed to open in search_proto_in_file\n");
	}
	FILE* f = fopen(NEW_FILE_NAME, "wb");//write to file
	if (f == NULL)
	{
		retval = false;
		printf("f failed to open in search_proto_in_file\n");
	}
	char temp[SIZE];
	int first_point_index;
	int second_point_index;
	int first_closer_index;
	int equal_index;
	if (retval)
	{
		while (fgets(temp, SIZE, fp) != NULL)
		{

			if (strstr(temp, str) != NULL)
			{
				first_point_index = find_char(temp, '.');
				first_closer_index = find_char(temp, '(');
				equal_index = find_char(temp, '=');
				second_point_index = find_char(temp + equal_index, '.');
				second_point_index += equal_index;

				// + 1 to not copy the position itself , -1 for not copying one letter afterwards
				fwrite(temp + first_point_index + 1, equal_index - first_point_index - 1, 1, f);
				fwrite(temp + second_point_index + 1, first_closer_index - second_point_index - 1, 1, f);
				puts(temp);
				fwrite("\n", 1, 1, f);
			}
		}
		fclose(fp);
		fclose(f);
	}
	return retval;
}

/*
============================================
General : function is responsible for sending the length of the file to the server
Parameters : sock - socket connection between client and server
			 *filesize - holds a pointer to the size that needs to be sent
			 filesize_len - the length of the file size pointer

Return Value : returns TRUE when the length of the data was sent correctly.
returns FALSE when there was a socket error.
============================================
*/
bool send_file_length(SOCKET sock, long* filesize, int filesize_len)
{
	bool retval = true;
	unsigned char* pbuf = (unsigned char*)filesize;
	int num = send(sock, pbuf, filesize_len, 0);
	if (num == SOCKET_ERROR)
	{
		retval = false;
	}
	return retval;
}

/*
============================================
General : transfers the size to network byte order
and sends data to the server
Parameters : sock - socket for the client - server connection
			 filesize - the value of the file size

Return Value : returns TRUE when the length of the data was sent correctly.
returns FALSE when there was a socket error. 

============================================
*/
bool convert_size(SOCKET sock, long filesize)
{
	filesize = htonl(filesize);
	return send_file_length(sock, &filesize, sizeof(filesize));
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
	bool retval = true;
	fseek(f, 0, SEEK_END);
	long filesize = ftell(f);
	rewind(f);
	if (filesize == EOF)
	{
		retval = false;
	}
	if (retval && !convert_size(sock, filesize))
	{
		retval = false;
	}
	if (filesize > 0 && retval)
	{
		char buffer[BUFFER_SIZE];
		while (filesize > 0 && retval)
		{
			size_t num = filesize;
			num = fread(buffer, 1, num, f);
			if (num < 1) {
				retval = false;
			}
			if (!send(sock, buffer, num, 0))
			{
				retval = false;
			}
			filesize -= num;
		}
	}
	return retval;
}

/*
============================================
General : opens the client socket in order to 
connect to the server.
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
============================================
General : sends the packet from the client
to the server in order to check them
Parameters : *userInput -a pointer to the id of the packet to send or 
			 an exit msg
			 *buf - the pointer to the data
			 sock - socket to send data through

Return Value : Returns true when the client has left with exit,
returns false when there was an error in socket
============================================
*/
bool handle_client(char* userInput, char* buf, SOCKET sock)
{
	char packet1[] = { /* Packet 1 */
	0x01, 0x01, 0x03, 0x01,0x01, 0};//working 1

	char packet2[] = { /* Packet 2 */
	0x02, 0x01, 0x03, 0x02,0x02 , 0 };//working 2

	char packet3[] = { /* Packet 3 */
	0x01, 0x05, 0x04, 0x05,0x04, 0 };//incorrect

	char packet4[] = { /* Packet 4 */
	0x02, 0x01, 0x03, 0x01,0x01, 0 };//incorrect
	int sendResult;
	bool condition = true;
	bool retval = false;
	while (condition) 
	{
		printf(">> ");
		scanf("%s", userInput);
		if (strcmp(userInput, "p1") == 0)
		{
			sendResult = send(sock, packet1, strlen(packet1), 0);
		}
		else if(strcmp(userInput, "p2") == 0)
		{
			sendResult = send(sock, packet2, strlen(packet2), 0);
		}
		else if (strcmp(userInput, "p3") == 0)
		{
			sendResult = send(sock, packet3, strlen(packet3), 0);
		}
		else if (strcmp(userInput, "p4") == 0)
		{
			sendResult = send(sock, packet4, strlen(packet4), 0);
		}
		else
		{
			sendResult = send(sock, userInput, strlen(userInput), 0);
		}
		if (strcmp(userInput, "exit") == 0)
		{
			retval = true;
			condition = false;
		}
		if (sendResult != SOCKET_ERROR)
		{
			ZeroMemory(buf, BUFFER_SIZE);
			int bytesReceived = recv(sock, buf, BUFFER_SIZE, 0);
			if (bytesReceived > 0)
			{
				printf("server %s\n", buf);
			}
		}
	}
	return retval;
}

void main()
{
	bool num = search_proto_in_file(FILE_NAME, "ProtoField");
	char* ipAddress = SERVER_IP;
	int port = PORT;
	SOCKET sock = open_socket(port, ipAddress);

	FILE* filehandle = fopen(NEW_FILE_NAME, "rb");//the new lua file
	if (filehandle != NULL)
	{
		bool check = send_file(sock, filehandle);
		fclose(filehandle);
	}
	char buf[BUFFER_SIZE];
	char userInput[INPUT_LEN];
	bool check = handle_client(userInput, buf, sock);
	if (check == TRUE)
	{
		printf("Client has left the server\n");
	}
	closesocket(sock);
	WSACleanup();
	system("pause");
}

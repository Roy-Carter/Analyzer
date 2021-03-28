#include "client.h"
SOCKET open_socket(int, char*);
bool search_proto_in_file(char*, char*);
int find_char(char*, char);
bool handle_client(char*, char*, SOCKET);
bool send_file(SOCKET, FILE*);
bool send_file_length(SOCKET, long);
bool send_raw(SOCKET, void*, int);
void remove_spaces(char*);
char* decisive_parameter(char*);
int search_opcode(char*);
bool search_fields_for_opcode(FILE*, char*);
char *search_sized_substr(char*, int, char*, int);
int detect_opcode_number(char*);
void update_descriptor(FILE*, lua_line*);
void write_to_file(FILE*, lua_line*);
bool update_line(lua_line *, FILE *, char *, int, int, int, int);

/*
============================================
General : This function prints a full descriptor of a specific lua line and updates the file.
Parameters : *lua_descriptor - the file to write the lua line into.
			 *line - a specific line structure to write to the file.
Return Value : None.
============================================
*/
void update_descriptor(FILE *lua_descriptor, lua_line *line)
{
	write_to_file(lua_descriptor, line);
	VAL_PRINT(line->name, s);
	VAL_PRINT(line->opcode, d);
	VAL_PRINT(line->str_size, s);
}
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
	while (*str != c) {
		index++;
		str++;
	}
	return index;
}

/*
============================================
General : This function purpose is to find the parameter that determines
		  how the protocol is split.
Parameters : *fp - the lua dissector file.

Return Value : returns the name of the decisive parameter of the protocol.
============================================
*/
char* decisive_parameter(char * fname)
{
	FILE* fp = fopen(FILE_NAME, "rb");//read from file
	if (fp == NULL) { OPEN_FAILED(fp, decisive_parameter); }
	char data[SIZE];
	bool flag = true;
	int equal_index, first_sqr_bracket, second_sqr_bracket;
	char *decisive_opcode = malloc(sizeof(char));
	if (decisive_opcode) {
		while (fgets(data, SIZE, fp) != NULL && flag) {
			if (strstr(data, "cols['info']") != NULL) {
				remove_spaces(data);
				equal_index = find_char(data, EQUAL_VAL);
				first_sqr_bracket = find_char(data + equal_index, '[') + equal_index;
				second_sqr_bracket = find_char(data + equal_index, ']') + equal_index;
				decisive_opcode = realloc(decisive_opcode, sizeof(char) * SUB_AND_DEC(second_sqr_bracket, first_sqr_bracket));
				if (decisive_opcode == NULL) { flag = false; }
				strncpy(decisive_opcode, ADD_AND_INC(data, first_sqr_bracket), SUB_AND_DEC(second_sqr_bracket, first_sqr_bracket));
				decisive_opcode[second_sqr_bracket - first_sqr_bracket - 1] = END_LINE_VAL;
				flag = false; // cols info shows only one time;
			}
		}
	}
	return decisive_opcode;
}

/*
============================================
General : The function is responsible of removing the spaces from a certain string
Parameters : *input - an input string to the function to remove spaces from
Return Value : None.
============================================
*/
void remove_spaces(char* input)
{
	if (input) {
		while (*input == '\t' || *input == ' ') {
			*input += 1;
		}
	}
}

/*
============================================
General : This function finds the start of the first occurrence of
		  the substring search_for of length search_for_len in the memory area
		  of search_in of length search_in_len.
		  the problem is that it searches for the full row , looking for spaces.
Parameters : *search_in - the string to search in .
			 search_in_len - the length of the string to search in.
			 *search_for - the string to search for.
			 search_for_len - the length of the string to search for.
Return Value : Returns a pointer to the start of the first occurrence of the substring.
============================================
*/
char *search_sized_substr(char *search_in, int search_in_len, char *search_for, int search_for_len)
{
	if (search_in == NULL) return NULL;
	if (search_in_len == 0) return NULL;
	if (search_for == NULL) return NULL;
	if (search_for_len == 0) return NULL;

	for (char *h = search_in; search_in_len >= search_for_len; ++h, --search_in_len) {
		if (!memcmp(h, search_for, search_for_len)) {
			return h;
		}
	}
	return NULL;
}

/*
============================================
General : This function purpose is to find wether the name of a specific line of the lua file is within an "if" section of the code
that indicates  to what opcode the line_name belongs to.
Parameters : *fp - file pointer , holds a pointer to the current position in the file
			 *line_name - the name of the lua line to add an opcode to.
Return Value :Returns true if the search was successful between the fp and the first "end" it sees (an if section) , otherwise if it fails,
return false.
============================================
*/
bool search_fields_for_opcode(FILE *fp, char * line_name)
{
	bool retval = false;
	char data[SIZE];
	while (!retval && fgets(data, SIZE, fp) != NULL && !(strstr(data, "end"))) {
		if (strstr(data, "add") != NULL && search_sized_substr(data, strlen(data), line_name, strlen(line_name)) != NULL) {//memmem needed because of the \0 (for example p_type\0)
			retval = true;
		}
	}
	return retval;
}
/*
============================================
General : The purpose of this function is to detect an opcode that represent
		  the current range that is being checked by the decisive parameter.
Parameters : *data - the data line that is read from the lua file.
Return Value : returns the opcode.
============================================
*/
int detect_opcode_number(char * data)
{
	int first_after, equal_index, i, second, num;
	char * str_to_num;
	equal_index = INC(find_char(data, EQUAL_VAL));
	first_after = find_char(data + equal_index, SPACE_VAL) + equal_index;
	second = find_char(ADD_AND_INC(data, first_after), SPACE_VAL) + INC(first_after);// +1 to jump to the next space
	str_to_num = malloc(sizeof(char)*(second - first_after));
	if (str_to_num == NULL) { num = -1; }
	else {
		strncpy(str_to_num, data + first_after, second - first_after);
		num = atoi(str_to_num);
		free(str_to_num);
	}
	return num;
}

/*
============================================
General : This function purpose is to find the correct opcode to a specific
		  line in the lua file.
Parameters : *lua_line_name - the name of the lua line to add an opcode to.
Return Value : returns the opcode if it found one , otherwise returns 0 if its a header
			   (not part of any if statements) or returns -1 if the file has failed to open or the conversion allocation
			   of the opcode was unsuccessful.
============================================
*/
int search_opcode(char * lua_line_name)
{
	int num = 0;
	FILE* fp = fopen(FILE_NAME, "rb");//read from file
	if (fp == NULL) {
		num = -1;
		OPEN_FAILED(fp, search_opcode);
	}
	if (num != -1) {
		char* decisive = decisive_parameter(FILE_NAME); //p_type
		char data[SIZE];
		while (fgets(data, SIZE, fp) != NULL) {
			if (strstr(data, decisive) != NULL && strstr(data, "if")) {
				if (search_fields_for_opcode(fp, lua_line_name)) {//if its within this part of the code
					num = detect_opcode_number(data);
					printf("the field: %s is a sub field of the message type --> opcode: %d\n", lua_line_name, num);
					return num;
				}
			}
		}
	}
	fclose(fp);
	return num;
}

/*
============================================
General : Writes a specific line descriptor into the lua descriptor file
Parameters : *lua_descriptor -  the lua file to write into.
			 *line - the lua line that contains the data to write into the file.
Return Value : None.
============================================
*/
void write_to_file(FILE* lua_descriptor, lua_line *line)
{
	WRITE_F(lua_descriptor, s, line->name);
	WRITE_F(lua_descriptor, s, line->str_size);
	WRITE_F(lua_descriptor, c, SPACE_VAL);
	WRITE_F(lua_descriptor, d, line->opcode);
	WRITE_F(lua_descriptor, s, "\n");
}

/*
============================================
General : This function updates the values of a specific line struct
Parameters : *line - the lua line struct to update.
			 *lua_descriptor - the descriptor file to update in.
			 *row_data - a pointer to the row data that was read from the file.
			 first_point_index - the index of the first point in a parameter line.
			 second_point_index - the index of the second point in a parameter line.
			 first_parenthesis_index - the first parenthesis index in the parameter line.
			 equal_index - the index of the "=" character in the parameter line.
Return Value : returns true if the update was done , otherwise returns false if the mallocs weren't successful.
============================================

*/
bool update_line(lua_line * line, FILE *lua_descriptor, char *row_data, int first_point_index, int second_point_index, int first_pthesis_index, int equal_index)
{
	bool retval = true;
	line->name = malloc(sizeof(char)* SUB_AND_INC(equal_index, first_point_index));//+1 for a pointer , otherwise , unknown crt behaviour
	line->str_size = malloc(sizeof(char)* SUB_AND_INC(first_pthesis_index, second_point_index));
	if (line->name == NULL || line->str_size == NULL) { retval = false; }
	else {
		strncpy(line->name, ADD_AND_INC(row_data, first_point_index), SUB_AND_DEC(equal_index, first_point_index));
		line->name[SUB_AND_DEC(equal_index, first_point_index)] = END_LINE_VAL;

		strncpy(line->str_size, ADD_AND_INC(row_data, second_point_index), SUB_AND_DEC(first_pthesis_index, second_point_index));
		line->str_size[SUB_AND_DEC(first_pthesis_index, second_point_index)] = END_LINE_VAL;

		line->opcode = search_opcode(line->name);
		update_descriptor(lua_descriptor, line);
		free(line->name);
		free(line->str_size);
	}
	return retval;
}
/*
create a function for adding to a struct
============================================
General : Manages the build of the lua file by building a specific line and updating the file.
Parameters : *row_data - a specific row that was read from the file.
			 *line - the lua line that contains the data to write into the file.
Return Value :
============================================
*/
bool build_lua_line(char *row_data, FILE *lua_descriptor)
{
	bool retval = true;
	int first_point_index, second_point_index, first_parenthesis_index, equal_index;
	lua_line *line = malloc(sizeof(lua_line));
	if (line == NULL) { retval = false; }
	first_point_index = find_char(row_data, POINT_VAL);
	first_parenthesis_index = find_char(row_data, '(');
	equal_index = find_char(row_data, EQUAL_VAL);
	second_point_index = find_char(row_data + equal_index, POINT_VAL) + equal_index;
	retval = update_line(line, lua_descriptor, row_data, first_point_index, second_point_index, first_parenthesis_index, equal_index);
	free(line);
	return retval;
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
	if (fp == NULL) {
		retval = false;
		OPEN_FAILED(fp, search_proto_in_file)
	}
	FILE* lua_descriptor = fopen(NEW_FILE_NAME, "wb");//write to file
	if (lua_descriptor == NULL) {
		retval = false;
		OPEN_FAILED(lua_descriptor, search_proto_in_file)
	}
	char data[SIZE];
	if (retval) {
		while (fgets(data, SIZE, fp) != NULL) {
			if (strstr(data, str) != NULL) {
				if (build_lua_line(data, lua_descriptor)) {
					puts("Added to LUA successfully!");
					puts("=========================================================================");
				}
			}
		}
		fclose(fp);
		fclose(lua_descriptor);
	}
	return retval;
}
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
	if (fseek(f, 0, SEEK_END) != 0) { return false; }
	long filesize = ftell(f);
	rewind(f);
	if (filesize == -1L) { return false; }
	if (!send_file_length(sock, filesize)) { return false; }
	if (filesize > 0) {
		char buffer[BUFFER_SIZE];
		do {
			size_t num = fread(buffer, 1, min(filesize, BUFFER_SIZE), f);
			if (num < 1) { return false; }
			if (!send_raw(sock, buffer, num)) { return false; }
			filesize -= num;
		} while (filesize > 0);
	}
	return true;
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
	if (wsResult != 0) {
		printf("client can't start winsock\n");
		return;
	}
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("can't create socket\n");
		WSACleanup();
		return;
	}
	SOCKADDR_IN hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress, &hint.sin_addr);

	int connResult = connect(sock, (SOCKADDR*)& hint, sizeof(hint));
	if (connResult == SOCKET_ERROR) {
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
	0x01, 0x01, 0x03, 0x01,0x01, 0 };//working 1

	char packet2[] = { /* Packet 2 */
	0x02, 0x01, 0x03, 0x02,0x03,0x02 , 0 };//working 2

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
		else if (strcmp(userInput, "p2") == 0)
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
	if (!recv_raw(sock, filesize, sizeof(*filesize))) { return false; }
	return true;
}

/*
================================================== =
General : writes to the lua file the data from the file
	that was received in the socket
	Parameters : sock - the socket between the client and server
	*f - the file to write the data received to
	Return Value : returns TRUE when everything was written to the file.
	returns FALSE if there's no data received or detected a socket problem.
	================================================== =
	*/
bool write_to_lua(SOCKET sock, FILE *f)
{
	long filesize;//size of address
	if (!recv_file_len(sock, &filesize)) { return false; }
	printf("file size (From C#) : %ld\n", filesize);
	if (filesize > 0)
	{
		char buffer[BUFFER_SIZE];
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

bool connect_gui()
{
	//================================================================
	FILE* filehandle1 = fopen("test.lua", "wb");//the new lua file
	if (filehandle1 == NULL)
	{
		fclose(filehandle1);
		printf("GUI CONNECT file failed to open!\n");
	}
	else {
		SOCKET sock1 = open_socket(5555, SERVER_IP);
		bool check = write_to_lua(sock1, filehandle1);
		if (check) {
			printf("file has been received from GUI!\n");
			puts("=========================================================================");
			fclose(filehandle1);
		}
	}
	//================================================================
}
void main()
{
	connect_gui();
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

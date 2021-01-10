#include "client.h"
SOCKET open_socket(int, char*);
bool search_proto_in_file(char*, char*);
int find_char(char*, char);
bool handle_client(char*, char*, SOCKET);
bool send_file_length(SOCKET, long*, int);
bool convert_size(SOCKET, long);
bool send_file(SOCKET, FILE*);
void remove_spaces(char*);
char* decisive_parameter(char*);
int search_opcode(char*);
bool search_fields_for_opcode(FILE*, char*);
char *search_sized_substr(char*, int, char*, int);
int detect_opcode_number(char*);
void update_descriptor(FILE*, lua_line*);
void write_to_file(FILE*, lua_line*);

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
	printf("line name = %s\n", line->name);
	printf("line opcode = %d\n", line->opcode);
	printf("str_size = %s\n", line->str_size);
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
	while (*str != c){
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
	if (fp == NULL){ printf("fp failed to open in decisive_parameter\n");}
	char data[SIZE];
	int equal_index,  first_closer, second_closer;
	char *decisive_opcode = malloc(sizeof(char));
	bool flag = true;
	if (decisive_opcode){
		while (fgets(data, SIZE, fp) != NULL && flag){
			if (strstr(data, "cols['info']") != NULL){
				remove_spaces(data);
				equal_index = find_char(data, EQUAL_VAL);
				first_closer = find_char(data + equal_index, '[') + equal_index;
				second_closer = find_char(data + equal_index, ']') + equal_index;
				decisive_opcode = realloc(decisive_opcode, sizeof(char) *(second_closer - first_closer - 1));
				if (decisive_opcode == NULL) { flag = false; }
				strncpy(decisive_opcode, data + first_closer + 1, second_closer - first_closer - 1);
				decisive_opcode[second_closer - first_closer - 1] = END_LINE_VAL;
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
	while (*input == '\t' || *input == ' '){
		*input += 1;
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
char *search_sized_substr(char *search_in, int search_in_len,char *search_for,int search_for_len)
{
	if (search_in == NULL) return NULL;
	if (search_in_len == 0) return NULL;
	if (search_for == NULL) return NULL; 
	if (search_for_len == 0) return NULL;

	for (char *h = search_in; search_in_len >= search_for_len; ++h, --search_in_len){
		if (!memcmp(h, search_for, search_for_len)){
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
bool search_fields_for_opcode(FILE *fp , char * line_name)
{
	bool retval = false;
	char temp[BUFFER_SIZE];
	while (!retval && fgets(temp, SIZE, fp) != NULL && !(strstr(temp, "end"))){
		if (strstr(temp,"add") != NULL && search_sized_substr(temp, strlen(temp), line_name, strlen(line_name)) != NULL){//memmem needed because of the \0 (for example p_type\0)
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
	int first_after, equal_index, i, second , num;
	char * str_to_num;
	equal_index = find_char(data, EQUAL_VAL) + 1;
	first_after = find_char(data + equal_index, SPACE_VAL) + equal_index;
	second = find_char(data + first_after + 1, SPACE_VAL) + (first_after + 1);// +1 to jump to the next space
	str_to_num = malloc(sizeof(char)*(second - first_after));
	if (str_to_num == NULL) { num = -1; }
	else{
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
	if (fp == NULL){
		num = -1;
		printf("fp failed to open in search_opcode\n");
	}
	if (num != -1){
		char* decisive = decisive_parameter(FILE_NAME); //p_type
		char data[BUFFER_SIZE];
		while (fgets(data, SIZE, fp) != NULL){
			if (strstr(data, decisive) != NULL && strstr(data, "if")){
				if (search_fields_for_opcode(fp, lua_line_name)){//if its within this part of the code
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
	fprintf(lua_descriptor, "%s", line->name);
	fprintf(lua_descriptor, "%s", line->str_size);
	fprintf(lua_descriptor, "%s", " ");
	fprintf(lua_descriptor, "%d", line->opcode);
	fwrite("\n", 1, 1, lua_descriptor);
}

/*
FIX THIS FUNCTION
NEEDS TO BE  SPLIT INTO FUNCTIONS
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
	int first_point_index, second_point_index, first_closer_index, equal_index;
	lua_line *line = malloc(sizeof(lua_line));
	if (line == NULL) { retval = false; }
	first_point_index = find_char(row_data, POINT_VAL);
	first_closer_index = find_char(row_data, '(');
	equal_index = find_char(row_data, EQUAL_VAL);
	second_point_index = find_char(row_data + equal_index, POINT_VAL);
	second_point_index += equal_index;

	line->name = malloc(sizeof(char)* equal_index - first_point_index + 1);//+1 for a pointer , otherwise , unknown crt behaviour
	if (line->name == NULL) { retval = false; }
	strncpy(line->name, row_data + first_point_index + 1, equal_index - first_point_index - 1);
	line->name[equal_index - first_point_index - 1] = END_LINE_VAL;

	line->str_size = malloc(sizeof(char)* first_closer_index - second_point_index + 1);
	if (line->str_size == NULL) { retval = false; }
	strncpy(line->str_size, row_data + second_point_index + 1, first_closer_index - second_point_index - 1);
	line->str_size[first_closer_index - second_point_index - 1] = END_LINE_VAL;

	line->opcode = search_opcode(line->name);
	
	update_descriptor(lua_descriptor, line);
	free(line->name);
	free(line->str_size);
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
	if (fp == NULL){
		retval = false;
		printf("fp failed to open in search_proto_in_file\n");
	}
	FILE* lua_descriptor = fopen(NEW_FILE_NAME, "w");//write to file
	if (lua_descriptor == NULL){
		retval = false;
		printf("f failed to open in search_proto_in_file\n");
	}
	char temp[SIZE];
	if (retval){
		while (fgets(temp, SIZE, fp) != NULL){
			if (strstr(temp, str) != NULL){
				if (build_lua_line(temp, lua_descriptor)){
					puts("Line was successfully added to the lua file !");
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
	if (num == SOCKET_ERROR){retval = false;}
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
	if (filesize == EOF) { retval = false; }
	if (retval && !convert_size(sock, filesize)) { retval = false; }
	if (filesize > 0 && retval){
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
	if (wsResult != 0){
		printf("client can't start winsock\n");
		return;
	}
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET){
		printf("can't create socket\n");
		WSACleanup();
		return;
	}
	SOCKADDR_IN hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress, &hint.sin_addr);

	int connResult = connect(sock, (SOCKADDR*)& hint, sizeof(hint));
	if (connResult == SOCKET_ERROR){
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

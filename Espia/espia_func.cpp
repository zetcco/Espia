#include "includes/espia_func.h"

void whoami(LPWSTR buffer, INT size_buffer) {
    DWORD size_comp = size_buffer;
    if (GetComputerNameW(buffer, &size_comp) == 0) {
        Debug(printf("Failed to retrieve the Computer Name : %d\n", GetLastError());)
        wcscat(buffer, L"<failed>");
        size_comp = 8;
    }
    wcscat(buffer + size_comp, L"@");
    size_comp++;
    DWORD size_usr = size_buffer;
    if (GetUserNameW(buffer + size_comp, &size_usr) == 0) {
        Debug(printf("Failed to retrieve the Hostname : %d\n", GetLastError()));
        wcscat(buffer, L"<failed>");
    }
}

#include "includes/espia_func.h"

void whoami(LPSTR buffer, INT size_buffer) {
    DWORD size = size_buffer;
    if (GetComputerNameA(buffer, &size) == 0) {
        Debug(printf("Failed to retrieve the Computer Name : %d\n", GetLastError());)
        strcat(buffer, "<failed>");
        size = 8;
    }
    strcat(buffer + size, "@");
    size++;
    if (GetUserNameA(buffer + size, &size) == 0) {
        Debug(printf("Failed to retrieve the Hostname : %d\n", GetLastError()));
        strcat(buffer, "<failed>");
    }
}
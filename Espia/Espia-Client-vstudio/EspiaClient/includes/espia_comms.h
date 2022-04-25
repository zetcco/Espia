#pragma once

#include "espia_error.h"
#include "espia_debug.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#if DEBUG
#include <stdio.h>
#endif

#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")

WORD espia_connect(PCSTR server_ip, PCSTR server_port);
WORD espia_disconnect();
INT espia_send(PWSTR send_buffer, int size_send_buffer);
INT espia_recv(PSTR recv_buffer, int size_recv_buffer);
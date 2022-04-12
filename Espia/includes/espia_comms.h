#pragma once

#include "espia_error.h"
#include "espia_debug.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#if DEBUG
#include <stdio.h>
#endif

#pragma comment(lib, "Ws2_32.lib")

WORD espia_connect(PCSTR server_ip, PCSTR server_port, SOCKET * connection);
WORD espia_disconnect(SOCKET * connection_socket);
INT espia_send(SOCKET* connection_socket, PWSTR send_buffer, int size_send_buffer);
INT espia_recv(SOCKET * connection_socket, PSTR recv_buffer, int size_recv_buffer);
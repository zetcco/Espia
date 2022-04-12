#include "includes/espia_comms.h"

WORD espia_connect(PCSTR server_ip, PCSTR server_port, SOCKET * connection_socket) {
	// Initialize the WSA
	WSADATA wsaData;
	int err;
	if ((err = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		Debug(printf("WSAStatup failed: %d\n", err);)
		return WSASTARTUP_FAIL;
	}

	// Create the socket
	struct addrinfo	* result = NULL,
					* ptr = NULL,
					hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve Server socket address
	if ((err = getaddrinfo(server_ip, server_port, &hints, &result)) != 0) {
		Debug(printf("gedaddrinfo failed: %d\n", err);)
		return ADDR_RESOLVE_FAIL;
	}

	// Initialize the Socket
	ptr = result;
	*connection_socket = INVALID_SOCKET;
	if ((*connection_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == INVALID_SOCKET) {
		Debug(printf("socket() failed: %ld\n", WSAGetLastError());)
		freeaddrinfo(ptr);
		WSACleanup();
		return SOCKET_INIT_FAIL;
	}

	// Connect to the server
	struct addrinfo* iter = ptr;
	while (iter != NULL && ((err = connect(*connection_socket, iter->ai_addr, (int)iter->ai_addrlen)) != 0))
		iter = iter->ai_next;
	freeaddrinfo(ptr);
	if (err == SOCKET_ERROR) {
		Debug(printf("Failed to connect to server\n");)
		closesocket(*connection_socket);
		return CONNECTION_FAIL;
	}

	return *connection_socket;
}

WORD espia_disconnect(SOCKET * connection_socket) {
	// Inform server about the connection termination
	int err;
	err = shutdown(*connection_socket, SD_SEND);
	if (err == SOCKET_ERROR) {
		Debug(printf("shutdown failed: %d\n", WSAGetLastError());)
		closesocket(*connection_socket);
		WSACleanup();
		return DISCONNECTION_FAIL;
	}

	// Close the socket and stop listening
	closesocket(*connection_socket);
	WSACleanup();
	return 0;
}

INT espia_send(SOCKET * connection_socket, PCSTR send_buffer, int size_send_buffer) {
	int err;
	if ((err = send(*connection_socket, send_buffer, size_send_buffer, 0)) == SOCKET_ERROR) {
		Debug(printf("send failed: %d\n", WSAGetLastError());)
		return SEND_FAIL;
	}
	else if (err == WSAEMSGSIZE) {
		Debug(printf("Given message size (%d) exceeds the Maximum message size\n", size_send_buffer);)
		return SEND_FAIL_INVALID_SIZE;
	}

	return err;
}

INT espia_recv(SOCKET * connection_socket, PSTR recv_buffer, int size_recv_buffer) {
    int err;
    if ((err = recv(*connection_socket, recv_buffer, size_recv_buffer, 0)) == SOCKET_ERROR) {
        Debug(printf("Recieve failed: %d\n", WSAGetLastError()));
        return RECV_FAIL;
    } else if (err == 0) {
        Debug(printf("Connection closed\n");)
    }
    return err;
}
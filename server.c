#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 13920
#define BUFFER_SIZE 1024

void send_file(SOCKET client_socket, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("File not found: %s\n", filename);
        send(client_socket, "File not found", sizeof("File not found"), 0);
        return;
    }

    char buffer[BUFFER_SIZE];
    int read_bytes;

    while ((read_bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_socket, buffer, read_bytes, 0);
    }

    fclose(file);
}

void handle_client(SOCKET client_socket) {
    char filename[BUFFER_SIZE];
    int received_bytes;

    // Receive the filename from the client
    if ((received_bytes = recv(client_socket, filename, sizeof(filename), 0)) <= 0) {
        printf("Client disconnected\n");
        closesocket(client_socket);
        return;
    }

    // Null-terminate the received filename
    filename[received_bytes] = '\0';

    // Send the file content to the client
    send_file(client_socket, filename);

    // Close the client socket when done
    closesocket(client_socket);
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int client_addr_len = sizeof(client_address);
    HANDLE thread_handle;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock\n");
        return 1;
    }

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        return 1;
    }

    // Initialize server address struct
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        printf("Binding failed\n");
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listening failed\n");
        return 1;
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Accept a connection from a client
        if ((client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_addr_len)) == INVALID_SOCKET) {
            printf("Acceptance failed\n");
            continue;
        }

        printf("Accepted connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Create a new thread to handle the client
        thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handle_client, (LPVOID)client_socket, 0, NULL);
        CloseHandle(thread_handle);  // Close the thread handle to avoid resource leak
    }

    // Close the server socket
    closesocket(server_socket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}

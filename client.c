#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT  13920
#define BUFFER_SIZE 1024

void receive_file(SOCKET server_socket, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error creating file: %s\n", filename);
        return;
    }

    char buffer[BUFFER_SIZE];
    int received_bytes;

    while ((received_bytes = recv(server_socket, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, received_bytes, file);
    }

    fclose(file);
}

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server_address;
    char filename[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed to initialize Winsock\n");
        return 1;
    }

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        return 1;
    }

    // Initialize server address struct
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("192.168.8.139");  // Change this to the server's IP address
    server_address.sin_port = htons(PORT);

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        printf("Connection failed\n");
        return 1;
    }

    printf("Connected to server\n");

    // Get the filename from the user
    printf("Enter the filename to request from the server: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = '\0';  // Remove the newline character

    // Send the filename to the server
    send(client_socket, filename, strlen(filename), 0);

    // Receive the file content from the server
    receive_file(client_socket, filename);

    // Display the content of the received file
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file: %s\n", filename);
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    printf("Content of %s:\n", filename);

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    fclose(file);

    // Close the client socket
    closesocket(client_socket);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
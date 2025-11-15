#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/select.h>

#define MAX_CLIENTS 1024
#define BUFFER_SIZE 2048

void handle_client_request(int clientSocket);

int main(int argc, char *argv[]) {
    int listenfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    
    fd_set master_set, read_set;
    int client_sockets[MAX_CLIENTS];
    int max_fd;
    int i;
    
    // Initialize client sockets array
    for (i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = -1;
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    int yes = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
        perror("setsockopt failed");
        close(listenfd);
        exit(1);
    }

    // Set socket to non-blocking mode
    int flags = fcntl(listenfd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl F_GETFL failed");
        close(listenfd);
        exit(1);
    }
    if (fcntl(listenfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl F_SETFL failed");
        close(listenfd);
        exit(1);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8080);
    

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Bind failed");
        close(listenfd);
        exit(1);
    }


    if (listen(listenfd, 10) < 0) {
        perror("Listen failed");
        close(listenfd);
        exit(1);
    }
    
    printf("listening port 8080...\n");
    
    // Initialize master set with listening socket
    FD_ZERO(&master_set);
    FD_SET(listenfd, &master_set);
    max_fd = listenfd;
    
    while(1) {
        // Copy master set to read set for select()
        read_set = master_set;

        int activity = select(max_fd + 1, &read_set, NULL, NULL, NULL);
        
        if (activity < 0 && errno != EINTR) {
            perror("select error");
            break;
        }
        
        // Check if listening socket has new connection
        if (FD_ISSET(listenfd, &read_set)) {
            socklen_t size = sizeof(client_addr);
            int clientSocket = accept(listenfd, (struct sockaddr*) &client_addr, &size);
            
            if (clientSocket >= 0) {
                // Set client socket to non-blocking
                flags = fcntl(clientSocket, F_GETFL, 0);
                if (flags >= 0) {
                    fcntl(clientSocket, F_SETFL, flags | O_NONBLOCK);
                }
                
                // Add to client sockets array
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (client_sockets[i] == -1) {
                        client_sockets[i] = clientSocket;
                        FD_SET(clientSocket, &master_set);
                        if (clientSocket > max_fd) {
                            max_fd = clientSocket;
                        }
                        printf("Connection established on socket %d\n", clientSocket);
                        break;
                    }
                }
                
                if (i == MAX_CLIENTS) {
                    printf("Too many clients, closing connection\n");
                    close(clientSocket);
                }
            } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("Accept failed");
            }
        }
        
        // Check all client sockets for data
        for (i = 0; i < MAX_CLIENTS; i++) {
            int clientSocket = client_sockets[i];
            if (clientSocket == -1) continue;
            
            if (FD_ISSET(clientSocket, &read_set)) {
                handle_client_request(clientSocket);
                
                // Remove from master set and close
                FD_CLR(clientSocket, &master_set);
                close(clientSocket);
                client_sockets[i] = -1;
            }
        }
    }
    
    // Cleanup
    close(listenfd);
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != -1) {
            close(client_sockets[i]);
        }
    }
    
    return 0;
}

void handle_client_request(int clientSocket) {
    char buffer[BUFFER_SIZE];
    char response[1024];

    char *responseHeaders = "HTTP/1.1 200\r\nContent-type:text/plain\r\nContent-Length: %ld\r\nConnection: Close\r\n\r\n%s";
    memset(response, 0, 1024);

    // receive request
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytes_received <= 0) {
        if (bytes_received == 0) {
            printf("Connection closed by client on socket %d\n", clientSocket);
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("Error receiving data");
        }
        return;
    }
    
    buffer[bytes_received] = '\0'; // Null terminate the received data

	// Get Path
	char* req_path = strtok(buffer, " ");
	if (req_path == NULL) {
        printf("Error parsing request on socket %d\n", clientSocket);
        return;
    }
	req_path = strtok(NULL, " ");
    if (req_path == NULL) {
        printf("Error parsing request path on socket %d\n", clientSocket);
        return;
    }

	if (strcmp(req_path, "/ping") == 0) {
		char *body = "Pong"; 
		printf("Sending response for /ping on socket %d\n", clientSocket);
		sprintf(response, responseHeaders, strlen(body), body);
		send(clientSocket, response, strlen(response), 0);
	} else {
        // HTTP 404 Not Found
        char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\nConnection: Close\r\n\r\nNot Found";
        printf("Sending 404 for path %s on socket %d\n", req_path, clientSocket);
        send(clientSocket, not_found, strlen(not_found), 0);
    }
}

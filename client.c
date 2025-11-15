#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    int sockfd = 0;
    struct sockaddr_in serv_addr;
    char sendBuff[1024];
    char recvBuff[1024];
    
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(recvBuff, '0', sizeof(recvBuff));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    
    // Convert IP address from string to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sockfd);
        exit(1);
    }

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        exit(1);
    }
    
    printf("Connected to server at localhost:8080\n");
    
    // Build HTTP POST request to /ping endpoint with "Ping" in the body
    const char *body = "Ping";
    snprintf(sendBuff, sizeof(sendBuff), 
             "POST /ping HTTP/1.1\r\n"
             "Host: localhost:8080\r\n"
             "User-Agent: C-Client/1.0\r\n"
             "Content-Type: text/plain\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             strlen(body), body);
    
    printf("Sending request:\n%s", sendBuff);
    
    // Send request
    if (send(sockfd, sendBuff, strlen(sendBuff), 0) < 0) {
        perror("Send failed");
        close(sockfd);
        exit(1);
    }
    
    printf("\nWaiting for response...\n\n");
    
    // Receive response
    int bytes_received = 0;
    int total_bytes = 0;
    
    while ((bytes_received = recv(sockfd, recvBuff + total_bytes, 
                                   sizeof(recvBuff) - total_bytes - 1, 0)) > 0) {
        total_bytes += bytes_received;
        if (total_bytes >= sizeof(recvBuff) - 1) {
            break; // Buffer full
        }
    }
    
    if (bytes_received < 0) {
        perror("Receive failed");
        close(sockfd);
        exit(1);
    }
    
    recvBuff[total_bytes] = '\0'; // Null terminate
    
    printf("Response received:\n%s\n", recvBuff);
    
    
    close(sockfd);
    return 0;
}


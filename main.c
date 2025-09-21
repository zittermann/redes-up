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
#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

void *handle_client(void *params);

int main(int argc, char *argv[]) {


    int listenfd = 0, clientSocket = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

    char sendBuff[1025];
    

    /* creates an UN-named socket inside the kernel and returns
     * an integer known as socket descriptor
     * This function takes domain/family as its first argument.
     * For Internet family of IPv4 addresses we use AF_INET
     */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    /* Do not wait to listener socket to be released
     */
    int yes=1;
    if (setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
        perror("setsockopt failed");
        close(listenfd);
        exit(1);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //sudo apt install net-tools
    serv_addr.sin_port = htons(8080);
    

    /* The call to the function "bind()" assigns the details specified
     * in the structure serv_addr' to the socket created in the step above
     */
    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Bind failed");
        close(listenfd);
        exit(1);
    }

    /* The call to the function "listen()" with second argument as 10 specifies
     * maximum number of client connections that server will queue for this listening
     * socket.
     */
    if (listen(listenfd, 10) < 0) {
        perror("Listen failed");
        close(listenfd);
        exit(1);
    }
    
    printf("listening port 8080...\n");
    
    while(1) {
        /* In the call to accept(), the server is put to sleep and when for an incoming
        * client request, the three way TCP handshake* is complete, the function accept()
        * wakes up and returns the socket descriptor representing the client socket.
        */
        socklen_t size = sizeof(client_addr);
        
        clientSocket =  accept(listenfd, (struct sockaddr*) &client_addr, &size);

        if(clientSocket < 0) {
            perror("Accept failed");
            close(listenfd);
            exit(1);
        }
        
        // send & recv cliente (clientSocket)
        pthread_t clienteThread;
        pthread_attr_t threadAttrs;

        pthread_attr_init(&threadAttrs);
        pthread_attr_setdetachstate(&threadAttrs, PTHREAD_CREATE_DETACHED);
        pthread_attr_setschedpolicy(&threadAttrs, SCHED_FIFO);  

        //pthread_create(&clienteThread, &threadAttrs, atenderCliente, &clientSocket);

        handle_client(&clientSocket);
    }

}

void *handle_client(void *params) {
    
    /* As soon as server gets a request from client, it prepares the date and time and
     * writes on the client socket through the descriptor returned by accept()
     */
    int clientSocket = *((int *) params);
    char buffer[2048];
    char response[1024];

    // build response
    char *responseHeaders = "HTTP/1.1 200\r\nContent-type:text/plain\r\nContent-Length: %ld\r\nConnection: Close\r\n\r\n%s";
    memset(response, 0, 1024);

    // receive request
    printf("Connection established on socket %d\n", clientSocket);
    memset(buffer, 0, 2048);
    int bytes_received = recv(clientSocket, buffer, 2047, 0);
    
    if (bytes_received <= 0) {
        printf("Error receiving data or connection closed\n");
        close(clientSocket);
        return NULL;
    }
    buffer[bytes_received] = '\0'; // Null terminate the received data

	// Get Path
	char* req_path = strtok(buffer, " ");
	req_path = strtok(NULL, " ");


	if (strcmp(req_path, "/ping") == 0) {
		char *body = "Pong"; 
		printf("\nSending response for /ping\n");
		sprintf(response, responseHeaders, strlen(body), body);
		send(clientSocket, response, strlen(response), 0);
	} else {
        // Send 404 for unknown paths
        char *not_found = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\nConnection: Close\r\n\r\nNot Found";
        send(clientSocket, not_found, strlen(not_found), 0);
    }

    close(clientSocket);
    
    return 0;
    
}

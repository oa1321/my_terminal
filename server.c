/*
    TCP/IP-server task 4
*/

#include<stdio.h>
// Linux and other UNIXes
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define SERVER_PORT 5060  //The port that the server listens
int open_server_socket(){
    signal(SIGPIPE, SIG_IGN); // on linux to prevent crash on closing socket
      
    // Open the listening (server) socket
    int listeningSocket = -1;  
	 
    if((listeningSocket = socket(AF_INET , SOCK_STREAM , 0 )) == -1){
        printf("Could not create listening socket : %d" ,errno);
    }

	// Reuse the address if the server socket on was closed
	// and remains for 45 seconds in TIME-WAIT state till the final removal.
	//
    int enableReuse = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse,sizeof(int)) < 0){
         printf("setsockopt() failed with error code : %d" , errno);
    }

    // "sockaddr_in" is the "derived" from sockaddr structure
    // used for IPv4 communication. For IPv6, use sockaddr_in6
    //
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr =  INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);  //network order
      
    // Bind the socket to the port with any IP at this port
    if (bind(listeningSocket, (struct sockaddr *)&serverAddress , sizeof(serverAddress)) == -1){
        printf("Bind failed with error code : %d" , errno); 
        close(listeningSocket);
        return -1;
    }
      
    printf("Bind() success\n");
  
    // Make the socket listening; actually mother of all client sockets.
    if (listen(listeningSocket, 500) == -1){//500 is a Maximum size of queue connection requests//number of concurrent connections 
	printf("listen() failed with error code : %d", errno);
        close(listeningSocket);
        return -1;
    }
      
    //Accept and incoming connection
    printf("Waiting for incoming TCP-connections...\n");
      
    struct sockaddr_in clientAddress;  //
    socklen_t clientAddressLen = sizeof(clientAddress);
    
    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddressLen = sizeof(clientAddress);
    int clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
    if (clientSocket == -1){
        printf("listen failed with error code : %d \n", errno);
        close(listeningSocket);
        close(clientSocket);
        return -1;
    }
    printf("Accept() success\n");
    char get_size[4096] = "o";
    while(strcmp(get_size,"") != 0){
        strcpy(get_size,"");
        int messageLen = 4095;
        int bytesSent = recv(clientSocket, get_size, messageLen, 0);
        if (bytesSent == -1){
            printf("recv failed with error code : %d \n", errno);
            close(listeningSocket);
            close(clientSocket);
            return -1;
        }
        get_size[bytesSent] = '\0';
        //prints the data recived
        printf("%s\n", get_size);
    }
    close(listeningSocket);
    return 0;
}
int main()
{
    while(1){
        open_server_socket();
    }
    return 0;
}

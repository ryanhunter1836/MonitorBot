/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
using namespace std;

int main()
{
     //socket file descriptors and port number
    int sockFd, newsockFd, portNumber = 80;
    int n;
    //size of client address
    socklen_t clilen;
    //input buffer size
    char buffer[256];
    //server and client addresses
    struct sockaddr_in serv_addr, cli_addr;
    //declare socket
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    //if the socket fails to open
    if (sockFd < 0) 
        cout<< "ERROR opening socket" << endl;;
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portNumber);
    if (bind(sockFd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    cout << "ERROR on binding" << endl;
    listen(sockFd, 5);
    clilen = sizeof(cli_addr);
    newsockFd = accept(sockFd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockFd < 0) 
        cout << "ERROR on accept" << endl;
    bzero(buffer,256);
    n = read(newsockFd, buffer, 255);
    
    if (n < 0) 
        cout << "ERROR reading from socket" << endl;
        
    cout << "Here is the message: " << buffer << endl;
    n = write(newsockFd,"I got your message",18);
    if (n < 0) 
        cout << "ERROR writing to socket" << endl;
        
    close(newsockFd);
    close(sockFd);
     return 0; 
}

//c++
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <vector>
#include <chrono>

//C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

int main()
{
//socket file descriptors and port number
	int* drivingCommand = new int;
	int* mode = new int;
	bool* lowBattery = new bool;
	float* ultrasonicData = new float;
	*lowBattery = false;
	*ultrasonicData = 10.111;
    cout << "Started thread" << endl;
    int sockFd;
    int newsockFd;
    int portNumber = 8420;
    int response;
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
    {
        cout<< "ERROR opening socket" << endl;
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    //set ip address to local host
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portNumber);
    if (bind(sockFd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        cout << "ERROR on binding" << endl;
    }
    //listen for incoming connections
    listen(sockFd, 5);
    clilen = sizeof(cli_addr);
    //accept the incoming connection
    newsockFd = accept(sockFd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockFd < 0) 
    {
        cout << "ERROR on accept" << endl;
    }
    //clear out the buffer
    bzero(buffer, 256);
    //read from the socket; blocking call
    response = read(newsockFd, buffer, 255);
    //low battery, ultrasonic data
        
    while(true)
    {
        //clear out the buffer
        bzero(buffer, 256);
        //read from the socket; blocking call
        response = read(newsockFd, buffer, 255);
        if (response < 0) 
        {
            cout << "ERROR reading from socket" << endl;
            break;
        }
        //parse the input data
        string respString = buffer;
        //cout << respString << endl;
        //driving command, ultrasonic data, mode, low battery
        try
        {
	        *drivingCommand = stoi(respString.substr(0, respString.find('.')));
			respString = respString.substr(0, respString.find('.') + 1);
			*mode = stoi(respString.substr(0, respString.find('.')));
			respString = ""; 
        }
        catch(...)
        {
        	cout << respString + '.' << endl;
        }
		
		cout << respString << endl;
		//cout << *drivingCommand << "." << endl;
		//cout << *mode << "." << endl;
		//low battery, ultrasonic data
		string output = to_string(*lowBattery) + to_string(*ultrasonicData);
		bzero(buffer, 256);
        response = write(newsockFd, output.c_str(), strlen(output.c_str()));
        if (response < 0) 
            cout << "ERROR writing to socket" << endl;
    }
        
    close(newsockFd);
    close(sockFd);
    //if socket fails, close the whole program
    return 0;
}
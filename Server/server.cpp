#pragma comment(lib, "Ws2_32.lib")
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

#define DEFAULT_PORT 13337

fd_set master;

// Receive message and send to other client socks
int recvAndSendToClients (SOCKET s) {
	char client_name[50];
	recv(s,client_name,50,0);
	
	char welcome_message[80] = "Server: Welcome to chat server ";
	strcat(welcome_message, client_name);
	for(int i=0;i<master.fd_count;i++) {
		send(master.fd_array[i], welcome_message, strlen(welcome_message), 0);
	}
	cout<<'\r';
	cout<<client_name<<" connected"<<endl;
	cout<<"Server: ";
	while (true ) {
		char recvbuf[4096];
		ZeroMemory(recvbuf,4096);
		int result = recv (s, recvbuf, 4096, 0 );
		if ( result > 0) {
			cout<<'\r';
			cout<< recvbuf<<endl;
			cout<<"Server: ";
			for(int i=0; i<master.fd_count; i++) {
				if(master.fd_array[i] != s) {
					send(master.fd_array[i],recvbuf,strlen(recvbuf),0);
				}
			}
		}
		else if ( result == 0 ) {
			FD_CLR(s,&master);
			cout<<'\r';
			cout<<"A client disconnected"<<endl;
			cout<<"Server: ";
			// Annouce to others
			for(int i=0;i<master.fd_count;i++) {
				char disconnected_message[]="Server : A client disconnected";
				send(master.fd_array[i], disconnected_message,strlen(disconnected_message),0);
			}
			return 1;
		}
		else {
			FD_CLR(s,&master);
			cout<<'\r';
			cout<<"a client receive failed: "<<WSAGetLastError()<<endl;
			cout<<"Server: ";
			// Annouce to others
			for(int i=0; i<master.fd_count; i++) {
				char disconnected_message[]="Server : A client disconnected";
				send(master.fd_array[i], disconnected_message,strlen(disconnected_message),0);
			}
			return 2;
		}
	}
}
// Accept function 
int accept0(SOCKET listenSOCK){
   	while(true) {
		SOCKET ClientSocket = accept(listenSOCK, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			cout<<'\r';
			cout<<"accept error: "<<WSAGetLastError()<<endl;
			cout<<"Server: ";
			closesocket(ClientSocket);
		}
		else {
			thread handle(recvAndSendToClients, ClientSocket);
			handle.detach();
			FD_SET(ClientSocket,&master);
			
		}
	}
}
// Send message to all clients
int sendAnouce(){
	string buf;
	do {
		cout<<"Server: "; 
		getline(cin,buf);
		buf="Server: "+buf;
		for(int i=0;i<master.fd_count;i++){
			int result = send(master.fd_array[i], buf.c_str(), strlen(buf.c_str()),0);
			if ( result == SOCKET_ERROR ) {
				cout<<'\r';
				cout<<"send to client "<<i<<" failed: "<<WSAGetLastError()<<endl;
				cout<<"Server: ";
			}
		}
	} while (true);
	return 0;
}


int main() {
	int result;
	/* 
		Initialize 
	*/
	// Initialize Winsock
	WSADATA wsadata;
	result = WSAStartup(MAKEWORD(2,2), &wsadata);
	if ( result != 0 ) {
		cout<<"WSAStartup error: "<<result<<endl;
		return 1;
	}
	else cout<<"WSAStartup OK"<<endl;

	// Create listening SOCK
	SOCKET listenSOCK = socket(AF_INET, SOCK_STREAM , IPPROTO_TCP);
	if ( listenSOCK == INVALID_SOCKET ) {
		cout<<"listenSOCK error: "<<WSAGetLastError()<<endl;
		WSACleanup();
		return 1;
	}
	else cout<<"listenSOCK OK"<<endl;
	
	// Bind addr and port to socket	
	struct addrinfo *res = NULL;
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	int port = -1;
	cout<<"Listen on port (default: 13337): ";
	cin>>port;
	if (port <= 0 || port > 65535){
		cout<<"Invalid, listen on 13337"<<endl;
		port = DEFAULT_PORT;
		result = getaddrinfo(NULL, to_string(DEFAULT_PORT).c_str(), &hints, &res);
	}
	else{
		result = getaddrinfo(NULL, to_string(port).c_str(), &hints, &res);
	}
	if ( result != 0 ) {
		cout<<"getaddrinfo error: "<<WSAGetLastError()<<endl;
		closesocket(listenSOCK);
		WSACleanup();
		return 1;
	}
	else cout<<"getaddrinfo OK"<<endl;
	result = ::bind( listenSOCK , res->ai_addr , (int)res->ai_addrlen);
	if ( result ==SOCKET_ERROR ) {
		cout<<"bind error: "<<WSAGetLastError()<<endl;
		closesocket(listenSOCK);
		WSACleanup();
		return 1;
	}
	else cout<<"bind OK"<<endl;
	freeaddrinfo(res);

	// Listening
	result = listen( listenSOCK , SOMAXCONN);
	if ( result == SOCKET_ERROR ) {
		cout<<"listen error: "<<WSAGetLastError()<<endl;
		closesocket(listenSOCK);
		WSACleanup();
		return 1;
	}
	else cout<<"listen OK"<<endl;
	cout<<"Listening on port "<<port<<endl;
	cout<<endl<<endl;
	/* 
		Done
	*/

	// Accept thread
	thread accept_thread(accept0, listenSOCK);
	accept_thread.detach();
	
	// Send anouce thread
	thread send_thread(sendAnouce);
	send_thread.join();
	
	// Cleanup
	WSACleanup();
	return 0;
}

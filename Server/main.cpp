#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
using namespace std;

#define DEFAULT_PORT "123"
//-----------------------------------------------------------
	fd_set master;
//	FD_ZERO(master); 
//	FD_SET( listenSOCK , &master );
//	while (true) {
//		fd_set copy = master ;
//		int socketCount = select( 0 , &copy , NULL , NULL , NULL );
//		for (int i=0;i<socketCount;i++) {
//			SOCKET sock = copy.fd_array[i];
//			if (sock == listenSOCK ) {
//				// Accept connection
//				SOCKET client = accept(listenSOCK , NULL, NULL );
//				// Add new connection connected
//				FD_SET(client , &master);
//				// Send a welcome message
//				char welmes[]="Welcome to chat server!!!";
//				send(client,welmes,strlen(welmes),0);
//			}
//			else {
//				char recvbuf[4096];
//				ZeroMemory(recvbuf,4096);
//				result = recv( sock , recvbuf , 4096 , 0 );
//				if (result <= 0 ) {
//					cout<<"recv error: "<<WSAGetLastError()<<endl;
//					closesocket(sock);
//					FD_CLR(sock , &master);
//				}
//				else {
//					// Send message to other clients
//					for (int i=0;i<master.fd_count;i++) {
//						SOCKET outSock = master.fd_array[i];
//						if (outSock != listenSOCK && outSock != sock ) {
//							send(outSock,recvbuf,result,0);
//						}
//					}
//				}
//			}
//		}
//	}


//------------------------------------------------------------------
// receive message and send to other client socks
int rvsd (SOCKET s) 
{
	char client_name[50];
	recv(s,client_name,50,0);
	
	char welmes[80]="Server: Welcome to chat server ";
	strcat(welmes,client_name);
	// annouce others ( send to all clients)
	for(int i=0;i<master.fd_count;i++) {
		send(master.fd_array[i],welmes,strlen(welmes),0);
	}
	cout<<'\r';
	cout<<client_name<<" connected"<<endl;
	cout<<"Server: ";
	while (true ) {
		char recvbuf[4096];
		ZeroMemory(recvbuf,4096);
		int result = recv ( s , recvbuf , 4096, 0 );
		if ( result > 0) {
			cout<<'\r';
			cout<< recvbuf<<endl;
			cout<<"Server: ";
			for(int i=0;i<master.fd_count;i++) {
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
			// annouce others
			for(int i=0;i<master.fd_count;i++) {
				char dis_annouce[]="Server : A client disconnected";
				send(master.fd_array[i],dis_annouce,strlen(dis_annouce),0);
			}
			return 1;
		}
		else {
			FD_CLR(s,&master);
			cout<<'\r';
			cout<<"a client recv failed: "<<WSAGetLastError()<<endl;
			cout<<"Server: ";
			
			// annouce others
			for(int i=0;i<master.fd_count;i++) {
				char dis_annouce[]="Server : A client disconnected";
				send(master.fd_array[i],dis_annouce,strlen(dis_annouce),0);
			}
			return 2;
		}
	}
}

//--------------------------------------------------------------
int ac(SOCKET listenSOCK)
{
   // Accept a client socket
   	while(true) {
		SOCKET ClientSocket = accept(listenSOCK, NULL, NULL);
    	if (ClientSocket == INVALID_SOCKET) {
    		cout<<'\r';
        	cout<<"accept error: "<<WSAGetLastError()<<endl;
        	cout<<"Server: ";
			closesocket(ClientSocket);
    	}
		else {
			thread handle(rvsd,ClientSocket);
			handle.detach();
			FD_SET(ClientSocket,&master);
			
		}
	}
}
//-------------------------------------------------------

// send anouce
int sd( )
{
	string buf;
	do {
		cout<<"Server: ";getline(cin,buf);
		buf="Server: "+buf;
		const char *sendbuf = buf.c_str();
		for(int i=0;i<master.fd_count;i++){
			int result=send(master.fd_array[i],sendbuf,strlen(sendbuf),0);
			if ( result == SOCKET_ERROR ) {
				cout<<'\r';
				cout<<"send to client "<<i<<" failed: "<<WSAGetLastError()<<endl;
				cout<<"Server: ";
			}
		}
	} while ( true );
	return 0;
}
//-------------------------------------------------------------










int main() 
{
    int result;
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
//--------------------------------------------------------------------------
//	// Declare variables
//	struct sockaddr_in saServer;
//	hostent* localHost;
//	char* localIP;
//
//
//	// Get the local host information
//	localHost = gethostbyname("");
//	localIP = inet_ntoa (*(struct in_addr *)*localHost->h_addr_list);
//
//	// Set up the sockaddr structure
//	saServer.sin_family = AF_INET;
//	saServer.sin_addr.s_addr = inet_addr(localIP);
//	saServer.sin_port = htons(5150);
//
//	// Bind the listening socket using the
//	// information in the sockaddr structure
//	result = bind( listenSOCK , (SOCKADDR*) &saServer, sizeof(saServer) );
//--------------------------------------------------------------------------------------
	
	struct addrinfo *res = NULL;
    struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &res);
    if ( result != 0 ) {
        cout<<"getaddrinfo error: "<<WSAGetLastError()<<endl;
        closesocket(listenSOCK);
        WSACleanup();
        return 1;
    }
	else cout<<"getaddrinfo OK"<<endl;
	
		
	result = bind( listenSOCK , res->ai_addr , (int)res->ai_addrlen);
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
	cout<<"Listening on port 123"<<endl;
//-------------------------------------------------------------------------	


 

	


	cout<<endl<<endl;
	
	// accept thread
	thread act(ac,listenSOCK);
	act.detach();
	
	// send anouce thread
	thread  sendt (sd);
	sendt.join();
	
    // cleanup
    
    WSACleanup();

    return 0;
}

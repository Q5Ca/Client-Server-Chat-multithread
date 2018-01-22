#undef UNICODE

#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
using namespace std;

#define DEFAULT_PORT "123"

int rv (SOCKET s) 
{
	while (true ) {
		char recvbuf[4096];
		ZeroMemory(recvbuf,4096);
		int result = recv ( s , recvbuf , 4096, 0 );
		if ( result > 0) {
			cout<<'\r';
			cout<< "Client: "<<recvbuf<<endl;
			cout<<"Server: ";
		}
		else if ( result == 0 ) {
			cout<<"Connection closed"<<endl;
			return 1;
		}
		else {
			cout<<"recv failed: "<<WSAGetLastError()<<endl;
			closesocket(s);
			WSACleanup();
			return 2;
		}
	}
}

int sd(SOCKET s )
{
	string buf;
	do {
		cout<<"Server: ";getline(cin,buf);
		if ( buf.length() >0) {
			const char *sendbuf = buf.c_str();
			if ( send( s , sendbuf , strlen(sendbuf) , 0 )  == SOCKET_ERROR ) {
				cout<<"send failed: "<<WSAGetLastError()<<endl;
				closesocket(s);
				WSACleanup();
				return 1;
			}
		}
	} while ( buf.length() >0 );
	return 0;
}

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
	
//	fd_set master;
//	FD_ZERO(&master);
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

    // Accept a client socket
    SOCKET ClientSocket1 = accept(listenSOCK, NULL, NULL);
    if (ClientSocket1 == INVALID_SOCKET) {
        cout<<"accept error: "<<WSAGetLastError()<<endl;
        closesocket(listenSOCK);
        WSACleanup();
        return 1;
    }
	else cout<<"Accepted client 1"<<endl;
	
//	SOCKET ClientSocket2 = accept(listenSOCK , NULL ,NULL);
//	cout<<"Accepted client 2"<<endl;
	
	
    // No longer need listen socket
    closesocket(listenSOCK);
    
//    char buf1[4096];
//    char buf2[4096];
//    while (true) {
//    	recv( ClientSocket1 , buf1,4096,0);
//    	cout<<buf1<<endl;
//    	recv(ClientSocket2,buf2,4096,0);
//    	
//    	cout<<buf2<<endl;
//    }
    
//
//    } while (iResult > 0);

	cout<<endl<<endl;
	
	// recv thread
	thread recvt(rv,ClientSocket1);
	recvt.detach();
	// send thread
	thread  sendt (sd,ClientSocket1);
	sendt.join();
	
    // cleanup
    closesocket(ClientSocket1);
//    closesocket(ClientSocket2);
    WSACleanup();

    return 0;
}

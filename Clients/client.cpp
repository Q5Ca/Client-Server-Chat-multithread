#pragma comment(lib, "Ws2_32.lib")
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

string name;

int sd(SOCKET s ){
	string buf;
	do {
		cout<<name<<" : ";getline(cin,buf);
		if ( buf.length() >0) {
			buf=name+" : "+buf;
			const char *sendbuf = buf.c_str();
			if ( send( s , sendbuf , strlen(sendbuf) , 0 )  == SOCKET_ERROR ) {
				cout<<"send failed: "<<WSAGetLastError()<<endl;
				closesocket(s);
				WSACleanup();
				getchar();
				return 1;
			}
		}
	} while ( buf.length() >0 );
	return 0;
}

int rv (SOCKET s) {
	while (true ) {
		char recvbuf[4096];
		ZeroMemory(recvbuf,4096);
		int result = recv ( s , recvbuf , 4096, 0 );
		if ( result > 0) {
			cout<<'\r';
			cout<< recvbuf<<endl;
			cout<<name<<" : ";
		}
		else if ( result == 0 ) {
			cout<<"Connection closed"<<endl;
			return 1;
		}
		else {
			cout<<"recv failed: "<<WSAGetLastError()<<endl;
			closesocket(s);
			WSACleanup();
			getchar();
			return 2;
		}
	}
}

int main(){
	int result;
	
	// Initialize Winsock
	
	WSADATA wsaData;
	result = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (result != 0) {
		cout<<"Initialize Winsock Fail: "<<result<<endl;
		getchar();
		return 1;
	}
	else cout<<"Initialize Winsock OK"<<endl;

	// Resolve the server address and port
	addrinfo hints,							  	//A pointer to an addrinfo structure that provides hints about the type of socket the caller supports.
			*res;                               //A linked list of one or more addrinfo structures that contains response information about the host.
	ZeroMemory( &hints, sizeof(hints) );		//The ai_addrlen, ai_canonname, ai_addr, and ai_next members of the addrinfo structure pointed to by the pHints parameter must be zero or NULL. 
												//Otherwise the GetAddrInfoEx function will fail with WSANO_RECOVERY.
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	string host;cout<<"Host: ";getline(cin,host);
	string port;cout<<"Port: ";getline(cin,port);
	result = getaddrinfo( host.c_str() , port.c_str() , &hints, &res);
	if ( result != 0) {
		cout<<"getaddrinfo failed: "<<result<<endl;
		getchar();
		return 1;
	}
	else cout<<"getaddrinfo OK"<<endl;


	// Create connecting Socket
	SOCKET connectSOCK = socket( res->ai_family , res->ai_socktype , res->ai_protocol );
	if ( connectSOCK == INVALID_SOCKET ) {
		cout<<"Create connect sock failed: "<<WSAGetLastError()<<endl;
		WSACleanup();
		getchar();
		return 1;
	}
	else cout<<"Create connect sock OK"<<endl;
	
	// Connect 
	result = connect( connectSOCK , res->ai_addr , (int) res->ai_addrlen);
	if ( result != 0) {
		cout<<"connect failed: "<<WSAGetLastError()<<endl;
		getchar();
		closesocket(connectSOCK );
		WSACleanup();
		return 1;
	}
	else cout<<"connect OK"<<endl;
	
	freeaddrinfo(res);	
	cout<<endl<<endl;
	// Connect complete

	// Ask name?
	cout<<"What name do you want? ";getline(cin,name);
	send( connectSOCK , name.c_str() , strlen(name.c_str()) , 0 );
	

	// Send thread
	thread sendt (sd,connectSOCK);
	sendt.detach();
	
	// Recv thread
	thread recvt (rv,connectSOCK);
	recvt.join();


	// Cleanup
	closesocket(connectSOCK);
	WSACleanup();
	
//	cout<<"Test PASSED"<<endl;
//	getchar();
	
	return 0;
}


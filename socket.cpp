#include "socket.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
namespace libls{
	Socket::~Socket(){
		close(socket_fd);
	}

	TcpSocket::TcpSocket(const char * host, uint16_t port){
		socket_fd = socket(PF_INET, SOCK_STREAM, 0);
		int i = 1;
		setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int));
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		inet_pton(AF_INET, host,  &(addr.sin_addr));
		bind(socket_fd, (sockaddr*)&addr, sizeof(addr));
	}

	TcpSocket::~TcpSocket(){
		shutdown(socket_fd, SHUT_RDWR);
	}
	
	 int TcpSocket::accept(){
		 return ::accept(socket_fd, NULL, NULL);
	}

	void TcpSocket::listen(size_t maxconn){
		::listen(socket_fd, maxconn);
	}

	ssize_t TcpSocket::recv(std::string &buf){
		return ::recv(socket_fd, &(buf[0]), buf.capacity(), MSG_NOSIGNAL);
	}

	void TcpSocket::send(std::string buf){
		::send(socket_fd, buf.c_str(), buf.size(), MSG_NOSIGNAL);
	}
}

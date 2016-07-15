#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <vector>
#include <cstdint>
#include <string>
#include <iostream>
namespace libls {
	/**
	 * Socket base class
	 */

	class Socket {
		template <class SockType> friend class Epoll;
		public:
		Socket(){};
		Socket(const Socket&) = delete;

		//TO-DO: move constructor
		void setNonBlockState(bool state){
			nonBlock = state;
			int flags;
			if (-1 == (flags = fcntl(socket_fd, F_GETFL, 0)))
				flags = 0;
			flags = state ? flags | O_NONBLOCK : flags ^ O_NONBLOCK ;
			fcntl(socket_fd, F_SETFL, flags);
		};

		int getSocketFd()	const{ return socket_fd; }; 
		bool isNonBlock()	const{ return nonBlock; };
		int getFlags()		const{ return flags; };
		
		virtual ~Socket();
		protected:
		bool nonBlock = false;
		int socket_fd;
		int flags = 0;
	};

	/**
	 * TCP socket
	 */
	
	class TcpSocket : public Socket {
		public:
			TcpSocket(int fd){socket_fd = fd;};
			TcpSocket(const char * addr, uint16_t port);
			void listen(size_t max_conn = 128);
			int accept();
			ssize_t recv(std::string &buf);
			void send(std::string buf);
			~TcpSocket();
		
	};
	/*
	class DGramSocket : public Socket {
		public:
		
		private:
		
	};

	class UnixSocket : public Socket {
		public:

		private:
	}
	*/

}

#endif

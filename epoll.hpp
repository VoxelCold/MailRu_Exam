#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "socket.hpp"
#include <sys/epoll.h>
namespace libls {
	template <class SockType> 
		class Epoll {
		//static_assert(std::is_base_of<Socket, SockType>::value);
		public:
		Epoll();
		Epoll(const Epoll&) = delete;

		std::vector<SockType *> wait(size_t timeout = -1);	

		void addSock(SockType &sock, int ev);
		//void addSock(SockType sock, int ev, void * ptr);

		void deleteSock(SockType &sock);
		
		~Epoll();
		private:
		size_t max_events = 128;
		int epoll_fd;
	};

	template <class SockType>
		Epoll<SockType>::Epoll(){
			epoll_fd = epoll_create1(0);	
		}

	template <class SockType> 
		std::vector<SockType*> Epoll<SockType>::wait(size_t timeout){
			std::vector<SockType*> v;
			epoll_event evnts[max_events];
			int nEvnts = epoll_wait(epoll_fd, evnts, max_events, timeout);
			for(int i = 0; i < nEvnts; i++){
				static_cast<SockType *>(evnts[i].data.ptr)->flags = evnts[i].events;
				v.push_back(static_cast<SockType *>(evnts[i].data.ptr));
			}
			return v;
		}

	template <class SockType> 
		void Epoll<SockType>::addSock(SockType &sock, int ev){
			epoll_event evnt;
			evnt.data.ptr = &sock;
			evnt.events = ev;
			epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock.getSocketFd(), &evnt);
		}

	template <class SockType> 
		void  Epoll<SockType>::deleteSock(SockType& sock){
			epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock.getSocketFd(), NULL);
		}

	template <class SockType> 
		Epoll<SockType>::~Epoll(){

		}
}

#endif

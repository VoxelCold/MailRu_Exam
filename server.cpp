#include "socket.hpp"
#include "epoll.hpp"
#include <iostream>
#include <thread>
#include <queue>
#include <atomic>
#include <unistd.h>
#include <unordered_set>
#include <fstream>
#include <sstream>
using namespace libls;

class SpinQueue {
	public:
		SpinQueue(){mLock.clear(std::memory_order_release);}
		void push(TcpSocket * sock){
			while(mLock.test_and_set(std::memory_order_acquire));
			if(mSocks.find(sock) == mSocks.end()){
				mQueue.push(sock);
				mSocks.insert(sock);
			}
			mLock.clear(std::memory_order_release);
			//cond_var
		}
		TcpSocket * waitAndPop(){
			return pop(); //cond_var	
		}
		TcpSocket * pop(){
			while(mLock.test_and_set(std::memory_order_acquire));
			if(mQueue.empty()){
				mLock.clear(std::memory_order_release);
				return nullptr;
			}
			TcpSocket * t = mQueue.front();
			mQueue.pop();
			mLock.clear(std::memory_order_release);
			return t;
		}

		void * del(TcpSocket * sock){
			while(mLock.test_and_set(std::memory_order_acquire));
			mSocks.erase(sock);
			mLock.clear(std::memory_order_release);
		}
	private:
		std::atomic_flag mLock;
		std::queue<TcpSocket *> mQueue;
		std::unordered_set<TcpSocket *> mSocks;
		//std::condition_variable mCondVar;
};

void process(SpinQueue & sockQueue, Epoll<TcpSocket> & http){
	while(true){
		TcpSocket * t = sockQueue.waitAndPop();
		if(t != nullptr){
			std::string buf(1024,0);
			ssize_t recvsz = t->recv(buf);
			if(recvsz == 0){
				http.deleteSock(*t);
				delete(t);
			}
			sockQueue.del(t);
			if(recvsz < 1) continue;
			buf.resize(recvsz);
			buf.shrink_to_fit();
			std::cout << "[" << std::this_thread::get_id() << "]: " << buf << std::endl;

			std::size_t pos = buf.find("?");
			if(pos == std::string::npos){
				pos = buf.find(" ", 4);
			}
			
			std::ifstream file(buf.substr(4, pos-4));
			std::stringstream ss;
			if(file.is_open()){
				file.seekg(0, std::ios::end);
				ss << "HTTP/1.0 200 OK";
				ss << "\r\n";
				ss << "Content-length: ";
				ss << file.tellg();
				ss << "\r\n";
				ss << "Content-Type: text/html";
				ss << "\r\n\r\n";
				file.seekg(0, std::ios::beg);
				ss << file.rdbuf();
				file.close();
			}else{
				ss << "HTTP/1.0 404 NOT FOUND";
				ss << "\r\n";
				ss << "Content-length: ";
				ss << 0;
				ss << "\r\n";
				ss << "Content-Type: text/html";
				ss << "\r\n\r\n";
			}
			t->send(ss.str());
		}
	}
};

//int nm = 0;
int main(int argc, char ** argv){
	int result = 0;
	std::string host, path;
	uint16_t port;
	while((result = getopt(argc,argv,"h:p:d:")) != -1){
		switch(result){
			case 'h':
				std::cout << "host: " << optarg << std::endl;
				host = optarg;
				break;
			case 'p':
				std::cout << "port: " << optarg << std::endl;
				port = std::stoul(optarg);
				break;	
			case 'd':
				std::cout << "path: " << optarg << std::endl;
				path = optarg;
				break;
		};
	};
	// Daemonising
	pid_t pid = fork();
	if(pid != 0) exit(0);
	
	//close(STDIN_FILENO);
	//close(STDOUT_FILENO);
	//close(STDERR_FILENO);
	
	chdir(path.c_str());
	Epoll<TcpSocket> http;
	TcpSocket master(host.c_str(), port);
	master.setNonBlockState(true);
	master.listen();
	http.addSock(master, EPOLLIN | EPOLLRDHUP);
	SpinQueue sockQueue;	
	for(int i = 0; i < 4; i++){
		std::thread t(process, std::ref(sockQueue), std::ref(http));
		t.detach();
	}
	while(true){
		auto sockEvents = http.wait();

		for(auto iter = sockEvents.cbegin(); iter != sockEvents.cend(); iter++){
			if(*iter == &master){
				TcpSocket * slave = new TcpSocket((*iter)->accept());
				slave->setNonBlockState(true);
				http.addSock(*slave, EPOLLIN);
			}else{
				if(((*iter)->getFlags() & EPOLLRDHUP) == 0){
					sockQueue.push(*iter);
					//nm++;
					//we may wait here if we have coroutines!
					//std::cout << "Pushed: " << nm << std::endl;
			
				}else{
					http.deleteSock(**iter);
					delete *iter;
				}
			}
		}
		
	}


	return 0;
}

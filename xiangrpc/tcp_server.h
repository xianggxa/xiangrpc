#pragma once


#include <memory>
#include<sys/socket.h>
#include<netinet/in.h>

#include"coroutines.h"
#include"reactor.h"
#include"coroutine_hook.h"
#include"io_thread.h"

#define THREAD_POOL_SIZE 2

namespace xiangrpc {
	class TcpAcceptor {
	public:
		using ptr = std::shared_ptr<TcpAcceptor>;
		TcpAcceptor(struct sockaddr_in m_addr) :m_server_addr_(m_addr) {};
		void init();							//创建socket并绑定端口和监听
		int toAccept();
	private:
		struct sockaddr_in m_server_addr_;

		int m_fd_;
	};

	class TcpServer {
	public:
		using ptr = std::shared_ptr<TcpServer>;
		TcpServer();
		~TcpServer();

		void start();

	private:

		void mainAcceptFunc();				//主循环，负责不断接受新连接

	private:
		struct sockaddr_in m_addr_;

		bool m_is_stop_accept_{ false };

		Reactor* m_main_reactor_{ nullptr };
		IOThreadPool::ptr m_io_pool_;
		Coroutine* m_accept_coro_;

		TcpAcceptor::ptr m_acceptor_;


	};
}
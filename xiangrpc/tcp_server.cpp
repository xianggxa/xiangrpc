#include "tcp_server.h"

namespace xiangrpc {
	void TcpAcceptor::init()
	{
		m_fd_ = socket(m_server_addr_.sin_family, SOCK_STREAM, 0);
		if (m_fd_ < 0) {
			LOG_ERROR("server socket create failed, error = %s", strerror(errno));
		}

		LOG_DEBUG("server socket create succed, fd=%d", m_fd_);

		int val = 1;
		if (setsockopt(m_fd_, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0) {		//设置地址复用
			LOG_ERROR("set REUSEADDR error");
		}

		if (-1 == bind(m_fd_, (struct sockaddr*)&m_server_addr_, sizeof m_server_addr_)) {
			LOG_ERROR("server bind filed, error = %s",strerror(errno));
			
		};
		if (-1 == listen(m_fd_, 128)) {
			LOG_ERROR("server listen failed, error = %s",strerror(errno));
		};



	}
	int TcpAcceptor::toAccept()
	{
		socklen_t len = sizeof m_server_addr_;
		int fd = xiangrpc::accept_hook(m_fd_, (struct sockaddr*)&m_server_addr_, &len);
		if (fd == -1) {
			LOG_DEBUG("server accept failed, error = %s", strerror(errno));
		}
		LOG_DEBUG("new client accepted success, port:%d", m_server_addr_.sin_port);

		return fd;

	}
	TcpServer::TcpServer()
	{
		m_io_pool_ = std::make_shared<IOThreadPool>(THREAD_POOL_SIZE);
		
		//decode module place

		m_main_reactor_ = xiangrpc::GetReactorInstance();
		m_main_reactor_->setReactorType(MainReactor);




	}

	void TcpServer::start()
	{
		m_acceptor_.reset(new TcpAcceptor(m_addr_));
		m_acceptor_->init();
		m_accept_coro_ = xiangrpc::AddTask(std::bind(&TcpServer::mainAcceptFunc, this));

		xiangrpc::Resume(m_accept_coro_);												//协程切换至mainacceptfun

		m_main_reactor_->loop();


	}

	void TcpServer::mainAcceptFunc()
	{ 
		while (!m_is_stop_accept_) {
			int fd = m_acceptor_->toAccept();			//异步等待新连接创建

			if (-1 == fd) {
				LOG_ERROR("mainfun accept error");
				xiangrpc::Yield();
				continue;
			}
			IOThread* io_thread = m_io_pool_->getIOThread();

			//connection place

			//io_thread->getReactor()->addCoroutine()


		}
	}


}
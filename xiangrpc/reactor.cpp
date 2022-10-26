#include"reactor.h"

namespace xiangrpc {

	#define EPOLL_TIMEOUT 10000
	Reactor::Reactor()
	{
		m_thread_id_ = gettid();

		if ((m_epoll_fd_ = epoll_create(1) <= 0)) {
			LOG_ERROR("epoll create error, sys error = %s ", strerror(errno));
			exit(0);
		}
		LOG_DEBUG("epoll created epfd = %d, threadfd = %d", m_epoll_fd_, m_thread_id_);

		if ((m_wake_fd_ = eventfd(0, EFD_NONBLOCK)) <= 0) {
			LOG_ERROR("reactor init failed, eventfd create failed, sys error = %s", strerror(errno));
			exit(0);
		}
		LOG_DEBUG("my eventfd = %d", m_wake_fd_);

		addWakeUpFd();

	}

	Reactor::~Reactor()
	{
		LOG_DEBUG("~Reactor %d ", m_epoll_fd_);
		close(m_epoll_fd_);
	}

	void Reactor::addEvent(int fd, epoll_event event)
	{
		if (fd == -1) {
			LOG_ERROR("add error ,fd = -1");
		}
		if (isLoopThread()) {
			addEventInLoopThread(fd, event);
			return;

		}
		{
			std::lock_guard<std::mutex> lock{ lock_ };
			m_pending_add_fds_.push_back(std::make_pair(fd,event));
		}

	}

	void Reactor::delEvent(int fd)
	{
		if (fd == -1) {
			LOG_ERROR("del error ,fd = -1");
		}
		if (isLoopThread()) {
			delEventInLoopThread(fd);
			return;

		}
		{
			std::lock_guard<std::mutex> lock{ lock_ };
			m_pending_del_fds_.push_back(fd);
		}
	}

	void Reactor::addTask(std::function<void()> task)
	{
		m_pending_task_.push_back(task);
	}

	void Reactor::addCoroutine(Coroutine* ptr)
	{
		auto func = [ptr]() {
			xiangrpc::Resume(ptr);

		};
		addTask(func);
	}

	void Reactor::loop()
	{
		if (m_is_looping_) {
			LOG_DEBUG("this reactor is lopping");
			return;
		}
		m_is_looping_ = true;
		const int MAX_EVENTS = 10;
		epoll_event ep_event[MAX_EVENTS + 1];


		while (!m_stop_flag_) {
			std::vector<std::function<void()>> tmp_task;
			{
				std::lock_guard<std::mutex> lock{ lock_ };
				tmp_task.swap(m_pending_task_);

			}
			//通过addcoro添加连接对应的协程到io线程的reactor的task队列并在这里执行
			for (size_t i = 0; i < tmp_task.size(); i++) {						//执行等待的未初始化的协程任务
																				//如进入io协程中的input
				if (tmp_task[i]) {
					tmp_task[i]();
				}
			}

			int num = epoll_wait(m_epoll_fd_, ep_event, MAX_EVENTS, EPOLL_TIMEOUT);

			if (num < 0) {
				LOG_ERROR("epoll_wait error ,errno=%s", strerror(errno));
			}
			else {
				for (int i = 0; i < num; i++) {
					epoll_event now_event = ep_event[i];
					xiangrpc::FdEvent* ptr = (xiangrpc::FdEvent*)now_event.data.ptr;
					if (now_event.data.fd == m_wake_fd_ && (now_event.events & READ)) {
						uint64_t tmp;
						while (1) {
							if ((read(m_wake_fd_, &tmp, 8) == -1) && errno == EAGAIN) {
								break;
							}
						}
					}
					else {
						if(ptr!=)

					}


				}
			}




		}


		m_is_looping_ = false;

	}

	void Reactor::stop()
	{
		if (!m_stop_flag_ && m_is_looping_) {//中断 
			m_stop_flag_ = true;
		}

		
	}

	void Reactor::setReactorType(ReactorType type)
	{
		m_reactor_typr_ = type;
	}

	void Reactor::wakeUp()
	{
		if (!m_is_looping_) {
			return;
		}
		uint64_t tmp = 1;
		if (write(m_wake_fd_, &tmp, 8) != 8) {
			LOG_ERROR("write wakefd error");
		}

	}

	void Reactor::addWakeUpFd()
	{
		epoll_event event;
		event.data.fd = m_wake_fd_;
		event.events = EPOLLIN;
		if ((epoll_ctl(m_epoll_fd_, EPOLL_CTL_ADD, m_wake_fd_, &event)) != 0) {
			LOG_ERROR("epoll_ctl wakefd failed,error = %s", strerror(errno));
		}

		m_fds_.insert(m_wake_fd_);
	}

	bool Reactor::isLoopThread() const
	{
		if (m_thread_id_ == gettid()) {
			return true;
		}
		return false;
	}

	void Reactor::addEventInLoopThread(int fd, epoll_event event)
	{
		int op = EPOLL_CTL_ADD;
		bool is_add = true;
		
		
		if (m_fds_.find(fd)!=m_fds_.end()) {
			is_add = false;
			op = EPOLL_CTL_MOD;
		}

		// epoll_event event;
		// event.data.ptr = fd_event.get();
		// event.events = fd_event->getListenEvents();

		if (epoll_ctl(m_epoll_fd_, op, fd, &event) != 0) {
			LOG_ERROR("epoll ctl error, fd = %d, sys errorinfo = %s", fd, strerror(errno));
			return;
		}
		if (is_add) {
			m_fds_.insert(fd);
		}
		LOG_DEBUG("fd add successed, fd = %d", fd);
		
	}

	void Reactor::delEventInLoopThread(int fd)
	{
		auto it = m_fds_.find(fd);
		if (it == m_fds_.end()) {
			LOG_DEBUG("fd %d not in this epoll", fd);
			return;
		}
		m_fds_.erase(it);
		LOG_DEBUG("fd del successed, fd = %d", fd);

	}


}
#pragma once

#include<memory>
#include<sys/epoll.h>
#include<functional>
#include<shared_mutex>

#include "reactor.h"
#include "coroutines.h"
#include"rlog.h"


namespace xiangrpc {

	enum IOEvent {
		READ = EPOLLIN,
		WRITE = EPOLLOUT,
		ET = EPOLLET,
	};

	class FdEvent : public std::enable_shared_from_this<FdEvent>
	{

	public:
		using ptr = std::shared_ptr<FdEvent>;

		FdEvent(int fd);

		virtual ~FdEvent();

		void setCallBack(IOEvent flag, std::function<void()> cb);

		std::function<void()> getCallBack(IOEvent flag) const;

		void addListenEvents(IOEvent event);

		void delListenEvents(IOEvent event);

		void updateToReactor();									//将监听事件变化更新到reactor中

		Reactor* getReactor() const { return m_reactor_; };

		void setReactor(Reactor* r) { m_reactor_ = r; };

		void setCoroutine(Coroutine* cor) { m_coroutine_ = cor; };

		Coroutine* getCoroutine() { return m_coroutine_; };

		int getFd() { return m_fd_; }

		void unregisterFromReactor();
	private:
		int m_fd_{ -1 };

		std::function<void()> m_read_callback_;
		std::function<void()> m_write_callback_;

		uint32_t m_events_{ 0 };

		Reactor* m_reactor_{ nullptr };

		Coroutine* m_coroutine_{ nullptr };






	};

	class FdEventContainer {								//线程共享静态类，用来获取fd对应的fdevent
	private:
		FdEventContainer();
		~FdEventContainer();
		static FdEventContainer* instance;
		std::shared_mutex lock_;
		std::vector<FdEvent::ptr> fd_event_map_;
	public:
		static FdEventContainer& getFdEventContainer();
		FdEvent::ptr getFdevent(int fd);




	};

	FdEventContainer& FdEventContainer::getFdEventContainer() {
		static FdEventContainer fd_event_container;
		return fd_event_container;
	}
	FdEvent::ptr FdEventContainer::getFdevent(int fd) {
		{
			std::shared_lock<std::shared_mutex> lock(lock_);
			if (fd < fd_event_map_.size()) {
				return fd_event_map_[fd];
			}

		}
		{
			std::unique_lock<std::shared_mutex> lock(lock_);
			int n = (int)fd * 1.5;
			for (int i = fd_event_map_.size(); i < n; i++) {
				fd_event_map_[i] = std::make_shared<FdEvent>(i);
			}
			
		}return fd_event_map_[fd];

	}
	
}

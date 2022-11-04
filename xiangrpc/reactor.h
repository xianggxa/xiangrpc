#pragma once

#include<sys/socket.h>
#include<sys/epoll.h>
#include<sys/eventfd.h>
#include<functional>
#include<unistd.h>
#include <mutex>
#include<set>

#include"coroutines.h"
#include"rlog.h"
#include"fd_event.h"

namespace xiangrpc {

	enum ReactorType
	{
		MainReactor = 1,
		SubReactor = 2

	};

	

	class Reactor {

	public:
		explicit Reactor();
		~Reactor();
		void addEvent(int fd, epoll_event event);

		void delEvent(int fd);

		void addTask(std::function<void()> task);

		void addCoroutine(Coroutine* ptr);

		void loop();

		void stop();

		void setReactorType(ReactorType type);

		void wakeUp();										//利用eventfd唤醒epoll
		void addWakeUpFd();									//添加eventfd的监听

	private:
		int m_epoll_fd_{ -1 };
		bool m_stop_flag_{ false };
		int m_thread_id_{ -1 };
		bool m_is_looping_{ false };
		int m_wake_fd_{ -1 };

		std::mutex lock_;

		ReactorType m_reactor_type_{ SubReactor };

		std::set<int> m_fds_;											//此reactor接管的连接fd

		std::set<std::pair<int, epoll_event>> m_pending_add_fds_;	//等待添加监控的事件队列
																		//存在主reactor将连接监听添加到工作reactor的行为

		std::vector<int> m_pending_del_fds_;							//等待移除监控的事件队列

		std::vector<std::function<void()>> m_pending_task_;				//等待执行的协程任务队列


		bool isLoopThread() const;										//判断当前是否为工作线程

		void addEventInLoopThread(int fd, epoll_event event);
		void delEventInLoopThread(int fd);










	};
	static thread_local Reactor* reactor_instance = nullptr;

	static Reactor* GetReactorInstance() {
		if (reactor_instance == nullptr)
			reactor_instance = new Reactor();
		return reactor_instance;

	}
	 
	class CoroutineTaskQueue {
	public:
		static CoroutineTaskQueue& getCorotineTaskQueue() {
			static CoroutineTaskQueue instance;
			return instance;
		}
		void push(FdEvent* fd);

		FdEvent* pop();
	private:
		std::queue<FdEvent*> m_task_;
		std::mutex m_mutex_;

	};

}
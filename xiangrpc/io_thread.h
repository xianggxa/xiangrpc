#pragma once


#include<memory>
#include <semaphore.h>
#include <cassert>

#include"coroutines.h"
#include"reactor.h"



namespace xiangrpc {
	class IOThread
	{
	public:
		using ptr = std::shared_ptr<IOThread>;
		IOThread();
		~IOThread();

		Reactor* getReactor();
		pthread_t getPthreadId();
		void setThreadIndex(const int index);
		int getThreadIndex();

	private:
		static void* main(void* arg);

	private:
		Reactor* m_reactor_;
		pthread_t m_thread_{ 0 };

		sem_t m_semaphore_;						//�ź���
		int m_index_;							//

	};

	class IOThreadPool {
	public:
		using ptr = std::shared_ptr<IOThreadPool>;

		IOThreadPool(int size);
		~IOThreadPool();

		IOThread* getIOThread();					//���ؾ�������߳�
	private:
		int m_size_{ 0 };
		int m_index_{ 0 };
		std::vector<IOThread::ptr> m_io_threads_;



	};
}

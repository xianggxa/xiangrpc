#include "io_thread.h"

namespace xiangrpc {

	static thread_local IOThread* io_thread = nullptr;

	IOThread::IOThread()
	{
		int rt = sem_init(&m_semaphore_, 0, 0);
		assert(rt == 0);

		pthread_create(&m_thread_, nullptr, &IOThread::main, this);	

		rt = sem_wait(&m_semaphore_);									//在一个新建线程初始化后才开速下一个线程
		assert(rt == 0);

		sem_destroy(&m_semaphore_);

	}

	IOThread::~IOThread()
	{
		m_reactor_->stop();
		pthread_join(m_thread_, nullptr);

		if (m_reactor_ != nullptr) {
			delete m_reactor_;
			m_reactor_ = nullptr;
		}
	}

	Reactor* IOThread::getReactor()
	{
		return m_reactor_;
	}

	pthread_t IOThread::getPthreadId()
	{
		return m_thread_;
	}

	void IOThread::setThreadIndex(const int index)
	{
		m_index_ = index;
	}

	int IOThread::getThreadIndex()
	{
		return m_index_;
	}

	void* IOThread::main(void* arg)
	{
		IOThread* n_thread = static_cast<IOThread*>(arg);

		io_thread = n_thread;									//在线程内部指定

		n_thread->m_reactor_ = GetReactorInstance();			//one thread pre loop
		n_thread->m_reactor_->setReactorType(SubReactor);

		sem_post(&n_thread->m_semaphore_);

		n_thread->m_reactor_->loop();
		

	}

	IOThreadPool::IOThreadPool(int size):m_size_(size)
	{
		m_io_threads_.resize(size);
		for (int i = 0; i < size; i++) {
			m_io_threads_[i] = std::make_shared<IOThread>();
			m_io_threads_[i]->setThreadIndex(i);
		}
	}

	IOThread* IOThreadPool::getIOThread()
	{
		if (m_index_ == m_size_) {
			m_index_ = 0;
		}
		return m_io_threads_[m_index_++].get();
		
	}


}
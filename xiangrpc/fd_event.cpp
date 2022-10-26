#include "fd_event.h"

namespace xiangrpc {
	FdEvent::FdEvent(int fd):
		m_fd_(fd),
		m_reactor_(xiangrpc::GetReactorInstance())
	{

	}
	FdEvent::~FdEvent()
	{
	}
	void FdEvent::setCallBack(IOEvent flag, std::function<void()> cb)
	{
		if (flag == READ) {
			m_read_callback_ = cb;
		}
		else if (flag == WRITE) {
			m_write_callback_ = cb;
		}
		else {
			LOG_ERROR("error callback flag");
		}
	}
	std::function<void()> FdEvent::getCallBack(IOEvent flag) const
	{
		if (flag == READ) {
			return m_read_callback_;
		}
		else if (flag == WRITE) {
			return m_write_callback_;
		}
		return nullptr;
	}
	void FdEvent::addListenEvents(IOEvent event)
	{
		if (m_events_ & event) {
			LOG_DEBUG("event has been added ");
			return;
		}
		m_events_ |= event;
		updateToReactor();
	}
	void FdEvent::delListenEvents(IOEvent event)
	{
		if (m_events_ & event) {
			m_events_ ^= event;
			updateToReactor();
			return;
		}
		LOG_DEBUG("event not exist");
	}
	void FdEvent::updateToReactor()
	{
		epoll_event event;
		event.events = m_events_;
		event.data.ptr = this;

		if (!m_reactor_) {
			m_reactor_ = xiangrpc::GetReactorInstance();
		}
		m_reactor_->addEvent(m_fd_, event);
	}
	void FdEvent::unregisterFromReactor()
	{
		m_reactor_->delEvent(m_fd_);
		m_events_ = 0;
		m_read_callback_ = nullptr;
		m_write_callback_ = nullptr;

	}
}
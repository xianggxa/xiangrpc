#include "tcp_buffer.h"

namespace xiangrpc {
	
	
	TcpBuffer::TcpBuffer(int size) {
		m_buffer_.resize(size);
	}
	
	TcpBuffer::~TcpBuffer() {

	}

	int TcpBuffer::readAble() {

		return m_write_index_ - m_read_index_;
	}

	int TcpBuffer::writeAble() {

		return m_buffer_.size() - m_write_index_;
	}

	int TcpBuffer::readIndex() const {
		return m_read_index_;
	}

	int TcpBuffer::writeIndex() const {
		return m_write_index_;
	}

	void TcpBuffer::writeToBuffer(const char* buf, int size)
	{
		if (size > writeAble()) {
			int n_size = (int)(1.5 * (size + m_write_index_));
			resizeBuffer(n_size);
		}
		memcpy(&m_buffer_[m_read_index_], buf, size);
		m_write_index_ += size;

	}

	void TcpBuffer::readFromBuffer(std::vector<char>& re, int size)
	{
		if (readAble() == 0) {
			LOG_DEBUG("read buffer empty!");
			return;
		}
		int read_size = std::min(readAble(), size);
		std::vector<char> vec(read_size);

		memcpy(&vec[0], &m_buffer_[m_read_index_], read_size);
		re.swap(vec);
		m_read_index_ += read_size;
		adjustBuffer();
	}

	void TcpBuffer::resizeBuffer(int size)
	{
		std::vector<char> vec(size);
		int n = std::min(size, readAble());
		memcpy(&vec[0], &m_buffer_[m_read_index_], n);

		m_buffer_.swap(vec);
		m_read_index_ = 0;
		m_write_index_ = m_read_index_ + n;
	}

	std::vector<char> TcpBuffer::getBufferVector()
	{
		return m_buffer_;
	}

	std::string TcpBuffer::getBufferString()
	{
		std::string n_string(readAble(), 0);
		memcpy(&n_string[0], &m_buffer_[m_read_index_], readAble());
		return n_string;
	}

	void TcpBuffer::resetRead(int index)
	{
		int n = index + m_read_index_;
		if (n > m_write_index_) {
			LOG_ERROR("resetread error");
			return;

		}

		m_read_index_ = n;
		adjustBuffer();
	}

	void TcpBuffer::resetWrite(int index)
	{
		int n = index + m_write_index_;
		if (n > m_buffer_.size()) {
			LOG_ERROR("resetwrite error");
			return;
		}

		m_write_index_ = n;
		adjustBuffer();
	}
	

	void TcpBuffer::adjustBuffer()
	{
		if (m_read_index_ > (int)(m_buffer_.size() / 3)) {

			std::vector<char> n_buffer(m_buffer_.size());
			int len = readAble();

			memcpy(&n_buffer[0], &m_buffer_[0], len);

			m_buffer_.swap(n_buffer);
			m_write_index_ = len;
			m_read_index_ = 0;
			
		}
	}


}
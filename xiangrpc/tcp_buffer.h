#pragma once

#include<memory>
#include<vector>
#include<cstring>

#include"rlog.h"

namespace xiangrpc {


	class TcpBuffer
	{
	public:
		using ptr = std::shared_ptr<TcpBuffer>;
		explicit TcpBuffer(int size);

		~TcpBuffer();

		int readAble();					//可读取的长度
				
		int writeAble();				//可写入的长度

		int readIndex() const;

		int writeIndex() const;

		void writeToBuffer(const char* buf, int size);

		void readFromBuffer(std::vector<char>& re, int size);

		void resizeBuffer(int size);

		std::vector<char> getBufferVector();

		std::string getBufferString();

		void resetRead(int index);				//读出后调整
		  
		void resetWrite(int index);				//写入后调整

		void adjustBuffer();					//读取长度达到1/3时进行重新调整


	private:
		int m_read_index_{ 0 };					//已读取的位置
		int m_write_index_{ 0 };				//写入的位置
		int m_size_{ 0 };
	public:
		std::vector<char> m_buffer_;


	};

}
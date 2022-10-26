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

		int readAble();					//�ɶ�ȡ�ĳ���
				
		int writeAble();				//��д��ĳ���

		int readIndex() const;

		int writeIndex() const;

		void writeToBuffer(const char* buf, int size);

		void readFromBuffer(std::vector<char>& re, int size);

		void resizeBuffer(int size);

		std::vector<char> getBufferVector();

		std::string getBufferString();

		void resetRead(int index);				//���������
		  
		void resetWrite(int index);				//д������

		void adjustBuffer();					//��ȡ���ȴﵽ1/3ʱ�������µ���


	private:
		int m_read_index_{ 0 };					//�Ѷ�ȡ��λ��
		int m_write_index_{ 0 };				//д���λ��
		int m_size_{ 0 };
	public:
		std::vector<char> m_buffer_;


	};

}
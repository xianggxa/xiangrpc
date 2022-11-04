#pragma once

#include<functional>
#include<ucontext.h>
#include<list>
#include<queue>
#include<string.h>

#include"rlog.h"

namespace xiangrpc {
#define STACK_SIZE (1024*1024)

	enum class CoroutineStatus
	{
		COROUTINE_SLEEP,
		COROUTINE_READY,
		COROUTINE_RUNNING,
		COROUTINE_SUSPEND

	};
	class Coroutine;

	struct CoroNode {
		CoroNode* pre_ = nullptr;
		CoroNode* next_ = nullptr;
		Coroutine* cur_cor = nullptr;
		CoroNode(Coroutine* coroutine) :cur_cor(coroutine) {}
	};
	struct CoroList {



		CoroNode* head_ = nullptr;
		CoroNode* back_ = nullptr;
		void push_back(CoroNode* coronode) {
			if (back_ == nullptr) {
				head_ = coronode;
				back_ = coronode;
			}
			else {
				back_->next_ = coronode;
				coronode->pre_ = back_;
				back_ = back_->next_;
			}
		}
		void erase(CoroNode* coronode) {
			if (coronode == head_) {
				head_ = coronode->next_;
			}
			if (coronode == back_) {
				back_ = coronode->pre_;
			}
			if (coronode->pre_ != nullptr) {
				coronode->pre_->next_ = coronode->next_;
			}
			if (coronode->next_ != nullptr) {
				coronode->next_->pre_ = coronode->pre_;
			}
			coronode->pre_ = nullptr;
			coronode->next_ = nullptr;

		}

	};

	class Schedule {
	public:
		Schedule();
		~Schedule();
		Coroutine* CreateCoroutine(std::function<void()> fun);						//������Э��
		Coroutine* AddTask(std::function<void()> fun);								//�������
		void Resume(Coroutine* coroutine);											//�ָ�Э���ֳ�
		void AddFreeCoroutine(CoroNode* coronode) { unu_cor_.push(coronode); }
		void CoroutineCheck();														//������Э�̽϶�ʱ��������
		void DeleLivnum() { liv_num_--; }
		void SetRunning(Coroutine* coroutine) { p_running = coroutine; }
		Coroutine* GetRunningCoroutine() { return p_running; }
		ucontext_t* GetMainctx() { return &sche_main_; }
		void Yield();
	private:
		ucontext_t sche_main_;

		int sche_size_;						//Э������
		int liv_num_;						//����Э����

		std::queue<CoroNode*> unu_cor_;		//�������Э��
		CoroList cred_cor_;					//�������д�����Э��
		Coroutine* p_running;				//�������е�Э��

		char sche_stack_[STACK_SIZE];		//����ջ


	};


	class Coroutine {
	public:
		Coroutine(std::function<void()> fun);
		~Coroutine();
		static void Start(Coroutine* coroutine, Schedule* sche);	//��װִ�к���
		void SetRun(std::function<void()> fun) { run_ = fun; }
		void SetStatus(CoroutineStatus statu) { status_ = statu; }
		void SetStackCap(size_t stack_cap) { stack_cap_ = stack_cap; }
		void SetStackSize(size_t stack_size) { stack_size_ = stack_size; }
		void SetNode(CoroNode* coronode) { this->coronode = coronode; }
		CoroutineStatus GetStatus() { return status_; }
		ucontext_t* GetUcontext() { return &ctx_; }
		size_t GetStackSize() { return stack_size_; }
		char* GetStack() { return stack_; }
		size_t GetStackCap() { return stack_cap_; }
		CoroNode* GetCoronode() { return coronode; }
		void Resize() { stack_ = new char[stack_cap_]; }

	private:
		CoroutineStatus status_;				//Э��״̬
		std::function<void()> run_;				//ִ�к�������
		ucontext_t ctx_;						//Э��������
		size_t stack_size_;						//ջ���пռ��С
		size_t stack_cap_;						//ջ����
		char* stack_;							//ջ��ַ
												//Э��ջ�����г�ʱ��� ����ʱʹ�ù���ջ
		CoroNode* coronode;						//����Э���������ж�Ӧ��λ��



	};
	static thread_local Schedule* instance = nullptr;

	static Schedule* GetInstance() {
		if (instance == nullptr)
			instance = new Schedule();
		return instance;
	}
	static Coroutine* CreateCoroutine(std::function<void()> fun) {//������Э��
		Schedule* ptr = GetInstance();
		return ptr->CreateCoroutine(fun);
	};
	static Coroutine* AddTask(std::function<void()> fun) {//�������
		Schedule* ptr = GetInstance();

		return ptr->AddTask(fun);
	}
	static void Resume(Coroutine* coroutine) {//�ָ�Э���ֳ�
		Schedule* ptr = GetInstance();
		ptr->Resume(coroutine);
	}

	static void CoroutineCheck() {//������Э�̽϶�ʱ��������
		Schedule* ptr = GetInstance();
		ptr->CoroutineCheck();
	}

	static void Yield() {
		Schedule* ptr = GetInstance();
		ptr->Yield();
	}
	static Coroutine* GetRunningCoroutine() {
		Schedule* ptr = GetInstance();
		return ptr->GetRunningCoroutine();
	}
}
/*
----------------------
|					 |											|  height
|--------------------|-->   stack + STACK_SIZE	-				|
|////////////////////|							|				|
|////////////////////|-->   used				|				|
|////////////////////|							|->STACK_SIZE	|
|--------------------|-->	dummy				|				|
|					 |							|				|
|--------------------|-->	stack				-				|  low
|																V
|
Э�̵Ķѿռ����Ϊջ�ռ�Ĵ洢���ռ䲼����ͬ
stack ��Ϊջ�ռ��Ϸ��������
����stack[k] Ϊstack�����׵�ַ��ƫ��������
���stackΪ���������ջ�ռ��ڵĵ͵�ַ
*/
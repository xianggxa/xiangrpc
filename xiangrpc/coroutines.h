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
		Coroutine* CreateCoroutine(std::function<void()> fun);						//增添新协程
		Coroutine* AddTask(std::function<void()> fun);								//添加任务
		void Resume(Coroutine* coroutine);											//恢复协程现场
		void AddFreeCoroutine(CoroNode* coronode) { unu_cor_.push(coronode); }
		void CoroutineCheck();														//当闲置协程较多时进行清理
		void DeleLivnum() { liv_num_--; }
		void SetRunning(Coroutine* coroutine) { p_running = coroutine; }
		Coroutine* GetRunningCoroutine() { return p_running; }
		ucontext_t* GetMainctx() { return &sche_main_; }
		void Yield();
	private:
		ucontext_t sche_main_;

		int sche_size_;						//协程容量
		int liv_num_;						//存活的协程数

		std::queue<CoroNode*> unu_cor_;		//储存空闲协程
		CoroList cred_cor_;					//储存所有创建的协程
		Coroutine* p_running;				//正在运行的协程

		char sche_stack_[STACK_SIZE];		//共享栈


	};


	class Coroutine {
	public:
		Coroutine(std::function<void()> fun);
		~Coroutine();
		static void Start(Coroutine* coroutine, Schedule* sche);	//封装执行函数
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
		CoroutineStatus status_;				//协程状态
		std::function<void()> run_;				//执行函数对象
		ucontext_t ctx_;						//协程上下文
		size_t stack_size_;						//栈运行空间大小
		size_t stack_cap_;						//栈容量
		char* stack_;							//栈地址
												//协程栈用于切出时存放 运行时使用共享栈
		CoroNode* coronode;						//储存协程在链表中对应的位置



	};
	static thread_local Schedule* instance = nullptr;

	static Schedule* GetInstance() {
		if (instance == nullptr)
			instance = new Schedule();
		return instance;
	}
	static Coroutine* CreateCoroutine(std::function<void()> fun) {//增添新协程
		Schedule* ptr = GetInstance();
		return ptr->CreateCoroutine(fun);
	};
	static Coroutine* AddTask(std::function<void()> fun) {//添加任务
		Schedule* ptr = GetInstance();

		return ptr->AddTask(fun);
	}
	static void Resume(Coroutine* coroutine) {//恢复协程现场
		Schedule* ptr = GetInstance();
		ptr->Resume(coroutine);
	}

	static void CoroutineCheck() {//当闲置协程较多时进行清理
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
协程的堆空间仅作为栈空间的存储布空间布局相同
stack 作为栈空间上分配的数组
由于stack[k] 为stack数组首地址加偏移量所得
因此stack为连续申请的栈空间内的低地址
*/
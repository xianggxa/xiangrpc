#include "coroutines.h"
namespace xiangrpc {



	Coroutine::Coroutine(std::function<void()> fun) :
		stack_cap_(0),
		stack_size_(0),
		stack_(nullptr),
		status_(CoroutineStatus::COROUTINE_READY)   //协程初始为准备状态
	{
		run_ = fun;
	}

	Coroutine::~Coroutine()
	{
		delete stack_;
	}

	void Coroutine::Start(Coroutine* coroutine, Schedule* sche)
	{

		coroutine->run_();
		coroutine->SetStatus(CoroutineStatus::COROUTINE_SLEEP);	//函数运行完后将协程设置为休眠状态
		sche->AddFreeCoroutine(coroutine->GetCoronode());		//将休眠协程放入队列
		sche->DeleLivnum();
		sche->SetRunning(nullptr);
		sche->CoroutineCheck();
	}

	Schedule::Schedule() :
		sche_size_(0),
		liv_num_(0),
		p_running(nullptr)
	{
	}

	Schedule::~Schedule()
	{

		for (auto p = cred_cor_.head_; p != nullptr; p = p->next_) {
			delete p->cur_cor;
			delete p;
		}
	}

	Coroutine* Schedule::CreateCoroutine(std::function<void()> fun)
	{
		Coroutine* cor = new Coroutine(fun);
		++sche_size_;
		++liv_num_;
		CoroNode* coronode = new CoroNode(cor);
		cred_cor_.push_back(coronode);
		coronode->cur_cor->SetNode(coronode);

		return coronode->cur_cor;


	}

	Coroutine* Schedule::AddTask(std::function<void()> fun)
	{
		if (!unu_cor_.empty()) {
			auto p = unu_cor_.front();
			unu_cor_.pop();
			++liv_num_;
			p->cur_cor->SetRun(fun);
			p->cur_cor->SetStatus(CoroutineStatus::COROUTINE_READY);
			p->cur_cor->SetNode(p);
			return p->cur_cor;

		}
		else return CreateCoroutine(fun);
	}

	void Schedule::Resume(Coroutine* coroutine)
	{	
		if (coroutine == nullptr || p_running != nullptr) { 
			LOG_ERROR("resume failed, coroutine still working");
			return;
		}
		switch (coroutine->GetStatus()) {
		case CoroutineStatus::COROUTINE_READY: {									//协程准备状态对协程进行初始化
			ucontext_t* ctx = coroutine->GetUcontext();
			getcontext(ctx);
			ctx->uc_stack.ss_sp = sche_stack_;									//将协程地址设为共享栈地址
			ctx->uc_stack.ss_size = STACK_SIZE;
			ctx->uc_link = &sche_main_;
			p_running = coroutine;												//标记为当前正在运行的协程
			coroutine->SetStatus(CoroutineStatus::COROUTINE_RUNNING);
			makecontext(ctx, (void(*)())Coroutine::Start, 2, coroutine, this);	//为协程设置执行函数
			swapcontext(&sche_main_, ctx);										//保存当前上下文并切换为协程上下文
			break;
		}
		case CoroutineStatus::COROUTINE_SUSPEND: {								//协程中断状态
			memcpy(sche_stack_ + STACK_SIZE - coroutine->GetStackSize(), coroutine->GetStack(), coroutine->GetStackSize());																	//memcpy从低地址到高地址复制
			p_running = coroutine;
			coroutine->SetStatus(CoroutineStatus::COROUTINE_RUNNING);
			ucontext_t* ctx = coroutine->GetUcontext();
			swapcontext(&sche_main_, ctx);
			break;
		}
		default:
			break;

		}



	}

	void Schedule::CoroutineCheck()
	{
		if (liv_num_ <= sche_size_ / 4 && liv_num_ > 100) {
			int num = sche_size_ / 2;
			while (num--) {
				CoroNode* coronode = unu_cor_.front();
				unu_cor_.pop();
				cred_cor_.erase(coronode);
				delete coronode->cur_cor;
				delete coronode;
			}
		}
	}

	void Schedule::Yield()
	{
		Coroutine* coroutine = p_running;
		if (coroutine == nullptr)return;
		char dummy = 0;
		if (coroutine->GetStackCap() < sche_stack_ + STACK_SIZE - &dummy) {	//当协程原有容量小于所需时
			delete coroutine->GetStack();
			coroutine->SetStackCap(sche_stack_ + STACK_SIZE - &dummy);
			coroutine->Resize();

		}
		coroutine->SetStackSize(sche_stack_ + STACK_SIZE - &dummy);
		memcpy(coroutine->GetStack(), &dummy, coroutine->GetStackSize());	//将协程栈复制到共享栈
		coroutine->SetStatus(CoroutineStatus::COROUTINE_SUSPEND);
		p_running = nullptr;
		ucontext_t* ctx = coroutine->GetUcontext();
		swapcontext(ctx, &sche_main_);



	}

}
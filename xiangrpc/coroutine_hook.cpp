#include "coroutine_hook.h"
#include "fd_event.h"

namespace xiangrpc {

    int accept_hook(int sockfd, sockaddr* addr, socklen_t* addrlen)
    {
       
        xiangrpc::FdEvent::ptr fd_event = xiangrpc::FdEventContainer::getFdEventContainer().getFdevent(sockfd);

        if (fd_event->getReactor() == nullptr) {
            fd_event->setReactor(xiangrpc::GetReactorInstance());
        }

        int n = accept(sockfd, addr, addrlen);
        if (n > 0) {
            return n;
        }
        toEpoll(fd_event, xiangrpc::IOEvent::READ);

        xiangrpc::Yield();

        fd_event->delListenEvents(xiangrpc::IOEvent::READ);
        return accept(sockfd, addr, addrlen);               //此时为协程唤醒后操作

    }

    ssize_t recv_hook(int fd, void* buf, size_t count)              //recv只负责确定接收，拆包解包在tcpconnection层
    {
        xiangrpc::FdEvent::ptr fd_event = xiangrpc::FdEventContainer::getFdEventContainer().getFdevent(fd);

        if (fd_event->getReactor() == nullptr) {
            fd_event->setReactor(xiangrpc::GetReactorInstance());
        }

        int n = recv(fd, buf, count, MSG_DONTWAIT);                         //非阻塞io       
        if (n > 0) {
            return n;
        }
        toEpoll(fd_event, xiangrpc::IOEvent::READ);

        xiangrpc::Yield();

        fd_event->delListenEvents(xiangrpc::IOEvent::READ);
        return recv(fd, buf, count, MSG_DONTWAIT);               //此时为协程唤醒后操作
        
    }

    ssize_t send_hook(int fd, const void* buf, size_t count)
    {
        xiangrpc::FdEvent::ptr fd_event = xiangrpc::FdEventContainer::getFdEventContainer().getFdevent(fd);

        if (fd_event->getReactor() == nullptr) {
            fd_event->setReactor(xiangrpc::GetReactorInstance());
        }

        int n = send(fd, buf, count, MSG_DONTWAIT);             //非阻塞io       
        if (n > 0) {
            return n;
        }
        toEpoll(fd_event, xiangrpc::IOEvent::WRITE);

        xiangrpc::Yield();

        fd_event->delListenEvents(xiangrpc::IOEvent::WRITE);
        return send(fd, buf, count, MSG_DONTWAIT);               //此时为协程唤醒后操作
        
    }

    int connect_hook(int sockfd, const sockaddr* addr, socklen_t addrlen)
    {

        return 0;
    }

    void toEpoll(xiangrpc::FdEvent::ptr fd_event, int events) {
        xiangrpc::Coroutine* run_cor = xiangrpc::GetRunningCoroutine();
        if (events & xiangrpc::IOEvent::READ) {
            LOG_DEBUG("fd: %d ,register read to epoll", fd_event->getFd());

            fd_event->setCoroutine(run_cor);
            fd_event->addListenEvents(xiangrpc::IOEvent::READ);             //注册到reactor所在epoll
        }
        if (events & xiangrpc::IOEvent::WRITE) {
            LOG_DEBUG("fd: %d ,register write to epoll", fd_event->getFd());

            fd_event->setCoroutine(run_cor);
            fd_event->addListenEvents(xiangrpc::IOEvent::WRITE);
        }


    }
}
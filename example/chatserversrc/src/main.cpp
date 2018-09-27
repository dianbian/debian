/**
 *  聊天服务程序入口函数
 **/
#include <signal.h>
#include <iostream>
#include "Logging.h"
#include "IMServer.h"
#include "singleton.h"

EventLoop g_mainLoop;

void prog_exit(int signo)
{
    std::cout << "program recv signal [" << signo << "] to exit." << std::endl;

    g_mainLoop.quit();
}


int main(int argc, char* argv[])
{
    //设置信号处理
    signal(SIGCHLD, SIG_DFL);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, prog_exit);
    signal(SIGTERM, prog_exit);

    

    Singleton<IMServer>::Instance().start(&g_mainLoop);

    LOG_INFO << "chatServer 0.0.0.1v initialzation complete";
    g_mainLoop.loop();

    return 0;
}

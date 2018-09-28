/**
 *  聊天服务程序入口函数
 **/
#include <signal.h>
#include <iostream>
#include "Logging.h"
#include "IMServer.h"
#include "singleton.h"
#include "AsyncLogging.h"

EventLoop g_mainLoop;

AsyncLogging* g_asyncLog = NULL;

void prog_exit(int signo)
{
    std::cout << "program recv signal [" << signo << "] to exit." << std::endl;

    g_mainLoop.quit();
}

void asyncOutput(const char* msg, int len)
{
    if (g_asyncLog != NULL)
    {
        g_asyncLog->append(msg, len);
        std::cout << msg << std::endl;
    }
}
int main(int argc, char* argv[])
{
    printf("..........\n");
    InitializeLog();
     
    Logger::setLogLevel(Logger::DEBUG);

    AsyncLogging log("logs/chatServer", 10000000);
    log.start();
    g_asyncLog = &log;
    Logger::setOutput(asyncOutput);


    //LOG_ << "logfilename is not set in config file";
    LOG_INFO << "chatServer 0.0.0.1v initialzation .......";
    //设置信号处理
    signal(SIGCHLD, SIG_DFL);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, prog_exit);
    signal(SIGTERM, prog_exit);
    
    Singleton<IMServer>::Instance().start(&g_mainLoop);

    //LOG_INFO << "chatServer 0.0.0.1v initialzation complete";
    g_mainLoop.loop();

    return 0;
}

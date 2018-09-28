/**
 *  �������������࣬IMServer.cpp
 **/
#include "InetAddress.h"
#include "Logging.h"
#include "singleton.h"
#include "IMServer.h"
#include "ClientSession.h"
#include "UserManager.h"
#include "configfilereader.h"

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

CConfigFileReader config;

//AsyncLogging* g_asyncLog = NULL;
/*
void asyncOutput(const char* msg, int len)
{
    if (g_asyncLog != NULL)
    {
        g_asyncLog->append(msg, len);
        //std::cout << msg << std::endl;
    }
}*/

void InitializeLog()
{
    config.InitFile("chatserver.conf");
    const char* logfilepath = config.GetConfigName("logfiledir");
    if (logfilepath == NULL)
    {
        //LOG_SYSFATAL << "logdir is not set in config file";
        exit(1);
    }
    DIR* dp = opendir(logfilepath);
    if (dp == NULL)
    {
        if (mkdir(logfilepath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
        {
            //LOG_SYSFATAL << "create base dir error, " << logfilepath << ", errno: " << errno << ", " << strerror(errno);
            exit(1);
        }
    }
    closedir(dp);

    const char* logfilename = config.GetConfigName("logfilename");
    if (logfilename == NULL)
    {
       //LOG_SYSFATAL << "logfilename is not set in config file";
       exit(1);
    }
    std::string strLogFileFullPath(logfilepath);
    strLogFileFullPath += logfilename;
    int kRollSize = 500 * 1000 * 1000;
    //printf("logname = %s\n", strLogFileFullPath.c_str());
    
 /*   AsyncLogging log(strLogFileFullPath.c_str(), kRollSize);
    log.start();
    g_asyncLog = &log;
    Logger::setOutput(asyncOutput);
    */
    
    //LOG_SYSFATAL << "logfilename is not set in config file"; 
}

bool IMServer::Init(const char* ip, short port, int threadNum, EventLoop* loop)
{   
    InetAddress addr(ip, port);
    m_server.reset(new TcpServer(loop, addr, "bianMan", TcpServer::kReusePort));
    m_server->setConnectionCallback(std::bind(&IMServer::OnConnection, this, std::placeholders::_1));
    m_server->setThreadNum(threadNum);
    //start listen
    m_server->start();

    return true;
}

void IMServer::start(EventLoop *loop)
{
    Logger::setLogLevel(Logger::INFO);
    config.InitFile("chatserver.conf");
    const char* logfilepath = config.GetConfigName("logfiledir");
    if (logfilepath == NULL)
    {
        LOG_SYSFATAL << "logdir is not set in config file";
        exit(1);
    }
   
    const char* listenip = config.GetConfigName("listenip");
    short listenport = (short)atol(config.GetConfigName("listenport"));
    int threadNum = (int)atol(config.GetConfigName("threadnum"));

    Init(listenip, listenport, threadNum, loop);
}

void IMServer::OnConnection(std::shared_ptr<TcpConnection> conn)
{
    if (conn->connected())
    {
        LOG_INFO << "client connected:" << conn->peerAddress().toIpPort();
        ++ m_baseUserId;
        std::shared_ptr<ClientSession> spSession(new ClientSession(conn));
        conn->setMessageCallback(std::bind(&ClientSession::OnRead, spSession.get(), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));       

        //std::lock_guard<std::mutex> guard(m_sessionMutex);
        // m_sessions.push_back(spSession);
    }
    else
    {
        OnClose(conn);
    }
}

void IMServer::OnClose(const std::shared_ptr<TcpConnection>& conn)
{
    //�Ƿ����û�����
    //bool bUserOffline = false;
    UserManager& userManager = Singleton<UserManager>::Instance();

    //TODO: �����Ĵ����߼�̫���ң���Ҫ�Ż�
    //std::lock_guard<std::mutex> guard(m_sessionMutex);
    for (auto iter = m_sessions.begin(); iter != m_sessions.end(); ++iter)
    {
        if ((*iter)->GetConnectionPtr() == NULL)
        {
            LOG_ERROR << "connection is NULL";
            break;
        }
        
        //ͨ���ȶ�connection�����ҵ���Ӧ��session
        if ((*iter)->GetConnectionPtr() == conn)
        {
            //��Session����֮ǰ�������ߵ���ЧSession������Ϊ���������ߣ��Ÿ������������������Ϣ
            if ((*iter)->IsSessionValid())
            { 
                //���������ߺ��ѣ��������������������Ϣ
                std::list<User> friends;
                int32_t offlineUserId = (*iter)->GetUserId();
                userManager.GetFriendInfoByUserId(offlineUserId, friends);
                for (const auto& iter2 : friends)
                {
                    for (auto& iter3 : m_sessions)
                    {
                        //�ú����Ƿ����ߣ����߻����session��
                        if (iter2.userid == iter3->GetUserId())
                        {
                            iter3->SendUserStatusChangeMsg(offlineUserId, 2);

                            LOG_INFO << "SendUserStatusChangeMsg to user(userid=" << iter3->GetUserId() << "): user go offline, offline userid = " << offlineUserId;
                        }
                    }
                }
            }
            else
            {
                LOG_INFO << "Session is invalid, userid=" << (*iter)->GetUserId();
            }
            
            //ͣ����Session�ĵ��߼��
            //(*iter)->DisableHeartbaetCheck();
            //�û�����
            m_sessions.erase(iter);
            //bUserOffline = true;
            LOG_INFO << "client disconnected: " << conn->peerAddress().toIpPort();
            break;
        }
    }

    LOG_INFO << "current online user count: " << m_sessions.size();
}

void IMServer::GetSessions(std::list<std::shared_ptr<ClientSession>>& sessions)
{
    //std::lock_guard<std::mutex> guard(m_sessionMutex);
    sessions = m_sessions;
}

bool IMServer::GetSessionByUserIdAndClientType(std::shared_ptr<ClientSession>& session, int32_t userid, int32_t clientType)
{
    //std::lock_guard<std::mutex> guard(m_sessionMutex);
    std::shared_ptr<ClientSession> tmpSession;
    for (const auto& iter : m_sessions)
    {
        tmpSession = iter;
        if (iter->GetUserId() == userid && iter->GetClientType() == clientType)
        {
            session = tmpSession;
            return true;
        }
    }

    return false;
}

bool IMServer::GetSessionsByUserId(std::list<std::shared_ptr<ClientSession>>& sessions, int32_t userid)
{
    //std::lock_guard<std::mutex> guard(m_sessionMutex);
    std::shared_ptr<ClientSession> tmpSession;
    for (const auto& iter : m_sessions)
    {
        tmpSession = iter;
        if (iter->GetUserId() == userid)
        {
            sessions.push_back(tmpSession);
            return true;
        }
    }

    return false;
}

int32_t IMServer::GetUserStatusByUserId(int32_t userid)
{
    //std::lock_guard<std::mutex> guard(m_sessionMutex);
    for (const auto& iter : m_sessions)
    {
        if (iter->GetUserId() == userid)
        {
            return iter->GetUserStatus();
        }
    }

    return 0;
}

int32_t IMServer::GetUserClientTypeByUserId(int32_t userid)
{
    //std::lock_guard<std::mutex> guard(m_sessionMutex);
    bool bMobileOnline = false;
    int clientType = CLIENT_TYPE_UNKOWN;
    for (const auto& iter : m_sessions)
    {
        if (iter->GetUserId() == userid)
        {   
            clientType = iter->GetUserClientType();
            //��������ֱ�ӷ��ص�������״̬
            if (clientType == CLIENT_TYPE_PC)
                return clientType;
            else if (clientType == CLIENT_TYPE_ANDROID || clientType == CLIENT_TYPE_IOS)
                bMobileOnline = true;
        }
    }

    //ֻ���ֻ����߲ŷ����ֻ�����״̬
    if (bMobileOnline)
        return clientType;

    return CLIENT_TYPE_UNKOWN;
}

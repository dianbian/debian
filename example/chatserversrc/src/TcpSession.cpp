/**
 * TcpSession.cpp
 * zhangyl 2017.03.09
 **/
#include "Logging.h"
#include "Msg.h"
#include "protocolstream.h"
#include "TcpSession.h"

TcpSession::TcpSession(const std::weak_ptr<TcpConnection>& tmpconn) : tmpConn_(tmpconn)
{
    
}

TcpSession::~TcpSession()
{
    
}

void TcpSession::Send(int32_t cmd, int32_t seq, const std::string& data)
{
    Send(cmd, seq, data.c_str(), data.length());
}

void TcpSession::Send(int32_t cmd, int32_t seq, const char* data, int32_t dataLength)
{
    std::string outbuf;
    balloon::BinaryWriteStream writeStream(&outbuf);
    writeStream.WriteInt32(cmd);
    writeStream.WriteInt32(seq);
    writeStream.WriteCString(data, dataLength);
    writeStream.Flush();

    SendPackage(outbuf.c_str(), outbuf.length());
}

void TcpSession::Send(const std::string& p)
{
    SendPackage(p.c_str(), p.length());
}

void TcpSession::Send(const char* p, int32_t length)
{
    SendPackage(p, length);
}

void TcpSession::SendPackage(const char* p, int32_t length)
{   
    string strPackageData;
    msg header = { (int32_t)length };
    //LOG_INFO << "Send data, header length:" << sizeof(header) << ", body length:" << outbuf.length();
    //插入一个包头
    strPackageData.append((const char*)&header, sizeof(header));
    strPackageData.append(p, length);

    //TODO: 这些Session和connection对象的生命周期要好好梳理一下
    if (tmpConn_.expired()) //检测weak_ptr<TcpConnection>是否有可用对象 类似 tmpConn.use_count == 0
    {
        //FIXME: 出现这种问题需要排查
        LOG_ERROR << "Tcp connection is destroyed , but why TcpSession is still alive ?";
        return;
    }

    std::shared_ptr<TcpConnection> conn = tmpConn_.lock(); //lock函数可获取有效的指针
    if (conn)
    {
        size_t length = strPackageData.length();
        //LOG_INFO << "Send data, length:" << length;
        //LOG_DEBUG_BIN((unsigned char*)strSendData.c_str(), length);
        conn->send(strPackageData.c_str(), length);
    }
}

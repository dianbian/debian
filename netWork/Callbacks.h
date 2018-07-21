
//this is a public header file.

#ifndef NETWORK_CALLBACKS_H
#define NETWORK_CALLBACKS_H

#include "../baseCom/Timestamp.h"

#include <memory>
#include <functional>

//all client visible callbacks go here.

class Buffer;
class TcpConnection;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::function<void()> TimerCallback;
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void (const TcpConnectionPtr&, size_t)> HighWaterMarkCallback;

//the data has been read to (buff, len)
typedef std::function<void (const TcpConnectionPtr&, Buffer*, Timestamp)> MessageCallback;

void defaultConnetionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receiveTime);


#endif //NETWORK_CALLBACKS_H


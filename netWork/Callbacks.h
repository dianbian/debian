
//this is a public header file.

#ifndef NETWORK_CALLBACKS_H
#define NETWORK_CALLBACKS_H

#include "../baseCom/Timestamp.h"

//all client visible callbacks go here.

class Buffer;
class TcpConnection;

typedef shared_ptr<TcpConnection> TcpConnectionPtr;
typedef function<void()> TimerCallback;
typedef function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef function<void (const TcpConnectionPtr&)> CloseCallback;
typedef function<void (const TcpConnectionPtr&)> WriteCompleteCallback;
typedef function<void (const TcpConnectionPtr&, size_t) HighWaterMarkCallback;

//the data has been read to (buff, len)
typedef function<void (const TcpConnectionPtr&, Buffer*, Timestamp)> MessageCallback;

void defaultConnetionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receiveTime);


#endif //NETWORK_CALLBACKS_H


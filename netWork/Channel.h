
#ifndef NETWORK_CHANNEL_H
#define NETWORK_CHANNEL_H

#include "../baseCom/Timestamp.h"
#include "../baseCom/noncopyeable.h"

#include <memory>
#include <functionl>

class EventLoop;

//A selectable I/O channel.
//this class does't own the file descriptor.
//the file descriptor could be a socket,
//an eventfd, a timerfd, or signalfd.

class Channel : public noncopyeable
{
	public:
	    typedef std::function<void ()> EventCallback;
		typedef std::function<void (Timestamp)> ReadEventCallback;
		
		Channel(EventLoop* loop, int fd);
		~Channel();
		
		void handleEvent(Timestamp receiveTime);
		void setReadCallback(const ReadEventCallback& cb) { readCallback_ = cb; }
		void setWriteCallback(const EventCallback& cb) { writeCallback = cb; }
		void setCloseCallback(const EventCallback& cb) { closeCallback = cb; }
		void setErrorCallback(const EventCallback& cb) { errorCallback = cb; }
#ifdef __GXX_EXPERIMENTAL_CXX0X__
        void setReadCallback(ReadEventCallback&& cb) { readCallback_ = std::move(cb); }
		void setWriteCallback(EventCallback&& cb) { writeCallback = std::move(cb); }
		void setCloseCallback(EventCallback&& cb) { closeCallback = std::move(cb); }
		void setErrorCallback(EventCallback&& cb) { errorCallback = std::move(cb); }
#endif
        //tie this channel to the owner object managed by shared_ptr,
		//prevent the owner object being destroyed in handleEvent.
		void tie(const std::shared_ptr<void>&);
		int fd() const { return fd_; }
		int events() const { return events_; }
		void set_revents(int revt) { revents_ = revt; }  //used by pollers
		bool isNoneEvent() const { return events_ == kNoneEvent; }
		
		void enableReading() { events_ |= kReadEvent; update(); }
		void disableReading() { events_ &= ~kReadEvent; update(); }
		void enableWriting() { events_ |= kWriteEvent; update(); }
		void disableWriting() { events_ &= ~kWriteEvent; update(); }
		void disableAll() { events_ = kNoneEvent; update(); }
		bool isWriting() const { return events_ & kWriteEvent; }
		bool isReading() const { return events_ & kReadEvent; }
		
		//for poller
		int index() { return index_; }
		void set_index(int idx) { index_ = idx; }
		
		//for debug
		std::string reventsToString() const;
		std::string eventsToString() const;
		
		void doNotLogHup() { logHup_ = false; }
		
		EventLoop* ownerLoop() { return loop_; }
		void remove();
	private:
	    static std::string eventsToString(fd, int ev);
		void update();
		void handleEventWithGuard(Timestamp receiveTime);
		
		static const int kNoneEvent;
		static const int kReadEvent；
		static const int kWriteEvent;
		
		EventLoop* loop_;
		const int fd_;
		int events_;
		int revents_; //it's the received event types of epoll or poll 
		int index_; // used by poller.
		bool logHup_;
		
		std::weak_ptr<void> tie_;
		bool tied_;
		bool eventHandling_;
		bool addedToLoop_;
		ReadEventCallback readCallback_;
		EventCallback writeCallback;
		EventCallback closeCallback;
		errorCallback errorCallback;
		
};

#endif //NETWORK_CHANNEL_H
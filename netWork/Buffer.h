
#ifndef NETWORK_BUFFER_H
#define NETWORK_BUFFER_H

#include "../baseCom/StringPiece.h"
#include "Endian.h"

#include <algorithm>
#include <vector>
#include <assert.h>
#include <string.h>

//A buffer class modeled after neetty.buffer.ChannelBuffer

/// @code
///// +-------------------+------------------+------------------+
///// | prependable bytes |  readable bytes  |  writable bytes  |
///// |                   |     (CONTENT)    |                  |
///// +-------------------+------------------+------------------+
///// |                   |                  |                  |
///// 0      <=      readerIndex   <=   writerIndex    <=     size
///// @endcode

class Buffer
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;

	explicit Buffer(size_t initialSize = kInitialSize)
		: buffer_(kCheapPrepend + initialSize), readerIndex_(kCheapPrepend), writerIndex_(kCheapPrepend) {
			assert(readableBytes() == 0);
			assert(writableBytes() == 0);
			assert(prependableBytes() == kCheapPrepend);
		}
	//implicit copy-ctor, move-ctor, dtor and assignment are fine
	//NOTE: implicit move-ator is added in g++ 4.6
	
	void swap(Buffer& rhs) {
		buffer_.swap(readerIndex_, rhs.readerIndex_);
		std::swap(readerIndex_, rhs.readerIndex_);
		std::swap(writerIndex_, rhs.writerIndex_);
	}

	size_t readableBytes() const { return writerIndex_ - readerIndex_; }

	size_t writableBytes() const { return buffer_.size() - writerIndex_; }

	size_t prependableBytes() const { return readerIndex_; }

	const char* peek() const { return begin() + readerIndex_; }

private:
	char* begin() { return &*buffer_.begin(); }

	const char* begin() const { return &*buffer_.begin(); }

	void makeSpace(size_t len) {
		if (writableBytes() + prependableBytes() < len + kCheapPrepend)) {
			//FIXME: move readable data
			buffer_.resize(writerIndex_ + len);
		}
		else {
			//move readable data to the front, make space inside buffer
			assert(kCheapPrepend < readerIndex_);
			size_t readable = readableBytes();
			std::copy(begin() + readerIndex_, 
					begin() + writerIndex_, 
					begin() + kCheapPrepend);
			readerIndex_ = kCheapPrepend;
			writerIndex_ = readerIndex_ + readable;
			assert(readable == readableBytes());
		}
	}

private:
	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;
	static const char kCRLF[];
};

#endif //NETWORK_BUFFER_H

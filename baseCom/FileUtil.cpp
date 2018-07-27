
#include "FileUtil.h"
#include "Logging.h"
#include "Types.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

AppendFile::AppendFile(StringArg filename) 
    : fp_(::fopen(filename.c_str(), "ae")), writtenBytes_(0) // 'e' for O_CLOEXEC
{
  assert(fp_);
  ::setbuffer(fp_, bffer_, sizeof buffer_);
  //posix_fadvise POSIX_FADV_DONTNEED 
}

AppendFile::~AppendFile()
{
  ::fclose(fp_);
}

void AppendFile::append(const char* logline, const size_t len)
{
  size_t n = write(logline, len);
  size_t remain = len - n;
  while(remain > 0)
  {
    size_t x = write(logline + n, remain);
    if (x == 0)
    {
      int err = ferror(fp_);
      if (err)
      {
        fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
      }
      break;
    }
    n += x;
    remain = len - n;
  } 
  writtenBytes_ += len;
}

void AppendFile::flush()
{
  ::fflush(fp_);
}

size_t AppendFile::write(const char* logline, size_t len)
{
  return ::fwrite_unlocked(logline, 1, len, fp_);
}

ReadSmallFile::ReadSmallFile(StringArg filename)
    : fd_(::open(filename.c_str(), O_RDONLY | O_CLOEXEC), err_(0)
{
  buf_[0] = '\0';
  if (fd_ < 0)
  {
    err_ = errno;
  }
}

ReadSmallFile::~ReadSmallFile()
{
  if (fd_ >= 0)
  {
    ::close(fd_);
  }
}

template<typename String>
int ReadSmallFile::readToString(int maxSize, String* content,
      int64_t* fileSize, int64_t* modifyTime, int64_t* createTime)
{
  static_assert(sizeof(off_t) == 8);
  assert(content != NULL);
  int err = err_;
  if (fd_ >= 0)
  {
    content.clear();
    if (fileSize)
    {
      struct stat statbuf;
      if (::fstat(fd_, &statbuf) == 0)
      {
        if (S_ISREG(statbuf.st_mode))
        {
          *fileSize = statbuf.st_size;
          content->reserve(static_cast<int>(std::min(implicit_cast<int64_t>(maxSize), *fileSize)));
        }
        else if (S_ISREG(statbuf.st_mode))
        {
          err = EISDIR;
        }
        if (modifyTime)
        {
          *modifyTime = statbuf.st_mtime;
        }
        if (createTime)
        {
          *createTime = statbuf.st_ctime;
        }
      }
      else
      {
        err = errno;
      }
    }
    while( content->size() < implicit_cast<size_t>(maxSize))
    {
      size_t toRead = std::min(implicit_cast<size_t>(maxSize) - content->size(), sizeof buf_));
      ssize_t n = ::read(fd_, buf_, toRead);
      if (n > 0)
      {
        content->append(buf_, n);
      }
      else
      {
        if (n < 0)
        {
          err = errno;
        }
        break;
      }
    }
  }
  return err;
}

int ReadSmallFile::readToBuffer(int* size)
{
  int err = err_;
  if (fd_ > 0)
  {
    ssize_t n = ::pread(fd_, buf_, sizeof(buf_) - 1, 0);
    if (n >= 0)
    {
      if (size)
      {
        *size = static_cast<int>(n);
      }
      buf_[n] = '\0';
    }
    else
    {
      err = errno;      
    }
  }
  return err;
}

template int readFile(StringArg filename, int maxSize, string* content, 
                      int64_t*, int64_t*, int64_t*);

template int ReadSmallFile::readToString(int maxSize, string* content,
                      int64_t*, int64_t*, int64_t*);
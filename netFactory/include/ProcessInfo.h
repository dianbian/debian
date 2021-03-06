
#ifndef BASECOM_PROCESSINFO_H
#define BASECOM_PROCESSINFO_H

#include "StringPiece.h"
#include "Timestamp.h"

#include <sys/types.h>

#include <vector>

using namespace std;

namespace ProcessInfo
{
  string pidString();
  string username();
  Timestamp startTime();
  int clockTicksPerSecond();
  int pageSize();
  bool isDebugBuild();  //constexpr
  
  string hostname();
  string procname();
  StringPiece procname(const string& stat);
  
  //read /proc/self/status
  string procStatus();
  //read /proc/self/stat
  string procStat();
  //read /proc/self/task/tid/stat
  string threadStat();
  //readlink /proc/self/exe
  string exePath();
  
  int openedFiles();
  int maxOpenFiles();
  
  struct CpuTime
  {
    double userSeconds;
    double systemSeconds;
    
    CpuTime() : userSeconds(0.0), systemSeconds(0.0) { }
  };
  CpuTime cpuTime();
  
  int numThreads();
  std::vector<pid_t> threads(); 
}

#endif //BASECOM_PROCESSINFO_H


#include "Exception.h"

#include <execinfo.h>
#include <stdlib.h>

Exception::Exception(const char* msg) : message_(msg)
{
	fillStackTrace();
}

Exception::Exception(const std::string& msg) : message_(msg)
{
	fillStackTrace();
}

Exception::~Exception() throw()
{
	
}

const char* Exception::what() const throw()
{
	return message_.c_str();
}

const char* Exception::stackTrace() const throw()
{
	return stack_.c_str();
}

void Exception::fillStackTrace()
{
	const int len = 200;
	void* buffer[len];
	int nptrs = ::backtrace(buffer, len);
	char** strings = ::backtrace_symbols(buffer, nptrs);
	if (strings)
	{
		for (int i = 0; i < nptrs; ++i)
		{
			stack_.append(strings[i]);
			stack_.push_back('\n');
		}
		free(strings);
	}
}


/*std::string demangle(const char *symbol)
{
    size_t size = 0;
    int status = 0;
    char temp[128] = {0};
    char *demangled = NULL;
    //first, try to demangle a c++ name
    if (sscanf(symbol, "%*[^(]%*[^_]%127[^)+]", temp) == 1) {
       // if ((demangled = abi::__cxa_demangle(temp, NULL, &size,
                        &status)) != NULL) {
            std::string result(demangled);
            free(demangled);
            return result;
        }
    }
    //if that didn't work, try to get a regular c symbol
    if (sscanf(symbol, "%127s", temp) == 1) {
        return temp;
    }
    //if all else fails, just return the symbol
    return symbol;
}*/

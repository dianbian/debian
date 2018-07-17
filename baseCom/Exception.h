
#ifndef BASECOM_EXCEPTION_H
#define BASECOM_EXCEPTION_H

#include <exception>

class Exception : public std::exception
{
	public:
	    explicit Exception(const char* msg);
		explicit Exception(const std::string& msg);
		virtual ~Exception() throw();
		virtual const char* what() const throw();
		const char* stackTrace const throw();
		
	private:
	    void fillStackTrace();
		
		std::string message_;
		std::string stack;
};

#endif //BASECOM_EXCEPTION_H
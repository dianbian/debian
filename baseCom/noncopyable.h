
#ifndef BASECOM_NONCOPYABLE_H
#define BASECOM_NONCOPYABLE_H

class noncopyable
{
	protected:
	    constexpr noncopyable() = default;
		~noncopyable() = default;
		noncopyable(const noncopyable&) = delete;
		noncopyable& operator=(const noncopyable&) = delete;
};

#endif //BASECOM_NONCOPYABLE_H

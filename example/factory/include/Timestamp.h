
#ifndef BASECOM_TIMESTAMP_H
#define BASECOM_TIMESTAMP_H

#include "copyable.h"

#include <string>

// this stamp is UTC, in microseconds resolution
// this class is immutable.
// it's recommended to pass it by value, since it's passed in register on x64

class Timestamp : public copyable/*,
				  public equality_comparable<Timestamp>,
				  public less_than_comparable<Timestamp>*/
{
 public:
		// constructs an invalid Timestamp.
    Timestamp() : microSecondsSinceEpoch_(0) { }

		//constructs a Timestamp at specific time 
		explicit Timestamp(int64_t microSecondsSinceEpochArg) :
			microSecondsSinceEpoch_(microSecondsSinceEpochArg) { }

		void swap(Timestamp& that)
		{
			std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
		}

		//defalt copy/assginment/dtor are okay
		std::string toString() const;

		std::string toFormattedString(bool showMicroseconds = true) const;

		bool valid() const { return microSecondsSinceEpoch_ > 0; }

		// for internal usage
		int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }

		time_t secondsSinceEpoch() const { 
			return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond); 
		}
		//get time of now.
		static Timestamp now();
		static Timestamp invalid() { return Timestamp(); }

		static Timestamp fromUnixTime(time_t t) {
			return fromUnixTime(t, 0);
		}

		static Timestamp fromUnixTime(time_t t, int microseconds) {
			return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
		}

		static const int kMicroSecondsPerSecond = 1000 * 1000;
	
	private:
		int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
	return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
	return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

//get time difference of two timestamps, result in seconds.
inline double timeDifference(Timestamp high, Timestamp low)
{
	int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
	return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

//add seconds to given timestamp.
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
	int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
	return Timestamp(timestamp.kMicroSecondsPerSecond + delta);
}

#endif //BASECOM_TIMESTAMP_H


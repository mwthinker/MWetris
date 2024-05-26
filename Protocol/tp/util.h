#ifndef PROTOCOL_TP_UTIL_H
#define PROTOCOL_TP_UTIL_H

#include <chrono>
#include <google/protobuf/util/time_util.h>

namespace tp {
	
	google::protobuf::Timestamp toTimestamp(const std::chrono::system_clock::time_point& timePoint) {
		auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timePoint.time_since_epoch());
		google::protobuf::Timestamp timestamp;
		timestamp.set_seconds(seconds.count());
		return timestamp;
	}

	std::chrono::year_month_day toYearMonthDay(const google::protobuf::Timestamp& timestamp) {
		auto timePoint = std::chrono::system_clock::from_time_t(timestamp.seconds());
		return std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(timePoint)};
	}



}

#endif

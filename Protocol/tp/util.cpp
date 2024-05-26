#include "util.h"
#include "shared.pb.h"

namespace tp {
	
	google::protobuf::Timestamp toTimestamp(const std::chrono::system_clock::time_point& timePoint);

	std::chrono::year_month_day toYearMonthDay(const google::protobuf::Timestamp& timestamp);

}

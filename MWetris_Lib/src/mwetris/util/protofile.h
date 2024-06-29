#ifndef MWETRIS_PROTOFILE_H
#define MWETRIS_PROTOFILE_H

#include <google/protobuf/util/time_util.h>
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif
#include <google/protobuf/message_lite.h>
#include <fstream>

namespace mwetris {
	
	template <typename Message>
	concept MessageLite = std::derived_from<Message, google::protobuf::MessageLite>;

	template <MessageLite Message>
	bool loadFromFile(Message& type, const std::string& file) {
		std::ifstream input{file};
		if (input.fail()) {
			return false;
		}
		return type.ParseFromIstream(&input);
	}

	template <MessageLite Message>
	void saveToFile(const Message& type, const std::string& file) {
		std::ofstream output{file};
		type.SerializePartialToOstream(&output);
	}

}

#endif

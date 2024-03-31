#ifndef MWETRIS_NETWORK_PROTOBUFMESSAGE_H
#define MWETRIS_NETWORK_PROTOBUFMESSAGE_H

#include <google/protobuf/message_lite.h>

#include <vector>

namespace mwetris::network {

	class ProtobufMessage {
	public:
		ProtobufMessage();

		explicit ProtobufMessage(int size);

		void clear();

		void setBuffer(const google::protobuf::MessageLite& message);

		int getSize() const noexcept {
			return static_cast<int>(buffer_.size());
		}

		constexpr int getHeaderSize() const noexcept {
			return 2;
		}

		void reserveBodySize();

		// At most 65535 bytes (2^16 - 1)
		void reserveBodySize(int size);

		int getBodySize() const;

		const unsigned char* getData() const {
			return buffer_.data();
		}

		unsigned char* getData() {
			return buffer_.data();
		}

		const unsigned char* getBodyData() const {
			return buffer_.data() + getHeaderSize();
		}

		unsigned char* getBodyData() {
			return buffer_.data() + getHeaderSize();
		}

	private:
		void reserveHeaderSize();

		void defineBodySize();

		std::vector<unsigned char> buffer_;
	};

}

#endif

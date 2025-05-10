#ifndef MWETRIS_NETWORK_PROTOBUFMESSAGE_H
#define MWETRIS_NETWORK_PROTOBUFMESSAGE_H

#include <google/protobuf/message_lite.h>
#include <asio/buffer.hpp>

#include <concepts>
#include <vector>

namespace network {

	template <typename Message>
	concept MessageLite = std::derived_from<Message, google::protobuf::MessageLite>;

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
		
		void reserveHeaderSize();

		void reserveBodySize();

		// At most 65535 bytes (2^16 - 1)
		void reserveBodySize(int size);

		int getBodySize() const;

		asio::const_buffer getDataBuffer() const {
			return asio::buffer(buffer_);
		}

		asio::mutable_buffer getMutableDataBuffer() {
			return asio::buffer(buffer_);
		}

		/// @brief Mutable buffer for the body data to be set.
		/// Important! Must reserve the body size before using this to set the data.
		/// 
		/// @return Mutable buffer
		asio::mutable_buffer getMutableBodyBuffer() {
			return asio::buffer(getBodyData(), getBodySize());
		}
		
		bool parseBodyInto(MessageLite auto& message) const {
			return message.ParseFromArray(getBodyData(), getBodySize());
		}

	private:
		const unsigned char* getBodyData() const {
			return buffer_.data() + getHeaderSize();
		}

		unsigned char* getBodyData() {
			return buffer_.data() + getHeaderSize();
		}

		void defineBodySize();

		std::vector<unsigned char> buffer_;
	};

}

#endif

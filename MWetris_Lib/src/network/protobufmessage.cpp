#include "protobufmessage.h"

namespace mwetris::network {

	ProtobufMessage::ProtobufMessage() {
	}

	ProtobufMessage::ProtobufMessage(int size)
		: buffer_(size + getHeaderSize()) {

		defineBodySize();
	}

	void ProtobufMessage::clear() {
		buffer_.resize(getHeaderSize());
		defineBodySize();
	}

	void ProtobufMessage::setBuffer(const google::protobuf::MessageLite& message) {
		auto size = message.ByteSizeLong();
		buffer_.resize(getHeaderSize() + size);
		// Assume that the message size fits in an int.
		message.SerializeToArray(buffer_.data() + getHeaderSize(), static_cast<int>(size));
		defineBodySize();
	}

	void ProtobufMessage::reserveBodySize() {
		buffer_.resize(getHeaderSize() + getBodySize());
	}

	void ProtobufMessage::reserveBodySize(int size) {
		buffer_.resize(getHeaderSize() + size);
		defineBodySize();
	}

	int ProtobufMessage::getBodySize() const {
		if (buffer_.empty()) {
			return 0;
		}
		return 255 * buffer_[0] + buffer_[1];
	}

	void ProtobufMessage::reserveHeaderSize() {
		buffer_.resize(getHeaderSize());
	}

	void ProtobufMessage::defineBodySize() {
		// Buffer size is at least header size.
		auto bodySize = buffer_.size() - getHeaderSize();
		buffer_[0] = ((bodySize >> 8) & 0xFF);
		buffer_[1] = (bodySize & 0xFF);
	}

}

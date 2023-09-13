#ifndef MWETRIS_NETWORK_CLIENT_H
#define MWETRIS_NETWORK_CLIENT_H

#include "protobufmessage.h"

namespace mwetris::network {

	class Client {
	public:
		virtual ~Client() = default;

		virtual const std::string& getUuid() const = 0;

		virtual bool receive(ProtobufMessage& message) = 0;

		virtual void send(ProtobufMessage&& message) = 0;

		virtual void acquire(ProtobufMessage& message) = 0;

		virtual void release(ProtobufMessage&& message) = 0;
	};

}

#endif

#include "debugclient.h"
#include "protobufmessagequeue.h"

#include <message.pb.h>

#include <spdlog/spdlog.h>

#include <queue>

namespace tp = tetris_protocol;

namespace mwetris::network {

	namespace {

		template<class>
		inline constexpr bool always_false_v = false;

	}

	class DebugClient::Impl {
	public:
		mw::PublicSignal<DebugClient::Impl, const std::vector<game::PlayerSlot>&> playerSlotsUpdated;

		Impl() {}

		~Impl() {}

		bool receive(ProtobufMessage& message) {
			if (toClient.empty()) {
				return false;
			}
			message = std::move(toClient.front());
			toClient.pop();
			return true;
		}

		void send(ProtobufMessage&& message) {
			wrapper_.Clear();
			bool valid = wrapper_.ParseFromArray(message.getBodyData(), message.getBodySize());
			messageQueue_.release(std::move(message));
			if (valid) {
				if (wrapper_.has_game_looby()) {
					const auto& slots = wrapper_.game_looby().slots();
					playerSlots_.clear();

					for (const auto& slot : slots) {
						switch (slot.slot_type()) {
							case tp::HUMAN:
								playerSlots_.push_back(game::Remote{
									.name = slot.name(),
									.ai = false
									});
								break;
							case tp::AI:
								playerSlots_.push_back(game::Remote{
									.name = slot.name(),
									.ai = true
									});
								break;
							case tp::REMOTE:
								playerSlots_.push_back(game::Remote{});
								break;
							case tp::OPEN_SLOT:
								playerSlots_.push_back(game::OpenSlot{});
								break;
							default:
								playerSlots_.push_back(game::ClosedSlot{});
								break;
						};
					}
				}
				playerSlotsUpdated(playerSlots_);
			} else {
				spdlog::error("Protocol error");
			}
		}

		void release(ProtobufMessage&& message) {
			messageQueue_.release(std::move(message));
		}

		void connect(const std::string& uuid) {
			wrapper_.Clear();
			connectedUuids_.push_back(uuid);
			for (const auto& connected : connectedUuids_) {
				auto uuidTp = wrapper_.mutable_connections()->add_uuids();
				uuidTp->assign(uuid);
			}
			sendToClient(wrapper_);
		}

		void disconnect(const std::string& uuid) {
			wrapper_.Clear();
			connectedUuids_.push_back(uuid);

			if (std::erase_if(connectedUuids_, [&uuid](const std::string& value) {
				return value == uuid;
			})) {
				for (const auto& connected : connectedUuids_) {
					auto uuidTp = wrapper_.mutable_connections()->add_uuids();
					uuidTp->assign(uuid);
				}
				sendToClient(wrapper_);
			} else {
				spdlog::warn("[DebugClient] Failed to disconnect uuid {} (does not match existing)", uuid);
			}
		}

		void handle() {}

		void acquire(ProtobufMessage& message) {
			messageQueue_.acquire(message);
		}

	private:
		void sendToClient(const tp::Wrapper& wrapper) {
			ProtobufMessage message;
			messageQueue_.acquire(message);
			message.setBuffer(wrapper_);
			toClient.push(std::move(message));
		}

		std::vector<game::PlayerSlot> playerSlots_;
		std::vector<std::string> connectedUuids_;

		tp::Wrapper wrapper_;

		ProtobufMessageQueue messageQueue_;

		std::queue<ProtobufMessage> toServer_;
		std::queue<ProtobufMessage> toClient;
	};

	void DebugClient::connect(const std::string& uuid) {
		impl_->connect(uuid);
	}

	void DebugClient::disconnect(const std::string& uuid) {
		impl_->disconnect(uuid);
	}

	DebugClient::DebugClient()
		: impl_{std::make_unique<DebugClient::Impl>()} {}

	DebugClient::~DebugClient() {}

	bool DebugClient::receive(ProtobufMessage& message) {
		return impl_->receive(message);
	}

	void DebugClient::send(ProtobufMessage&& message) {
		impl_->send(std::move(message));
	}

	mw::signals::Connection DebugClient::addPlayerSlotsCallback(const std::function<void(const std::vector<game::PlayerSlot>&)>& callback) {
		return impl_->playerSlotsUpdated.connect(callback);
	}

	void DebugClient::acquire(ProtobufMessage& message) {
		impl_->acquire(message);
	}

	void DebugClient::release(ProtobufMessage&& message) {
		impl_->release(std::move(message));
	}

}


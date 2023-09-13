#include "debugserver.h"
#include "debugclient.h"
#include "protobufmessagequeue.h"
#include "util.h"
#include "game/player.h"
#include "game/remoteplayer.h"

#include <helper.h>

#include <message.pb.h>

#include <spdlog/spdlog.h>

#include <queue>

namespace tp = tetris_protocol;

namespace mwetris::network {

	namespace {

		void toTpSlot(const game::PlayerSlot& playerSlot, tp::Slot& tpSlot) {
			tpSlot.set_slot_type(tp::SlotType::UNSPECIFIED_SLOT_TYPE);

			std::visit([&](auto&& slot) mutable {
				using T = std::decay_t<decltype(slot)>;
				if constexpr (std::is_same_v<T, game::Human>) {
					tpSlot.set_slot_type(tp::SlotType::HUMAN);
					tpSlot.set_name(slot.name);
				} else if constexpr (std::is_same_v<T, game::Ai>) {
					tpSlot.set_slot_type(tp::SlotType::AI);
					tpSlot.set_name(slot.name);
				} else if constexpr (std::is_same_v<T, game::Remote>) {
					tpSlot.set_slot_type(tp::SlotType::REMOTE);
				} else if constexpr (std::is_same_v<T, game::OpenSlot>) {
					tpSlot.set_slot_type(tp::SlotType::OPEN_SLOT);
				} else if constexpr (std::is_same_v<T, game::ClosedSlot>) {
					tpSlot.set_slot_type(tp::SlotType::CLOSED_SLOT);
				} else {
					static_assert(always_false_v<T>, "non-exhaustive visitor!");
				}
			}, playerSlot);
		}

	}

	class DebugServer::Impl {
	public:
		mw::PublicSignal<DebugServer::Impl, const std::vector<game::PlayerSlot>&> playerSlotsUpdated;
		mw::PublicSignal<DebugServer::Impl, const std::vector<game::RemotePlayerPtr>&> gameCreated;
		mw::PublicSignal<DebugServer::Impl, const game::InitGameEvent&> initGameEvent;
		mw::PublicSignal<DebugServer::Impl, const ConnectedClient&> connectedClientListener;

		Impl() {}

		~Impl() {}

		void update(const sdl::DeltaTime& deltaTime) {
			for (auto& remote : remotes_) {
				receivedFromClient(*remote.client);
			}
		}

		void receivedFromClient(DebugClient& client) {
			ProtobufMessage message;
			while (client.pollSentMessage(message)) {
				wrapper_.Clear();
				bool valid = wrapper_.ParseFromArray(message.getBodyData(), message.getBodySize());
				if (valid) {
					receivedFromClient(client, wrapper_);
				} else {
					spdlog::error("Protocol error");
				}

				messageQueue_.release(std::move(message));
			}
		}

		void receivedFromClient(Client& client, const tp::Wrapper& wrapper) {
			for (auto& remote : remotes_) {
				if (remote.client.get() == &client) {
					if (!remote.allowToConnect) {
						sendFailedToConnect(*remote.client);
						return;
					}
				}
			}

			if (wrapper_.has_game_looby()) {
				handleGameLooby(wrapper_.game_looby());
			}
			if (wrapper_.has_game_command()) {
				handleGameCommand(wrapper_.game_command());
			}
			if (wrapper_.has_create_server_game()) {
				handleCreateGameServer(wrapper_.create_server_game());
			}
			if (wrapper_.has_board_move()) {
				handleBoardMove(wrapper_.board_move());
			}
			if (wrapper_.has_next_block()) {
				handleBoardNextBlock(wrapper_.next_block());
			}
			if (wrapper_.has_board_external_squares()) {
				handleBoardExternalSquares(wrapper_.board_external_squares());
			}
			if (wrapper_.has_game_restart()) {
				handleGameRestart(wrapper_.game_restart());
			}
			playerSlotsUpdated(playerSlots_);
		}

		void handleGameRestart(const tp::GameRestart& gameRestart) {
			auto current = static_cast<tetris::BlockType>(gameRestart.current());
			auto next = static_cast<tetris::BlockType>(gameRestart.next());
			for (auto& [_, player] : remotePlayers_) {
				player->updateRestart(current, next);
			}
		}

		void handleBoardMove(const tp::BoardMove& boardMove) {
			auto move = static_cast<tetris::Move>(boardMove.move());
			remotePlayers_[boardMove.uuid()]->updateMove(move);
		}

		void handleBoardNextBlock(const tp::BoardNextBlock& boardNextBlock) {
			auto blockType = static_cast<tetris::BlockType>(boardNextBlock.next());
			remotePlayers_[boardNextBlock.uuid()]->updateNextBlock(blockType);
		}

		void handleBoardExternalSquares(const tp::BoardExternalSquares& boardExternalSquares) {
			auto& remotePlayer = remotePlayers_[boardExternalSquares.uuid()];
			static std::vector<tetris::BlockType> blockTypes;
			blockTypes.clear();
			for (const auto& tpBlockType : boardExternalSquares.block_types()) {
				blockTypes.push_back(static_cast<tetris::BlockType>(tpBlockType));
			}
			remotePlayer->updateAddExternalRows(blockTypes);
		}

		void handleCreateGameServer(const tp::CreateServerGame& createServerGame) {
			remotePlayers_.clear();

			auto current = static_cast<tetris::BlockType>(createServerGame.current());
			auto next = static_cast<tetris::BlockType>(createServerGame.next());

			std::vector<game::PlayerBoardPtr> playerBoards;
			for (const auto& tpLocalPlayer : createServerGame.local_players()) {
				auto remotePlayer = std::make_shared<game::RemotePlayer>(current, next, tpLocalPlayer.uuid());
				remotePlayers_[remotePlayer->getUuid()] = remotePlayer;
				playerBoards.push_back(remotePlayer->getPlayerBoard());
			}

			initGameEvent(game::InitGameEvent{playerBoards.begin(), playerBoards.end()});
		}

		void handleGameCommand(const tp::GameCommand& gameCommand) {
			paused_ = gameCommand.pause();
		}

		void handleGameLooby(const tp::GameLooby& gameLooby) {
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
				}
			}
			
			wrapper_.Clear();
			auto tpGameLooby = wrapper_.mutable_game_looby();

			for (const auto& slot : playerSlots_) {
				auto tpSlot = tpGameLooby->add_slots();
				toTpSlot(slot, *tpSlot);
			}
			sendToClients(wrapper_);
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
			sendToClients(wrapper_);
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
				sendToClients(wrapper_);
			} else {
				spdlog::warn("[DebugServer] Failed to disconnect uuid {} (does not match existing)", uuid);
			}
		}

		void handle() {}

		void acquire(ProtobufMessage& message) {
			messageQueue_.acquire(message);
		}

		void sendFailedToConnect(DebugClient& client) {
			wrapper_.Clear();
			wrapper_.mutable_failed_to_connect();
			sendToClient(client, wrapper_);
		}

		void sendPause(bool pause) {
			paused_ = pause;
			wrapper_.Clear();
			wrapper_.mutable_game_command()->set_pause(paused_);
			sendToClients(wrapper_);
		}

		void restartGame() {
			wrapper_.Clear();
			auto gameRestart = wrapper_.mutable_game_restart();
			gameRestart->set_current(static_cast<tp::BlockType>(tetris::randomBlockType()));
			gameRestart->set_next(static_cast<tp::BlockType>(tetris::randomBlockType()));
			sendToClients(wrapper_);
		}

		bool isPaused() const {
			return paused_;
		}

		std::shared_ptr<Client> createClient(std::shared_ptr<DebugServer> debugServer) {
			auto client = std::make_shared<DebugClient>(debugServer);
			const auto& remote = remotes_.emplace_back(Remote{
				.allowToConnect = false,
				.connected = false,
				.client = client
			});
			triggerConnectedClient(remote);
			return client;
		}

		std::vector<ConnectedClient> getConnectedClients() const {
			std::vector<ConnectedClient> connectedClients;
			for (const Remote& remote : remotes_) {
				connectedClients.push_back(convertToConnectedClient(remote));
			}
			return connectedClients;
		}

		void allowClientToConnect(const std::string& uuid, bool allowed) {
			for (auto& remote : remotes_) {
				if (remote.client->getUuid() == uuid) {
					remote.allowToConnect = allowed;
					triggerConnectedClient(remote);
					break;
				}
			}
		}

	private:
		struct Remote {
			bool allowToConnect;
			bool connected;
			std::shared_ptr<DebugClient> client;
		};

		ConnectedClient convertToConnectedClient(const Remote& remote) const {
			return ConnectedClient{
				.uuid = remote.client->getUuid(),
				.connected = remote.connected,
				.allowToConnect = remote.allowToConnect
			};
		}

		void triggerConnectedClient(const Remote& remote) {
			connectedClientListener(convertToConnectedClient(remote));
		}

		void sendToClients(const tp::Wrapper& wrapper) {
			for (auto& remote : remotes_) {
				sendToClient(*remote.client, wrapper);
			}
		}

		void sendToClient(DebugClient& client, const tp::Wrapper& wrapper) {
			ProtobufMessage message;
			messageQueue_.acquire(message);
			message.setBuffer(wrapper);
			client.pushReceivedMessage(std::move(message));
		}

		std::map<std::string, game::RemotePlayerPtr> remotePlayers_;
		std::vector<game::PlayerSlot> playerSlots_;
		std::vector<std::string> connectedUuids_;
		bool paused_ = false;

		tp::Wrapper wrapper_;
		ProtobufMessageQueue messageQueue_;
		std::vector<Remote> remotes_;
	};

	void DebugServer::update(const sdl::DeltaTime& deltaTime) {
		impl_->update(deltaTime);
	}

	std::shared_ptr<Client> DebugServer::createClient() {
		return impl_->createClient(shared_from_this());
	}

	void DebugServer::disconnect(const std::string& uuid) {
		impl_->disconnect(uuid);
	}

	DebugServer::DebugServer()
		: impl_{std::make_unique<DebugServer::Impl>()} {}

	DebugServer::~DebugServer() {}

	mw::signals::Connection DebugServer::addPlayerSlotsCallback(const std::function<void(const std::vector<game::PlayerSlot>&)>& callback) {
		return impl_->playerSlotsUpdated.connect(callback);
	}

	mw::signals::Connection DebugServer::addInitGameCallback(const std::function<void(const game::InitGameEvent&)>& callback) {
		return impl_->initGameEvent.connect(callback);
	}

	void DebugServer::acquire(ProtobufMessage& message) {
		impl_->acquire(message);
	}

	void DebugServer::release(ProtobufMessage&& message) {
		impl_->release(std::move(message));
	}

	void DebugServer::sendPause(bool pause) {
		impl_->sendPause(pause);
	}

	bool DebugServer::isPaused() const {
		return impl_->isPaused();
	}

	void DebugServer::restartGame() {
		impl_->restartGame();
	}

	std::vector<ConnectedClient> DebugServer::getConnectedClients() const {
		return impl_->getConnectedClients();
	}

	void DebugServer::allowClientToConnect(const std::string& uuid, bool allow) {
		impl_->allowClientToConnect(uuid, allow);
	}

	mw::signals::Connection DebugServer::addClientListener(const std::function<void(const ConnectedClient&)>& callback) {
		return impl_->connectedClientListener.connect(callback);
	}

}

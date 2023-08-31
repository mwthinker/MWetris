#include "network.h"

#include "game/tetrisgame.h"
#include "protobufmessagequeue.h"
#include "util.h"

#include <message.pb.h>

#include <net/client.h>

#include <fmt/core.h>
#include <spdlog/spdlog.h>

namespace tp = tetris_protocol;

namespace mwetris::network {

	namespace {

		constexpr std::string_view ServerIp = "127.0.0.1";
		constexpr int Port = 59412;

		std::vector<game::Human> extractHumans(const std::vector<game::PlayerSlot>& playerSlots) {
			std::vector<game::Human> humans;
			for (const auto& playerSlot : playerSlots) {
				if (auto human = std::get_if<game::Human>(&playerSlot); human) {
					humans.push_back(game::Human{
						.name = human->name,
						.device = human->device
					});
				}
			}
			return humans;
		}

		std::vector<game::Ai> extractAis(const std::vector<game::PlayerSlot>& playerSlots) {
			std::vector<game::Ai> ais;
			for (const auto& playerSlot : playerSlots) {
				if (auto ai = std::get_if<game::Ai>(&playerSlot); ai) {
					ais.push_back(game::Ai{
						.name = ai->name,
						.ai = ai->ai}
					);
				}
			}
			return ais;
		}

		tp::SlotType playerSlotToTpSlotType(const game::PlayerSlot& playerSlot) {
			tp::SlotType slotType = tp::SlotType::UNSPECIFIED_SLOT_TYPE;
			std::visit([&](auto&& slot) mutable {
				using T = std::decay_t<decltype(slot)>;
				if constexpr (std::is_same_v<T, game::Human>) {
					slotType = tp::SlotType::HUMAN;
				} else if constexpr (std::is_same_v<T, game::Ai>) {
					slotType = tp::SlotType::AI;
				} else if constexpr (std::is_same_v<T, game::Remote>) {
					slotType = tp::SlotType::REMOTE;
				} else if constexpr (std::is_same_v<T, game::OpenSlot>) {
					slotType = tp::SlotType::OPEN_SLOT;
				} else if constexpr (std::is_same_v<T, game::ClosedSlot>) {
					slotType = tp::SlotType::CLOSED_SLOT;
				} else {
					static_assert(always_false_v<T>, "non-exhaustive visitor!");
				}
			}, playerSlot);
			return slotType;
		}

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

	class Network::Impl {
	public:
		Impl(std::shared_ptr<Client> client, std::shared_ptr<game::TetrisGame> tetrisGame)
			: client_{client}
			, tetrisGame_{tetrisGame} {
			
			for (int i = 0; i < 4; ++i) {
				playerSlots_.push_back(game::OpenSlot{});
			}

			//thread_ = std::jthread(&Impl::run, this);
		}

		~Impl() {
		}

		void send(const tp::Wrapper& wrapper) {
			ProtobufMessage message;
			client_->acquire(message);
			message.setBuffer(wrapper_);
			client_->send(std::move(message));
		}

		void update() {
			ProtobufMessage message;
			while (client_->receive(message)) {
				wrapper_.Clear();
				bool valid = wrapper_.ParseFromArray(message.getBodyData(), message.getBodySize());
				client_->release(std::move(message));
				if (wrapper_.has_game_looby()) {
					handleGameLooby(wrapper_.game_looby());
				}
				if (wrapper_.has_game_command()) {
					handleGameCommand(wrapper_.game_command());
				}
				if (wrapper_.has_connections()) {
					handleConnections(wrapper_.connections());
				}
			}
		}

		void handleGameCommand(const tp::GameCommand& gameCommand) {
			spdlog::info("[Network] Paused: {}", gameCommand.pause() ? "true" : "false");
			tetrisGame_->pause();
			if (gameCommand.restart()) {
				tetrisGame_->restartGame();
			}
		}

		void handleGameLooby(const tp::GameLooby& gameLooby) {
			
		}

		void handleConnections(const tp::Connections& connections) {
			for (const auto& uuid : connections.uuids()) {
				spdlog::info("[Network] Connected uuid: {}", uuid);
			}
		}

		void setPlayerSlot(const game::PlayerSlot& playerSlot, int index) {
			wrapper_.Clear();
			auto tpGameLooby = wrapper_.mutable_game_looby();

			if (index >= 0 && index < playerSlots_.size()) {
				playerSlots_[index] = playerSlot;
				playerSlotUpdate(playerSlot, index);
				
				for (const auto& slot : playerSlots_) {
					auto tpSlot = tpGameLooby->add_slots();
					toTpSlot(slot, *tpSlot);
				}
				send(wrapper_);
			}
		}

		bool createGame(std::unique_ptr<game::GameRules> gameRules, int w, int h) {
			localPlayers_ = game::PlayerFactory{}.createPlayers(w, h, extractHumans(playerSlots_), extractAis(playerSlots_));
			for (auto& player : localPlayers_) {
				connections_ += player->addPlayerBoardUpdateCallback([this, &player](game::PlayerBoardEvent playerBoardEvent) {
					std::visit([&](auto&& event) {
						handlePlayerBoardUpdate(*player, event);
					}, playerBoardEvent);
				});
				connections_ += player->addEventCallback([this, &player](tetris::BoardEvent boardEvent, int nbr) {
					handleBoardEvent(*player, boardEvent, nbr);
				});
			}

			tetrisGame_->createGame(std::move(gameRules), w, h, localPlayers_, {});

			wrapper_.Clear();
			for (auto& player : localPlayers_) {
				auto tpPlayer = wrapper_.mutable_create_server_game()->add_local_players();
				tpPlayer->set_ai(false);
				tpPlayer->set_level(1);
				tpPlayer->set_points(1);
				tpPlayer->set_name("name");
				tpPlayer->set_uuid("ASDASD");
			}
			send(wrapper_);
			return true;
		}

		const std::string& getServerId() const {
			return serverId_;
		}

		void handlePlayerBoardUpdate(const game::Player& player, const game::UpdateRestart& updateRestart) {
			spdlog::info("[Network] handle UpdateRestart: current={}, next={}", static_cast<char>(updateRestart.current), static_cast<char>(updateRestart.next));
		}

		void handlePlayerBoardUpdate(const game::Player& player, const game::UpdatePlayerData& updatePlayerData) {
			spdlog::info("[Network] handle UpdatePlayerData");
		}

		void handlePlayerBoardUpdate(const game::Player& player, const game::ExternalRows& externalRows) {
			spdlog::info("[Network] handle ExternalRows");
		}

		void handlePlayerBoardUpdate(const game::Player& player, const game::UpdateMove& updateMove) {
			spdlog::info("[Network] handle UpdateMove: {}", static_cast<int>(updateMove.move));
		}

		void handlePlayerBoardUpdate(const game::Player& player, const game::UpdateNextBlock& updateNextBlock) {
			spdlog::info("[Network] handle UpdateNextBlock: {}", static_cast<char>(updateNextBlock.next));
		}

		void handleBoardEvent(const game::Player& player, tetris::BoardEvent boardEvent, int nbr) {
			spdlog::info("[Network] handle UpdateMove: {}, {}", static_cast<char>(boardEvent), nbr);
		}

		void sendPause(bool pause) {
			wrapper_.Clear();
			wrapper_.mutable_game_command()->set_pause(pause);
			send(wrapper_);
		}

	private:
		mw::Signal<game::PlayerSlot, int> playerSlotUpdate;
		
		std::vector<game::PlayerSlot> playerSlots_;
		mw::signals::ScopedConnections connections_;

		std::string serverId_ = "sdfghjklzxcvbnm";
		std::vector<game::PlayerPtr> localPlayers_;
		
		tp::Wrapper wrapper_;
		std::shared_ptr<Client> client_;
		std::shared_ptr<game::TetrisGame> tetrisGame_;
	};

	Network::Network(std::shared_ptr<Client> client, std::shared_ptr<game::TetrisGame> tetrisGame)
		: impl_{std::make_unique<Network::Impl>(std::move(client), std::move(tetrisGame))} {
	}

	const std::string& Network::getServerId() const {
		return impl_->getServerId();
	}

	Network::~Network() {
	}

	void Network::sendPause(bool pause) {
		impl_->sendPause(pause);
	}

	void Network::update() {
		impl_->update();
	}

	void Network::setPlayerSlot(const game::PlayerSlot& playerSlot, int slot) {
		impl_->setPlayerSlot(playerSlot, slot);
	}

	bool Network::createGame(std::unique_ptr<game::GameRules> gameRules, int w, int h) {
		return impl_->createGame(std::move(gameRules), w, h);
	}

}

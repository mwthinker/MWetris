#ifndef MWETRIS_NETWORK_NETWORK_H
#define MWETRIS_NETWORK_NETWORK_H

#include "game/remoteplayerboard.h"

#include "game/remoteplayer.h"
#include "game/player.h"

#include <thread>
#include <memory>
#include <string>

#include <mw/signal.h>

namespace mwetris::network {

	struct Remote {
		int slotIndex;
		game::RemotePlayerPtr remotePlayer;
	};

	class Network {
	public:
		mw::PublicSignal<Network, Remote> remotePlayerConnected;
		mw::PublicSignal<Network, game::RemotePlayerPtr> remotePlayerDisconnected;

		Network();

		void update() {
			while (!addRemotePlayers_.empty()) {
				auto player = addRemotePlayers_.back();
				addRemotePlayers_.pop_back();
				remotePlayerConnected(player);
			}
			while (!removeRemotePlayers_.empty()) {
				auto player = removeRemotePlayers_.back();
				removeRemotePlayers_.pop_back();
				remotePlayerDisconnected(player);
			}
		}

		void addPlayers(std::vector<game::PlayerPtr>& players, const std::vector<game::RemotePlayerPtr>& remotePlayers);

		void removeRemotePlayer(game::RemotePlayerBoardPtr&& remotePlayer);

		const std::string& getServerId() const {
			return serverId_;
		}

		void debugAddRemotePlayer(int slotIndex) {
			addRemotePlayers_.push_back(
				Remote{
					.slotIndex = slotIndex,
					.remotePlayer = std::make_shared<game::RemotePlayer>()
				}
			);
		}

		void debugRemoveRemotePlayer(game::RemotePlayerPtr remotePlayer) {
			removeRemotePlayers_.push_back(remotePlayer);
		}

	private:
		mw::signals::ScopedConnections connections_;

		void handlePlayerBoardUpdate(const game::Player& player, const game::UpdateRestart& updateRestart);
		void handlePlayerBoardUpdate(const game::Player& player, const game::UpdatePlayerData& updatePlayerData);
		void handlePlayerBoardUpdate(const game::Player& player, const game::ExternalRows& externalRows);

		std::vector<game::PlayerPtr> localPlayerse_;
		std::vector<game::RemotePlayerPtr> removeRemotePlayers_;
		std::vector<Remote> addRemotePlayers_;

		void run();

		class Impl;
		//std::unique_ptr<Impl> impl_;
		std::jthread thread_;
		std::string serverId_ = "sdfghjklzxcvbnm";
	};

}

#endif

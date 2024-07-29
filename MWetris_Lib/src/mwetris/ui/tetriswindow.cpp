#include "tetriswindow.h"
#include "tetriscontroller.h"
#include "imguiextra.h"
#include "configuration.h"

#include "scene/about.h"
#include "scene/creategameroom.h"
#include "scene/settings.h"
#include "scene/highscore.h"
#include "scene/newhighscore.h"
#include "scene/addplayer.h"
#include "scene/resume.h"
#include "scene/play.h"
#include "scene/joingameroom.h"
#include "scene/gameroomlobby.h"
#include "game/serialize.h"
#include "game/tetrisgame.h"
#include "graphic/gamecomponent.h"
#include "network/client.h"
#include "network/network.h"
#include "util/auxiliary.h"

#include <sdl/imguiauxiliary.h>

#include <spdlog/spdlog.h>

#include <chrono>

using namespace std::chrono_literals;

namespace mwetris::ui {

	namespace {

		constexpr ImGuiWindowFlags ImguiMainWindow
			= ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoScrollWithMouse
			| ImGuiWindowFlags_MenuBar;

		constexpr ImGuiWindowFlags ImguiSecondaryWindow
			= ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoDocking
			| ImGuiWindowFlags_NoScrollbar
			| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoScrollWithMouse
			| ImGuiWindowFlags_MenuBar;
		
		constexpr auto DefaultRefreshRate = 30;
		
		constexpr auto CheckRefreshRateInterval = 30s;

		int acceptNameInput(ImGuiInputTextCallbackData* data) {
			return data->BufTextLen < 30;
		}

		void MainWindow(const SubWindow& subWindow, std::invocable auto&& t) {
			if (subWindow.getType() == TetrisWindow::Type::MainWindow) {
				ImGui::MainWindow(subWindow.getName().c_str(), ImguiMainWindow, t);
			} else {
				ImGui::SetNextWindowSize({400, 600});
				ImGui::Window(subWindow.getName().c_str(), nullptr, ImguiSecondaryWindow, t);
			}
		}
	}

	TetrisWindow::TetrisWindow(const std::string& windowName, Type type, sdl::Window& window,
		std::shared_ptr<game::DeviceManager> deviceManager,
		std::shared_ptr<network::Network> network)
		: window_{window}
		, deviceManager_{deviceManager}
		, type_{type}
		, windowName_{windowName}
		, modalStateMachine_{ui::scene::StateMachine::StateType::Modal} {

		tetrisController_ = std::make_shared<TetrisController>(deviceManager_, network, std::make_shared<graphic::GameComponent>());

		connections_ += deviceManager_->deviceConnected.connect([](game::DevicePtr device) {
			spdlog::info("Device found: {}", device->getName());
		});

		pauseMenuText_ = "Pause";

		initPreLoop();
	}

	TetrisWindow::~TetrisWindow() {
		Configuration::getInstance().quit();

		if (type_ == Type::MainWindow) {
			tetrisController_->saveDefaultGame();
		}
	}

	int TetrisWindow::getCurrentMonitorHz() const {
		if (SDL_DisplayMode displayMode; SDL_GetCurrentDisplayMode(SDL_GetWindowDisplayIndex(window_.getSdlWindow()), &displayMode) == 0) {
			if (displayMode.refresh_rate > 0) {
				spdlog::info("[TetrisWindow] Window {}Hz", displayMode.refresh_rate);
				return displayMode.refresh_rate;
			} else {
				spdlog::info("[TetrisWindow] Window Hz is unspecified");
			}
		} else {
			spdlog::warn("[TetrisWindow] Could not extract Window Hz, uses instead {}. Error: {}", DefaultRefreshRate, SDL_GetError());
		}
		return DefaultRefreshRate;
	}

	void TetrisWindow::initPreLoop() {
		Configuration::getInstance().bindTextureFromAtlas();
		background_ = Configuration::getInstance().getBackgroundSprite();

		mainStateMachine_.emplace<scene::GameRoomLooby>(tetrisController_, deviceManager_);
		mainStateMachine_.emplace<scene::Resume>(tetrisController_, deviceManager_);
		mainStateMachine_.emplace<scene::Play>(tetrisController_);
		
		modalStateMachine_.emplace<scene::JoinGameRoom>(tetrisController_);
		modalStateMachine_.emplace<scene::CreateGameRoom>(tetrisController_);
		modalStateMachine_.emplace<scene::Settings>();
		modalStateMachine_.emplace<scene::HighScore>();
		modalStateMachine_.emplace<scene::NewHighScore>([this]() {
			modalStateMachine_.switchTo<scene::HighScore>();
		});
		modalStateMachine_.emplace<scene::About>();

		connections_ += tetrisController_->tetrisEvent.connect([this](const TetrisEvent& tetrisEvent) {
			std::visit([&](auto&& event) {
				onTetrisEvent(event);
			}, tetrisEvent);
		});

		// Keep the update loop in sync with monitor.
		tetrisController_->setFixTimestep(1.0 / getCurrentMonitorHz());
		timeHandler_.scheduleRepeat([this]() {
			tetrisController_->setFixTimestep(1.0 / getCurrentMonitorHz());
		}, util::toSeconds(CheckRefreshRateInterval), std::numeric_limits<int>::max());

		tetrisController_->createDefaultGame(deviceManager_->getDefaultDevice1());
		tetrisController_->pause();
	}

	void TetrisWindow::onTetrisEvent(const game::GamePause& gamePause) {
		if (gamePause.pause) {
			pauseMenuText_ = gamePause.countDown > 0 ? "Stop Countdown" : "Unpause";
		} else {
			pauseMenuText_ = "Pause";
		}
	}

	void TetrisWindow::onTetrisEvent(const game::GameOver& gameOver) {
		game::DefaultPlayerData data{};
		if (const auto playerData{std::get_if<game::DefaultPlayerData>(&gameOver.player->getPlayerData())}; playerData) {
			data = *playerData;
		}

		if (const auto& player = gameOver.player;  tetrisController_->isDefaultGame() && game::isNewHighScore(data.points)) {
			scene::NewHighScoreData data;
			data.name = "Name"; // TODO! Get name from player
			data.points = data.points;
			data.clearedRows = player->getClearedRows();
			data.level = data.level;
			modalStateMachine_.switchTo<scene::NewHighScore>(data);
		}
	}

	void TetrisWindow::onTetrisEvent(const PlayerSlotEvent& playerSlotEvent) {
		spdlog::debug("[TetrisWindow] Player slot event");
	}

	void TetrisWindow::onTetrisEvent(const mwetris::GameRoomEvent& createGameRoomEvent) {
		scene::GameRoomLoobyData data;
		switch (createGameRoomEvent.type) {
			case GameRoomType::LocalGameRoomLooby:
				data.type = scene::GameRoomLoobyData::Type::Network;
				modalStateMachine_.switchTo<scene::EmptyScene>();
				mainStateMachine_.switchTo<scene::GameRoomLooby>(data);
				break;
			case GameRoomType::NetworkGameRoomLooby:
				data.type = scene::GameRoomLoobyData::Type::Network;
				modalStateMachine_.switchTo<scene::EmptyScene>();
				mainStateMachine_.switchTo<scene::GameRoomLooby>(data);
				break;
			case GameRoomType::OutsideGameRoom:
				modalStateMachine_.switchTo<scene::EmptyScene>();
				mainStateMachine_.switchTo<scene::Resume>();
				break;
			case GameRoomType::GameSession:
				modalStateMachine_.switchTo<scene::EmptyScene>();
				mainStateMachine_.switchTo<scene::Play>();
				break;
			case GameRoomType::NetworkWaitingCreateGameRoom:
				break;
		}
	}

	void TetrisWindow::onTetrisEvent(const network::GameRoomListEvent& gameRoomListEvent) {
		// Do nothing here.
	}

	void TetrisWindow::onTetrisEvent(const game::GameRoomConfigEvent& gameRoomConfigEvent) {
		// Do nothing here.
	}

	void TetrisWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
		auto deltaTimeSeconds = util::toSeconds(deltaTime);
		tetrisController_->update(deltaTimeSeconds);
		timeHandler_.update(deltaTimeSeconds);

		ImGui::PushFont(Configuration::getInstance().getImGuiDefaultFont());
		imGuiMainWindow(deltaTime);

		modalStateMachine_.imGuiUpdate(deltaTime);

		ImGui::PopFont();
	}

	void TetrisWindow::imGuiMainWindow(const sdl::DeltaTime& deltaTime) {
		MainWindow(*this, [&]() {
			ImGui::ImageBackground(background_);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0, 10.0});
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 10.f);
			ImGui::MenuBar([&]() {
				ImGui::Menu("Game", [&]() {
					bool insideGameRoom = tetrisController_->isGameRoomSession();

					ImGui::BeginDisabled(insideGameRoom);
					if (ImGui::MenuItem(game::hasSavedGame() ? "Resume Single Player" : "New Single Player", "F1")) {
						tetrisController_->createDefaultGame(deviceManager_->getDefaultDevice1());
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Create Local Game")) {
						tetrisController_->createLocalGameRoom();
					}
					if (ImGui::MenuItem("Create Network Game")) {
						modalStateMachine_.switchTo<scene::CreateGameRoom>();
					}
					if (ImGui::MenuItem("Join Network Game")) {
						modalStateMachine_.switchTo<scene::JoinGameRoom>();
					}
					ImGui::EndDisabled();
					ImGui::BeginDisabled(!insideGameRoom);
					if (ImGui::MenuItem("Leave game")) {
						tetrisController_->leaveGameRoom();
					}
					ImGui::EndDisabled();
					ImGui::Separator();
					if (ImGui::MenuItem("Highscore")) {
						tetrisController_->pause();
						modalStateMachine_.switchTo<scene::HighScore>();
					}
					if (ImGui::MenuItem("Preferences")) {
						modalStateMachine_.switchTo<scene::Settings>();
					}
					if (ImGui::MenuItem("Quit", "ESQ")) {
						window_.quit();
					}
				});
				ImGui::Menu("Current", [&]() {
					if (ImGui::MenuItem(pauseMenuText_.c_str(), "P")) {
						tetrisController_->pause();
					}
					if (ImGui::MenuItem("Restart", "F5")) {
						tetrisController_->restartGame();
					}
				});
				ImGui::Menu("Help", [&]() {
					if (ImGui::MenuItem("About")) {
						modalStateMachine_.switchTo<scene::About>();
					}
				});
			});
			ImGui::PopStyleVar(2);
			
			mainStateMachine_.imGuiUpdate(deltaTime);
		});
	}

	void TetrisWindow::imGuiEventUpdate(const SDL_Event& windowEvent) {
		if (type_ == Type::SecondaryWindow) {
			return;
		}

		switch (windowEvent.type) {
			case SDL_KEYDOWN:
				switch (windowEvent.key.keysym.sym) {
					case SDLK_F1:
						tetrisController_->createDefaultGame(deviceManager_->getDefaultDevice1());
						break;
					case SDLK_F5:
						tetrisController_->restartGame();
						break;
					case SDLK_p: [[fallthrough]];
					case SDLK_PAUSE:
						tetrisController_->pause();
						break;
				}
				break;
		}
	}

	const std::string& TetrisWindow::getName() const {
		return windowName_;
	}

	SubWindow::Type TetrisWindow::getType() const {
		return type_;
	}

}

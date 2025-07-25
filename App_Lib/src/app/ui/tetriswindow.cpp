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
#include "scene/networkerror.h"
#include "game/serialize.h"
#include "game/tetrisgame.h"
#include "graphic/gamecomponent.h"
#include "cnetwork/network.h"
#include "util/auxiliary.h"

#include <network/client.h>

#include <IconsFontAwesome6.h>
#include <spdlog/spdlog.h>

#include <chrono>

using namespace std::chrono_literals;

namespace app::ui {

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

		void addImageQuad(const app::TextureView& texture,
			const glm::vec2& pos, const glm::vec2& size, sdl::Color color = sdl::color::White) {

			ImVec2 a = pos;
			ImVec2 b = pos + glm::vec2{size.x, 0.f};
			ImVec2 c = pos + glm::vec2{size.x, size.y};
			ImVec2 d = pos + glm::vec2{0.f, size.y};

			ImVec2 uv_c{texture.pos.x + texture.size.x, texture.pos.y + texture.size.y};
			ImVec2 uv_d{texture.pos.x, texture.pos.y + texture.size.y};
			ImVec2 uv_a{texture.pos.x, texture.pos.y};
			ImVec2 uv_b{texture.pos.x + texture.size.x, texture.pos.y};

			ImGui::GetWindowDrawList()->PrimQuadUV(a, b, c, d, uv_a, uv_b, uv_c, uv_d, color.toImU32());
		}

		void imageBackground(const app::TextureView& texture) {
			auto drawList = ImGui::GetWindowDrawList();
			drawList->PushTexture(Configuration::getInstance().getTextureAtlasBinding());
			drawList->PrimReserve(6, 4);
			addImageQuad(texture, ImGui::GetCursorScreenPos(), ImGui::GetWindowSize());
			drawList->PopTexture();
		}
	}

	TetrisWindow::TetrisWindow(const std::string& windowName, Type type, sdl::Window& window,
		std::shared_ptr<game::DeviceManager> deviceManager,
		std::shared_ptr<cnetwork::Network> network)
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
		if (auto displayMode = SDL_GetCurrentDisplayMode(SDL_GetDisplayForWindow(window_.getSdlWindow())); displayMode != nullptr) {
			if (displayMode->refresh_rate > 0) {
				spdlog::info("[TetrisWindow] Window {}Hz", displayMode->refresh_rate);
				return displayMode->refresh_rate;
			} else {
				spdlog::info("[TetrisWindow] Window Hz is unspecified");
			}
		} else {
			spdlog::warn("[TetrisWindow] Could not extract Window Hz, uses instead {}. Error: {}", DefaultRefreshRate, SDL_GetError());
		}
		return DefaultRefreshRate;
	}

	void TetrisWindow::initPreLoop() {
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
		modalStateMachine_.emplace<scene::NetworkError>();

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

		ImGui::GetStyle().Colors[ImGuiCol_Button] = Color{"#000000"};
		ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = Color{"#4D4D4D"};
		ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = Color{"#292929"};
		ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = Color{"#1FFF00"};
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

	void TetrisWindow::onTetrisEvent(const app::GameRoomEvent& createGameRoomEvent) {
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

	void TetrisWindow::onTetrisEvent(const cnetwork::GameRoomListEvent& gameRoomListEvent) {
		// Do nothing here.
	}

	void TetrisWindow::onTetrisEvent(const game::GameRoomConfigEvent& gameRoomConfigEvent) {
		// Do nothing here.
	}

	void TetrisWindow::onTetrisEvent(const cnetwork::NetworkErrorEvent& networkErrorEvent) {
		if (modalStateMachine_.isCurrentScene<scene::JoinGameRoom>() ||
			modalStateMachine_.isCurrentScene<scene::CreateGameRoom>() ||
			networkErrorEvent.insideGameRoom) {
			
			// Show only error popup when it is of importance for the user.
			modalStateMachine_.switchTo<scene::NetworkError>();
		}
	}

	void TetrisWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
		auto deltaTimeSeconds = util::toSeconds(deltaTime);
		tetrisController_->update(deltaTimeSeconds);
		timeHandler_.update(deltaTimeSeconds);

		ImGui::PushFont(Configuration::getInstance().getImGuiDefaultFont(), 0.f);
		imGuiMainWindow(deltaTime);

		modalStateMachine_.imGuiUpdate(deltaTime);

		ImGui::PopFont();
	}

	void TetrisWindow::imGuiMainWindow(const sdl::DeltaTime& deltaTime) {
		MainWindow(*this, [&]() {
			/*
			ImGui::Window("Style Editor", [&]() {
				ImGui::ShowStyleEditor();
			});
			*/

			imageBackground(background_);

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
					ImGui::BeginDisabled(!tetrisController_->isConnectedToServer());
					if (ImGui::MenuItem("Create Network Game")) {
						modalStateMachine_.switchTo<scene::CreateGameRoom>();
					}
					if (ImGui::MenuItem("Join Network Game")) {
						modalStateMachine_.switchTo<scene::JoinGameRoom>();
					}
					ImGui::EndDisabled();
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
					ImGui::Menu("Player1", [&]() {
						ImGui::TextUnformatted("Move: Arrow keys");
						ImGui::ComboScoped("Device", "Keybard1", [&]() {
							ImGui::Selectable("Keyboard1", true, ImGuiSelectableFlags_AllowDoubleClick, ImVec2{0, 0});
							ImGui::Selectable("Keyboard2", false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2{0, 0});
						});
						static int das = 60;
						ImGui::SeparatorText("DAS/ARR");
						ImGui::Button("Default"); 
						ImGui::SameLine();
						ImGui::Button("Save");
						ImGui::SliderInt("DAS", &das, 0, 1000, "%d MS", ImGuiSliderFlags_AlwaysClamp);
						static int arr = 100;
						ImGui::SliderInt("ARR", &arr, 0, 1000, "%d MS", ImGuiSliderFlags_AlwaysClamp);
						ImGui::Separator();
					});

				});
				ImGui::PushStyleColor(ImGuiCol_Text, tetrisController_->isConnectedToServer() ? sdl::color::Green : sdl::color::Red);
				ImGui::Text(ICON_FA_GLOBE);
				ImGui::PopStyleColor();
				if (ImGui::IsItemHovered()) {
					ImGui::Tooltip([&]() {
						ImGui::Text(tetrisController_->isConnectedToServer()
							? "Connected to game server"
							: "Not connected to game server"
						);
					});
				}
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
			case SDL_EVENT_KEY_DOWN:
				switch (windowEvent.key.key) {
					case SDLK_F1:
						tetrisController_->createDefaultGame(deviceManager_->getDefaultDevice1());
						break;
					case SDLK_F5:
						tetrisController_->restartGame();
						break;
					case SDLK_P: [[fallthrough]];
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

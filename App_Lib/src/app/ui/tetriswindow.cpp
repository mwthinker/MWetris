#include "tetriswindow.h"
#include "tetriscontroller.h"
#include "imguiextra.h"
#include "controlspresenter.h"
#include "configuration.h"
#include "theme.h"

#include "scene/about.h"
#include "scene/creategameroom.h"
#include "scene/controls.h"
#include "scene/help.h"
#include "scene/settings.h"
#include "scene/highscore.h"
#include "scene/newhighscore.h"
#include "scene/addplayer.h"
#include "scene/gamelauncher.h"
#include "scene/play.h"
#include "scene/joingameroom.h"
#include "scene/leavegame.h"
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
#include <string>
#include <utility>

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
		std::shared_ptr<ControlProfiles> controlProfiles,
		std::shared_ptr<cnetwork::Network> network,
		std::function<void()> requestRestart)
		: window_{window}
		, deviceManager_{deviceManager}
		, controlProfiles_{std::move(controlProfiles)}
		, controlsPresenter_{std::make_shared<ControlsPresenter>(windowName, deviceManager, controlProfiles_)}
		, requestRestart_{std::move(requestRestart)}
		, modalStateMachine_{ui::scene::StateMachine::StateType::Modal}
		, singlePlayerControls_{Configuration::getInstance().getLastSinglePlayerControls()}
		, type_{type}
		, windowName_{windowName} {

		auto gameComponent = std::make_shared<graphic::GameComponent>(
			[presenter = controlsPresenter_](const game::PlayerPtr& player, float width) {
				presenter->drawDeviceProfileSelection(player, width);
			});
		tetrisController_ = std::make_shared<TetrisController>(deviceManager_, controlProfiles_, network, std::move(gameComponent));

		connections_ += deviceManager_->deviceConnected.connect([](game::DevicePtr device) {
			spdlog::info("Device found: {}", device->getName());
		});

		pauseMenuText_ = "Pause";

		initPreLoop();
	}

	TetrisWindow::~TetrisWindow() {
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

		mainStateMachine_.emplace<scene::GameRoomLooby>(tetrisController_, deviceManager_, controlProfiles_);
		mainStateMachine_.emplace<scene::GameLauncher>(tetrisController_,
			[this]() { startSinglePlayer(); },
			[this]() { modalStateMachine_.switchTo<scene::CreateGameRoom>(); },
			[this]() { modalStateMachine_.switchTo<scene::JoinGameRoom>(); });
		mainStateMachine_.emplace<scene::Play>(tetrisController_);
		
		modalStateMachine_.emplace<scene::JoinGameRoom>(tetrisController_);
		modalStateMachine_.emplace<scene::CreateGameRoom>(tetrisController_);
		modalStateMachine_.emplace<scene::Settings>(controlsPresenter_, requestRestart_);
		modalStateMachine_.emplace<scene::Controls>(controlsPresenter_);
		modalStateMachine_.emplace<scene::HighScore>();
		modalStateMachine_.emplace<scene::NewHighScore>([this]() {
			modalStateMachine_.switchTo<scene::HighScore>();
		});
		modalStateMachine_.emplace<scene::About>();
		modalStateMachine_.emplace<scene::HowToPlay>();
		modalStateMachine_.emplace<scene::KeyboardShortcuts>();
		modalStateMachine_.emplace<scene::NetworkError>();
		modalStateMachine_.emplace<scene::LeaveGame>(tetrisController_, [this]() { window_.quit(); });

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

		mainStateMachine_.switchTo<scene::GameLauncher>();

		refreshSettings();
		connections_ += Configuration::getInstance().settingsChanged.connect([this](const auto&, const auto&) {
			refreshSettings();
		});
	}

	void TetrisWindow::refreshSettings() {
		auto& configuration = Configuration::getInstance();
		auto& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Button] = configuration.getButtonBackgroundColor();
		style.Colors[ImGuiCol_ButtonHovered] = configuration.getButtonHoverColor();
		style.Colors[ImGuiCol_ButtonActive] = configuration.getButtonFocusColor();
		style.Colors[ImGuiCol_Header] = configuration.getButtonBackgroundColor();
		style.Colors[ImGuiCol_HeaderHovered] = configuration.getButtonHoverColor();
		auto menuActiveColor = static_cast<ImVec4>(configuration.getButtonHoverColor());
		menuActiveColor.w *= 0.45f;
		style.Colors[ImGuiCol_HeaderActive] = menuActiveColor;
		style.Colors[ImGuiCol_FrameBg] = configuration.getComboBoxBackgroundColor();
		style.Colors[ImGuiCol_FrameBgHovered] = configuration.getComboBoxFocusColor();
		style.Colors[ImGuiCol_FrameBgActive] = configuration.getComboBoxFocusColor();
		style.Colors[ImGuiCol_Border] = configuration.getComboBoxBorderColor();
		style.Colors[ImGuiCol_CheckMark] = configuration.getChecboxCheckColor();
		style.FrameBorderSize = 1.f;
	}

	game::DevicePtr TetrisWindow::getSinglePlayerDevice() const {
		auto device = deviceManager_->findDevice(singlePlayerControls_.deviceGuid);
		if (device && device->isConnected()) {
			return device;
		}
		auto devices = deviceManager_->getAllDevicesAvailable();
		return devices.empty() ? nullptr : devices.front();
	}

	void TetrisWindow::startSinglePlayer() {
		auto device = getSinglePlayerDevice();
		if (!device) {
			return;
		}
		auto profileId = singlePlayerControls_.profileId;
		if (!controlProfiles_->getProfile(profileId)) {
			profileId = controlProfiles_->getAssignedProfileId(device->getGuid());
		}
		tetrisController_->createDefaultGame(std::move(device), profileId, singlePlayerControls_.startLevel);
		singlePlayerControls_.startLevel = tetrisController_->getDefaultStartLevel();
		rememberSinglePlayerControls();
	}

	void TetrisWindow::rememberSinglePlayerControls() {
		auto player = tetrisController_->getDefaultPlayer();
		if (!player || !player->getDevice() || !player->getProfileId()) {
			return;
		}
		Configuration::SinglePlayerControls controls{
			.deviceGuid = player->getDevice()->getGuid(),
			.profileId = *player->getProfileId(),
			.startLevel = singlePlayerControls_.startLevel,
		};
		if (controls == singlePlayerControls_) {
			return;
		}
		singlePlayerControls_ = controls;
		if (auto result = Configuration::getInstance().persistLastSinglePlayerControls(controls); !result) {
			spdlog::warn("Could not remember the single-player controls: {}", result.error());
		}
	}

	void TetrisWindow::pauseOnFocusLost() {
		if (!tetrisController_->isPaused()) {
			tetrisController_->pause();
		}
	}

	void TetrisWindow::onTetrisEvent(const game::GamePause& gamePause) {
		if (gamePause.pause) {
			pauseMenuText_ = gamePause.countDown > 0 ? "Stop Countdown" : "Unpause";
		} else {
			pauseMenuText_ = "Pause";
		}
	}

	void TetrisWindow::onTetrisEvent(const game::GameOver& gameOver) {
		game::DefaultPlayerData playerData{};
		if (const auto data{std::get_if<game::DefaultPlayerData>(&gameOver.player->getPlayerData())}; data) {
			playerData = *data;
		}

		if (const auto& player = gameOver.player;  tetrisController_->isDefaultGame() && game::isNewHighScore(playerData.points)) {
			scene::NewHighScoreData highScoreData;
			highScoreData.name = "Name"; // TODO! Get name from player
			highScoreData.points = playerData.points;
			highScoreData.clearedRows = player->getClearedRows();
			highScoreData.level = playerData.level;
			highScoreData.startLevel = playerData.startLevel;
			modalStateMachine_.switchTo<scene::NewHighScore>(highScoreData);
		}
	}

	void TetrisWindow::onTetrisEvent(const PlayerSlotEvent& playerSlotEvent) {
		spdlog::debug("[TetrisWindow] Player slot event");
	}

	void TetrisWindow::onTetrisEvent(const app::GameRoomEvent& createGameRoomEvent) {
		scene::GameRoomLoobyData data;
		switch (createGameRoomEvent.type) {
			case GameRoomType::LocalGameRoomLooby:
				data.type = scene::GameRoomLoobyData::Type::Local;
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
				mainStateMachine_.switchTo<scene::GameLauncher>();
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
				auto menu = [&](const char* label, std::invocable auto&& content) {
					ImGui::PushStyleColor(ImGuiCol_Border,
						Configuration::getInstance().getButtonHoverColor().toImU32());
					ImGui::PushFont(Configuration::getInstance().getImGuiMediumFont(), 0.f);
					bool open = ImGui::BeginMenu(label);
					ImGui::PopFont();
					if (open) {
						content();
						ImGui::EndMenu();
					}
					ImGui::PopStyleColor();
				};
				menu("Game", [&]() {
					const bool insideGameRoom = tetrisController_->isGameRoomSession();
					ImGui::BeginDisabled(insideGameRoom);
					if (ImGui::MenuItem(game::hasSavedGame() ? "Resume Single Player" : "New Single Player", "F1")) {
						startSinglePlayer();
					}
					if (ImGui::MenuItem("Create Local Game")) {
						tetrisController_->createLocalGameRoom();
					}
					ImGui::BeginDisabled(!tetrisController_->isConnectedToServer());
					if (ImGui::BeginMenu("Online")) {
						if (ImGui::MenuItem("Create Network Game")) {
							modalStateMachine_.switchTo<scene::CreateGameRoom>();
						}
						if (ImGui::MenuItem("Join Network Game")) {
							modalStateMachine_.switchTo<scene::JoinGameRoom>();
						}
						ImGui::EndMenu();
					}
					ImGui::EndDisabled();
					ImGui::EndDisabled();
					if (ImGui::BeginMenu("Single Player Starting Level")) {
						int startLevel{singlePlayerControls_.startLevel};
						ImGui::SetNextItemWidth(180.f);
						if (ImGui::SliderInt("##SinglePlayerStartLevel", &startLevel, 0, game::TetrisMaxLevel, "Level %d")) {
							singlePlayerControls_.startLevel = game::clampStartLevel(startLevel);
						}
						if (ImGui::IsItemDeactivatedAfterEdit()) {
							if (tetrisController_->isDefaultGame()) {
								tetrisController_->setDefaultStartLevel(singlePlayerControls_.startLevel);
							}
							if (auto result = Configuration::getInstance().persistLastSinglePlayerControls(singlePlayerControls_); !result) {
								spdlog::warn("Could not remember the single-player starting level: {}", result.error());
							}
						}
						ImGui::TextDisabled("Applies to the next new game or restart.");
						ImGui::EndMenu();
					}
					ImGui::Separator();
					ImGui::BeginDisabled(!insideGameRoom);
					if (ImGui::MenuItem(pauseMenuText_.c_str(), "P")) {
						tetrisController_->pause();
					}
					if (ImGui::MenuItem("Restart", "F5")) {
						tetrisController_->restartGame();
					}
					if (ImGui::MenuItem("Leave Game")) {
						modalStateMachine_.switchTo<scene::LeaveGame>();
					}
					ImGui::EndDisabled();
					ImGui::Separator();
					if (ImGui::MenuItem("High Scores")) {
						modalStateMachine_.switchTo<scene::HighScore>();
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Quit")) {
						if (insideGameRoom) {
							scene::LeaveGameData data;
							data.quitApplication = true;
							modalStateMachine_.switchTo<scene::LeaveGame>(data);
						} else {
							window_.quit();
						}
					}
				});
				menu("Settings", [&]() {
					if (ImGui::MenuItem("Preferences...")) {
						modalStateMachine_.switchTo<scene::Settings>();
					}
					if (ImGui::MenuItem("Controls...")) {
						modalStateMachine_.switchTo<scene::Controls>();
					}
				});
				menu("Help", [&]() {
					if (ImGui::MenuItem("How to Play...")) {
						modalStateMachine_.switchTo<scene::HowToPlay>();
					}
					if (ImGui::MenuItem("Keyboard Shortcuts...")) {
						modalStateMachine_.switchTo<scene::KeyboardShortcuts>();
					}
					ImGui::Separator();
					if (ImGui::MenuItem("About...")) {
						modalStateMachine_.switchTo<scene::About>();
					}
				});
				ImGui::PushStyleColor(ImGuiCol_Text,
					tetrisController_->isConnectedToServer() ? theme::Success.toImU32() : theme::Danger.toImU32());
				ImGui::Text(ICON_FA_GLOBE);
				ImGui::PopStyleColor();
				if (ImGui::IsItemHovered()) {
					ImGui::Tooltip([&]() {
						ImGui::PushFont(Configuration::getInstance().getImGuiRegularSmallFont(), 0.f);
						ImGui::Text(tetrisController_->isConnectedToServer()
							? "Connected to game server"
							: "Not connected to game server"
						);
						ImGui::PopFont();
					});
				}
			});
			ImGui::PopStyleVar(2);
			controlsPresenter_->drawProfileDialog();
			controlsPresenter_->drawBindingDialog();
			
			if (mainStateMachine_.isCurrentScene<scene::Play>()) {
				mainStateMachine_.imGuiUpdate(deltaTime);
			} else {
				ImGui::PushContentStyle();
				if (ImGui::BeginChild("Page content", {0.f, 0.f}, ImGuiChildFlags_AlwaysUseWindowPadding)) {
					mainStateMachine_.imGuiUpdate(deltaTime);
				}
				ImGui::EndChild();
				ImGui::PopContentStyle();
			}
		});
		rememberSinglePlayerControls();
	}

	void TetrisWindow::imGuiEventUpdate(const SDL_Event& windowEvent) {
		if (type_ == Type::SecondaryWindow) {
			return;
		}

		switch (windowEvent.type) {
			case SDL_EVENT_KEY_DOWN:
				switch (windowEvent.key.key) {
					case SDLK_F1:
						if (!tetrisController_->isGameRoomSession()
							&& modalStateMachine_.isCurrentScene<scene::EmptyScene>()) {
							startSinglePlayer();
						}
						break;
					case SDLK_F5:
						if (tetrisController_->isGameRoomSession()
							&& modalStateMachine_.isCurrentScene<scene::EmptyScene>()) {
							tetrisController_->restartGame();
						}
						break;
					case SDLK_P: [[fallthrough]];
					case SDLK_PAUSE:
						if (tetrisController_->isGameRoomSession()
							&& modalStateMachine_.isCurrentScene<scene::EmptyScene>()) {
							tetrisController_->pause();
						}
						break;
					case SDLK_ESCAPE:
						if (tetrisController_->isGameRoomSession()
							&& modalStateMachine_.isCurrentScene<scene::EmptyScene>()) {
							tetrisController_->pause();
						}
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

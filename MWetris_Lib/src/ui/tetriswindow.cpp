#include "tetriswindow.h"
#include "imguiextra.h"
#include "configuration.h"

#include "scene/about.h"
#include "scene/settings.h"
#include "scene/highscore.h"
#include "scene/newhighscore.h"
#include "scene/addplayer.h"
#include "scene/joingame.h"

#include "game/serialize.h"
#include "game/tetrisgame.h"

#include "graphic/gamecomponent.h"

#include <sdl/imguiauxiliary.h>

#include <spdlog/spdlog.h>

#include <chrono>

using namespace std::chrono_literals;

namespace mwetris::ui {

	namespace {

		template<class>
		inline constexpr bool always_false_v = false;

		constexpr auto DefaultRefreshRate = 30;

		constexpr auto CheckRefreshRateInterval = 30s;

		int acceptNameInput(ImGuiInputTextCallbackData* data) {
			return data->BufTextLen < 30;
		}
		
		constexpr double toSeconds(const auto& duration) {
			return std::chrono::duration<double>(duration).count();
		}

		void reset(std::vector<scene::PlayerSlot>& slots) {
			for (auto& slot : slots) {
				slot = scene::OpenSlot{};
			}
		}

		int playersInSlots(const std::vector<scene::PlayerSlot>& playerSlots) {
			int nbr = 0;
			for (const auto& playerSlot : playerSlots) {
				std::visit([&](auto&& slot) mutable {
					using T = std::decay_t<decltype(slot)>;
					if constexpr (std::is_same_v<T, scene::DeviceType>) {
						++nbr;
					} else if constexpr (std::is_same_v<T, scene::AiType>) {
						++nbr;
					} else if constexpr (std::is_same_v<T, scene::InternetPlayer>) {
						++nbr;
					} else if constexpr (std::is_same_v<T, scene::OpenSlot>) {
						// Skip.
					} else {
						static_assert(always_false_v<T>, "non-exhaustive visitor!");
					}
				}, playerSlot);
			}
			return nbr;
		}

		int internetPlayersInSlots(const std::vector<scene::PlayerSlot>& playerSlots) {
			int nbr = 0;
			for (const auto& playerSlot : playerSlots) {
				std::visit([&](auto&& slot) mutable {
					using T = std::decay_t<decltype(slot)>;
					if constexpr (std::is_same_v<T, scene::DeviceType>) {
						// Skip.
					} else if constexpr (std::is_same_v<T, scene::AiType>) {
						// Skip.
					} else if constexpr (std::is_same_v<T, scene::InternetPlayer>) {
						++nbr;
					} else if constexpr (std::is_same_v<T, scene::OpenSlot>) {
						// Skip.
					} else {
						static_assert(always_false_v<T>, "non-exhaustive visitor!");
					}
				}, playerSlot);
			}
			return nbr;
		}

		std::vector<game::Human> extractHumans(const std::vector<scene::PlayerSlot>& playerSlots) {
			std::vector<game::Human> humans;
			for (const auto& playerSlot : playerSlots) {
				std::visit([&](auto&& slot) mutable {
					using T = std::decay_t<decltype(slot)>;
					if constexpr (std::is_same_v<T, scene::DeviceType>) {
						humans.push_back(game::Human{.name = slot.name, .device = slot.device});
					} else if constexpr (std::is_same_v<T, scene::AiType>) {
						// Skip
					} else if constexpr (std::is_same_v<T, scene::InternetPlayer>) {
						// Skip
					} else if constexpr (std::is_same_v<T, scene::OpenSlot>) {
						// Skip.
					} else {
						static_assert(always_false_v<T>, "non-exhaustive visitor!");
					}
				}, playerSlot);
			}
			return humans;
		}

		std::vector<game::Ai> extractAis(const std::vector<scene::PlayerSlot>& playerSlots) {
			std::vector<game::Ai> ais;
			for (const auto& playerSlot : playerSlots) {
				std::visit([&](auto&& slot) mutable {
					using T = std::decay_t<decltype(slot)>;
					if constexpr (std::is_same_v<T, scene::DeviceType>) {
						// Skip
					} else if constexpr (std::is_same_v<T, scene::AiType>) {
						ais.push_back(game::Ai{.name = slot.name, .ai = slot.ai});
					} else if constexpr (std::is_same_v<T, scene::InternetPlayer>) {
						// Skip
					} else if constexpr (std::is_same_v<T, scene::OpenSlot>) {
						// Skip.
					} else {
						static_assert(always_false_v<T>, "non-exhaustive visitor!");
					}
				}, playerSlot);
			}
			return ais;
		}

		std::vector<game::RemotePlayerPtr> extractRemotePlayers(const std::vector<scene::PlayerSlot>& playerSlots) {
			std::vector<game::RemotePlayerPtr> remotePlayers;
			for (const auto& playerSlot : playerSlots) {
				std::visit([&](auto&& slot) mutable {
					using T = std::decay_t<decltype(slot)>;
					if constexpr (std::is_same_v<T, scene::DeviceType>) {
						// Skip
					} else if constexpr (std::is_same_v<T, scene::AiType>) {
						// Skip
					} else if constexpr (std::is_same_v<T, scene::InternetPlayer>) {
						if (slot.remotePlayer) {
							remotePlayers.push_back(slot.remotePlayer);
						}
					} else if constexpr (std::is_same_v<T, scene::OpenSlot>) {
						// Skip.
					} else {
						static_assert(always_false_v<T>, "non-exhaustive visitor!");
					}
				}, playerSlot);
			}
			return remotePlayers;
		}

		bool removeRemotePlayer(std::vector<scene::PlayerSlot>& playerSlots, const game::RemotePlayerPtr& remotePlayer) {
			for (auto& slot : playerSlots) {
				if (auto internetPlayer{std::get_if<scene::InternetPlayer>(&slot)}; internetPlayer && internetPlayer->remotePlayer == remotePlayer) {
					internetPlayer->remotePlayer = nullptr;
					return true;
				}
			}
			return false;
		}

	}

	constexpr ImGuiWindowFlags ImguiNoWindow
		= ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoDocking
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoBackground
		| ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_MenuBar;

	TetrisWindow::TetrisWindow() {
		setPosition(Configuration::getInstance().getWindowPositionX(), Configuration::getInstance().getWindowPositionY());
		setSize(Configuration::getInstance().getWindowWidth(), Configuration::getInstance().getWindowHeight());
		setResizeable(true);
		setTitle("MWetris");
		setIcon(Configuration::getInstance().getWindowIcon());
		setBordered(Configuration::getInstance().isWindowBordered());
		setShowDemoWindow(true);

		deviceManager_ = std::make_shared<game::DeviceManager>();
		auto connection = deviceManager_->deviceConnected.connect([](game::DevicePtr device) {
			spdlog::info("Device found: {}", device->getName());
		});

		for (int i = 0; i < 4; ++i) {
			playerSlots_.push_back(scene::OpenSlot{});
		}

		pauseMenuText_ = "Pause";
	}

	TetrisWindow::~TetrisWindow() {
		Configuration::getInstance().quit();
	}

	int TetrisWindow::getCurrentMonitorHz() const {
		if (SDL_DisplayMode displayMode; SDL_GetCurrentDisplayMode(SDL_GetWindowDisplayIndex(getSdlWindow()), &displayMode) == 0) {
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
		sdl::ImGuiWindow::initPreLoop();
		auto& io{ImGui::GetIO()};

		Configuration::getInstance().bindTextureFromAtlas();
		background_ = Configuration::getInstance().getBackgroundSprite();

		io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 12); // Used by demo window.
		Configuration::getInstance().getImGuiButtonFont();
		Configuration::getInstance().getImGuiDefaultFont();
		Configuration::getInstance().getImGuiHeaderFont();

		game_ = std::make_shared<game::TetrisGame>();
		gameComponent_ = std::make_unique<graphic::GameComponent>();

		sceneStateMachine_.emplace<scene::EmptyScene>();
		sceneStateMachine_.emplace<scene::Settings>();
		sceneStateMachine_.emplace<scene::HighScore>();
		sceneStateMachine_.emplace<scene::NewHighScore>([this]() {
			openPopUp<scene::HighScore>();
		});
		sceneStateMachine_.emplace<scene::About>();
		sceneStateMachine_.emplace<scene::AddPlayer>([this](scene::AddPlayer& createGame) {
			int index = createGame.getSlotIndex();

			if (index < playerSlots_.size()) {
				playerSlots_[index] = createGame.getPlayer();
			} else {
				spdlog::warn("[TetrisWindow] Player slot index {} out of range (size = {})", index, playerSlots_.size());
			}
		}, deviceManager_);
		sceneStateMachine_.emplace<scene::JoinGame>(game_, deviceManager_);

		game_->setFixTimestep(1.0 / getCurrentMonitorHz());

		connections_ += game_->initGameEvent.connect(gameComponent_.get(), &mwetris::graphic::GameComponent::initGame);
		connections_ += game_->gameOverEvent.connect([this](game::GameOver gameOver) {
			game::DefaultPlayerData data{};
			if (const auto playerData{std::get_if<game::DefaultPlayerData>(&gameOver.playerBoard->getPlayerData())}; playerData) {
				data = *playerData;
			}

			if (const auto& playerBoard = *gameOver.playerBoard;  game_->isDefaultGame() && game::isNewHighScore(data.points)) {
				scene::NewHighScoreData data;
				data.name = playerBoard.getName();
				data.points = data.points;
				data.clearedRows = playerBoard.getClearedRows();
				data.level = data.level;
				openPopUp<scene::NewHighScore>(data);
			}
		});
		connections_ += game_->gamePauseEvent.connect([this](const game::GamePause& gamePause) {
			gameComponent_->gamePause(gamePause);

			if (gamePause.pause) {
				if (gamePause.countDown > 0) {
					pauseMenuText_ = "Stop Countdown";
				} else {
					pauseMenuText_ = "Unpause";
				}
			} else {
				pauseMenuText_ = "Pause";
			}
		});

		connections_ += network_.remotePlayerConnected.connect([&](network::Remote remote) {
			if (remote.slotIndex >= 0 && remote.slotIndex < playerSlots_.size()) {
				if (const auto internetPlayer{std::get_if<scene::InternetPlayer>(&playerSlots_[remote.slotIndex])}; internetPlayer && !internetPlayer->remotePlayer) {
					internetPlayer->remotePlayer = remote.remotePlayer;
				} else {
					spdlog::warn("[TetrisWindow] Fail to attach RemotePlayer to slot", remote.slotIndex);
				}
			} else {
				spdlog::warn("[TetrisWindow] Remote Player slot index {} out of range (size = {})", remote.slotIndex, playerSlots_.size());
			}
		});
		connections_ += network_.remotePlayerDisconnected.connect([&](game::RemotePlayerPtr remotePlayer) {
			if (removeRemotePlayer(playerSlots_, remotePlayer)) {
				spdlog::info("[TetrisWindow] Remote Player removed");
			} else {
				spdlog::warn("[TetrisWindow] Remote Player fail to be removed, not found");
			}
		});

		// Keep the update loop in sync with monitor.
		timeHandler_.scheduleRepeat([this]() {
			game_->setFixTimestep(1.0 / getCurrentMonitorHz());
		}, toSeconds(CheckRefreshRateInterval), std::numeric_limits<int>::max());

		startNewGame();
	}

	void TetrisWindow::eventUpdate(const SDL_Event& windowEvent) {
		sdl::ImGuiWindow::eventUpdate(windowEvent);
		deviceManager_->eventUpdate(windowEvent);
	}

	void TetrisWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
		network_.update();
		auto deltaTimeSeconds = toSeconds(deltaTime);
		game_->update(deltaTimeSeconds);
		timeHandler_.update(deltaTimeSeconds);

		ImGui::PushFont(Configuration::getInstance().getImGuiDefaultFont());
		imGuiMainWindow(deltaTime);

		constexpr auto popUpId = "Popup";
		if (openPopUp_) {
			openPopUp_ = false;
			ImGui::OpenPopup(popUpId);
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, {0.5f, 0.5f});
			ImGui::SetNextWindowSize({800, 800}, ImGuiCond_Appearing);
		}
		if (!ImGui::PopupModal(popUpId, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar, [&]() {
			auto pos = ImGui::GetCursorScreenPos();
			auto size = ImVec2{60.f, 30.f};

			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - size.x - 10.f); // TODO! Fix more less hard coded right alignment.
			if (ImGui::AbortButton("Cancel", size) || ImGui::IsKeyDown(ImGuiKey_Escape)) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::SetCursorScreenPos(pos);

			sceneStateMachine_.imGuiUpdate(deltaTime);
		})) {
			sceneStateMachine_.switchTo<scene::EmptyScene>();
		}

		ImGui::PopFont();
	}

	void TetrisWindow::imGuiMainWindow(const sdl::DeltaTime& deltaTime) {
		ImGui::MainWindow("MainWindow", ImguiNoWindow, [&]() {
			ImGui::ImageBackground(background_);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0, 10.0});
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 10.f);
			ImGui::MenuBar([&]() {
				ImGui::Menu("Game", [&]() {
					if (ImGui::MenuItem(game::hasSavedGame() ? "Resume Single Player" : "New Single Player", "F1")) {
						customGame = false;
						game_->createDefaultGame(deviceManager_->getDefaultDevice1());
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Create Game")) {
						customGame = true;
						reset(playerSlots_);
					}
					if (ImGui::MenuItem("Join Game")) {
						openPopUp<scene::JoinGame>();
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Highscore")) {
						game_->pause();
						openPopUp<mwetris::ui::scene::HighScore>();
					}
					if (ImGui::MenuItem("Preferences")) {
						openPopUp<mwetris::ui::scene::Settings>();
					}
					if (ImGui::MenuItem("Quit", "ESQ")) {
						sdl::Window::quit();
					}
				});
				ImGui::Menu("Current", [&]() {
					if (ImGui::MenuItem(pauseMenuText_.c_str(), "P")) {
						game_->pause();
					}
					if (ImGui::MenuItem("Restart", "F5")) {
						game_->restartGame();
					}
				});
				ImGui::Menu("Help", [&]() {
					if (ImGui::MenuItem("About")) {
						openPopUp<mwetris::ui::scene::About>();
					}
				});
			});
			ImGui::PopStyleVar(2);

			auto h = ImGui::GetCursorPosY();
			auto lowerBar = customGame ? 100 : 0;
			auto size = ImGui::GetWindowSize();

			if (customGame) {
				imGuiCustomGame(size.x, size.y - h - lowerBar, toSeconds(deltaTime));
			} else {
				gameComponent_->draw(size.x, size.y - h - lowerBar, toSeconds(deltaTime));
			}
			if (customGame) {
				ImGui::SetCursorPosY(200);
				ImGui::Separator();
				static std::string serverId = "asddgasasdf";
				ImGui::Text("Server Id: ");
				ImGui::SameLine();
				ImGui::InputText("##ServerId", &serverId, ImGuiInputTextFlags_ReadOnly);

				float width = ImGui::GetWindowWidth() - 2 * ImGui::GetCursorPosX();
				float height = 100.f;
				float y = ImGui::GetWindowHeight() - height - ImGui::GetStyle().WindowPadding.y;

				ImGui::BeginDisabled(playersInSlots(playerSlots_) == 0);
				ImGui::SetCursorPosY(y);

				if (ImGui::ConfirmationButton("Create Game", {width, height})) {
					game_->createGame(
						std::make_unique<game::SurvivalGameRules>(),
						TetrisWidth, TetrisHeight,
						extractHumans(playerSlots_),
						extractAis(playerSlots_),
						extractRemotePlayers(playerSlots_));
					customGame = false;
				}
				ImGui::EndDisabled();
			}
		});
	}

	void TetrisWindow::imGuiCustomGame(int windowWidth, int windowHeight, double deltaTime) {
		float width = windowWidth;
		float height = windowHeight;
		if (playerSlots_.size() > 1) {
			width = windowWidth / playerSlots_.size();
		}

		auto drawList = ImGui::GetWindowDrawList();
		auto pos = ImGui::GetCursorScreenPos();

		for (int i = 0; i < playerSlots_.size(); ++i) {
			auto& playerSlot = playerSlots_[i];

			ImGui::PushID(i + 1);
			ImGui::SetCursorScreenPos(pos);
			pos.x += width;

			ImGui::BeginGroup();
			std::visit([&](auto&& slot) mutable {
				using T = std::decay_t<decltype(slot)>;
				if constexpr (std::is_same_v<T, scene::DeviceType>) {
					ImGui::Text("scene::DeviceType");
					ImGui::Text("Player name: %s", slot.name.c_str());
					if (ImGui::AbortButton("Remove")) {
						playerSlot = scene::OpenSlot{};
					}
				} else if constexpr (std::is_same_v<T, scene::AiType>) {
					ImGui::Text("scene::AiType");
					ImGui::Text("Player name: %s", slot.name.c_str());
					if (ImGui::AbortButton("Remove")) {
						playerSlot = scene::OpenSlot{};
					}
				} else if constexpr (std::is_same_v<T, scene::InternetPlayer>) {
					ImGui::Text("scene::InternetPlayer");
					if (slot.remotePlayer) {
						ImGui::Text("Connected");
					} else {
						ImGui::Text("Open");
					}
					if (ImGui::AbortButton("Remove")) {
						playerSlot = scene::OpenSlot{};
					}
				} else if constexpr (std::is_same_v<T, scene::OpenSlot>) {
					if (ImGui::Button("Open Slot", {100, 100})) {
						scene::AddPlayerData data{};
						data.index = i;
						openPopUp<scene::AddPlayer>(data);
					}
				} else {
					static_assert(always_false_v<T>, "non-exhaustive visitor!");
				}
			}, playerSlot);
			ImGui::EndGroup();

			ImGui::PopID();
		}
	}

	void TetrisWindow::imGuiEventUpdate(const SDL_Event& windowEvent) {
		switch (windowEvent.type) {
			case SDL_WINDOWEVENT:
				switch (windowEvent.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						gl::glViewport(0, 0, windowEvent.window.data1, windowEvent.window.data2);
						break;
					case SDL_WINDOWEVENT_LEAVE:
						break;
					case SDL_WINDOWEVENT_CLOSE:
						sdl::Window::quit();
						break;
				}
				break;
			case SDL_KEYDOWN:
				switch (windowEvent.key.keysym.sym) {
					case SDLK_0:
						network_.debugAddRemotePlayer(0);
						break;
					case SDLK_1:
						network_.debugAddRemotePlayer(1);
						break;
					case SDLK_2:
						network_.debugAddRemotePlayer(2);
						break;
					case SDLK_3:
						network_.debugAddRemotePlayer(3);
						break;
					case SDLK_4:
						if (auto remotePlayers = extractRemotePlayers(playerSlots_); !remotePlayers.empty()) {
							network_.debugRemoveRemotePlayer(remotePlayers.back());
						}
						break;
					case SDLK_ESCAPE:
						game_->saveDefaultGame();
						sdl::Window::quit();
						break;
					case SDLK_F1:
						game_->createDefaultGame(deviceManager_->getDefaultDevice1());
						break;
					case SDLK_F5:
						game_->restartGame();
						break;
					case SDLK_p: [[fallthrough]];
					case SDLK_PAUSE:
						game_->pause();
						break;
				}
				break;
			case SDL_QUIT:
				game_->saveDefaultGame();
				sdl::Window::quit();
				break;
		}
	}

	void TetrisWindow::startNewGame() {
		game_->createDefaultGame(deviceManager_->getDefaultDevice1());
	}

}

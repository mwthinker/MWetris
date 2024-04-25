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
#include "game/localplayerboardbuilder.h"

#include "graphic/gamecomponent.h"

#include "network/client.h"
#include "network/debugclient.h"
#include "util.h"

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
		
		constexpr double toSeconds(const auto& duration) {
			return std::chrono::duration<double>(duration).count();
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
		std::shared_ptr<network::Client> client
	)
		: window_{window}
		, deviceManager_{deviceManager}
		, client_{client}
		, type_{type}
		, windowName_{windowName}
	{
		game_ = std::make_shared<game::TetrisGame>();
		network_ = std::make_shared<network::Network>(client_, game_);

		connections_ += deviceManager_->deviceConnected.connect([](game::DevicePtr device) {
			spdlog::info("Device found: {}", device->getName());
		});

		for (int i = 0; i < 4; ++i) {
			playerSlots_.push_back(game::OpenSlot{});
		}

		pauseMenuText_ = "Pause";

		initPreLoop();
	}

	TetrisWindow::~TetrisWindow() {
		Configuration::getInstance().quit();

		if (type_ == Type::MainWindow) {
			game_->saveDefaultGame();
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
				network_->setPlayerSlot(createGame.getPlayer(), index);
			} else {
				spdlog::warn("[TetrisWindow] Player slot index {} out of range (size = {})", index, playerSlots_.size());
			}
		}, deviceManager_);
		sceneStateMachine_.emplace<scene::JoinGame>(game_, network_);

		connections_ += network_->addPlayerSlotListener([this](game::PlayerSlot playerSlot, int slot) {
			playerSlots_[slot] = playerSlot;
		});

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
			//network_->sendPause(gamePause.pause);
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

		// Keep the update loop in sync with monitor.
		timeHandler_.scheduleRepeat([this]() {
			game_->setFixTimestep(1.0 / getCurrentMonitorHz());
		}, toSeconds(CheckRefreshRateInterval), std::numeric_limits<int>::max());

		startNewGame();
	}

	void TetrisWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
		network_->update();
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
		MainWindow(*this, [&]() {
			//networkDebugWindow_.imGuiUpdate(deltaTime);

			ImGui::ImageBackground(background_);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0, 10.0});
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 10.f);
			ImGui::MenuBar([&]() {
				ImGui::Menu("Game", [&]() {
					if (ImGui::MenuItem(game::hasSavedGame() ? "Resume Single Player" : "New Single Player", "F1")) {
						customGame_ = false;
						game_->createDefaultGame(deviceManager_->getDefaultDevice1());
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Create Game")) {
						customGame_ = true;
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
						window_.quit();
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
			auto lowerBar = customGame_ ? 100 : 0;
			auto size = ImGui::GetWindowSize();

			if (customGame_) {
				imGuiCustomGame(size.x, size.y - h - lowerBar, toSeconds(deltaTime));
			} else {
				gameComponent_->draw(size.x, size.y - h - lowerBar, toSeconds(deltaTime));
			}
			if (customGame_) {
				bool internet = network_->isConnected();
				if (ImGui::Checkbox("Network", &internet)) {
					if (internet) {
						network_->connect();
						spdlog::debug("Network on");
					} else {
						network_->disconnect();
						spdlog::debug("Network off");
					}
				}
				if (internet) {
					serverId_ = network_->getServerId();
					ImGui::SetCursorPosY(200);
					ImGui::Separator();
					ImGui::Text("Server Id: ");
					ImGui::SameLine();
					ImGui::InputText("##ServerId", &serverId_, ImGuiInputTextFlags_ReadOnly);
				}

				float width = ImGui::GetWindowWidth() - 2 * ImGui::GetCursorPosX();
				float height = 100.f;
				float y = ImGui::GetWindowHeight() - height - ImGui::GetStyle().WindowPadding.y;

				ImGui::BeginDisabled(playersInSlots(playerSlots_) == 0);
				ImGui::SetCursorPosY(y);

				if (ImGui::ConfirmationButton("Create Game", {width, height})) {
					if (internet) {
						network_->startGame(std::make_unique<game::SurvivalGameRules>(), TetrisWidth, TetrisHeight);
					} else {
						game_->createGame(
							std::make_unique<game::SurvivalGameRules>(),
							game::PlayerFactory{}
								.createPlayers(TetrisWidth, TetrisHeight
									, extract<game::Human>(playerSlots_)
									, extract<game::Ai>(playerSlots_)));
					}
					customGame_ = false;
				}
				ImGui::EndDisabled();
			}
		});
	}

	void TetrisWindow::imGuiCustomGame(int windowWidth, int windowHeight, double deltaTime) {
		float width = windowWidth;

		if (playerSlots_.size() > 1) {
			width = windowWidth / playerSlots_.size();
		}
		
		auto pos = ImGui::GetCursorScreenPos();

		for (int i = 0; i < playerSlots_.size(); ++i) {
			auto& playerSlot = playerSlots_[i];

			ImGui::PushID(i + 1);
			ImGui::SetCursorScreenPos(pos);
			pos.x += width;

			ImGui::BeginGroup();
			std::visit([&](auto&& slot) mutable {
				using T = std::decay_t<decltype(slot)>;
				if constexpr (std::is_same_v<T, game::Human>) {
					ImGui::Text("game::Human");
					ImGui::Text("Player name: %s", slot.name.c_str());
					if (ImGui::AbortButton("Remove")) {
						playerSlot = game::OpenSlot{};
						network_->setPlayerSlot(playerSlot, i);
					}
				} else if constexpr (std::is_same_v<T, game::Ai>) {
					ImGui::Text("game::Ai");
					ImGui::Text("Player name: %s", slot.name.c_str());
					if (ImGui::AbortButton("Remove")) {
						playerSlot = game::OpenSlot{};
						network_->setPlayerSlot(playerSlot, i);
					}
				} else if constexpr (std::is_same_v<T, game::Remote>) {
					ImGui::Text("game::Remote");
					if (ImGui::AbortButton("Remove")) {
						playerSlot = game::OpenSlot{};
						network_->setPlayerSlot(playerSlot, i);
					}
				} else if constexpr (std::is_same_v<T, game::ClosedSlot>) {
					// Skip.
				} else if constexpr (std::is_same_v<T, game::OpenSlot>) {
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
		if (type_ == Type::SecondaryWindow) {
			return;
		}

		switch (windowEvent.type) {
			case SDL_KEYDOWN:
				switch (windowEvent.key.keysym.sym) {
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
		}
	}

	const std::string& TetrisWindow::getName() const {
		return windowName_;
	}

	SubWindow::Type TetrisWindow::getType() const {
		return type_;
	}

	void TetrisWindow::startNewGame() {
		game_->createDefaultGame(deviceManager_->getDefaultDevice1());
	}

}

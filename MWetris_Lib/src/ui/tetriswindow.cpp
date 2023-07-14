#include "tetriswindow.h"
#include "imguiextra.h"
#include "configuration.h"

#include "scene/about.h"
#include "scene/network.h"
#include "scene/settings.h"
#include "scene/highscore.h"
#include "scene/newhighscore.h"
#include "scene/customgame.h"

#include "game/keyboard.h"
#include "game/computer.h"
#include "game/serialize.h"
#include "game/tetrisgame.h"

#include "graphic/gamecomponent.h"

#include <sdl/imguiauxiliary.h>

#include <spdlog/spdlog.h>

namespace mwetris::ui {

	namespace {

		game::ComputerPtr findAiDevice(const std::string& name) {
			auto ais = Configuration::getInstance().getAiVector();
			for (const auto& ai : ais) {
				if (ai.getName() == name) {
					return std::make_shared<game::Computer>(ai);
				}
			}
			return std::make_shared<game::Computer>(ais.back());
		}

		int acceptNameInput(ImGuiInputTextCallbackData* data) {
			return data->BufTextLen < 30;
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
		auto connection = deviceManager_->deviceFound.connect([](game::DevicePtr device) {
			spdlog::info("Device found: {}", device->getName());
		});
	}

	TetrisWindow::~TetrisWindow() {
		Configuration::getInstance().quit();
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
		sceneStateMachine_.emplace<scene::Network>();
		sceneStateMachine_.emplace<scene::CustomGame>(game_, deviceManager_);
		openPopUp<scene::CustomGame>();
		
		computers_.push_back(findAiDevice(Configuration::getInstance().getAi1Name()));
		computers_.push_back(findAiDevice(Configuration::getInstance().getAi2Name()));
		computers_.push_back(findAiDevice(Configuration::getInstance().getAi3Name()));
		computers_.push_back(findAiDevice(Configuration::getInstance().getAi4Name()));

		startNewGame();
	}

	void TetrisWindow::eventUpdate(const SDL_Event& windowEvent) {
		sdl::ImGuiWindow::eventUpdate(windowEvent);
		deviceManager_->eventUpdate(windowEvent);
	}

	void TetrisWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
		auto deltaTimeSeconds = std::chrono::duration<double>(deltaTime).count();
		game_->update(deltaTimeSeconds);
		
		ImGui::PushFont(Configuration::getInstance().getImGuiDefaultFont());

		if (openPopUp_) {
			openPopUp_ = false;
			ImGui::OpenPopup("Popup");
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			ImGui::SetNextWindowSize({800, 800}, ImGuiCond_Appearing);
		}
		if (!ImGui::PopupModal("Popup", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar, [&]() {
			sceneStateMachine_.imGuiUpdate(deltaTime);

			if (ImGui::IsKeyDown(ImGuiKey_Escape)) {
				ImGui::CloseCurrentPopup();
				sceneStateMachine_.switchTo<scene::EmptyScene>();
			}
		})) {
			imGuiMainMenu(deltaTime);
		}

		ImGui::PopFont();
	}

	void TetrisWindow::imGuiMainMenu(const sdl::DeltaTime& deltaTime) {
		ImGui::MainWindow("MainWindow", ImguiNoWindow, [&]() {
			ImGui::ImageBackground(background_);

			ImGui::MenuBar([&]() {
				ImGui::Menu("Main", [&]() {
					if (ImGui::MenuItem("New Single Game", "F2")) {
						game_->restartGame();
					}
					if (ImGui::MenuItem("Custom Game")) {
						openPopUp<scene::CustomGame>();
					}
					if (ImGui::MenuItem("Highscore")) {
						openPopUp<mwetris::ui::scene::HighScore>();
					}
					if (ImGui::MenuItem("Quit", "ESQ")) {
						sdl::Window::quit();
					}
				});
				ImGui::Menu("Settings", [&]() {
					if (ImGui::MenuItem("Preferences")) {
						openPopUp<mwetris::ui::scene::Settings>();
					}
				});
				ImGui::Menu("In Game", []() {
					if (ImGui::MenuItem("Pause", "P OR Pause")) {}
					if (ImGui::MenuItem("Restart", "F2")) {}
				});
				ImGui::Menu("Network Game", [&]() {
					if (ImGui::MenuItem("Undo", "CTRL+Z")) {
						openPopUp<mwetris::ui::scene::Network>();
					}
					if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
					ImGui::Separator();
					if (ImGui::MenuItem("Cut", "CTRL+X")) {}
					if (ImGui::MenuItem("Copy", "CTRL+C")) {}
					if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				});
				ImGui::Menu("Help", [&]() {
					if (ImGui::MenuItem("About")) {
						openPopUp<mwetris::ui::scene::About>();
					}
				});
			});
			auto h = ImGui::GetCursorPosY();
			auto size = ImGui::GetWindowSize();
			gameComponent_->draw(size.x, size.y - h, std::chrono::duration<double>(deltaTime).count());
		});
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
					case SDLK_ESCAPE:
						game_->saveCurrentGame();
						sdl::Window::quit();
						break;
					case SDLK_F2:
						game_->restartGame();
						break;
					case SDLK_p: [[fallthrough]];
					case SDLK_PAUSE:
						game_->pause();
						break;
				}
				break;
			case SDL_QUIT:
				sdl::Window::quit();
				break;
		}
	}

	void TetrisWindow::resumeGame() {
		int rows = Configuration::getInstance().getActiveLocalGameRows();
		int columns = Configuration::getInstance().getActiveLocalGameColumns();

		nbrAis_ = 0;
		nbrHumans_ = 0;
	}

	void TetrisWindow::startNewGame() {
		connections_.clear();
		connections_ += game_->initGameEvent.connect(gameComponent_.get(), &mwetris::graphic::GameComponent::initGame);
		connections_ += game_->gameOverEvent.connect([this](game::GameOver gameOver) {
			if (game_->getNbrOfPlayers() == 1 && game::isNewHighScore(gameOver.player)) {
				scene::NewHighScoreData data;
				data.name = gameOver.player->getName();
				data.points = gameOver.player->getPoints();
				data.clearedRows = gameOver.player->getClearedRows();
				data.level = gameOver.player->getLevel();
				openPopUp<scene::NewHighScore>(data);
			}
		});
		connections_ += game_->gamePauseEvent.connect([this](game::GamePause gamePause) {
			gameComponent_->gamePause(gamePause);
		});
		connections_ += game_->countDownGameEvent.connect([this](game::CountDown countDown) {
			gameComponent_->countDown(countDown);
		});

		if (game::hasSavedGame()) {
			game_->resumeGame(deviceManager_->getAllDevicesAvailable());
		} else {
			game_->createGame({deviceManager_->getDefaultDevice1()});
		}
	}

}

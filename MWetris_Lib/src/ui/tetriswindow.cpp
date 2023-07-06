#include "tetriswindow.h"
#include "imguiextra.h"
#include "configuration.h"

#include "scene/about.h"
#include "scene/network.h"
#include "scene/settings.h"
#include "scene/highscore.h"
#include "scene/newhighscore.h"

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
		//| ImGuiWindowFlags_NoNavInputs;

	TetrisWindow::TetrisWindow() {
		setPosition(Configuration::getInstance().getWindowPositionX(), Configuration::getInstance().getWindowPositionY());
		setSize(Configuration::getInstance().getWindowWidth(), Configuration::getInstance().getWindowHeight());
		setResizeable(true);
		setTitle("MWetris");
		setIcon(Configuration::getInstance().getWindowIcon());
		setBordered(Configuration::getInstance().isWindowBordered());
		setShowDemoWindow(true);
	}

	TetrisWindow::~TetrisWindow() {
		Configuration::getInstance().quit();
	}

	void TetrisWindow::initPreLoop() {
		sdl::ImGuiWindow::initPreLoop();
		auto& io{ImGui::GetIO()};
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		Configuration::getInstance().bindTextureFromAtlas();
		background_ = Configuration::getInstance().getBackgroundSprite();
		
		io.Fonts->AddFontFromFileTTF("fonts/Ubuntu-B.ttf", 12); // Used by demo window.
		Configuration::getInstance().getImGuiButtonFont();
		Configuration::getInstance().getImGuiDefaultFont();
		Configuration::getInstance().getImGuiHeaderFont();
		
		sceneStateMachine_.emplace<scene::Settings>();
		sceneStateMachine_.emplace<scene::HighScore>();
		sceneStateMachine_.emplace<scene::About>();
		sceneStateMachine_.emplace<scene::Network>();
		
		computers_.push_back(findAiDevice(Configuration::getInstance().getAi1Name()));
		computers_.push_back(findAiDevice(Configuration::getInstance().getAi2Name()));
		computers_.push_back(findAiDevice(Configuration::getInstance().getAi3Name()));
		computers_.push_back(findAiDevice(Configuration::getInstance().getAi4Name()));

		devices_.push_back(std::make_shared<game::Keyboard>("Keyboard 1", SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_RCTRL));
		devices_.push_back(std::make_shared<game::Keyboard>("Keyboard 2", SDLK_s, SDLK_a, SDLK_d, SDLK_w, SDLK_LCTRL));

		game_ = std::make_unique<game::TetrisGame>();
		gameComponent_ = std::make_unique<graphic::GameComponent>();

		startNewGame();
	}

	void TetrisWindow::imGuiUpdate(const sdl::DeltaTime& deltaTime) {
		auto deltaTimeSeconds = std::chrono::duration<double>(deltaTime).count();
		game_->update(deltaTimeSeconds);
		
		ImGui::PushFont(Configuration::getInstance().getImGuiDefaultFont());

		if (openPopUp_) {
			openPopUp_ = false;
			ImGui::OpenPopup("Popup");
		}
		if (!ImGui::PopupModal("Popup", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar, [&]() {
			sceneStateMachine_.imGuiUpdate(deltaTime);

			if (ImGui::IsKeyDown(ImGuiKey_Escape)) {
				ImGui::CloseCurrentPopup();
			}
		})) {
			ImGui::MainWindow("MainWindow", ImguiNoWindow, [&]() {
				ImGui::ImageBackground(background_);

				ImGui::MenuBar([&]() {
					ImGui::Menu("Main", [&]() {
						if (ImGui::MenuItem("New Game", "F2")) {
							startNewGame();
						}
						if (ImGui::MenuItem("Custom Game")) {

						}
						if (ImGui::MenuItem("Highscore")) {
							ImVec2 center = ImGui::GetMainViewport()->GetCenter();
							ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
							ImGui::SetNextWindowSize({500, 500}, ImGuiCond_Appearing);

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
				gameComponent_->draw(size.x, size.y - h, deltaTimeSeconds);
			});
		}

		ImGui::PopFont();
	}

	void TetrisWindow::imGuiEventUpdate(const SDL_Event& windowEvent) {
		for (auto& device : devices_) {
			device->eventUpdate(windowEvent);
		}

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

	std::vector<game::DevicePtr> TetrisWindow::getCurrentDevices() const {
		std::vector<game::DevicePtr> playerDevices(devices_.begin(), devices_.begin() + nbrHumans_);

		for (int i = 0; i < nbrAis_; ++i) {
			if (computers_[i]) {
				playerDevices.push_back(computers_[i]);
			}
		}

		return playerDevices;
	}

	game::DevicePtr TetrisWindow::findHumanDevice(const std::string& name) const {
		for (const auto& device : devices_) {
			if (device->getName() == name) {
				return device;
			}
		}
		return devices_[0];
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
				sceneStateMachine_.switchTo<scene::NewHighScore>();
				//openPopup_ = true;
				//gameOver_ = gameOver;
			}
		});
		connections_ += game_->gamePauseEvent.connect([this](game::GamePause gamePause) {
			gameComponent_->gamePause(gamePause);
		});
		connections_ += game_->countDownGameEvent.connect([this](game::CountDown countDown) {
			gameComponent_->countDown(countDown);
		});

		std::vector<game::DevicePtr> devices;
		devices.push_back(devices_[0]);

		if (game::hasSavedGame()) {
			game_->resumeGame(devices);
		} else {
			game_->createGame(devices);
		}
	}

}

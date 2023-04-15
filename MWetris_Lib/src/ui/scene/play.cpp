#include "play.h"
#include "event.h"
#include "game/keyboard.h"
#include "game/computer.h"

namespace mwetris::ui::scene {

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

	}

	Play::Play() {
		computers_.push_back(findAiDevice(Configuration::getInstance().getAi1Name()));
		computers_.push_back(findAiDevice(Configuration::getInstance().getAi2Name()));
		computers_.push_back(findAiDevice(Configuration::getInstance().getAi3Name()));
		computers_.push_back(findAiDevice(Configuration::getInstance().getAi4Name()));


		devices_.push_back(std::make_shared<game::Keyboard>("Keyboard 1", SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_RCTRL));
		devices_.push_back(std::make_shared<game::Keyboard>("Keyboard 2", SDLK_s, SDLK_a, SDLK_d, SDLK_w, SDLK_LCTRL));

		crossSprite_ = mwetris::Configuration::getInstance().getCrossSprite();
		manSprite_ = mwetris::Configuration::getInstance().getHumanSprite();
		aiSprite_ = mwetris::Configuration::getInstance().getComputerSprite();
	}

	void Play::eventUpdate(const SDL_Event& windowEvent) {
		for (auto& device : devices_) {
			device->eventUpdate(windowEvent);
		}
		switch (windowEvent.type) {
			case SDL_KEYDOWN:
				switch (windowEvent.key.keysym.sym) {
					case SDLK_F2:
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

	void Play::imGuiUpdate(const DeltaTime& deltaTime) {
		auto deltaTimeSeconds = std::chrono::duration<double>(deltaTime).count();
		game_->update(deltaTimeSeconds);

		auto menuHeight = mwetris::Configuration::getInstance().getWindowBarHeight();

		ImGui::Bar([&]() {
			ImGui::PushButtonStyle();
			
			if (ImGui::Button("Menu", {100.5f, menuHeight})) {
				emitEvent(Event::Menu);
			}
			
			ImGui::SameLine();
			if (ImGui::Button("Restart", {120.5f, menuHeight})) {
				game_->restartGame();
			}

			ImGui::SameLine();
			if (ImGui::ManButton("Human", nbrHumans_, static_cast<int>(devices_.size()), crossSprite_, manSprite_, {menuHeight, menuHeight})) {
				game_->createGame(getCurrentDevices());
			}
			ImGui::SameLine();
			if (ImGui::ManButton("Ai", nbrAis_, static_cast<int>(computers_.size()), crossSprite_, aiSprite_, {menuHeight, menuHeight})) {
				game_->createGame(getCurrentDevices());
			}
			ImGui::PopButtonStyle();
		});

		size_ = ImGui::GetWindowSize();
		
		if (gameComponent_) {
			gameComponent_->draw(size_.x, size_.y - menuHeight, deltaTimeSeconds);
		}
	}

	std::vector<game::DevicePtr> Play::getCurrentDevices() const {
		std::vector<game::DevicePtr> playerDevices(devices_.begin(), devices_.begin() + nbrHumans_);

		for (int i = 0; i < nbrAis_; ++i) {
			if (computers_[i]) {
				playerDevices.push_back(computers_[i]);
			}
		}

		return playerDevices;
	}

	game::DevicePtr Play::findHumanDevice(const std::string& name) const {
		for (const auto& device : devices_) {
			if (device->getName() == name) {
				return device;
			}
		}
		return devices_[0];
	}

	void Play::switchedFrom() {
		game_->saveCurrentGame();
	}

	void Play::switchedTo() {
		gameComponent_ = std::make_unique<graphic::GameComponent>();
		game_ = std::make_unique<game::TetrisGame>();
		connections_.clear();
		connections_ += game_->initGameEvent.connect(gameComponent_.get(), &mwetris::graphic::GameComponent::initGame);

		std::vector<game::DevicePtr> devices;
		devices.push_back(devices_[0]);
		game_->createGame(devices);
	}

	void Play::resumeGame() {
		int rows = Configuration::getInstance().getActiveLocalGameRows();
		int columns = Configuration::getInstance().getActiveLocalGameColumns();

		nbrAis_ = 0;
		nbrHumans_ = 0;
	}

}

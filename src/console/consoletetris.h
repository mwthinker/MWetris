#ifndef CONSOLETETRIS_H
#define CONSOLETETRIS_H

#include "tetrisparameters.h"
#include "tetrisgameevent.h"
#include "device.h"
#include "tetrisgame.h"
#include "tetrisboard.h"
#include "computer.h"
#include "consolegraphic.h"
#include "consolekeyboard.h"
#include "device.h"
#include "ai.h"
#include "tetrisparameters.h"

#include <console/console.h>

#include <array>

namespace tetris {

	class ConsoleTetris : public console::Console {
	public:
		ConsoleTetris();
		~ConsoleTetris();

	protected:
		void initPreLoop() override;

	private:
		void update(double deltaTime) override;

		void eventUpdate(console::ConsoleEvent& consoleEvent) override;

		void printMainMenu();

		void printGameMenu();

		void printGame();

		void draw(int x, int y, std::string text);

		void draw(int x, int y, std::string text, console::Color color);

		void drawClear(int x, int y, std::string text, console::Color color);

		void handleConnectionEvent(TetrisGameEvent& tetrisEvent);

		IDevicePtr findAiDevice(std::string name) const;

		void moveMenuUp();
		void moveMenuDown();

		void restartCurrentGame();

		void execute(TetrisMenu option);

		TetrisGame tetrisGame_;

		std::array<IDevicePtr, 3> activeAis_;

		TetrisMenu mode_;
		TetrisMenu option_;
		std::shared_ptr<ConsoleKeyboard> keyboard1_, keyboard2_;
		int humanPlayers_;
		int aiPlayers_;
		std::vector<IDevicePtr> activePlayers_;
		std::map<int, ConsoleGraphic> graphicPlayers_;
		std::array<char, 100 * 100> screen_;
		mw::signals::Connection gameConnection_;
	};

}

#endif // CONSOLETETRIS_H

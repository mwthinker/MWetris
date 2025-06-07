#include <gtest/gtest.h>

#include <app/game/serialize.h>
#include <app/game/keyboard.h>

namespace app::game {

	namespace {

		SDL_Event createKeyboardEvent(SDL_Keycode keyCode, SDL_EventType type) {
			SDL_KeyboardEvent keyBoardEvent{
				.type = (Uint32) type,
				.keysym = SDL_Keysym{
					.sym = keyCode
				}
			};
			SDL_Event event;
			event.key = keyBoardEvent;
			return event;
		}

	}

	class KeyboardTest : public ::testing::Test {
	protected:

		KeyboardTest() {}

		~KeyboardTest() override {}

		void SetUp() override {
			
		}

		void TearDown() override {}
	};

	TEST_F(KeyboardTest, keyPressedOneFrame) {
		// Given
		Keyboard keyboard{"keyboard", KeyboardMapping{
			.down=SDLK_DOWN,
			.right=SDLK_RIGHT,
			.left=SDLK_LEFT,
			.rotate=SDLK_UP,
			.downGround=SDLK_RCTRL
		}};

		// When
		keyboard.eventUpdate(createKeyboardEvent(SDLK_DOWN, SDL_KEYDOWN));

		// Then
		auto down = keyboard.getInput().down;
		ASSERT_TRUE(down.pressed);
		ASSERT_TRUE(down.held);
		ASSERT_FALSE(down.released);
	}

	TEST_F(KeyboardTest, keyPressedTwoFrames) {
		// Given
		Keyboard keyboard{"keyboard", KeyboardMapping{
			.down=SDLK_DOWN,
			.right=SDLK_RIGHT,
			.left=SDLK_LEFT,
			.rotate=SDLK_UP,
			.downGround=SDLK_RCTRL
		}};

		// When
		keyboard.eventUpdate(createKeyboardEvent(SDLK_DOWN, SDL_KEYDOWN));
		keyboard.tick();

		// Then
		auto down = keyboard.getInput().down;
		ASSERT_FALSE(down.pressed);
		ASSERT_TRUE(down.held);
		ASSERT_FALSE(down.released);
	}

	TEST_F(KeyboardTest, keyReleased) {
		// Given
		Keyboard keyboard{"keyboard", KeyboardMapping{
			.down=SDLK_DOWN,
			.right=SDLK_RIGHT,
			.left=SDLK_LEFT,
			.rotate=SDLK_UP,
			.downGround=SDLK_RCTRL
		}};
		keyboard.eventUpdate(createKeyboardEvent(SDLK_DOWN, SDL_KEYDOWN));
		keyboard.tick();
		keyboard.tick();
		keyboard.eventUpdate(createKeyboardEvent(SDLK_DOWN, SDL_KEYUP));

		// When

		// Then
		auto down = keyboard.getInput().down;
		ASSERT_FALSE(down.pressed);
		ASSERT_FALSE(down.held);
		ASSERT_TRUE(down.released);
	}

}

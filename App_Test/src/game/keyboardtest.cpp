#include <gtest/gtest.h>

#include <app/game/serialize.h>
#include <app/game/keyboard.h>

namespace app::game {

	namespace {

		SDL_Event createKeyboardEvent(SDL_Keycode keyCode, SDL_EventType type) {
			SDL_KeyboardEvent keyBoardEvent{
				.type = type,
				.key = keyCode
			};
			SDL_Event event;
			event.key = keyBoardEvent;
			return event;
		}

	}

	TEST(KeyboardTest, beginMappingClearsInputAndAwaitsLeft) {
		// Given
		Keyboard keyboard{"Keyboard 1", getDefaultKeyboardBindings(1)};
		keyboard.eventUpdate(createKeyboardEvent(SDLK_LEFT, SDL_EVENT_KEY_DOWN));
		ASSERT_TRUE(keyboard.getInput().left.held);

		// When
		keyboard.beginMapping();

		// Then
		EXPECT_EQ(keyboard.getInput(), Input{});
		ASSERT_TRUE(keyboard.getMappingStatus());
		EXPECT_EQ(keyboard.getMappingStatus()->awaiting, InputAction::Left);
	}

	TEST(KeyboardTest, captureSwapsDuplicateBindingInDraft) {
		// Given
		Keyboard keyboard{"Keyboard 1", getDefaultKeyboardBindings(1)};
		keyboard.beginMapping();

		// When
		auto consumed = keyboard.captureMappingEvent(createKeyboardEvent(SDLK_RIGHT, SDL_EVENT_KEY_DOWN));

		// Then
		EXPECT_TRUE(consumed);
		auto draft = keyboard.getDraftBindings();
		ASSERT_TRUE(draft);
		EXPECT_EQ(draft->left, SDLK_RIGHT);
		EXPECT_EQ(draft->right, SDLK_LEFT);
	}

	TEST(KeyboardTest, commitAppliesCompleteDraft) {
		// Given
		Keyboard keyboard{"Keyboard 1", getDefaultKeyboardBindings(1)};
		keyboard.beginMapping();
		const std::array keys{SDLK_RIGHT, SDLK_J, SDLK_K, SDLK_L, SDLK_SPACE};
		for (auto key : keys) {
			ASSERT_TRUE(keyboard.captureMappingEvent(createKeyboardEvent(key, SDL_EVENT_KEY_DOWN)));
		}

		// When
		auto result = keyboard.commitMapping();

		// Then
		ASSERT_TRUE(result);
		EXPECT_EQ(keyboard.getBindings(), (KeyboardBindings{
			.left = SDLK_RIGHT,
			.right = SDLK_J,
			.softDrop = SDLK_K,
			.rotate = SDLK_L,
			.hardDrop = SDLK_SPACE,
		}));
		EXPECT_FALSE(keyboard.getMappingStatus());
	}

	TEST(KeyboardTest, committedMappingHandlesGameplayInput) {
		// Given
		Keyboard keyboard{"Keyboard 1", getDefaultKeyboardBindings(1)};
		keyboard.beginMapping();

		const std::array keys{SDLK_RIGHT, SDLK_J, SDLK_K, SDLK_L, SDLK_SPACE};
		for (auto key : keys) {
			ASSERT_TRUE(keyboard.captureMappingEvent(createKeyboardEvent(key, SDL_EVENT_KEY_DOWN)));
		}
		ASSERT_TRUE(keyboard.commitMapping());

		// When
		keyboard.eventUpdate(createKeyboardEvent(SDLK_RIGHT, SDL_EVENT_KEY_DOWN));

		// Then
		EXPECT_TRUE(keyboard.getInput().left.pressed);
		EXPECT_FALSE(keyboard.getInput().right.pressed);
	}

	TEST(KeyboardTest, repeatedKeyDoesNotAdvanceMapping) {
		// Given
		Keyboard keyboard{"Keyboard 1", getDefaultKeyboardBindings(1)};
		keyboard.beginMapping();
		auto repeated = createKeyboardEvent(SDLK_A, SDL_EVENT_KEY_DOWN);
		repeated.key.repeat = 1;

		// When
		auto consumed = keyboard.captureMappingEvent(repeated);

		// Then
		EXPECT_TRUE(consumed);
		ASSERT_TRUE(keyboard.getMappingStatus());
		EXPECT_EQ(keyboard.getMappingStatus()->completed, 0);
	}

	TEST(KeyboardTest, cancelKeepsActiveMapping) {
		// Given
		auto defaults = getDefaultKeyboardBindings(1);
		Keyboard keyboard{"Keyboard 1", defaults};
		keyboard.beginMapping();
		keyboard.captureMappingEvent(createKeyboardEvent(SDLK_A, SDL_EVENT_KEY_DOWN));

		// When
		keyboard.cancelMapping();

		// Then
		EXPECT_EQ(keyboard.getBindings(), defaults);
		EXPECT_FALSE(keyboard.getMappingStatus());
	}

}

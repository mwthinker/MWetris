#include <gtest/gtest.h>

#include <app/game/dasarrhandler.h>

#include <limits>
#include <stdexcept>

namespace app::game {

	TEST(DasArrHandlerTest, repeatsAfterDasAtArrIntervalsAndResetsOnRelease) {
		DasArrHandler handler{0.1, 0.05};
		EXPECT_DOUBLE_EQ(handler.getDas(), 0.1);
		EXPECT_DOUBLE_EQ(handler.getArr(), 0.05);

		handler.update(0.0, {.held = true, .pressed = true});
		EXPECT_TRUE(handler.doAction());
		EXPECT_FALSE(handler.doAction());

		handler.update(0.09, {.held = true});
		EXPECT_FALSE(handler.doAction());
		handler.update(0.02, {.held = true});
		EXPECT_TRUE(handler.doAction());

		handler.update(0.04, {.held = true});
		EXPECT_FALSE(handler.doAction());
		handler.update(0.02, {.held = true});
		EXPECT_TRUE(handler.doAction());

		handler.update(0.0, {.released = true});
		handler.update(0.0, {.held = true, .pressed = true});
		EXPECT_TRUE(handler.doAction());
	}

	TEST(DasArrHandlerTest, changedTimingClearsPendingAndHeldRepeatState) {
		DasArrHandler handler{0.2, 0.05};
		handler.update(0.0, {.held = true, .pressed = true});
		handler.setTiming(0.3, 0.1);
		EXPECT_FALSE(handler.doAction());
		EXPECT_DOUBLE_EQ(handler.getDas(), 0.3);
		EXPECT_DOUBLE_EQ(handler.getArr(), 0.1);

		handler.update(0.29, {.held = true});
		EXPECT_FALSE(handler.doAction());
		handler.update(0.02, {.held = true});
		EXPECT_TRUE(handler.doAction());

		handler.reset();
		handler.update(0.0, {.held = true, .pressed = true});
		handler.setTiming(0.3, 0.1);
		EXPECT_TRUE(handler.doAction());
	}

	TEST(DasArrHandlerTest, rejectsNonPositiveAndNonFiniteTiming) {
		EXPECT_THROW((DasArrHandler{0.0, 0.1}), std::invalid_argument);
		EXPECT_THROW((DasArrHandler{0.1, -0.1}), std::invalid_argument);
		EXPECT_THROW((DasArrHandler{std::numeric_limits<double>::infinity(), 0.1}), std::invalid_argument);
		EXPECT_THROW((DasArrHandler{0.1, std::numeric_limits<double>::quiet_NaN()}), std::invalid_argument);

		DasArrHandler handler{0.1, 0.05};
		EXPECT_THROW(handler.setTiming(-0.1, 0.05), std::invalid_argument);
		EXPECT_DOUBLE_EQ(handler.getDas(), 0.1);
		EXPECT_DOUBLE_EQ(handler.getArr(), 0.05);
	}

}

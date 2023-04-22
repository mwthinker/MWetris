#ifndef MWETRIS_UI_SCENE_EVENT_H
#define MWETRIS_UI_SCENE_EVENT_H

namespace mwetris::ui::scene {

	enum class Event {
		NotDefined,
		Menu,
		ResumePlay,
		Play,
		NetworkPlay,
		HighScore,
		Settings,
		Exit
	};

}

#endif

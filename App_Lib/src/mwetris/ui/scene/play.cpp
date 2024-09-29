#include "play.h"
#include "util/auxiliary.h"

namespace mwetris::ui::scene {

	Play::Play(std::shared_ptr<TetrisController> tetrisController)
		: tetrisController_{std::move(tetrisController)} {
	}

	void Play::imGuiUpdate(const DeltaTime& deltaTime) {
		auto h = ImGui::GetCursorPosY();
		auto size = ImGui::GetWindowSize();
		
		tetrisController_->draw(size.x, size.y - h, util::toSeconds(deltaTime));
	}

}

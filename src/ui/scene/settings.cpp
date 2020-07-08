#include "settings.h"
#include "event.h"

namespace tetris::ui::scene {

	namespace {

		bool ComboAi(const char* name, int& item, const std::vector<Ai>& ais, ImGuiComboFlags flags = 0) {
			int oldItem = item;
			ImGui::ComboScoped(name, ais[item].getName().c_str(), flags, [&]() {
				auto size = ais.size();
				for (int n = 0; n < size; ++n)
				{
					bool isSelected = (item == n);
					if (ImGui::Selectable(ais[n].getName().c_str(), isSelected)) {
						item = n;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
			});
			return oldItem != item;
		}

	}

	void Settings::imGuiUpdate(const std::chrono::high_resolution_clock::duration& deltaTime) {
		auto menuHeight = tetris::TetrisData::getInstance().getWindowBarHeight();
		auto labelColor = tetris::TetrisData::getInstance().getLabelTextColor();

		ImGui::Bar([&]() {
			ImGui::PushButtonStyle();
			if (ImGui::Button("Menu", {100.5f, menuHeight})) {
				emitEvent<Event>(Event::Menu);
			}
			ImGui::PopButtonStyle();
		});

		ImGui::PushFont(tetris::TetrisData::getInstance().getImGuiHeaderFont());
		ImGui::TextColored(labelColor, "Settings");
		ImGui::PopFont();

		ImGui::Indent(10.f);
		ImGui::Dummy({0.0f, 5.0f});

		ImGui::PushFont(tetris::TetrisData::getInstance().getImGuiDefaultFont());
		bool check = tetris::TetrisData::getInstance().isWindowBordered();
		if (ImGui::Checkbox("Border around window", &check)) {
			tetris::TetrisData::getInstance().setWindowBordered(check);
		}

		check = tetris::TetrisData::getInstance().isFullscreenOnDoubleClick();
		if (ImGui::Checkbox("Fullscreen on double click", &check)) {
			tetris::TetrisData::getInstance().setFullscreenOnDoubleClick(check);
		}

		check = tetris::TetrisData::getInstance().isMoveWindowByHoldingDownMouse();
		if (ImGui::Checkbox("Move the window by holding down left mouse button", &check)) {
			tetris::TetrisData::getInstance().setMoveWindowByHoldingDownMouse(check);
		}

		check = tetris::TetrisData::getInstance().isWindowVsync();
		if (ImGui::Checkbox("Vsync", &check)) {
			tetris::TetrisData::getInstance().setWindowVsync(check);
		}

		check = tetris::TetrisData::getInstance().isLimitFps();
		if (ImGui::Checkbox("FpsLimiter", &check)) {
			tetris::TetrisData::getInstance().setLimitFps(check);
		}

		check = tetris::TetrisData::getInstance().isWindowPauseOnLostFocus();
		if (ImGui::Checkbox("Paus on lost focus", &check)) {
			tetris::TetrisData::getInstance().setWindowPauseOnLostFocus(check);
		}

		check = tetris::TetrisData::getInstance().isShowDownBlock();
		if (ImGui::Checkbox("Show down block", &check)) {
			tetris::TetrisData::getInstance().setShowDownBlock(check);
		}

		ImGui::PushFont(tetris::TetrisData::getInstance().getImGuiHeaderFont());
		ImGui::TextColored(labelColor, "Ai players");
		ImGui::PopFont();

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.20f);

		const auto& ais = TetrisData::getInstance().getAiVector();
		int nbrAi1 = std::find_if(ais.begin(), ais.end(), [](const Ai& ai) { return ai.getName() == TetrisData::getInstance().getAi1Name(); }) - ais.begin();
		if (ComboAi("Ai1", nbrAi1, ais)) {
			TetrisData::getInstance().setAi1Name(ais[nbrAi1].getName());
		}
		int nbrAi2 = std::find_if(ais.begin(), ais.end(), [](const Ai& ai) { return ai.getName() == TetrisData::getInstance().getAi2Name(); }) - ais.begin();
		if (ComboAi("Ai2", nbrAi2, ais)) {
			TetrisData::getInstance().setAi2Name(ais[nbrAi2].getName());
		}
		int nbrAi3 = std::find_if(ais.begin(), ais.end(), [](const Ai& ai) { return ai.getName() == TetrisData::getInstance().getAi3Name(); }) - ais.begin();
		if (ComboAi("Ai3", nbrAi3, ais)) {
			TetrisData::getInstance().setAi4Name(ais[nbrAi3].getName());
		}
		int nbrAi4 = std::find_if(ais.begin(), ais.end(), [](const Ai& ai) { return ai.getName() == TetrisData::getInstance().getAi4Name(); }) - ais.begin();
		if (ComboAi("Ai4", nbrAi4, ais)) {
			TetrisData::getInstance().setAi4Name(ais[nbrAi4].getName());
		}

		ImGui::PopItemWidth();
		ImGui::PopFont();
	}

}
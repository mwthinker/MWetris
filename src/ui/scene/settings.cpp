#include "settings.h"
#include "event.h"

namespace mwetris::ui::scene {

	namespace {

		bool ComboAi(const char* name, int& item, const std::vector<tetris::Ai>& ais, ImGuiComboFlags flags = 0) {
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

	void Settings::imGuiUpdate(const DeltaTime& deltaTime) {
		auto menuHeight = mwetris::Configuration::getInstance().getWindowBarHeight();
		auto labelColor = mwetris::Configuration::getInstance().getLabelTextColor();

		ImGui::Bar([&]() {
			ImGui::PushButtonStyle();
			if (ImGui::Button("Menu", {100.5f, menuHeight})) {
				emitEvent(Event::Menu);
			}
			ImGui::PopButtonStyle();
		});

		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::TextColored(labelColor, "Settings");
		ImGui::PopFont();

		ImGui::Indent(10.f);
		ImGui::Dummy({0.0f, 5.0f});

		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiDefaultFont());
		bool check = mwetris::Configuration::getInstance().isWindowBordered();
		if (ImGui::Checkbox("Border around window", &check)) {
			mwetris::Configuration::getInstance().setWindowBordered(check);
		}

		check = mwetris::Configuration::getInstance().isFullscreenOnDoubleClick();
		if (ImGui::Checkbox("Fullscreen on double click", &check)) {
			mwetris::Configuration::getInstance().setFullscreenOnDoubleClick(check);
		}

		check = mwetris::Configuration::getInstance().isMoveWindowByHoldingDownMouse();
		if (ImGui::Checkbox("Move the window by holding down left mouse button", &check)) {
			mwetris::Configuration::getInstance().setMoveWindowByHoldingDownMouse(check);
		}

		check = mwetris::Configuration::getInstance().isWindowVsync();
		if (ImGui::Checkbox("Vsync", &check)) {
			mwetris::Configuration::getInstance().setWindowVsync(check);
		}

		check = mwetris::Configuration::getInstance().isLimitFps();
		if (ImGui::Checkbox("FpsLimiter", &check)) {
			mwetris::Configuration::getInstance().setLimitFps(check);
		}

		check = mwetris::Configuration::getInstance().isWindowPauseOnLostFocus();
		if (ImGui::Checkbox("Paus on lost focus", &check)) {
			mwetris::Configuration::getInstance().setWindowPauseOnLostFocus(check);
		}

		check = mwetris::Configuration::getInstance().isShowDownBlock();
		if (ImGui::Checkbox("Show down block", &check)) {
			mwetris::Configuration::getInstance().setShowDownBlock(check);
		}

		ImGui::PushFont(mwetris::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::TextColored(labelColor, "Ai players");
		ImGui::PopFont();

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.20f);

		const auto& ais = Configuration::getInstance().getAiVector();
		int nbrAi1 = std::find_if(ais.begin(), ais.end(), [](const tetris::Ai& ai) { return ai.getName() == Configuration::getInstance().getAi1Name(); }) - ais.begin();
		if (ComboAi("Ai1", nbrAi1, ais)) {
			Configuration::getInstance().setAi1Name(ais[nbrAi1].getName());
		}
		int nbrAi2 = std::find_if(ais.begin(), ais.end(), [](const tetris::Ai& ai) { return ai.getName() == Configuration::getInstance().getAi2Name(); }) - ais.begin();
		if (ComboAi("Ai2", nbrAi2, ais)) {
			Configuration::getInstance().setAi2Name(ais[nbrAi2].getName());
		}
		int nbrAi3 = std::find_if(ais.begin(), ais.end(), [](const tetris::Ai& ai) { return ai.getName() == Configuration::getInstance().getAi3Name(); }) - ais.begin();
		if (ComboAi("Ai3", nbrAi3, ais)) {
			Configuration::getInstance().setAi4Name(ais[nbrAi3].getName());
		}
		int nbrAi4 = std::find_if(ais.begin(), ais.end(), [](const tetris::Ai& ai) { return ai.getName() == Configuration::getInstance().getAi4Name(); }) - ais.begin();
		if (ComboAi("Ai4", nbrAi4, ais)) {
			Configuration::getInstance().setAi4Name(ais[nbrAi4].getName());
		}

		ImGui::PopItemWidth();
		ImGui::PopFont();
	}

}

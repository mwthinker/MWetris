#include "settings.h"
#include "types.h"
#include "configuration.h"
#include "ui/imguiextra.h"

namespace app::ui::scene {

	namespace {

		bool ComboAi(const char* name, int& item, const std::vector<Configuration::Ai>& ais, ImGuiComboFlags flags = 0) {
			int oldItem = item;
			ImGui::ComboScoped(name, ais[item].name.c_str(), flags, [&]() {
				auto size = ais.size();
				for (int n = 0; n < size; ++n)
				{
					bool isSelected = (item == n);
					if (ImGui::Selectable(ais[item].name.c_str(), isSelected)) {
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
		auto labelColor = app::Configuration::getInstance().getLabelTextColor();

		ImGui::PushFont(app::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::TextColored(labelColor, "Settings");
		ImGui::PopFont();

		ImGui::Indent(10.f);
		ImGui::Dummy({0.0f, 5.0f});

		ImGui::PushFont(app::Configuration::getInstance().getImGuiDefaultFont());
		bool check = app::Configuration::getInstance().isWindowBordered();
		if (ImGui::Checkbox("Border around window", &check)) {
			app::Configuration::getInstance().setWindowBordered(check);
		}

		check = app::Configuration::getInstance().isFullscreenOnDoubleClick();
		if (ImGui::Checkbox("Fullscreen on double click", &check)) {
			app::Configuration::getInstance().setFullscreenOnDoubleClick(check);
		}

		check = app::Configuration::getInstance().isMoveWindowByHoldingDownMouse();
		if (ImGui::Checkbox("Move the window by holding down left mouse button", &check)) {
			app::Configuration::getInstance().setMoveWindowByHoldingDownMouse(check);
		}

		check = app::Configuration::getInstance().isWindowVsync();
		if (ImGui::Checkbox("Vsync", &check)) {
			app::Configuration::getInstance().setWindowVsync(check);
		}

		check = app::Configuration::getInstance().isLimitFps();
		if (ImGui::Checkbox("FpsLimiter", &check)) {
			app::Configuration::getInstance().setLimitFps(check);
		}

		check = app::Configuration::getInstance().isWindowPauseOnLostFocus();
		if (ImGui::Checkbox("Paus on lost focus", &check)) {
			app::Configuration::getInstance().setWindowPauseOnLostFocus(check);
		}

		check = app::Configuration::getInstance().isShowDownBlock();
		if (ImGui::Checkbox("Show down block", &check)) {
			app::Configuration::getInstance().setShowDownBlock(check);
		}

		ImGui::PushFont(app::Configuration::getInstance().getImGuiHeaderFont());
		ImGui::TextColored(labelColor, "Ai players");
		ImGui::PopFont();

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.20f);

		const auto& ais = Configuration::getInstance().getAiVector();
		int nbrAi1 = std::find_if(ais.begin(), ais.end(), [](const Configuration::Ai& ai) { return ai.name == Configuration::getInstance().getAi1Name(); }) - ais.begin();
		if (ComboAi("Ai1", nbrAi1, ais)) {
			Configuration::getInstance().setAi1Name(ais[nbrAi1].name);
		}
		int nbrAi2 = std::find_if(ais.begin(), ais.end(), [](const Configuration::Ai& ai) { return ai.name == Configuration::getInstance().getAi2Name(); }) - ais.begin();
		if (ComboAi("Ai2", nbrAi2, ais)) {
			Configuration::getInstance().setAi2Name(ais[nbrAi2].name);
		}
		int nbrAi3 = std::find_if(ais.begin(), ais.end(), [](const Configuration::Ai& ai) { return ai.name == Configuration::getInstance().getAi3Name(); }) - ais.begin();
		if (ComboAi("Ai3", nbrAi3, ais)) {
			Configuration::getInstance().setAi4Name(ais[nbrAi3].name);
		}
		int nbrAi4 = std::find_if(ais.begin(), ais.end(), [](const Configuration::Ai& ai) { return ai.name == Configuration::getInstance().getAi4Name(); }) - ais.begin();
		if (ComboAi("Ai4", nbrAi4, ais)) {
			Configuration::getInstance().setAi4Name(ais[nbrAi4].name);
		}

		ImGui::PopItemWidth();
		ImGui::PopFont();
	}

}

#ifndef MWETRIS_IMGUIEXTRA_H
#define MWETRIS_IMGUIEXTRA_H

#include "types.h"
#include "configuration.h"

#include <sdl/imguiauxiliary.h>

#include <sdl/sprite.h>

#include <chrono>
#include <concepts>

namespace ImGui {

	void PushButtonStyle();

	void PopButtonStyle();

	void Text(const std::chrono::year_month_day& ymd);

	void TextWithBackgroundColor(const char* str, sdl::Color color, const ImVec2& padding = {});

	void TextWithBackgroundColor(int nbr, sdl::Color color, const ImVec2& padding = {});

	void IgnoreCursor(std::invocable auto&& t);

	template<typename T>
	concept HasNameMember = requires(T t) {
		{ t.name };
	};

	template <typename Type> requires HasNameMember<Type>
	const Type& ComboUniqueType(const char* label, const std::vector<Type>& boards, int id = 0) {
		using Pair = std::pair<const char*, int>;
		static std::map<Pair, int> indexByLabel;
		auto& item = indexByLabel[Pair{label, id}];

		ImGui::ComboScoped(label, boards[item].name.c_str(), ImGuiComboFlags_None, [&]() {
			for (int n = 0; n < boards.size(); ++n) {
				bool isSelected = (item == n);
				if (ImGui::Selectable(boards[n].name.c_str(), isSelected)) {
					item = n;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
		});

		return boards[item];
	}

	bool ConfirmationButton(const char* label, const ImVec2& size = {});

	bool AbortButton(const char* label, const ImVec2& size = {});

	void DebugDrawForeground();

}

#endif

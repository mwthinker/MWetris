#ifndef MWETRIS_IMGUIEXTRA_H
#define MWETRIS_IMGUIEXTRA_H

#include "../types.h"
#include "../configuration.h"

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

	bool ConfirmationButton(const char* label, const ImVec2& size = {});

	bool AbortButton(const char* label, const ImVec2& size = {});

	template<typename T>
	concept ComboType = requires(T t) {
		{ t.name } -> std::convertible_to<std::string>;
		std::is_aggregate_v<T>;
	};

	template <ComboType Type>
	class ComboVector {
	public:
		explicit ComboVector(const std::vector<Type>& items)
			: items_{items} {}

		bool imGuiCombo(const char* label, ImGuiComboFlags flags = 0) {
			return ImGui::ComboScoped(label, items_[selected_].name.c_str(), flags, [&]() {
				for (int n = 0; n < items_.size(); ++n) {
					bool isSelected = (selected_ == n);
					if (ImGui::Selectable(items_[n].name.c_str(), isSelected)) {
						selected_ = n;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
			});
		}

		const Type& getSelected() const {
			return items_[selected_];
		}

	private:
		int selected_ = 0;
		std::vector<Type> items_;
	};

}

#endif

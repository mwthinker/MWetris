#ifndef APP_IMGUIEXTRA_H
#define APP_IMGUIEXTRA_H

#include "../types.h"
#include "../configuration.h"

#include <algorithm>
#include <chrono>
#include <cfloat>
#include <concepts>

namespace ImGui {

	void PushButtonStyle();

	void PopButtonStyle();

	void PushContentStyle();

	void PopContentStyle();

	ImVec2 ModalSize(const ImVec2& logicalSize, float viewportFraction = 0.9f);

	float ModalFooterHeight();

	bool BeginModalBody(const char* id, bool border = false);

	void EndModalBody();
	bool MediumButton(const char* label, const ImVec2& size = {});
	bool CloseIconButton(const ImVec2& size = {});
	bool MediumSmallButton(const char* label);
	bool LinkButton(const char* label);
	bool SemiBoldCollapsingHeader(const char* label, ImGuiTreeNodeFlags flags = 0);
	void SecondaryText(const char* format, ...);

	void Text(const std::chrono::year_month_day& ymd);

	void TextWithBackgroundColor(const char* str, sdl::Color color, const ImVec2& padding = {});

	void TextWithBackgroundColor(int nbr, sdl::Color color, const ImVec2& padding = {});

	void IgnoreCursor(std::invocable auto&& t);

	bool ConfirmationButton(const char* label, const ImVec2& size = {});

	bool AbortButton(const char* label, const ImVec2& size = {});

	void FormTable(const char* id, std::invocable auto&& content, float labelWidth = 160.f) {
		if (ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_PadOuterX)) {
			ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, labelWidth);
			ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);
			content();
			ImGui::EndTable();
		}
	}

	void FormRow(const char* label, std::invocable auto&& control) {
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::TextUnformatted(label);
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::PushID(label);
		control();
		ImGui::PopID();
	}

	inline void AlignNextItemRight(float width) {
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.f, ImGui::GetContentRegionAvail().x - width));
	}

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

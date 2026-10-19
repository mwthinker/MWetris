#include "settings.h"

#include "types.h"
#include "ui/controlspresenter.h"
#include "ui/imguiextra.h"

#include <IconsFontAwesome6.h>

#include <algorithm>
#include <array>
#include <string>

namespace app::ui::scene {

	namespace {
		template<typename T>
		void drawDefaultReset(const char* id, T& value, const T& defaultValue) {
			if (value == defaultValue) {
				return;
			}
			ImGui::SameLine();
			ImGui::PushID(id);
			if (ImGui::MediumSmallButton(ICON_FA_ARROW_ROTATE_LEFT)) {
				value = defaultValue;
			}
			if (ImGui::IsItemHovered()) {
				ImGui::PushFont(Configuration::getInstance().getImGuiRegularSmallFont(), 0.f);
				ImGui::SetTooltip("Restore default");
				ImGui::PopFont();
			}
			ImGui::PopID();
		}

		bool colorEdit(const char* label, std::string& color, const std::string* defaultColor) {
			sdl::Color parsed{color};
			ImVec4 value = parsed;
			bool changed = ImGui::ColorEdit4(label, &value.x, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);
			if (changed) {
				color = sdl::Color{value.x, value.y, value.z, value.w}.toHexString();
			}
			if (defaultColor) {
				drawDefaultReset(label, color, *defaultColor);
			}
			return changed;
		}

		void restartOnlyLabel() {
			ImGui::SameLine();
			ImGui::SecondaryText("(restart required)");
		}

		void inputPath(const char* label, std::string& value, const std::string* defaultValue) {
			ImGui::InputText(label, &value);
			if (defaultValue) {
				drawDefaultReset(label, value, *defaultValue);
			}
			restartOnlyLabel();
		}

		bool aiCombo(const char* label, std::string& selected, const std::vector<Configuration::AiDefinition>& definitions, const std::string* defaultValue) {
			bool changed = false;
			ImGui::ComboScoped(label, selected.c_str(), [&]() {
				auto drawOption = [&](const std::string& name) {
					bool isSelected = selected == name;
					if (ImGui::Selectable(name.c_str(), isSelected)) {
						selected = name;
						changed = true;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				};
				drawOption("Default");
				for (const auto& definition : definitions) {
					drawOption(definition.name);
				}
			});
			if (defaultValue) {
				drawDefaultReset(label, selected, *defaultValue);
			}
			return changed;
		}

	}

	Settings::Settings(std::shared_ptr<ControlsPresenter> controlsPresenter, std::function<void()> requestRestart)
		: controlsPresenter_{std::move(controlsPresenter)}
		, requestRestart_{std::move(requestRestart)} {
	}

	void Settings::switchedTo(const SceneData&) {
		showAdvanced_ = false;
		defaults_.reset();
		defaultsError_.clear();
		error_.clear();
		restartReasons_.clear();
		auto defaults = Configuration::getInstance().getDefaultEditableSettings();
		if (defaults) {
			defaults_ = std::move(*defaults);
		} else {
			defaultsError_ = defaults.error();
		}
		try {
			committed_ = Configuration::getInstance().getEditableSettings();
			for (auto& name : committed_.activeAiNames) {
				bool exists = name == "Default" || std::ranges::find(committed_.ais, name, &Configuration::AiDefinition::name) != committed_.ais.end();
				if (!exists) {
					name = "Default";
				}
			}
			draft_ = committed_;
			loaded_ = true;
		} catch (const std::exception& exception) {
			error_ = exception.what();
			loaded_ = false;
		}
	}

	void Settings::switchedFrom() {
		committed_ = {};
		draft_ = {};
		defaults_.reset();
		defaultsError_.clear();
		error_.clear();
		restartReasons_.clear();
		loaded_ = false;
	}

	ImVec2 Settings::getSize() const {
		auto viewportSize = ImGui::GetMainViewport()->WorkSize;
		return {std::min(820.f, viewportSize.x - 20.f), std::min(760.f, viewportSize.y - 20.f)};
	}

	void Settings::drawAiSelectors() {
		if (ImGui::SemiBoldCollapsingHeader("AI players", ImGuiTreeNodeFlags_DefaultOpen)) {
			const auto* defaults = defaults_ ? &*defaults_ : nullptr;
			ImGui::PushItemWidth(260.f);
			for (int index = 0; index < std::ssize(draft_.activeAiNames); ++index) {
				auto label = "AI " + std::to_string(index + 1);
				aiCombo(label.c_str(), draft_.activeAiNames[index], draft_.ais, defaults ? &defaults->activeAiNames[index] : nullptr);
			}
			ImGui::PopItemWidth();
		}
	}

	void Settings::drawBasicSettings() {
		const auto* defaults = defaults_ ? &*defaults_ : nullptr;
		if (ImGui::SemiBoldCollapsingHeader("General / Display", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderFloat("UI scale", &draft_.window.uiScale, 0.5f, 2.f, "%.2fx", ImGuiSliderFlags_AlwaysClamp);
			if (defaults) {
				drawDefaultReset("UI scale", draft_.window.uiScale, defaults->window.uiScale);
			}
			ImGui::Checkbox("Pause on focus loss", &draft_.window.pauseOnLostFocus);
			if (defaults) {
				drawDefaultReset("Pause on focus loss", draft_.window.pauseOnLostFocus, defaults->window.pauseOnLostFocus);
			}
			ImGui::Checkbox("Limit FPS", &draft_.window.limitFps);
			if (defaults) {
				drawDefaultReset("Limit FPS", draft_.window.limitFps, defaults->window.limitFps);
			}
			ImGui::Checkbox("Show ghost block", &draft_.board.showDownBlock);
			if (defaults) {
				drawDefaultReset("Show ghost block", draft_.board.showDownBlock, defaults->board.showDownBlock);
			}
			if (draft_.board.showDownBlock) {
				colorEdit("Ghost block color", draft_.board.downBlockColor, defaults ? &defaults->board.downBlockColor : nullptr);
			}
		}
		drawAiSelectors();
		if (ImGui::SemiBoldCollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
			controlsPresenter_->drawMenu();
			ImGui::SecondaryText("Control profile changes are saved immediately.");
		}
	}

	void Settings::drawCustomAis() {
		if (!ImGui::SemiBoldCollapsingHeader("Custom AI")) {
			return;
		}
		if (defaults_ && draft_.ais != defaults_->ais) {
			ImGui::TextUnformatted("Definitions");
			drawDefaultReset("Custom AI definitions", draft_.ais, defaults_->ais);
		}
		for (int index = 0; index < std::ssize(draft_.ais); ++index) {
			auto& definition = draft_.ais[index];
			ImGui::PushID(index);
			ImGui::PushFont(Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
			ImGui::SeparatorText(definition.name.empty() ? "Unnamed AI" : definition.name.c_str());
			ImGui::PopFont();
			ImGui::InputText("Name", &definition.name);
			ImGui::InputTextMultiline("Value function", &definition.valueFunction, {0.f, 55.f});
			bool inUse = std::ranges::find(draft_.activeAiNames, definition.name) != draft_.activeAiNames.end();
			ImGui::BeginDisabled(inUse);
			if (ImGui::MediumButton("Delete")) {
				draft_.ais.erase(draft_.ais.begin() + index);
				--index;
			}
			ImGui::EndDisabled();
			if (inUse) {
				ImGui::SameLine();
				ImGui::SecondaryText("Remap active AI slots before deleting.");
			}
			ImGui::PopID();
		}
		if (ImGui::MediumButton("Add custom AI")) {
			std::string name{"Custom AI"};
			for (int suffix = 2; std::ranges::find(draft_.ais, name, &Configuration::AiDefinition::name) != draft_.ais.end(); ++suffix) {
				name = "Custom AI " + std::to_string(suffix);
			}
			draft_.ais.push_back({std::move(name), "-1*holes - 0.4*landingHeight"});
		}
	}

	void Settings::drawAdvancedSettings() {
		const auto* defaults = defaults_ ? &*defaults_ : nullptr;
		if (ImGui::SemiBoldCollapsingHeader("Window", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::InputInt("Width", &draft_.window.width);
			if (defaults) {
				drawDefaultReset("Width", draft_.window.width, defaults->window.width);
			}
			ImGui::InputInt("Height", &draft_.window.height);
			if (defaults) {
				drawDefaultReset("Height", draft_.window.height, defaults->window.height);
			}
			ImGui::InputInt("Minimum width", &draft_.window.minWidth);
			if (defaults) {
				drawDefaultReset("Minimum width", draft_.window.minWidth, defaults->window.minWidth);
			}
			ImGui::InputInt("Minimum height", &draft_.window.minHeight);
			if (defaults) {
				drawDefaultReset("Minimum height", draft_.window.minHeight, defaults->window.minHeight);
			}
			draft_.window.width = std::clamp(draft_.window.width, 320, 7680);
			draft_.window.height = std::clamp(draft_.window.height, 320, 4320);
			draft_.window.minWidth = std::clamp(draft_.window.minWidth, 200, 7680);
			draft_.window.minHeight = std::clamp(draft_.window.minHeight, 200, 4320);
			ImGui::Checkbox("Resizable", &draft_.window.resizable);
			if (defaults) {
				drawDefaultReset("Resizable", draft_.window.resizable, defaults->window.resizable);
			}
			ImGui::Checkbox("Window border", &draft_.window.bordered);
			if (defaults) {
				drawDefaultReset("Window border", draft_.window.bordered, defaults->window.bordered);
			}
			ImGui::Checkbox("Start maximized", &draft_.window.maximized);
			if (defaults) {
				drawDefaultReset("Start maximized", draft_.window.maximized, defaults->window.maximized);
			}
			ImGui::Checkbox("Fullscreen on double-click", &draft_.window.fullscreenOnDoubleClick);
			if (defaults) {
				drawDefaultReset("Fullscreen on double-click", draft_.window.fullscreenOnDoubleClick, defaults->window.fullscreenOnDoubleClick);
			}
			ImGui::Checkbox("Drag window", &draft_.window.moveWindowByHoldingDownMouse);
			if (defaults) {
				drawDefaultReset("Drag window", draft_.window.moveWindowByHoldingDownMouse, defaults->window.moveWindowByHoldingDownMouse);
			}
		}

		if (ImGui::SemiBoldCollapsingHeader("Graphics / Board")) {
			ImGui::Checkbox("VSync", &draft_.window.vsync);
			if (defaults) {
				drawDefaultReset("VSync", draft_.window.vsync, defaults->window.vsync);
			}
			restartOnlyLabel();
			ImGui::SliderFloat("Board border size", &draft_.board.borderSize, 0.f, 20.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
			if (defaults) {
				drawDefaultReset("Board border size", draft_.board.borderSize, defaults->board.borderSize);
			}
			colorEdit("Board border", draft_.board.borderColor, defaults ? &defaults->board.borderColor : nullptr);
			colorEdit("Start area", draft_.board.startAreaColor, defaults ? &defaults->board.startAreaColor : nullptr);
			colorEdit("Outer squares", draft_.board.outerSquareColor, defaults ? &defaults->board.outerSquareColor : nullptr);
			colorEdit("Inner squares", draft_.board.innerSquareColor, defaults ? &defaults->board.innerSquareColor : nullptr);
		}

		if (ImGui::SemiBoldCollapsingHeader("Network")) {
			ImGui::SecondaryText("Changes require restart.");
			ImGui::InputText("Global server address", &draft_.network.global.ip);
			if (defaults) {
				drawDefaultReset("Global server address", draft_.network.global.ip, defaults->network.global.ip);
			}
			ImGui::InputInt("Global server port", &draft_.network.global.port);
			if (defaults) {
				drawDefaultReset("Global server port", draft_.network.global.port, defaults->network.global.port);
			}
			ImGui::InputText("Game server address", &draft_.network.server.ip);
			if (defaults) {
				drawDefaultReset("Game server address", draft_.network.server.ip, defaults->network.server.ip);
			}
			ImGui::InputInt("Game server port", &draft_.network.server.port);
			if (defaults) {
				drawDefaultReset("Game server port", draft_.network.server.port, defaults->network.server.port);
			}
			draft_.network.global.port = std::clamp(draft_.network.global.port, 1, 65535);
			draft_.network.server.port = std::clamp(draft_.network.server.port, 1, 65535);
		}

		if (ImGui::SemiBoldCollapsingHeader("Appearance")) {
			colorEdit("Heading text", draft_.appearance.labelTextColor, defaults ? &defaults->appearance.labelTextColor : nullptr);
			colorEdit("Button text", draft_.appearance.buttonTextColor, defaults ? &defaults->appearance.buttonTextColor : nullptr);
			colorEdit("Button background", draft_.appearance.buttonBackgroundColor, defaults ? &defaults->appearance.buttonBackgroundColor : nullptr);
			colorEdit("Button hover", draft_.appearance.buttonHoverColor, defaults ? &defaults->appearance.buttonHoverColor : nullptr);
			colorEdit("Button active", draft_.appearance.buttonFocusColor, defaults ? &defaults->appearance.buttonFocusColor : nullptr);
			colorEdit("Button border", draft_.appearance.buttonBorderColor, defaults ? &defaults->appearance.buttonBorderColor : nullptr);
		}

		if (ImGui::SemiBoldCollapsingHeader("Assets / Theme")) {
			inputPath("UI font", draft_.assets.windowFont, defaults ? &defaults->assets.windowFont : nullptr);
			inputPath("Board font", draft_.assets.boardFont, defaults ? &defaults->assets.boardFont : nullptr);
			inputPath("Window icon", draft_.assets.icon, defaults ? &defaults->assets.icon : nullptr);
			inputPath("Background", draft_.assets.background, defaults ? &defaults->assets.background : nullptr);
			constexpr std::array labels{"I sprite", "J sprite", "L sprite", "O sprite", "S sprite", "T sprite", "Z sprite"};
			for (int index = 0; index < std::ssize(labels); ++index) {
				inputPath(labels[index], draft_.assets.blockSprites[index], defaults ? &defaults->assets.blockSprites[index] : nullptr);
			}
		}
		drawCustomAis();
	}

	void Settings::apply() {
		auto result = Configuration::getInstance().persistEditableSettings(draft_);
		if (!result) {
			error_ = result.error();
			return;
		}
		committed_ = draft_;
		error_.clear();
		restartReasons_ = result->restartReasons;
		if (restartReasons_.empty()) {
			close();
		}
	}

	void Settings::imGuiUpdate(const DeltaTime&) {
		auto labelColor = Configuration::getInstance().getLabelTextColor();
		ImGui::PushFont(Configuration::getInstance().getImGuiSemiBoldFont(), 0.f);
		ImGui::TextColored(labelColor, "Preferences");
		ImGui::PopFont();

		if (!loaded_) {
			ImGui::TextWrapped("%s", error_.c_str());
			return;
		}

		ImGui::Checkbox("Show advanced settings", &showAdvanced_);
		ImGui::Separator();
		auto draftBeforeRendering = draft_;
		ImGui::BeginChild("SettingsContent", {0.f, -160.f}, true);
		drawBasicSettings();
		if (showAdvanced_) {
			drawAdvancedSettings();
		}
		ImGui::EndChild();
		if (draft_ != draftBeforeRendering) {
			error_.clear();
		}

		auto validation = Configuration::getInstance().validateEditableSettings(draft_);
		if (!error_.empty()) {
			ImGui::TextColored({1.f, 0.35f, 0.35f, 1.f}, "%s", error_.c_str());
		}
		if (!validation) {
			ImGui::TextColored({1.f, 0.35f, 0.35f, 1.f}, "%s", validation.error().c_str());
		}
		bool dirty = draft_ != committed_;
		if (!restartReasons_.empty()) {
			ImGui::TextUnformatted("Saved. Restart required for:");
			ImGui::SameLine();
			ImGui::BeginDisabled(dirty || !requestRestart_);
			if (ImGui::MediumButton("Restart now", {120.f, 30.f})) {
				requestRestart_();
			}
			ImGui::EndDisabled();
			for (const auto& reason : restartReasons_) {
				ImGui::BulletText("%s", reason.c_str());
			}
		}

		ImGui::BeginDisabled(!dirty || !validation);
		if (ImGui::ConfirmationButton("Apply", {100.f, 30.f})) {
			apply();
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!defaults_);
		if (ImGui::MediumButton("Restore Defaults", {140.f, 30.f})) {
			draft_ = *defaults_;
			error_.clear();
		}
		ImGui::EndDisabled();
		if (!defaults_ && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
			ImGui::PushFont(Configuration::getInstance().getImGuiRegularSmallFont(), 0.f);
			ImGui::SetTooltip("Defaults unavailable: %s", defaultsError_.c_str());
			ImGui::PopFont();
		}
	}

}

#include "settings.h"

#include "types.h"
#include "ui/controlspresenter.h"
#include "ui/imguiextra.h"

#include <IconsFontAwesome6.h>

#include <algorithm>
#include <array>
#include <cfloat>
#include <string>

namespace app::ui::scene {

	namespace {
		bool drawResetButton(const char* id) {
			ImGui::PushID(id);
			ImGui::PushFont(Configuration::getInstance().getImGuiDefaultFont(), 0.f);
			const bool reset = ImGui::SmallButton(ICON_FA_ARROW_ROTATE_LEFT);
			ImGui::PopFont();
			if (ImGui::IsItemHovered()) {
				ImGui::PushFont(Configuration::getInstance().getImGuiRegularSmallFont(), 0.f);
				ImGui::SetTooltip("Restore default");
				ImGui::PopFont();
			}
			ImGui::PopID();
			return reset;
		}

		template<typename T>
		void drawDefaultReset(const char* id, T& value, const T& defaultValue) {
			if (value == defaultValue) {
				return;
			}
			ImGui::SameLine();
			if (drawResetButton(id)) {
				value = defaultValue;
			}
		}

		void settingsTable(const char* id, std::invocable auto&& content) {
			if (ImGui::BeginTable(id, 3, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_PadOuterX)) {
				ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, 210.f);
				ImGui::TableSetupColumn("Control", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Reset", ImGuiTableColumnFlags_WidthFixed, 28.f);
				content();
				ImGui::EndTable();
			}
		}

		template<typename T>
		void settingRow(const char* label, T& value, const T* defaultValue, std::invocable auto&& control) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::AlignTextToFramePadding();
			ImGui::TextUnformatted(label);
			ImGui::TableNextColumn();
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::PushID(label);
			control();
			ImGui::PopID();
			ImGui::TableNextColumn();
			if (defaultValue && value != *defaultValue && drawResetButton(label)) {
				value = *defaultValue;
			}
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
		return {860.f, 760.f};
	}

	void Settings::drawAiSelectors() {
		if (ImGui::SemiBoldCollapsingHeader("AI players", ImGuiTreeNodeFlags_DefaultOpen)) {
			const auto* defaults = defaults_ ? &*defaults_ : nullptr;
			settingsTable("AI player settings", [&]() {
				for (int index = 0; index < std::ssize(draft_.activeAiNames); ++index) {
					auto label = "AI " + std::to_string(index + 1);
					settingRow(label.c_str(), draft_.activeAiNames[index], defaults ? &defaults->activeAiNames[index] : nullptr, [&]() {
						aiCombo("##Value", draft_.activeAiNames[index], draft_.ais, nullptr);
					});
				}
			});
		}
	}

	void Settings::drawColorSchemeSelector() {
		auto schemes = Configuration::getColorSchemes();
		auto selected = std::ranges::find(schemes, draft_.board.colorScheme, &Configuration::ColorSchemeInfo::id);
		const char* preview = selected == schemes.end() ? "Custom" : selected->name.data();

		settingsTable("Color scheme setting", [&]() {
			settingRow("Color scheme", draft_.board.colorScheme, static_cast<const std::string*>(nullptr), [&]() {
				ImGui::ComboScoped("##Value", preview, [&]() {
					for (const auto& scheme : schemes) {
						bool isSelected = draft_.board.colorScheme == scheme.id;
						if (ImGui::Selectable(scheme.name.data(), isSelected)) {
							Configuration::applyColorScheme(draft_.board, scheme.id);
						}
						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}
					}
				});
			});
		});
		ImGui::SecondaryText("Individual color changes create a Custom scheme.");
		ImGui::Separator();
	}

	void Settings::drawBasicSettings() {
		const auto* defaults = defaults_ ? &*defaults_ : nullptr;
		drawColorSchemeSelector();
		if (ImGui::SemiBoldCollapsingHeader("General / Display", ImGuiTreeNodeFlags_DefaultOpen)) {
			settingsTable("General display settings", [&]() {
				settingRow("UI scale", draft_.window.uiScale, defaults ? &defaults->window.uiScale : nullptr, [&]() {
					ImGui::SliderFloat("##Value", &draft_.window.uiScale, 0.5f, 2.f, "%.2fx", ImGuiSliderFlags_AlwaysClamp);
				});
				settingRow("Pause on focus loss", draft_.window.pauseOnLostFocus, defaults ? &defaults->window.pauseOnLostFocus : nullptr, [&]() {
					ImGui::Checkbox("##Value", &draft_.window.pauseOnLostFocus);
				});
				settingRow("Limit FPS", draft_.window.limitFps, defaults ? &defaults->window.limitFps : nullptr, [&]() {
					ImGui::Checkbox("##Value", &draft_.window.limitFps);
				});
				settingRow("Show ghost block", draft_.board.showDownBlock, defaults ? &defaults->board.showDownBlock : nullptr, [&]() {
					ImGui::Checkbox("##Value", &draft_.board.showDownBlock);
				});
				if (draft_.board.showDownBlock) {
					settingRow("Ghost block color", draft_.board.downBlockColor, defaults ? &defaults->board.downBlockColor : nullptr, [&]() {
						colorEdit("##Value", draft_.board.downBlockColor, nullptr);
					});
					draft_.board.colorScheme = Configuration::detectColorScheme(draft_.board);
				}
			});
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
			settingsTable("Window settings", [&]() {
				auto inputInt = [&](const char* label, int& value, const int* defaultValue) {
					settingRow(label, value, defaultValue, [&]() { ImGui::InputInt("##Value", &value); });
				};
				inputInt("Width", draft_.window.width, defaults ? &defaults->window.width : nullptr);
				inputInt("Height", draft_.window.height, defaults ? &defaults->window.height : nullptr);
				inputInt("Minimum width", draft_.window.minWidth, defaults ? &defaults->window.minWidth : nullptr);
				inputInt("Minimum height", draft_.window.minHeight, defaults ? &defaults->window.minHeight : nullptr);
				auto checkbox = [&](const char* label, bool& value, const bool* defaultValue) {
					settingRow(label, value, defaultValue, [&]() { ImGui::Checkbox("##Value", &value); });
				};
				checkbox("Resizable", draft_.window.resizable, defaults ? &defaults->window.resizable : nullptr);
				checkbox("Window border", draft_.window.bordered, defaults ? &defaults->window.bordered : nullptr);
				checkbox("Start maximized", draft_.window.maximized, defaults ? &defaults->window.maximized : nullptr);
				checkbox("Fullscreen on double-click", draft_.window.fullscreenOnDoubleClick, defaults ? &defaults->window.fullscreenOnDoubleClick : nullptr);
				checkbox("Drag window", draft_.window.moveWindowByHoldingDownMouse, defaults ? &defaults->window.moveWindowByHoldingDownMouse : nullptr);
			});
			draft_.window.width = std::clamp(draft_.window.width, 320, 7680);
			draft_.window.height = std::clamp(draft_.window.height, 320, 4320);
			draft_.window.minWidth = std::clamp(draft_.window.minWidth, 200, 7680);
			draft_.window.minHeight = std::clamp(draft_.window.minHeight, 200, 4320);
		}

		if (ImGui::SemiBoldCollapsingHeader("Graphics / Board")) {
			settingsTable("Graphics board settings", [&]() {
				settingRow("VSync", draft_.window.vsync, defaults ? &defaults->window.vsync : nullptr, [&]() {
					ImGui::Checkbox("##Value", &draft_.window.vsync);
					restartOnlyLabel();
				});
				settingRow("Board border size", draft_.board.borderSize, defaults ? &defaults->board.borderSize : nullptr, [&]() {
					ImGui::SliderFloat("##Value", &draft_.board.borderSize, 0.f, 20.f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
				});
				auto color = [&](const char* label, std::string& value, const std::string* defaultValue) {
					settingRow(label, value, defaultValue, [&]() { colorEdit("##Value", value, nullptr); });
				};
				color("Board border", draft_.board.borderColor, defaults ? &defaults->board.borderColor : nullptr);
				color("Start area", draft_.board.startAreaColor, defaults ? &defaults->board.startAreaColor : nullptr);
				color("Outer squares", draft_.board.outerSquareColor, defaults ? &defaults->board.outerSquareColor : nullptr);
				color("Inner squares", draft_.board.innerSquareColor, defaults ? &defaults->board.innerSquareColor : nullptr);
				color("Player card", draft_.board.playerCardBackgroundColor, defaults ? &defaults->board.playerCardBackgroundColor : nullptr);
				color("Player card border", draft_.board.playerCardBorderColor, defaults ? &defaults->board.playerCardBorderColor : nullptr);
				color("Sidebar", draft_.board.sidebarBackgroundColor, defaults ? &defaults->board.sidebarBackgroundColor : nullptr);
				color("Sidebar border", draft_.board.sidebarBorderColor, defaults ? &defaults->board.sidebarBorderColor : nullptr);
				color("Next preview", draft_.board.previewBackgroundColor, defaults ? &defaults->board.previewBackgroundColor : nullptr);
				color("Next preview border", draft_.board.previewBorderColor, defaults ? &defaults->board.previewBorderColor : nullptr);
				color("Player text", draft_.board.playerCardTextColor, defaults ? &defaults->board.playerCardTextColor : nullptr);
			});
			draft_.board.colorScheme = Configuration::detectColorScheme(draft_.board);
		}

		if (ImGui::SemiBoldCollapsingHeader("Network")) {
			ImGui::SecondaryText("Changes require restart.");
			settingsTable("Network settings", [&]() {
				auto address = [&](const char* label, std::string& value, const std::string* defaultValue) {
					settingRow(label, value, defaultValue, [&]() { ImGui::InputText("##Value", &value); });
				};
				auto port = [&](const char* label, int& value, const int* defaultValue) {
					settingRow(label, value, defaultValue, [&]() { ImGui::InputInt("##Value", &value); });
				};
				address("Global server address", draft_.network.global.ip, defaults ? &defaults->network.global.ip : nullptr);
				port("Global server port", draft_.network.global.port, defaults ? &defaults->network.global.port : nullptr);
				address("Game server address", draft_.network.server.ip, defaults ? &defaults->network.server.ip : nullptr);
				port("Game server port", draft_.network.server.port, defaults ? &defaults->network.server.port : nullptr);
			});
			draft_.network.global.port = std::clamp(draft_.network.global.port, 1, 65535);
			draft_.network.server.port = std::clamp(draft_.network.server.port, 1, 65535);
		}

		if (ImGui::SemiBoldCollapsingHeader("Appearance")) {
			settingsTable("Appearance settings", [&]() {
				auto color = [&](const char* label, std::string& value, const std::string* defaultValue) {
					settingRow(label, value, defaultValue, [&]() { colorEdit("##Value", value, nullptr); });
				};
				color("Heading text", draft_.appearance.labelTextColor, defaults ? &defaults->appearance.labelTextColor : nullptr);
				color("Button text", draft_.appearance.buttonTextColor, defaults ? &defaults->appearance.buttonTextColor : nullptr);
				color("Button background", draft_.appearance.buttonBackgroundColor, defaults ? &defaults->appearance.buttonBackgroundColor : nullptr);
				color("Button hover", draft_.appearance.buttonHoverColor, defaults ? &defaults->appearance.buttonHoverColor : nullptr);
				color("Button active", draft_.appearance.buttonFocusColor, defaults ? &defaults->appearance.buttonFocusColor : nullptr);
				color("Button border", draft_.appearance.buttonBorderColor, defaults ? &defaults->appearance.buttonBorderColor : nullptr);
			});
		}

		if (ImGui::SemiBoldCollapsingHeader("Assets / Theme")) {
			settingsTable("Asset theme settings", [&]() {
				auto path = [&](const char* label, std::string& value, const std::string* defaultValue) {
					settingRow(label, value, defaultValue, [&]() {
						ImGui::SetNextItemWidth(-130.f);
						inputPath("##Value", value, nullptr);
					});
				};
				path("UI font", draft_.assets.windowFont, defaults ? &defaults->assets.windowFont : nullptr);
				path("Board font", draft_.assets.boardFont, defaults ? &defaults->assets.boardFont : nullptr);
				path("Window icon", draft_.assets.icon, defaults ? &defaults->assets.icon : nullptr);
				path("Background", draft_.assets.background, defaults ? &defaults->assets.background : nullptr);
				constexpr std::array labels{"I sprite", "J sprite", "L sprite", "O sprite", "S sprite", "T sprite", "Z sprite"};
				for (int index = 0; index < std::ssize(labels); ++index) {
					path(labels[index], draft_.assets.blockSprites[index], defaults ? &defaults->assets.blockSprites[index] : nullptr);
				}
			});
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
		ImGui::BeginModalBody("SettingsContent", true);
		drawBasicSettings();
		if (showAdvanced_) {
			drawAdvancedSettings();
		}
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
		ImGui::EndModalBody();

		constexpr float restoreButtonWidth = 140.f;
		constexpr float applyButtonWidth = 100.f;
		const float actionWidth = restoreButtonWidth + applyButtonWidth + ImGui::GetStyle().ItemSpacing.x;
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.f, ImGui::GetContentRegionAvail().x - actionWidth));
		ImGui::BeginDisabled(!defaults_);
		if (ImGui::MediumButton("Restore Defaults", {restoreButtonWidth, 0.f})) {
			draft_ = *defaults_;
			error_.clear();
		}
		ImGui::EndDisabled();
		if (!defaults_ && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
			ImGui::PushFont(Configuration::getInstance().getImGuiRegularSmallFont(), 0.f);
			ImGui::SetTooltip("Defaults unavailable: %s", defaultsError_.c_str());
			ImGui::PopFont();
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(!dirty || !validation);
		if (ImGui::ConfirmationButton("Apply", {applyButtonWidth, 0.f})) {
			apply();
		}
		ImGui::EndDisabled();
	}

}

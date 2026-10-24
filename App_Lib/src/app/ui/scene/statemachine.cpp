#include "statemachine.h"

#include "../imguiextra.h"

namespace app::ui::scene {

	StateMachine::StateMachine(StateType stateType)
		: stateType_{stateType} {

		emplace<scene::EmptyScene>();
	}

	StateMachine::~StateMachine() {
		if (currentKey_ != 0) {
			scenes_[currentKey_]->switchedFrom();
		}
	}

	void StateMachine::imGuiUpdate(const DeltaTime& deltaTime) {
		ImGui::PushID(this);
		if (stateType_ == StateType::Modal) {
			constexpr auto popUpId = "Popup";
			if (openPopUp_) {
				openPopUp_ = false;
				ImGui::OpenPopup(popUpId);
			}
			auto size = currentKey_ ? scenes_[currentKey_]->getSize() : ImVec2{};
			ImGui::SetNextModalWindow(size.x > 0 && size.y > 0 ? size : ImVec2{800.f, 800.f});
			ImGui::PushContentStyle();
			const ImGuiWindowFlags sceneFlags = currentKey_ ? scenes_[currentKey_]->getWindowFlags() : ImGuiWindowFlags_None;
			ImGui::PopupModal(popUpId, nullptr,
				ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | sceneFlags, [&]() {
				auto pos = ImGui::GetCursorScreenPos();
				const auto closeButtonLength = ImGui::GetFrameHeight();
				const auto size = ImVec2{closeButtonLength, closeButtonLength};

				ImGui::SetCursorPosX(ImGui::GetWindowWidth() - size.x - ImGui::GetStyle().WindowPadding.x);
				if (ImGui::CloseIconButton(size) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
					requestClose();
				}
				if (closePopUp_) {
					closePopUp_ = false;
					ImGui::CloseCurrentPopup();
				}

				ImGui::SetCursorScreenPos(pos);

				if (currentKey_) {
					scenes_[currentKey_]->imGuiUpdate(deltaTime);
				}
			});
			ImGui::PopContentStyle();
			if (closeRequested_) {
				closeRequested_ = false;
				switchTo<EmptyScene>();
			}
		} else if (stateType_ == StateType::Fullscreen) {
			if (currentKey_) {
				scenes_[currentKey_]->imGuiUpdate(deltaTime);
			}
		}
		ImGui::PopID();
	}

	void StateMachine::requestClose() {
		closeRequested_ = true;
		closePopUp_ = true;
	}

	void Scene::close() {
		if (stateMachine_) {
			stateMachine_.stateMachine_->requestClose();
		}
	}
	
	Scene::StateMachineWrapper::StateMachineWrapper(StateMachine* stateMachine)
		: stateMachine_{stateMachine} {
	}

	Scene::StateMachineWrapper::operator bool() const {
		return stateMachine_ != nullptr;
	}

}

#include "statemachine.h"

#include "../imguiextra.h"

namespace mwetris::ui::scene {

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
				ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, {0.5f, 0.5f});
				
				auto size = currentKey_ ? scenes_[currentKey_]->getSize() : ImVec2{};
				if (size.x > 0 && size.y > 0) {
					ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);
				} else {
					ImGui::SetNextWindowSize({800, 800}, ImGuiCond_Appearing);
				}
			}
			ImGui::PopupModal(popUpId, nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar, [&]() {
				auto pos = ImGui::GetCursorScreenPos();
				auto size = ImVec2{60.f, 30.f};

				ImGui::SetCursorPosX(ImGui::GetWindowWidth() - size.x - 10.f); // TODO! Fix more less hard coded right alignment.
				if (ImGui::AbortButton("Cancel", size) || ImGui::IsKeyDown(ImGuiKey_Escape)) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::SetCursorScreenPos(pos);

				if (currentKey_) {
					scenes_[currentKey_]->imGuiUpdate(deltaTime);
				}
			});
		} else if (stateType_ == StateType::Fullscreen) {
			if (currentKey_) {
				scenes_[currentKey_]->imGuiUpdate(deltaTime);
			}
		}
		ImGui::PopID();
	}
	
	Scene::StateMachineWrapper::StateMachineWrapper(StateMachine* stateMachine)
		: stateMachine_{stateMachine} {
	}

	Scene::StateMachineWrapper::operator bool() const {
		return stateMachine_ != nullptr;
	}

}

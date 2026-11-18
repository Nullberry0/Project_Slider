#include <SDL3/SDL.h>
#include <iostream> 

#include "Types.h"
#include "UI.h"
#include "Editor.h"
#include "Renderer.h"

using std::string;
using std::vector;
using std::visit;
using std::holds_alternative;
using std::is_same_v;
using std::decay_t;
using std::cout;
using std::endl;

void HandleUIElements(RenderVars renderVars, vector<UIElement> &UIElements, InputState inputState, MenuState &menuState){
    for(auto &element: UIElements){
        visit([&](auto &ui) {
            if(ButtonVisibility(element, menuState) == true){
                ui.properties.visible = true;
                Vector2 pos;
                Size2D size;
                ConvertUIToScreen(ui.properties.anchor, ui.properties.pixelOffset, ui.properties.relativeSize, ui.properties.pixelSize, renderVars, pos, size);
                if(IsPointInBox(pos, pos + size, inputState.mousePos)){
                    if(inputState.leftMouse == MouseState::Pressed || inputState.leftMouse == MouseState::Holding ){
                        ui.isBeingPressed = true;
                    }
                    else{
                        ui.isBeingPressed = false;
                    }
                    if constexpr (is_same_v<decay_t<decltype(ui)>, Button>) { // only compile if, decay_t removes the & so button& becomes button
                        if(inputState.leftMouse == MouseState::Released){
                            if(ui.func == ButtonFunc::StartEditing){
                                menuState = MenuState::Editing;
                            }
                            if(ui.func == ButtonFunc::InsertRect){
                                // insert a rect here;
                            }
                        }
                    }
                }
                else{
                    if(inputState.leftMouse == MouseState::Released){
                        ui.isBeingPressed = false;
                    }
                }
            }
            else{
                ui.properties.visible = false;
            }
        }, element);
    }
}

bool ButtonVisibility(UIElement element, MenuState menuState){
    bool answer = false;

    visit([&](auto& ui) {
        if (ui.group == ButtonGroup::StartPage) {
            if(menuState == MenuState::StartMenu){
                answer = true;
                return;
            }
            else{
                answer = false;
                return;
            }
        }
        if(ui.group == ButtonGroup::MainEditing){
            if(menuState == MenuState::Editing){
                answer = true;
                return;
            }
            else {
                answer = false;
                return;
            }
        }
        else{
            answer = true;
            return;
        }
    }, element);
    return answer;
}
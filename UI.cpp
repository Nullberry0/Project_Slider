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

void HandleUIElements(RenderVars renderVars, vector<UIElement> &UIElements, InputState inputState, ButtonModifiableValues &vars){
    for(auto &element: UIElements){
        visit([&](auto &ui) {
            if(ButtonVisibility(element, vars.menuState) == true){

                ui.properties.isVisible = true;
                Vector2 pos;
                Size2D size;
                ConvertUIToScreen(ui.properties.anchor, ui.properties.pixelOffset, ui.properties.relativeSize, ui.properties.pixelSize, renderVars, pos, size);

                if constexpr (is_same_v<decay_t<decltype(ui)>, Dropdown>){
                    if(ui.isOpen){
                        Vector2 checkPos = pos;
                        checkPos.x += size.width/2;
                        checkPos.y += size.height/2; // finds the bottom
                        for(size_t i = 0; i < ui.options.size(); i++){ // loops through each option to draw it
                            DropdownOption &option = ui.options.at(i);
                            
                            checkPos.y += ui.distBetweenOptions; // now at the top of the option we want to draw             
                            checkPos.y += option.size.height /2;

                            checkPos.x -= option.size.width/2;

                            if(IsPointInBox(checkPos, checkPos + option.size, inputState.mousePos)){
                                if(inputState.leftMouse == MouseState::Pressed || inputState.leftMouse == MouseState::Holding){
                                    if(inputState.leftMouse == MouseState::Pressed){
                                        option.isBeingPressed = true;
                                    }
                                }
                                else option.isBeingPressed = false;
                                if(inputState.leftMouse == MouseState::Released){
                                    if(option.func == ButtonFunc::InsertRect){
                                        vars.slideshow[vars.pageIndex].fileElements.push_back({
                                            RectElement{
                                                ElementBasics{
                                                    10, // temporary, will need to be real ID
                                                    inputState.mousePos,
                                                    Size2D{
                                                        200, 
                                                        200
                                                    },
                                                    Color{
                                                        255, 255, 255, 255
                                                    }
                                                },
                                                ElementEditorState{},
                                                0
                                            }
                                        });
                                    }
                                }
                            }
                            else{
                                if(inputState.leftMouse == MouseState::Released){
                                    option.isBeingPressed = false;
                                }
                            }

                            checkPos.x += option.size.width/2;
                            checkPos.y += option.size.height /2; // if you do the math you will see that this is needed
                        }
                    }
                }
                if(IsPointInBox(pos, pos + size, inputState.mousePos)){
                    if((inputState.leftMouse == MouseState::Pressed || inputState.leftMouse == MouseState::Holding)){
                        if(inputState.leftMouse == MouseState::Pressed){
                            ui.isBeingPressed = true;
                        }
                    }
                    else{
                        ui.isBeingPressed = false;
                    }
                    
                    if(inputState.leftMouse == MouseState::Released){
                        if constexpr (is_same_v<decay_t<decltype(ui)>, Button>) { // only compile if, decay_t removes the & so button& becomes button
                            if(ui.func == ButtonFunc::StartEditing){
                                vars.menuState = MenuState::Editing;
                            }
                        }
                        else if constexpr (is_same_v<decay_t<decltype(ui)>, Dropdown>){
                            ui.isOpen = !ui.isOpen;
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
                ui.properties.isVisible = false;
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
#include <SDL3/SDL.h>
#include <iostream> 

#include "Types.h"
#include "UI.h"
#include "Editor.h"
#include "Renderer.h"

using std::string;
using std::vector;

void HandleButtons(RenderVars renderVars, vector<Button> &buttons, InputState inputState, MenuState &menuState){
    for(auto &button : buttons){
        if(ButtonVisibility(button, menuState)){
            button.visible = true;
            Vector2 pos;
            Size2D size;
            ConvertUIToScreen(button.anchor, button.pixelOffset, button.relativeSize, button.pixelSize, button.preserveAspect, renderVars, pos, size);
            if(IsPointInBox(pos, pos + size, inputState.mousePos)){
                if(inputState.leftMouse == MouseState::Pressed || inputState.leftMouse == MouseState::Holding ){
                    button.isBeingPressed = true;
                }
                else{
                    button.isBeingPressed = false;
                }
                if(inputState.leftMouse == MouseState::Released){
                    if(button.func == ButtonFunc::StartEditing){
                        menuState = MenuState::Editing;
                    }
                }
            }
            else{
                if(inputState.leftMouse == MouseState::Released){
                    button.isBeingPressed = false;
                }
            }
        }
        else{
            button.visible = false;
        }
    }
}

bool ButtonVisibility(Button button, MenuState menuState){
    if(button.group == ButtonGroup::StartPage){
        if(menuState == MenuState::StartMenu){
            return true;
        }
        else{
            return false;
        }
    }
    else return true;
}
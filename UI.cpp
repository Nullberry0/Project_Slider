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
        Vector2 pos;
        Size2D size;
        ConvertUIToScreen(button.anchor, button.pixelOffset, button.relativeSize, button.pixelSize, renderVars, pos, size);
        if(IsPointInBox(pos, pos + size, inputState.mousePos)){
            if(inputState.leftMouse == MouseState::Pressed || inputState.leftMouse == MouseState::Holding ){
                button.isBeingPressed = true;
                if(inputState.leftMouse == MouseState::Pressed){
                    if(button.func == ButtonFunc::StartEditing){
                        menuState = MenuState::Editing;
                    }
                }
            }
            else{
                button.isBeingPressed = false;
            }
        }
    }
}

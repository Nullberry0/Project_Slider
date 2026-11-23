#include <SDL3/SDL.h>
#include <nlohmann/json.hpp> // Run json files
#include <iostream> // Printing
#include <algorithm>
#include <vector>
#include <SDL3_ttf/SDL_ttf.h> // Text render engine
#include <SDL3_image/SDL_image.h>

#include "Types.h"
#include "Editor.h"
#include "Renderer.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::unordered_map;
using std::visit;
using std::holds_alternative;
using std::is_same_v;
using std::decay_t;

// A Struct declared in Editor.h

InputState RecordEditorInput(SDL_Event event){

    InputState answer{};

    answer.leftMouse = MouseState::None; // If it isnt any of the others, it will be set to none

    SDL_GetMouseState(&answer.mousePos.x, &answer.mousePos.y);

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if(event.button.button == SDL_BUTTON_LEFT){
            answer.leftMouse = MouseState::Pressed;
        }
    }

    else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if(event.button.button == SDL_BUTTON_LEFT){
            answer.leftMouse = MouseState::Released;
        }
    }  

    answer.keyStroke = Keystroke::None;

    if (event.type == SDL_EVENT_KEY_DOWN){
        if(event.key.key == SDLK_DELETE) {
            answer.keyStroke = Keystroke::Delete;
        }
    }

    return answer;
}

void UpdateEditor(vector<Slide> &slideshow, int currentPage, InputState input, RenderVars renderVars, unordered_map<string, unordered_map<int, TTF_Font*>> &fonts){

    auto& elements = slideshow[currentPage].fileElements;

    for (auto& fileElement : elements)
    {
        visit([&](auto& element){
            using type = decay_t<decltype(element)>;

            Size2D size = element.basics.size;

            Vector2 topLeftPos = element.basics.pos; // not yet the top left

            if constexpr (is_same_v<type, RectElement> || is_same_v<type, ImageElement>)
            {
                ConvertCanvasToScreen(size, topLeftPos, renderVars, true);
            }
            else if constexpr (is_same_v<type, TextElement>){
                size = {0,0};

                TTF_Font* font = GetFont(element.fontSrc, element.fontSize, fonts);

                TTF_GetStringSize(font, element.content.c_str(), 0, &size.width, &size.height);

                ConvertCanvasToScreen(size, topLeftPos, renderVars, false);

            }
            if(IsPointInBox(topLeftPos, topLeftPos + size, input.mousePos)){
                if(input.leftMouse == MouseState::Pressed){
                    Vector2 offset = (renderVars.windowSize - renderVars.viewportSize) / 2.0f + renderVars.appliedViewportOffset;

                    Vector2 canvasMousePos{ Map(input.mousePos.x - offset.x, 0, renderVars.viewportSize.width, 0, renderVars.canvasSize.width),
                        Map(input.mousePos.y - offset.y, 0, renderVars.viewportSize.height, 0, renderVars.canvasSize.height)};

                    element.editorState.selectedOffset = element.basics.pos - canvasMousePos;
                    element.editorState.isSelected = true;

                }
            }   
            else{
                if(input.leftMouse == MouseState::Pressed){
                    element.editorState.isSelected = false;
                }
            }
            if(element.editorState.isSelected && input.leftMouse == MouseState::Holding) // change the elements position if it is being held
            {
                Vector2 offset = (renderVars.windowSize - renderVars.viewportSize) / 2.0f + renderVars.appliedViewportOffset;

                element.basics.pos.x = Map(input.mousePos.x - offset.x, 0, renderVars.viewportSize.width, 0, renderVars.canvasSize.width) + element.editorState.selectedOffset.x;
                element.basics.pos.y = Map(input.mousePos.y - offset.y, 0, renderVars.viewportSize.height, 0, renderVars.canvasSize.height) + element.editorState.selectedOffset.y;

            }
        }, fileElement);
    }

}

float Map(float value, float inputStart, float inputEnd, float outputStart, float outputEnd){
    return outputStart + ((outputEnd - outputStart) / (inputEnd - inputStart)) * (value - inputStart);
}

bool IsPointInBox(Vector2 pos1, Vector2 pos2, Vector2 point){
    if(pos1.x < point.x && pos2.x > point.x && 
        point.y > pos1.y && pos2.y > point.y){
            return true;
    }
    else return false;
}
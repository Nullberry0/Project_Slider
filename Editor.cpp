#include <SDL3/SDL.h>
#include <nlohmann/json.hpp> // Run json files
#include <iostream> // Printing
#include <algorithm>
#include <vector>

#include "Renderer.h"
#include "Editor.h"

using std::cout;
using std::endl;

// A Struct declared in Editor.h

InputState RecordEditorInput(SDL_Event event){

    InputState answer{};

    Uint32 mouseButtonsMask = SDL_GetMouseState(&answer.mousePos.x, &answer.mousePos.y); // This returns a mask of all the mouse buttons pressed, as well as get the mouse X and Y

    answer.leftMouse = MouseState::None; // If it isnt any of the others, it will be set to none

    if((mouseButtonsMask & SDL_BUTTON_LMASK) != 0){ // If the left mouse button isnt masked out, it is being held
        answer.leftMouse = MouseState::Holding;
    }

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

void UpdateEditor(nlohmann::json& jsonFile, int currentPage, InputState input, RenderVars renderVars, std::vector<int>& selectedElements){

    auto& elements = jsonFile["pages"][currentPage]["elements"];

    for (size_t i = 0; i < elements.size(); i++)
    {
        auto& element = elements[i];

        if(element["type"] == "rect")
        {
            Size2D size{element["width"].get<int>(), element["height"].get<int>()};
            size = size / renderVars.canvasSize * renderVars.viewportSize;

            Vector2 topLeftPos{element["x"].get<float>(), element["y"].get<float>()};

            topLeftPos = ((renderVars.windowSize - renderVars.viewportSize)/2) + topLeftPos / renderVars.canvasSize * renderVars.viewportSize;

            topLeftPos += renderVars.windowSize * renderVars.appliedViewportOffset - (size / 2); // offset

            if(topLeftPos.x < input.mousePos.x && topLeftPos.x + size.width > input.mousePos.x && 
                input.mousePos.y > topLeftPos.y && topLeftPos.y + size.height > input.mousePos.y){ // box collision

                if(input.leftMouse == MouseState::Pressed){
                    if(std::ranges::find(selectedElements, i) == selectedElements.end()){
                        selectedElements.push_back(i);  // The elements index is NOT being selected
                    } 
                    else std::erase(selectedElements, i); //If you click the element and it is already selected
                }
            }

            if(std::ranges::find(selectedElements, i) != selectedElements.end())
            {
                Vector2 offset = (renderVars.windowSize - renderVars.viewportSize) / 2.0f + (renderVars.windowSize * renderVars.appliedViewportOffset);

                element["x"] = Map(input.mousePos.x - offset.x, 0, renderVars.viewportSize.width, 0, renderVars.canvasSize.width);
                element["y"] = Map(input.mousePos.y - offset.y, 0, renderVars.viewportSize.height, 0, renderVars.canvasSize.height);
            }     
        }
    }
}

float Map(float value, float inputStart, float inputEnd, float outputStart, float outputEnd){
    return outputStart + ((outputEnd - outputStart) / (inputEnd - inputStart)) * (value - inputStart);
}
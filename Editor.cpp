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

void UpdateEditor(nlohmann::json& jsonFile, int currentPage, InputState input, RenderVars renderVars, std::vector<int>& selectedElements, std::unordered_map<uint64_t, Vector2> &selectedElementOffsets){

    auto& elements = jsonFile["pages"][currentPage]["elements"];

    bool isHoveringSomething = false;

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
                isHoveringSomething = true;

                if(input.leftMouse == MouseState::Pressed){
                    if(std::ranges::find(selectedElements, element["id"].get<int>()) == selectedElements.end()){ // checks if it isnt already selected

                        selectedElements.push_back(element["id"].get<int>());
                    } 
                    Vector2 offset = (renderVars.windowSize - renderVars.viewportSize) / 2.0f + (renderVars.windowSize * renderVars.appliedViewportOffset);

                    Vector2 canvasMousePos{ Map(input.mousePos.x - offset.x, 0, renderVars.viewportSize.width, 0, renderVars.canvasSize.width),  
                        Map(input.mousePos.y - offset.y, 0, renderVars.viewportSize.height, 0, renderVars.canvasSize.height)};

                    selectedElementOffsets[element["id"].get<int>()] = Vector2{element["x"].get<float>(), element["y"].get<float>()} - canvasMousePos;
                }
            }
            else{
                if(input.leftMouse == MouseState::Pressed){
                    if(isHoveringSomething == false){
                         selectedElements = {};
                    }
                    else std::erase(selectedElements, element["id"].get<int>());
                    
                }
            }

            if(std::ranges::find(selectedElements, element["id"].get<int>()) != selectedElements.end() && input.leftMouse == MouseState::Holding)
            {
                Vector2 offset = (renderVars.windowSize - renderVars.viewportSize) / 2.0f + (renderVars.windowSize * renderVars.appliedViewportOffset);

                element["x"] = Map(input.mousePos.x - offset.x, 0, renderVars.viewportSize.width, 0, renderVars.canvasSize.width) + selectedElementOffsets[element["id"].get<int>()].x;
                element["y"] = Map(input.mousePos.y - offset.y, 0, renderVars.viewportSize.height, 0, renderVars.canvasSize.height) + selectedElementOffsets[element["id"].get<int>()].y;
            }     
        }
    }
}

float Map(float value, float inputStart, float inputEnd, float outputStart, float outputEnd){
    return outputStart + ((outputEnd - outputStart) / (inputEnd - inputStart)) * (value - inputStart);
}
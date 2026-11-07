#include <SDL3/SDL.h>
#include <nlohmann/json.hpp> // Run json files
#include <iostream> // Printing
#include <algorithm>
#include <vector>
#include <SDL3_ttf/SDL_ttf.h> // Text render engine
#include <SDL3_image/SDL_image.h>

#include "Types.h"
#include "Editor.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::unordered_map;

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

void UpdateEditor(nlohmann::json& jsonFile, int currentPage, InputState input, RenderVars renderVars, std::unordered_map<uint64_t, Vector2> &selectedElementOffsets, unordered_map<string, unordered_map<int, TTF_Font*>> &fonts){

    auto& elements = jsonFile["pages"][currentPage]["elements"];

    for (size_t i = 0; i < elements.size(); i++)
    {
        auto& element = elements[i];

        if(element["type"] == "rect" || element["type"] == "image")
        {
            Size2D size{element["width"].get<int>(), element["height"].get<int>()};

            Vector2 topLeftPos{element["x"].get<float>(), element["y"].get<float>()};

            ConvertCanvasToScreen(size, topLeftPos, renderVars, true);

            if(topLeftPos.x < input.mousePos.x && topLeftPos.x + size.width > input.mousePos.x && 
                input.mousePos.y > topLeftPos.y && topLeftPos.y + size.height > input.mousePos.y){ // box collision

                if(input.leftMouse == MouseState::Pressed){
                    Vector2 offset = (renderVars.windowSize - renderVars.viewportSize) / 2.0f + (renderVars.windowSize * renderVars.appliedViewportOffset);

                    Vector2 canvasMousePos{ Map(input.mousePos.x - offset.x, 0, renderVars.viewportSize.width, 0, renderVars.canvasSize.width),
                        Map(input.mousePos.y - offset.y, 0, renderVars.viewportSize.height, 0, renderVars.canvasSize.height)};

                    selectedElementOffsets[element["id"].get<int>()] = Vector2{element["x"].get<float>(), element["y"].get<float>()} - canvasMousePos;

                }
            }
            else{
                if(input.leftMouse == MouseState::Pressed){
                    selectedElementOffsets.erase(element["id"].get<int>());
                }
            }

            if(selectedElementOffsets.contains(element["id"].get<int>()) && input.leftMouse == MouseState::Holding) // change the elements position if it is being held
            {
                Vector2 offset = (renderVars.windowSize - renderVars.viewportSize) / 2.0f + (renderVars.windowSize * renderVars.appliedViewportOffset);

                element["x"] = Map(input.mousePos.x - offset.x, 0, renderVars.viewportSize.width, 0, renderVars.canvasSize.width) + selectedElementOffsets[element["id"].get<int>()].x;
                element["y"] = Map(input.mousePos.y - offset.y, 0, renderVars.viewportSize.height, 0, renderVars.canvasSize.height) + selectedElementOffsets[element["id"].get<int>()].y;
            }
        }
        else if(element["type"] == "text"){
            Size2D size{0,0};

            auto fontIt = fonts.find(element["fontSrc"].get<string>());

            int fontSize = static_cast<int>(element["fontSize"].get<float>() * renderVars.canvasScaleFactor);

            TTF_Font* font;

            if (fontIt != fonts.end()) {
                auto sizeIt = fontIt->second.find(fontSize);

                if (sizeIt != fontIt->second.end()) { // if the font size exists
                    font = sizeIt->second;
                }
                else{
                    font = TTF_OpenFont(element["fontSrc"].get<string>().c_str(), fontSize);
                    fonts[element["fontSrc"].get<string>().c_str()][fontSize] = font;
                }
            } 
            else{
                font = TTF_OpenFont(element["fontSrc"].get<string>().c_str(), fontSize);
                fonts[element["fontSrc"].get<string>().c_str()][fontSize] = font;
            }

            if (!font) {
                cerr << "Yeah the font wouldn't load in the Editor" << SDL_GetError() << endl;
                continue;
            }

            TTF_GetStringSize(font, element["content"].get<string>().c_str(), 0, &size.width, &size.height);

            Vector2 topLeftPos{element["x"].get<float>(), element["y"].get<float>()};

            ConvertCanvasToScreen(size, topLeftPos, renderVars, false);

            if(topLeftPos.x < input.mousePos.x && topLeftPos.x + size.width > input.mousePos.x && 
                input.mousePos.y > topLeftPos.y && topLeftPos.y + size.height > input.mousePos.y){ // box collision

                if(input.leftMouse == MouseState::Pressed){
                    Vector2 offset = (renderVars.windowSize - renderVars.viewportSize) / 2.0f + (renderVars.windowSize * renderVars.appliedViewportOffset);

                    Vector2 canvasMousePos{ Map(input.mousePos.x - offset.x, 0, renderVars.viewportSize.width, 0, renderVars.canvasSize.width),  
                        Map(input.mousePos.y - offset.y, 0, renderVars.viewportSize.height, 0, renderVars.canvasSize.height)};
                    
                    selectedElementOffsets[element["id"].get<int>()] = Vector2{element["x"].get<float>(), element["y"].get<float>()} - canvasMousePos;
                }
            }
            else{
                if(input.leftMouse == MouseState::Pressed){
                    selectedElementOffsets.erase(element["id"].get<int>());
                }
            }

            if(selectedElementOffsets.contains(element["id"].get<int>()) && input.leftMouse == MouseState::Holding) // change the elements position if it is being held
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
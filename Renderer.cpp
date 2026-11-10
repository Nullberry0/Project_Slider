#include <SDL3/SDL.h> // Imports the library that is used to draw everything
#include <iostream> // Printing
#include <fstream> // Reading files
#include <nlohmann/json.hpp> // Run json files
#include <array> // For arrays
#include <SDL3_ttf/SDL_ttf.h> // Text render engine
#include <SDL3_image/SDL_image.h>

#include "Renderer.h"
#include "UI.h"

using std::cerr;
using std::cout;
using std::string;
using std::endl;
using std::pair;
using std::ifstream;
using std::array;
using std::vector;
using std::unordered_map;

using json = nlohmann::json;


void RenderFile(const json jsonFile, int openPageIndex, SDL_Renderer* renderer, RenderVars renderVars, unordered_map<string, unordered_map<int, TTF_Font*>> &fonts, unordered_map<string, SDL_Texture*> &images){

    auto elements = jsonFile["pages"][openPageIndex]["elements"];

    for (const auto& element : elements)
    {

        string type = element["type"];

        Vector2 pos{element["x"].get<float>(), element["y"].get<float>()};

        if(type == "rect")
        {
            Size2D size{element["width"].get<int>(), element["height"].get<int>()};

            ConvertCanvasToScreen(size, pos, renderVars, true);       

            auto c = element["color"];
            Color color = {c[0].get<uint8_t>(), c[1].get<uint8_t>(), c[2].get<uint8_t>(), 255};

            SDL_FRect rect = {pos.x, pos.y, static_cast<float>(size.width), static_cast<float>(size.height)};

            SDL_SetRenderDrawColor(renderer, color[0], color[1], color[2], 255);
            SDL_RenderFillRect(renderer, &rect);

        }
        else if(type == "text"){
            
            string content = element["content"].get<string>();
            auto c = element["color"];
            Color color = {c[0].get<uint8_t>(), c[1].get<uint8_t>(), c[2].get<uint8_t>(), 255};

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
                cerr << "Yeah the font wouldn't load in the renderer" << SDL_GetError() << endl;
                continue;
            }

            SDL_Color textColor = {color[0], color[1], color[2], 255};

            SDL_Surface* surface = TTF_RenderText_Blended( // This function renders the wanted text onto a surface
                font,
                content.c_str(), // This turns the string we want to render into a c string which the function can read
                0, // Automatically calculate the size in pixels of the text
                textColor
            );
 
            if (surface) {

                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                if (texture) {

                    Size2D size{surface->w, surface->h}; // Gets the width of the surface in pixels

                    // size does not need adjusting because the font is already changed on the font initialize line

                    ConvertCanvasToScreen(size, pos, renderVars, false);

                    SDL_FRect destination = {pos.x, pos.y, static_cast<float>(size.width), static_cast<float>(size.height)};

                    SDL_RenderTexture(renderer, texture, nullptr, &destination);
                    SDL_DestroyTexture(texture);

                }

                SDL_DestroySurface(surface);
            }
        }
        else if(type == "image"){
            SDL_Texture* imageTexture = GetImage(element["src"].get<string>(), images, renderer);

            Size2D size{element["width"].get<int>(), element["height"].get<int>()};
            ConvertCanvasToScreen(size, pos, renderVars, true);

            SDL_FRect destination = {pos.x, pos.y, static_cast<float>(size.width), static_cast<float>(size.height)};

            SDL_RenderTexture(renderer, imageTexture, nullptr, &destination);
        }
    }
}

void RenderViewRect(SDL_Renderer* renderer, RenderVars renderVars, Color bgColor) {

    Vector2 pos = (renderVars.windowSize - renderVars.viewportSize) /2 + (renderVars.windowSize * renderVars.appliedViewportOffset);

    SDL_FRect rect = {pos.x, pos.y, static_cast<float>(renderVars.viewportSize.width), static_cast<float>(renderVars.viewportSize.height) };

    SDL_SetRenderDrawColor(renderer, bgColor[0], bgColor[1], bgColor[2], 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &rect);
}

void RenderOutline(SDL_Renderer* renderer, int openPageIndex, json jsonFile, RenderVars renderVars, std::unordered_map<uint64_t, Vector2> selectedElementOffsets){

    SDL_SetRenderDrawColor(renderer, 23, 82, 112, 255);

    auto elements = jsonFile["pages"][openPageIndex]["elements"];

    for( const auto &element : elements){
        if(element["type"] == "rect"){
            int id = element["id"].get<int>();
            if(selectedElementOffsets.contains(id)){

                Vector2 pos{element["x"].get<float>(), element["y"].get<float>()};

                Size2D size{element["width"].get<int>(), element["height"].get<int>()};
                size += renderVars.windowSize.width / 50; //outline thickness

                ConvertCanvasToScreen(size, pos, renderVars, true);

                SDL_FRect rect = {pos.x, pos.y, static_cast<float>(size.width), static_cast<float>(size.height)};

                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
}

void RenderMenu(SDL_Renderer* renderer, RenderVars renderVars, MenuState menuState, unordered_map<string, SDL_Texture*> &images, Uint32 ticks){

    if(menuState == MenuState::BootCredits)
    {
        if(ticks > 1200 && ticks < 4500)    // Hide for the beginging
        {       
            Vector2 pos{
                renderVars.canvasSize.width / 2.0f,
                renderVars.canvasSize.height / 2.0f
            };

            SDL_Texture* imageTexture = GetImage("SlideshowFiles/LogoLoad.png", images, renderer);

            if(ticks > 2000){
                int d = 255 -((ticks - 2000) * .4); 
                Clamp(d, 0, 255);
                Uint8 darkness = static_cast<int>(d);
                SDL_SetTextureColorMod(imageTexture, darkness, darkness, darkness);
            }

            SDL_SetTextureScaleMode(imageTexture, SDL_SCALEMODE_PIXELART); // Remove Bilinair filtering

            int sameSize = static_cast<int>(renderVars.canvasSize.width / 1.2);

            Size2D size{sameSize, sameSize};
            ConvertCanvasToScreen(size, pos, renderVars, true);

            SDL_FRect destination = {pos.x, pos.y, static_cast<float>(size.width), static_cast<float>(size.height)};

            SDL_RenderTexture(renderer, imageTexture, nullptr, &destination);
        }
    }

}

SDL_Texture* GetImage(string src, unordered_map<string, SDL_Texture*> &images, SDL_Renderer* renderer){
    SDL_Texture* imageTexture;

    auto findVar = images.find(src.c_str());

    if(findVar != images.end()){
        imageTexture = images[findVar->first];
    }
    else{
        imageTexture = IMG_LoadTexture(renderer, src.c_str());
        images[src.c_str()] = imageTexture;
    }
    return imageTexture;
    
}

void Clamp(int& value, int min, int max){
    if(value > max) value = max;
    if(min > value) value = min;
}

float Min(float value1, float value2){
    if(value1 > value2) return value2;
    else return value1;
}

int GetPageCount(const json jsonFile){
    return jsonFile["pages"].size();
}

void ConvertCanvasToScreen(Size2D &size, Vector2 &pos, const RenderVars renderVars, const bool changeSize){
    if(changeSize) size = size / renderVars.canvasSize * renderVars.viewportSize;

    pos = ((renderVars.windowSize - renderVars.viewportSize)/2) + pos / renderVars.canvasSize * renderVars.viewportSize - (size /2); //The part in the beginning is so that the middle is relative to the view rect
    pos += renderVars.windowSize * renderVars.appliedViewportOffset; // Offset
}
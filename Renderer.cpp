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
using std::visit;
using std::holds_alternative;
using std::is_same_v;
using std::decay_t;

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

            int fontSize = static_cast<int>(element["fontSize"].get<float>() * renderVars.canvasScaleFactor);

            Size2D size = {0, 0};

            string fontSrc = element["fontSrc"].get<string>();

            ConvertCanvasToScreen(size, pos, renderVars, false);

            RenderText(renderer, content, fontSrc, fontSize, fonts, color, pos);

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

    Vector2 pos = (renderVars.windowSize - renderVars.viewportSize) /2 + renderVars.appliedViewportOffset;

    SDL_FRect rect = {pos.x, pos.y, static_cast<float>(renderVars.viewportSize.width), static_cast<float>(renderVars.viewportSize.height) };

    SDL_SetRenderDrawColor(renderer, bgColor[0], bgColor[1], bgColor[2], 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &rect);
}

void RenderOutline(SDL_Renderer* renderer, int openPageIndex, json jsonFile, RenderVars renderVars, unordered_map<uint64_t, Vector2> selectedElementOffsets){

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

void RenderUIButtons(SDL_Renderer* renderer, RenderVars renderVars, vector<UIElement> UIElements, unordered_map<string, unordered_map<int, TTF_Font*>> &fonts){
    for(const auto &element : UIElements){
        visit([&](auto &ui){
            if(ui.properties.isVisible)
            {
                Vector2 pos;
                Size2D size;
                ConvertUIToScreen(ui.properties.anchor, ui.properties.pixelOffset, ui.properties.relativeSize, ui.properties.pixelSize, renderVars, pos, size);
                
                SDL_FRect rect = {pos.x, pos.y, static_cast<float>(size.width), static_cast<float>(size.height)};

                if(ui.isBeingPressed) SDL_SetRenderDrawColor(renderer, 190, 119, 43, 255);
                else SDL_SetRenderDrawColor(renderer, ui.properties.color.r, ui.properties.color.g, ui.properties.color.b, 255);
                RenderRoundedRect(renderer, rect, renderVars, 10);

                if(ui.text != ""){
                    int fontSize = ui.fontSize;
                    if(ui.properties.pixelSize == Size2D{-1, -1}){
                        fontSize *= renderVars.canvasScaleFactor;
                    }
                    RenderText(renderer, ui.text, "SlideshowFiles/Fonts/font1.ttf", fontSize, fonts, Color{0, 0, 0}, pos + size/2);
                }
                if constexpr (is_same_v<decay_t<decltype(ui)>, Dropdown>) {
                    if(ui.isOpen){
                        Vector2 drawPos = pos;
                        drawPos.x += size.width/2;
                        drawPos.y += size.height/2; // finds the bottom
                        for(size_t i = 0; i < ui.options.size(); i++){ // loops through each option to draw it
                            DropdownOption option = ui.options.at(i);
                            
                            drawPos.y += ui.distBetweenOptions; // now at the top of the option we want to draw             
                            drawPos.y += option.size.height /2;

                            drawPos.x -= option.size.width/2;
                            

                            SDL_FRect rect = {drawPos.x, drawPos.y, static_cast<float>(option.size.width), static_cast<float>(option.size.height)};
                            SDL_SetRenderDrawColor(renderer, option.color.r, option.color.g, option.color.b, 255);
                            SDL_RenderFillRect(renderer, &rect);

                            drawPos.x += option.size.width/2;
                            drawPos.y += option.size.height /2; // if you do the math you will see that this is needed

                            RenderText(renderer, option.text, "SlideshowFiles/Fonts/font1.ttf", option.fontSize, fonts, Color{0, 0, 0}, drawPos);

                        }
                    }
                }
            }
        }, element);
    }
}

void RenderRoundedRect(SDL_Renderer* renderer, SDL_FRect rect, RenderVars renderVars, float roundness){
    SDL_RenderFillRect(renderer, &rect);
    RenderCircle(renderer, Vector2{500, 500}, 80);
}

void RenderCircle(SDL_Renderer* renderer, Vector2 pos, int radius){

    // not gonna pretend to know how this works...

    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);

    SDL_FColor color = {
        r / 255.0f,
        g / 255.0f,
        b / 255.0f,
        a / 255.0f
    };

    const int segments = 32;
    const float PI = 3.1415f;

    vector<SDL_Vertex> vertices;

    for (int i = 0; i < segments; i++) {
        float angle1 = 2.0f * PI * i / segments;
        float angle2 = 2.0f * PI * (i + 1) / segments;

        vertices.push_back(
            {{pos.x, pos.y}, color, {0, 0}}
        );

        vertices.push_back(
            {{pos.x + cosf(angle1) * radius, pos.y + sinf(angle1) * radius}, color, {0, 0}}
        );

        vertices.push_back(
            {{pos.x + cosf(angle2) * radius, pos.y + sinf(angle2) * radius}, color, {0, 0}}
        );

    }

    SDL_RenderGeometry(renderer, nullptr, vertices.data(), vertices.size(), nullptr, 0);

}

void RenderText(SDL_Renderer* renderer, string content, string src, int fontSize, unordered_map<string, unordered_map<int, TTF_Font*>> &fonts, Color color, Vector2 pos){
    TTF_Font* font = GetFont(src, fontSize, fonts);

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

            pos -= size/2;

            // size does not need adjusting because the font is already changed on the font initialize line

            SDL_FRect destination = {pos.x, pos.y, static_cast<float>(size.width), static_cast<float>(size.height)};

            SDL_RenderTexture(renderer, texture, nullptr, &destination);
            SDL_DestroyTexture(texture);

        }

        SDL_DestroySurface(surface);
    }
    else{
        cout << "No Surface Made For Rendering Text!" << SDL_GetError() << endl;
    }
} 

MenuState RenderMenu(SDL_Renderer* renderer, RenderVars renderVars, MenuState menuState, unordered_map<string, SDL_Texture*> &images, Uint32 ticks){

    if(menuState == MenuState::BootCredits)
    {
        if(ticks > 1200 && ticks < 3500)    // Hide for the beginging
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

            return MenuState::BootCredits;
        }
        if(ticks > 3500){
            return MenuState::StartMenu;
        }
    }
    return menuState;
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
    // THIS RETURNS THE LEFT TOP CORNER

    if(changeSize) size = size / renderVars.canvasSize * renderVars.viewportSize;

    pos = ((renderVars.windowSize - renderVars.viewportSize)/2) + pos / renderVars.canvasSize * renderVars.viewportSize - (size /2); //The part in the beginning is so that the middle is relative to the view rect
    pos += renderVars.appliedViewportOffset; // Offset
}

void ConvertUIToScreen(UIAnchor anchor, Vector2 pixelOffset, Vector2 relativeSize, Size2D pixelSize, RenderVars renderVars, Vector2 &pos, Size2D &size){
    // THIS RETURNS THE LEFT TOP CORNER

    if(anchor == UIAnchor::Center){
        pos = renderVars.windowSize /2;
    }
    if(anchor == UIAnchor::TopCenter){
        pos = {static_cast<float>(renderVars.windowSize.width /2), 0};
    }
    if(anchor == UIAnchor::TopLeft){
        pos = {0, 0};
    }
    else{
        pos = renderVars.windowSize /2; // TEMPORARY
    }
    pos += pixelOffset;
    
    if(pixelSize == Size2D{-1, -1}){
        size = relativeSize * Min(renderVars.windowSize.width, renderVars.windowSize.height);
    }
    else{
        size = pixelSize;
    }

    pos -= size/2;
}

TTF_Font* GetFont(string src, int fontSize, unordered_map<string, unordered_map<int, TTF_Font*>> &fonts){
    TTF_Font* font;

    auto fontIt = fonts.find(src);

    if (fontIt != fonts.end()) {
        auto sizeIt = fontIt->second.find(fontSize);

        if (sizeIt != fontIt->second.end()) { // if the font size exists
            font = sizeIt->second;
        }
        else{
            font = TTF_OpenFont(src.c_str(), fontSize);
            fonts[src.c_str()][fontSize] = font;
        }
    } 
    else{
        font = TTF_OpenFont(src.c_str(), fontSize);
        fonts[src.c_str()][fontSize] = font;
    }

    if (!font) {
        cerr << "Yeah the font couldnt be find or made" << SDL_GetError() << endl;
        return nullptr;
    }
    return font;
}
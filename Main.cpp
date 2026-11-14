#include <SDL3/SDL.h> // Imports the library that is used to draw everything
#include <SDL3/SDL_main.h> // For flexibilty on startup
#include <iostream> // Printing
#include <fstream> // Reading files
#include <nlohmann/json.hpp> // Run json files
#include <array> // For arrays
#include <SDL3_ttf/SDL_ttf.h> // Text render engine
#include <cmath> // For absolute value
#include <random> // I mean I think even I can understand what this one does; it is used for screenshake
#include <stdexcept> // For different error messages like out of range

#include "Types.h"
#include "Editor.h" // The editor header file
#include "UI.h"
#include "Menu.h"
#include "Renderer.h"

using std::cout;
using std::cerr;
using std::string;
using std::endl;
using std::ifstream;
using std::array;
using std::vector;
using std::sin;
using std::cos;
using std::unordered_map;

using json = nlohmann::json;

// FUNCTION PROTOTYPES
//      Otherwise the functions are not declared yet, and yes this can also be fixed by placing the functions in front

bool StartSDL();
bool StartSDLTTF();
bool InitializeRoot(SDL_Window*& root, Size2D& windowSize);
bool InitializeRenderer(SDL_Renderer*& renderer, SDL_Window* window);

void StartTransitions(const json jsonFileCurrentPage, Color &currentBgColor, Color &targetBgColor, float &bgColorInterpolateSpeed, float &screenShakeTimer, Vector2 &additionalViewportOffset, float &screenShakeDuration, 
   Vector2 &originalScreenShakeIntensity, float &screenShakeFrequency);

void UpdateRenderVars(RenderVars &renderVars, const Vector2 editingViewportOffset, const MenuState menuState, Vector2 &additionalViewportOffset, SDL_Window* root);

Size2D GetCanvasSize(const json memoryJson);

std::mt19937 generator(std::random_device{}()); // Initialize random seed gen



    // FILE DECLARATION

    string openFile = "Slideshow1.slider";

    ifstream file(openFile);

    json memoryJson = json::parse(file); // A copy of the json file which can be modified without changing the underlying file

    // VARIABLE DECLARATION

    Size2D windowSize{1280, 720}; 

    Size2D canvasSize = GetCanvasSize(memoryJson);

    float canvasScaleFactor = Min(windowSize.width / canvasSize.width, windowSize.height / canvasSize.height);
  
    Size2D viewportSize{static_cast<int>(canvasSize.width * canvasScaleFactor), static_cast<int>(canvasSize.height * canvasScaleFactor)};

    Vector2 appliedViewportOffset{0, 0}; // A 1 meaning 100% shifted out the screen

    RenderVars renderVars{
        windowSize,
        canvasSize,
        viewportSize,
        appliedViewportOffset,
        canvasScaleFactor
    };

    unordered_map<uint64_t, Vector2> selectedElementOffsets{}; // the offset between the middle of the object and the cursor when it is first selected
    unordered_map<string, unordered_map<int, TTF_Font*>> fonts; // font are cashed so that you don't have to remake the font every time you want to use it. This is better for preformance
    unordered_map<string, SDL_Texture*> images; 

    vector<Button> UIButtons{{
        ButtonFunc::StartEditing,
        ButtonGroup::StartPage,
        UIAnchor::Center,
        Vector2{0, 0}, // offset
        Size2D{-1, -1}, // pixel size
        Vector2{0.3, 0.3}, // relative size
        Color{70, 130, 220, 255},
        true, // visible
        true, // aspect locked
        false // is pressed
    }};

    int pageIndex = 0;

    int lastPageIndex = pageIndex;

    MenuState menuState = MenuState::BootCredits;

    //        the applied offset is in the renderVars scruct

    const Vector2 editingViewportOffset{0.1, 0.1};
    Vector2 additionalViewportOffset{0, 0}; // This is for screen shake effects

    Vector2 originalScreenShakeIntensity{0, 0};

    float screenShakeFrequency = 0; // determines how many times the shake ossolates in a given period of time

    float screenShakeTimer = 0;
    float screenShakeDuration = 0;

    int pageCount = GetPageCount(memoryJson);

    float deltaTime = 0;

    Color currentBgColor{0, 0, 0, 255};
    Color targetBgColor{};
    float bgColorInterpolateSpeed; // This will be set to a different value pased on the transition parameters

    InputState editorInput{};


int main() // Runs once on startup
{
    float lastTime = SDL_GetTicks();

    // SDL STARTUP

    SDL_Window* root;
    SDL_Renderer* renderer;

    if( !StartSDL() ) return 1;
    if( !StartSDLTTF() ) return 1;
    
    if( !InitializeRoot(root, renderVars.windowSize) ) return 1;
    if( !InitializeRenderer(renderer, root) ) return 1;

    // MAIN LOOP

    bool running = true;
    SDL_Event event;

    StartTransitions(memoryJson["pages"][pageIndex], currentBgColor, targetBgColor, bgColorInterpolateSpeed, screenShakeTimer, additionalViewportOffset, 
            screenShakeDuration, originalScreenShakeIntensity, screenShakeFrequency);  

    while (running)
    {
        lastPageIndex = pageIndex;

        deltaTime = (SDL_GetTicks() - lastTime) / 1000.0f;
        lastTime = SDL_GetTicks();

        editorInput.leftMouse = MouseState::None;

        while (SDL_PollEvent(&event))
        {
            InputState newInput = RecordEditorInput(event);

            if (newInput.leftMouse == MouseState::Pressed || newInput.leftMouse == MouseState::Released) {
                editorInput.leftMouse = newInput.leftMouse;
            }

            editorInput.mousePos = newInput.mousePos;

            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {

                if(event.key.key == SDLK_RIGHT){
                    pageIndex++;
                    Clamp(pageIndex, 0, pageCount -1);
                }
                if(event.key.key == SDLK_LEFT){
                    pageIndex--;
                    Clamp(pageIndex, 0, pageCount -1);
                }
                if(event.key.key == SDLK_ESCAPE){
                    if(menuState == MenuState::Viewing || menuState == MenuState::Editing){
                        menuState = menuState == MenuState::Editing ? MenuState::Viewing : MenuState::Editing;
                    }
                    else if(menuState == MenuState::StartMenu){
                        menuState = MenuState::Editing; // TEMPORARY
                    }
                }
            }
        }

        if (editorInput.leftMouse == MouseState::None) {
            Uint32 buttons = SDL_GetMouseState(nullptr, nullptr);
            if (buttons & SDL_BUTTON_LMASK) {  // If the left mouse button isnt masked out, it is being held
                editorInput.leftMouse = MouseState::Holding;
            }
        }

        // for(const auto &[id, offset] : selectedElementOffsets){
        //     cout << id << endl;
        // }

        if(bgColorInterpolateSpeed > -0.5f){ // the interpolate speed is -1 when target == current
            int sameColorsCounter = 0;
            for(int i = 0; i < 3; i++){ // Cycle through the R G and B values of current color
                currentBgColor[i] += (bgColorInterpolateSpeed/100) * (targetBgColor[i] - currentBgColor[i]);
                if(abs(currentBgColor[i] - targetBgColor[i]) < 1){
                    currentBgColor[i] = targetBgColor[i];
                    sameColorsCounter++;
                }
            }
            if(sameColorsCounter == 3){
                bgColorInterpolateSpeed = -1;   
                currentBgColor = targetBgColor;    
            } 
        }

        if(lastPageIndex != pageIndex) StartTransitions(memoryJson["pages"][pageIndex], currentBgColor, targetBgColor, bgColorInterpolateSpeed, screenShakeTimer, additionalViewportOffset, 
            screenShakeDuration, originalScreenShakeIntensity, screenShakeFrequency);

        if(screenShakeTimer > 0){
            screenShakeTimer -= deltaTime;
            float t = screenShakeTimer / screenShakeDuration;
            float elapsed = screenShakeDuration - screenShakeTimer;
            t = t * t * t;
            additionalViewportOffset = {originalScreenShakeIntensity.x * t * sin(elapsed * screenShakeFrequency), 
                originalScreenShakeIntensity.y * t * cos(elapsed * screenShakeFrequency)};
        }
        else{
            additionalViewportOffset = {0, 0};
        }
        
        UpdateRenderVars(renderVars, editingViewportOffset, menuState, additionalViewportOffset, root);

        if(menuState == MenuState::Editing) UpdateEditor(memoryJson, pageIndex, editorInput, renderVars, selectedElementOffsets, fonts);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        HandleButtons(renderVars, UIButtons, editorInput, menuState);

        if(menuState == MenuState::Viewing || menuState == MenuState::Editing) RenderViewRect(renderer, renderVars, currentBgColor);

        if(menuState == MenuState::Editing) RenderOutline(renderer, pageIndex, memoryJson, renderVars, selectedElementOffsets);

        if(menuState == MenuState::Viewing || menuState == MenuState::Editing) RenderFile(memoryJson, pageIndex, renderer, renderVars, fonts, images);
        HandleMenu(renderer, renderVars, menuState, images, SDL_GetTicks());
        RenderUIButtons(renderer, renderVars, UIButtons);

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    for (auto& [fontSrc, sizes] : fonts) {
        for (auto& [fontSize, font] : sizes) {
            TTF_CloseFont(font);
        }
    }
    for (auto& [imageSrc, image]: images) {
        SDL_DestroyTexture(image);
    }

    fonts.clear();
    images.clear();

    SDL_DestroyWindow(root);
    TTF_Quit();
    SDL_Quit();

    return 0;
}

void StartTransitions(const json jsonFileCurrentPage, Color &currentBgColor, Color &targetBgColor, float &bgColorInterpolateSpeed, float &screenShakeTimer, Vector2 &additionalViewportOffset, float &screenShakeDuration, 
   Vector2 &originalScreenShakeIntensity, float &screenShakeFrequency){

    auto transitions = jsonFileCurrentPage["slideTransitions"];

    if(transitions.size() == 0){
        auto color = jsonFileCurrentPage["bgColor"];
        currentBgColor = {color[0].get<uint8_t>(), color[1].get<uint8_t>(), color[2].get<uint8_t>(), 255};
        targetBgColor = currentBgColor;
        bgColorInterpolateSpeed = -1.0f;
    }

    bool interpolateColorFlag = false;

    for (const auto& transition : transitions)
    {
        string type = transition["type"];

        if (type == "InterpolateBgColor")
        {
            interpolateColorFlag = true;
            auto color = jsonFileCurrentPage["bgColor"];
            targetBgColor = {color[0].get<uint8_t>(), color[1].get<uint8_t>(), color[2].get<uint8_t>(), 255};
            bgColorInterpolateSpeed = transition["speed"];
        }

        if (type == "Shake"){
            screenShakeFrequency = transition["frequency"].get<float>() * 6.28318; // 2pi, to make it per second

            screenShakeTimer = transition["duration"].get<float>();
            screenShakeDuration = screenShakeTimer;

            additionalViewportOffset.x = std::uniform_real_distribution<float>(-transition["intensity"].get<float>() /100, transition["intensity"].get<float>() / 100)(generator);
            additionalViewportOffset.y = (transition["intensity"].get<float>()/100) - abs(additionalViewportOffset.x);
 
            if(std::bernoulli_distribution(0.5)(generator) == true){
                 additionalViewportOffset.y *= -1;
            }

            originalScreenShakeIntensity = additionalViewportOffset;
        }

    }
    if(!interpolateColorFlag){
        auto color = jsonFileCurrentPage["bgColor"];
        currentBgColor = {color[0].get<uint8_t>(), color[1].get<uint8_t>(), color[2].get<uint8_t>(), 255};
        targetBgColor = currentBgColor;
        bgColorInterpolateSpeed = -1.0f;
    }
}

void UpdateRenderVars(RenderVars &renderVars, const Vector2 editingViewportOffset, const MenuState menuState, Vector2 &additionalViewportOffset, SDL_Window* root){

    SDL_GetWindowSize(root, &renderVars.windowSize.width, &renderVars.windowSize.height);
    
    renderVars.canvasScaleFactor = Min(static_cast<float>(renderVars.windowSize.width) / renderVars.canvasSize.width, static_cast<float>(renderVars.windowSize.height) / renderVars.canvasSize.height);
    //cout << renderVars.canvasScaleFactor << endl;

    if(menuState == MenuState::Editing){
        renderVars.viewportSize *= 0.8;
        renderVars.canvasScaleFactor *= 0.8;

        renderVars.appliedViewportOffset = editingViewportOffset;
    }
    else{
        renderVars.appliedViewportOffset = {0, 0};
    }

    renderVars.viewportSize = renderVars.canvasSize * renderVars.canvasScaleFactor;

    renderVars.appliedViewportOffset += additionalViewportOffset;
}

bool StartSDL(){

    if (!SDL_Init(SDL_INIT_VIDEO)) // Video is responsible for all the rendering
    {
        cerr << "SDL is having a bad day!" << SDL_GetError() << endl; 
        return false;
    }
    else return true;

}

bool StartSDLTTF(){

    if (!TTF_Init()) {
        cerr << "The text engine lost his words!" << SDL_GetError() << endl; 
        return false;
    }
    else return true;

}

bool InitializeRenderer(SDL_Renderer*& renderer, SDL_Window* window){

    renderer = SDL_CreateRenderer(window, nullptr);

    if(!renderer)
    {
        cerr << "The renderer lost its colors and can't be started!" << SDL_GetError() << endl;
        SDL_Quit(); // Call This because SDL is already innitialized
        return false;
    }
    else return true;
}

bool InitializeRoot(SDL_Window*& root, Size2D& windowSize){ // The *& by sdl window is because the * means a pointer and & means I want to edit the original

    root = SDL_CreateWindow(
        "SlideshowMaker",
        windowSize.width,
        windowSize.height,
        SDL_WINDOW_RESIZABLE
    );

    if (!root)
    {
        cerr << "The window refuses to be created!" << SDL_GetError() << endl;
        SDL_Quit(); // Call This because SDL is already innitialized
        return false;
    }
    else return true;

}

Size2D GetCanvasSize(const json memoryJson){

    auto canvas = memoryJson["canvas"];

    return Size2D{canvas["width"], canvas["height"]};
}
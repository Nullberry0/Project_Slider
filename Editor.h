#pragma once // just good to have so it doesnt get rereferrensed multiple times or something

#include <nlohmann/json.hpp> // Run json files
#include <SDL3/SDL.h>

#include "Renderer.h"

enum class Keystroke {
    Delete, 
    None
};

enum class MouseState {
    Pressed, 
    Released, 
    Holding,
    None
};

struct InputState {
    Vector2 mousePos{};
    MouseState leftMouse = MouseState::None;
    Keystroke keyStroke;
};

InputState RecordEditorInput(SDL_Event event);

void UpdateEditor(nlohmann::json& jsonFile, int currentPage, InputState input, RenderVars renderVars, std::vector<int>& selectedElements);

float Map(float value, float inputStart, float inputEnd, float outputStart, float outputEnd);
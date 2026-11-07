#pragma once

#include <SDL3/SDL.h>
#include <iostream> 

#include "Types.h"

using std::string;

enum class MenuState{
    Menu,
    Editing,
    Viewing
};

enum class ButtonFunc{
    StartEditing
};

struct Button {
    ButtonFunc func;

    Vector2 pos;
    Size2D size;

    Color color;

    bool visible = true;

    bool isBeingPressed = false; // This is for displaying a different version of the button while it is being pressed
};

// void HandleButtons(RenderVars renderVars, );
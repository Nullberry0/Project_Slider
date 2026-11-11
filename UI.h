#pragma once

#include <SDL3/SDL.h>
#include <iostream> 

#include "Types.h"

using std::string;

enum class MenuState{
    StartMenu,
    Editing,
    Viewing,
    BootCredits
};

enum class ButtonFunc{
    StartEditing
};
enum class ButtonGroup{
    StartPage,
    MainEditing
};

enum class UIAnchor{
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

struct Button {
    ButtonFunc func;

    ButtonGroup group;

    UIAnchor anchor;
    Vector2 pixelOffset;
    Size2D pixelSize; // if is -1 then use relative size, otherwise use pixel size
    Vector2 relativeSize; // a 1 is full window a 0 being no pixels

    Color color;

    bool visible = true;

    bool isBeingPressed = false; // This is for displaying a different version of the button while it is being pressed
};

// void HandleButtons(RenderVars renderVars, );
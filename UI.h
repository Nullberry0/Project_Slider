#pragma once

#include <SDL3/SDL.h>
#include <iostream> 
#include <variant>
#include <vector>

#include "Types.h"

using std::string;
using std::vector;
using std::variant;

enum class MenuState{
    StartMenu,
    Editing,
    Viewing,
    BootCredits
};

enum class ButtonFunc{
    StartEditing,
    InsertRect,
    InsertHelloWorldText
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

struct UIProperties {
    UIAnchor anchor;
    Vector2 pixelOffset;
    Size2D pixelSize; // if is -1 then use relative size, otherwise use pixel size
    Vector2 relativeSize; // a 1 is full window a 0 being no pixels

    Color color;

    bool isVisible = true;
};

struct Button {
    ButtonFunc func;

    ButtonGroup group;

    UIProperties properties;

    string text = "";
    int fontSize = 22;

    bool isBeingPressed = false; // This is for displaying a different version of the button while it is being pressed
};

struct DropdownOption {
    ButtonFunc func;
    string text;
    Color textColor;
    Size2D size;
    int fontSize = 22;
    Color color;
    bool isBeingPressed = false;
    bool isVisible = true; // if this is true but the dropdown is closed it will not show
};

struct Dropdown {
    vector<DropdownOption> options;

    ButtonGroup group;

    UIProperties properties;

    int selectedIndex = 0; // 0 is the top one, 1 is the second top one etc.

    bool isOpen = false;
    int distBetweenOptions = 30; // in pixels, distance between closest points

    string text = "";
    int fontSize = 22;

    bool isBeingPressed = false; // This is for displaying a different version of the button while it is being pressed

    bool isVisible = true;
};

using UIElement = variant<Button, Dropdown>; // so that different types of UI can still be in the same list

void HandleUIElements(RenderVars renderVars, vector<UIElement> &UIElements, InputState inputState, MenuState &menuState);

bool ButtonVisibility(UIElement element, MenuState menuState);
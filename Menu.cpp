#include <SDL3/SDL.h>
#include <iostream> 

#include "Types.h"
#include "Editor.h" 
#include "Renderer.h" 
#include "UI.h"

#include "Menu.h"

using std::unordered_map;
using std::cout;
using std::endl;

void HandleMenu(SDL_Renderer* renderer, RenderVars renderVars, MenuState &menuState, unordered_map<string, SDL_Texture*> &images, Uint32 ticks){
    menuState = RenderMenu(renderer, renderVars, menuState, images, ticks); // this is so that the numbers stay consistent
}
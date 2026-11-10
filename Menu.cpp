#include <SDL3/SDL.h>
#include <iostream> 

#include "Types.h"
#include "Editor.h" 
#include "Renderer.h" 
#include "UI.h"

#include "Menu.h"

using std::unordered_map;

void HandleMenu(SDL_Renderer* renderer, RenderVars renderVars, MenuState &menuState, unordered_map<string, SDL_Texture*> &images, Uint32 ticks){
    RenderMenu(renderer, renderVars, menuState, images, ticks);
}
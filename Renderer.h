#pragma once // just good to have so it doesnt get coplied multiple times 

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h> // Text render engine
#include <nlohmann/json.hpp>
#include <utility>

#include "UI.h"
#include "Menu.h"
#include "Types.h"

using std::unordered_map;
using std::string;
using std::vector;

using json = nlohmann::json;

void RenderFile(const json jsonFile, int openPageIndex, SDL_Renderer* renderer, RenderVars renderVars, unordered_map<string, unordered_map<int, TTF_Font*>> &fonts, unordered_map<string, SDL_Texture*> &images);

void RenderViewRect(SDL_Renderer* renderer, RenderVars renderVars, Color bgColor);

void Clamp(int& value, int min, int max);
float Min(float value1, float value2);

int GetPageCount(const nlohmann::json jsonFile);

void RenderOutline(SDL_Renderer* renderer, int openPageIndex, json jsonFile, RenderVars renderVars, std::unordered_map<uint64_t, Vector2> selectedElementOffsets);
void ConvertCanvasToScreen(Size2D &size, Vector2 &pos, const RenderVars renderVars, const bool changeSize);

SDL_Texture* GetImage(string src, unordered_map<string, SDL_Texture*> &images, SDL_Renderer* renderer);
bool IsPointInBox(Vector2 pos1, Vector2 pos2, Vector2 point);

MenuState RenderMenu(SDL_Renderer* renderer, RenderVars renderVars, MenuState menuState, unordered_map<string, SDL_Texture*> &images, Uint32 ticks);

void RenderUIButtons(SDL_Renderer* renderer, RenderVars renderVars, vector<Button> buttons);
void ConvertUIToScreen(UIAnchor anchor, Vector2 pixelOffset, Vector2 relativeSize, Size2D pixelSize, bool preserveAspect, RenderVars renderVars, Vector2 &pos, Size2D &size);
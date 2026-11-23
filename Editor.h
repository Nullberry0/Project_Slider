#pragma once // just good to have so it doesnt get rereferrensed multiple times or something

#include <nlohmann/json.hpp> // Run json files
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

using std::unordered_map;
using std::string;

InputState RecordEditorInput(SDL_Event event);

void UpdateEditor(vector<Slide> &slideshow, int currentPage, InputState input, RenderVars renderVars, unordered_map<string, unordered_map<int, TTF_Font*>> &fonts);

float Map(float value, float inputStart, float inputEnd, float outputStart, float outputEnd);
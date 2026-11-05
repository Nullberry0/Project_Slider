#pragma once // just good to have so it doesnt get coplied multiple times 
#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <utility>

using json = nlohmann::json;
using std::vector;

struct Size2D; 

struct Vector2 {
    float x;
    float y;

    Vector2 operator*(float multiplier) const {
        return {x * multiplier, y * multiplier};
    }

    Vector2& operator+=(float value) {
        x += value;
        y += value;
        return *this;
    }

    Vector2& operator+=(const Vector2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    // overloads that need size2D the prototypes are here
    Vector2 operator/(const Size2D& other) const;
    Vector2 operator-(const Size2D& other) const;
    Vector2 operator*(const Size2D& other) const;
    Vector2& operator-=(const Size2D& other);

    Vector2 operator/(float divider) const {
        return {x / divider, y / divider};
    }

    Vector2 operator+(const Vector2& other) const {
        return {x + other.x, y + other.y};
    }
    Vector2& operator-=(const Vector2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    Vector2 operator-(const Vector2& other) const {
        return {x - other.x, y - other.y};
    }
};


struct Size2D {
    int width;
    int height;

    Size2D operator*(float multiplier) const {
        return {
            static_cast<int>(width * multiplier),
            static_cast<int>(height * multiplier)
        };
    }

    Size2D operator*(const Size2D& other) const {
        return {
            width * other.width,
            height * other.height
        };
    }

    Vector2 operator*(const Vector2& other) const {
        return {
            width * other.x,
            height * other.y
        };
    }

    Size2D& operator*=(float multiplier) {
        width = static_cast<int>(width * multiplier);
        height = static_cast<int>(height * multiplier);
        return *this;
    }

    Size2D operator-(const Size2D& other) const {
        return {
            width - other.width,
            height - other.height
        };
    }

    Size2D operator/(int divider) const {
        return {
            width / divider,
            height / divider
        };
    }

    Vector2 operator+(const Vector2& other) const {
        return {
            width + other.x,
            height + other.y
        };
    }

    Vector2 operator-(const Vector2& other) const {
        return {
            width - other.x,
            height - other.y
        };
    }

    Vector2 operator/(const Size2D& other) const {
        return {
            static_cast<float>(width) / other.width,
            static_cast<float>(height) / other.height
        };
    }

    Size2D& operator+=(int adder) {
        width += adder;
        height += adder;
        return *this;
    }


    Size2D& operator=(const Vector2& other) {
        width = static_cast<int>(other.x);
        height = static_cast<int>(other.y);
        return *this;
    }
    Size2D& operator-=(const Size2D& other) {
        width -= other.width;
        height -= other.height;
        return *this;
    }
};


// Now Size2D is defined, so these operators can be implemented.

inline Vector2 Vector2::operator/(const Size2D& other) const {
    return {
        x / other.width,
        y / other.height
    };
}

inline Vector2 Vector2::operator-(const Size2D& other) const {
    return {
        x - other.width,
        y - other.height
    };
}

inline Vector2 Vector2::operator*(const Size2D& other) const {
    return {
        x * other.width,
        y * other.height
    };
}

inline Vector2& Vector2::operator-=(const Size2D& other) {
    x -= other.width;
    y -= other.height;
    return *this;
}


struct Color {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;

    Uint8& operator[](int index) {
        switch (index) {
            case 0: return r;
            case 1: return g;
            case 2: return b;
            case 3: return a;
            default: throw std::out_of_range("Invalid color index");
        }
    }
};




struct RenderVars
{
    Size2D windowSize{};
    Size2D canvasSize{};
    Size2D viewportSize{};
    Vector2 appliedViewportOffset{};
    float canvasScaleFactor;
};



void RenderFile(const json jsonFile, int openPageIndex, SDL_Renderer* renderer, RenderVars renderVars);

void RenderViewRect(SDL_Renderer* renderer, RenderVars renderVars, Color bgColor);

void Clamp(int& value, int min, int max);
float Min(float value1, float value2);

int GetPageCount(const nlohmann::json jsonFile);

void RenderOutline(SDL_Renderer* renderer, int openPageIndex, json jsonFile, RenderVars renderVars, std::unordered_map<uint64_t, Vector2> selectedElementOffsets);
void ConvertCanvasToScreen(Size2D &size, Vector2 &pos, const RenderVars renderVars, const bool changeSize);
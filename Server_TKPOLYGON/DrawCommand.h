#pragma once
// DrawCommand.h
#pragma once
#pragma pack(push, 1)
#include "pch.h"
struct DrawCommand
{
    float vertices[3][3];  // [頂点0〜2][x,y,z]
    float color[3];        // RGB
};
#pragma pack(pop)

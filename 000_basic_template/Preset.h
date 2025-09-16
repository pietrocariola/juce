#pragma once

#include <cstring>
#include <initializer_list>
#include <vector>

//==============================================================================
// Preset.h: estrutura padrao dos presets
//==============================================================================

struct Preset
{
    Preset(const char* pname,  std::initializer_list<float> param_list)
    {
        strcpy(this->name, pname);
        param = param_list;
    }

    char name[40];
    std::vector<float> param;
};

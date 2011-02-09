//-----------------------------------------------------------
//    
//-----------------------------------------------------------
#ifndef SYNTH_PARSE_H
#define SYNTH_PARSE_H

#include <string>
#include <vector>
#include "tree.h"
#include "synth_mod_base.h"

namespace TSynth{

    bool Parse(std::string const& _strexpr, creek::tree<SynthModBasePtr>& _tree);

    std::string GetToken(std::string::const_iterator& _it, std::string::const_iterator _end);

    SynthModBasePtr MakeSynthModFromString(std::string const& _str);
}
#endif


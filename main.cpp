#include <iostream>
#include "synth.h"
#include "cui.h"

int main()
{
    TSynth::Synth synth;
    TSynth::Cui cui(synth, std::cin, std::cout);
    
    try{
        cui.Run();
    }
    catch(std::exception const& _er){
        std::cout << "catch: " << _er.what() << std::endl;
    }
    catch(...){
        std::cout << "unknown error" << std::endl;
        throw;
    }
}




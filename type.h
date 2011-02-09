//-----------------------------------------------------------
//    
//-----------------------------------------------------------
#ifndef SYNTH_TYPE_H
#define SYNTH_TYPE_H

#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace TSynth{
    
    typedef double Real;
    
    typedef std::uint8_t UInt8;
    typedef std::uint16_t UInt16;
    typedef std::uint32_t UInt32;
    
    //-----------------------------------------------------------
    //    enum class SynthModType
    //-----------------------------------------------------------
    enum class SynthModType : int
    {
        NULLARY = 0,
        UNARY = 1,
        BINARY = 2
    };
    
    //-----------------------------------------------------------
    //    enum class EGState
    //-----------------------------------------------------------
    enum class EGState : int
    {
        ATTACK = 301,
        DECAY = 302,
        SUSTAIN = 303,
        RELEASE = 304,
        OFF = 305
    };
    
    //-----------------------------------------------------------
    //    WaveType
    //-----------------------------------------------------------
    enum class WaveType : int
    {
        SIN = 201,
        SAW = 202,
        TRI = 203,
        SQU = 204
    };
    
    //-----------------------------------------------------------
    //    struct ADSR
    //-----------------------------------------------------------
    struct ADSR
    {
	    Real attack; // on the second time scale
	    Real decay;  // on the second time scale
	    Real sustain;// 0 to 1.0
	    Real release;// on the second time scale
    };
    
    //-----------------------------------------------------------
    //    struct MidiReceivable
    //-----------------------------------------------------------
    struct MidiReceivable{};
    
    template<typename T>
    struct IsMidiReceivable : public std::is_base_of<MidiReceivable, T> {};
}//---- namespace
#endif


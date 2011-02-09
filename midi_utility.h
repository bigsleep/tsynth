#ifndef SYKES_MIDI_UTILITY_H
#define SYKES_MIDI_UTILITY_H

#include <cstdint>
#include <cstddef>
#include <cmath>

namespace sykes{
namespace midi{
    extern std::uint8_t const min_note_number;
    extern std::uint8_t const max_note_number;
    extern std::uint8_t const note_table_size;
    extern double const note_table[];
    
    struct message
    {
        std::uint32_t data;
        
        inline static std::uint8_t channel(message _m)
        { return static_cast<std::uint8_t>(_m.data & 0xFF); }
        
        inline static std::uint8_t status(message _m)
        { return static_cast<std::uint8_t>(_m.data & 0xFF); }
        
        inline static std::uint8_t data1(message _m)
        { return static_cast<std::uint8_t>((_m.data >> 8) & 0xFF); }
        
        inline static std::uint8_t data2(message _m)
        { return static_cast<std::uint8_t>((_m.data >> 16) & 0xFF); }
    };
    
    struct channel_voice_message_type
    {
        static std::uint8_t const NOTE_OFF             = 0x80;
        static std::uint8_t const NOTE_ON              = 0x90;
        static std::uint8_t const POLYPHONIC_PRESSURE  = 0xA0;
        static std::uint8_t const CONTROL_CHANGE       = 0xB0;
        static std::uint8_t const PROGRAM_CHANGE       = 0xC0;
        static std::uint8_t const CHANNEL_PRESSURE     = 0xD0;
        static std::uint8_t const PITCH_BEND           = 0xE0;
    };
    typedef channel_voice_message_type CVMT;
    
    struct channel_mode_message_type
    {
        static std::uint8_t const ALL_SOUND_OFF        = 0x78;
        static std::uint8_t const RESET_ALL_CNTROLLERS = 0x79;
        static std::uint8_t const LOCAL_CONTROL        = 0x7A;
        static std::uint8_t const ALL_NOTES_OFF        = 0x7B;
        static std::uint8_t const OMNI_OFF             = 0x7C;
        static std::uint8_t const OMNI_ON              = 0x7D;
        static std::uint8_t const MONO                 = 0x7E;
        static std::uint8_t const POLY                 = 0x7F;
    };
    typedef channel_mode_message_type CMMT;
    
    struct system_message_type
    {
        static std::uint8_t const SYSTEM_EXCLUSIVE     = 0xF0;
        static std::uint8_t const SONG_POSITION        = 0xF2;
        static std::uint8_t const SONG_SELECT          = 0xF3;
        static std::uint8_t const TUNE_REQUEST         = 0xF6;
        static std::uint8_t const END_OF_EXCLUSIVE     = 0xF7;
        static std::uint8_t const TIMING_CLOCK         = 0xF8;
        static std::uint8_t const START                = 0xFA;
        static std::uint8_t const CONTINUE             = 0xFB;
        static std::uint8_t const ACCTIVE_SENSING      = 0xFE;
        static std::uint8_t const RESET                = 0xFF;
    };
    typedef system_message_type SMT;
    
    inline bool is_channel_message(message _m)
    {
        std::uint8_t stat = message::status(_m);
        return ((stat >= 0x80) && (stat < 0xF0));
    }
    
    inline bool is_system_message(message _m)
    {
        std::uint8_t stat = message::status(_m);
        return ((stat >= 0xF0) && (stat <= 0xFF));
    }
    
    inline message make_message(std::uint8_t _stat, std::uint8_t _d1, std::uint8_t _d2)
    {
        std::uint32_t i =
            ((_d2 << 16) & 0xFF0000) |
            ((_d2 << 8) & 0x00FF00) |
            (_stat & 0x0000FF); 
        return message{i};
    }
    
    inline message make_message(std::uint8_t const (&_d)[3])
    {
        std::uint32_t i =
            ((_d[2] << 16) & 0xFF0000) |
            ((_d[1] << 8) & 0x00FF00) |
            (_d[0] & 0x0000FF);
        return message{i};
    }
    
    inline std::uint8_t calc_note_number(double _f)
    {
        if(_f <= note_table[0]) return 0;
        static double const c1 = 1.0 / 440.0;
        std::uint8_t num = static_cast<std::uint8_t>(double(69.0 + 12.0 * std::log2(_f * c1) + 0.5));
        if(num >= note_table_size)
            return (note_table_size - 1);
        else
            return num;
    }
}}//----

#endif



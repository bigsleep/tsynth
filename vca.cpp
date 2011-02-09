//-----------------------------------------------------------
//    SynthVCA
//-----------------------------------------------------------
#ifndef SYNTH_VCA_H
#define SYNTH_VCA_H

#include "type.h"
#include "constants.h"
#include "midi_utility.h"
#include "eg.h"
#include "synth_mod.h"
#include <functional>

namespace TSynth{
    
    //-----------------------------------------------------------
    //    class SynthVCA
    //-----------------------------------------------------------
    class SynthVCA : MidiReceivable
    {
    public:
	    typedef Real result_type;
        inline SynthVCA()
            : m_level(Constants::vca_default_level), m_last_val(),
            m_level_function(), m_active(false), m_use_eg(true)
        {}
        
        inline SynthVCA(double _l, double _a, double _d, double _s, double _r)
            : m_level(_l), m_last_val(),
            m_level_function(_a, _d, _s, _r), m_active(false), m_use_eg(true)
        {}
        
        inline Real operator()(Real _in)
        {
            if(m_use_eg){
                m_last_val = _in * m_level * m_level_function();
                return m_last_val;
            }else{
                m_last_val = _in * m_level;
                return m_last_val;
            }
        }
        
        inline void SetLevel(Real _l)
        {
            if((_l >= 0.0)&&(_l <= 1.0)) m_level = _l;
        }
        
        inline Real GetLevel() const
        { return m_level; }
        
        inline bool IsActive() const
        {
            if(m_use_eg) return m_level_function.IsActive();
            else return m_active;
        }
        
        inline void UseEG(bool _b = true)
        {
            m_use_eg = _b;
        }
        
        inline void MidiReceive(sykes::midi::message _m)
        {
            m_level_function.MidiReceive(_m);
            switch(sykes::midi::message::status(_m))
            {
                case sykes::midi::channel_voice_message_type::NOTE_ON:
                    m_active = true;
                    return;
                case sykes::midi::channel_voice_message_type::NOTE_OFF:
                    m_active = false;
                    return;
                default:
                    return;
            }
        }
    private:
        Real m_level;
        Real m_last_val;
        SynthEG m_level_function;
        bool m_active;
        bool m_use_eg;
        
        TSYNTH_USE_AS_MOD
    };
    
    TSYNTH_DECLARE_UNARY_MOD(SynthVCA, (StoD(), StoD(), StoD(), StoD(), StoD()))
}

#endif



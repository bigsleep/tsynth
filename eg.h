//-----------------------------------------------------------
//    SynthEG
//-----------------------------------------------------------
#ifndef SYNTH_EG_H
#define SYNTH_EG_H

#include "type.h"
#include "constants.h"
#include "synth_mod_base.h"
#include <cstddef>

namespace TSynth{

    extern Real const inv_exp_table[];
    extern ADSR const default_ADSR;
    //-----------------------------------------------------------
    //    class SynthEG
    //-----------------------------------------------------------
    class SynthEG
    {
    public:
        SynthEG();
        SynthEG(Real _a, Real _d, Real _s, Real _r);
        
        Real operator()();
        
        inline void SetState(EGState _egs)
        {
	        ResetPhase();
            m_state = _egs;
        }
        
        inline EGState GetState() const
        { return m_state; }
        
        inline void ResetPhase()
        { m_phase = 0; }
        
        inline void SetAttack(Real _ac)
        {
            Real tmp = static_cast<Real>(SynthModBase::GetSampleRate()) * _ac;
            m_attack = static_cast<std::size_t>(tmp);
            m_attack_rate = 1.0 / tmp;
        }
        
        inline void SetDecay(Real _dc)
        {
            Real tmp = static_cast<Real>(SynthModBase::GetSampleRate()) * _dc;
            m_decay = static_cast<std::size_t>(tmp);
            m_delta_phase_decay = static_cast<Real>(Constants::inv_exp_table_size) / tmp;
        }
        
        inline void SetSustain(Real _sl)
        {
            if((_sl >= 0.0) && (_sl <= 1.0)){
                m_sustain = _sl;
                m_release_level = 1.0 - m_sustain;
            }
        }
        
        inline void SetRelease(Real _rc)
        {
            Real tmp = static_cast<Real>(SynthModBase::GetSampleRate()) * _rc;
            m_release = static_cast<std::size_t>(tmp);
            m_delta_phase_release = static_cast<Real>(Constants::inv_exp_table_size) / tmp;
        }
        
        inline void SetADSR(ADSR const& _adsr)
        {
            SetAttack(_adsr.attack);
            SetDecay(_adsr.decay);
            SetSustain(_adsr.sustain);
            SetRelease(_adsr.release);
        }
    
        inline void MidiReceive(sykes::midi::message _m)
        {
            switch(sykes::midi::message::status(_m))
            {
                case sykes::midi::channel_voice_message_type::NOTE_ON:
                    SetState(EGState::ATTACK);
                    return;
                case sykes::midi::channel_voice_message_type::NOTE_OFF:
                    m_release_level = m_last_val;
                    SetState(EGState::RELEASE);
                    return;
                default:
                    return;
            }
        }
        
        inline bool IsActive() const
        {
            return (m_state != EGState::OFF);
        }
    private:
        std::size_t m_phase;
        EGState m_state;
        std::size_t m_attack;
        std::size_t m_decay;
        Real m_sustain;
        std::size_t m_release;
        Real m_delta_phase_decay;
        Real m_delta_phase_release;
        Real m_release_level;
        Real m_attack_rate;
        Real m_last_val;
    };
}
#endif


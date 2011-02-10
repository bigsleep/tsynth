//-----------------------------------------------------------
//    eg.cpp
//-----------------------------------------------------------
#include "eg.h"
#include "constants.h"
#include <math.h>

namespace TSynth{
    
    ADSR const default_ADSR = {0.02, 0.1, 0.4, 0.2};    
    
    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    SynthEG::SynthEG() :
        m_phase(),
        m_state(EGState::OFF),
        m_attack(),
        m_decay(),
        m_sustain(),
        m_release(),
        m_delta_phase_decay(),
        m_delta_phase_release(),
        m_release_level(),
        m_attack_rate(),
        m_last_val()
    {
        SetADSR(default_ADSR);
    }
    
    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    SynthEG::SynthEG(Real _a, Real _d, Real _s, Real _r) :
        m_phase(),
        m_state(EGState::OFF),
        m_attack(),
        m_decay(),
        m_sustain(),
        m_release(),
        m_delta_phase_decay(),
        m_delta_phase_release(),
        m_release_level(),
        m_attack_rate(),
        m_last_val()
    {
        SetAttack(_a);
        SetDecay(_d);
        SetSustain(_s);
        SetRelease(_r);
    }
    
    //-----------------------------------------------------------
    //    
    //-----------------------------------------------------------
    Real SynthEG::operator()()
    {
        switch(GetState()){
            case EGState::ATTACK:
            {
                if(m_phase < m_attack){
                    m_last_val = (m_attack_rate * (Real)m_phase);
                    ++m_phase;
                    return m_last_val;
                }else{
                    SetState(EGState::DECAY);
                }
            }
            case EGState::DECAY:
            {
                if(m_phase < m_decay){
                    Real n1 = m_delta_phase_decay * (Real)m_phase;
                    m_last_val = (1.0 - m_sustain) * inv_exp_table[std::size_t(n1)] + m_sustain;
                    ++m_phase;
                    return m_last_val;
                }else{
                    SetState(EGState::SUSTAIN);
                }
            }
            case EGState::SUSTAIN:
            {
                m_last_val = m_sustain;
                return (m_sustain);
            }
            case EGState::RELEASE:
            {
                if(m_phase < m_release){
                    Real n1 = m_delta_phase_release * (Real)m_phase;
                    m_last_val = m_release_level * inv_exp_table[std::size_t(n1)];
                    ++m_phase;
                    return m_last_val;
                }else{
                    SetState(EGState::OFF);
                }
            }
            case EGState::OFF:
            default:
            {
                m_last_val = 0.0;
                m_phase = 0;
                return 0.0;
            }
        }
    }

}//---- namespace


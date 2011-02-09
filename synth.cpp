//-----------------------------------------------------------
//    Synth
//-----------------------------------------------------------

#include "synth.h"

namespace TSynth{
    Synth::Synth()
        :
        m_buffer_size(Constants::default_buffer_size),
        m_state(),
        m_synth(),
        m_buffer(m_buffer_size * 2, 0.0),
        m_pcm_out(SynthModBase::GetSampleRate(), m_buffer_size),
        m_midi_in("TSynth"),
        m_midi_event(),
        m_mutex()
    {
        std::fill(m_state.begin(), m_state.end(), MonoState{false, 0, 0});
        std::fill(m_midi_event.begin(), m_midi_event.end(), message{0});
        m_pcm_out.set_callback(std::bind(&Synth::OnPcm, this));
        m_midi_in.set_on_midi_event(std::bind(&Synth::OnMidiEvent, this,
            std::placeholders::_1, std::placeholders::_2));
    }
        
    Synth::~Synth()
    {
        Stop();
    }
        
    void Synth::Compose(std::string const& _str)
    {
        lock_type lk(m_mutex);
        m_synth[0].ComposeMonoSynthFromString(_str);
        for(std::size_t i = 1; i < max_poly; ++i){
            m_synth[i] = m_synth.front().Clone();
        }
    }
        
    void Synth::Start()
    {
        m_pcm_out.start();
        m_midi_in.start();
    }
        
    void Synth::Stop()
    {
        m_midi_in.stop();
        m_pcm_out.stop();
    }
            
    void Synth::OnPcm()
    {
        using namespace sykes::midi;
        decltype(m_state) state_copy;
        decltype(m_midi_event) midi_event_copy;
        {
            lock_type lk_stat(m_state_mutex);
            lock_type lk(m_midi_event_mutex);
            state_copy = m_state;
            midi_event_copy = m_midi_event;
            std::fill(m_midi_event.begin(), m_midi_event.end(), message{0});
        }
        
        lock_type lk(m_mutex);
        std::fill(m_buffer.begin(), m_buffer.end(), 0.0);
        for(std::size_t i = 0; i < max_poly; ++i){
            if(midi_event_copy[i].data != 0){
                m_synth[i].MidiReceive(midi_event_copy[i]);
            }
            if(state_copy[i].active){
                for(std::size_t j = 0; j < m_buffer_size; ++j){
                    double d = m_synth[i]();
                    m_buffer[j * 2] += d;
                    m_buffer[j * 2 + 1] += d;
                }
                if(!m_synth[i].IsActive()){
                    lock_type lk_state(m_state_mutex);
                    m_state[i] = MonoState{false, 0, 0};
                }
            }
        }
        m_pcm_out.write_buffer(m_buffer);
    }
    
    void Synth::OnMidiEvent(sykes::midi::message _m, ptime _t)
    {
        using namespace sykes::midi;
        std::uint8_t state = sykes::midi::message::status(_m);
        std::uint8_t data1 = sykes::midi::message::data1(_m);
        std::uint8_t data2 = sykes::midi::message::data2(_m);
        
        if(data2 == 0 && state == CVMT::NOTE_ON)
            state = CVMT::NOTE_OFF;
        
        switch(state)
        {
            case CVMT::NOTE_ON:
            {
                lock_type lk_stat(m_state_mutex);
                lock_type lk(m_midi_event_mutex);
                for(std::size_t i = 0; i < max_poly; ++i){
                    if(!m_state[i].active && m_midi_event[i].data == 0){
                        m_state[i].active = true;
                        m_state[i].note = data1;
                        m_state[i].velocity = data2;
                        m_midi_event[i] = _m;
                        return;
                    }
                }
                return;
            }
            case CVMT::NOTE_OFF:
            {
                lock_type lk_stat(m_state_mutex);
                lock_type lk(m_midi_event_mutex);
                for(std::size_t i = 0; i < max_poly; ++i){
                    if(m_state[i].note == data1 || message::data1(m_midi_event[i]) == data1){
                        m_midi_event[i] = make_message(state, data1, data2);
                    }
                }
                return;
            }
            default:
                return;
        }
    }

}


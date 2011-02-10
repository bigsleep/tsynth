//-----------------------------------------------------------
//    Synth
//-----------------------------------------------------------
#ifndef SYNTH_SYNTH_H
#define SYNTH_SYNTH_H

#include <cstddef>
#include <array>
#include <vector>
#include <string>

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "type.h"
#include "constants.h"
#include "mono_synth.h"
#include "alsa/alsa_pcm_out.h"
#include "alsa/alsa_midi_in.h"

namespace TSynth{
    
    //-----------------------------------------------------------
    //    class Synth
    //-----------------------------------------------------------
    class Synth
    {
    private:
        struct MonoState
        {
            bool active;
            std::uint8_t note;
            std::uint8_t velocity;
        };
        
    public:
        typedef float format_type;
        typedef boost::posix_time::ptime ptime;
        typedef sykes::midi::message message;
        static std::size_t const max_poly = 16;
        
        Synth();
        ~Synth();
        void Compose(std::string const& _str);
        void Start();
        void Stop();
        
    private:
        typedef sykes::alsa_pcm_out pcm_out_type;
        typedef sykes::alsa_midi_in midi_in_type;
        typedef boost::recursive_mutex mutex_type;
        typedef mutex_type::scoped_lock lock_type;
        std::size_t m_buffer_size;
        std::array<MonoState, max_poly> m_state;
        std::array<MonoSynth, max_poly> m_synth;
        std::vector<format_type> m_buffer; 
        pcm_out_type m_pcm_out;
        midi_in_type m_midi_in;
        std::array<sykes::midi::message, max_poly> m_midi_event;
        mutex_type m_mutex;
        mutex_type m_state_mutex;
        mutex_type m_midi_event_mutex;
        
        // private member functions
        void OnPcm();
        
        void OnMidiEvent(sykes::midi::message _m, ptime _t);
    };

}

#endif



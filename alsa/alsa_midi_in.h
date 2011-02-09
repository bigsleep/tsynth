#ifndef SYKES_ALSA_MIDI_IN_H
#define SYKES_ALSA_MIDI_IN_H

#include <alsa/asoundlib.h>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <memory>
#include <string>
#include <functional>
#include "midi_utility.h"

namespace sykes{
    
    struct snd_seq_t_deleter
    {
        inline snd_seq_t_deleter(){ }

        inline snd_seq_t_deleter(const snd_seq_t_deleter&) { }

        inline void operator()(snd_seq_t* _ptr) const
        {
            if(_ptr) snd_seq_close(_ptr);
        }
    };
    
    class alsa_midi_in
    {
    public:
        typedef boost::posix_time::ptime ptime;
        alsa_midi_in();
        alsa_midi_in(std::string const& _name);
        ~alsa_midi_in();
        
        void start();
        void stop();
        bool is_running() const;
        int port() const;
        int client() const;
        void set_on_midi_event(std::function<void(midi::message, ptime)> const& _f);
        
    private:
        typedef boost::recursive_mutex mutex_type;
        typedef mutex_type::scoped_lock lock_type;
        typedef boost::thread thread_type;
        
        bool m_running;
        std::string const m_name;
        int m_client_id;
        int m_port_id;
        std::unique_ptr<snd_seq_t, snd_seq_t_deleter> m_handle;
        std::vector<pollfd> m_poll;
        
        mutex_type mutable m_mutex;
        mutex_type mutable m_running_mutex;
        std::unique_ptr<boost::thread> m_worker;
        std::function<void(midi::message _m, ptime)> m_on_midi_event;
        
        void init();
        void routine();
    };
}

#endif




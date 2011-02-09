#include "alsa_midi_in.h"

namespace sykes{
    
    alsa_midi_in::alsa_midi_in()
        :
        m_running(false),
        m_name("sykes::alsa_midi_in"),
        m_client_id(-1),
        m_port_id(-1),
        m_handle(),
        m_poll(),
        m_mutex(),
        m_running_mutex(),
        m_worker(),
        m_on_midi_event()
    {
        init();
    }
    
    alsa_midi_in::alsa_midi_in(std::string const& _name)
        :
        m_running(false),
        m_name(_name),
        m_client_id(-1),
        m_port_id(-1),
        m_handle(),
        m_poll(),
        m_mutex(),
        m_running_mutex(),
        m_worker(),
        m_on_midi_event()
    {
        init();
    }
    
    alsa_midi_in::~alsa_midi_in()
    {
        {
            stop();
            if(m_port_id >= 0)
                snd_seq_delete_port(m_handle.get(), m_port_id);
        }
    }
    
    void alsa_midi_in::init()
    {
        if(!m_handle){
            snd_seq_t *handle = 0;
            snd_seq_open(&handle, "default", SND_SEQ_OPEN_DUPLEX, 0);
            if(!handle) std::runtime_error("alsa_midi_in::init()");
            m_handle = std::unique_ptr<snd_seq_t, snd_seq_t_deleter>(handle);
            snd_seq_set_client_name(handle, m_name.c_str());
        }
        
        if(m_port_id < 0){
            snd_seq_port_info_t *pinfo;
            snd_seq_port_info_alloca( &pinfo );
            snd_seq_port_info_set_capability( pinfo,
                SND_SEQ_PORT_CAP_WRITE |
                SND_SEQ_PORT_CAP_SUBS_WRITE );
            snd_seq_port_info_set_type( pinfo,
                SND_SEQ_PORT_TYPE_MIDI_GENERIC |
                SND_SEQ_PORT_TYPE_APPLICATION );
            snd_seq_port_info_set_name(pinfo, m_name.c_str());
            m_port_id = snd_seq_create_port(m_handle.get(), pinfo);
            if(m_port_id < 0) std::runtime_error("alsa_midi_in::init()");
            m_client_id = snd_seq_client_id(m_handle.get());
        }
        
        // poll
        {
            int const poll_count = snd_seq_poll_descriptors_count(m_handle.get(), POLLIN);
            if(poll_count > 0){
                m_poll.resize(poll_count, pollfd{0, 0, 0});
                int const count = snd_seq_poll_descriptors(m_handle.get(), &m_poll[0], poll_count, POLLIN);
                if(count < poll_count)
                    for(int i = 0, sz = (poll_count - m_poll.size()); i < sz; ++i)
                        m_poll.pop_back();
            }
            if(m_poll.size() == 0)
                throw std::runtime_error("alsa_midi_in::init()");
        }
    }
    
    void alsa_midi_in::set_on_midi_event(std::function<void(midi::message, ptime)> const& _f)
    {
        lock_type lk(m_mutex);
        m_on_midi_event = _f;
    }
    
    void alsa_midi_in::start()
    {
        lock_type lk(m_mutex);
        lock_type lk2(m_running_mutex);
        if(m_running) return;
        
        m_worker = std::unique_ptr<boost::thread>(new boost::thread(std::bind(&alsa_midi_in::routine, this)));
        m_running = true;
    }
    
    void alsa_midi_in::stop()
    {
        {
            lock_type lk(m_running_mutex);
            if(!m_running) return;
            m_running = false;
        }
        m_worker->join();
    }
    
    bool alsa_midi_in::is_running() const
    {
        lock_type lk(m_running_mutex);
        return m_running;
    }
    
    int alsa_midi_in::port() const
    {
        lock_type lk(m_mutex);
        return m_port_id;
    }
    
    int alsa_midi_in::client() const
    {
        lock_type lk(m_mutex);
        return m_client_id;
    }
    
    midi::message to_message(snd_seq_event_t const& _ev);
    
    void alsa_midi_in::routine()
    {
        snd_seq_drop_input(m_handle.get());
        while(1){
            {
                lock_type lk(m_running_mutex);
                if(!m_running) break;
            }
            {
                lock_type lk(m_mutex);
                
                if(poll(&m_poll[0], m_poll.size(), 1000) > 0){
                    ptime t = boost::posix_time::microsec_clock::universal_time();
                    for(std::size_t i = 0, sz = m_poll.size(); i < sz; ++i){
                        if(m_poll[i].revents > 0){
                            snd_seq_event_t *ev = 0;
                            snd_seq_event_input(m_handle.get(), &ev);
                            
                            if(ev != 0 && bool(m_on_midi_event)){
                                midi::message m = to_message(*ev);
                                m_on_midi_event(m, t);
                            }
                            snd_seq_free_event(ev);
                        }
                    }
                }
            }
        }
        snd_seq_drop_input(m_handle.get());
    }
    
    typedef std::unique_ptr<snd_midi_event_t, void(*)(snd_midi_event*)> snd_midi_event_t_ptr;
    snd_midi_event_t_ptr make_snd_midi_event_t_ptr(std::size_t size);
    
    midi::message to_message(snd_seq_event_t const& _ev)
    {
        static std::size_t const bsize = 12;
        static snd_midi_event_t_ptr midiev = make_snd_midi_event_t_ptr(bsize);
        
        std::uint8_t byte[bsize] = {};
        snd_midi_event_init(midiev.get());
        int size = snd_midi_event_decode(midiev.get(), &byte[0], bsize, &_ev);
        
        switch(size){
            case 0:
                return midi::message{0};
            case 1:
                return midi::message{byte[0]};
            case 2:
                return midi::message{std::uint32_t(
                    (byte[1] << 8) | (byte[0]))};
            case 3:
                return midi::message{std::uint32_t(
                    (byte[2] << 16) | (byte[1] << 8) | (byte[0]))};
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
                return midi::message{std::uint32_t(
                    (byte[3] << 24) | (byte[2] << 16) | (byte[1] << 8) | (byte[0]))};
            default:
                return midi::message{0};
        }
    }
    
    snd_midi_event_t_ptr make_snd_midi_event_t_ptr(std::size_t size)
    {
        snd_midi_event_t* midiev = 0;
        snd_midi_event_new(size, &midiev);
        return std::move(snd_midi_event_t_ptr(midiev, &snd_midi_event_free));
    }
}




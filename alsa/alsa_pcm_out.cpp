#include "alsa_pcm_out.h"

namespace sykes{
    
#define THROW_AT_ERROR(exp, tothrow) __throw_if((exp), (tothrow))
#define STRINGIZE(x) STRINGIZE_IMPL(x)
#define STRINGIZE_IMPL(x) #x

    template<typename Except>
    inline void __throw_if(bool _b, Except _ex)
    {
        if(_b) throw _ex;
    }
    
    alsa_pcm_out::alsa_pcm_out()
        :
        m_running(false),
        m_sample_rate(alsa_pcm_default::sample_rate),
        m_buffer_size(alsa_pcm_default::buffer_size),
        m_periods(),
        m_pcm_format(),
        m_handle(),
        m_poll(),
        m_transfer_buffer_float(),
        m_transfer_buffer_short(),
        m_callback(),
        m_worker(),
        m_mutex(),
        m_running_mutex(),
        m_buffer_mutex()
    {
        open_device();
        open_poll();
        set_parameter();
    }
    
    alsa_pcm_out::alsa_pcm_out(std::size_t _sample_rate, std::size_t _buffer_size)
        :
        m_running(false),
        m_sample_rate(_sample_rate),
        m_buffer_size(_buffer_size),
        m_periods(),
        m_pcm_format(),
        m_handle(),
        m_poll(),
        m_transfer_buffer_float(),
        m_transfer_buffer_short(),
        m_callback(),
        m_worker(),
        m_mutex(),
        m_running_mutex(),
        m_buffer_mutex()
    {
        open_device();
        open_poll();
        set_parameter();
    }
    
    alsa_pcm_out::~alsa_pcm_out()
    {
        {
        stop();
        }
    }
    
    void alsa_pcm_out::start()
    {
        lock_type lk(m_mutex);
        lock_type lk2(m_running_mutex);
        if(m_running) return;
        
        m_worker = std::unique_ptr<boost::thread>(new boost::thread(&alsa_pcm_out::routine, this));
        m_running = true;
    }
    
    void alsa_pcm_out::stop()
    {
        {
            lock_type lk(m_running_mutex);
            if(!m_running) return;
            m_running = false;
        }
        m_worker->join();
    }
    
    void alsa_pcm_out::write_buffer(std::vector<buffer_format_type> const& _data)
    {
        std::size_t const size = std::min(m_buffer_size * m_channel_count, _data.size());
        switch(m_pcm_format){
            case SND_PCM_FORMAT_FLOAT:
            {
                auto it = m_transfer_buffer_float.begin();
                lock_type lk(m_buffer_mutex);
                std::copy(_data.begin(), _data.begin() + size, it);
                break;
            }
            case SND_PCM_FORMAT_S16:
            {
                static std::int16_t const max = std::numeric_limits<std::int16_t>::max();
                static buffer_format_type const scale = static_cast<buffer_format_type>(max);
                for(std::size_t i = 0; i < size; ++i){
                    m_transfer_buffer_short[i] = static_cast<std::int16_t>(_data[i] * scale);
                }
                break;
            }
            default:
                break;
        }
    }
    
    void alsa_pcm_out::set_callback(std::function<void(void)> const& _f)
    {
        lock_type lk(m_mutex);
        m_callback = _f;
    }
    
    bool alsa_pcm_out::is_running() const
    {
        lock_type lk(m_running_mutex);
        return m_running;
    }
    
    std::size_t alsa_pcm_out::buffer_size() const
    {
        lock_type lk(m_mutex);
        return m_buffer_size;
    }
    
    std::size_t alsa_pcm_out::sample_rate() const
    {
        lock_type lk(m_mutex);
        return m_sample_rate;
    }
    
    void alsa_pcm_out::set_parameter()
    {
        if(m_handle){
            // setting hardware parameters
            snd_pcm_hw_params_t *hw_params = 0;
            typedef std::unique_ptr<snd_pcm_hw_params_t, void(*)(snd_pcm_hw_params_t*)> hw_params_ptr;
            
            THROW_AT_ERROR( snd_pcm_hw_params_malloc(&hw_params) < 0,
                std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__)));
            hw_params_ptr hwp(hw_params, &snd_pcm_hw_params_free);
            THROW_AT_ERROR(
                snd_pcm_hw_params_any(m_handle.get(), hw_params) < 0,
                std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__)));
            THROW_AT_ERROR(
                snd_pcm_hw_params_set_access(m_handle.get(), hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0,
                std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__)));
            THROW_AT_ERROR( snd_pcm_hw_params_set_rate_near(m_handle.get(), hw_params, &m_sample_rate, 0),
                std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__)));
            THROW_AT_ERROR( snd_pcm_hw_params_set_channels(m_handle.get(), hw_params, 2) < 0,
                std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__)));
            THROW_AT_ERROR( snd_pcm_hw_params_set_period_size(m_handle.get(), hw_params, m_buffer_size, 0) < 0,
                std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__)));
            
            // set format
            if( snd_pcm_hw_params_set_format(m_handle.get(), hw_params, SND_PCM_FORMAT_FLOAT) == 0 ){
                m_pcm_format = SND_PCM_FORMAT_FLOAT;
                m_transfer_buffer_float.resize(m_buffer_size * m_channel_count, 0.0);
            }else if( snd_pcm_hw_params_set_format(m_handle.get(), hw_params, SND_PCM_FORMAT_S16) == 0 ){
                m_pcm_format = SND_PCM_FORMAT_S16;
                m_transfer_buffer_short.resize(m_buffer_size * m_channel_count, 0.0);
            }else
                throw std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__));
            
            // set periods
            m_periods = alsa_pcm_default::periods;
            while(snd_pcm_hw_params_set_periods(m_handle.get(), hw_params, m_periods, 0) < 0
                && m_periods < alsa_pcm_default::max_periods) ++m_periods;
            if(m_periods >= alsa_pcm_default::max_periods)
                throw std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__));
            
            THROW_AT_ERROR( snd_pcm_hw_params(m_handle.get(), hw_params) < 0,
                std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__)));

            // setting software parameters
            snd_pcm_sw_params_t *sw_params = 0;
            typedef std::unique_ptr<snd_pcm_sw_params_t, void(*)(snd_pcm_sw_params_t*)> sw_params_ptr;
            
            THROW_AT_ERROR( snd_pcm_sw_params_malloc(&sw_params) < 0,
                std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__)));
            sw_params_ptr swp(sw_params, &snd_pcm_sw_params_free);
            
            THROW_AT_ERROR( snd_pcm_sw_params_current(m_handle.get(), sw_params) < 0,
                std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__)));
            THROW_AT_ERROR( snd_pcm_sw_params_set_avail_min(m_handle.get(), sw_params, m_buffer_size) < 0,
                std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__)));
            THROW_AT_ERROR( snd_pcm_sw_params_set_tstamp_mode(m_handle.get(), sw_params, SND_PCM_TSTAMP_ENABLE) < 0,
                std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__)));
            THROW_AT_ERROR( snd_pcm_sw_params(m_handle.get(), sw_params) < 0,
                std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__)));
        }else{
            throw std::runtime_error("in alsa_pcm_out::set_parameter() at line:" STRINGIZE(__LINE__));
        }
    }
    
    void alsa_pcm_out::open_device()
    {
        {
            snd_pcm_t* handle_tmp = 0;
            int error = snd_pcm_open(
                &handle_tmp,
                "default",
                SND_PCM_STREAM_PLAYBACK,
                0
            );
            if(error) throw std::runtime_error("alsa_pcm_out::open_device() " STRINGIZE(__LINE__));
            m_handle = std::unique_ptr<snd_pcm_t, snd_pcm_t_deleter>(handle_tmp);
        }
    }
    
    void alsa_pcm_out::open_poll()
    {
        if(m_handle){
            int const poll_count = snd_pcm_poll_descriptors_count(m_handle.get());
            if(poll_count > 0){
                m_poll.resize(poll_count, pollfd{0, 0, 0});
                int const count = snd_pcm_poll_descriptors(m_handle.get(), &m_poll[0], poll_count);
                if(count < poll_count)
                    for(int i = 0, sz = (poll_count - m_poll.size()); i < sz; ++i) m_poll.pop_back();
            }
        }else{
            throw std::runtime_error("alsa_pcm_out::open_poll()");
        }
    }
    
    void alsa_pcm_out::routine()
    {
        switch(m_pcm_format)
        {
            case SND_PCM_FORMAT_FLOAT:
                routine_for_float_format();
                break;
            case SND_PCM_FORMAT_S16:
                routine_for_short_format();
                break;
            default:
                break;
        }
    }
    
    void alsa_pcm_out::routine_for_float_format()
    {
        while(1){
            {
                lock_type lk(m_running_mutex);
                if(!m_running) break;
            }
            {
                lock_type lk(m_mutex);
                if(poll(&m_poll[0], m_poll.size(), alsa_pcm_default::poll_wait) > 0) {
                    for(std::size_t i = 0, sz = m_poll.size(); i < sz; ++i){
                        if(m_poll[i].revents > 0){
                            if(m_callback) m_callback();
                            {
                                lock_type lk_buffer(m_buffer_mutex);
                                if(snd_pcm_writei(m_handle.get(), &m_transfer_buffer_float[0], m_buffer_size)
                                    < int(m_buffer_size))
                                {
                                    THROW_AT_ERROR( snd_pcm_prepare(m_handle.get()) != 0,
                                        std::runtime_error("alsa_pcm_out::routine_for_float_format()" STRINGIZE(__LINE__)));
                                }
                            }
                        }
                    }
                }
                boost::this_thread::sleep(boost::posix_time::milliseconds(1));
            }
        }
    }
    
    
    void alsa_pcm_out::routine_for_short_format()
    {
        while(1){
            {
                lock_type lk(m_running_mutex);
                if(!m_running) break;
            }
            {
                lock_type lk(m_mutex);
                if(poll(&m_poll[0], m_poll.size(), alsa_pcm_default::poll_wait) > 0) {
                    for(std::size_t i = 0, sz = m_poll.size(); i < sz; ++i){
                        if(m_poll[i].revents > 0){
                            if(m_callback) m_callback();
                            
                            lock_type lk_buffer(m_buffer_mutex);
                            if(snd_pcm_writei(m_handle.get(), &m_transfer_buffer_short[0], m_buffer_size)
                                < int(m_buffer_size))
                            {
                                THROW_AT_ERROR( snd_pcm_prepare(m_handle.get()) != 0,
                                    std::runtime_error("alsa_pcm_out::routine_for_short_format()" STRINGIZE(__LINE__)));
                            }
                        }
                    }
                }
            }
        }
    }
}//----





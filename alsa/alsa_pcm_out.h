#ifndef ALSA_PCM_OUT_H
#define ALSA_PCM_OUT_H

// linux
#include <sys/poll.h>
// alsa
#include <alsa/asoundlib.h>
// boost
#include <boost/detail/endian.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
// std
#include <cstddef>
#include <vector>
#include <memory>
#include <functional>
#include <cstdint>
#include <type_traits>
#include <limits>


namespace sykes{
    
    struct alsa_pcm_default
    {
        static std::size_t const sample_rate = 44100;
        static std::size_t const buffer_size = 1024;
        static std::size_t const periods = 3;
        static std::size_t const max_periods = 12;
        static int const poll_wait = 1000;
    };
    
    template<typename T>
    struct alsa_pcm_format
    {
        static snd_pcm_format_t const value = SND_PCM_FORMAT_UNKNOWN;
    };
    
    template<snd_pcm_format_t Format>
    struct alsa_pcm_format_type{};
    
    struct snd_pcm_t_deleter
    {
        inline void operator()(snd_pcm_t* _ptr) const
        {
            if(_ptr) snd_pcm_close(_ptr);
        }
    };
    
    class alsa_pcm_out
    {
    public:
        typedef float buffer_format_type;
        
        alsa_pcm_out();
        alsa_pcm_out(std::size_t _sample_rate, std::size_t _buffer_size);
        ~alsa_pcm_out();
        
        void start();
        void stop();
        
        void set_callback(std::function<void(void)> const& _f);
        bool is_running() const;
        std::size_t buffer_size() const;
        std::size_t periods() const;
        std::size_t sample_rate() const;
        void write_buffer(std::vector<buffer_format_type> const& _data);
        
    private:
        typedef boost::recursive_mutex mutex_type;
        typedef mutex_type::scoped_lock lock_type;
        typedef boost::thread thread_type;
        static std::size_t const m_channel_count = 2;
        
        bool m_running;
        std::size_t m_sample_rate;
        std::size_t m_buffer_size;
        std::size_t m_periods;
        snd_pcm_format_t m_pcm_format;
        
        std::unique_ptr<snd_pcm_t, snd_pcm_t_deleter> m_handle;
        std::vector<pollfd> m_poll;
        
        std::vector<float> m_transfer_buffer_float;
        std::vector<std::int16_t> m_transfer_buffer_short;
        std::function<void(void)> m_callback;
        std::unique_ptr<thread_type> m_worker;
        mutex_type mutable m_mutex;
        mutex_type mutable m_running_mutex;
        mutex_type mutable m_buffer_mutex;
        
        void set_parameter();
        void open_device();
        void open_poll();
        void routine();
        void routine_for_float_format();
        void routine_for_short_format();
    };

#define SYKES_ALSA_PCM_FORMAT(tp, val) \
    template<> \
    struct alsa_pcm_format< tp >\
    { static snd_pcm_format_t const value =  val ; };\
    template<> \
    struct alsa_pcm_format_type< val >\
    { typedef tp type ; };
    
    SYKES_ALSA_PCM_FORMAT(float, SND_PCM_FORMAT_FLOAT)
    SYKES_ALSA_PCM_FORMAT(double, SND_PCM_FORMAT_FLOAT64)
    SYKES_ALSA_PCM_FORMAT(std::int8_t, SND_PCM_FORMAT_S8)
    SYKES_ALSA_PCM_FORMAT(std::uint8_t, SND_PCM_FORMAT_U8)
    SYKES_ALSA_PCM_FORMAT(std::int16_t, SND_PCM_FORMAT_S16)
    SYKES_ALSA_PCM_FORMAT(std::uint16_t, SND_PCM_FORMAT_U16)
    SYKES_ALSA_PCM_FORMAT(std::int32_t, SND_PCM_FORMAT_S32)
    SYKES_ALSA_PCM_FORMAT(std::uint32_t, SND_PCM_FORMAT_U32)

#undef SYKES_ALSA_PCM_FORMAT

}//----
#endif





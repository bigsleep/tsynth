//-----------------------------------------------------------
//    SynthVCO
//-----------------------------------------------------------
#ifndef SYNTH_CONSTANTS_H
#define SYNTH_CONSTANTS_H

#include "type.h"
#include <vector>
#include <functional>

namespace TSynth{

    struct Constants
    {
        static std::size_t const default_sample_rate = 44100;
        static std::size_t const default_buffer_size = 2048;
        
        static WaveType const vco_default_wave_type = WaveType::SIN;
        static std::size_t const vco_default_frequency = 1000;
        static std::size_t const vco_wave_table_size = 10240;
        static Real const vco_min_frequency = 8.0;
        static Real const vco_max_frequency = 12000.0;
        
        static std::size_t const vcf_filter_table_size = 10240;
        static Real const vcf_min_cutoff = 200.0;
        static Real const vcf_max_cutoff = 10000.0;
        static Real const vcf_default_cutoff = 2000.0;
        static std::size_t const inv_exp_table_size = 10240;
        
        static Real const vca_default_level = 0.5;
        
        static std::size_t const id_root_vca = 0;
        
    };

}//---- namespace
#endif


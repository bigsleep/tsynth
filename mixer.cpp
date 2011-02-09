//-----------------------------------------------------------
//    Mixer
//-----------------------------------------------------------
#include "type.h"
#include "synth_mod.h"

namespace TSynth{
    //-----------------------------------------------------------
    //    class Mixer
    //-----------------------------------------------------------
    class Mixer
    {
    public:
	    typedef Real result_type;
        Mixer(){}
        
        inline Real operator()(Real _in1, Real _in2) const
        {
            return (_in1 + _in2);
        }
    private:
        TSYNTH_USE_AS_MOD
    };

    TSYNTH_DECLARE_BINARY_MOD(Mixer, ())
}//---- namespace


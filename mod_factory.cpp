//-----------------------------------------------------------
//    
//-----------------------------------------------------------
#include "type.h"
#include "mod_factory.h"

namespace TSynth{
    
    SynthModBasePtr SynthModFactory::Create(key_type const& _cmd)
    {
        return GetInstance().m_dispatcher.call(_cmd);
    }
    
    SynthModBasePtr SynthModFactory::Create(std::vector<key_type> const& _cmd)
    {
        return GetInstance().m_dispatcher.call(_cmd);
    }
    
    SynthModFactory& SynthModFactory::GetInstance()
    {
        static SynthModFactory the_instance;
        return the_instance;
    }
}//---- namespace



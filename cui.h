//-----------------------------------------------------------
//    Cui
//-----------------------------------------------------------
#ifndef CUI_H
#define CUI_H

#include <iostream>
#include <string>
#include <functional>
#include "command_dispatcher.h"
#include "synth.h"

namespace TSynth{
    
    class Cui
    {
    public:
        inline Cui(Synth& synth, std::istream& _i, std::ostream& _o)
            : m_synth(synth),
            m_in(_i), m_out(_o),
            m_cmd(MakeCommandDispatcher(synth))
        {
        }
        
        inline void Run()
        {
            Routine();
        }
    private:
        Synth& m_synth;
        std::istream& m_in;
        std::ostream& m_out;
        sykes::command_dispatcher<void> const m_cmd;
        
        inline void Routine()
        {
            while(1){
                std::string tmp;
                tmp.reserve(100);
                
                m_out << "> ";
                m_out.flush();
                std::getline(m_in, tmp);
                
                if(tmp == "quit") break;
                
                try{
                    m_cmd.call(tmp);
                }
                catch(sykes::command_dispatch_error const& _err)
                {
                    m_out << "command error: " << _err.what() << std::endl;
                }
                catch(...)
                {
                    throw;
                }
            }
        }
        
        inline sykes::command_dispatcher<void>
        MakeCommandDispatcher(Synth& synth)
        {
            sykes::command_dispatcher<void> tmp;
            tmp.register_command(
                "start",
                std::bind(&Synth::Start, &synth));
            tmp.register_command(
                "stop",
                std::bind(&Synth::Stop, &synth));
            tmp.register_command(
                "compose",
                std::bind(&Synth::Compose, &synth, std::placeholders::_1),
                sykes::nocast());
            return tmp;
        }
    };

}

#endif



//-----------------------------------------------------------
//    
//-----------------------------------------------------------
#ifndef SYNTH_MOD_FACTORY_H
#define SYNTH_MOD_FACTORY_H

#include "command_dispatcher.h"
#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <type_traits>
#include <boost/lexical_cast.hpp>

namespace TSynth{
    class SynthModBase;
    typedef std::shared_ptr<SynthModBase> SynthModBasePtr;
    
    class SynthModFactory
    {
    public:
        typedef std::string key_type;
        
        template<typename F, typename ... CastFuncs>
        static key_type RegisterMod(key_type const& _key, F f, std::tuple<CastFuncs ...> const& cfunc)
        {
            GetInstance().m_dispatcher.register_command(_key, f, cfunc);
            return _key;
        }
        
        static SynthModBasePtr Create(key_type const& _cmd);
        static SynthModBasePtr Create(std::vector<key_type> const& _cmd);
    private:
        SynthModFactory(){}
        static SynthModFactory& GetInstance();
        sykes::command_dispatcher<SynthModBasePtr> m_dispatcher;
    };
    
    template<typename Tp>
    struct AModFactory
    {
        typedef SynthModBasePtr result_type;
        template<typename ... T>
        SynthModBasePtr operator()(T ... arg) const
        {
            return SynthModBasePtr(new Tp(arg...));
        }
        
        SynthModBasePtr operator()() const
        {
            return SynthModBasePtr(new Tp());
        }
    };
    
    struct StoD
    {
        inline double operator()(std::string const& _str) const
        {
            return boost::lexical_cast<double>(_str);
        }
    };
    
#define TSYNTH_USE_AS_MOD \
    public: \
    std::string const& ModName() const { return tsynth_mod_key; } \
    private: \
    static std::string const tsynth_mod_key; \

#define TSYNTH_DECLARE_NULLARY_MOD(cls, cast_funcs) \
    std::string const cls::tsynth_mod_key = \
        SynthModFactory::RegisterMod(#cls, AModFactory<NullaryMod<cls>>(), std::make_tuple cast_funcs);

#define TSYNTH_DECLARE_UNARY_MOD(cls, cast_funcs) \
    std::string const cls::tsynth_mod_key = \
        SynthModFactory::RegisterMod(#cls, AModFactory<UnaryMod<cls>>(), std::make_tuple cast_funcs);

#define TSYNTH_DECLARE_BINARY_MOD(cls, cast_funcs) \
    std::string const cls::tsynth_mod_key = \
        SynthModFactory::RegisterMod(#cls, AModFactory<BinaryMod<cls>>(), std::make_tuple cast_funcs);
}//---- namespace
#endif



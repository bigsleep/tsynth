//-----------------------------------------------------------
//    
//-----------------------------------------------------------
#ifndef SYNTH_MOD_BASE_H
#define SYNTH_MOD_BASE_H

#include "type.h"
#include "midi_utility.h"
#include "constants.h"
#include "tree.h"

#include <string>
#include <memory>
#include <functional>
#include <type_traits>

#include <iostream>

namespace TSynth{
    
    class SynthModBase;
    class SynthVCA;
    
    typedef std::shared_ptr<SynthModBase> SynthModBasePtr;
    typedef creek::tree<std::function<Real(void)>>::const_child_iterator TreeFunctionIterator;
    
    inline Real __Zero()
    { return 0.0; }
    
    //-----------------------------------------------------------
    //    class SynthModBase
    //-----------------------------------------------------------
    class SynthModBase
    {
    public:
        typedef Real result_type;
        
        inline SynthModType GetType() const
        { return m_type; }
        
        inline static void SetSampleRate(std::size_t _sr)
        {
            SynthModBase::SampleRate<>::value = _sr;
        }
        
        inline static std::size_t GetSampleRate()
        {
            return SynthModBase::SampleRate<>::value;
        }
        
        inline SynthModBasePtr Clone() const
        {
            return SynthModBasePtr(m_NewAsBase());
        }
        
        inline explicit SynthModBase(SynthModType _t)
            : m_type(_t)
        {}
        
        inline void MidiReceive(sykes::midi::message _message)
        {
            m_MidiReceive(_message);
        }
        
        inline bool IsActive() const
        {
            return m_IsActive();
        }
        
        std::string const& Name() const
        {
            return m_Name();
        }
        
        inline std::function<Real(void)>
        MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end)
        {
            return m_MakeFunction(_it, _end);
        }
    private:
        SynthModType const m_type;
        
        template<bool B = true>
        struct SampleRate
        {
            static std::size_t value;
        };
        virtual SynthModBase* m_NewAsBase() const = 0;
        inline virtual void m_MidiReceive(sykes::midi::message _message){}
        virtual std::function<Real(void)>
        m_MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end) = 0;
        virtual std::string const& m_Name() const = 0;
        virtual bool m_IsActive() const
        { return true; }
    };
    
    template<bool B>
    std::size_t SynthModBase::template SampleRate<B>::value = Constants::default_sample_rate;
    
    //-----------------------------------------------------------
    //    class NullaryMod
    //-----------------------------------------------------------
    template<typename ModT, typename = void>
    class NullaryMod
        : public SynthModBase
    {
    public:
        NullaryMod()
            : SynthModBase(SynthModType::NULLARY), m_mod()
        {}
        
        template<typename ... Ts>
        NullaryMod(Ts ... args)
            : SynthModBase(SynthModType::NULLARY), m_mod(args...)
        {}
        
    private:
        ModT m_mod;
        
        inline virtual std::function<Real(void)>
        m_MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end)
        {
            return std::ref(m_mod);
        }
        
        inline virtual SynthModBase* m_NewAsBase() const
        {
            return new NullaryMod(*this);
        }
        
        inline virtual std::string const& m_Name() const
        {
            return m_mod.ModName();
        }
    };
    
    // specialization for midi receivable mod
    template<typename ModT>
    class NullaryMod<ModT, typename std::enable_if<IsMidiReceivable<ModT>::value, void>::type>
        : public SynthModBase
    {
    public:
        NullaryMod()
            : SynthModBase(SynthModType::NULLARY), m_mod()
        {}
        
        template<typename ... Ts>
        NullaryMod(Ts ... args)
            : SynthModBase(SynthModType::NULLARY), m_mod(args...)
        {}
        
    private:
        ModT m_mod;
        
        inline virtual std::function<Real(void)>
        m_MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end)
        {
            return std::ref(m_mod);
        }
        
        inline virtual SynthModBase* m_NewAsBase() const
        {
            return new NullaryMod(*this);
        }
        
        inline virtual void m_MidiReceive(sykes::midi::message _message)
        {
            m_mod.MidiReceive(_message);
        }
        
        inline virtual std::string const& m_Name() const
        {
            return m_mod.ModName();
        }
    };
    
    //-----------------------------------------------------------
    //    class UnaryMod
    //-----------------------------------------------------------
    template<typename ModT, typename = void>
    class UnaryMod : public SynthModBase
    {
    public:
        UnaryMod()
            : SynthModBase(SynthModType::UNARY), m_mod()
        {}
        
        template<typename ... Ts>
        UnaryMod(Ts ... args)
            : SynthModBase(SynthModType::UNARY), m_mod(args...)
        {}
        
    private:
        ModT m_mod;
        
        inline virtual std::function<Real(void)>
        m_MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end)
        {
            assert(_it != _end);
            return std::bind(&ModT::operator(), std::ref(m_mod), std::bind(*_it));
        }
        
        inline virtual SynthModBase* m_NewAsBase() const
        {
            return new UnaryMod(*this);
        }
        
        inline virtual std::string const& m_Name() const
        {
            return m_mod.ModName();
        }
    };
    
    // specialization for midi receivable mod
    template<typename ModT>
    class UnaryMod<ModT,
        typename std::enable_if<
            IsMidiReceivable<ModT>::value
            && !std::is_same<ModT, SynthVCA>::value, void>::type>
        : public SynthModBase
    {
    public:
        UnaryMod()
            : SynthModBase(SynthModType::UNARY), m_mod()
        {}
        
        template<typename ... Ts>
        UnaryMod(Ts ... args)
            : SynthModBase(SynthModType::UNARY), m_mod(args...)
        {}
        
    private:
        ModT m_mod;
        
        inline virtual std::function<Real(void)>
        m_MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end)
        {
            assert(_it != _end);
            return std::bind(&ModT::operator(), std::ref(m_mod), std::bind(*_it));
        }
        
        inline virtual SynthModBase* m_NewAsBase() const
        {
            return new UnaryMod(*this);
        }
        
        inline virtual void m_MidiReceive(sykes::midi::message _message)
        {
            m_mod.MidiReceive(_message);
        }
        
        inline virtual std::string const& m_Name() const
        {
            return m_mod.ModName();
        }
    };
    
    // specialization for vca
    template<typename ModT>
    class UnaryMod<ModT, typename std::enable_if<std::is_same<ModT, SynthVCA>::value, void>::type>
        : public SynthModBase
    {
    public:
        UnaryMod()
            : SynthModBase(SynthModType::UNARY), m_mod()
        {}
        
        template<typename ... Ts>
        UnaryMod(Ts ... args)
            : SynthModBase(SynthModType::UNARY), m_mod(args...)
        {}
        
    private:
        ModT m_mod;
        
        inline virtual std::function<Real(void)>
        m_MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end)
        {
            assert(_it != _end);
            return std::bind(&ModT::operator(), std::ref(m_mod), std::bind(*_it));
        }
        
        inline virtual SynthModBase* m_NewAsBase() const
        {
            return new UnaryMod(*this);
        }
        
        inline virtual void m_MidiReceive(sykes::midi::message _message)
        {
            m_mod.MidiReceive(_message);
        }
        
        inline virtual std::string const& m_Name() const
        {
            return m_mod.ModName();
        }
        
        inline virtual bool m_IsActive() const
        {
            return m_mod.IsActive();
        }
    };
    //-----------------------------------------------------------
    //    class BinaryMod
    //-----------------------------------------------------------
    template<typename ModT, typename = void>
    class BinaryMod : public SynthModBase
    {
    public:
        BinaryMod()
            : SynthModBase(SynthModType::BINARY), m_mod()
        {}
        
        template<typename ... Ts>
        BinaryMod(Ts ... args)
            : SynthModBase(SynthModType::BINARY), m_mod(args...)
        {}
        
    private:
        ModT m_mod;
        
        inline std::function<Real(void)>
        m_MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end)
        {
            std::function<Real(void)> tmp;
            if(_it != _end){
                tmp = (*_it);
                ++_it;
            }else{
                tmp = &__Zero;
            }
            
            while(_it != _end){
                assert(bool(*_it));
                tmp = std::bind(
                    &ModT::operator(),
                    std::ref(m_mod),
                    std::bind(*_it),
                    std::bind(tmp)
                );
                ++_it;
            }
            return tmp;
        }
        
        inline virtual SynthModBase* m_NewAsBase() const
        {
            return new BinaryMod(*this);
        }
        
        inline virtual std::string const& m_Name() const
        {
            return m_mod.ModName();
        }
    };
    
    // specialization for midi receivable mod
    template<typename ModT>
    class BinaryMod<ModT, typename std::enable_if<IsMidiReceivable<ModT>::value, void>::type>
        : public SynthModBase
    {
    public:
        BinaryMod()
            : SynthModBase(SynthModType::BINARY), m_mod()
        {}
        
        template<typename ... Ts>
        BinaryMod(Ts ... args)
            : SynthModBase(SynthModType::BINARY), m_mod(args...)
        {}
        
    private:
        ModT m_mod;
        
        inline virtual std::function<Real(void)>
        m_MakeFunction(TreeFunctionIterator _it, TreeFunctionIterator _end)
        {
            std::function<Real(void)> tmp;
            if(_it != _end){
                tmp = (*_it);
                ++_it;
            }else{
                tmp = &__Zero;
            }
            
            while(_it != _end){
                assert(bool(*_it));
                tmp = std::bind(
                    &ModT::operator(),
                    std::ref(m_mod),
                    std::bind(*_it),
                    std::bind(tmp)
                );
                ++_it;
            }
            return tmp;
        }
        
        inline virtual SynthModBase* m_NewAsBase() const
        {
            return new BinaryMod(*this);
        }
        
        inline virtual void m_MidiReceive(sykes::midi::message _message)
        {
            m_mod.MidiReceive(_message);
        }
        
        inline virtual void m_Name() const
        {
            return m_mod.ModName();
        }
    };
}//---- namespace
#endif


//-----------------------------------------------------------
//    MonoSynth
//-----------------------------------------------------------
#ifndef MONO_SYNTH_H
#define MONO_SYNTH_H

#include "type.h"
#include "tree.h"
#include "midi_utility.h"
#include "synth_mod_base.h"

namespace TSynth{
    class SynthModBase;
    //-----------------------------------------------------------
    //    class MonoSynth
    //-----------------------------------------------------------
    class MonoSynth
    {
    public:
        typedef std::shared_ptr<SynthModBase> SynthModBasePtr;
        typedef creek::tree<SynthModBasePtr>::preorder_iterator Iterator;
        typedef creek::tree<SynthModBasePtr>::const_preorder_iterator ConstIterator;
        typedef int IdType;
        
        MonoSynth();
        ~MonoSynth();
        
        inline Real operator()() const
        {
            return m_function();
        }
        
        void MidiReceive(sykes::midi::message _m);
        
        Iterator Insert(Iterator _it, IdType _id, SynthModBasePtr _mod);
        
        Iterator AppendChild(Iterator _it, IdType _id, SynthModBasePtr _mod);
        
        void AddMod(IdType _id, SynthModBasePtr _mod);
        
        void SetParameterMod(Iterator _it, IdType _id, SynthModBasePtr _mod);
        
        inline Iterator Begin()
        { return m_mod_tree.preorder_begin(); }
        
        inline Iterator End()
        { return m_mod_tree.preorder_end(); }
        
        inline ConstIterator Begin() const
        { return m_mod_tree.preorder_begin(); }
        
        inline ConstIterator End() const
        { return m_mod_tree.preorder_end(); }
        
        bool CheckTree() const;
        
        inline void Clear();
                
        void Swap(MonoSynth& _other);
        
        inline bool IsActive() const
        {
            return (bool(m_root_vca)) ? m_root_vca->IsActive() : false;
        }
    private:
        //typedef boost::bimaps::bimap<IdType, SynthModBasePtr> MapType;
        creek::tree<SynthModBasePtr> m_mod_tree;
        //MapType m_mod_map;
        std::function<Real(void)> m_function;
        SynthModBasePtr m_root_vca;
        
        
    public:
        void BindModsInTree();
        void ComposeMonoSynthFromString(std::string const& _str);
        MonoSynth Clone() const;
    };
}//---- namespace

#endif


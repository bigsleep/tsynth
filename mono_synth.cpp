//-----------------------------------------------------------
//    monosynth.cpp
//-----------------------------------------------------------
#include <stdexcept>
#include "mono_synth.h"
#include "synth_mod_base.h"
#include "mod_factory.h"
#include "eg.h"
#include "parse.h"

namespace TSynth{
    inline Real _Zero(void) { return 0.0; }
    
    MonoSynth::MonoSynth()
            : m_mod_tree(),
            m_function(&_Zero) {}
        
    MonoSynth::~MonoSynth(){}
    
    void MonoSynth::BindModsInTree()
    {
        if(!CheckTree()) throw std::runtime_error("parse error");
        
        if((**(m_mod_tree.preorder_begin())).Name() == "SynthVCA"){
            m_root_vca = *(m_mod_tree.preorder_begin());
        }else{
            m_mod_tree.insert(m_mod_tree.preorder_begin(), SynthModFactory::Create("SynthVCA 0.1 0.02 0.1 0.4 0.2"));
            m_root_vca = *(m_mod_tree.preorder_begin());
        }
        
        creek::tree<std::function<Real(void)>> func_tree;
        {
            auto it_mod = m_mod_tree.preorder_begin();
            auto end_mod = m_mod_tree.preorder_end();
            auto it_func = func_tree.preorder_begin();
            
            it_func = func_tree.insert(it_func, std::function<Real(void)>());
            while(it_mod != end_mod){
                auto it_child = it_mod.begin();
                auto end_child = it_mod.end();
                while(it_child != end_child){
                    func_tree.append_child(it_func, std::function<Real(void)>());
                    ++it_child;
                }
                ++it_mod;
                ++it_func;
            }
        }
        {
            auto it_mod = m_mod_tree.postorder_begin();
            auto end_mod = m_mod_tree.postorder_end();
            auto it_func = func_tree.postorder_begin();
            while(it_mod != end_mod){
                (*it_func) = (**it_mod).MakeFunction(it_func.begin(), it_func.end());
                ++it_mod;
                ++it_func;
            }
            
            m_function = *(func_tree.preorder_begin());
        }
    }
    
    void MonoSynth::ComposeMonoSynthFromString(std::string const& _str)
    {
        if(Parse(_str, m_mod_tree)){
            BindModsInTree();
        }
    }
    
    MonoSynth MonoSynth::Clone() const
    {
        // copy mod tree
        MonoSynth tmp;
        {
            auto it = (*this).m_mod_tree.preorder_begin();
            auto end = (*this).m_mod_tree.preorder_end();
            auto it_copy = tmp.m_mod_tree.preorder_begin();
            if(it != end){
                it_copy = tmp.m_mod_tree.insert(it_copy, (**it).Clone());
                //tmp.m_mod_map.insert(MapType::value_type(m_mod_map.right.at(*it), *it_copy));
            }
            while(it != end){
                auto it_child = it.begin();
                auto end_child = it.end();
                while(it_child != end_child){
                    auto ch = tmp.m_mod_tree.append_child(it_copy, (**it_child).Clone());
                    //tmp.m_mod_map.insert(MapType::value_type(m_mod_map.right.at(*it_child), *ch));
                    ++it_child;
                }
                ++it;
                ++it_copy;
            }
        }
        
        tmp.BindModsInTree();
        return tmp;
    }
    
    void MonoSynth::Clear()
    {
        m_mod_tree.clear();
        m_function = &_Zero;
        m_root_vca.reset();
    }
    
    bool MonoSynth::CheckTree() const
    {
        creek::tree<SynthModBasePtr>::const_preorder_iterator curr = m_mod_tree.preorder_begin();
        creek::tree<SynthModBasePtr>::const_preorder_iterator end = m_mod_tree.preorder_end();
        
        if(curr == end) return false;
        while(curr != end){
            auto modtype = (**curr).GetType();
            switch(modtype)
            {
                case SynthModType::NULLARY:
                    if(curr.has_child()) return false;
                    break;
                case SynthModType::UNARY:
                    if(!curr.has_child()) return false;
                    break;
                case SynthModType::BINARY:
                    if(!curr.has_child()) return false;
                    break;
            };
            ++curr;
        }
        return true;
    }
    
#if 0
    MonoSynth::Iterator MonoSynth::Insert(Iterator _it, IdType _id, SynthModBasePtr _mod)
    {
        m_mod_map.insert(MapType::value_type(_id, _mod));
        return m_mod_tree.insert(_it, _mod);
    }
    
    MonoSynth::Iterator MonoSynth::AppendChild(Iterator _it, IdType _id, SynthModBasePtr _mod)
    {
        m_mod_map.insert(MapType::value_type(_id, _mod));
        return m_mod_tree.append_child(_it, _mod);
    }
    
    void MonoSynth::AddMod(IdType _id, SynthModBasePtr _mod)
    {
        m_mod_map.insert(MapType::value_type(_id, _mod));
    }
#endif
    
    void MonoSynth::MidiReceive(sykes::midi::message _m)
    {
        auto curr = m_mod_tree.preorder_begin(), end = m_mod_tree.preorder_end();
        
        while(curr != end){
            (**curr).MidiReceive(_m);
            ++curr;
        }
    }
    
    void MonoSynth::Swap(MonoSynth& _other)
    {
        m_mod_tree.swap(_other.m_mod_tree);
        m_function.swap(_other.m_function);
        m_root_vca.swap(_other.m_root_vca);
    }
}//---- namespace


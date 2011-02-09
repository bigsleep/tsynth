#include <vector>
#include <functional>
#include <numeric>
#include <boost/lexical_cast.hpp>
#include "parse.h"
#include "mod_factory.h"

namespace TSynth{
    bool Parse(std::string const& _strexpr, creek::tree<SynthModBasePtr>& _tree)
    {
        if(_strexpr == "") return false;
        
        typedef creek::tree<SynthModBasePtr>::preorder_iterator preorder_iterator;
        creek::tree<SynthModBasePtr> treetmp;
        preorder_iterator curr = treetmp.preorder_begin();
        
        std::string::const_iterator it_str = _strexpr.begin(), it_end = _strexpr.end();
        int rbracket_count = 0, lbracket_count = 0;
        
        
        creek::tree<std::string> treestr;
        auto itstr = treestr.preorder_begin();
        while(1){
            std::string token = GetToken(it_str, it_end);
            
            if(token == "") break;
            
            if(token == "("){
                ++rbracket_count;
                token = GetToken(it_str, it_end);
                if(token == "(" || token == ")" || token == "")
                    return false;
                
                if(treetmp.empty()){
                    curr = treetmp.insert(curr, MakeSynthModFromString(token));
                    itstr = treestr.insert(itstr, token);
                }else{
                    curr = treetmp.append_child(curr, MakeSynthModFromString(token));
                    itstr = treestr.append_child(itstr, token);
                }
            }else if(token == ")"){
                if(treetmp.empty()) return false;
                ++lbracket_count;
                curr = curr.get_parent();
                itstr = itstr.get_parent();
            }else{
                if(treetmp.empty()) return false;
                treetmp.append_child(curr, MakeSynthModFromString(token));
                treestr.append_child(itstr, token);
            }
            
        }
        
        if(rbracket_count != lbracket_count) return false;
        
        _tree = treetmp;
        return true;
    }

    std::vector<std::string> SplitString(std::string const _str, std::string const& _delimiters = " []\t")
    {
        std::string::const_iterator it1 = _str.begin(), end1 = _str.end();
        std::vector<std::string> splited;
        splited.reserve(10);
        
        while(it1 != end1){
            while(_delimiters.find(*it1) != std::string::npos && (it1 != end1))
                ++it1;
            std::string strtmp;
            strtmp.reserve(10);
            
            while(_delimiters.find(*it1) == std::string::npos && (it1 != end1)){
                strtmp.push_back(*it1);
                ++it1;
            }
            if(strtmp != "") splited.push_back(strtmp);
        }
        return std::move(splited);
    }

    std::string GetToken(std::string::const_iterator& _it, std::string::const_iterator _end)
    {
        while(1){
            if(_it == _end) return std::string();
            char c = *_it;
            if(c != ' ' && c != '\t' && c != '\r' && c != '\n')
                break;
            else
                ++_it;
        }
        
        if((*_it) == '('){
            ++_it;
            return std::string("(");
        }
        if((*_it) == ')'){
            ++_it;
            return std::string(")");
        }
        if((*_it) == ']') throw std::runtime_error("TSynth::GetToken()");
        
        std::string tmp;
        while(1){
            if(_it == _end) return tmp;
            char c = *_it;
            if(c == '['){
                tmp.push_back(c);
                ++_it;
                while(1){
                    if(*_it == ']'){
                        tmp.push_back(*_it);
                        ++_it;
                        break;
                    }else if(*_it == '[')
                        throw std::runtime_error("TSynth::GetToken()");
                    else{
                        tmp.push_back(*_it);
                        ++_it;
                    }
                }
            }else if(c != ' ' && c != '\t' && c != '\r' && c != '\n' && c != '(' && c != ')'){
                tmp.push_back(c);
                ++_it;
            }else return tmp;
        }
    }
    
    SynthModBasePtr MakeSynthModFromString(std::string const& _str)
    {
        return SynthModFactory::Create(SplitString(_str));
    }
}


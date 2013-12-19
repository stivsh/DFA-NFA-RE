#ifndef STATE_H
#define STATE_H
#include <list>
#include <vector>
#include <map>
#include <set>
using namespace std;
#include <transition.h>
class State{
    bool Final;
    set<pair<State*, char> > ToThisStateLink;
    bool EpsilonFrom;
    vector<Transition> transitions;
public:
    void addTransition(Transition &transition){
        if(transition.isEpsilon()){
            EpsilonFrom=true;
            transitions.push_back(transition);
            if(transitions.size()>1){
                for(size_t i=0;i<transitions.size()-1;++i){
                    if(!transitions[i].isEpsilon()){
                        std::swap(transitions[i],transitions[transitions.size()-1]);
                    }
                }
            }
        }else{
            transitions.push_back(transition);
        }
        transition.to->ToThisStateLink.insert(make_pair(this,transition.Signal()));
    }
    void addTransition(State* _to,char _signal=0, bool _epsilon=false){
        Transition transition(_to,_signal,_epsilon);
        addTransition(transition);
    }
    State(bool _Final=false):Final(_Final),EpsilonFrom(false){}
    void addTransitions(map<char, set<State*> > &transmitions){
        map<char, set<State*> >::iterator it=transmitions.begin();
        for(;it!=transmitions.end();++it){
            set<State*>::iterator it2=it->second.begin();
            for(;it2!=it->second.end();++it2){
                addTransition(*it2,it->first);
            }
        }

    }
    set<pair<State*, char> >& getToThisStateLink(){
        return ToThisStateLink;
    }

    void ClearTransitions(set<State*> &all_states){
        for(vector<Transition>::iterator it=transitions.begin(); it!=transitions.end();++it){
            if(all_states.count(it->to))
                it->to->ToThisStateLink.erase(make_pair(this,it->Signal()));
        }
        transitions.clear();
        EpsilonFrom=false;
    }
    void setFinal(bool _Final){
        Final=_Final;
    }
    bool isFinal(){
        return Final;
    }
    bool isEpsilonFrom(){
        return EpsilonFrom;
    }

    vector<Transition>& getTransitions(){return transitions;}

};
#endif // STATE_H

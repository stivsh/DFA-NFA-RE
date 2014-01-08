#ifndef STATE_H
#define STATE_H
#include <list>
#include <vector>
#include <map>
#include <set>
#include "transmissioncollection.h"
using namespace std;
#include <transition.h>
#include "incomingtransmitions.h"
class State{
    bool final;
    IncomingTransmitions incoming_;
    bool epsilon_from;
    TransmissionCollection transitions;
public:
    IncomingTransmitions& incoming(){return incoming_;}
    State(bool _final=false):final(_final),incoming_(this),epsilon_from(false),transitions(this){}
    bool is_indexed(){return false;}
    void set_final(bool _final){
        final=_final;
    }
    bool is_final(){
        return final;
    }
    TransmissionCollection& get_transitions(){return transitions;}
    bool clouser_operation(std::set<State*> *all_states){//return true if this state must be final
        if(!transitions.has_epsilon())return final;
        bool set_is_final=final;
        map<char, set<State*> > transmitions; //список связей
        set<State*> TraversedStates;
        vector<State*> next_states;next_states.reserve(10);
        next_states.push_back(this);
        while(next_states.size()){
            State* nstate=next_states.back();
            next_states.pop_back();
            TraversedStates.insert(nstate);
            if(nstate->is_final())set_is_final=true;
            TransmissionCollection& transitions=nstate->get_transitions();
            transition_iter it=transitions.begin();
            for(;it!=transitions.end();++it){
                if(it->is_epsilon()&&!TraversedStates.count(it->to())){
                    next_states.push_back(it->to());
                }else{
                    transmitions[it->signal()].insert(it->to());
                }
            }
        }
        transitions.clear_transitions(all_states);
        transitions.add_transition(transmitions);
        final=set_is_final;
        return set_is_final;
    }
    ostream& operator>>(ostream& os){
        os.write(reinterpret_cast<char*>(&final),sizeof(bool));
        os.write(reinterpret_cast<char*>(&epsilon_from),sizeof(bool));
        return os;
    }
    istream& operator<<(istream& is){
        is.read(reinterpret_cast<char*>(&final),sizeof(bool));
        is.read(reinterpret_cast<char*>(&epsilon_from),sizeof(bool));
        return is;
    }
};
#endif // STATE_H

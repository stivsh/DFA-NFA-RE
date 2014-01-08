#include "transmissioncollection.h"
#include "transition.h"
#include "state.h"
using namespace  std;
TransmissionCollection::TransmissionCollection(State* _this_state):has_epsilon_(false),this_state(_this_state){}
transition_iter TransmissionCollection::begin()const{
    return transitions.begin();
}
transition_iter TransmissionCollection::end()const{
    return transitions.end();
}
bool TransmissionCollection::exist(State* state, char signal,bool epsinon)const{
    for(transition_iter it=begin();it!=end();++it){
        if(it->signal()==signal && it->to()==state && it->is_epsilon()==epsinon)
            return true;
    }
    return false;
}
bool TransmissionCollection::exist(const Transition& t)const{
    for(transition_iter it=begin();it!=end();++it){
        if(*it==t)
            return true;
    }
    return false;
}
void TransmissionCollection::erase(State* state,char signal){
    for(transition_iter it=transitions.begin();it!= transitions.end();++it){
        if(it->signal()==signal && it->to()==state){
            it->to()->incoming().erase(this_state,signal);
            transitions.erase(it);
            return;
        }
    }
}

bool TransmissionCollection::has_epsilon(){return has_epsilon_;}
void TransmissionCollection::add_transition(Transition transition){
    if(transition.is_epsilon())
        has_epsilon_=true;
    transitions.insert(transition);
    transition.to()->incoming().insert(this_state,transition.signal());
}
void TransmissionCollection::add_transition(State* _to,char _signal, bool _epsilon){
    Transition transition(_to,_signal,_epsilon);
    add_transition(transition);
}
void TransmissionCollection::clear_transitions(std::set<State*> *states){
    if(states){
        for(transition_iter it=begin(); it!=end();++it){
            if(states->count(it->to()))
                it->to()->incoming().erase(this_state,it->signal());
        }
    }
    transitions.clear();
    has_epsilon_=false;
}
void TransmissionCollection::add_transition(std::map<char, std::set<State*> > &transmitions){
    map<char, set<State*> >::iterator it=transmitions.begin();
    for(;it!=transmitions.end();++it){
        set<State*>::iterator it2=it->second.begin();
        for(;it2!=it->second.end();++it2){
            add_transition(*it2,it->first);
        }
    }
}
State * TransmissionCollection::get_next_state(char signal){
    for(transition_iter bit=transitions.begin();bit!=transitions.end();++bit){
        if(bit->signal()==signal){
            return bit->to();
        }
    }
    return 0;
}
std::size_t TransmissionCollection::size(){
    return transitions.size();
}
bool TransmissionCollection::operator==(const TransmissionCollection& tc){
    return transitions==tc.transitions;
}
void TransmissionCollection::load_from_stream(istream& is,std::map<State*, State*>& new_state_to_old_state){
    is.read(reinterpret_cast<char*>(&has_epsilon_),sizeof(bool));
    size_t transition_count;
    is.read(reinterpret_cast<char*>(&transition_count),sizeof(size_t));
    for(size_t i=0;i!=transition_count;++i){
        Transition tran(0,0);
        tran<<is;
        add_transition(new_state_to_old_state[tran.to()],tran.signal(),tran.is_epsilon());
    }
}
ostream& TransmissionCollection::operator>>(ostream& os){
    os.write(reinterpret_cast<char*>(&has_epsilon_),sizeof(bool));
    size_t tran_size=transitions.size();
    os.write(reinterpret_cast<char*>(&tran_size),sizeof(size_t));
    for(set<Transition>::iterator it=transitions.begin();it!=transitions.end();++it){
        *it>>os;
    }
    return os;
}

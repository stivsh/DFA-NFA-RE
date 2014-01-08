#include "incomingtransmitions.h"
#include "state.h"
using namespace std;
IncomingTransmitions::IncomingTransmitions(State* this_state_):this_state(this_state_){}
bool IncomingTransmitions::contain(State* state, char signal){
    if(incoming_links.count(state)&&incoming_links[state].count(signal))
        return true;
    else
        return false;

}

size_t IncomingTransmitions::size() const{return incoming_links.size();}
intransition_iter IncomingTransmitions::begin(){return incoming_links.begin();}
intransition_iter IncomingTransmitions::end(){return incoming_links.end();}
void IncomingTransmitions::insert(State* state,char signal){
    incoming_links[state].insert(signal);
}
void IncomingTransmitions::erase(State* state,char signal){
    if(incoming_links.count(state)){
        incoming_links[state].erase(signal);
        if(!incoming_links[state].size()){
            incoming_links.erase(state);
        }
    }
}
void IncomingTransmitions::back_delete_transition_to(State* state,char signal){
    state->get_transitions().erase(this_state,signal);
}

void IncomingTransmitions::back_delete_all_transition_to(){
    while(incoming_links.size()){
        intransition_iter it=incoming_links.begin();
        State* state=it->first;
        char signal=*(it->second.begin());
        state->get_transitions().erase(this_state,signal);
    }
}

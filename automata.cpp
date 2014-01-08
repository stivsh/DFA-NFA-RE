#include "automata.h"
void Automata::clear(bool delete_states){
    if(delete_states){
        set<State*>::iterator it=all_states.begin();
        for(;it!=all_states.end();++it){
            delete_state(*it,false);
        }
    }
    all_states.clear();
    finals.clear();
    start=0;
}


State* Automata::create_state(bool final, set<State*>* state_set){
    State* state=new State(final);
    if(!state_set)
        all_states.insert(state);
    else
        state_set->insert(state);
    return state;
}
void Automata::delete_state(State* state, bool erase){
    if(all_states.count(state))
        delete state;
    if(erase)all_states.erase(state);

}
void Automata::delete_unachievable(){
    //TODO test
    while(true){
        bool there_is_no_unachievable_states=true;
        set<State*>::iterator it= all_states.begin();
        while(it!=all_states.end()){
            if((*it)!=start && !(*it)->incoming().size()){
                (*it)->get_transitions().clear_transitions(&all_states);
                delete_state(*it);
                there_is_no_unachievable_states=false;

            }
            it++;
        }
        if(there_is_no_unachievable_states)break;
    }

}
void Automata::epsilon_nfa_to_nfa(){
    set<State*> new_finals;
    for(set<State*>::iterator it=all_states.begin();it!=all_states.end();++it){
        if((*it)->clouser_operation(&all_states)){
            new_finals.insert(*it);
        }
    }
    delete_unachievable();
    finals.clear();
    set_intersection(new_finals.begin(),new_finals.end(),all_states.begin(),all_states.end(),inserter(finals,finals.begin()));
}


Automata::Automata(bool epsilon):min(true),dfa(true){//create epsilon/zero automata;
    if(epsilon){
        start=create_state();
        State* final=create_state(true);
        start->get_transitions().add_transition(final,0,true);
        finals.push_back(final);
    }else{
        start=create_state();
        finals.push_back(create_state(true));
    }
}
Automata::Automata(char Char):min(true),dfa(true){//creape char atomata;
    start=create_state();
    State* finalState=create_state(true);
    start->get_transitions().add_transition(finalState,Char);
    finals.push_back(finalState);
}
Automata::Automata(Automata& a_obj){
    start=a_obj.start;
    finals=a_obj.finals;
    min=a_obj.min;
    dfa=a_obj.dfa;
    all_states=a_obj.all_states;
    a_obj.clear(false);
}
Automata& Automata::operator=(Automata& a_src){
    clear();
    start=a_src.start;
    finals=a_src.finals;
    min=a_src.min;
    dfa=a_src.dfa;
    all_states=a_src.all_states;

    a_src.clear(false);
    return *this;
}
void Automata::clone_to(Automata& dist){
    dist.clear();
    map<State*, State*> old_state_to_new;
    dist.min=min;
    dist.dfa=dfa;
    for(set<State*>::iterator it=all_states.begin();it!=all_states.end();++it){
        State* new_state=dist.create_state((*it)->is_final());
        if((*it)->is_final())
            dist.finals.push_back(new_state);
        if(*it==start)
            dist.start=new_state;
        old_state_to_new.insert(make_pair(*it,new_state));
    }
    for(set<State*>::iterator it=all_states.begin();it!=all_states.end();++it){
        TransmissionCollection& transition=(*it)->get_transitions();
        State* new_state=old_state_to_new[*it];
        for(transition_iter tit=transition.begin();tit!=transition.end();++tit){
            State* to=old_state_to_new[tit->to()];
            char signal=tit->signal();
            bool epsilon=tit->is_epsilon();
            new_state->get_transitions().add_transition(to,signal,epsilon);
        }
    }

}

Automata::~Automata(){clear();}
State* Automata::step(State* state, char signal){
    return state->get_transitions().get_next_state(signal);
}
State* Automata::get_start(){
    return start;
}

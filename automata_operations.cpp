#include "automata.h"
Automata& Automata::concatinate(Automata& automata){
    if(!automata.start->incoming().size()){
        TransmissionCollection& transition = automata.start->get_transitions();
        for(list<State*>::iterator it=finals.begin();it!=finals.end();++it){
            transition_iter tit=transition.begin();
            for(;tit!=transition.end();++tit){
                (*it)->get_transitions().add_transition(*tit);
            }
            (*it)->set_final(automata.start->is_final());
        }
        automata.start->get_transitions().clear_transitions(&automata.all_states);
        automata.delete_state(automata.start);
    }else{
        list<State*>::iterator it=finals.begin();
        Transition tran(automata.start,0,true);
        for(;it!=finals.end();++it){
            (*it)->set_final(false);
            (*it)->get_transitions().add_transition(tran);
        }
    }
    finals=automata.finals;
    automata.finals.clear();
    automata.start=NULL;
    copy(automata.all_states.begin(), automata.all_states.end(),
        inserter( all_states, all_states.begin() ) );
    automata.all_states.clear();
    min=false;
    dfa=false;
    return *this;
}
Automata& Automata::Union(Automata& automata){
    if(!start->incoming().size()&& !automata.start->incoming().size()){
        TransmissionCollection& transition = automata.start->get_transitions();
        transition_iter tit=transition.begin();
        set<State*> finals_to_delete;
        for(;tit!=transition.end();++tit){//быстрое слияние для коротких автоматов
            if(tit->to()->is_final()&&!tit->to()->is_indexed()&&!tit->to()->get_transitions().size()
                    &&finals.size()&&!finals.front()->get_transitions().size()&&!finals.front()->is_indexed()){

                start->get_transitions().add_transition(finals.front(),tit->signal(), tit->is_epsilon());
                finals_to_delete.insert(tit->to());
            }else{
                start->get_transitions().add_transition(*tit);
            }
        }
        for(set<State*>::iterator it=finals_to_delete.begin();it!=finals_to_delete.end();++it){
            automata.delete_state(*it);
            automata.finals.remove(*it);
        }
        if(automata.start->is_final() || start->is_final()){
            start->set_final(true);
            finals.push_back(start);
        }
        automata.start->get_transitions().clear_transitions(&automata.all_states);
        if(automata.start->is_final())automata.finals.remove(automata.start);
        automata.delete_state(automata.start);

        copy(automata.finals.begin(), automata.finals.end(),
            inserter( finals, finals.begin() ) );

    }else{
        State* new_start=create_state();
        new_start->get_transitions().add_transition(start,0,true);
        new_start->get_transitions().add_transition(automata.start,0,true);
        start=new_start;

        State* final_state=create_state(true);
        list<State*>::iterator it=finals.begin();
        Transition tran(final_state,0,true);
        for(;it!=finals.end();++it){
            (*it)->set_final(false);
            (*it)->get_transitions().add_transition(tran);
        }
        for(it=automata.finals.begin();it!=automata.finals.end();++it){
            (*it)->set_final(false);
            (*it)->get_transitions().add_transition(tran);
        }
        finals.clear();
        finals.push_back(final_state);
    }

    copy(automata.all_states.begin(), automata.all_states.end(),
        inserter( all_states, all_states.begin() ) );
    automata.start=NULL;
    automata.all_states.clear();
    automata.finals.clear();
    min=false;
    dfa=false;
    return *this;
}


Automata& Automata::knee_star(){//TODO immediately to DFA if posible
    State* start_state=create_state();
    State* final_state=create_state(true);
    start_state->get_transitions().add_transition(final_state,0,true);
    start_state->get_transitions().add_transition(start,0,true);

    list<State*>::iterator it=finals.begin();
    Transition tranToF(final_state,0,true);
    Transition tranToS(start,0,true);
    for(;it!=finals.end();++it){
        (*it)->set_final(false);
        (*it)->get_transitions().add_transition(tranToF);
        (*it)->get_transitions().add_transition(tranToS);
    }
    start=start_state;
    finals.clear();
    finals.push_back(final_state);

    min=false;
    dfa=false;
    return *this;
}
Automata& Automata::Union(char signal){
    list<State*>::iterator it=finals.begin();
    for(;it!=finals.end();++it){
        start->get_transitions().add_transition(*it,signal);
    }
    return *this;
}
Automata& Automata::concatinate(char signal){
    State* FinalState=create_state(true);
    list<State*>::iterator it=finals.begin();
    Transition tran(FinalState,signal);
    for(;it!=finals.end();++it){
        (*it)->set_final(false);
        (*it)->get_transitions().add_transition(tran);
    }
    finals.clear();
    finals.push_back(FinalState);
    return *this;
}
Automata& Automata::operator*(char signal){
    return Union(signal);
}
Automata& Automata::operator*(Automata& automata){
    return Union(automata);
}
Automata& Automata::operator+(char signal){
    return concatinate(signal);
}
Automata& Automata::operator+(Automata& automata){
    return concatinate(automata);
}
bool Automata::has_unachievable_states(){
    queue<State*> states;
    set<State*> traversed_states;
    traversed_states.insert(start);
    states.push(start);
    while(states.size()){
        State* state=states.front();
        states.pop();
        transition_iter tit=state->get_transitions().begin();
        transition_iter tend=state->get_transitions().end();
        for(;tit!=tend;++tit){
            if(!traversed_states.count(tit->to())){
                states.push(tit->to());
                traversed_states.insert(tit->to());
            }
        }
    }
    if(traversed_states.size()<all_states.size())
        return true;
    return false;

}
bool Automata::has_dead_end_states(){
    for(set<State*>::iterator it=all_states.begin();it!=all_states.end();++it){
        if(!(*it)->get_transitions().size()&&!(*it)->is_final())
            return true;
    }
    return false;
}

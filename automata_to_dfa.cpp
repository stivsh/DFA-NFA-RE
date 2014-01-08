#include "automata.h"
typedef set<State*> TODFAStateType;
typedef map<char, TODFAStateType > TODFATarnsitions;
typedef pair< TODFAStateType, TODFATarnsitions > TODFAStateANDTransitions;

bool Automata::is_dfa(){
   return dfa;
}
void get_new_state_type_transitions(TODFAStateType& state, TODFATarnsitions& transitions){
    TODFAStateType::iterator sit=state.begin();
    for(;sit!=state.end();++sit){
        transition_iter tit=(*sit)->get_transitions().begin();
        transition_iter endt=(*sit)->get_transitions().end();
        for(;tit!=endt;++tit){
            transitions[tit->signal()].insert(tit->to());
        }
    }
}
void get_new_power_sets(list<TODFAStateANDTransitions > &states,State* start){
    set<TODFAStateType> TraversedNewStates;
    TODFAStateType new_start;
    new_start.insert(start);
    queue<TODFAStateType > next_states;
    next_states.push(new_start);
    while(next_states.size()){
        TODFAStateType& next_state=next_states.front();
        TODFATarnsitions state_transitions;
        //1)заполняем связи состояния
        get_new_state_type_transitions(next_state, state_transitions);
        //2)добавляем в очередь всех кого нет в state_transitions
        states.push_back(TODFAStateANDTransitions(next_state,state_transitions));
        next_states.pop();
        TODFATarnsitions::iterator it=state_transitions.begin();
        for(;it!=state_transitions.end();++it){
            if(!TraversedNewStates.count(it->second)){
                next_states.push(it->second);
                TraversedNewStates.insert(it->second);
            }
        }
    }
}
State* create_new_staes_for_dfa(list<TODFAStateANDTransitions >& new_set_states, set<State*>& new_states, list<State*>& new_finals, map<set<State*>,  State* >& newToOldStatesMap,Automata* aut){
    State* new_start=0;
    list<TODFAStateANDTransitions >::iterator it=new_set_states.begin();
    for(;it!=new_set_states.end();++it){
        State* new_state=aut->create_state(false,&new_states);
        //старый старт
        if(it->first.size()==1 && it->first.count(aut->get_start())){
            new_start=new_state;
        }
        for(set<State*>::iterator sit=it->first.begin();sit!=it->first.end();++sit){
            if((*sit)->is_final()){
                new_state->set_final(true);
                new_finals.push_back(new_state);
            }
        }
        newToOldStatesMap.insert(pair<set<State*>&,State*>(it->first,new_state));
    }
    return new_start;
}
void create_transitions_for_dfa(list<TODFAStateANDTransitions >& new_states, map<set<State*>,  State* >& newToOldStatesMap){
    list<TODFAStateANDTransitions >::iterator it=new_states.begin();
    for(;it!=new_states.end();++it){
        State* state = newToOldStatesMap[it->first];
        TODFATarnsitions& transitions=it->second;
        TODFATarnsitions::iterator tit=transitions.begin();
        for(;tit!=transitions.end();++tit){
            state->get_transitions().add_transition(newToOldStatesMap[tit->second],tit->first);
        }
    }
}

void Automata::to_dfa(){
    epsilon_nfa_to_nfa();
    //посути мы тут создаём автомат заново проходя все его вершины из старта
    list<TODFAStateANDTransitions > new_set_states;
    //получаем список новых состояний, состоящих из состояний-множеств старых
    get_new_power_sets(new_set_states,start);
    //получили обсолюто новый автомат.
    set<State*> new_states;
    list<State*> new_finals;
    //создаём новые состояния;
    map<set<State*>,  State* > newToOldStatesMap;
    State* new_start=create_new_staes_for_dfa(new_set_states,new_states,new_finals,newToOldStatesMap,this);
    //создаём связи для новых состояний
    create_transitions_for_dfa(new_set_states,newToOldStatesMap);
    //чистим старый
    clear();
    //заполняем новый
    start=new_start;
    all_states=new_states;
    finals=new_finals;
    dfa=true;
}


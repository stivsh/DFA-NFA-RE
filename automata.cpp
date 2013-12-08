#include "automata.h"
//TEMP
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
void Automata::Clear(){
    set<State*>::iterator it=all_states.begin();
    for(;it!=all_states.end();++it){
        DeleteState(*it,false);
    }
}


State* Automata::CreateState(bool final, set<State*>* state_set){
    State* state=new State(final);
    if(!state_set)
        all_states.insert(state);
    else
        state_set->insert(state);
    return state;
}
void Automata::DeleteState(State* state, bool erase){
    delete state;
    if(erase)all_states.erase(state);

}
set<State*> Automata::GetReachableStates(){
    set<State*> TraversedStates;
    queue<State*> next_states;
    next_states.push(start);
    while(next_states.size()){
        State* nstate=next_states.front();
        next_states.pop();
        TraversedStates.insert(nstate);
        vector<Transition>& transitions=nstate->getTransitions();
        vector<Transition>::iterator it=transitions.begin();
        for(;it!=transitions.end();++it){
            if(!TraversedStates.count(it->to))
                next_states.push(it->to);
        }
    }
    return TraversedStates;
}


bool Automata::GetNewTransmitiomForEpsilonOutState(State* state){
    if(!state->isEpsilonFrom())return state->isFinal();
    bool set_is_final=state->isFinal();
    map<char, set<State*> > transmitions; //список связей
    set<State*> TraversedStates;
    vector<State*> next_states;next_states.reserve(10);
    next_states.push_back(state);
    while(next_states.size()){
        State* nstate=next_states.back();
        next_states.pop_back();
        TraversedStates.insert(nstate);
        if(nstate->isFinal())set_is_final=true;
        vector<Transition>& transitions=nstate->getTransitions();
        vector<Transition>::iterator it=transitions.begin();
        for(;it!=transitions.end();++it){
            if(it->isEpsilon()&&!TraversedStates.count(it->to)){
                next_states.push_back(it->to);
            }else{
                transmitions[it->signal].insert(it->to);
            }
        }
    }
    state->ClearTransitions(all_states);
    state->addTransitions(transmitions);
    return set_is_final;
}

void Automata::GetSetOfStetesWhithEpsilonOut(set<State*>& StatesWhithEpsilonLinkOut){
    set<State*>::iterator it=all_states.begin();
    for(;it!=all_states.end();++it){
        if((*it)->isEpsilonFrom())
            StatesWhithEpsilonLinkOut.insert(*it);
    }
}
void Automata::EpsilonNFAToNFA(){
    //1)найти все вершины из которых исходят эпсилоны
    set<State*> StatesWhithEpsilonLinkOut;
    GetSetOfStetesWhithEpsilonOut(StatesWhithEpsilonLinkOut);
    //2)для каждого состояния с эпсилон выходом находим новое множество связей(без эпсилон связей)
    set<State*> new_finals;
    copy(finals.begin(), finals.end(),
        inserter( new_finals, new_finals.begin() ) );
    set<State*>::iterator sit=StatesWhithEpsilonLinkOut.begin();
    for(;sit!=StatesWhithEpsilonLinkOut.end();++sit){
        if(GetNewTransmitiomForEpsilonOutState(*sit)){
            (*sit)->setFinal(true);
            new_finals.insert(*sit);
        }
    }
    set<State*> reachableStates=GetReachableStates();
    //3)удаляем все недостижимые состояния из автомата(состояния на которые указывали только эпсилон переходы)
    vector<State*> stateToDelete;
    stateToDelete.resize(all_states.size()-reachableStates.size());
    set_difference(all_states.begin(),all_states.end(),reachableStates.begin(),reachableStates.end(),stateToDelete.begin());
    all_states=reachableStates;
    for(vector<State*>::iterator it=stateToDelete.begin();it!=stateToDelete.end();++it){
        DeleteState(*it,false);
    }
    finals.clear();
    set_intersection(new_finals.begin(),new_finals.end(),reachableStates.begin(),reachableStates.end(),inserter(finals,finals.begin()));
}
void Automata::getNewStateTypeTransitions(TODFAStateType& state, TODFATarnsitions& transitions){
    TODFAStateType::iterator sit=state.begin();
    for(;sit!=state.end();++sit){
        vector<Transition>& simple_transitions=(*sit)->getTransitions();
        vector<Transition>::iterator tit=simple_transitions.begin();
        for(;tit!=simple_transitions.end();++tit){
            transitions[tit->signal].insert(tit->to);
        }
    }
}
void Automata::getNewPowerSetsStete(list<TODFAStateANDTransitions > &states){
    set<TODFAStateType> TraversedNewStates;
    TODFAStateType new_start;
    new_start.insert(start);
    queue<TODFAStateType > next_states;
    next_states.push(new_start);
    while(next_states.size()){
        TODFAStateType& next_state=next_states.front();
        TODFATarnsitions state_transitions;
        //1)заполняем связи состояния
        getNewStateTypeTransitions(next_state, state_transitions);
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
State* Automata::createNewStaesForDFA(list<TODFAStateANDTransitions >& new_set_states, set<State*>& new_states, list<State*>& new_finals, map<set<State*>,  State* >& newToOldStatesMap){
    State* new_start=0;
    list<TODFAStateANDTransitions >::iterator it=new_set_states.begin();
    for(;it!=new_set_states.end();++it){
        State* new_state=CreateState(false,&new_states);
        //старый старт
        if(it->first.size()==1 && it->first.count(start)){
            new_start=new_state;
        }
        for(set<State*>::iterator sit=it->first.begin();sit!=it->first.end();++sit){
            if((*sit)->isFinal()){
                new_state->setFinal(true);
                new_finals.push_back(new_state);
            }
        }
        newToOldStatesMap.insert(pair<set<State*>&,State*>(it->first,new_state));
    }
    return new_start;
}
//new_set_states
void Automata::createTransitionsForDFA(list<TODFAStateANDTransitions >& new_states, map<set<State*>,  State* >& newToOldStatesMap){
    list<TODFAStateANDTransitions >::iterator it=new_states.begin();
    for(;it!=new_states.end();++it){
        State* state = newToOldStatesMap[it->first];
        TODFATarnsitions& transitions=it->second;
        TODFATarnsitions::iterator tit=transitions.begin();
        for(;tit!=transitions.end();++tit){
            state->addTransition(newToOldStatesMap[tit->second],tit->first);
        }
    }
}
bool Automata::IsDependentState(State* fs,State* ss,DistinguashbleStatesCollection&  DependentStates){
    typedef vector<Transition>::iterator ItType;
    //ФИНАЛЬНЫЕ И НЕ ФИНАЛЬНЫЕ ТУТ НЕ ХРОНЯТСЯ ОНИ И ТАК ПОНЯТНО ЧТО ЗАВИСИМЫЕ
    if((fs->isFinal() && !ss->isFinal())||(!fs->isFinal() && ss->isFinal()))return false;
    vector<Transition>& fs_trans=fs->getTransitions();
    vector<Transition>& ss_trans=ss->getTransitions();
    for(ItType fstit=fs_trans.begin();fstit!=fs_trans.end();++fstit){
        bool not_find_equ_signal=true;
        for(ItType sstit=ss_trans.begin();sstit!=ss_trans.end();++sstit){
            if(sstit->signal==fstit->signal){//проверяем на зависимость
                not_find_equ_signal=false;
                State* ssTo=sstit->to;
                State* fsTo=fstit->to;
                if(fsTo==ssTo)continue;
                if((fsTo->isFinal() && !ssTo->isFinal())||(!fsTo->isFinal() && ssTo->isFinal()))
                    return true;
                if(DependentStates.exist(fsTo,ssTo)){
                    return true;
                }

            }
        }
        if(not_find_equ_signal)return true;//!если не найден экв. сигнал то один ссылается на терминальный а второй нет(терминальный со всеми не зависим)
    }
    return false;
}
bool Automata::IsIndependent(State* fstate,State* sstate,DistinguashbleStatesCollection& dependentStates){
    if((fstate->isFinal() && !sstate->isFinal())||(!fstate->isFinal() && sstate->isFinal()))return false;
    if(dependentStates.exist(fstate,sstate))return false;
    return true;
}
void Automata::MergeStatesOutputSignals(State* fstate,State* sstate){
    map<char, State*> new_signals;
    typedef vector<Transition>::iterator ItType;
    vector<Transition>& s_transitionsget=sstate->getTransitions();
    vector<Transition>& f_transitions=fstate->getTransitions();
    for(ItType it=s_transitionsget.begin();it!=s_transitionsget.end();++it){
        if(it->to==sstate){
            new_signals.insert(pair<char, State*>(it->signal,fstate));
        }else{
            new_signals.insert(pair<char, State*>(it->signal,it->to));
        }
    }
    for(ItType it=f_transitions.begin();it!=f_transitions.end();++it){
        new_signals.insert(pair<char, State*>(it->signal,it->to));
    }
    fstate->ClearTransitions(all_states);
    sstate->ClearTransitions(all_states);
    for(map<char, State*>::iterator it=new_signals.begin();it!=new_signals.end();++it){
        fstate->addTransition(it->second,it->first);
    }
}
void Automata::MergeStatesInputSignals(State* fstate,State* sstate){
    map<State*, set<char> > link_to_fstate;
    set<State*>& f_toThisStateLink=fstate->getToThisStateLink();
    for(set<State*>::iterator it=f_toThisStateLink.begin();it!=f_toThisStateLink.end();++it){
        vector<Transition>& transitions=(*it)->getTransitions();
        for(vector<Transition>::iterator tit=transitions.begin();tit!=transitions.end();++tit){
            if(tit->to==fstate){
                link_to_fstate[*it].insert(tit->signal);
            }
        }

    }
    //переставляем все связи указывающие на младщенького
    set<State*>& s_toThisStateLink=sstate->getToThisStateLink();
    for(set<State*>::iterator it=s_toThisStateLink.begin();it!=s_toThisStateLink.end();++it){
        vector<Transition>& transitions=(*it)->getTransitions();

        for(vector<Transition>::iterator tit=transitions.begin();tit!=transitions.end();++tit){
            if(tit->to==sstate){
                if(link_to_fstate.count(*it)&&link_to_fstate[*it].count(tit->signal)){
                    tit=transitions.erase(tit);
                    --tit;
                }else{
                    tit->setTo(fstate);
                    fstate->getToThisStateLink().insert(*it);
                }
            }
        }

    }
}

void Automata::MergeStates(State* fstate,State* sstate){
    MergeStatesOutputSignals(fstate,sstate);
    MergeStatesInputSignals(fstate,sstate);
    if(sstate==start)start=fstate;
}

void Automata::GlueStates(DistinguashbleStatesCollection& dependentStates){
    set<State*> mergedStates;
        set<State*>::iterator fstate=all_states.begin();
        set<State*>::iterator fend=--all_states.end();
        for(;fstate!=fend;++fstate){
            set<State*>::iterator sstate=fstate;++sstate;
            for(;sstate!=all_states.end();++sstate){
                State* fstate_p=(*fstate>*sstate)?*fstate:*sstate; //всегда оставляем старший
                State* sstate_p=(*fstate>*sstate)?*sstate:*fstate;
                if(!mergedStates.count(sstate_p) && !mergedStates.count(fstate_p)){
                    if(IsIndependent(fstate_p,sstate_p,dependentStates)){
                        MergeStates(fstate_p,sstate_p);
                        mergedStates.insert(sstate_p);
                    }
                }

            }
        }
        for(set<State*>::iterator it=mergedStates.begin();it!=mergedStates.end();++it){
            DeleteState(*it);
        }

    //удалить все смёрдженые состояния
}
void Automata::getDependedStates(DistinguashbleStatesCollection& dependentStates){
    bool changes;
    do{
        changes=false;
        set<State*>::iterator fit=all_states.begin();
        set<State*>::iterator efit=--all_states.end();
        do{
          set<State*>::iterator sit=fit;++sit;
            for(;sit!=all_states.end();++sit){
              if(!dependentStates.exist(*fit,*sit)
                      && IsDependentState(*fit,*sit,dependentStates)){
                dependentStates.insert(*fit,*sit);
                changes=true;
            }
          }
        }while(++fit!=efit);
    }while(changes);
}

void Automata::OptimizeDFA(){
    if(!dfa)return;
    DistinguashbleStatesCollection dependentStates;//больший указатель первый
    //находим список зависимых состояний
    getDependedStates(dependentStates);
    //склеиваем состояния
    GlueStates(dependentStates);
    min=true;
}

void Automata::ToDFA(){
    EpsilonNFAToNFA();
    //посути мы тут создаём автомат заново проходя все его вершины из старта
    list<TODFAStateANDTransitions > new_set_states;
    //получаем список новых состояний, состоящих из состояний-множеств старых
    getNewPowerSetsStete(new_set_states);
    //получили обсолюто новый автомат.
    set<State*> new_states;
    list<State*> new_finals;
    //создаём новые состояния;
    map<set<State*>,  State* > newToOldStatesMap;
    start=createNewStaesForDFA(new_set_states,new_states,new_finals,newToOldStatesMap);
    //создаём связи для новых состояний
    createTransitionsForDFA(new_set_states,newToOldStatesMap);
    //чистим старый
    Clear();
    //заполняем новый
    all_states=new_states;
    finals=new_finals;
    dfa=true;
}
Automata::Automata(bool epsilon):min(true),dfa(true){//create epsilon/zero automata;
    if(epsilon){
        start=CreateState(true);
        finals.push_back(start);
    }else{
        start=CreateState();
        finals.push_back(CreateState(true));
    }
}
Automata::Automata(char Char):min(true),dfa(true){//creape char atomata;
    start=CreateState();
    State* finalState=CreateState(true);
    start->addTransition(finalState,Char);
    finals.push_back(finalState);
}
void Automata::Concatinate(char signal){
    State* FinalState=CreateState(true);
    list<State*>::iterator it=finals.begin();
    Transition tran(FinalState,signal);
    for(;it!=finals.end();++it){
        (*it)->setFinal(false);
        (*it)->addTransition(tran);
    }
    finals.clear();
    finals.push_back(FinalState);
}
void Automata::Concatinate(Automata& automata){
    if(!automata.start->getToThisStateLink().size()){
        vector<Transition>& transition = automata.start->getTransitions();
        for(list<State*>::iterator it=finals.begin();it!=finals.end();++it){
            vector<Transition>::iterator tit=transition.begin();
            for(;tit!=transition.end();++tit){
                (*it)->addTransition(*tit);
            }
            (*it)->setFinal(automata.start->isFinal());
        }
        automata.start->ClearTransitions(automata.all_states);
        automata.DeleteState(automata.start);
    }else{
        list<State*>::iterator it=finals.begin();
        Transition tran(automata.start,0,true);
        for(;it!=finals.end();++it){
            (*it)->setFinal(false);
            (*it)->addTransition(tran);
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
}
void Automata::Union(char signal){
    list<State*>::iterator it=finals.begin();
    for(;it!=finals.end();++it){
        start->addTransition(*it,signal);
    }
}
void Automata::Union(Automata& automata){
    if(!start->getToThisStateLink().size()&& !automata.start->getToThisStateLink().size()
            && automata.finals.size()==1 && automata.finals.front()->getTransitions().size()==0 &&
            finals.size()==1 && finals.front()->getTransitions().size()==0){
        vector<Transition>& transition = automata.start->getTransitions();
        vector<Transition>::iterator tit=transition.begin();
        for(;tit!=transition.end();++tit){
            start->addTransition(*tit);
        }
        start->setFinal(automata.start->isFinal()||start->isFinal());
        automata.start->ClearTransitions(automata.all_states);

        State* old_final=automata.finals.front();
        State* new_final=finals.front();
        set<State*>& fin_to_state=old_final->getToThisStateLink();
        for(set<State*>::iterator it=fin_to_state.begin();it!=fin_to_state.end();++it){
            vector<Transition>& transition=(*it)->getTransitions();
            vector<Transition>::iterator tit=transition.begin();
            for(;tit!=transition.end();++tit){
                if(tit->to==old_final){
                    tit->setTo(new_final);
                    new_final->getToThisStateLink().insert(*it);
                }
            }
        }
        automata.DeleteState(automata.finals.front());
        automata.DeleteState(automata.start);
        automata.start=NULL;
    }else{
        State* new_start=CreateState();
        new_start->addTransition(start,0,true);
        new_start->addTransition(automata.start,0,true);
        start=new_start;

        State* finalState=CreateState(true);
        list<State*>::iterator it=finals.begin();
        Transition tran(finalState,0,true);
        for(;it!=finals.end();++it){
            (*it)->setFinal(false);
            (*it)->addTransition(tran);
        }
        for(it=automata.finals.begin();it!=automata.finals.end();++it){
            (*it)->setFinal(false);
            (*it)->addTransition(tran);
        }
        finals.clear();
        finals.push_back(finalState);
    }
    automata.finals.clear();
    copy(automata.all_states.begin(), automata.all_states.end(),
        inserter( all_states, all_states.begin() ) );
    automata.all_states.clear();


    min=false;
    dfa=false;
}


void Automata::KneeStar(){//TODO immediately to DFA if posible
    State* startState=CreateState();
    State* finalState=CreateState(true);
    startState->addTransition(finalState,0,true);
    startState->addTransition(start,0,true);

    list<State*>::iterator it=finals.begin();
    Transition tranToF(finalState,0,true);
    Transition tranToS(start,0,true);
    for(;it!=finals.end();++it){
        (*it)->setFinal(false);
        (*it)->addTransition(tranToF);
        (*it)->addTransition(tranToS);
    }
    start=startState;
    finals.clear();
    finals.push_back(finalState);

    min=false;
    dfa=false;
}

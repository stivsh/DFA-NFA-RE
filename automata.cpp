#include "automata.h"
//TEMP
#include <iostream>
#include <string>
void Automata::Clear(){
    set<State*>::iterator it=all_states.begin();
    for(;it!=all_states.end();++it){
        DeleteState(*it,false);
    }
}

bool Automata::ToCCode(ostream& ost,std::string& libname){
    if(!dfa)return false;
    ost<<"#include \""<<libname<<".h\""<<endl;
    ost<<"extern \"C\" void FindFirst(char* start, char* last,char** first_pos, char** last_pos, bool lazy){"<<endl;
    ost<<"\t*first_pos=start-1;"<<endl;
    ost<<"\tchar* pos=start;"<<endl;
    ost<<"\t*last_pos=0;"<<endl;
    ost<<"\tgoto start_State;"<<endl;
    ost<<"\tget_new_start_or_return:"<<endl;
    ost<<"\tif(*last_pos)return; //match"<<endl;
    ost<<"\t++(*first_pos);"<<endl;
    ost<<"\tpos=*first_pos-1;"<<endl;
    ost<<"\tif(pos>last)return;"<<endl;
    ost<<"\tgoto start_State;"<<endl;
    //генерируем для всех состояний переходы
    ost<<endl<<endl;
    for(set<State*>::iterator it=all_states.begin();it!=all_states.end();++it){
        if(*it==start)
            ost<<"\tstart_State";
        else
            ost<<"\tL"<<((size_t)(*it));
        ost<<":"<<endl;
        if((*it)->isFinal()){
            ost<<"\t*last_pos=pos;"<<endl;
            ost<<"\tif(lazy)return;"<<endl;
        }
        ost<<"\tif(++pos>last)return;"<<endl;
        ost<<"\tswitch(*pos){"<<endl;
        //код для всех переходов
         vector<Transition>& transitions=(*it)->getTransitions();
         for(vector<Transition>::iterator tit=transitions.begin();tit!=transitions.end();++tit){
                 ost<<"\t\tcase "<<(int)tit->Signal()<<":goto ";
                 if(tit->To()==start){
                     ost<<"start_State;"<<endl;
                 }else{
                     ost<<"L"<<((size_t)(tit->To()))<<";"<<endl;
                 }
         }
        ost<<"\tdefault:goto get_new_start_or_return;}"<<endl;
        ost<<endl<<endl;
    }
    ost<<"}"<<endl;
    return true;
}
State* GetNextState(queue<State*>& next_states,set<State*>& TraversedStates){
    if(!next_states.size())return 0;
    State* nstate=next_states.front();
    next_states.pop();
    TraversedStates.insert(nstate);
    vector<Transition>& transitions=nstate->getTransitions();
    vector<Transition>::iterator it=transitions.begin();
    for(;it!=transitions.end();++it){
        if(!TraversedStates.count(it->to))
            next_states.push(it->to);
    }
    return nstate;
}
boost::shared_array<char> Automata::ToByteCode(int *len){
    if(!dfa){
        *len=0;
        return boost::shared_array<char>(0);}
    /*FORMAT: |1b isFinal|2b ushort size=sum(len sig+index)|char sig|ulong index of next state|......*/
    vector<char> blob;
    blob.reserve(all_states.size()*(3+sizeof(unsigned int)+sizeof(char)*4));
    //формируем автомат,начиная с старт обходим весь автомат, первый обход без адресов
    queue<State*> next_states;set<State*> TraversedStates;next_states.push(start);
    State* next_state;
    map<State*, size_t> stetes_position;
    map<State*, set<char*> > indexex_of_state_links;
    while((next_state=GetNextState(next_states,TraversedStates))!=0){
        //добаляем следующее состояние
        stetes_position[next_state]=blob.size();
        char final=(next_state->isFinal());
        blob.push_back(final);
        size_t len_index=blob.size();
        blob.resize(blob.size()+sizeof(unsigned short));
        unsigned short state_size=0;
        vector<Transition>& transitions=next_state->getTransitions();
        for(vector<Transition>::iterator tit=transitions.begin();
                tit!=transitions.end();++tit){
            blob.push_back(tit->Signal());
            indexex_of_state_links[tit->To()].insert(&(blob.back())+1);
            blob.resize(blob.size()+sizeof(unsigned int));
            state_size+=sizeof(unsigned int)+sizeof(char);
        }
        *((unsigned short*)(&(blob[len_index])))=state_size;
    }
    //проставляем индексы для всех ссылок;
    for(map<State*, size_t>::iterator it=stetes_position.begin();
                it!=stetes_position.end();++it){
        if(!indexex_of_state_links.count(it->first))continue;
        for(set<char*>::iterator sit=indexex_of_state_links[it->first].begin();
                    sit!=indexex_of_state_links[it->first].end();++sit){
            unsigned int* index_pointer=(unsigned int*)(*sit);
            *index_pointer=it->second;
        }
    }
    char* blob_flat_memory=new char[blob.size()];
    memcpy(blob_flat_memory,&(blob[0]),blob.size());
    *len=blob.size();
    return boost::shared_array<char>(blob_flat_memory);
}
inline long long GetNextPosition(char* block_pos, char* pos){
    unsigned short signals_size=*((unsigned short*)block_pos);
    block_pos+=sizeof(unsigned short);
    long long position=-1;
    unsigned short treversed_signals_size=0;
    while(treversed_signals_size<signals_size){
        unsigned long long block=*((unsigned long long*)block_pos);
        char singal1=(char)(block&0x00000000000000ff);
        char signal2=(char)((block>>8*5)&0x00000000000000ff);
        if(singal1==(*pos)){
            position=(unsigned int)((block>>8)&0x00000000ffffffff);
            break;
        }
        treversed_signals_size+=5;
        if(treversed_signals_size>=signals_size)break;
        if(signal2==(*pos)){
            position=*((unsigned int*)(block_pos+6));
            break;
        }
        treversed_signals_size+=sizeof(unsigned int)+sizeof(char);
        block_pos+=(sizeof(unsigned int)+sizeof(char))*2;
    }
    return position;
}
bool Automata::FindFirstByByteCode(char* blob,char* start, char* last,char** first_pos, char** last_pos, bool lazy){
    char*pos=start;
    *first_pos=start;
    *last_pos=0;
    char* block_pos=blob;

    while(true){
        char is_final;
        is_final=*(block_pos++);
        if(is_final){
            *last_pos=pos;
            if(lazy)break;
        }

        long long position=GetNextPosition(block_pos,pos);
        if(position==-1){//нет переходя по сигналу
            if(*last_pos)break;
            ++(*first_pos);
            pos=(*first_pos);
            block_pos=blob;
            if(pos>=last)break;
        }else{//найден переход
            ++pos;
            if(pos>=last)break;
            block_pos=blob+position;
        }
    }
    return (*last_pos);


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
    //3)удаляем все недостижимые состояния из автомата(состояния на которые указывали только эпсилон переходы) TODO test
    while(true){
        bool there_is_no_unachievable_states=true;
        set<State*>::iterator it= all_states.begin();
        while(it!=all_states.end()){
            if((*it)!=start && !(*it)->getToThisStateLink().size()){
                (*it)->ClearTransitions(all_states);
                DeleteState(*it,false);
                there_is_no_unachievable_states=false;
                all_states.erase(it++);
            }else{
                ++it;
            }
        }
        if(there_is_no_unachievable_states)break;
    }
/*
    set<State*> reachableStates=GetReachableStates();

    vector<State*> stateToDelete;
    stateToDelete.resize(all_states.size()-reachableStates.size());
    set_difference(all_states.begin(),all_states.end(),reachableStates.begin(),reachableStates.end(),stateToDelete.begin());
    all_states=reachableStates;
    for(vector<State*>::iterator it=stateToDelete.begin();it!=stateToDelete.end();++it){
        DeleteState(*it,false);
    }

*/
    finals.clear();
    set_intersection(new_finals.begin(),new_finals.end(),all_states.begin(),all_states.end(),inserter(finals,finals.begin()));
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
    set<pair<State*, char> >& f_toThisStateLink=fstate->getToThisStateLink();
    for(set<pair<State*, char> >::iterator it=f_toThisStateLink.begin();it!=f_toThisStateLink.end();++it){
        vector<Transition>& transitions=(it->first)->getTransitions();
        for(vector<Transition>::iterator tit=transitions.begin();tit!=transitions.end();++tit){
            if(tit->to==fstate){
                link_to_fstate[it->first].insert(tit->Signal());
            }
        }

    }
    //переставляем все связи указывающие на младщенького
    set<pair<State*, char> >& s_toThisStateLink=sstate->getToThisStateLink();
    for(set<pair<State*, char> >::iterator it=s_toThisStateLink.begin();it!=s_toThisStateLink.end();++it){
        vector<Transition>& transitions=it->first->getTransitions();

        for(vector<Transition>::iterator tit=transitions.begin();tit!=transitions.end();++tit){
            if(tit->to==sstate){
                if(link_to_fstate.count(it->first)&&link_to_fstate[it->first].count(tit->signal)){
                    tit=transitions.erase(tit);
                    --tit;
                }else{
                    tit->setTo(fstate);
                    fstate->getToThisStateLink().insert(make_pair(it->first,tit->Signal()));
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
        set<pair<State*, char> >& fin_to_state=old_final->getToThisStateLink();
        for(set<pair<State*, char> >::iterator it=fin_to_state.begin();it!=fin_to_state.end();++it){
            vector<Transition>& transition=it->first->getTransitions();
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

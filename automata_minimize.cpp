#include "automata.h"

bool Automata::is_minimized(){
    return min;
}
bool is_dependent_state(State* fs,State* ss,DistinguashbleStatesCollection&  DependentStates){
    //ФИНАЛЬНЫЕ И НЕ ФИНАЛЬНЫЕ ТУТ НЕ ХРОНЯТСЯ ОНИ И ТАК ПОНЯТНО ЧТО ЗАВИСИМЫЕ
    if((fs->is_final() && !ss->is_final())||(!fs->is_final() && ss->is_final()))return false;
    TransmissionCollection& fs_trans=fs->get_transitions();
    TransmissionCollection& ss_trans=ss->get_transitions();
    for(transition_iter fstit=fs_trans.begin();fstit!=fs_trans.end();++fstit){
        bool not_find_equ_signal=true;
        for(transition_iter sstit=ss_trans.begin();sstit!=ss_trans.end();++sstit){
            if(sstit->signal()==fstit->signal()){//проверяем на зависимость
                not_find_equ_signal=false;
                State* ssTo=sstit->to();
                State* fsTo=fstit->to();
                if(fsTo==ssTo)continue;
                if((fsTo->is_final() && !ssTo->is_final())||(!fsTo->is_final() && ssTo->is_final()))
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
bool is_independent(State* fstate,State* sstate,DistinguashbleStatesCollection& dependentStates){
    if((fstate->is_final() && !sstate->is_final())||(!fstate->is_final() && sstate->is_final()))return false;
    if(dependentStates.exist(fstate,sstate))return false;
    return true;
}
void Automata::merge_states_output_signals(State* fstate,State* sstate){
    map<char, State*> new_signals;
    TransmissionCollection& s_transitionsget=sstate->get_transitions();
    TransmissionCollection& f_transitions=fstate->get_transitions();
    for(transition_iter it=s_transitionsget.begin();it!=s_transitionsget.end();++it){
        if(it->to()==sstate){
            new_signals.insert(pair<char, State*>(it->signal(),fstate));
        }else{
            new_signals.insert(pair<char, State*>(it->signal(),it->to()));
        }
    }
    for(transition_iter it=f_transitions.begin();it!=f_transitions.end();++it){
        new_signals.insert(pair<char, State*>(it->signal(),it->to()));
    }
    fstate->get_transitions().clear_transitions(&all_states);
    fstate->get_transitions().clear_transitions(&all_states);
    for(map<char, State*>::iterator it=new_signals.begin();it!=new_signals.end();++it){
        fstate->get_transitions().add_transition(it->second,it->first);
    }
}
void merge_states_input_signals(State* fstate,State* sstate){
    //переставляем все связи указывающие на младщенького
    IncomingTransmitions& s_incoming=sstate->incoming();
    for(intransition_iter it=s_incoming.begin();it!=s_incoming.end();++it){
        State* from=(it->first==sstate)?fstate:it->first;
        set<char>& signal_s=it->second;
        for(set<char>::iterator isig=signal_s.begin();isig!=signal_s.end();++isig){
            if(!from->get_transitions().exist(fstate,*isig)){
                from->get_transitions().add_transition(fstate,*isig);
            }
        }
    }
    sstate->incoming().back_delete_all_transition_to();
}

void Automata::merge_states(State* fstate,State* sstate){
    merge_states_output_signals(fstate,sstate);
    merge_states_input_signals(fstate,sstate);
    if(sstate==start)start=fstate;
}
set<State*>  Automata::glue_states(DistinguashbleStatesCollection& dependentStates){
    set<State*> mergedStates;
    set<State*>::iterator fstate=all_states.begin();
        set<State*>::iterator fend=--all_states.end();
        for(;fstate!=fend;++fstate){
            set<State*>::iterator sstate=fstate;++sstate;
            for(;sstate!=all_states.end();++sstate){
                State* fstate_p=(*fstate>*sstate)?*fstate:*sstate; //всегда оставляем старший
                State* sstate_p=(*fstate>*sstate)?*sstate:*fstate;
                if(!mergedStates.count(sstate_p) && !mergedStates.count(fstate_p)){
                    if(is_independent(fstate_p,sstate_p,dependentStates)){
                        merge_states(fstate_p,sstate_p);
                        mergedStates.insert(sstate_p);
                    }
                }

            }
        }
        return mergedStates;
}
void get_depended_states(DistinguashbleStatesCollection& dependentStates, set<State*>& states){
    bool changes;
    do{
        changes=false;
        set<State*>::iterator fit=states.begin();
        set<State*>::iterator efit=--states.end();
        do{
          set<State*>::iterator sit=fit;++sit;
            for(;sit!=states.end();++sit){
              if(!dependentStates.exist(*fit,*sit)
                      && is_dependent_state(*fit,*sit,dependentStates)){
                dependentStates.insert(*fit,*sit);
                changes=true;
            }
          }
        }while(++fit!=efit);
    }while(changes);
}
void Automata::optimize_dfa(){
    if(!dfa)return;
    DistinguashbleStatesCollection dependentStates;//больший указатель первый
    //находим список зависимых состояний
    get_depended_states(dependentStates, all_states);
    //склеиваем состояния
     set<State*> mergedStates=glue_states(dependentStates);
    //удалить все смёрдженые состояния
    for(set<State*>::iterator it=mergedStates.begin();it!=mergedStates.end();++it){
        delete_state(*it);
    }
    min=true;
}
void Automata::minimize(){
    if(!dfa)return;
    optimize_dfa();
    min=true;
}
inline void insert_new_vector(vector<vector<State*> > &mergebal_states,State* state){
    mergebal_states.push_back(vector<State*>());
    mergebal_states.back().push_back(state);
}
inline void sets_for_merge(vector<vector<State*> >&mergebal_states,vector<State*>& states){
    for(size_t j=0;j!=states.size();++j){
        if(!mergebal_states.size()){
            insert_new_vector(mergebal_states,states[j]);
        }else{
            bool find;
            for(size_t q=0;q!=mergebal_states.size();++q){
                if(mergebal_states[q].front()->get_transitions()
                        ==states[j]->get_transitions()){
                    mergebal_states[q].push_back(states[j]);
                    find=true;
                    break;
                }
            }
            if(!find){
                insert_new_vector(mergebal_states,states[j]);
            }
        }
    }
}
State* Automata::suffix_minimize_merge_states(vector<State*>& mergebal_states){
    if(mergebal_states.size()<2)return mergebal_states[0];
    State* fstate=mergebal_states.at(0);
    for(size_t i=1;i!=mergebal_states.size();++i){
        if(!fstate->is_final()&&mergebal_states[i]->is_final()){
            fstate->set_final(true);
            finals.push_back(fstate);
            finals.remove(mergebal_states[i]);
        }else if(mergebal_states[i]->is_final()){
            finals.remove(mergebal_states[i]);
        }
        if(start==mergebal_states[i])
            start=fstate;
        merge_states_input_signals(fstate,mergebal_states[i]);
        mergebal_states[i]->get_transitions().clear_transitions(&all_states);
        delete_state(mergebal_states[i]);
    }
    return fstate;
}

void Automata::suffix_minimize(){
    if(!dfa)return;
    vector<vector<State*> >sets_to_test_for_merge;
    vector<vector<State*> >new_sets_to_test_for_merge;

    vector<State*> tmp_v;
    copy(finals.begin(),finals.end(),inserter(tmp_v,tmp_v.begin()));
    sets_to_test_for_merge.push_back(tmp_v);
    while(sets_to_test_for_merge.size()){
        new_sets_to_test_for_merge.clear();
        for(size_t i=0;i!=sets_to_test_for_merge.size();++i){
           vector<vector<State*> > mergebal_states;
           sets_for_merge(mergebal_states,sets_to_test_for_merge[i]);
           for(size_t j=0;j!=mergebal_states.size();++j){
               State* new_state;
               if(mergebal_states[j].size()>=2){
                    new_state=suffix_minimize_merge_states(mergebal_states[j]);
               }else{
                   continue;
                   new_state=mergebal_states[j].at(0);
               }
               vector<State*> test_for_m;
               for(intransition_iter it=new_state->incoming().begin();it!=new_state->incoming().end();++it)
                    test_for_m.push_back(it->first);
               if(test_for_m.size())
                    new_sets_to_test_for_merge.push_back(test_for_m);
           }
        }
        sets_to_test_for_merge=new_sets_to_test_for_merge;
    }


}

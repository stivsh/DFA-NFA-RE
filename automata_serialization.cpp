#include "automata.h"
ostream& Automata::operator>>(ostream& os){
    //wtite CRC
    os.write(reinterpret_cast<char*>(&min),sizeof(bool));
    os.write(reinterpret_cast<char*>(&dfa),sizeof(bool));
    os.write(reinterpret_cast<char*>(&start),sizeof(State*));
    size_t states_count=all_states.size();
    os.write(reinterpret_cast<char*>(&states_count),sizeof(size_t));
    for(set<State*>::iterator it=all_states.begin();it!=all_states.end();++it){
        os.write(reinterpret_cast<const char*>(&*it),sizeof(State*));
        **it>>os;
    }
    for(set<State*>::iterator it=all_states.begin();it!=all_states.end();++it){
        os.write(reinterpret_cast<const char*>(&*it),sizeof(State*));
        (*it)->get_transitions()>>os;
    }
    return os;
}
istream& Automata::operator<<(istream& is){
    //check CRC
    clear();
    State* start_old;
    size_t states_count;
    is.read(reinterpret_cast<char*>(&min),sizeof(bool));
    is.read(reinterpret_cast<char*>(&dfa),sizeof(bool));
    is.read(reinterpret_cast<char*>(&start_old),sizeof(State*));
    is.read(reinterpret_cast<char*>(&states_count),sizeof(size_t));
    map<State*, State*> old_states_to_new;
    for(size_t i=0;i!=states_count;++i){
        State* stay_old;
        is.read(reinterpret_cast<char*>(&stay_old),sizeof(State*));
        State* state=create_state();
        (*state)<<is;
        if(state->is_final())
            finals.push_back(state);
        if(stay_old==start_old)
            start=state;
        old_states_to_new.insert(make_pair(stay_old,state));
    }
    for(size_t i=0;i!=states_count;++i){
        State* stay_old;
        is.read(reinterpret_cast<char*>(&stay_old),sizeof(State*));
        TransmissionCollection& trans=old_states_to_new[stay_old]->get_transitions();
        trans.load_from_stream(is,old_states_to_new);
    }
    return is;
}

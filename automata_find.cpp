#include "automata.h"
using namespace std;

bool Automata::exact_match(char* text,size_t len){
    State* state=start;
    for(size_t i=0;i!=len;++i){
        state=state->get_transitions().get_next_state(text[i]);
        if(!state)return false;
    }
    return state->is_final();
}

pair<char*, char*> Automata::index_in(char* text, size_t len,bool lazy){
    if(len<1)return pair<char*, char*>(0,0);
    char* first=0;
    char* last=0;
    for(char* next_start_pos=text;next_start_pos<text+len;++next_start_pos){
        State* state=start;
        if(state->is_final()){
            first=next_start_pos;
            last=next_start_pos;
            if(lazy)
                break;
        }
        for(char* pos=next_start_pos;pos<text+len;++pos){
            state=state->get_transitions().get_next_state(*pos);
            if(!state)break;
            if(state->is_final()){
                first=next_start_pos;
                last=pos+1;
                if(lazy)
                    break;
            }
        }
        if(last)
            break;
    }
#ifdef DEBUG
    if(last&&(last<=first||first<text)throw "error in index_in";
#endif
    return pair<char*, char*>(first,last);
}

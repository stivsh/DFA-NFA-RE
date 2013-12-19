#ifndef PATTERNCHECKER_H
#define PATTERNCHECKER_H
#include <state.h>
class PatternChecker{
    Automata& atomata;
    State* state;
    public:
    PatternChecker(Automata& _atomata):atomata(_atomata){}
    bool ExactMatch(char* first, int len){
        State* state=atomata.GetStart();
        char* pos=first;
        while(state && pos-first<len){
            state=atomata.Step(state,*(pos++));
        }
        if(state && state->isFinal())
            return true;
        else
            return false;
    }
    pair<char*, char*> IndexIn(char* first, size_t len,bool lazy=true){//lazy-самую первую
        if(len<1)return pair<char*, char*>(NULL,NULL);
        for(char* next_start_pos=first;next_start_pos<first+len;++next_start_pos){
            char* last_end=NULL;
            State* state=atomata.GetStart();
            if(state->isFinal()){
                last_end=first;
                if(lazy){
                    return pair<char*, char*>(next_start_pos, last_end);
                }
            }
            for(char* pos=next_start_pos;pos<first+len;++pos){
                state=atomata.Step(state,*pos);
                if(!state)break;
                if(state->isFinal()){
                    last_end=pos;
                    if(lazy){
                        return pair<char*, char*>(next_start_pos, last_end);
                    }
                }

            }
            if(last_end)
                return pair<char*, char*>(next_start_pos, last_end);
        }
        return pair<char*, char*>(NULL,NULL);
    }
};
#endif // PATTERNCHECKER_H

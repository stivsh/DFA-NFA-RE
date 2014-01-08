#ifndef PATTERNCHECKER_H
#define PATTERNCHECKER_H
#include <state.h>
class PatternChecker{
    Automata& atomata;
    State* state;
    public:
    PatternChecker(Automata& _atomata):atomata(_atomata){}
    bool ExactMatch(char* first, int len){

    }
    pair<char*, char*> IndexIn(char* first, size_t len,bool lazy=true){//lazy-самую первую


    }
};
#endif // PATTERNCHECKER_H

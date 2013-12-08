#ifndef TRANSITION_H
#define TRANSITION_H
class State;
class Transition{
public:
    bool epsilon;
    State* to;
    char signal;
    Transition(State* _to,char _signal=0, bool _epsilon=false):
    epsilon(_epsilon),
    to(_to),
    signal(_signal){}
    bool isEpsilon(){return epsilon;}
    void setTo(State* state){
        to=state;
    }
};

#endif // TRANSITION_H

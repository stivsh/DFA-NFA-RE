#ifndef INCOMINGTRANSMITIONS_H
#define INCOMINGTRANSMITIONS_H
#include <set>
#include "map"
class State;
typedef std::map<State*, std::set<char> >::iterator intransition_iter;
class IncomingTransmitions
{
    State* this_state;
    std::map<State*, std::set<char> > incoming_links;
public:
    IncomingTransmitions(State* this_state_);
    bool contain(State* state, char signal);
    std::size_t size() const;
    intransition_iter begin();
    intransition_iter end();
    void insert(State* state,char signal);
    void erase(State* state,char signal);
    void back_delete_transition_to(State* state,char signal);
    void back_delete_all_transition_to();
};

#endif // INCOMINGTRANSMITIONS_H

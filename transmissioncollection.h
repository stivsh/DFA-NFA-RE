#ifndef TRANSMISSIONCOLLECTION_H
#define TRANSMISSIONCOLLECTION_H

#include <vector>
#include <map>
#include <set>
#include <iostream>
class Transition;
class State;
typedef std::set<Transition>::const_iterator transition_iter;
class TransmissionCollection
{
    bool has_epsilon_;
    State* this_state;
    std::set<Transition> transitions;
public:
    TransmissionCollection(State* _this_state);
    std::size_t size();
    transition_iter begin()const;
    transition_iter end()const;
    bool exist(const Transition& t)const;
    bool exist(State* state, char signal,bool epsinon=false)const;
    void erase(State* state,char signal);
    bool has_epsilon();
    void add_transition(Transition transition);
    void add_transition(State* _to,char _signal=0, bool _epsilon=false);
    void clear_transitions(std::set<State*> *states=0);
    void add_transition(std::map<char, std::set<State*> > &transmitions);
    State * get_next_state(char signal);
    bool operator==(const TransmissionCollection& tc);
    void load_from_stream(std::istream& is,std::map<State*, State*>& new_state_to_old_state);
    std::ostream& operator>>(std::ostream& os);
};

#endif // TRANSMISSIONCOLLECTION_H

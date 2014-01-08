#ifndef AUTOMATA_H
#define AUTOMATA_H
#include <state.h>
#include <iostream>
#include <string>
#include <set>
#include <list>
#include <queue>
#include <map>
#include <algorithm>
#include <state.h>
#include <poolalloc.h>
#include <cstring>
#include "automata_types.h"
using namespace std;
class Automata{
    State* start;
    list<State*> finals;
    bool min;
    bool dfa;
    set<State*> all_states;
    void clear(bool delete_states=true);
    void delete_state(State* state, bool erase=true);
    void epsilon_nfa_to_nfa();
    //new_set_states
    void optimize_dfa();
    void delete_unachievable();
    State* suffix_minimize_merge_states(vector<State*>& mergebal_states);
    void merge_states_output_signals(State* fstate,State* sstate);
    void merge_states(State* fstate,State* sstate);
    set<State*>  glue_states(DistinguashbleStatesCollection& dependentStates);
public:
    State* create_state(bool final=false, set<State*>* state_set=0);
    Automata(Automata& a_obj);
    Automata& operator=(Automata& a_src);
    void clone_to(Automata& dist);
    bool to_c_code(ostream& ost,std::string& libname);
    State* step(State* state, char signal);
    ~Automata();

    Automata(bool epsilon=true);
    Automata(char Char);
    Automata& concatinate(char signal);
    Automata& concatinate(Automata& automata);
    Automata& operator+(char signal);//eco concat
    Automata& operator+(Automata& automata);
    Automata& Union(char signal);
    Automata& Union(Automata& automata);
    Automata& operator*(char signal);//eco Union
    Automata& operator*(Automata& automata);


    Automata& knee_star();
    void to_dfa();
    void minimize();
    bool is_minimized();
    void suffix_minimize();
    bool is_dfa();

    State* get_start();
    bool exact_match(char* text,size_t len);
    pair<char*, char*> index_in(char* text, size_t len,bool lazy=true);
    bool has_unachievable_states();
    bool has_dead_end_states();
    ostream& operator>>(ostream& os);
    istream& operator<<(istream& is);
};

#endif // AUTOMATA_H

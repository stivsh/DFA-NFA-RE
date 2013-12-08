#ifndef AUTOMATA_H
#define AUTOMATA_H
#include <state.h>
#include <set>
#include <list>
#include <queue>
#include <map>
#include <algorithm>
#include <state.h>
#include <poolalloc.h>
using namespace std;
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
class PreciseDistinguashbleStatesCollection{
    typedef boost::unordered::unordered_set<pair<size_t,size_t> > set_type;
    typedef boost::unordered::unordered_map<size_t, set_type > map_type;
    map_type Map;
public:
    bool exist(State* f, State* s){
        if(Map.count((size_t)f^(size_t)s)){
            if(f<s){
                State* t=s;
                s=f;
                f=t;
            }
            pair<size_t,size_t> pp((size_t)f,(size_t)s);
            if(Map[(size_t)f^(size_t)s].count(pp))return true;
        //Map[(size_t)f^(size_t)s].insert(pair<size_t,size_t>((size_t)f,(size_t)s));
        }
        return false;
    }
    void insert(State* f, State* s){
        if(f<s){
            State* t=s;
            s=f;
            f=t;
        }
        Map[(size_t)f^(size_t)s].insert(pair<size_t,size_t>((size_t)f,(size_t)s));
    }
};
class UnPreciseDistinguashbleStatesCollection{
    typedef boost::unordered::unordered_set<size_t> set_type;
    set_type Set;
public:
    bool exist(State* f, State* s){
        return Set.count((size_t)f^(size_t)s);
    }
    void insert(State* f, State* s){
        Set.insert((size_t)f^(size_t)s);
    }
};
typedef UnPreciseDistinguashbleStatesCollection DistinguashbleStatesCollection;
class Automata{
    State* start;
    list<State*> finals;
    bool min;
    bool dfa;
    set<State*> all_states;
    void Clear();
    typedef set<State*> TODFAStateType;
    typedef map<char, TODFAStateType > TODFATarnsitions;
    typedef pair< TODFAStateType, TODFATarnsitions > TODFAStateANDTransitions;

    State* CreateState(bool final=false, set<State*>* state_set=__null);
    void DeleteState(State* state, bool erase=true);
    set<State*> GetReachableStates();

    bool GetNewTransmitiomForEpsilonOutState(State* state);
    void GetSetOfStetesWhithEpsilonOut(set<State*>& StatesWhithEpsilonLinkOut);
    void EpsilonNFAToNFA();
    void getNewStateTypeTransitions(TODFAStateType& state, TODFATarnsitions& transitions);
    void getNewPowerSetsStete(list<TODFAStateANDTransitions > &states);
    State* createNewStaesForDFA(list<TODFAStateANDTransitions >& new_set_states, set<State*>& new_states, list<State*>& new_finals, map<set<State*>,  State* >& newToOldStatesMap);
    //new_set_states
    void createTransitionsForDFA(list<TODFAStateANDTransitions >& new_states, map<set<State*>,  State* >& newToOldStatesMap);
    bool IsDependentState(State* fs,State* ss,DistinguashbleStatesCollection&  DependentStates);
    bool IsIndependent(State* fstate,State* sstate,DistinguashbleStatesCollection& dependentStates);
    void MergeStatesOutputSignals(State* fstate,State* sstate);
    void MergeStatesInputSignals(State* fstate,State* sstate);
    void MergeStates(State* fstate,State* sstate);
    void GlueStates(DistinguashbleStatesCollection& dependentStates);
    void getDependedStates(DistinguashbleStatesCollection& dependentStates);
    void OptimizeDFA();
public:
    State* GetStart(){return start;}
    State* Step(State* state, char signal){
        vector<Transition>& transition=state->getTransitions();
        vector<Transition>::iterator eit=transition.end();
        for(vector<Transition>::iterator bit=transition.begin();bit!=eit;++bit){
            if(bit->signal==signal){
                return bit->to;
            }
        }
        return NULL;
    }
    ~Automata(){Clear();}

    Automata(bool epsilon);
    Automata(char Char);
    void Concatinate(char signal);
    void Concatinate(Automata& automata);
    void Union(char signal);
    void Union(Automata& automata);


    void KneeStar();

    void ToDFA();

    void Minimize(){
        //for dfa only
        min=true;
    }

    bool isMinimized(){
        return min;
    }

    bool isDFA(){
       return dfa;
    }
};

#endif // AUTOMATA_H

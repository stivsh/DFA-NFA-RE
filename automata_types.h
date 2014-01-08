#ifndef AUTOMATA_TYPES_H
#define AUTOMATA_TYPES_H
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/shared_array.hpp>
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

#endif // AUTOMATA_TYPES_H

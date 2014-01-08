#ifndef TRANSITION_H
#define TRANSITION_H
#include <iostream>
class State;
class Transition{
    bool epsilon;
    State* to_;
    char signal_;
public:
    Transition(State* _to,char _signal=0, bool _epsilon=false):
    epsilon(_epsilon),
    to_(_to),
    signal_(_signal){}
    bool is_epsilon()const{return epsilon;}
    char signal()const{
        return signal_;
    }

    State* to()const{
        return to_;
    }
    bool operator==(const Transition& t)const{
        return epsilon==t.epsilon&&to_==t.to_&&signal_==t.signal_;
    }
    bool operator!=(const Transition& t)const{
        return !operator==(t);
    }
    bool operator<(const Transition& t)const{
        if(t.epsilon && !epsilon){
            return true;
        }else if(!t.epsilon && epsilon){
            return false;
        }else if(t.signal_ > signal_ ){
            return true;
        }else if(t.signal_ < signal_ ){
            return false;
        }else if(t.to_>to_){
            return true;
        }else if(t.to_<to_){
            return false;
        }
        return false;
    }
    std::ostream& operator>>(std::ostream& os)const{
        os.write(reinterpret_cast<const char*>(&epsilon),sizeof(bool));
        os.write(reinterpret_cast<const char*>(&to_),sizeof(State*));
        os.write(static_cast<const char*>(&signal_),sizeof(char));
        return os;
    }

    std::istream& operator<<(std::istream& is){
        is.read(reinterpret_cast<char*>(&epsilon),sizeof(bool));
        is.read(reinterpret_cast<char*>(&to_),sizeof(State*));
        is.read(static_cast<char*>(&signal_),sizeof(char));
        return is;
    }
};

#endif // TRANSITION_H

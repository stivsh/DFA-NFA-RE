#include <iostream>
#include <string>
#include "automata.h"
bool Automata::to_c_code(ostream& ost,std::string& libname){
    if(!dfa)return false;
    ost<<"#include \""<<libname<<".h\" "<<endl;
    ost<<"extern \"C\" void find_first_lib(char* start, char* last,char** begin, char** end, bool lazy){"<<endl;
    ost<<"\t*begin=start;"<<endl;
    ost<<"\tchar* pos=start-1;"<<endl;
    ost<<"\t*end=0;"<<endl;
    ost<<"\tgoto start_state;"<<endl;
    ost<<"\tget_new_start_or_return:"<<endl;
    ost<<"\tif(*end!=0)return; //match"<<endl;
    ost<<"\t++(*begin);"<<endl;
    ost<<"\tpos=*begin-1;"<<endl;
    ost<<"\tif(pos>last){*begin=0;*end=0;return;};"<<endl;
    ost<<"\tgoto start_state;"<<endl;
    //генерируем для всех состояний переходы
    ost<<endl<<endl;
    for(set<State*>::iterator it=all_states.begin();it!=all_states.end();++it){
        if(*it==start)
            ost<<"\tstart_state";
        else
            ost<<"\tL"<<((size_t)(*it));
        ost<<":"<<endl;
        if((*it)->is_final()){
            ost<<"\t*end=pos+1;"<<endl;
            ost<<"\tif(lazy)return;"<<endl;
        }
        ost<<"\tif(++pos>last)goto get_new_start_or_return;"<<endl;
        ost<<"\tswitch(*pos){"<<endl;
        //код для всех переходов
         TransmissionCollection& transitions=(*it)->get_transitions();
         for(transition_iter tit=transitions.begin();tit!=transitions.end();++tit){
                 ost<<"\t\tcase "<<(int)tit->signal()<<":goto ";
                 if(tit->to()==start){
                     ost<<"start_state;"<<endl;
                 }else{
                     ost<<"L"<<((size_t)(tit->to()))<<";"<<endl;
                 }
         }
        ost<<"\tdefault:goto get_new_start_or_return;}"<<endl;
        ost<<endl<<endl;
    }
    ost<<"}"<<endl;
    return true;
}

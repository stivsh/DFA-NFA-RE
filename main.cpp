#define DEBUG 1
#ifdef UNIX
    #define PLUS_HOME( x ) "/home/stiv/"#x
#else
    #define PLUS_HOME( x ) "D:\\"#x
#endif
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QTextCodec>
#include <list>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <QByteArray>
#include <cstring>
#include <automata.h>
#include <common.h>
#include "find_lib.h"
#include "reparser.h"
const int DekimetersCount=300;
const int BigTextLen=1024*1024*20;
int union_time;
int dfa_time;
int epsilon_time;
using namespace std;
vector<QTextCodec*> getTextCodecs(){
    vector<QTextCodec*> text_codecs;
    text_codecs.push_back(QTextCodec::codecForName("Windows-1251"));
    text_codecs.push_back(QTextCodec::codecForName("UTF-8"));
    return text_codecs;
}

int readWordList(vector<QString>& words){
    QFile file(PLUS_HOME(badwords.txt));
    if (!file.open(QIODevice::ReadOnly |QIODevice::Text))return -1;
    QTextStream in(&file);
    in.setCodec("Windows-1251");//TODO get codec name as parameter
    while(!in.atEnd()){
        QString line = in.readLine();
        bool repeats=false;
        for(size_t i=0;i!=words.size();++i){
            if(words[i].indexOf(line)!=-1||line.indexOf(words[i])!=-1){
                repeats=true;
                break;
            }
        }
        if(!repeats && line.size()<DekimetersCount)
            words.push_back(line);

    }
    file.close();
    std::random_shuffle(words.begin(),words.end());
    return 0;
}
vector<QString> getWordList(vector<QString>& words,size_t len){
    vector<QString> out;
    out.reserve(words.size());
    for(size_t i=0;i!=len && i!=words.size();++i)
        out.push_back(words[i]);
    return out;
}
char* createText(size_t len, vector<QString>& words, list<char*>* bad_words_pos=0){
    vector<QTextCodec*> text_codecs=getTextCodecs();
    char* bigText= new char[len];
    memset(bigText,93,len-1);len-=1;
     //создаём длинную, длинную сироку содержащую эти слова, сол случайно изменёнными регистрами символов
    char *pos=bigText;
    while(true){
        if(rand()%100>20){//вставляем случайную 300 символьную последовательность
            for(int i=0;i!=DekimetersCount;++i){
                if(pos+i>=bigText+len-1)break;
                char c=93;
                *(pos+i)=c;
            }
            pos+=DekimetersCount;
        }else{//вставляем случайное слово
            int index=rand()%words.size();
            QTextCodec* codec=text_codecs[rand()%text_codecs.size()];
            QByteArray ba=codec->fromUnicode(words[index]);
            if(pos+ba.size()>=bigText+len-1)break;
            memcpy(pos,ba.data(),ba.size());
            if(bad_words_pos)bad_words_pos->push_back(pos);
            pos+=ba.size();
        }
    }
    return bigText;
}
void _chreate_char_atomata(char* c,size_t len,Automata& charatom){
    for(size_t i=0;i!=len;++i){
        charatom.concatinate(c[i]);
    }
}
void createAtoumata(vector<QString>& word_list,Automata& aut){//TODO вовсех кодировках со всеми регистрами
    vector<QTextCodec*> text_codecs=getTextCodecs();
    clock_t t=clock();
    for(size_t str_ind=0;str_ind!=word_list.size();++str_ind){
        QString str=word_list[str_ind];
        Automata wordAut(true);
        for(int ch_ind=0; ch_ind!=str.size();++ch_ind){
            QString tmp_str(str.at(ch_ind));
            Automata tmp_automata(false);
            set<QByteArray> ba_set;
            for(size_t cod_ind=0; cod_ind!=text_codecs.size();++cod_ind){
                QByteArray ba=text_codecs[cod_ind]->fromUnicode(tmp_str.toUpper());
                if(!ba_set.count(ba)){
                    Automata  atom(true);
                    _chreate_char_atomata(ba.data(), ba.size(),atom);
                    tmp_automata.Union(atom);
                    ba_set.insert(ba);
                }
                ba=text_codecs[cod_ind]->fromUnicode(tmp_str.toLower());
                if(!ba_set.count(ba)){
                    Automata  atom(true);
                    _chreate_char_atomata(ba.data(), ba.size(),atom);
                    tmp_automata.Union(atom);
                    ba_set.insert(ba);
                }
            }
            wordAut.concatinate(tmp_automata);
        }
        aut.Union(wordAut);
    }

    cout<<"has_dead_end_states b_dfa:"<<aut.has_dead_end_states()<<endl;
    cout<<"has_unachievable_states b_dfa:"<<aut.has_unachievable_states()<<endl;
    union_time=timeMesure(t);
    t=clock();
    aut.to_dfa();
    dfa_time=timeMesure(t);
    cout<<"has_dead_end_states a_dfa:"<<aut.has_dead_end_states()<<endl;
    cout<<"has_unachievable_states a_dfa:"<<aut.has_unachievable_states()<<endl;
    aut.suffix_minimize();
    cout<<"has_dead_end_states a_sm:"<<aut.has_dead_end_states()<<endl;
    cout<<"has_unachievable_states a_sm:"<<aut.has_unachievable_states()<<endl;
}
inline size_t text_len(char* first, char* last){
    return last-first+1;
}
inline char* last_symbol(char* text, size_t len){
    return text+len-1;
}



bool StupidOptimaTest(){
    return true;
}
void find_all(char* text,size_t len,Automata& aut,list<pair<char*,char*> >* bad_words_pos=0){
    pair<char*, char*> par(0,0);
    char* next_pos=text;
    char* lastsimbol=last_symbol(text, len);
    do{
        par=aut.index_in(next_pos,text_len(next_pos,lastsimbol),false);
        if(par.first){
            next_pos=par.second;
            if(bad_words_pos)bad_words_pos->push_back(make_pair(par.first,par.second));
            if(next_pos>=lastsimbol)return;
        }
    }while(par.first!=0);
}
void find_all_lib(char* text,size_t len,list<pair<char*,char*> >* bad_words_pos=0){
    char* begin=0;
    char* end=0;
    char* next_pos=text;
    char* lastsimbol=last_symbol(text, len);
    do{

        find_first_lib(next_pos,lastsimbol,&begin,&end,false);
        if(end){
            next_pos=end;
            if(bad_words_pos)bad_words_pos->push_back(make_pair(begin,end));
            if(next_pos>=lastsimbol)return;
        }
    }while(end!=0);

}
bool StupidTest(){
    //проверить тут parce re
    Automata aut;
    vector<QTextCodec*> text_codecs;
    text_codecs.push_back(QTextCodec::codecForName("Windows-1251"));
    parcere("a|bcg",aut,text_codecs);

    string s1="abcg";
    string s2="acg";
    string s3="bcg";
    string s4="bc";
    string s5="ag";
    aut.to_dfa();
    pair<char*,char*> p=aut.index_in(const_cast<char*>(s1.c_str()),s1.size());
    if(p.first==s1.c_str())
        return false;
    p=aut.index_in(const_cast<char*>(s2.c_str()),s2.size());
    if(p.first!=s2.c_str())
        return false;
    p=aut.index_in(const_cast<char*>(s3.c_str()),s3.size());
    if(p.first!=s3.c_str())
        return false;
    p=aut.index_in(const_cast<char*>(s4.c_str()),s4.size());
    if(p.first)
        return false;
    p=aut.index_in(const_cast<char*>(s5.c_str()),s5.size());
    if(p.first)
        return false;

    //opimize stupid test
    //MIHA
    //MAHA
    Automata a10(true);
    Automata a20(true);
    Automata a30(true);
    a10+'M'+'I'+'H'+'A';
    a20+'M'+'A'+'H'+'A';
    a30+'K'+'P'+'A';
    a10*a20;
    a10.to_dfa();
    a10*a30;
    a10.to_dfa();
    a10.suffix_minimize();
    return true;
}
bool LittelBitLessStupidTest(){
  size_t word_count=50;
  vector<QString> bad_words;
  vector<QString> word_list;
  readWordList(bad_words);
  word_list=getWordList(bad_words,word_count);
  list<char*> bad_words_pos;
  char* bigText=createText(1024*2,word_list,&bad_words_pos);
  Automata aut(false);
  createAtoumata(word_list,aut);
  list<pair<char*,char*> > new_bad_words_pos;
  find_all(bigText,1024*2,aut,&new_bad_words_pos);

  delete[] bigText;
  return new_bad_words_pos.size()<=bad_words_pos.size()&&new_bad_words_pos.size()!=0;
}

int main(int argc, char *argv[])
{   
    (void)argc;(void)argv;
    //очень тупая проверка на корректность
    bool test=StupidTest();
    test=LittelBitLessStupidTest()&&test;
    ofstream fsseqrch_time;
    ofstream fsgen_time;
    ofstream fsunion_time;
    ofstream fsdfa_time;
    ofstream fsepsilon_time;
    fsseqrch_time.open(PLUS_HOME(search_time.txt));
    fsgen_time.open(PLUS_HOME(gen_time.txt));
    fsunion_time.open(PLUS_HOME(union_time.txt));
    fsdfa_time.open(PLUS_HOME(dfa_time.txt));
    fsepsilon_time.open(PLUS_HOME(epsilon_time.txt));
    for(size_t word_count=3000;word_count<=3000;word_count+=50){
    //читаем список плохох слов
        vector<QString> bad_words;
        vector<QString> word_list;
        readWordList(bad_words);
        word_list=getWordList(bad_words,word_count);
        list<char*> bad_words_pos_in_text;
        char* bigText1=createText(BigTextLen,word_list,&bad_words_pos_in_text);
        char* bigText2=new char[BigTextLen];
        memcpy(bigText2,bigText1,BigTextLen);
        cout<<"word_in_text:"<<bad_words_pos_in_text.size()<<endl;
        clock_t t=clock();
        Automata aut(false);
        createAtoumata(word_list,aut);

        /////////////
        int time;
        fsgen_time<<word_count<<" "<<(time=timeMesure(t))<<endl;
        cout<<"gen_time:"<<word_count<<" "<<time<<endl;

        fsunion_time<<word_count<<" "<<union_time<<endl;
        cout<<"union_time:"<<word_count<<" "<<union_time<<endl;

        fsdfa_time<<word_count<<" "<<dfa_time<<endl;
        cout<<"dfa_time:"<<word_count<<" "<<dfa_time<<endl;

        fsepsilon_time<<word_count<<" "<<epsilon_time<<endl;
        cout<<"epsilon_time:"<<word_count<<" "<<epsilon_time<<endl;

        //создаём исходник библиотеки
        t=clock();
        ofstream fsLibC;
        fsLibC.open(PLUS_HOME(/ttt/untitled/find_lib.c));
        std::string lib_name="find_lib";
        aut.to_c_code(fsLibC, lib_name);
        fsLibC.close();
        cout<<"create_c_file:"<<word_count<<" "<<timeMesure(t)<<endl;

        t=clock();
        list<pair<char*,char*> > bad_words_pos;
        find_all_lib(bigText1,BigTextLen,&bad_words_pos);
        cout<<"seqrch_lib_time:"<<word_count<<" "<<timeMesure(t)<<endl;
        cout<<"find:"<<bad_words_pos.size()<<endl;
        for(list<pair<char*,char*> >::iterator it=bad_words_pos.begin();it!=bad_words_pos.end();++it){
            for(char* c=it->first;c!=it->second;++c)
                *c=93;
        }
        bool bigText1_test=true;
        for(char* c=bigText1;c!=bigText1+BigTextLen-2;++c){
            if(*c!=93){
                bigText1_test=false;
                break;
            }
        }
        cout<<"bigText1 test:"<<bigText1_test<<endl;
        ofstream out;
        out.open(PLUS_HOME(aut.dat),ios_base::out|ios_base::trunc|ios_base::binary);
        aut>>out;
        out.close();
        ifstream ifs;
        ifs.open(PLUS_HOME(aut.dat),ios_base::in|ios_base::binary);
        Automata aut2;
        aut2<<ifs;
        ifs.close();
        bad_words_pos.clear();
        t=clock();
        find_all(bigText2,BigTextLen,aut2,&bad_words_pos);

        fsseqrch_time<<word_count<<" "<<(time=timeMesure(t))<<endl;
        cout<<"seqrch_time:"<<word_count<<" "<<time<<endl;
        cout<<"find:"<<bad_words_pos.size()<<endl;
        for(list<pair<char*,char*> >::iterator it=bad_words_pos.begin();it!=bad_words_pos.end();++it){
            for(char* c=it->first;c!=it->second;++c)*c=93;
        }
        bool bigText2_test=true;
        for(char* c=bigText2;c!=bigText2+BigTextLen-2;++c){
            if(*c!=93){
                bigText2_test=false;
                break;
            }
        }
        cout<<"bigText2 test:"<<bigText2_test<<endl;

        fsseqrch_time.flush();
        fsgen_time.flush();
        fsunion_time.flush();
        fsdfa_time.flush();
        fsepsilon_time.flush();
        delete[] bigText1;
        delete[] bigText2;

    }
    return 0;
}

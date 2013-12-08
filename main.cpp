#ifdef UNIX
    #define PLUS_HOME( x ) "/home/stiv/"#x
#else
    #define PLUS_HOME( x ) "D:\\"#x
#endif
#include <list>
#include <algorithm>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
//#include <boost/regex.hpp>
#include <cstring>
#include <automata.h>
#include <common.h>
#include <patternchecker.h>
const int BigTextLen=1024*1024*200;
int union_time;
int dfa_time;
int epsilon_time;
using namespace std;


void readWordList(vector<string>& words){
    std::ifstream infile(PLUS_HOME(badwords.txt));
    std::string line;
    while (std::getline(infile, line))
    {
        words.push_back(line);
    }
    std::random_shuffle(words.begin(),words.end());
}
vector<string> getWordList(vector<string>& words,size_t len){
    vector<string> out;
    out.reserve(words.size());
    for(size_t i=0;i!=len && i!=words.size();++i)
        out.push_back(words[i]);
    return out;
}
char* createText(size_t len, vector<string>& words, list<char*>* bad_words_pos=0){
    char* bigText= new char[len];
    memset(bigText,0,len-1);len-=1;
     //создаём длинную, длинную сироку содержащую эти слова, сол случайно изменёнными регистрами символов
    char *pos=bigText;
    while(true){
        if(rand()%40!=0){//вставляем случайную 300 символьную последовательность
            for(int i=0;i!=900;++i){
                if(pos+i>=bigText+len-1)break;
                char c=93+rand()%34;
                *(pos+i)=c;
            }
            pos+=900;
        }else{//вставляем случайное слово
            int index=rand()%words.size();
            string &s=words[index];
            if(pos+s.size()>=bigText+len-1)break;
            memcpy(pos,s.c_str(),s.size());
            if(bad_words_pos)bad_words_pos->push_back(pos);
            pos+=s.size();
        }
    }
    return bigText;
}
void createAtoumata(Automata &aut,vector<string>& word_list){//TODO вовсех кодировках со всеми регистрами
    clock_t t=clock();
    for(size_t i=0;i!=word_list.size();++i){
        Automata wordAut(true);
        for(size_t j=0;j!=word_list[i].size();++j){
            wordAut.Concatinate(*(word_list[i].c_str()+j));
        }
        aut.Union(wordAut);
    }
    union_time=timeMesure(t);
    t=clock();
    aut.ToDFA();
    dfa_time=timeMesure(t);
}
void findAll(char* text,size_t len,PatternChecker& pc,list<char*>* bad_words_pos=0){
    pair<char*, char*> par(NULL,NULL);
    char* next_pos=text;
    char* lastsimbol=text+len-1;
    do{
        par=pc.IndexIn(next_pos,lastsimbol-next_pos+1,false);
        if(par.first){
            next_pos=par.second+1;
            if(bad_words_pos)bad_words_pos->push_back(par.first);
            if(next_pos>=lastsimbol)return;
        }
    }while(par.first!=NULL);
}
bool StupidOptimaTest(){
    return true;
}

bool StupidTest(){
    Automata aut(false);
    Automata a1(true);
    a1.Concatinate('a');
    aut.Union(a1);
    Automata a2('b');
    aut.Union(a2);

    Automata a3('c');
    Automata a4('g');
    a3.Concatinate(a4);
    aut.Concatinate(a3);

    string s1="abcg";
    string s2="acg";
    string s3="bcg";
    string s4="bc";
    string s5="ag";
    aut.ToDFA();
    PatternChecker pc(aut);
    pair<char*,char*> p=pc.IndexIn(const_cast<char*>(s1.c_str()),s1.size());
    if(p.first==s1.c_str())
        return false;
    p=pc.IndexIn(const_cast<char*>(s2.c_str()),s2.size());
    if(p.first!=s2.c_str())
        return false;
    p=pc.IndexIn(const_cast<char*>(s3.c_str()),s3.size());
    if(p.first!=s3.c_str())
        return false;
    p=pc.IndexIn(const_cast<char*>(s4.c_str()),s4.size());
    if(p.first)
        return false;
    p=pc.IndexIn(const_cast<char*>(s5.c_str()),s5.size());
    if(p.first)
        return false;
    return true;
}
bool LittelBitLessStupidTest(){
  size_t word_count=50;
  vector<string> bad_words;
  vector<string> word_list;
  readWordList(bad_words);
  word_list=getWordList(bad_words,word_count);
  list<char*> bad_words_pos;
  char* bigText=createText(1024*20,word_list,&bad_words_pos);
  Automata aut(false);
  createAtoumata(aut,word_list);

  PatternChecker pc(aut);
  list<char*> new_bad_words_pos;
  findAll(bigText,1024*20,pc,&new_bad_words_pos);

  delete[] bigText;
  return new_bad_words_pos.size()<=bad_words_pos.size()&&new_bad_words_pos.size()!=0&&new_bad_words_pos.size()>(bad_words_pos.size()-5);
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
    for(size_t word_count=3600;word_count<=3600;word_count+=50){
    //читаем список плохох слов
        vector<string> bad_words;
        vector<string> word_list;
        readWordList(bad_words);
        word_list=getWordList(bad_words,word_count);
        char* bigText=createText(BigTextLen,word_list);
        Automata aut(false);
        clock_t t=clock();
        createAtoumata(aut,word_list);
        int time;
        fsgen_time<<word_count<<" "<<(time=timeMesure(t))<<endl;
        cout<<"gen_time:"<<word_count<<" "<<time<<endl;

        fsunion_time<<word_count<<" "<<union_time<<endl;
        cout<<"union_time:"<<word_count<<" "<<union_time<<endl;

        fsdfa_time<<word_count<<" "<<dfa_time<<endl;
        cout<<"dfa_time:"<<word_count<<" "<<dfa_time<<endl;

        fsepsilon_time<<word_count<<" "<<epsilon_time<<endl;
        cout<<"epsilon_time:"<<word_count<<" "<<epsilon_time<<endl;

        PatternChecker pc(aut);
        t=clock();
        list<char*> bad_words_pos;
        findAll(bigText,BigTextLen,pc,&bad_words_pos);

        fsseqrch_time<<word_count<<" "<<(time=timeMesure(t))<<endl;
        cout<<"seqrch_time:"<<word_count<<" "<<time<<endl;
        cout<<"find:"<<bad_words_pos.size()<<endl;

        fsseqrch_time.flush();
        fsgen_time.flush();
        fsunion_time.flush();
        fsdfa_time.flush();
        fsepsilon_time.flush();
        delete[] bigText;

    }
    return 0;
}

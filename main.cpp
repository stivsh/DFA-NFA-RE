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
//#include <boost/regex.hpp>
#include <cstring>
#include <automata.h>
#include <common.h>
#include <patternchecker.h>
#include <boost/shared_array.hpp>
const int BigTextLen=1024*1024*200;//200
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
    memset(bigText,0,len-1);len-=1;
     //создаём длинную, длинную сироку содержащую эти слова, сол случайно изменёнными регистрами символов
    char *pos=bigText;
    while(true){
        if(rand()%40!=0){//вставляем случайную 300 символьную последовательность
            for(int i=0;i!=100;++i){
                if(pos+i>=bigText+len-1)break;
                char c=93;//93+rand()%34;
                *(pos+i)=c;
            }
            pos+=100;
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
void chreate_char_atomata(char* c,size_t len,Automata& charatom){
    for(size_t i=0;i!=len;++i){
        charatom.Concatinate(c[i]);
    }
}
void createAtoumata(vector<QString>& word_list,Automata& aut){//TODO вовсех кодировках со всеми регистрами
    vector<QTextCodec*> text_codecs=getTextCodecs();
    clock_t t=clock();
    for(size_t str_ind=0;str_ind!=word_list.size();++str_ind){
        QString str=word_list[str_ind];
        Automata wordAut(true);
        for(size_t ch_ind=0; ch_ind!=str.size();++ch_ind){
            QString tmp_str(str.at(ch_ind));
            Automata tmp_automata(false);
            set<QByteArray> ba_set;
            for(size_t cod_ind=0; cod_ind!=text_codecs.size();++cod_ind){
                QByteArray ba=text_codecs[cod_ind]->fromUnicode(tmp_str.toUpper());
                if(!ba_set.count(ba)){
                    Automata  atom(true);
                    chreate_char_atomata(ba.data(), ba.size(),atom);
                    tmp_automata.Union(atom);
                    ba_set.insert(ba);
                }
                ba=text_codecs[cod_ind]->fromUnicode(tmp_str.toLower());
                if(!ba_set.count(ba)){
                    Automata  atom(true);
                    chreate_char_atomata(ba.data(), ba.size(),atom);
                    tmp_automata.Union(atom);
                    ba_set.insert(ba);
                }
            }
            wordAut.Concatinate(tmp_automata);
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
/*void LibfindAll(char* text,size_t len,list<char*>* bad_words_pos=0){
    char* next=text;
    do{
        char* first_pos;
        char* last_pos;
        FindFirst(next, text+len-1,&first_pos,&last_pos, false);
        if(!last_pos)return;
        if(bad_words_pos)bad_words_pos->push_back(first_pos);
        next=last_pos+1;
    }while(next<text+len);

}*/
void LibfindBlob(boost::shared_array<char> &blob,char* text,size_t len,list<char*>* bad_words_pos=0){
    char* next=text;
    do{
        char* first_pos;
        char* last_pos;
        bool res=Automata::FindFirstByByteCode(&(blob[0]),next,text+len,&first_pos,&last_pos,false);
        if(!res)return;
        if(bad_words_pos)bad_words_pos->push_back(first_pos);
        next=last_pos+1;
    }while(next<text+len);
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
  vector<QString> bad_words;
  vector<QString> word_list;
  readWordList(bad_words);
  word_list=getWordList(bad_words,word_count);
  list<char*> bad_words_pos;
  char* bigText=createText(1024*2,word_list,&bad_words_pos);
  Automata aut(false);
  createAtoumata(word_list,aut);

  PatternChecker pc(aut);
  list<char*> new_bad_words_pos;
  findAll(bigText,1024*2,pc,&new_bad_words_pos);

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
        vector<QString> bad_words;
        vector<QString> word_list;
        readWordList(bad_words);
        word_list=getWordList(bad_words,word_count);
        list<char*> bad_words_pos_in_text;
        char* bigText=createText(BigTextLen,word_list,&bad_words_pos_in_text);
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
        aut.ToCCode(fsLibC, lib_name);
        fsLibC.close();
        cout<<"create_c_file:"<<word_count<<" "<<timeMesure(t)<<endl;

        PatternChecker pc(aut);
        t=clock();
        list<char*> bad_words_pos;
        findAll(bigText,BigTextLen,pc,&bad_words_pos);

        fsseqrch_time<<word_count<<" "<<(time=timeMesure(t))<<endl;
        cout<<"seqrch_time:"<<word_count<<" "<<time<<endl;
        cout<<"find:"<<bad_words_pos.size()<<endl;


        t=clock();
        list<char*> lib_f_wor;
        int blob_len;
        boost::shared_array<char> blob=aut.ToByteCode(&blob_len);
        t=clock();
        LibfindBlob(blob,bigText,BigTextLen,&lib_f_wor);
        cout<<"blob_find_time:"<<lib_f_wor.size()<<":time:"<<timeMesure(t)<<endl;


        fsseqrch_time.flush();
        fsgen_time.flush();
        fsunion_time.flush();
        fsdfa_time.flush();
        fsepsilon_time.flush();
        delete[] bigText;

    }
    return 0;
}

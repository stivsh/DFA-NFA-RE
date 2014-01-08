#define DEBUG 1
#ifdef UNIX
    #define PLUS_HOME( x ) "/home/stiv/"#x
#else
    #define PLUS_HOME( x ) "D:\\"#x
#endif
#include <QString>
#include <QTextStream>
#include <QTextCodec>
#include <automata.h>
#include "reparser.h"
#include <cstdio>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>
using namespace std;
int main(int argc, char *argv[])
{
    (void)argc;(void)argv;
    vector<QTextCodec*> text_codecs;
    text_codecs.push_back(QTextCodec::codecForName("Windows-1251"));
    for(;;){
        string cre;
        cout<<"re:";
        getline(cin,cre);
        QString re(cre.c_str());
        Automata a;
        parcere(re,a,text_codecs);
        string line;
        cout<<"line:";
        getline(cin, line);
        char *cline=new char[line.size()+1];
        memcpy(cline,line.c_str(),line.size());
        pair<char*, char*> pos=a.index_in(cline,line.size(),false);
        if(pos.first){
            for(char *c=pos.first;c!=pos.second;++c){
                *c='*';
            }
        }
        cline[line.size()]='\0';
        cout<<cline<<endl;
        delete[] cline;


    }
    return 0;
}

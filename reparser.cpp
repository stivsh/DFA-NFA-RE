#include "reparser.h"
void chreate_char_atomata(char* c,size_t len,Automata& charatom){
    for(size_t i=0;i!=len;++i){
        charatom.concatinate(c[i]);
    }
}
void create_symbol_automata(Automata&a, QChar c,const vector<QTextCodec*>& text_codecs){
    QString tmp_str(c);
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
    a=tmp_automata;
}
bool is_symbol(const QChar& c){
    if(c!='('&&c!=')'&&c!='*'&&c!='+'&&c!='|'&&c!='\\'&&c!='?')
        return true;
    return false;

}
enum Operation{
    PLUS_OP,
    OR_OP
};
void do_operation(Operation op,Automata& a1,Automata& a2){
    if(op==PLUS_OP)a1+a2;
    if(op==OR_OP)a1*a2;
}
void spaces_aut(Automata& a){
    a*' '*'\t'*'\n'*'\v'*'\f'*'\r';
}
void symbols_aut(Automata& a){
    a*'a'*'b'*'c'*'d'*'e'*'f'*'g'*'h'*'i'*'j'*
            'k'*'l'*'m'*'n'*'o'*'p'*'q'*'r'*'s'*'t'*
            'u'*'v'*'w'*'x'*'y'*'z'
            *'A'*'B'*'C'*'D'*'E'*'F'*'G'*'H'*'I'*'J'*
            'K'*'L'*'M'*'N'*'O'*'P'*'Q'*'R'*'S'*'T'*
            'U'*'V'*'W'*'X'*'Y'*'Z';
}
void digits_aut(Automata& a){
    a*'0'*'1'*'2'*'3'*'4'*'5'*'6'*'7'*'8'*'9';
}

void read_special_symbol(QChar c, Automata& a,const vector<QTextCodec*>& text_codecs){
    if(c=='d'){
        digits_aut(a);
        return;
    }
    if(c=='s'){
        spaces_aut(a);
        return;
    }
    if(c=='w'){
        symbols_aut(a);
        return;
    }
    create_symbol_automata(a,c,text_codecs);
}

void create_str_automata(Operation op,int& pos,QString &re,Automata& aut,const vector<QTextCodec*>& text_codecs){
    if(pos>=re.size())return;
    for(;pos<re.size();++pos){
        if(is_symbol(re[pos])){
            Automata a;
            create_symbol_automata(a,re[pos],text_codecs);
            do_operation(op,aut,a);
            op=PLUS_OP;
            continue;
        }
        if(re[pos]=='|'){
            op=OR_OP;
            continue;
        }
        if(re[pos]=='*'){
            aut.knee_star();
            op=PLUS_OP;
            continue;
        }
        if(re[pos]=='+'){
            Automata a;
            aut.clone_to(a);
            a.knee_star();
            aut+a;
            op=PLUS_OP;
            continue;
        }
        if(re[pos]=='?'){
            Automata a;
            aut*a;
            op=PLUS_OP;
            continue;
        }
        if(re[pos]=='('){
            Automata a;
            create_str_automata(PLUS_OP,++pos,re,a,text_codecs);
            do_operation(op,aut,a);
            op=PLUS_OP;
            continue;
        }
        if(re[pos]=='\\'){
            ++pos;
            if(pos>=re.size())return;
            Automata a(false);
            read_special_symbol(re[pos],a,text_codecs);
            do_operation(op,aut,a);
            continue;
        }
        if(re[pos]==')'){
            return;
        }

    }
}

void create_automata(QString &re,Automata& a,const vector<QTextCodec*>& text_codecs){
    Automata aut;
    int pos=0;
    create_str_automata(PLUS_OP,pos,re,aut,text_codecs);
    a=aut;
    a.to_dfa();
    a.suffix_minimize();
}

void parcere(QString regex,Automata& a,const vector<QTextCodec*>& text_codecs){
    create_automata(regex, a,text_codecs);
}

#ifndef REPARSER_H
#define REPARSER_H
#include <QString>
#include "automata.h"
#include <queue>
#include <string>
#include <stack>
#include <exception>
#include <QTextCodec>
void parcere(QString regex,Automata& a,const vector<QTextCodec*>& text_codecs);

#endif // REPARSER_H

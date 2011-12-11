#include <QRegExp>
#include <QVector>
#include <QPair>
#include <QStringList>
#include <QDebug>
#include <QString>
#include <QTextStream>
class commandresult{
public:
    QStringList l;
    QVector<int> splited;
};

class command{
public:
QString a;
QString b;
bool need_a;
bool need_b;
bool print_between;
QVector<QPair<int,int> > get_matches(QString pattern,QString input)
{
    QVector<QPair<int,int> > ret;
    QString str = input;
    QRegExp rx(pattern);
    int count = 0;
    int pos = 0;
    while ((pos = rx.indexIn(str, pos)) != -1) {
        ++count;
        ret.append(QPair<int,int>(pos,pos+rx.matchedLength()));
        pos += rx.matchedLength();
    }
    return ret;
}

commandresult process(QString input)
{
    QVector<QPair<int,int> > aa=get_matches(a,input);
    QVector<QPair<int,int> > bb=get_matches(b,input);
    QVector<QPair<int,int> > pairs;
    int i;
    int prev_pos=0;
    int index=0;
    commandresult r;
    for (i=0;i<aa.size();i++)
    {
        int j;
        int startpos=need_a?aa[i].first:aa[i].second;
        int endpos;
        for (j=0;j<bb.size();j++)
        {
            endpos=need_b?bb[j].second:bb[j].first;
  //          qDebug()<<"startpos:"<<startpos<<"endpos:"<<endpos;
            if (startpos<endpos) break;
        }
        //    qDebug()<<startpos<<"<-startpos"<<prev_pos<<"<-prevpos";
        r.l.append(input.mid(prev_pos,startpos-prev_pos));
       // qDebug()<<"prevpos->startpos:"<<input.mid(prev_pos,startpos-prev_pos);
        index++;
        r.splited.append(index);
        if (print_between) r.l.append(input.mid(startpos,endpos-startpos));
        else
            r.l.append("");
        index++;
        prev_pos=endpos;

    }
    r.l.append(input.mid(prev_pos) );
return r;

}

};

class run{
public:
    bool default_print;
    QVector<command> commands;
    QString process(QString input){
        QString actual=input;
        foreach(command c,commands)
        {
            commandresult cr=c.process(actual);
            actual="";
            int i;
            if (!default_print)
            {
            for (i=0;i<cr.splited.size();i++)
            {
       //           qDebug()<<"crli:"<<cr.l[cr.splited[i] ];

                actual+=cr.l[cr.splited[i] ];
            }

            }
            else
            {
                for (i=0;i<cr.l.size();i++)
                {
                  //  qDebug()<<"crli:"<<cr.l[i];
                    actual+=cr.l[i];
                }
            }
        }
        return actual;
    }
};

QTextStream out(stdout);
void print_help()
{
    out<<"hasznalat,parameterek: default_print:([true|false]) (baloldali_regex) kell(beleveszi a matchelesbe):([true|false]) (jobboldali_regex) kell(beleveszi a matchelesbe):([true|false|])\n";
    out<<"mit jelent a default_print?ha false,akkor azt írom ki,ami a ket matcheles kozott van.ha true,akkorazt írom ki ami a match parokon kivul seik,tehat tul keppen kiveszek.\n";
    exit(0);
}
bool parse(QString i){
    if (i=="true")
        return true;
    if (i=="false")
        return false;
    qDebug()<<"true-nak vagy false-nak akartam felolvasni,nem sikerult:"<<i;
    qDebug()<<"kilepek";
    print_help();
}
void test();

int main(int argc,char ** argv)
{
 //   test();
    QStringList l;
    for (int i=0;i<argc;i++)
        l.append(QString(argv[i]) );
    if (l.contains("--help"))
        print_help();
run r;
command c;
r.default_print=parse(l[1]);
c.print_between=!parse(l[1]);//fixme:egyelőre együtt mozog a kettő.vagyis ha azt akarod,hogy csak a találatokat írja ki,akkor false,ha azt akarod,h kivegye belőle a találatokat akkor true
c.a=l[2];
c.need_a=parse(l[3]);
c.b=l[4];
c.need_b=parse(l[5]);
r.commands.append(c);


QTextStream qtin(stdin);
QString input=qtin.readAll();
out<<r.process(input);

}

void test()
{
    {
        command c;
        c.a="<";
        c.b=">";
        c.need_a=false;
        c.need_b=false;
        c.print_between=false;
        run r;
        r.commands.append(c);
        r.default_print=true;

        QString input="ez maradjon<ez ne>ez meg a vege<ez megint nem kell> ez kell>ez is kell";
        QString output=r.process(input);
        if (output=="ez maradjon<>ez meg a vege<> ez kell>ez is kell")
        {}else {
            qDebug()<<"0HIBA!!!!";
            qDebug()<<output;
            qDebug()<<"elvart:ez maradjon<>ez meg a vege<> ez kell>ez is kell";

        }
        }

    {
    command c;
    c.a="<";
    c.b=">";
    c.need_a=true;
    c.need_b=true;
    c.print_between=false;
    run r;
    r.commands.append(c);
    r.default_print=true;

    QString input="ez maradjon<ez ne>ez meg a vege<ez megint nem kell> ez kell>ez is kell";
    QString output=r.process(input);
    if (output=="ez maradjonez meg a vege ez kell>ez is kell")
    {}else {
        qDebug()<<"1HIBA!!!!";
        qDebug()<<output;
        qDebug()<<"ez maradjonez meg a vege ez kell>ez is kell";

    }
    }

{
        command c;
    c.a="<";
    c.b=">";
    c.need_a=false;
    c.need_b=false;
    c.print_between=true;
    run r;
    r.commands.append(c);
    r.default_print=false;

    QString input="2ez maradjon<ez ne>ez meg a vege<ez megint nem kell> ez kell>ez is kell";
    QString output=r.process(input);
//    qDebug()<<output;
    if (output==    "ez neez megint nem kell"
)
    {}else {
        qDebug()<<"HIBA!!!!";
        qDebug()<<output;
        qDebug()<<"elvart:ez neez megint nem kell";

    }

    }

    {
            command c;
        c.a="<";
        c.b=">";
        c.need_a=true;
        c.need_b=true;
        c.print_between=true;
        run r;
        r.commands.append(c);
        r.default_print=false;

        QString input="ez maradjon<ez ne>ez meg a vege<ez megint nem kell> ez kell>ez is kell";
        QString output=r.process(input);
//        qDebug()<<output;
        if (output=="<ez ne><ez megint nem kell>")
        {

        }
        else {qDebug()<<"3HIBA";
            qDebug()<<output;
            qDebug()<<"elvart:<ez ne><ez megint nem kell>";

        }
    }
    {
            command c;
        c.a="a";
        c.b="b";
        c.need_a=true;
        c.need_b=true;
        c.print_between=false;
        run r;
        r.commands.append(c);
        r.default_print=true;

        QString input="kezdodikaez nem kellbez kell";
        QString output=r.process(input);
        QString exp="kezdodikez kell";
        if (output!=exp)
        {qDebug()<<"4HIBA";
            qDebug()<<output;
            qDebug()<<"elvart:"<<exp;
        }
    }

        exit(0);
        }


#include <../drawer.hh>

#include <QTcpSocket>
#include <QTcpServer>
//this game will be like labirinth  lite for mobile phones.

class Ball
{
public:
    bool start;
    const QPointF startpos;
    Ball(QPointF f):pos(f),startpos(300,300)//nagyon csunya.
    {
        start=true;
    }

    QPointF &getpos()
    {
        return pos;
    }
    QPointF &getdirection()
    {
        return direction;
    }

    void add_move(QPointF r)
    {
        QPointF prev_mouse_pos2=r;
        r=r-startpos;
        direction+=r;
//        qDebug()<<direction;
    }
    void move()
    {
        double szorzo=0.001;
        pos+=direction*szorzo;
        //qDebug()<<direction;
    }

    QString towholeString()
    {
        QString msg=
                QString::number((int)getpos().rx())+" "
                +QString::number((int)getpos().ry())+" "
                +QString::number((int)getdirection().rx())+" "
                +QString::number((int)getdirection().ry())+" ";
        return msg;
    }

    QString toString()
    {
        QString msg=
                QString::number((int)getdirection().rx())+" "
                +QString::number((int)getdirection().ry())+" ";
        return msg;
    }

    QPointF prev_mouse_pos;
    QPointF direction;
    QPointF pos;
};

/**
  Ket fele controller lesz.az egyik a lokalis valtozatokast teszi ra a ballra,a masik meg a netrol jovokkel update-eli.
  */
class BallController{
public:
    BallController(){}
    void setBall(Ball * b){this->b=b;}
    Ball *b;
};
class LocalBalllController:public BallController{
public:
    LocalBalllController()
    {
                QCursor::setPos(QPoint(300,300));
    }

    void process(QMouseEvent * e)
    {
        b->add_move(QCursor::pos());
        QCursor::setPos(QPoint(300,300));
    }
};

class RemoteBallController:public BallController{
public:
    QTcpSocket * socket;
    RemoteBallController(QTcpSocket *socket):socket(socket)
    {
    }

    void update()
    {
        while (socket->canReadLine())
        {
                char f[1000];
                socket->readLine(f,1000);
                //qDebug()<<"reader kuldott"<<QString(f);
                QStringList l=QString(f).split(" ");

                //b->getpos().setX(l[0].toInt());
                //b->getpos().setY(l[1].toInt());

                b->getdirection().setX(l[0].toInt());
                b->getdirection().setY(l[1].toInt());

        }
    }

    void send(QString a)
    {
        QString msg=a+"\n";
        socket->write(msg.toAscii().data(),msg.length());
    }
};

class client{
public:
    QVector<Ball*> *b;
    QTcpSocket * socket;
    Ball * ownball;
    client(QString host,QVector<Ball*> *b):b(b)
    {
        socket=new QTcpSocket();
        socket->connectToHost(host,12345);
    }
    void update()
    {
        ownball=b->first();
        {
            //QString msg=QString::number((int)ownball->getpos().rx())+" "+QString::number((int)ownball->getpos().ry())+"\n";
            QString msg=ownball->toString()+"\n";
            qDebug()<<"kliens kuld"<<msg;
            socket->write(msg.toAscii().data(),msg.length());
        }


        while (socket->canReadLine())
        {
            Ball * first=b->first();
      //      for (int i=1;i<b->size();i++)
        //    {delete b->back();b->pop_back();}

            char f[1000];
            socket->readLine(f,1000);
            qDebug()<<QString(f);
            QStringList l=QString(f).split(" ");
            int fieldnum_for_ball=4;
            int elemszam=(l.size()-1)/fieldnum_for_ball;
            while (elemszam>b->size())
                b->push_back(new Ball(QPointF(300,300)));
            QVector<int> ii;
            int pp=0;
            int own_index=l[0].toInt()-1;

            for (int i=0;i<elemszam;i++)
                {
                int index=0;
                if (i==own_index)
                {
                    pp=1;
                    index=0;
//                    continue;
                }
                else
                    index=i+1-pp;
                qDebug()<<i<<" "<<pp<<" "<<index<<elemszam<<b->size()<<b->operator [](index)->getpos();
                 b->operator [](index)->getpos().setX(l[(i)*fieldnum_for_ball+1].toInt());
                 b->operator [](index)->getpos().setY(l[(i)*fieldnum_for_ball+2].toInt());

                 b->operator [](index)->getdirection().setX(l[(i)*fieldnum_for_ball+3].toInt());
                 b->operator [](index)->getdirection().setY(l[(i)*fieldnum_for_ball+4].toInt());

                }


            for (int i=0;i<b->size();i++)
                qDebug()<<b->operator [](i)->getpos().rx()<<b->operator [](i)->getpos().ry();
        }
    }
};

class server{
public:
    QTcpServer * tcpServer;
    QVector<Ball*> *b;
    QVector<RemoteBallController*> remote;
    server(QVector<Ball*> *remoteballs):b(remoteballs)
    {
        tcpServer = new QTcpServer();
        if (!tcpServer->listen(QHostAddress::Any,12345)) {
            exit(0);
        }
    }
    void update()
    {
        if (tcpServer->hasPendingConnections() )
        {
            Ball * newb=new Ball(QPointF(300,300));
            RemoteBallController * r=new RemoteBallController(tcpServer->nextPendingConnection());
            r->setBall(newb);
            b->push_back(newb);
            remote.push_back(r);
        }

        foreach (RemoteBallController *r,remote)
        {
            r->update();
        }//mindenkitol bekertem a valtoztatasokat

        foreach(Ball * ab,*b)
        {
           // qDebug()<<ab;
            ab->move();
        }


        QString msg;
        foreach (Ball * w,*b)
        {
            msg+=w->towholeString();
        }

        //most pedig mindenkinek elkuldom a valtoztatasokat.
        int i=1;
        foreach (RemoteBallController *r,remote)
        {
            r->send(QString::number(i)+" "+msg);
            i++;
        }
    }

};

class Drawer2: public Drawer
{
public:
    QTimer *timer;
public:
    server * s;
    client * cli;
    QVector<Ball*> *remoteballs;
    LocalBalllController c;
public:
    Drawer2(int argc,char **argv):remoteballs()
    {
        s=0;
        cli=0;
        remoteballs=new QVector<Ball*>();
        if (argc==1)
            s=new server(remoteballs);
        else
        {
            cli=new client(QString(argv[1]),remoteballs);
            Ball *a=new Ball(QPointF(300,300));
            remoteballs->push_back(a);
            c.setBall(a);
        }

        QWidget::startTimer(20);
    }

    void timerEvent(QTimerEvent *e){
        //b.add_move(QPointF());
//        b.move();
        if (s) s->update();//mindenkinek elkuldom az infokat,es lekerem a helyeket.
        update();
        if (cli) cli->update();
        foreach(Ball * ab,*remoteballs)
        {
           // qDebug()<<ab;
            if (ab->getpos().rx()<0)
                ab->getpos().rx()=0;
            ab->move();
        }
    }

    void paintEvent(QPaintEvent *){//kulonvenni.
        QPainter p(this);
        foreach(Ball * b,*remoteballs)
            p.drawArc(b->getpos().rx(),b->getpos().ry(),100,100,0,5760);
    }
    
    void mouseMoveEvent(QMouseEvent *e)
    {//pos az elozohoz kepest?
        c.process(e);
        update();
    }
    void mousePressEvent(QMouseEvent *){
        update();
    }
    virtual void keyPressEvent(QKeyEvent *e)
    {
        if (e->text()=="q")
        exit(0);
    }

    
};


int main(int argc,char **argv)
{
    Drawer2 d(argc,argv);
    d.start(800,600);
//    d.start();
}


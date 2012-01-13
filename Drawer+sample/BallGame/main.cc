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
    void add_move(QPointF r)
    {
        QPointF prev_mouse_pos2=r;
        r=r-startpos;
        direction+=r;
    }
    void move()
    {
        double szorzo=0.001;
        pos+=direction*szorzo;        
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
        if (socket->canReadLine())
        {
                char f[1000];
                socket->readLine(f,1000);
                //qDebug()<<"reader kuldott"<<QString(f);
                QStringList l=QString(f).split(" ");

                b->getpos().setX(l[0].toInt());
                b->getpos().setY(l[1].toInt());



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
        if (socket->canReadLine())
        {
            Ball * first=b->first();
      //      for (int i=1;i<b->size();i++)
        //    {delete b->back();b->pop_back();}

            char f[1000];
            socket->readLine(f,1000);
            qDebug()<<QString(f);
            QStringList l=QString(f).split(" ");
            int elemszam=(l.size()-1)/2;
            while (elemszam>b->size())
                b->push_back(new Ball(QPointF(300,300)));
            QVector<int> ii;
            int pp=0;
int cel=(l.size()-1-1)/2;
qDebug()<<"a"<<l.size()<<cel;
            for (int i=1;i<cel;i++)
                //ii.push_back(l[i].toInt() );
                if (i-1!=l[0].toInt() )
          //      b->push_back(new Ball(QPointF(l[i*2+pp].toInt(),l[i*2+pp+1].toInt()) ));
                {
                    qDebug()<<126<<" "<<i<<" "<<l[i*2+pp-1].toInt()<<" "<<l[i*2+pp].toInt();
                    b->operator [](i)->getpos().setX(l[i*2+pp-1].toInt());
                 b->operator [](i)->getpos().setY(l[i*2+pp].toInt());
                }
                    else pp=0;
            for (int i=0;i<b->size();i++)
                qDebug()<<b->operator [](i)->getpos().rx()<<b->operator [](i)->getpos().ry();
        }
        ownball=b->first();
        {
            QString msg=QString::number((int)ownball->getpos().rx())+" "+QString::number((int)ownball->getpos().ry())+"\n";
            qDebug()<<"kliens kuld"<<msg;
            socket->write(msg.toAscii().data(),msg.length());
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


        QString msg;
        foreach (Ball * w,*b)
        {
            msg+=QString::number((int)w->getpos().rx())+" "+QString::number((int)w->getpos().ry())+" ";
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
    Drawer2():remoteballs()
    {
        s=0;
        cli=0;
        Ball *a=new Ball(QPointF(300,300));
        remoteballs=new QVector<Ball*>();
        remoteballs->push_back(a);
        c.setBall(a);
        QWidget::startTimer(10);
    }
    void args(int argc,char **argv)
    {
        if (argc==1)
            s=new server(remoteballs);
        else
        {
            cli=new client(QString(argv[1]),remoteballs);
        }
    }

    void timerEvent(QTimerEvent *e){
        //b.add_move(QPointF());
//        b.move();
        if (s) s->update();
        if (cli) cli->update();
        foreach(Ball * ab,*remoteballs)
        {
           // qDebug()<<ab;
            ab->move();
        }
        update();
    }

    void paintEvent(QPaintEvent *){//kulonvenni.
        QPainter p(this);
        qDebug()<<"remoteszam:"<<remoteballs->size();
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
    Drawer2 d;
    d.args(argc,argv);
    d.start(800,600);
//    d.start();
}


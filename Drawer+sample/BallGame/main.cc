#include <../drawer.hh>

#define ballsize 10

#include <QTcpSocket>
#include <QTcpServer>
//this game will be like labirinth  lite for mobile phones.

qreal distance(QPointF &a,QPointF &b)
{
    qreal asq=qAbs(a.rx()-b.rx());asq*=asq;
    qreal bsq=qAbs(a.ry()-b.ry());bsq*=bsq;
    return sqrt(asq+bsq);
}

qreal sqr(qreal a)
{
    return a*a;
}

class Ball;

void calculate_collision(Ball * a,Ball * b, qreal b2m=1);

bool wall_collision(Ball * a);

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
    QByteArray toarray()
    {
        QByteArray msg;
        qint32 dirx=(qint32)getdirection().rx();
        qint32 diry=(qint32)getdirection().ry();
        msg.append (  (char *)&dirx,4 );
        msg.append (  (char *)&diry,4 );
        return msg;
    }

    QByteArray towholearray()
    {
        QByteArray msg;
        qint32 posx=(qint32)getpos().rx();
        qint32 posy=(qint32)getpos().ry();
        qint32 dirx=(qint32)getdirection().rx();
        qint32 diry=(qint32)getdirection().ry();

        msg.append (  (char *)&posx,4 );
        msg.append (  (char *)&posy,4 );
        msg.append (  (char *)&dirx,4 );
        msg.append (  (char *)&diry,4 );
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

        while (socket->bytesAvailable()>=8)
        {

                //b->getpos().setX(l[0].toInt());
                //b->getpos().setY(l[1].toInt());
                qint32 dirx;
                qint32 diry;
                socket->read((char*)&dirx,4);
                socket->read((char*)&diry,4);

                qDebug()<<"read_dirbef"<<b->getdirection().rx()<<b->getdirection().ry()
                <<dirx<<diry;

//                b->getdirection().setX(dirx);
  //              b->getdirection().setY(diry);
  //              if ((b->getdirection().rx()*(-1)!=(dirx) ) )
                if (qAbs(b->getdirection().rx()*-1-(dirx) )>15 )
                //if (dirx*-1!=b->getdirection().rx())

                b->getdirection().rx()=(dirx);
//                if ((b->getdirection().ry()*(-1)!=(diry) ) )
    //            if (qAbs(b->getdirection().ry()*-(diry) )<1000 )
//                if (diry*-1!=b->getdirection().ry())
                if (qAbs(b->getdirection().ry()*-1-(diry) )>15 )
                b->getdirection().ry()=(diry);

      //          qDebug()<<"read_dir"<<b->getdirection().rx()<<b->getdirection().ry();

        }
    }

    void send(QByteArray a)
    {
        socket->write(a);
    }
};

class client{
public:
    QVector<Ball*> *b;
    QTcpSocket * socket;
    Ball * ownball;
    qreal last_sent_x,last_sent_y;
    client(QString host,QVector<Ball*> *b):b(b)
    {
        socket=new QTcpSocket();
        socket->connectToHost(host,12345);
        last_sent_x=0;
        last_sent_y=0;
    }
    void update()
    {
        ownball=b->first();
        qreal rx=ownball->getdirection().rx();
        qreal ry=ownball->getdirection().ry();


        //qDebug()<<last_sent_x<<ownball->getdirection().rx();
        //if (last_sent_x== ownball->getdirection().rx() && last_sent_y==ownball->getdirection().ry() )
        {//nem cseszegette a szerver az iranyvektort.
            //QString msg=QString::number((int)ownball->getpos().rx())+" "+QString::number((int)ownball->getpos().ry())+"\n";
            qDebug()<<"sent"<<ownball->getdirection().rx()<<ownball->getdirection().ry();
            //socket->write(ownball->towholearray() );
            ownball->getdirection().rx()=rx;
            ownball->getdirection().ry()=ry;
            socket->write(ownball->toarray());
        }

        while (socket->bytesAvailable()>=8)
        {
            Ball * first=b->first();
      //      for (int i=1;i<b->size();i++)
        //    {delete b->back();b->pop_back();}
qint32 ownindex;
qint32 lsize;
socket->read((char*)&ownindex,4);
socket->read((char*)&lsize,4);
QVector<qint32> l;
l.push_back(ownindex);
int fieldnum_for_ball=4;
for (int i=0;i<lsize;i++)
{
    for (int j=0;j<fieldnum_for_ball;j++)
    {
        qint32 tmp;
        socket->read((char*)&tmp,4);
        l.push_back(tmp);
    }
}

            int elemszam=(l.size()-1)/fieldnum_for_ball;
            while (elemszam>b->size())
                b->push_back(new Ball(QPointF(300,300)));
            int pp=0;
            int own_index=l[0];

            for (int i=0;i<elemszam;i++)
                {
                int index=0;
                if (i==own_index)
                {
                    pp=1;
                    index=0;
                }
                else
                    index=i+1-pp;
                 b->operator [](index)->getpos().setX(l[(i)*fieldnum_for_ball+1]);
                 b->operator [](index)->getpos().setY(l[(i)*fieldnum_for_ball+2]);

                 b->operator [](index)->getdirection().setX(l[(i)*fieldnum_for_ball+3]);
                 b->operator [](index)->getdirection().setY(l[(i)*fieldnum_for_ball+4]);
                 if (index==0)
                     qDebug()<<"rec"<<b->operator [](index)->getpos().rx()<<b->operator [](index)->getpos().ry()
                               <<b->operator [](index)->getdirection().rx()<<b->operator [](index)->getdirection().ry();

            }
        }

        last_sent_x=ownball->getdirection().rx();
        last_sent_y=ownball->getdirection().ry();


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
    QVector<int> gameplay_logic()
    {
        QVector<int> ret;
        for (int i=0;i<b->size();i++)
        {
            QPointF icenter(b->operator [](i)->getpos().rx()+ballsize/2,b->operator [](i)->getpos().ry()+ballsize/2  );
            for (int j=i+1;j<b->size();j++)
            {
                QPointF jcenter(
                            b->operator [](j)->getpos().rx()+ballsize/2,
                            b->operator [](j)->getpos().ry()+ballsize/2  );
                if (distance(icenter,jcenter)<ballsize)
                    {
                    calculate_collision(b->operator [](i),b->operator [](j));
                    }
            }
            wall_collision(b->operator [](i));

        }
        foreach(Ball * ab,*b)
        {
           // qDebug()<<ab;
            ab->move();
        }
        return ret;
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

        QVector<int> new_directions=gameplay_logic();//do phisycs and stuff
        for (int i=0;i<new_directions.size();i++)
        {

        }
        QByteArray msg;
        qint32 bsize=b->size();
        msg.append((char*)&bsize,4);

        QByteArray msg2;
        foreach (Ball * w,*b)
        {
            msg2+=w->towholearray();
            QByteArray b=w->towholearray();
        }

        //most pedig mindenkinek elkuldom a valtoztatasokat.
        qint32 i=0;
        foreach (RemoteBallController *r,remote)
        {
            QByteArray msg3;
            msg3.append((char*)&i,4);

            r->send(msg3+msg+msg2);
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

        QWidget::startTimer(8);
    }

    void timerEvent(QTimerEvent *e){
        //b.add_move(QPointF());
//        b.move();
        if (s) s->update();//mindenkinek elkuldom az infokat,es lekerem a helyeket.
        update();
        if (cli) cli->update();
    }

    void paintEvent(QPaintEvent *){//kulonvenni.
        QPainter p(this);
        foreach(Ball * b,*remoteballs)
            p.drawArc(b->getpos().rx(),b->getpos().ry(),ballsize,ballsize,0,5760);
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



void calculate_collision(Ball * a,Ball * b,qreal b2m)
{
    //forras: http://www.developer.nokia.com/Community/Wiki/Collision_for_Balls
    qreal dx=a->getpos().rx()-b->getpos().rx();
    qreal dy=a->getpos().ry()-b->getpos().ry();
    qreal colision_angle=atan2(dx,dy);
    qreal speed1=sqrt(sqr(a->getdirection().rx())+sqr(b->getdirection().rx()) );
    qreal speed2=sqrt(sqr(a->getdirection().ry())+sqr(b->getdirection().ry()) );
    qreal direction1=atan2(a->getdirection().rx(),a->getdirection().ry());
    qreal direction2=atan2(b->getdirection().rx(),b->getdirection().ry());

    qreal vx_1 = speed1 * cos(direction1 - colision_angle);
    qreal vy_1 = speed1 * sin(direction1 - colision_angle);
    qreal vx_2 = speed2 * cos(direction2 - colision_angle);
    qreal vy_2 = speed2 * sin(direction2 - colision_angle);

    qreal ball1mass=1,ball2mass=b2m;
    qreal final_vx_1 = ((ball1mass - ball2mass) * vx_1 + (ball2mass + ball2mass) * vx_2)/(ball1mass + ball2mass);
    qreal final_vx_2 = ((ball1mass + ball1mass) * vx_1 + (ball2mass - ball1mass) * vx_2)/(ball1mass + ball2mass);
    qreal final_vy_1 = vy_1;
    qreal final_vy_2 = vy_2;

     a->getdirection().rx() = cos(colision_angle) * final_vx_1 + cos(colision_angle + M_PI/2) * final_vy_1;
     a->getdirection().ry() = sin(colision_angle) * final_vx_1 + sin(colision_angle + M_PI/2) * final_vy_1;
     b->getdirection().rx() = cos(colision_angle) * final_vx_2 + cos(colision_angle + M_PI/2) * final_vy_2;
     b->getdirection().ry() = sin(colision_angle) * final_vx_2 + sin(colision_angle + M_PI/2) * final_vy_2;
}

bool wall_collision(Ball * a)
{
    int w=800,h=600;
    if (a->getpos().rx()<0)
    {
        a->getdirection().rx()*=-1.;
        a->getpos().rx()=0;
        qDebug()<<"balrol";
    return true;
    }
else
    if (a->getpos().rx()>w-ballsize*3)
    {
        a->getdirection().rx()*=-1.;
        a->getpos().rx()=w-ballsize*3;
        qDebug()<<"jobbrol";
        return true;
    }

    if (a->getpos().ry()<0)
    {
        a->getdirection().ry()*=-1.;
        a->getpos().ry()=0;
        qDebug()<<"fentrol";
        return true;
    }
else
    if (a->getpos().ry()>h-ballsize*3)
    {
        a->getdirection().ry()*=-1.;
        a->getpos().ry()=h-ballsize*3;
        qDebug()<<"lentrol";
        return true;
    }

    return false;
}

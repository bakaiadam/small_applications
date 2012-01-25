#include <../drawer.hh>

#define ballsize 10
#define w 800
#define h 600
#define border 100

#include <QTcpSocket>
#include <QTcpServer>
//this game will be like labirinth  lite for mobile phones.

QByteArray imagetobytearray(const QImage * img)
{
    QBuffer b;
    b.open(QIODevice::ReadWrite);
    img->save(&b,"jpg");
    QByteArray bb=b.buffer();
    //quint32 size=bb.size();QByteArray bb2((char*)&size,4);
    //return bb2+bb;
    return bb;
}

void readimage(QTcpSocket * socket, QImage *img)
{
//    quint32 size;
//    socket->read((char *)&size,4);
//    QByteArray ba=socket->read(size);
//   QBuffer buf(&ba,0);
//    buf.open(QIODevice::ReadOnly);
    img->load(socket,"jpg");
}



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
    qint32 lap;
    qint32 &getlap(){return lap;}
    bool start;
    const QPointF startpos;
    Ball(QPointF f):pos(f),startpos(300,300)//nagyon csunya.
    {
        lap=0;
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

        msg.append (  (char *)&lap,4 );

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
    qreal sendx,sendy;
    RemoteBallController(QTcpSocket *socket):socket(socket)
    {
        sendx=0;
        sendy=0;
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
                b->getdirection().rx()+=(dirx);
                b->getdirection().ry()+=(diry);
                sendx=0;sendy=0;
      //          qDebug()<<"read_dir"<<b->getdirection().rx()<<b->getdirection().ry();

        }
    }

    void send(QImage * a)
    {
        QDataStream st(socket);
        st<<*a;
    }
    void send(const QByteArray & ba)
    {
        socket->write(ba);
    }

};

class client{
public:
    QVector<Ball*> *b;
    QTcpSocket * socket;
    Ball * ownball;
    qreal last_sent_x,last_sent_y;
    QImage * canvas;
    client(QString host,QVector<Ball*> *b,QImage * canvas):b(b),canvas(canvas)
    {
        ownball=b->operator [](0);
        socket=new QTcpSocket();
        socket->connectToHost(host,12345);
        last_sent_x=0;
        last_sent_y=0;
    }
    void update()
    {
        while (socket->bytesAvailable()>0)
        {
         //   qDebug()<<"aval:"<<socket->bytesAvailable();
//            QDataStream st(socket);
  //          st>>*canvas;
        readimage(socket,canvas);
        }
        socket->write(ownball->toarray());
        ownball->direction.rx()=0;
        ownball->direction.ry()=0;
    }
};

class server{
public:
    QImage *canvas;
    QTcpServer * tcpServer;
    QVector<Ball*> *b;
    QVector<RemoteBallController*> remote;
    QVector<int> pos;
    server(QVector<Ball*> *remoteballs,QImage * c):b(remoteballs),canvas(c)
    {
        tcpServer = new QTcpServer();
        if (!tcpServer->listen(QHostAddress::Any,12345)) {
            qDebug()<<"listen nem sikerult";
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
                    ret.push_back(i);
                    ret.push_back(j);
                    }
            }
            if (wall_collision(b->operator [](i))) ret.push_back(i);

        }
        foreach(Ball * ab,*b)
        {
           // qDebug()<<ab;
            ab->move();
        }

        int i=0;
        foreach(Ball * ab,*b)
        {
            if (i==pos.size())
                pos.push_back(1);
            else
            {

                qreal rx=ab->getpos().rx(),ry=ab->getpos().ry();
                if (pos[i]==3 && rx<border && ry<border)
                {
                    ab->lap++;
                    pos[i]=0;
                }
                if (pos[i]==0 && rx>w-border && ry<border)
                {
                    pos[i]=1;
                }
                if (pos[i]==1 && rx>w-border && ry>h-border)
                {
                    pos[i]=2;
                }
                if (pos[i]==2 && rx<border && ry>h-border)
                {
                    pos[i]=3;
                }
            }
            i++;
        }



        return ret;
    }

    void update_canvas()
    {
        QPainter p(canvas);
        p.fillRect(QRect(0,0,w,h),Qt::white);
                int i=0;
                foreach (RemoteBallController *r,remote)
                {
                    Ball *b=r->b;
                    p.setBrush(QBrush(QColor::fromHsv(i*360/remote.size()+1,255,255) ));
                    p.drawChord(b->getpos().rx(),b->getpos().ry(),ballsize,ballsize,0,5760);
                    p.drawText(i*w/remote.size(),10,QString::number(b->getlap() ) );
                i++;
                }
                p.setBrush(Qt::NoBrush);
                p.drawRect(0,0,border,border);
                p.drawRect(w-border,0,border,border);
                p.drawRect(w-border,h-border,border,border);
                p.drawRect(0,h-border,border,border);
                p.end();
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
        update_canvas();
        QByteArray ba=imagetobytearray(canvas);

       //most pedig mindenkinek elkuldom a valtoztatasokat.
        qint32 i=0;
        foreach (RemoteBallController *r,remote)
        {
                r->send(ba);
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
    QImage *gamecanvas;
    QMutex m;
public:
    ~Drawer2(){}
    Drawer2(int width, int height, int argc,char **argv):remoteballs()
    {
        gamecanvas=new QImage(width,height,QImage::Format_RGB888);
        m.lock();
        s=0;
        cli=0;
        remoteballs=new QVector<Ball*>();
        if (argc==1)
            s=new server(remoteballs,gamecanvas);
        else
        {
            Ball *a=new Ball(QPointF(300,300));
            remoteballs->push_back(a);
            c.setBall(a);
            cli=new client(QString(argv[1]),remoteballs,gamecanvas);
        }
        QWidget::startTimer(8);
        m.unlock();
        start(width,height);
    }

    void timerEvent(QTimerEvent *e){
      //  qDebug()<<__LINE__;
        m.lock();
      //  qDebug()<<__LINE__;
        //if (!a && width()!=0)
          //  a=new QImage(QSize(width(),height()),QImage::Format_RGB888);

        //b.add_move(QPointF());
//        b.move();
      //  qDebug()<<__LINE__;
        if (s) s->update();//mindenkinek elkuldom az infokat,es lekerem a helyeket.
      //  qDebug()<<__LINE__;
        update();
      //  qDebug()<<__LINE__;
        if (cli) cli->update();
      //  qDebug()<<__LINE__;
        m.unlock();
      //  qDebug()<<__LINE__;
    }

    void paintEvent(QPaintEvent *){//kulonvenni.
        QPainter p2(this);
        p2.drawImage(QPoint(),*gamecanvas);
p2.end();
    /*    QPainter p2(a);
        p2.begin(a);
        render(&p2);
        p2.end();
      */  //delete a;

    }
    
    void mouseMoveEvent(QMouseEvent *e)
    {//pos az elozohoz kepest?
        m.lock();
        if (cli)
        c.process(e);
        update();
        m.unlock();
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
    QApplication app(argc,argv);
    Drawer2 d(800,600,argc,argv);
//    d.start(800,600);
    return app.exec();
}


#include <QVector2D>
void calculate_collision(Ball * a,Ball * b,qreal b2m)
{
    //forras: http://www.developer.nokia.com/Community/Wiki/Collision_for_Balls
    /*qreal dx=a->getpos().rx()-b->getpos().rx();
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
     */
//http://stackoverflow.com/questions/345838/ball-to-ball-collision-detection-and-handling

    // get the mtd
    QVector2D position(a->getpos());
    QVector2D ballposition(b->getpos());
        QVector2D delta = (position-(ballposition));
        qreal d = delta.length();
        // minimum translation distance to push balls apart after intersecting
        qreal radius=ballsize/2;
        QVector2D mtd = delta*(((radius + radius)-d)/d);


        qreal mass=1.0;
        qreal ballmass=1.0;
        // resolve intersection --
        // inverse mass quantities
        float im1 = 1 / mass;
        float im2 = 1 / ballmass;

        // push-pull them apart based off their mass
        position = position+(mtd*(im1 / (im1 + im2)));
        ballposition = ballposition-(mtd*(im2 / (im1 + im2)));

        // impact speed
        QVector2D velocity(a->getdirection());
        QVector2D ballvelocity(b->getdirection());
        QVector2D v = velocity-ballvelocity ;
        qreal vn = QVector2D::dotProduct(v,mtd.normalized());
        // sphere intersecting but moving away from each other already
        if (vn > 0.0f) return;

        // collision impulse
        qreal restitution=0.9;
        float i = (-(1.0f + restitution) * vn) / (im1 + im2);
        QVector2D impulse = mtd*(i);

        // change in momentum

        velocity = velocity+(impulse*(im1));
        ballvelocity = ballvelocity-(impulse*(im2));
        a->getpos()=QPointF(position.toPointF());
        b->getpos()=QPointF(ballposition.toPointF());
        a->getdirection()=QPointF(velocity.toPointF());
        b->getdirection()=QPointF(ballvelocity.toPointF());

}

bool wall_collision(Ball * a)
{
    qreal restitution=0.9;
    restitution*=-1;

    if (a->getpos().rx()<0)
    {
        a->getdirection().rx()*=restitution;
        a->getpos().rx()=0;
        qDebug()<<"balrol";
    return true;
    }
else
    if (a->getpos().rx()>w-ballsize*3)
    {
        a->getdirection().rx()*=restitution;
        a->getpos().rx()=w-ballsize*3;
        qDebug()<<"jobbrol";
        return true;
    }

    if (a->getpos().ry()<0)
    {
        a->getdirection().ry()*=restitution;
        a->getpos().ry()=0;
        qDebug()<<"fentrol";
        return true;
    }
else
    if (a->getpos().ry()>h-ballsize*3)
    {
        a->getdirection().ry()*=restitution;
        a->getpos().ry()=h-ballsize*3;
        qDebug()<<"lentrol";
        return true;
    }

    return false;
}

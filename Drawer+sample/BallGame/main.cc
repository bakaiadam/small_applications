#include <../drawer.hh>
#include <chipmunk/chipmunk.h>
#define ballsize 10
#define w 800
#define h 600
#define border 100

#include <QTcpSocket>
#include <QTcpServer>
//this game will be like labirinth  lite for mobile phones.


class client_settings{
public:
    QString host;
    bool keyboard1;
    bool keyboard2;
    bool mouse;
    bool need_image;
};

class Ball;

void calculate_collision(Ball * a,Ball * b, qreal b2m=1);

bool wall_collision(Ball * a);

QByteArray imagetobytearray(const QImage * img)
{
    QBuffer b;
    b.open(QIODevice::ReadWrite);
    img->save(&b,"jpg");
    QByteArray bb=b.buffer();
    quint32 size=bb.size();QByteArray bb2((char*)&size,4);
    return bb2+bb;
    //return bb;
}

void readimage(QTcpSocket * socket, QImage *img)
{
    quint32 size;
    socket->read((char *)&size,4);
//    QByteArray ba=socket->read(size);
//   QBuffer buf(&ba,0);
//    buf.open(QIODevice::ReadOnly);
    while (socket->bytesAvailable()<0)
        socket->waitForReadyRead();
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

class Ball
{
public:
    qint32 lap;
    qint32 &getlap(){return lap;}
    bool start;
    const QPointF startpos;
    cpSpace *space;
    cpBody * body;
    Ball(QPointF f):pos(f),startpos(0,0)//nagyon csunya.
    {
      space=0;
      body=0;
    }

    Ball(QPointF f,cpSpace *space):pos(f),startpos(0,0)//nagyon csunya.
    {
      this->space=space;

      cpFloat radius = 5;
      cpFloat mass = 1;
      cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);

      body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
      cpBodySetPos(body, cpv(pos.x(), pos.y()));

      cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
      cpShapeSetElasticity(ballShape, 0.5f);
      cpShapeSetFriction(ballShape, 0.0f);


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
      cpVect v=cpBodyGetPos(body);
      pos.rx()=v.x;
      pos.ry()=v.y;
      if (body)
      cpBodyApplyImpulse(body,cpv(direction.rx()*0.05,direction.ry()*0.05),cpv(0,0));
      direction.rx()=0;
      direction.ry()=0;
//        cpBodyApplyForce(body,cpv(direction.rx()*0.001,direction.ry()*0.001),cpv(0,0));
        /*        double szorzo=0.01;
        pos+=direction*szorzo;
        */
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
ennek az osnek a celja az h ilyenekbe legyenek benne mindig a jatekszabalyok.egeszen a pattogastol kezdve a pontszamolasig minden.*/
class GameMap{
   public:
        virtual void process(QVector<Ball*> &balls, QPainter &p)=0;
    virtual Ball * new_user()=0;
};

class AlapMap:public GameMap
{//van benne pattogas.
    QVector<int> pontok;
    QPointF elso,masodik;
    QRegion tilos_helyek;

    QVector<cpBody*> bodies;
    cpSpace *space;
    cpFloat timeStep;
public:
    AlapMap()
    {
      //http://chipmunk-physics.net/release/ChipmunkLatest-Docs/#cpShape
        int dist_from_border=40;
        elso=QPointF(dist_from_border,dist_from_border);
        masodik=QPointF(w-dist_from_border,h-dist_from_border);
        tilos_helyek+=(QRect(0,200,700,20)  );
        tilos_helyek+=(QRect(100,300,700,20));

        timeStep = 1.0/60.0;
        cpVect gravity = cpv(0, 0);
        space = cpSpaceNew();
        cpSpaceSetGravity(space, gravity);

        cpShape *ground = cpSegmentShapeNew(space->staticBody, cpv(0, 0), cpv(0, 600), 0);
        cpShapeSetFriction(ground, 0.0);
        cpShapeSetElasticity(ground, 1.0f);
        cpSpaceAddShape(space, ground);

        ground = cpSegmentShapeNew(space->staticBody, cpv(0, 0), cpv(800, 0), 0);
        cpShapeSetFriction(ground, 0.0);
        cpShapeSetElasticity(ground, 1.0f);
        cpSpaceAddShape(space, ground);

        ground = cpSegmentShapeNew(space->staticBody, cpv(0, 600), cpv(800, 600), 0);
        cpShapeSetFriction(ground, 0.0);
        cpShapeSetElasticity(ground, 1.0f);
        cpSpaceAddShape(space, ground);

        ground = cpSegmentShapeNew(space->staticBody, cpv(800, 0), cpv(800, 600), 0);
        cpShapeSetFriction(ground, 0.0);
        cpShapeSetElasticity(ground, 1.0f);
        cpSpaceAddShape(space, ground);

        //        tilos_helyek=tilos_helyek.intersect(QRect(0,200,700,20) );

    }

    Ball * new_user()
    {
        if (pontok.size()==0)
            return new Ball(elso,space );
        if (pontok.size()==1)
            return new Ball(masodik,space );
        return NULL;
    }

    void gameplay_logic(QVector<Ball*> &b)
    {
  /*      for (int i=0;i<b.size();i++)
        {
            QPointF icenter(b.operator [](i)->getpos().rx()+ballsize/2,b.operator [](i)->getpos().ry()+ballsize/2  );
            for (int j=i+1;j<b.size();j++)
            {
                QPointF jcenter(
                            b.operator [](j)->getpos().rx()+ballsize/2,
                            b.operator [](j)->getpos().ry()+ballsize/2  );
                if (distance(icenter,jcenter)<ballsize)
                    {
                    calculate_collision(b.operator [](i),b.operator [](j));
                    }
            }
            wall_collision(b.operator [](i));
        }
*/
      cpSpaceStep(space, timeStep);

#define a(i,start,stop) if (b.size()>i) if (distance(b[i]->pos,stop)<30 || tilos_helyek.contains((QPoint(b[i]->pos.x(),b[i]->pos.y()) ) ) ) \
        {\
            if (distance(b[i]->pos,stop)<30 ) pontok[i]++;\
            b[i]->pos=start;\
            b[i]->direction=QPointF();\
        }
        a(0,elso,masodik);
        a(1,masodik,elso);
        #undef a

/*
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
*/
    }

    void update_canvas(QVector<Ball*> &balls,QPainter &p)
    {
        p.fillRect(QRect(0,0,w,h),Qt::white);
                int i=0;
                foreach (QRect r,tilos_helyek.rects() )
                {
                    p.fillRect(r,Qt::green );
                }

                foreach (Ball *b,balls)
                {
                    p.setBrush(QBrush(QColor::fromHsv(i*360/balls.size()+1,255,255) ));
                    p.drawChord(b->getpos().rx(),b->getpos().ry(),ballsize,ballsize,0,5760);
                    p.drawText(i*w/balls.size(),10,QString::number(pontok[i] ) );
                i++;
                }
                p.setBrush(Qt::NoBrush);
//                p.drawRect(0,0,border,border);
  //              p.drawRect(w-border,0,border,border);
    //            p.drawRect(w-border,h-border,border,border);
      //          p.drawRect(0,h-border,border,border);

                p.end();
    }

     virtual void process(QVector<Ball*> &balls, QPainter &p)
     {
        while (pontok.size()<balls.size())
            pontok.push_back(0);
           gameplay_logic(balls);
           update_canvas(balls,p);
     }

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
    QPoint def_pos;
    LocalBalllController()
    {
        def_pos=QPoint(300,300);
        QCursor::setPos(def_pos);
    }

    void process(QMouseEvent * e)
    {
        b->add_move(QCursor::pos()-def_pos);
        QCursor::setPos(def_pos);
    }
};

class LocalKeyBallController:public BallController
{
    QVector<int> a;
    QVector<bool> pushed;
public:
    LocalKeyBallController(int ka, int b, int c , int d)
    {
        a.push_back(ka);
        a.push_back(b);
        a.push_back(c);
        a.push_back(d);
        pushed=QVector<bool>(4,false);
    }
    bool valid_input(QKeyEvent * e)
    {
        return (a.contains(e->key()));
    }


    void process(QKeyEvent * e)
    {
        QPoint move;
        if (e->key()==a[0] || pushed[0])
        {
            move+=QPoint(-1,0);
            pushed[0]=true;
        }
        if (e->key()==a[1] || pushed[1])
        {
            move+=QPoint(0,-1);
            pushed[1]=true;
        }
        if (e->key()==a[2] || pushed[2])
        {
            move+=QPoint(1,0);
            pushed[2]=true;
        }
        if (e->key()==a[3] || pushed[3])
        {
            move+=QPoint(0,1);
            pushed[3]=true;
        }
        move*=70;
//        qDebug()<<move;
        b->add_move(move);
  //      QCursor::setPos(QPoint(300,300));
    }
    void r(QKeyEvent *e)
    {
        if (e->key()==a[0])
        {
            pushed[0]=false;
        }
        if (e->key()==a[1])
        {
            pushed[1]=false;
        }
        if (e->key()==a[2])
        {
            pushed[2]=false;
        }
        if (e->key()==a[3])
        {
            pushed[3]=false;
        }
    }
};

//ez felelős a szervertől a kliens felé menő kommunikációért.
class RemoteBallController:public BallController{
public:
    QTcpSocket * socket;
    qreal sendx,sendy;
    bool send_image;
    RemoteBallController(QTcpSocket *socket):socket(socket)
    {
        quint8 send_image_int;
                socket->read((char*)&send_image_int,1);
                if (send_image_int) send_image=true;
                else send_image=false;
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
        }
    }

    void send(QImage * a)
    {
        if (!send_image) return;
        QDataStream st(socket);
        st<<*a;
    }
    void send(const QByteArray & ba)
    {
        if (!send_image) return;
        if (socket->bytesToWrite()==0)
        socket->write(ba);
        socket->flush();
    }

};

class client{
public:
    QVector<Ball*> *b;//ez nem a shared-ra pointer.mert akkor az elemek kétszer kerülnének be.
    QVector<QTcpSocket *> socket;
    Ball * ownball;
    qreal last_sent_x,last_sent_y;
    QImage * canvas;
    client_settings s;
    int mouse_pos;
    int k1_pos;
    int k2_pos;
    LocalBalllController * mouse_cont;
    LocalKeyBallController * k1,*k2;
    client(client_settings cls,QVector<Ball*> *a,QImage * canvas):b(0),canvas(canvas)
    {
        b=new QVector<Ball*>();
        mouse_pos=-1;
        k1_pos=-1;
        k2_pos=-1;
        s=cls;
        qDebug()<<"host"<<cls.host;
        bool need_image=cls.need_image;
#define add_socket(flag,pos) if (flag) {b->push_back(new Ball(QPointF()));socket.push_back(new QTcpSocket() );pos=socket.size()-1;socket.last()->connectToHost(cls.host,12345);quint8 need_image_int;if (need_image) {quint8 need_image_int=1;need_image=false;}else {} socket.last()->write((char*)&need_image_int,1); }
        add_socket(cls.keyboard1,k1_pos);
        add_socket(cls.keyboard2,k2_pos);
        add_socket(cls.mouse,mouse_pos);
#undef add_socket
        qDebug()<<"mouse:"<<cls.mouse;
        qDebug()<<"cls k1:"<<cls.keyboard1;
        qDebug()<<"cls k2:"<<cls.keyboard2;
        if (mouse_pos!=-1)
        {mouse_cont=new LocalBalllController();
         mouse_cont->setBall(b->operator [](mouse_pos));
        }
        if (k1_pos!=-1)
        {
            k1=new LocalKeyBallController(65, //a
                                          87 ,//w
                                          68 ,//d
                                          83 );//s
            k1->setBall(b->operator [](k1_pos));
        }
        if (k2_pos!=-1)
        {
            k2=new LocalKeyBallController(16777234 ,
                                          16777235 ,
                                          16777236 ,
                                          16777237 );
            k2->setBall(b->operator [](k2_pos));
        }
    }
    void update()
    {
        while (socket[0]->bytesAvailable()>0)
        {
         //   qDebug()<<"aval:"<<socket->bytesAvailable();
//            QDataStream st(socket);
  //          st>>*canvas;
            readimage(socket[0],canvas);
        }
        for(int i=0;i<qMin(b->size(),socket.size());i++)
        {
        socket[i]->write(b->operator [](i)->toarray());
        //qDebug()<<        b->operator [](i)->direction.rx()<<" "<<b->operator [](i)->direction.ry();
        b->operator [](i)->direction.rx()=0;
        b->operator [](i)->direction.ry()=0;
        }
    }
    void mouseMoveEvent(QMouseEvent *e)
    {
       if (mouse_pos>=0)
           mouse_cont->process(e);
    }
    virtual void keyPressEvent(QKeyEvent *e)
    {
        if (k1_pos>=0)
            k1->process(e);
        if (k2_pos>=0)
            k2->process(e);
        //cli->keyPressEvent(e);
    }
    virtual void keyReleaseEvent(QKeyEvent *e)
    {
        if (k1_pos>=0)
            k1->r(e);
        if (k2_pos>=0)
            k2->r(e);
        //cli->keyPressEvent(e);
    }


};

class server{
public:
    AlapMap jatekmap;
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

    void update()
    {
        if (tcpServer->hasPendingConnections() )
        {
            qDebug()<<"becsatlakozo";
            Ball * newb=jatekmap.new_user();
            if (newb)
            {
            RemoteBallController * r=new RemoteBallController(tcpServer->nextPendingConnection());
            r->setBall(newb);
            b->push_back(newb);
            remote.push_back(r);
            }

        }

        foreach (RemoteBallController *r,remote)
        {
            r->update();
        }//mindenkitol bekertem a valtoztatasokat

        //itt lehet elmentni az elozo allapotot,h hogy nezett ki a canvas ha kesobb kell diffeleshez.
    //    gameplay_logic();
        QPainter p(canvas);
    //    update_canvas(p);
        jatekmap.process(*b,p);
        QByteArray ba=imagetobytearray(canvas);

       //most pedig mindenkinek elkuldom a valtoztatasokat.
        qint32 i=0;
        foreach (RemoteBallController *r,remote)
        {
            r->b->move();
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
    QVector<Ball*> *remoteballs;//valszeg nem kell
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
        QStringList arglist;
        for (int i=1;i<argc;i++)
        {
            arglist.push_back(argv[i]);
        }
        QString type;
        if (arglist.size()==0)
            arglist.push_back("server");//server,no beállítások,ha nincs semmi paraméter.
        type=arglist.takeFirst();
        bool valid_argument=false;
        if (type=="server")
        {s=new server(remoteballs,gamecanvas);
            valid_argument=true;
}
        if ( (type=="client" && arglist.size()>1) ||
             ( type=="server" && arglist.size()>0 ) )
        {
            valid_argument=true;
            client_settings cli_set;
            cli_set.mouse=false;cli_set.keyboard1=false;cli_set.keyboard2=false;
            if (type=="server")
                cli_set.host="localhost";
                else
                cli_set.host=arglist.takeFirst();
            for (int i=0;i<arglist.size();i++)
            {
                bool need_image=false;
/*#define check_argument(key,variable)                 if (arglist[i]==key)  {qDebug()<<key<<__LINE__;variable=true;need_image=true;}
                check_argument("k1",cli_set.keyboard1);
                check_argument("k2",cli_set.keyboard2);
                check_argument("mouse",cli_set.mouse);
#undef check_argument
*/
                if (arglist[i]=="k1")  {cli_set.keyboard1=true;need_image=true;}
                if (arglist[i]=="k2")  {cli_set.keyboard2=true;need_image=true;}
                if (arglist[i]=="mouse")  {cli_set.mouse=true;need_image=true;}
                if (need_image)
                    cli_set.need_image=true;
            }
            //FIXME:egyelore mindig true lesz a need_image.az h melyik socketen kerek kepoeet azt majd egy lentebbi szint fogja kitalalni.
            cli=new client(cli_set,remoteballs,gamecanvas);
        }
        else if (!valid_argument || (type=="client") ) {
            qFatal("hibas parameterezes!!!parameterezes:app_name [server|client] [hostname kliensnel] k1,k2,mouse.amelyiket kered olyan kontrollered lesz.nem lehet olyat indítani ahol egyiket sem kered.");
        }
        m.unlock();
        start(width,height);
                QWidget::startTimer(8);
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

    void paintEvent(QPaintEvent *e){//kulonvenni.
      //  qDebug()<<"paintevent begin";

        QPainter p2(this);
            if (cli)
                p2.drawImage(QPoint(),*gamecanvas);
        p2.end();
      //  qDebug()<<"paintevent end";
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
            cli->mouseMoveEvent(e);
        update();
        m.unlock();
    }
    void mousePressEvent(QMouseEvent *){
        update();
    }
    virtual void keyPressEvent(QKeyEvent *e)
    {

     qDebug()<<e->key();
        if (cli)
          cli->keyPressEvent(e);
        if (e->text()=="q")
        exit(0);
    }
    virtual void keyReleaseEvent(QKeyEvent *e)
    {
        if (cli)
          cli->keyReleaseEvent(e);
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
        qreal radius=ballsize/2.;
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
//        if (vn > 0.0f) return;

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

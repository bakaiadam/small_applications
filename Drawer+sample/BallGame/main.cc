#include <../drawer.hh>
//#include <chipmunk/chipmunk.h>
#include <chipmunk_private.h>
#define ballsize 10
#define w 800
#define h 600
#define border 100

#include <QTcpSocket>
#include <QTcpServer>
//this game will be like labirinth  lite for mobile phones.


enum CollisionType{
    BALL=1,
    RESETOBJ=2,
    DESTOBJ=3,
    SIMPLEOBJ=4,
    WALL=5,
    UPPERLIMIT=6//mindig kell egy upperlimit amit nem hasznal mas.
};


cpVect qpointtocpvect(const QPoint &p)
{
    return cpv((float)p.x(),(float)p.y());
}

int collision(cpArbiter *arb, cpSpace *mainSpace,
                                void *ignore);
int nocollision(cpArbiter *arb, cpSpace *mainSpace,
                                void *ignore);
int callbackcollision(cpArbiter *arb, cpSpace *mainSpace,
                                void *ignore);
void
drawShape(cpShape *shape, void *painter_pointer);//FIXME:ebbol tenyleg lehetne eleg konnyen valami qbytearray-t csinalni es azt kuldeni,csak akkor el lesz festve a kep kuldes lehetosege.meg az nem lenne annyiraflexibilis,viszont sokkal gyorsabb.

class controller_settings{
public:
    bool needed;
    int sensitivity;
};

class client_settings{
public:
    QString host;
    controller_settings keyboard1;
    controller_settings keyboard2;
    controller_settings mouse;
    bool need_image;
};

class Ball;

class GameMap;
class ShapeData{
public:
    GameMap * map;
    cpShape * shape;
    QColor color;//TODO:esetleg hasznos gettereket tenni bele.pl a collision typera.
    CollisionType get_col(){return (CollisionType)(shape->collision_type);}
    int index;
};

/**
ennek az osnek a celja az h ilyenekbe legyenek benne mindig a jatekszabalyok.egeszen a pattogastol kezdve a pontszamolasig minden.*/
class GameMap{
   public:
        virtual void process(QVector<Ball*> &balls, QPainter &p)=0;//ez rajzol
    virtual Ball * new_user()=0;//ez ad uj user-t.azert van szukseg a kihirdetesere,mert ez lesz majd kivulrol is allogatva a halozatrol.sot,teknikailag belulrol nem is fogunk foglalkozni vele,mert a kivulrol allogatas ra van kotve h a belso allapotat allogassa.
    virtual int collision(ShapeData* a,ShapeData *b)=0;//a visszateresi ertek mondja meg h legyn-e utkozes.
};

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

class AlapMap;
class Ball
{
public:
    qint32 points;
    qint32 &getlap(){return points;}
    bool start;
    const QPointF startpos;
    cpSpace *space;
    cpBody * body;
    QColor color;
    GameMap * a;
    cpShape * shape;
    Ball()//ha így hivod akkor kliensbol vagy cska azert hasznalod h kenyelmesen le legyenek kezelve a direction-ök.
    {
      space=0;
      body=0;
    }

    void reset_pos()
    {
        cpBodySetPos(body,cpv((int)startpos.x(),(int)startpos.y()));
        cpBodySetVel(body,cpv(0,0));
    }

/*    void in_dest(int index)
    {
        a->in_dest(this,index);
    }
*/
    Ball(QPointF f,cpSpace *space,QColor color,GameMap * a):pos(f),startpos(f),color(color),a(a)
    {

      this->space=space;

      cpFloat radius = 5;
      cpFloat mass = 1;
      cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);

      body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
      cpBodySetPos(body, cpv(pos.x(), pos.y()));

      shape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
      shape->collision_type=1;//1-es a balltype,2-es azok amelyekhez ha hozzaersz ujrakezdodik a jatek.
      cpShapeSetElasticity(shape, 0.5f);
      cpShapeSetFriction(shape, 0.0f);
      ShapeData * s=new ShapeData;
      s->map=a;
      s->color=color;
      qDebug()<<s->color.red()<<s->color.green()<<s->color.blue();
      s->shape=shape;
      shape->data=s;

        points=0;
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

        msg.append (  (char *)&points,4 );

        return msg;
    }

    QPointF prev_mouse_pos;
    QPointF direction;
    QPointF pos;
};

#include <QDateTime>
int random_int()
{
    static bool first_run=true;
    if (first_run)
    {
        qsrand(QDateTime::currentDateTime().toTime_t());
        first_run=false;
    }
    return qrand();
}

class AlapMap:public GameMap
{//van benne pattogas.
    QVector<int> pontok;
    QPoint elso,masodik;
    QVector<Ball *> balls;
//    QRegion tilos_helyek;

//    QVector<cpBody*> bodies;
    QVector<cpBody*> bodies_for_update;

    QVector<QPoint> dest_points;
    cpSpace *space;
    cpFloat timeStep;
public:
    void add_walls()
    {
        QPoint left_top(0,0);
        QPoint right_bottom(w,h);
        QVector<QPoint> corner;
        corner.push_back(left_top);
        corner.push_back(QPoint(right_bottom.x(),left_top.y()));
        corner.push_back(right_bottom);
        corner.push_back(QPoint(left_top.x(),right_bottom.y()));
        for (int i=0;i<4;i++)
        {
            cpShape *ground = cpSegmentShapeNew(space->staticBody, cpv((float)(corner[i].x()),
                                                                       (float)corner[i].y())
                                                , cpv((float)corner[(i+1)%4].x(), (float)corner[(i+1)%4].y()), 0);
              cpShapeSetFriction(ground, 0.0);
            cpShapeSetElasticity(ground, 1.0f);
            cpSpaceAddShape(space, ground);
            ShapeData * s=new ShapeData;
            s->map=this;
            s->color=QColor::fromRgb(0,0,0);
            s->shape=ground;
            ground->data=s;
        }
    }
void add_dest_points()
{
    dest_points.push_back(elso);//valszeg nem itt hanem a hívójában kéne inicializálni
    dest_points.push_back(masodik);
    int i=0;
    foreach(QPoint p,dest_points)
    {
    cpFloat radius = 50;
    cpFloat mass = 0.01;
    cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);

    cpBody * body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
    cpBodySetPos(body, qpointtocpvect(p));

    cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(body, radius, cpvzero));
    ballShape->collision_type=1;//1-es a balltype,2-es azok amelyekhez ha hozzaersz ujrakezdodik a jatek.
    cpShapeSetElasticity(ballShape, 0.5f);
    cpShapeSetFriction(ballShape, 0.0f);
    ballShape->collision_type=3;
    ShapeData * s=new ShapeData;
    s->map=this;
    s->color=QColor::fromRgb(120,120,120);
    s->shape=ballShape;
    s->index=i;
    i++;
    ballShape->data=s;
    }
}
    void add_pos_reset_bodies()//ezek azok amelyknek a collision id-je 2
{
        cpBody *porgobody;

        cpShape *shape;

        int reset_bodies_num=10;
        for (int i=0;i<reset_bodies_num;i++){
        // We create an infinite mass rogue body to attach the line segments too
        // This way we can control the rotation however we want.
        porgobody = cpBodyNew(INFINITY, INFINITY);
        cpBodySetPos(porgobody,cpv(random_int()%(w-200)+100,
                                   random_int()%(h-200)+100
                                   ));
        cpBodySetAngVel(porgobody, (random_int()%1000)/100.0-5.0 );
        // Set up the static box.
        float angle=((float)(random_int()%720))/180.0*M_PI;
        float length=(random_int()%20)*7+20;
        cpVect a = cpv(sin(angle)*length, length*cos(angle) );
        //        cpVect b = cpv(a.y, -1.0*a.x);
        cpVect b = cpv(sin(angle+M_PI)*length, length*cos(angle+M_PI) );

        shape = cpSpaceAddShape(space, cpSegmentShapeNew(porgobody, a, b, 0.0f));
        cpShapeSetElasticity(shape, 1.0f);
        cpShapeSetFriction(shape, 1.0f);
        shape->collision_type=2;

        ShapeData * s=new ShapeData;
        s->map=this;
        s->color=QColor::fromRgb(0,120,120);
        s->shape=shape;
        shape->data=s;

        bodies_for_update.push_back(porgobody);
        }
    }
    void add_simple_bodies()
    {
            float sw=20;
            float sh=10;
            cpVect theVerts[] ={
            cpv( - sw, - sh),
            cpv( - sw,   sh),
            cpv(   sw,   sh),
            cpv(   sw, - sh)
            };
            float mass=0.2;
    //        cpSpaceAddShape(space, cpPolyShapeNew(cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForPoly(mass, 4, theVerts, cpv(100,100)))) , 4, theVerts, cpv(100,100)));
            cpShape * shape=cpPolyShapeNew( space->staticBody, 4, theVerts, cpv(100,100));
            cpSpaceAddShape(space, shape);

            ShapeData * s=new ShapeData;
            s->map=this;
            s->color=QColor::fromRgb(0,0,120);
            s->shape=shape;
            shape->data=s;

    }

    AlapMap()
    {
        //http://files.slembcke.net/chipmunk/release/ChipmunkLatest-Docs/#cpShape
      //http://chipmunk-physics.net/release/ChipmunkLatest-Docs/#cpShape
        /** space:melyik térben zajolik a dolog,a két nulla pedig a z h milyen collision_id-ju dolgok összeütkörzésénél történjen valami.a mi esetünkben mivel senkinek sem lett beállítva érték,ezért mindegyiknek az értéke nulla,így most minden esetben kiírja azt h collision.*/

        int dist_from_border=40;
                elso=QPoint(dist_from_border,dist_from_border);
                masodik=QPoint(w-dist_from_border,h-dist_from_border);


        timeStep = 1.0/60.0;
        cpVect gravity = cpv(0, 0);
        space = cpSpaceNew();
        cpSpaceSetGravity(space, gravity);

add_walls();//maybe there should be an add_standing_static function,that contains add_walls
add_pos_reset_bodies();
add_simple_bodies();//non static objects.
add_dest_points();
//1es:labda,2:es olyan dolog ami visszadob,3as:celok
for (int q=0;q<UPPERLIMIT;q++)
    for (int qq=q;qq<UPPERLIMIT;qq++)
        cpSpaceAddCollisionHandler(space,q,qq,callbackcollision,0,0,0,0);
    }

    Ball * new_user()
    {
        Ball * new_ball=0;
        if (balls.size()==0)
{            new_ball=new Ball(elso,space,QColor::fromHsv(0,255,255),this );
            ((ShapeData*)new_ball->shape->data)->index=0;
 }       if (balls.size()==1)
  {          new_ball=new Ball(masodik,space,QColor::fromHsv(180,255,255),this );
            ((ShapeData*)new_ball->shape->data)->index=1;
        }     if (new_ball)
            balls.push_back(new_ball);
        return new_ball;
    }

    void gameplay_logic(QVector<Ball*> &b)
    {
      cpSpaceStep(space, timeStep);
    }

    void update_canvas(QVector<Ball*> &balls,QPainter &p)
    {//a balls parameter mar nem is kell.
        cpFloat dt = 1.0f/60.0f;
        foreach(cpBody *b,bodies_for_update)
            cpBodyUpdatePosition(b, dt);


        p.fillRect(QRect(0,0,w,h),Qt::white);
        p.setBrush(QBrush(QColor::fromRgb(0,0,0) ) );
        cpSpaceEachShape(space, drawShape, &p);

        int i=0;
        foreach (Ball *b,balls)
                   {
                       p.setBrush(QBrush(QColor::fromHsv(i*360/balls.size()+1,255,255) ));
                       p.drawText(i*w/balls.size(),10,QString::number(b->points ) );
                   i++;
                   }
                p.end();
    }

     virtual void process(QVector<Ball*> &balls, QPainter &p)
     {//FIXME:a balls ref valszeg tök felesleges.
        while (pontok.size()<balls.size())
            pontok.push_back(0);
           gameplay_logic(balls);
           update_canvas(balls,p);
     }

    int collision(ShapeData* a,ShapeData *b)
    {
#define paircond(w,e) (a->get_col()==e && b->get_col()==w) || (b->get_col()==e && a->get_col()==w)
#define get_index(q) (a->get_col()==q?a->index:b->index)
        if ( paircond(BALL,DESTOBJ) )
        {
            if (a->index!=b->index)
            {
            int idx=get_index(BALL);
                balls[idx]->points++;
                balls[idx]->reset_pos();
            }
        }
        if (paircond(RESETOBJ,BALL))
        {
            int idx=get_index(BALL);
            balls[idx]->reset_pos();
        }
        if (a->get_col()==DESTOBJ || b->get_col()==DESTOBJ)
            return FALSE;
        return TRUE;
    }

};


class WrestlingMap:public GameMap
{
    QVector<int> pontok;
    QPoint elso,masodik;
    QVector<Ball *> balls;
//    QRegion tilos_helyek;

//    QVector<cpBody*> bodies;
    QVector<cpBody*> bodies_for_update;

    QVector<QPoint> dest_points;
    cpSpace *space;
    cpFloat timeStep;
public:
    void add_walls()
    {
        QPoint left_top(0,0);
        QPoint right_bottom(w,h);
        QVector<QPoint> corner;
        corner.push_back(left_top);
        corner.push_back(QPoint(right_bottom.x(),left_top.y()));
        corner.push_back(right_bottom);
        corner.push_back(QPoint(left_top.x(),right_bottom.y()));
        for (int i=0;i<4;i++)
        {
            cpShape *ground = cpSegmentShapeNew(space->staticBody, cpv((float)(corner[i].x()),
                                                                       (float)corner[i].y())
                                                , cpv((float)corner[(i+1)%4].x(), (float)corner[(i+1)%4].y()), 0);
              cpShapeSetFriction(ground, 0.0);
            cpShapeSetElasticity(ground, 1.0f);
            cpSpaceAddShape(space, ground);
            ShapeData * s=new ShapeData;
            s->map=this;
            s->color=QColor::fromRgb(0,0,0);
            s->shape=ground;
            ground->collision_type=RESETOBJ;
            ground->data=s;
        }
    }
    void add_simple_bodies()
    {
            float sw=20;
            float sh=10;
            cpVect theVerts[] ={
            cpv( - sw, - sh),
            cpv( - sw,   sh),
            cpv(   sw,   sh),
            cpv(   sw, - sh)
            };
            float mass=0.2;
    //        cpSpaceAddShape(space, cpPolyShapeNew(cpSpaceAddBody(space, cpBodyNew(mass, cpMomentForPoly(mass, 4, theVerts, cpv(100,100)))) , 4, theVerts, cpv(100,100)));
            cpShape * shape=cpPolyShapeNew( space->staticBody, 4, theVerts, cpv(100,100));
            cpSpaceAddShape(space, shape);

            ShapeData * s=new ShapeData;
            s->map=this;
            s->color=QColor::fromRgb(0,0,120);
            s->shape=shape;
            shape->data=s;

    }

    WrestlingMap()
    {
        //http://files.slembcke.net/chipmunk/release/ChipmunkLatest-Docs/#cpShape
      //http://chipmunk-physics.net/release/ChipmunkLatest-Docs/#cpShape
        /** space:melyik térben zajolik a dolog,a két nulla pedig a z h milyen collision_id-ju dolgok összeütkörzésénél történjen valami.a mi esetünkben mivel senkinek sem lett beállítva érték,ezért mindegyiknek az értéke nulla,így most minden esetben kiírja azt h collision.*/

        int dist_from_border=40;
                elso=QPoint(dist_from_border,dist_from_border);
                masodik=QPoint(w-dist_from_border,h-dist_from_border);


        timeStep = 1.0/60.0;
        cpVect gravity = cpv(0, 0);
        space = cpSpaceNew();
        cpSpaceSetGravity(space, gravity);

add_walls();//maybe there should be an add_standing_static function,that contains add_walls
//add_pos_reset_bodies();
//add_simple_bodies();//non static objects.
//add_dest_points();
//1es:labda,2:es olyan dolog ami visszadob,3as:celok
for (int q=0;q<UPPERLIMIT;q++)
    for (int qq=q;qq<UPPERLIMIT;qq++)
        cpSpaceAddCollisionHandler(space,q,qq,callbackcollision,0,0,0,0);
    }

    Ball * new_user()
    {
//      qDebug()<<(QColor::fromHsv(0,255,255));
        Ball * new_ball=0;
        if (balls.size()==0)
{            new_ball=new Ball(elso,space,QColor::fromHsv(0,255,255),this );
            ((ShapeData*)new_ball->shape->data)->index=0;
 }       if (balls.size()==1)
  {          new_ball=new Ball(masodik,space,QColor::fromHsv(180,255,255),this );
            ((ShapeData*)new_ball->shape->data)->index=1;
        }     if (new_ball)
            balls.push_back(new_ball);
        return new_ball;
    }

    void gameplay_logic(QVector<Ball*> &b)
    {
      cpSpaceStep(space, timeStep);
    }

    void update_canvas(QVector<Ball*> &balls,QPainter &p)
    {//a balls parameter mar nem is kell.
        cpFloat dt = 1.0f/60.0f;
        foreach(cpBody *b,bodies_for_update)
            cpBodyUpdatePosition(b, dt);


        p.fillRect(QRect(0,0,w,h),Qt::white);
        p.setBrush(QBrush(QColor::fromRgb(0,0,0) ) );
        cpSpaceEachShape(space, drawShape, &p);

        int i=0;
        foreach (Ball *b,balls)
                   {
                       p.setBrush(QBrush(QColor::fromHsv(i*360/balls.size()+1,255,255) ));
                       p.drawText(i*w/balls.size(),10,QString::number(b->points ) );
                   i++;
                   }
                p.end();
    }

     virtual void process(QVector<Ball*> &balls, QPainter &p)
     {//FIXME:a balls ref valszeg tök felesleges.
        while (pontok.size()<balls.size())
            pontok.push_back(0);
           gameplay_logic(balls);
           update_canvas(balls,p);
     }

    int collision(ShapeData* a,ShapeData *b)
    {//igaz,ha ütköznek.hamis ha átmennek egymáson.
      if (balls.size()==1) return TRUE;
#define paircond(w,e) (a->get_col()==e && b->get_col()==w) || (b->get_col()==e && a->get_col()==w)
#define get_index(q) (a->get_col()==q?a->index:b->index)
        if (paircond(RESETOBJ,BALL))
        {
            int idx=get_index(BALL);
            balls[ qMin ( (idx ^ 1) & 1,balls.size()-1) ]->points++;//FIXME:2-nél több játékosnál hogyan kapják a pontot?
            foreach(Ball * b, balls)
              {
                b->reset_pos();
              }
        }
        if (a->get_col()==DESTOBJ || b->get_col()==DESTOBJ)
            return FALSE;
        return TRUE;
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
    int sensitivity;
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
        b->add_move((2+sensitivity)*(QCursor::pos()-def_pos));
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
        move*=(70+sensitivity);
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
                qDebug()<<__LINE__<<send_image_int;
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
        qDebug()<<cls.keyboard1.needed<<" "<<cls.keyboard2.needed<<" "<<cls.mouse.needed;
        bool need_image=cls.need_image;
#define add_socket(flag,pos) if (flag) {b->push_back(new Ball());socket.push_back(new QTcpSocket() );pos=socket.size()-1;socket.last()->connectToHost(cls.host,12345);quint8 need_image_int;if (need_image) {quint8 need_image_int=1;need_image=false;}else {} socket.last()->write((char*)&need_image_int,1); }
        add_socket(cls.keyboard1.needed,k1_pos);
        add_socket(cls.keyboard2.needed,k2_pos);
        add_socket(cls.mouse.needed,mouse_pos);
#undef add_socket
        if (mouse_pos!=-1)
        {mouse_cont=new LocalBalllController();
         mouse_cont->setBall(b->operator [](mouse_pos));
         mouse_cont->sensitivity=cls.mouse.sensitivity;
        }
        if (k1_pos!=-1)
        {
            k1=new LocalKeyBallController(65, //a
                                          87 ,//w
                                          68 ,//d
                                          83 );//s
            k1->setBall(b->operator [](k1_pos));
            k1->sensitivity=cls.keyboard1.sensitivity;
//            qDebug()<<__LINE__<<k1->sensitivity;
        }
        if (k2_pos!=-1)
        {
            k2=new LocalKeyBallController(16777234 ,
                                          16777235 ,
                                          16777236 ,
                                          16777237 );
            k2->setBall(b->operator [](k2_pos));
            k2->sensitivity=cls.keyboard2.sensitivity;
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
    GameMap * jatekmap;
    QImage *canvas;
    QTcpServer * tcpServer;
    QVector<Ball*> *b;
    QVector<RemoteBallController*> remote;
    QVector<int> pos;
    server(QVector<Ball*> *remoteballs,QImage * c):b(remoteballs),canvas(c)
    {
        //jatekmap=new AlapMap;
      jatekmap=new WrestlingMap;
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
            Ball * newb=jatekmap->new_user();
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
        jatekmap->process(*b,p);
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
            cli_set.mouse.needed=false;cli_set.keyboard1.needed=false;cli_set.keyboard2.needed=false;
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
                if (arglist[i]=="k1") update_controller_settings(cli_set.keyboard1,arglist,i,need_image);
                if (arglist[i]=="k2") update_controller_settings(cli_set.keyboard2,arglist,i,need_image);
                if (arglist[i]=="mouse") update_controller_settings(cli_set.mouse,arglist,i,need_image);
                if (need_image)
                    cli_set.need_image=true;
            }
            if (arglist.contains("no_need_image"))
                cli_set.need_image=false;
            //FIXME:egyelore mindig true lesz a need_image.az h melyik socketen kerek kepoeet azt majd egy lentebbi szint fogja kitalalni.
            cli=new client(cli_set,remoteballs,gamecanvas);
        }
        else if (!valid_argument || (type=="client") ) {
            qFatal("hibas parameterezes!!!parameterezes:app_name [server|client] [hostname kliensnel] k1,k2,mouse.amelyiket kered olyan kontrollered lesz.kliensnel nem lehet olyat indítani ahol egyiket sem kered.szervernel ha nem kersz semmilyen kontrolt,akkor csak spectator leszel.minden kontroller utan lehet irni szamot,h mennyi legyen az erzekenysege.lehet pozitiv es negativ is.");
        }
        m.unlock();
        start(width,height);
                QWidget::startTimer(8);
    }
    void static update_controller_settings(controller_settings &c,const QStringList &arglist,int index,bool &need_image)
    {
        c.needed=true;
        c.sensitivity=0;
        int i=index;
        if (i+1>=arglist.size()) return;
                            bool ok;
                            int sensitivity=arglist[i+1].toInt(&ok);
                            if (ok) c.sensitivity=sensitivity;
                            need_image=true;
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

int nocollision(cpArbiter *arb, cpSpace *mainSpace,
                                void *ignore) {
return FALSE;
}
int callbackcollision(cpArbiter *arb, cpSpace *mainSpace,
                                void *ignore) {
//return FALSE;
    CP_ARBITER_GET_SHAPES(arb, a, b);
/*    Ball * ba=0;
    int dest_index;
    if (a->collision_type==1)
    {
        ba=(Ball*)a->data;
        dest_index=*((int*)b->data);
    }
    else
{        ba=(Ball*)b->data;
         dest_index=*((int*)a->data);
    }
*/    return ((ShapeData*)(a->data))->map->collision((ShapeData*)a->data,(ShapeData*)b->data);
//    ba->a->in_dest(ba,dest_index);
//    return FALSE;
}
int collision(cpArbiter *arb, cpSpace *mainSpace,
                                void *ignore) {

    // use this macro to declare two shapes and call
    // cpArbiterGetShapes which uses the cpArbiter
    // which contains info on the collision such as
    // the shapes involved, contacts, normals, etc.
    CP_ARBITER_GET_SHAPES(arb, a, b);

    Ball * ba=(Ball*)a->data;
    if (ba) ba->reset_pos();
    ba=(Ball*)b->data;
    if (ba) ba->reset_pos();


    // use the data property of the cpShape to pull
    // out your custom data object
    //SpaceData * shapeDataA = a->data;
    //SpaceData * shapeDataB = b->data;
    qDebug()<<"collision\n";
    return TRUE;
//    if ( /* a should collide with b */ ) {
//        return TRUE;
//    } else if ( /* a should avoid b */ ) {
//        return FALSE;
//    }
}

void
drawShape(cpShape *shape, void *painter_pointer)//FIXME:ebbol tenyleg lehetne eleg konnyen valami qbytearray-t csinalni es azt kuldeni,csak akkor el lesz festve a kep kuldes lehetosege.meg az nem lenne annyiraflexibilis,viszont sokkal gyorsabb.
{
#define setcolor_do_undo(color,action) { \
    QBrush b_orig=p.brush();\
    QPen p_orig=p.pen();\
    p.setPen(color);\
    p.setBrush(color);\
    action \
    p.setBrush(b_orig);\
    p.setPen(p_orig);\
    }

    QPainter &p=*((QPainter*)painter_pointer);
    cpBody *body = shape->body;
    //Color color = ColorForShape(shape);

    switch(shape->klass->type){
        case CP_CIRCLE_SHAPE: {
            cpCircleShape *circle = (cpCircleShape *)shape;
            //ChipmunkDebugDrawCircle(circle->tc, body->a, circle->r, LINE_COLOR, color);
            //qDebug()<<"x y"<<(int)circle->tc.x<<(int)circle->tc.y;
            QColor paint_color;
            int r=(int)circle->r;
            if (shape->collision_type==3)
                paint_color=QColor::fromRgb(100,100,255);
                else
                paint_color=( (ShapeData*)((Ball*)shape->data) )->color;
            //qDebug()<<__LINE__<<circle->tc.x<<circle->tc.y<<paint_color.red()<<paint_color.green()<<paint_color.blue()<<r;
            //paint_color=QColor::fromRgb(0,0,0);
           // p.setBrush(paint_color);
           // p.setPen(paint_color);
//p.drawChord((int)circle->tc.x-r,(int)circle->tc.y-r,r*2,r*2,0,5760);
            setcolor_do_undo(paint_color,p.drawChord((int)circle->tc.x-r,(int)circle->tc.y-r,r*2,r*2,0,5760););
            break;
        }
        case CP_SEGMENT_SHAPE: {
            cpSegmentShape *seg = (cpSegmentShape *)shape;
#define draw p.drawLine(QPoint(seg->ta.x,seg->ta.y),QPoint(seg->tb.x,seg->tb.y));
            if (shape->collision_type==2)
                setcolor_do_undo(QColor::fromHsv(270,255,255),draw;)
                else
                draw;
#undef draw
        //ChipmunkDebugDrawFatSegment(seg->ta, seg->tb, seg->r, LINE_COLOR, color);
            break;
        }
        case CP_POLY_SHAPE: {
            cpPolyShape *poly = (cpPolyShape *)shape;
            QPoint * points=new QPoint[poly->numVerts];
            for (int i=0;i<poly->numVerts;i++)
            {
                points[i].rx()=poly->tVerts[i].x;
                points[i].ry()=poly->tVerts[i].y;
                //qDebug()<<points->rx()<<" "<<points->ry();
            }
#define draw             p.drawPolygon(points,poly->numVerts);
            if (shape->collision_type==2)
                setcolor_do_undo(QColor::fromHsv(270,255,255),draw)
            else
                draw;
#undef draw

//            ChipmunkDebugDrawPolygon(poly->numVerts, poly->tVerts, LINE_COLOR, color);
            delete points;
            break;
        }
        default:
        qDebug()<<shape->klass->type;
        break;
    }
}

#include <../drawer.hh>

#include <QTcpSocket>
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

    QPointF getpos()
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

class Drawer2: public Drawer
{
public:
    QTimer *timer;
public:
    QVector<Ball*> remoteballs;
    LocalBalllController c;
    Drawer2()
    {
        Ball *a=new Ball(QPointF(300,300));
        remoteballs.push_back(a);
        c.setBall(a);
         QWidget::startTimer(10);
    }
    void timerEvent(QTimerEvent *e){
        //b.add_move(QPointF());
//        b.move();
        foreach(Ball * ab,remoteballs)
        {
           // qDebug()<<ab;
            ab->move();
        }
        update();
    }

    void paintEvent(QPaintEvent *){//kulonvenni.
        QPainter p(this);
        foreach(Ball * b,remoteballs)
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
        exit(0);
    }

    
};


int main()
{
    Drawer2 d;
    //d.start(800,600);
    d.start();
}

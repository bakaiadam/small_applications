#include <../drawer.hh>

//this game will be like labirinth  lite for mobile phones.

class Ball
{
public:
    bool start;
    Ball(QPointF f):pos(f)
    {
        start=true;
    }

    QPointF getpos()
    {
        return pos;
    }
    void add_move(QPointF r)
    {
        r=QCursor::pos();
        QCursor::setPos(QPoint(300,300));
        if (start)
{            prev_mouse_pos=r;start=false;}
        QPointF prev_mouse_pos2=r;
        //r=prev_mouse_pos-r;
        r=r-prev_mouse_pos;
        qDebug()<<r;
        direction+=r;
        qDebug()<<pos;
//        pos=prev_mouse_pos;
        //prev_mouse_pos=prev_mouse_pos2;
        prev_mouse_pos=QPointF(300,300);
        
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

class Drawer2: public Drawer
{
public:
    QTimer *timer;
public slots:
    void timerTimeout()
    {
        b.add_move(QPointF());
        update();        
    }
public:    
    Ball b;
    Drawer2():b(QPointF(300,300) )
    {
        timer = new QTimer((QWidget*)this);
           timer->setInterval(10);
     //   connect(timer, SIGNAL(timeout()), (QWidget*)this, SLOT(timerTimeout()));
    QWidget::startTimer(10);
    }
    void timerEvent(QTimerEvent *e){
        //b.add_move(QPointF());
        b.move();
        update();
    }

    void paintEvent(QPaintEvent *){
        QPainter p(this);
        p.drawArc(b.getpos().rx(),b.getpos().ry(),100,100,0,5760);
        //p.drawLine(0,0,300,300);
        printf("e\n");
    }
    
    void mouseMoveEvent(QMouseEvent *e)
    {//pos az elozohoz kepest?
        
        b.add_move(e->posF());
        update();
    }
    void mousePressEvent(QMouseEvent *){
        update();
    }
    virtual void keyPressEvent(QKeyEvent *e)
    {
        qDebug()<<e->text();
     //   return;
        QPointF f;
        if (e->text()=="d")
            f=QPointF(1,0);
        if (e->text()=="a")
            f=QPointF(-1,0);
        if (e->text()=="w")
            f=QPointF(0,-1);
        if (e->text()=="s")
            f=QPointF(0,1);
        b.add_move(f);
        update();
        qDebug()<<e->text();
        exit(0);
    }

    
};


int main()
{
    Drawer2 d;
    //d.start(800,600);
    d.start();
}
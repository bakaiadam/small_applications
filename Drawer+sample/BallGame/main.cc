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
            char f[500];
            socket->readLine(f,500);
            qDebug()<<QString(f);
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
        }
    }

};

class Drawer2: public Drawer
{
public:
    QTimer *timer;
public:
    server s;
    QVector<Ball*> remoteballs;
    LocalBalllController c;
public slots:
    void new_connect()
    {
        qDebug()<<"a";
    }
public:
    Drawer2():remoteballs(),s(&remoteballs)
    {
        Ball *a=new Ball(QPointF(300,300));
        remoteballs.push_back(a);
        c.setBall(a);
        QWidget::startTimer(10);
    }
    void timerEvent(QTimerEvent *e){
        //b.add_move(QPointF());
//        b.move();
        s.update();
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
        if (e->text()=="q")
        exit(0);
    }

    
};


int main()
{
    Drawer2 d;
    d.start(800,600);
//    d.start();
}


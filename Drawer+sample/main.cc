#include<vector>
#include <map>
#include <math.h>
#include <iostream>
#include <drawer.hh>
using namespace std;

class Drawer2: public Drawer
{
public:
    void paintEvent(QPaintEvent *){
        QPainter p(this);
        p.drawLine(0,0,300,300);
        printf("e\n");
    }
    void mousePressEvent(QMouseEvent *){
        printf("a\n");
        update();
    }
};


/**
  *simple immutable vector(i like immutable classes),with only two parameters,so its starting from (0,0)
  */
class vector2d{
public:
    const float x,y;
    vector2d(float x,float y):x(x),y(y)
    {
        
    }
    float length() const
    {
        return sqrt(x*x+y*y);
    }

    vector2d normalize()
    {
        float l=length();
        return vector2d(x/l,y/l);
    }

    float angle()
    {
        vector2d n=normalize();
       // cout<<"x:"<<n.x<<"\n";
        float ret=acos(n.x);//az arc cos vissza adja hogyha beteszem az xet,hogy mennyi az elfordulás a nullától.de csak akkor,ha y pozitív,ha negatív,akkor lefele kell nezni a megadott zzalekot.
        if (n.y<0)
            ret=2*M_PI-ret;
        //cout<<n.x<<" "<<n.y<<" "<<ret<<"\n";
        return ret;
    }
};
#include <stdio.h>
vector<int> convex_hull(const vector<pair<float,float> > p)
{//convex hull:vannak a síkon pontok.feladat ezne pontok halmazából egy olyan index listát csinálni,amiket ha összekötök egy konvex poligon jön ki,amiben benne van az összes elem.
    /*
      a következő algoritmusra gondoltam:
      elindulok a lehető magyobb y érték ponttal,és 0 fok irányába nézek.(0->jobb,90->fel,180->bal).csak egyebkent majd radian lesz hasznalva.
    ez a legkisebb y pont tuti benn lesz a pontok között.ezután fogom magam és megkeresem azt a pontot amihez a lehető legkisebb jobbra való elfordulás kell
    ez a pont lesz a következő ő lesz,felveszem és megint elvégzem vele is a  műveeletet.előbb utóbb körül érek,és örülök.
        
*/
    vector<int> ret;
    float maxy=p[0].second;
    float maxindex=0;
    for (int i=1;i<p.size();i++)
    {
        if (p[i].second>=maxy)
            if (p[i].second>maxy || p[maxindex].first<p[i].first ){//ha egyforma az y-nyuk,akkor a baloldalabbit valassza,h kov lepesben racuppanjon az eggyel jobbrabb levore.
            maxy=p[i].second;
            maxindex=i;
        }
    }
    float irany=0;
    float x=p[maxindex].first,y=p[maxindex].second;
    int aktindex=maxindex;
    ret.push_back(maxindex);
    if (p.size()!=1)
    while (ret[ret.size()-1]!=maxindex || ret.size()==1)//amíg nem erunk korbe.
    {
//        if (aktindex!=-1) printf("aktindex:%d\n",aktindex);
        float minangle=9999;//TODO normalis szamot
        int minindex=-1;
        int i;
        for (i=0;i<p.size();i++)//megkeresem azt amihez a legkisebbet kell elfordulnom balra.
            if (i!=aktindex)
            {
                //float a=vector2d(p[i].first-p[aktindex].first,p[i].second-p[aktindex].second).angle();
                float a=vector2d(p[aktindex].first-p[i].first,p[aktindex].second-p[i].second).angle();
        //        cout<<aktindex<<"->"<<i<<" "<<a<<"\n";                
                
                //printf("a:%d,i:%d,psize:%d\n",a,i,p.size());
                //cout<<p.size()<<"\n";
          //      cout<<a<<"\n";
                if (a<minangle && irany<=a)
//                if (a>minangle)
                               
                {
                    minangle=a;minindex=i;
                }
            }
      //  cout<<minindex<<"\n";
     
        //sleep(1);
//        printf("m:%d\n",minindex);
        ret.push_back(minindex);
        aktindex=minindex;
        irany=minangle;
    }
    return ret;
}

#include <time.h>
#include <math.h>

int randint()
{
    static bool first_run=true;
    if (first_run)
    {
        srand ( time(NULL) );
        first_run=false;
    }
    return rand();
}

float negyzet(float a){return a*a;}
float trianglearea(float a,float b,float c,float d,float e,float f)
{
    //http://en.wikipedia.org/wiki/Triangle
    float alen=sqrt( negyzet(a-c)+negyzet(b-d) );
    float blen=sqrt( negyzet(a-e)+negyzet(b-f) );
    float clen=sqrt( negyzet(c-e)+negyzet(d-f) );
    float szog=acos( (negyzet(alen)+negyzet(blen)-negyzet(clen) ) /(2*alen*blen) );
    //return 0.5*alen*blen*sin(szog);
    float s=(alen+blen+clen)/2.;
    return sqrt(s*(s-alen)*(s-blen)*(s-clen));
}

bool isinside(float ex,float ey,
              float a,float b,
              float c,float d,
              float e,float f)
{
//    return trianglearea(a,b,c,d,e,f)==trianglearea(a,b,c,d,ex,ey)+trianglearea(a,b,ex,ey,e,f)+
  //          trianglearea(ex,ey,c,d,e,f);//FIXME:lehet h tul sokszor fog false-ot visszaadni kerekitesi hiba miatt.
    return abs(trianglearea(a,b,c,d,e,f)-(trianglearea(a,b,c,d,ex,ey)+trianglearea(a,b,ex,ey,e,f)+
              trianglearea(ex,ey,c,d,e,f)) )<0.0000001 ;//FIXME:lehet h tul sokszor fog false-ot visszaadni kerekitesi hiba miatt.

}


float poly_cont(vector<pair<float,float> > inp,pair<float,float> point)
{//only for convex polygon
    
        for (int iii=1;iii+1<inp.size();iii++)
        if (isinside(point.first,point.second,
                     inp[0].first,inp[0].second,
                     inp[iii].first,inp[iii].second,
                     inp[iii+1].first,inp[iii+1].second)
            ) return true;
        return false;
}

float poly_area(vector<pair<float,float> > inp)
{
    float s=0;
    for (int iii=1;iii+1<inp.size();iii++)
        s+=trianglearea(inp[0].first,inp[0].second,inp[iii].first,inp[iii].second,inp[iii+1].first,inp[iii+1].second);
return s;    
}


class Drawer3: public Drawer
{
public:
    void paintEvent(QPaintEvent *){
        vector<pair<float,float> > inp;
        /*int pointcount=randint()%20+1;
        for (int i=0;i<pointcount;i++)
        inp.push_back(pair<float,float>(randint()%width(),randint()%height()));//TODO:random legyen.
        */
        int pointcount=12;
//        inp.push_back(pair<float,float>() );
        inp.push_back(pair<float,float>(5*20,7*20) );
        inp.push_back(pair<float,float>(3*20,3*20) );
        inp.push_back(pair<float,float>(4*20,6*20) );
        inp.push_back(pair<float,float>(4*20,11*20) );
        inp.push_back(pair<float,float>(4*20,8*20) );
        inp.push_back(pair<float,float>(10*20,6*20) );
        inp.push_back(pair<float,float>(6*20,6*20) );
        inp.push_back(pair<float,float>(6*20,3*20) );
        inp.push_back(pair<float,float>(7*20,9*20) );
        inp.push_back(pair<float,float>(10*20,4*20) );
        inp.push_back(pair<float,float>(10*20,9*20) );
        inp.push_back(pair<float,float>(1*20,7*20) );
        
        vector<int> c_h=convex_hull(inp);
        QPainter p(this);
        for (int i=0;i<inp.size();i++)
            //p.drawPoint(inp[i].first,inp[i].second);
{
            //p.drawLine(inp[i].first,inp[i].second-5,inp[i].first,inp[i].second+5);
            //p.drawLine(inp[i].first-5,inp[i].second,inp[i].first+5,inp[i].second);
            p.drawText(inp[i].first,inp[i].second,QString::number(i));
        }
        printf("%d\n",c_h.size());
        for (int i=0;i<c_h.size()-1;i++)
            //p.drawPoint(inp[i].first,inp[i].second);
        {          
            printf("%d\n",i);
            p.drawLine(inp[c_h[i]].first,inp[c_h[i]].second,inp[c_h[i+1]].first,inp[c_h[i+1]].second);
        }
        vector<pair<float,float> > poly;
        for (int i=0;i<c_h.size()-1;i++)
            poly.push_back(inp[c_h[i]]);
        
        for (int i=0;i<poly.size();i++)
         {
//            p.drawText(poly[i].first,poly[i].second,QString("poly"));
        }
        
        
        vector<pair<float,float> > inp2;
        pointcount=randint()%20+1;
        pointcount=8000;
        for (int i=0;i<pointcount;i++)
        {          
            printf("%d\n",i);
            inp2.push_back(pair<float,float>(randint()%width(),randint()%height()));//TODO:random legyen.
            //p.drawText(inp2[i].first,inp2[i].second,QString::number(i)+QString(poly_cont(poly,inp2[i])?"benn":"kinn"));
            if (!poly_cont(poly,inp2[i])) p.setPen(QColor(0,0,255));
            else
                p.setPen(QColor(255,255,0));
            p.drawPoint(inp2[i].first,inp2[i].second);
        }
        

        printf("e\n");
    }
    void mousePressEvent(QMouseEvent *){
        printf("a\n");
        update();
    }
    virtual void keyPressEvent(QKeyEvent *)
    {
        exit(0);
    }
};


int main()
{
    Drawer3 d;
    d.start(800,600);
    //d.start();
}

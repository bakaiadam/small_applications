#ifndef DRAWER_HH
#define DRAWER_HH
#include <QWidget>



/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>

#include <QMainWindow>

class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QSlider;

class GLWidget;

class MainWindow : public QMainWindow
{
    //FIXME?Q_OBJECT

public:
    MainWindow(int x,int y,QWidget* fWidget);

};


MainWindow::MainWindow(int x,int y,QWidget* fWidget)
{
    QWidget *centralWidget = new QWidget;
    setCentralWidget(centralWidget);
    
    QGridLayout *centralLayout = new QGridLayout;
    centralWidget->setLayout(centralLayout);
  centralLayout->addWidget(fWidget);

    setWindowTitle(tr("Grabber"));
    resize(x,y);
}




class Drawer:protected QApplication,public QWidget
{
    public:
    Drawer():QApplication(0,0),
      QWidget()
    {
        setFocusPolicy(Qt::StrongFocus);
        
    }

    //mindenki megirhatja a gyerekekben a ...event fv-eket.
    
    /**
      *ez egy blokkolo hivas,utana mar cska a hivogatasokkal meg timerekkel tudod kezelni a dolgot
      */
    void start()
    {
        QDesktopWidget * desk = qApp->desktop() ;
        int width = desk->width() ;
        int height = desk->height() ;
        MainWindow mainWin(width,height,this);
        mainWin.showFullScreen();
        mainWin.show();
        exec();
    }

    void start(int w,int h){
        
        MainWindow mainWin(w,h,this);
        mainWin.show();
        exec();
    }
    
};

#endif // DRAWER_HH
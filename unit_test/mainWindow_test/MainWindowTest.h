//
// Created by tdiallo on 07/08/24.
//

#ifndef MAINWINDOWTEST_H
#define MAINWINDOWTEST_H
#include <QTest>
#include "/home/tdiallo/CLionProjects/Projet_final/flasherapp/sources/jlink_command/JlinkCommand.h"



class MainWindowTest : public QObject
{
    Q_OBJECT
    private slots:
    void initTestCase();
    void cleanUpTestCase();

};



#endif //MAINWINDOWTEST_H

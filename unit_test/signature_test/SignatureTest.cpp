//
// Created by tdiallo on 06/08/24.
//

#include "SignatureTest.h"
#include <QObject>
#include <qtimer.h>
#include <QSignalSpy>
#include <QtTest/QtTest>
#include <QList>
#include <QMessageBox>
#include <QVBoxLayout>
#include <iostream>

void SignatureTest::initTestCase()
{
    std::cout<<"\n";
    qDebug() << "Initialisation de tous les tests...\n";
}





void SignatureTest::cleanupTestCase()
{
    std::cout<<"\n";
    qDebug() << "Effacement de tous les test...\n";
}


QTEST_MAIN(SignatureTest);


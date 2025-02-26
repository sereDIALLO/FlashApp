//
// Created by tdiallo on 06/08/24.
//

#include "SerialNumberTest.h"
#include "/home/tdiallo/CLionProjects/Projet_final/flasherapp/sources/serial_number/SerialNumber.h"
//Initialisation de tous les test
void SerialNumberTest::initTestCase()
{
    qDebug() << "Initialisation de tous les test...\n";
}
void SerialNumberTest::dateInt_should_currentDate()
{
    qDebug() << "date testing...\n";
    SerialNumber s;
    QCOMPARE(s.getCurrentDate(), (QDate::currentDate().toString("yyyyMMdd")).toInt());
}
void SerialNumberTest::dateStr_should_currentDate()
{
    qDebug() << "date str testing...\n";
    SerialNumber s;
    QCOMPARE(s.currentDate(), QDate::currentDate().toString("yyyy-MM-dd"));
}
void SerialNumberTest::time_should_currentTime()
{
    qDebug() << " time testing...\b";
    SerialNumber s;
    QCOMPARE(s.getTime(), QTime::currentTime().toString("HH:mm:ss"));
}
void SerialNumberTest::endCarec_should_p()
{
    qDebug() << "end character testing...\n";
    SerialNumber s;
    QCOMPARE(s.endCarac("biocorp"), 'p');
}
void SerialNumberTest::caracHex_should_1400000()
{
    qDebug() << "character hex testing...\n";
    SerialNumber s;
    QCOMPARE(s.caracHex('e'), 1400000);
}
void SerialNumberTest::timeStamp_should_currentStamp()
{
    qDebug() << "time Stamp testing...";
    SerialNumber s;
    QDateTime datetime = QDateTime::currentDateTimeUtc();
    int stamp = datetime.toSecsSinceEpoch();
    QCOMPARE(s.timeStamp(), stamp);
}
//Nettoyage de tous les test
void SerialNumberTest::cleanUpTestCase()
{
    qDebug() << "Nettoyage de tous les test...\n";

}



QTEST_MAIN(SerialNumberTest);
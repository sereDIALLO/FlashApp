//
// Created by tdiallo on 06/08/24.
//

#ifndef SERIALNUMBERTEST_H
#define SERIALNUMBERTEST_H
#include <QTest>



class SerialNumberTest : public QObject
{
  Q_OBJECT
  private slots:
  void initTestCase();
  void dateInt_should_currentDate();
  void dateStr_should_currentDate();
  void time_should_currentTime();
  void endCarec_should_p();
  void caracHex_should_1400000();
  void timeStamp_should_currentStamp();
  void cleanUpTestCase();


};



#endif //SERIALNUMBERTEST_H

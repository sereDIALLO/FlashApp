//
// Created by tdiallo on 06/08/24.
//

#ifndef SIGNATURETEST_H
#define SIGNATURETEST_H
#include <QTest>


class SignatureTest : public  QObject
{
    Q_OBJECT
    private slots:
    void initTestCase();
    void cleanupTestCase();
};


#endif //SIGNATURETEST_H

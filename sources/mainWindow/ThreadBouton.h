//
// Created by tdiallo on 12/27/25.
//

#ifndef THREADBOUTON_H
#define THREADBOUTON_H

#include <QObject>
#include <QString>
#include "JlinkCommand.h"

class ThreadBouton : public QObject{

    Q_OBJECT

public:
    explicit ThreadBouton(const QString& firmwarePath, const QString& probeSerialNumber, QObject* parent = nullptr);

    signals:
        void flashingFinished(bool success);
        void flashingProgress(const QString& message);

    public slots:
        void doFlashing();

private:
    QString m_firmwarePath;
    QString m_probeSerialNumber;
    uint64_t m_numeroLot;

};



#endif //THREADBOUTON_H

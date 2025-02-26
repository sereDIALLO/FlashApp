//
// Created by tdiallo on 12/27/25.
//

#include "ThreadBouton.h"
#include "SerialNumber.h"
#include "mainwindow.h"

#include <QDebug>

// le constructeur
ThreadBouton::ThreadBouton(const QString& firmwarePath, const QString& probeSerialNumber,QObject* parent)
    : QObject(parent), m_firmwarePath(firmwarePath), m_probeSerialNumber(probeSerialNumber){}

void ThreadBouton::doFlashing()
{
    emit flashingProgress("Flashing in progress...");

    JlinkCommand device;
    SerialNumber serial;
    MainWindow w;
    std::string device_name = "nRF52832_xxAA";
    std::string interface = "SWD";
    int speed = 4000;
    std::string jlinkExecutable = "JLinkExe";

    std::string serialAdressDate = "0x100010C0"; // UICR-16 pour la date de production
    std::string serialAdressCount = "0x100010C4"; // UICR-17 pour la production counter

    std::string boardSerialAdressFaible = "0x100010C8"; // UICR-18
    std::string boardSerialAdressFort = "0x100010CC"; // UICR-19

    uint64_t numeroLot = w.getNumeroLot();

    int  timeStamp = serial.timeStamp();
    bool success = device.executeJlinkCommand(
        device_name,
        interface,
        speed,
        jlinkExecutable,
        m_firmwarePath.toStdString(),
        m_probeSerialNumber.toStdString(),
        serialAdressDate,
        timeStamp,
        boardSerialAdressFaible,
        boardSerialAdressFort,
        numeroLot
        );


    if (success)
    {
        emit flashingProgress("Flashing completed successfully!");
    } else
    {
        emit flashingProgress("Flashing failed. Please try again.");
    }

    emit flashingFinished(success);
}


// pour chaque sonde connecté je crée un thread qui gère le processus de flashage
// ajouter un second paramètre " probeSerialNumber
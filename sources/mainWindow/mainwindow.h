//
// Created by tdiallo on 24/07/24.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QFileDialog>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextStream>
#include <QDebug>
#include <iostream>
#include <mbedtls/pk.h>
#include <QColorDialog>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow() override;
        uint64_t getNumeroLot();
    private slots:
        void loadFirmware();
        void flash_firmware();
        void runflashFirmware();
        void timer_check_Probe();
        void timer_check_Device();
        void valider();
        void updateProbe();

    signals:
        void flashFinished();

    private:
        Ui::MainWindow *ui;
        QString firmwarePath{};
        QString probeSerialNumber{};
        std::string signaturePath{};
    uint64_t numeroLot{};
        mbedtls_pk_context public_key{};

        QTimer *timerSonde{};
        QTimer *timerDevice{};
        int tim ;
        int flashesInProgress = 0;


        bool executeJLinkCommands(const QString &device, const QString &interface, int speed, const QString &jlinkExecutable, const QString &hexFilePath);
};


#endif //MAINWINDOW_H

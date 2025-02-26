//
// Created by tdiallo on 24/07/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "mainwindow.h"

#include <fstream>

#include "SerialNumber.h"
#include "JlinkCommand.h"
#include "CheckSignature.h"
#include "ui_MainWindow.h"
#include <QTimer>
#include <QInputDialog>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QTest>
#include <QtConcurrent>

#include "ThreadBouton.h"


bool isFlashing = false;

// le constructeur
MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow),
    firmwarePath(""),
    signaturePath(),
    numeroLot(),
    public_key()
{
    ui->setupUi(this);
    SerialNumber serial;
    JlinkCommand device;

    ui->frame_status->hide();
    ui->label_flash->hide();
    ui->flash->hide();
    ui->frame_flash->hide();
    ui->frame_info->hide();
    ui->frame_lot->hide();

    QString currentDate = serial.currentDate();
    ui->L_date->setText(currentDate);
    QString currentT = serial.getTime();
    ui->L_heure->setText(currentT);
    std::ifstream file;
    int c{};
    file.open("../registre.txt");
    if (!file.is_open())
    {
        return;
    }
    file >> c;
    file >> c;
    ui->lcdNumber->display(c);

    std::string device_name     = "nRF52832_xxAA";
    std::string interface       = "SWD";
    int speed = 4000;
    std::string jlinkExecutable = "JLinkExe";

    QString lotStr = ui->lineEdit->text();
    QRegularExpression regex("\\d{5}");
    QValidator *validator = new QRegularExpressionValidator(regex, ui->lineEdit);
    ui->lineEdit->setValidator(validator);

    //----------------------------Verification de la sonde au demarrage du logiciel------------------------------
    QStringList list = device.get_connected_probes();
    for (const QString &s : list) {
        qDebug() << s;
    }
    if (device.verifyProbe(jlinkExecutable))
    {
        //verification sonde
        timerSonde = new QTimer(this);
        connect(timerSonde, &QTimer::timeout, this, &MainWindow::timer_check_Probe);
        timerSonde->start(200);

        ui->update->hide();
        ui->L_sonde->setText("<font color = 'green'> connected</font>");

    }
    else
    {
        // Si la sonde n'est pas dectecter au demarrage, on actualise pour une potentielle détection
        ui->select->hide();
        ui->update->show();
        ui->L_sonde->setText("<font color = 'red'> Not connected...</font>");
        ui->label_load->setText("<font color = 'red'> Probe is not connected. Connect probe and retry</font>");

        QObject::connect(ui->update, &QPushButton::clicked, this, &MainWindow::updateProbe);
    }

    QObject::connect(ui->select, &QPushButton::clicked, this, &MainWindow::loadFirmware);
    QObject::connect(ui->valider, &QPushButton::clicked, this, &MainWindow::valider);
    QObject::connect(ui->flash, &QPushButton::clicked, this, &MainWindow::flash_firmware);

}

// le destructeur
MainWindow::~MainWindow()
{
    delete ui;
}
// chargement du firmware
void MainWindow::loadFirmware()
{
    firmwarePath = QFileDialog::getOpenFileName(this, "Chargement du Firmware", "", "Hex Files (*.hex)");
    if (!firmwarePath.isEmpty())
    {
        qDebug() << "Firmware loaded:" << firmwarePath;

        std::string fw = firmwarePath.toStdString().substr(firmwarePath.toStdString().length() - 15);
        ui->L_fw->setText(fw.data());

        //-----------------------------Verification de la signature -----------------------------

        CheckSignature check;
        //charger la clé public
        public_key = check.loadPulicKey("../publickey.pub");
        //Verifier la clé publique
        if (mbedtls_pk_get_len(&public_key) == 0)
        {
            // QMessageBox::critical(this, "Error", "Cannot open the public key. Check and retry.");
            std::cerr << "Cannot open the public key. Check and retry.\n";
        }
        // chargement du fichier hex
        std::vector<unsigned char> firmwareData = check.readFirmware(firmwarePath.toStdString());

        // chargement de la signature.
        // J'ai supprimé les 3 dernier caractère et j'ai concatener avec signature
        // poour pouvoir recupérer le chemin de la signature.

        std::string pathStr = firmwarePath.toStdString();
        signaturePath = pathStr.substr(0, pathStr.size() - 3) + "signature";

        std::vector<unsigned char> signatureData = check.readSignature(signaturePath);
        // Verifier que la signature et le firmware sont chargés
        if (signatureData.data() == nullptr && firmwareData.data() == nullptr)
        {
            // QMessageBox::critical(this, "Error", "Cannot open the signature or firmware file. Check and retry.\n");
            ui->label_load->setText("<font color = 'red'> Cannot open the signature or firmware file. Check and retry</font>");
        }

        // verification de la signature
        if (check.verifySignature(public_key, firmwareData, signatureData))
        {
            ui->select->hide();
            ui->frame_lot->show();
            std::cout << "Signature is valid." << std::endl;
            ui->L_signature->setText("<font color = 'green'>  OK</font>");
            ui->label_load->setText("<font color = 'green'>Firmware loaded successfully.</font>");

            // ui -> frame_status -> show();
            // ui -> label_flash -> show();
            // ui -> flash -> show();
            // ui -> frame_flash -> show();
            // ui -> frame_info -> show();


        }
        else
        {
            std::cerr << "Signature is invalid." << std::endl;
            ui->L_signature->setText("<font color = 'red'> NOK</font>");
            ui->label_load->setText("<font color = 'red'>Signature verification failed. Verify file and retry.</font>");
            //QMessageBox::critical(this, "Error", "Signature is invalid.\n Cannot launch flash.");

        }


        // Libérer la mémoire de la clés public
        mbedtls_pk_free(&public_key);
    }
    else
    {
        qDebug() << "No firmware selected.";
        ui->label_load->setText("<font color = 'red'>No firmware selected. Please select firmware version</font>");
        // QColor couleur = QColorDialog::getColor(Qt::red, this);
    }
}


// Methode pour le numro de lot.
void MainWindow::valider()
{

    QString lotStr = ui->lineEdit->text();
    QRegularExpression regex("\\d{5}");
    QValidator *validator = new QRegularExpressionValidator(regex, ui->lineEdit);
    ui->lineEdit->setValidator(validator);
    if (ui->lineEdit->hasAcceptableInput())
    {
        MainWindow::getNumeroLot();
        std::cout << "numero de lot est : " << numeroLot << std::endl;
        //Verifier que le firmware et le numero de lot sont valident et le bouton charger à disparu avant de commencer à flasher
        if (!firmwarePath.isEmpty() && (getNumeroLot() or numeroLot == 0) && ui->select->isHidden())
        {

            ui->frame_lot->hide();
            ui->L_lot->setText(lotStr);
            ui->frame_status->show();
            ui->label_flash->show();
            ui->flash->show();
            ui->frame_flash->show();
            ui->frame_info->show();
        }
        else
        {
            ui->label_lot->setText("<font color = 'red'>Veuillez charger le firmware puis saisir un numéro de lot</font>");
            std::cerr << "veuillez charger le firmware et donner un numero de lot.\n";
        }
    }
    else
    {
        std::cerr << "Veuillez entrer un nombre de 5 chiffres.\n";
        ui->label_lot->setText("<font color = 'red'>Please enter a 5 digit batch number ant then press validate </font>");
    }

    // timerDevice = new QTimer(this);
    // connect(timerDevice, &QTimer::timeout, this, &MainWindow::timer_check_Device);
    // timerDevice->start(200);
}


// Methode qui permet de flasher le device.
void MainWindow::runflashFirmware()
{
    JlinkCommand device;
    if (isFlashing)
    {
        qDebug() << "Flash in progress, please wait!";
        return;
    }

    QStringList probes = device.get_connected_probes();
    if (probes.empty())
    {
        ui->label_status->setText("No probes connected");
        return;
    }

    // Marquer le flashage comme en cours
    isFlashing = true;
    flashesInProgress = probes.size();  // Nombre total de flashs en cours

    // Désactiver le bouton pour empêcher les clics multiples
    ui->flash->setEnabled(false);
    ui->label_status->setText("Flashing in progress...");

    for (const QString &probe : probes)
    {
        QThread* flashThread = new QThread;
        ThreadBouton* worker = new ThreadBouton(firmwarePath, probe);

        worker->moveToThread(flashThread);

        // Connexion des signaux et slots
        QObject::connect(flashThread, &QThread::started, worker, &ThreadBouton::doFlashing);
        QObject::connect(worker, &ThreadBouton::flashingFinished, this, [=](bool success) {
            flashesInProgress--;

            if (success)
                ui->label_status->setText("Flashing completed successfully for probe: " + probe);
            else
                ui->label_status->setText("Flashing failed for probe: " + probe);

            // Vérifier si tous les flashs sont terminés
            if (flashesInProgress == 0)
            {
                ui->flash->setEnabled(true);
                isFlashing = false;
            }

            // Nettoyage des ressources
            flashThread->quit();
            flashThread->wait();
            worker->deleteLater();
            flashThread->deleteLater();
        });

        QObject::connect(worker, &ThreadBouton::flashingProgress, this, [=](const QString& message) {
            ui->label_status->setText("[" + probe + "] " + message);
        });

        // Démarrer le thread
        flashThread->start();
    }
}


void MainWindow::flash_firmware()
{
    MainWindow::runflashFirmware();
}
//
// bool MainWindow::executeJLinkCommands(const QString &device,
//                                       const QString &interface,
//                                       int speed,
//                                       const QString &jlinkExecutable,
//                                       const QString &hexFilePath)
// {
//     QTemporaryFile tempFile;
//     if (!tempFile.open())
//     {
//         std::cerr << "Cannot create temporary file.\n";
//         return false;
//     }
//     // Ecrire les commandes JLink dans le fichier temporaire
//     QTextStream out(&tempFile);
//     out << "device " << device << "\n";
//     out << "if " << interface << "\n";
//     out << "speed " << speed << "\n";
//     out << "exec SetVerifyDownload = 3\n";
//     out << "connect\n";
//     out << "r\n";
//     out << "Erase\n";
//     out << "loadfile " << hexFilePath << "\n";
//     out << "W1 0x100010C4 00000001\n";
//     //Activation du rbp
//     out << "W1 0x10001208 0x00\n";
//     out << "exit\n";
//     tempFile.close();
//
//     QProcess process;
//     process.start(jlinkExecutable,
//                   {
//                       "-CommandFile",
//                       tempFile.fileName()
//                   });
//
//     if (!process.waitForStarted())
//     {
//         qCritical() << "Failed to start JLink:" << process.errorString();
//         return false;
//     }
//
//     if (!process.waitForFinished())
//     {
//         qCritical() << "JLink did not finish properly:" << process.errorString();
//         return false;
//     }
//
//     // Récupération de la sortie
//     QString output = process.readAll();
//     qDebug() << output;
//
//     if (output.contains("Cortex-M4 identified") || output.contains("Found SW-DP"))
//     {
//         return true;
//     }
//     else
//     {
//         std::cerr << "Device not connected properly";
//         return false;
//     }
// }
//
// //Slot de vérification de la sonde.
//
void MainWindow::timer_check_Probe()
{
    std::string  jlinkExecutable = "JLinkExe";
    JlinkCommand device;
    if (device.verifyProbe(jlinkExecutable))
    {
        std::cout << "probe is connnected.\n";
        ui->L_sonde->setText("<font color = 'green'>  connected</font>");
    }
    else
    {
        std::cerr << "probe is not connected.\n";
        ui->L_sonde->setText("<font color = 'red'> Not connected</font>");
        ui->L_device->setText("<font color = 'red'> Not connected</font>");
        return;
    }
    return;
}
//
//Slot de vérification de device
void MainWindow::timer_check_Device()
{
    std::string  device_name     = "nRF52832_xxAA";
    std::string  interface       = "SWD";
    int          speed           = 4000;
    std::string  jlinkExecutable = "JLinkExe";
    JlinkCommand device;
    if (device.verifyConnection(jlinkExecutable, device_name, interface, speed))
    {
        ui->L_device->setText("<font color = 'green'> connected</font>");
    }
    else
    {
        ui->L_device->setText("<font color = 'red'> Not connected</font>");
    }
}

uint64_t MainWindow::getNumeroLot()
{
    QString lotStr = ui->lineEdit->text();
    numeroLot = lotStr.toInt();
    return numeroLot;
}

//methode pour actualiser la sonde
void MainWindow::updateProbe()
{
    JlinkCommand probe;
    std::string  jlinkExecutable = "JLinkExe";
    if (probe.verifyProbe(jlinkExecutable))
    {
        ui->label_load->setText("<font color = 'green'> Probe is connected. Load firmware</font>");
        ui->select->show();
        ui->update->hide();
        timerSonde = new QTimer(this);
        QObject::connect(timerSonde, &QTimer::timeout, this, &MainWindow::timer_check_Probe);
        timerSonde->start(200);
    }
    else
    {
        std::cerr << "probe not connected\n";
    }
}

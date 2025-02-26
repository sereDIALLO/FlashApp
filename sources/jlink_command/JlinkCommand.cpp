//
// Created by tdiallo on 22/07/24.
//

#include "JlinkCommand.h"
#include <QProcess>
#include <QTextStream>
#include <QTemporaryFile>
#include <qdebug.h>
#include <QMessageBox>


//constructeur
JlinkCommand::JlinkCommand()
{

}

//Verify device connexion and probe
bool JlinkCommand::verifyConnection( const std::string &jlinkExecutable, const std::string &device, const std::string &interface, int speed)
{
    // Fichier temporaire pour stocker les informations de connexions
    QTemporaryFile tempFile;
    if (!tempFile.open())
    {
       std::cerr << "Cannot create temporary file.\n";
        return false;
    }
    //Ecrire les commandes de vérification dans le ficier temporaire

    QTextStream out(&tempFile);
    out << "device " << device.data() << "\n";
    out << "si " << interface.data() << "\n";
    out << "speed " << speed << "\n";
    out << "con\n";
    out << "exit\n";
    tempFile.close();

    QProcess process;
    process.start(jlinkExecutable.data(), {"-CommandFile", tempFile.fileName()});

    if (!process.waitForStarted())
    {
        qCritical() << "Failed to start JLink for connection verification:" << process.errorString();
        return false;
    }

    if (!process.waitForFinished())
    {
        qCritical() << "JLink did not finish connection verification properly:" << process.errorString();
        return false;
    }
    // Récupération de la sortie
    QString output = process.readAll();
    qDebug() << output;

    if (output.contains("Cortex-M4 identified") && (output.contains("69651114") or output.contains("505000713") ))
    {
        return true;
    }
    else
    {
        std::cerr << "Device not connected properly";
        return false;
    }

}

// méthode de récuparation des numéros de séries de toutes les sondes connectées.
QStringList JlinkCommand::get_connected_probes()
{
    QStringList sondes{};
    QProcess process;
    process.start("JLinkExe");
    if (!process.waitForStarted()) {
        qCritical() << "Failed to start JLink execution:" << process.errorString();
        return sondes;
    }
    //Envoyer les commandes et quitter jlinkexe
    process.write("ShowEmuList\n");
    process.write("exit\n");
    process.closeWriteChannel();
    process.waitForFinished();
    //Lire la sortie
    QString output = process.readAllStandardOutput();

    // analyser la sortie pour trouver les numéros de série
    QStringList lines = output.split("\n");
    for (const QString &line : lines)
    {
        if (line.contains("Serial number:")) {
            // Extraire le numéro de série
            QString serialNumber = line.split("Serial number:").last().split(",").first().trimmed();
            sondes.append(serialNumber);
        }
    }
    return sondes;


}

// Verify probe
bool JlinkCommand::verifyProbe(const std::string& JlinkExecutable)
{
    // on créer un fichier temporaire ou stocker les informations
    QTemporaryFile file;
    if(!file.open())
    {
        std::cerr << "Cannot open the temporary file.\n";
        return false;
    }
    // Ecrire les commmandes de vérification dans le fichier temporaire
    QTextStream out(&file);
    out << "ShowEmulist\n";
    out << "exit\n";
    file.close();

    QProcess process;
    process.start(JlinkExecutable.data(), {"-CommandFile", file.fileName()});
    if(!process.waitForStarted())
    {
        qCritical() << "Failed to start JLink for connection verification:" << process.errorString();
        return false;
    }
    if(!process.waitForFinished())
    {
        qCritical() << "JLink did not finish connection verification properly:" << process.errorString();
        return false;
    }
    // recupération de la sortie de jlink.

    QString output = process.readAll();
    if(output.contains("69651114") || output.contains("505000713") )
    {
        std::cout <<"probe is connected.\n";
        return true;
    }
    else
    {
        std::cerr << "probe is not connected.\n";
        return false;
    }

}

// execute jlink commande

bool JlinkCommand::executeJlinkCommand( const std::string &device,
                                        const std::string &interface,
                                        int speed, const std::string &jlinkExecutable,
                                        const std::string &hexFilePath,
                                        const std::string &probeSerialNumber,
                                        const std::string &serialAdressDate,
                                        int timeStamp,
                                        const std::string &boardSerialAdressFaible,
                                        const std::string &boardSerialAdressFort,
                                        uint64_t boardSerial)
{
    QTemporaryFile tempFile;
    if (!tempFile.open()) {
        std::cerr << "Cannot create temporary file.\n";
        return false;
    }

    // Ecriture  des commandes JLink dans le fichier temporaire "tempFile"
    //convertir le numero en chaine hexadécimal


    /*
     NB : il faut impérativement convertir le timeStamp et la production counter en  chaine hexadécimal
          avant d'envoyer les données aux
          registres.
     */

    QString serialNumberDateHex = QString("%1").arg(timeStamp, 8, 16, QLatin1Char('0')).toUpper();
    // QString serialNumberCountHex = QString("%1").arg(serialNumberCount, 7, 16, QLatin1Char('0')).toUpper();
    //
    QString serialBoardHexString = QString("%1").arg(boardSerial, 16, 16, QLatin1Char('0')).toUpper();
    //qDebug() << serialBoardHexString;
    QTextStream out(&tempFile);
    out << "device " << device.data() << "\n";
    out << "si " << interface.data() << "\n";
    out << "speed " << speed << "\n";
    out << "serial " << probeSerialNumber.data() <<"\n";
    out << "exec SetVerifyDownload = 3\n";
    out << "connect\n";
    out << "Erase\n";
    // load firmware
    out << "loadfile " << hexFilePath.data() << "\n";
    // Write serial number with date production and counter production.
    out << "W4 " << serialAdressDate.data() << " " << serialNumberDateHex << "\n"; // UICR-16 pour la date de production
    // out << "W4 " << serialAddressCount.data() << " " << serialNumberCountHex << "\n"; // UICR-17 pour la production counter
    // // Write hardware Serial Number.
    out << "W4 " << boardSerialAdressFort.data() << " " << serialBoardHexString.left(8) << "\n"; //UICR 19 poids fort
    out << "W4 " << boardSerialAdressFaible.data() << " " << serialBoardHexString.right(8) << "\n"; //UICR 18 poids faible

    // Activation du rbp pour empêcher la relecture des données.
    out << "W1 0x10001208 0x00\n";
    out << "r\n"; // Reset device.
    out << "exit\n";
    tempFile.close();

    QProcess process;
    process.start(jlinkExecutable.data(), {"-CommandFile", tempFile.fileName()});

    if (!process.waitForStarted()) {
        qCritical() << "Failed to start JLink:" << process.errorString();
        return false;
    }

    if (!process.waitForFinished()) {
        qCritical() << "JLink did not finish properly:" << process.errorString();
        return false;
    }

    // Récupération de la sortie
    QString output = process.readAll();
    //convertir la sortie en string
    std::string outputStr = output.toStdString();
    std::cout<< outputStr;
    // qDebug() << output;
    if(!output.contains("Programming failed") or !output.contains("Error while programming flash"))
    {
        std::cout << "Device flash successfully. \n";
        return true;
    }
    else
    {
        std::cerr << " Device not flash succesfully.\n";
        return false;
    }

}

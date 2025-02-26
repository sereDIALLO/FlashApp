//
// Created by tdiallo on 18/07/24.
//

#include "SerialNumber.h"

#include <fstream>
#include <QDate>
#include <sstream>
#include <unistd.h>
#include <iostream>

SerialNumber::SerialNumber()
{

}

int SerialNumber::getCurrentDate()
{
    QDate date = QDate::currentDate();
    QString dateStr = date.toString("yyyyMMdd");
    return dateStr.toInt();
}
// Methode pour recupèrer la date en string
QString SerialNumber::currentDate()
{
    QDate date = QDate::currentDate();
    QString dateStr = date.toString("yyyy-MM-dd");
    return dateStr;
}
// methode pour renvoyer l'heure
QString SerialNumber::getTime()
{
    QTime currentTime = QTime::currentTime();
    QString timeStr = currentTime.toString("HH:mm:ss");
    return timeStr;;
}
// Methode pour retourner le nom de la machine

std::string SerialNumber::getComputerName()
{

    char computerName[256];
#ifdef _WIN32
    DWORD size = sizeof(computerName);
    if(GetComputerName(computerName, &size))
        return std::string(computerName);
    else
        return  "Windows Unknown";
#else
    if(gethostname(computerName, sizeof(computerName)) == 0)
        return std::string(computerName);
    else
        return "Linux Unknown";


#endif

}


// recuparation du dernier caractère

char SerialNumber::endCarac(const std::string& str)
{
    const std::string lastCarac = str.substr(str.length() -1 );
    const char* carc = lastCarac.c_str();
    return *carc;
}

//convert the last character to hex value if possible

int SerialNumber::caracHex(const char carac)
{

    int n = 0;
    //je recupère la dernière caractère du nom
    // char lastCarac = endCarac(str);
    // char lastCarac = str.back();
    try
    {
        std::stringstream ss;
        //Vérifier que le caractère est valide pour une conversion hexadecimal
        if(!std::isxdigit(carac))
        {
            throw std::invalid_argument("caractère non valide pour une converion hexadécimale");

        }
        //convertir le caractère en hex
        ss << std::hex <<carac;
        ss >> n;
    }
    catch(std::string const& chaine)
    {
        //si une exception est levée on assigne n à zéro
        std::cerr << chaine <<std::endl;
        n = 0;
    }

    return n * 100000;

}

int SerialNumber::compteurJournalier()
{
    count = 0;
    int currentDate = getCurrentDate();
    std::string computerName = getComputerName();
    char lastCarac = endCarac(computerName);
    int lastCaracHex = caracHex(lastCarac);
    std::ifstream myfile;
    myfile.open("../registre.txt");
    if (myfile.is_open())
    {
        // std::getline(myfile, lastDate);
        myfile >> lastDate;
        myfile >> count;
        myfile.close();
    }

    // comparaison des dates
    if(lastDate != currentDate)
    {
        count = -1;
        lastDate = currentDate;
    }
    //Incrementation du compteur
    count++;

    //Ecriture des donnees dans le fichier
    std::ofstream file;
    file.open("../registre.txt");
    if (file)
    {
        file << lastDate << std::endl;
        file << count <<std::endl;
        file << computerName <<std::endl;
        file << lastCarac<<std::endl;
        file << lastCaracHex;
        std::flush(file);
        file.close();
    }

return  count;
}

//time stamp
int SerialNumber::timeStamp()
{
    QDateTime currentDateTime = QDateTime::currentDateTimeUtc();
    return currentDateTime.toSecsSinceEpoch();
}


//
// Created by tdiallo on 18/07/24.
//

#ifndef SERIALNUMBER_H
#define SERIALNUMBER_H
#include <QString>
#include <string>
#include <vector>
#include <QMessageBox>


class SerialNumber
{
    public:
    SerialNumber();
    //return Current date
    int getCurrentDate();
    QString currentDate();
    //
    QString getTime();
    // return computer name
    static std::string getComputerName();
    //return end carac of name machine.
    static char endCarac(const std::string& str);
    // return end carac in hex
    static int caracHex(char carac);
    int compteurJournalier();

    int timeStamp();

    private:
    int count{};
    int lastDate;

};



#endif //SERIALNUMBER_H

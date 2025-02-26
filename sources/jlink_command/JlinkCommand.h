//
// Created by tdiallo on 22/07/24.
//

#ifndef JLINKCOMMAND_H
#define JLINKCOMMAND_H
#include <iostream>
#include <QString>
#include <QStringList>



class JlinkCommand
{
    public:
    JlinkCommand();
    bool verifyConnection( const std::string &jlinkExecutable, const std::string &device, const std::string &interface, int speed);
    bool verifyProbe(const std::string &JlinkExecutable);
    bool executeJlinkCommand( const std::string &device, const std::string &interface, int speed, const std::string &jlinkExecutable, const std::string &hexFilePath, const std::string &probeSerialNumber, const std::string &serialAdressDate,
                                        int timeStamp,
                                        const std::string &boardSerialAdressFaible,
                                        const std::string &boardSerialAdressFort,
                                        uint64_t boardSerial);
    QStringList get_connected_probes();

    private:
};



#endif //JLINKCOMMAND_H

//
// Created by tdiallo on 18/07/24.
//

#ifndef CHECKSIGNATURE_H
#define CHECKSIGNATURE_H

#include <mbedtls/pk.h>
#include <string>
#include <vector>


class CheckSignature
{
    public:
    // constructeur
    CheckSignature();
    //Methode pour lire le firmware
    std::vector<unsigned char> readFirmware(const std::string& filePath);
    // Methode pour lire la signature
    std::vector<unsigned char> readSignature(const std::string& filePath);
    //Methode pour écrire dans un fichier
    void writeOnFile(const std::string& filePath, std::vector<unsigned char> data);
    // Methode pour charger une clé privée
    mbedtls_pk_context loadPrivateKey(const std::string& filename);
    //Methode pour charger une clé publique
    mbedtls_pk_context loadPulicKey(const std::string& filename);

    // Methode pour signer un message avec une clé privée
    std::vector<unsigned char> signMessage(mbedtls_pk_context& pk, const std::vector<unsigned char>& message);
    // Methode pour vérifier la signature
    bool verifySignature(mbedtls_pk_context& pk, const std::vector<unsigned char>& message, const std::vector<unsigned char>& signature);





    private:
    mbedtls_pk_context private_key{};
    mbedtls_pk_context public_key{};
    std::vector<unsigned char> firmware{};
    std::vector<unsigned char> signature{};

};



#endif //CHECKSIGNATURE_H

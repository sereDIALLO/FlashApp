//
// Created by tdiallo on 18/07/24.
//

#include "CheckSignature.h"
#include <mbedtls/sha256.h>
#include <iostream>
#include <fstream>


//constructeur

CheckSignature::CheckSignature()
{

}

// Methode pour lire le firmware
std::vector<unsigned char> CheckSignature::readFirmware(const std::string& filePath)
{

    std::ifstream file(filePath, std::ios::binary);
    //on verifier si l'ouveture à bien réussi
    if (!file.is_open())
    {
        std::cerr << "Cannot open the firmware file." << std::endl;
        // QMessageBox::(,"Error", "Cannot open the firmware file.\n");
    }
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

//Methode pour lire la signature
std::vector<unsigned char> CheckSignature::readSignature(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    //verification de l'ouverture du fichier
    if(!file.is_open())
    {
        std::cerr <<" Cannot open the signature file.\n";
    }
    return std::vector<unsigned char>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

//Methode pour écrire dans un fichier
void CheckSignature::writeOnFile(const std::string& filePath, std::vector<unsigned char> data)
{
    std::ofstream file(filePath, std::ios::binary);
    file.write(reinterpret_cast<char *>(data.data()), data.size());
}

// Methode pour charger une clé privée depuis un fichier pem
mbedtls_pk_context CheckSignature::loadPrivateKey(const std::string& filename)
{
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    if (mbedtls_pk_parse_keyfile(&pk, filename.c_str(), nullptr) != 0)
    {
        std::cerr << " Error loading private key." << std::endl;
    }
    return pk;
}
// Methode pour charger une clé public depuis un fichier pub
mbedtls_pk_context CheckSignature::loadPulicKey(const std::string& filename)
{
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    if (mbedtls_pk_parse_public_keyfile(&pk, filename.c_str()) != 0)
    {
        std::cerr <<" Error loading public key. " <<std::endl;
    }
    return pk;
}

// Methode pour singer un message
std::vector<unsigned char> CheckSignature::signMessage(mbedtls_pk_context& pk, const std::vector<unsigned char>& message)
{
    std::vector<unsigned char> hash(32);
    // Il faut 512 bits pour RSA-4096
    std::vector<unsigned char> signature(512);

    //Hachage du message

    mbedtls_sha256_ret(message.data(), message.size(), hash.data(), 0);
    size_t signature_len = 0;
    if (mbedtls_pk_sign(&pk, MBEDTLS_MD_SHA256, hash.data(), 0, signature.data(), &signature_len, nullptr, nullptr) != 0)
    {
        std::cerr <<" Error signing message." << std::endl;
    }
    signature.resize(signature_len);
    return signature;
}

// Methode pour vérifier la signature
bool CheckSignature::verifySignature(mbedtls_pk_context& pk, const std::vector<unsigned char>& message, const std::vector<unsigned char>& signature)
{
    unsigned char hash[32];
    // Hachage du message
    mbedtls_sha256_ret(message.data(), message.size(), hash, 0);
    return mbedtls_pk_verify( &pk, MBEDTLS_MD_SHA256, hash, 0, signature.data(), signature.size() ) == 0;
}










//
// Created by diago on 2024-06-04.
//

#include "../instance.hpp"

bool
lurch::instance::generate_self_signed_cert(const std::string &certfile_path, const std::string &keyfile_path, const long certificate_version) {

    if(certificate_version > 2 || certificate_version < 0) {
        return false;
    }

    EVP_PKEY* pkey = nullptr;
    X509* cert = nullptr;
    X509_NAME* name = nullptr;
    FILE* cert_fp = nullptr;
    FILE* key_fp = nullptr;
    errno_t err = 0x00;

    auto _ = lurch::defer([&] {
        if (cert_fp) fclose(cert_fp);
        if (key_fp) fclose(key_fp);
        if (name) X509_NAME_free(name);
        if (cert) X509_free(cert);
        if (pkey) EVP_PKEY_free(pkey);
    });


    io::info("\n\nbeginning certificate generation. version:" + std::to_string(certificate_version + 1));
    io::info("generating evp key...");

    pkey = EVP_PKEY_new();
    if (!pkey) {
        std::cerr << "error creating EVP_PKEY" << std::endl;
        return false;
    }

    io::info("creating evp key context...");
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if(!ctx) {
        std::cerr << "error creating EVP_PKEY_CTX" << std::endl;
        return false;
    }

    io::info("generating RSA key pair...");
    if (    EVP_PKEY_keygen_init(ctx) <= 0 ||
            EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, LURCH_RSA_KEYSIZE) <= 0 ||
            EVP_PKEY_keygen(ctx, &pkey) <= 0) {

        std::cerr << "failed to generate RSA key pair" << std::endl;
        return false;
    }

    io::info("generating x509 certificate...");
    EVP_PKEY_CTX_free(ctx);
    cert = X509_new();
    if(!cert) {
        std::cerr << "failed to generate X509 certificate." << std::endl;
        return false;
    }


    X509_set_version(cert, certificate_version);
    ASN1_INTEGER_set(X509_get_serialNumber(cert), 12345);

    X509_gmtime_adj(X509_get_notBefore(cert), 0);
    X509_gmtime_adj(X509_get_notAfter(cert), 31536000L);


    name = X509_NAME_new();
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"Self-Signed", -1, -1, 0);

    X509_set_subject_name(cert, name);
    X509_set_issuer_name(cert, name);
    X509_set_pubkey(cert, pkey);


    io::info("signing certificate...");
    if (!X509_sign(cert, pkey, EVP_sha256())) {
        std::cerr << "failed to sign certificate." << std::endl;
        return false;
    }

    io::info("writing certificate and key to disk...");
    err = fopen_s(&cert_fp, certfile_path.c_str(), "wb");
    if (err != 0 || !PEM_write_X509(cert_fp, cert)) {
        std::cerr << "failed to write certificate to file" << std::endl;
        return false;
    }

    err = fopen_s(&key_fp, keyfile_path.c_str(), "wb");
    if (err != 0 || !PEM_write_PrivateKey(key_fp, pkey, nullptr, nullptr, 0, nullptr, nullptr)) {
        std::cerr << "failed to write private key to file" << std::endl;
        return false;
    }


    io::success("done.\n");
    return true;
}

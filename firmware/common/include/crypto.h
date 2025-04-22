/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#ifndef CRYPTO_H
#define CRYPTO_H

#include <linux/crypto.h>
#include <linux/scatterlist.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <crypto/akcipher.h>

#define AES_KEY_LEN  16
#define AES_BLOCK_SIZE 16

// [L6] Presentation Layer: AES block encryption of payload
int aesEncrypt(void *payloadData, size_t len, u8 *key, u8 *iv);
int aesDecrypt(void *data, size_t len, u8 *key, u8 *iv);
int rsaEncrypt(u8 *plaintext, size_t len, u8 *encrypted, struct crypto_akcipher *tfm);
int rsaDecrypt(u8 *encrypted, size_t len, u8 *decrypted, struct crypto_akcipher *tfm);
int encryptWithRSA_AES(void *payloadData, size_t len, u8 *aesKey, u8 *iv, u8 *rsaPublicKey, u8 *encryptedData);
int decryptWithRSA_AES(void *encryptedData, size_t len, u8 *aesKey, u8 *iv, u8 *rsaPrivateKey, void *decryptedData);

#endif // CRYPTO_H

/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "crypto.h"

// AES encryption (no change)
int aesEncrypt(void *payloadData, size_t len, u8 *key, u8 *iv)
{
    struct crypto_cipher *tfm;
    int i;

    if (len % AES_BLOCK_SIZE != 0)
    {
        pr_err("[TX][L6] Data length must be a multiple of 16 bytes\n");
        return -EINVAL;
    }

    tfm = crypto_alloc_cipher("aes", 0, 0);
    if (IS_ERR(tfm))
    {
        pr_err("[TX][L6] Failed to allocate AES cipher\n");
        return PTR_ERR(tfm);
    }

    if (crypto_cipher_setkey(tfm, key, AES_KEY_LEN))
    {
        pr_err("[TX][L6] Failed to set AES key\n");
        crypto_free_cipher(tfm);
        return -EIO;
    }

    for (i = 0; i < len; i += AES_BLOCK_SIZE)
    {
        crypto_cipher_encrypt_one(tfm, payloadData + i, payloadData + i);
    }

    crypto_free_cipher(tfm);

    return 0;
}

// AES decryption (no change)
int aesDecrypt(void *data, size_t len, u8 *key, u8 *iv)
{
    struct crypto_cipher *tfm;
    int i;

    if (len % AES_BLOCK_SIZE != 0) {
        pr_err("AES decryption error: data size must be multiple of 16 bytes\n");
        return -EINVAL;
    }

    tfm = crypto_alloc_cipher("aes", 0, 0);
    if (IS_ERR(tfm)) {
        pr_err("Failed to allocate AES cipher\n");
        return PTR_ERR(tfm);
    }

    if (crypto_cipher_setkey(tfm, key, AES_KEY_LEN)) {
        pr_err("Failed to set AES key\n");
        crypto_free_cipher(tfm);
        return -EIO;
    }

    for (i = 0; i < len; i += AES_BLOCK_SIZE) {
        crypto_cipher_decrypt_one(tfm, data + i, data + i);
    }

    crypto_free_cipher(tfm);
    return 0;
}

int rsaEncrypt(u8 *plaintext, size_t len, u8 *encrypted, struct crypto_akcipher *tfm)
{
    struct scatterlist sg_in, sg_out;
    struct akcipher_request *req;
    int err;

    // Prepare scatterlist for input
    sg_init_one(&sg_in, plaintext, len);

    // Prepare scatterlist for output
    sg_init_one(&sg_out, encrypted, crypto_akcipher_maxsize(tfm));

    // Allocate memory for akcipher_request
    req = akcipher_request_alloc(tfm, GFP_KERNEL);
    if (!req) {
        pr_err("Failed to allocate akcipher_request\n");
        return -ENOMEM;
    }

    // Set up the request
    akcipher_request_set_crypt(req, &sg_in, &sg_out, len, len);

    // Encrypt using RSA
    err = crypto_akcipher_encrypt(req);
    if (err) {
        pr_err("RSA encryption failed\n");
    }

    // Free the request structure
    akcipher_request_free(req);

    return err;
}

int rsaDecrypt(u8 *encrypted, size_t len, u8 *decrypted, struct crypto_akcipher *tfm)
{
    struct scatterlist sg_in, sg_out;
    struct akcipher_request *req;
    int err;

    // Prepare scatterlist for input
    sg_init_one(&sg_in, encrypted, len);

    // Prepare scatterlist for output
    sg_init_one(&sg_out, decrypted, crypto_akcipher_maxsize(tfm));

    // Allocate memory for akcipher_request
    req = akcipher_request_alloc(tfm, GFP_KERNEL);
    if (!req) {
        pr_err("Failed to allocate akcipher_request\n");
        return -ENOMEM;
    }

    // Set up the request
    akcipher_request_set_crypt(req, &sg_in, &sg_out, len, len);

    // Decrypt using RSA
    err = crypto_akcipher_decrypt(req);
    if (err) {
        pr_err("RSA decryption failed\n");
    }

    // Free the request structure
    akcipher_request_free(req);

    return err;
}

// Encryption with AES and RSA for the key exchange (updated)
int encryptWithRSA_AES(void *payloadData, size_t len, u8 *aesKey, u8 *iv, u8 *rsaPublicKey, u8 *encryptedData)
{
    struct crypto_cipher *aesTfm;
    struct crypto_akcipher *rsaTfm;
    int err;
    u8 aesEncryptedKey[AES_KEY_LEN];

    // Step 1: Encrypt the AES key with RSA (public key encryption)
    rsaTfm = crypto_alloc_akcipher("rsa", 0, 0);
    if (IS_ERR(rsaTfm)) {
        pr_err("Failed to allocate RSA cipher\n");
        return PTR_ERR(rsaTfm);
    }

    err = rsaEncrypt(aesKey, AES_KEY_LEN, aesEncryptedKey, rsaTfm);
    if (err) {
        crypto_free_akcipher(rsaTfm);
        return err;
    }

    crypto_free_akcipher(rsaTfm);

    // Step 2: Encrypt the payload data with AES (using the AES key)
    aesTfm = crypto_alloc_cipher("aes", 0, 0);
    if (IS_ERR(aesTfm)) {
        pr_err("Failed to allocate AES cipher\n");
        return PTR_ERR(aesTfm);
    }

    if (crypto_cipher_setkey(aesTfm, aesKey, AES_KEY_LEN)) {
        pr_err("Failed to set AES key\n");
        crypto_free_cipher(aesTfm);
        return -EIO;
    }

    // Encrypt the data
    err = aesEncrypt(payloadData, len, aesKey, iv);
    if (err) {
        crypto_free_cipher(aesTfm);
        return err;
    }

    // Store the RSA-encrypted AES key followed by AES-encrypted data
    memcpy(encryptedData, aesEncryptedKey, AES_KEY_LEN);
    memcpy(encryptedData + AES_KEY_LEN, payloadData, len);

    crypto_free_cipher(aesTfm);

    return 0;
}

// Decryption with AES and RSA for the key exchange (updated)
int decryptWithRSA_AES(void *encryptedData, size_t len, u8 *aesKey, u8 *iv, u8 *rsaPrivateKey, void *decryptedData)
{
    struct crypto_cipher *aesTfm;
    struct crypto_akcipher *rsaTfm;
    int err;
    u8 aesEncryptedKey[AES_KEY_LEN];
    u8 rsaDecryptedKey[AES_KEY_LEN];

    // Step 1: Extract the RSA-encrypted AES key (first part of the encrypted data)
    memcpy(aesEncryptedKey, encryptedData, AES_KEY_LEN);

    // Step 2: Decrypt the AES key using RSA (private key decryption)
    rsaTfm = crypto_alloc_akcipher("rsa", 0, 0);
    if (IS_ERR(rsaTfm)) {
        pr_err("Failed to allocate RSA cipher\n");
        return PTR_ERR(rsaTfm);
    }

    err = rsaDecrypt(aesEncryptedKey, AES_KEY_LEN, rsaDecryptedKey, rsaTfm);
    if (err) {
        crypto_free_akcipher(rsaTfm);
        return err;
    }

    crypto_free_akcipher(rsaTfm);

    // Step 3: Decrypt the payload data with AES (using the decrypted AES key)
    aesTfm = crypto_alloc_cipher("aes", 0, 0);
    if (IS_ERR(aesTfm)) {
        pr_err("Failed to allocate AES cipher\n");
        return PTR_ERR(aesTfm);
    }

    if (crypto_cipher_setkey(aesTfm, rsaDecryptedKey, AES_KEY_LEN)) {
        pr_err("Failed to set AES key\n");
        crypto_free_cipher(aesTfm);
        return -EIO;
    }

    // Decrypt the payload
    err = aesDecrypt(encryptedData + AES_KEY_LEN, len - AES_KEY_LEN, rsaDecryptedKey, iv);
    if (err) {
        crypto_free_cipher(aesTfm);
        return err;
    }

    // Copy the decrypted payload back
    memcpy(decryptedData, encryptedData + AES_KEY_LEN, len - AES_KEY_LEN);

    crypto_free_cipher(aesTfm);

    return 0;
}

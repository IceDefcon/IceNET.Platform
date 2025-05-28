/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "crypto.h"
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 8, 0)
#include <linux/crypto.h>

// AES encryption with legacy crypto_cipher API
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

// AES decryption with legacy crypto_cipher API
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

#else
#include <crypto/skcipher.h>
#include <linux/scatterlist.h>

// AES encryption with modern crypto_skcipher API
int aesEncrypt(void *payloadData, size_t len, u8 *key, u8 *iv)
{
    struct crypto_skcipher *tfm;
    struct skcipher_request *req = NULL;
    struct scatterlist sg;
    int ret;

    if (len % AES_BLOCK_SIZE != 0)
        return -EINVAL;

    tfm = crypto_alloc_skcipher("ecb(aes)", 0, 0);
    if (IS_ERR(tfm))
        return PTR_ERR(tfm);

    ret = crypto_skcipher_setkey(tfm, key, AES_KEY_LEN);
    if (ret)
        goto out;

    req = skcipher_request_alloc(tfm, GFP_KERNEL);
    if (!req) {
        ret = -ENOMEM;
        goto out;
    }

    sg_init_one(&sg, payloadData, len);
    skcipher_request_set_crypt(req, &sg, &sg, len, NULL);

    ret = crypto_skcipher_encrypt(req);

    skcipher_request_free(req);
out:
    crypto_free_skcipher(tfm);
    return ret;
}

// AES decryption with modern crypto_skcipher API
int aesDecrypt(void *data, size_t len, u8 *key, u8 *iv)
{
    struct crypto_skcipher *tfm;
    struct skcipher_request *req = NULL;
    struct scatterlist sg;
    int ret;

    if (len % AES_BLOCK_SIZE != 0)
        return -EINVAL;

    tfm = crypto_alloc_skcipher("ecb(aes)", 0, 0);
    if (IS_ERR(tfm))
        return PTR_ERR(tfm);

    ret = crypto_skcipher_setkey(tfm, key, AES_KEY_LEN);
    if (ret)
        goto out;

    req = skcipher_request_alloc(tfm, GFP_KERNEL);
    if (!req) {
        ret = -ENOMEM;
        goto out;
    }

    sg_init_one(&sg, data, len);
    skcipher_request_set_crypt(req, &sg, &sg, len, NULL);

    ret = crypto_skcipher_decrypt(req);

    skcipher_request_free(req);
out:
    crypto_free_skcipher(tfm);
    return ret;
}

#endif

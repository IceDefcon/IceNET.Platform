/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include <linux/crypto.h>
#include <linux/scatterlist.h>

#include "crypto.h"

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

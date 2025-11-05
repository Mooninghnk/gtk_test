
#include <openssl/evp.h>
#include <openssl/ripemd.h>
#include <openssl/sha.h>
#include <secp256k1.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitcoin.h"


static const char BASE58_ALPHABET[] =
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

    int derive_pubkey(const unsigned char *privkey, unsigned char *pubkey_compressed) {
        secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);

    }

#include <openssl/evp.h>
#include <openssl/ripemd.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <secp256k1.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitcoin.h"

// Constants
#define WIF_LENGTH 52
#define PRIVKEY_SIZE 32
#define PUBKEY_COMPRESSED_SIZE 33
#define HASH160_SIZE 20
#define CHECKSUM_SIZE 4

static const char BASE58_ALPHABET[] =
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

// secp256k1 curve order (private keys must be less than this)
static const unsigned char CURVE_ORDER[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
    0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B,
    0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x41
};

// Global secp256k1 context (created once, reused)
static secp256k1_context *global_ctx = NULL;

// Initialize secp256k1 context
int bitcoin_init(void) {
    if (global_ctx != NULL) {
        return 1; // Already initialized
    }

    global_ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    if (global_ctx == NULL) {
        return 0;
    }

    // Randomize context for additional security
    unsigned char seed[32];
    if (RAND_bytes(seed, 32) == 1) {
        int ret = secp256k1_context_randomize(global_ctx, seed);
        OPENSSL_cleanse(seed, 32);
        if (!ret) {
            secp256k1_context_destroy(global_ctx);
            global_ctx = NULL;
            return 0;
        }
    }

    return 1;
}

// Cleanup secp256k1 context
void bitcoin_cleanup(void) {
    if (global_ctx != NULL) {
        secp256k1_context_destroy(global_ctx);
        global_ctx = NULL;
    }
}

// Check if private key is valid (not zero, less than curve order)
static int is_valid_privkey(const unsigned char *privkey) {
    // Check if key is all zeros
    int all_zero = 1;
    for (int i = 0; i < PRIVKEY_SIZE; i++) {
        if (privkey[i] != 0) {
            all_zero = 0;
            break;
        }
    }
    if (all_zero) return 0;

    // Check if key is less than curve order
    for (int i = 0; i < PRIVKEY_SIZE; i++) {
        if (privkey[i] < CURVE_ORDER[i]) {
            return 1;
        } else if (privkey[i] > CURVE_ORDER[i]) {
            return 0;
        }
    }
    return 0; // Equal to curve order is invalid
}

// Generate a random private key
int generate_priv_key(unsigned char *privkey) {
    int attempts = 0;
    const int MAX_ATTEMPTS = 10000;

    while (attempts < MAX_ATTEMPTS) {
        if (RAND_bytes(privkey, PRIVKEY_SIZE) != 1) {
            return 0;
        }

        if (is_valid_privkey(privkey)) {
            return 1;
        }
        attempts++;
    }

    return 0; // Failed to generate valid key
}

// Derive compressed public key from private key using secp256k1
int derive_pubkey(const unsigned char *privkey, unsigned char *pubkey_compressed) {
    if (global_ctx == NULL) {
        fprintf(stderr, "Error: Bitcoin library not initialized. Call bitcoin_init() first.\n");
        return 0;
    }

    if (!is_valid_privkey(privkey)) {
        return 0;
    }

    secp256k1_pubkey pubkey;

    if (!secp256k1_ec_pubkey_create(global_ctx, &pubkey, privkey)) {
        return 0;
    }

    size_t output_len = PUBKEY_COMPRESSED_SIZE;
    secp256k1_ec_pubkey_serialize(global_ctx, pubkey_compressed, &output_len, &pubkey,
                                   SECP256K1_EC_COMPRESSED);

    return 1;
}

// Compute RIPEMD160(SHA256(input))
void hash160(const unsigned char *input, size_t len, unsigned char *output) {
    unsigned char sha256_result[32];
    SHA256(input, len, sha256_result);
    RIPEMD160(sha256_result, 32, output);
    OPENSSL_cleanse(sha256_result, 32);
}

// Base58 encoding with checksum
char *base58_encode(const unsigned char *data, size_t len) {
    if (data == NULL || len == 0) {
        return NULL;
    }

    // Add 4-byte checksum
    unsigned char *extended = malloc(len + CHECKSUM_SIZE);
    if (!extended) return NULL;

    memcpy(extended, data, len);

    unsigned char hash[32];
    SHA256(data, len, hash);
    SHA256(hash, 32, hash);
    memcpy(extended + len, hash, CHECKSUM_SIZE);
    OPENSSL_cleanse(hash, 32);

    size_t extended_len = len + CHECKSUM_SIZE;

    // Count leading zeros
    size_t zero_count = 0;
    while (zero_count < extended_len && extended[zero_count] == 0) {
        zero_count++;
    }

    // Allocate output buffer (worst case: each byte becomes ~1.38 base58 digits)
    size_t max_size = extended_len * 2; // Conservative estimate
    unsigned char *temp = calloc(max_size, 1);
    if (!temp) {
        free(extended);
        return NULL;
    }

    // Convert to base58
    size_t output_size = 0;
    for (size_t i = 0; i < extended_len; i++) {
        int carry = extended[i];
        for (size_t j = 0; j < output_size || carry; j++) {
            if (j >= max_size) break;
            carry += 256 * temp[j];
            temp[j] = carry % 58;
            carry /= 58;
            if (j >= output_size) output_size = j + 1;
        }
    }

    // Convert to Base58 characters
    char *result = malloc(zero_count + output_size + 1);
    if (!result) {
        free(temp);
        free(extended);
        return NULL;
    }

    for (size_t i = 0; i < zero_count; i++) {
        result[i] = '1';
    }

    for (size_t i = 0; i < output_size; i++) {
        result[zero_count + i] = BASE58_ALPHABET[temp[output_size - 1 - i]];
    }
    result[zero_count + output_size] = '\0';

    free(temp);
    free(extended);
    return result;
}

// Convert private key to WIF format
int private_key_to_wif(const unsigned char *privkey, char *wif_out) {
    unsigned char extended[34];
    extended[0] = 0x80; // Mainnet prefix
    memcpy(extended + 1, privkey, PRIVKEY_SIZE);
    extended[33] = 0x01; // Compressed flag

    char *wif = base58_encode(extended, 34);
    if (!wif) {
        return 0;
    }

    size_t wif_len = strlen(wif);
    if (wif_len > WIF_LENGTH) {
        free(wif);
        return 0;
    }

    snprintf(wif_out, WIF_LENGTH + 1, "%s", wif);
    free(wif);
    return 1;
}

// Generate Bitcoin address from compressed public key
char *generate_address(const unsigned char *pubkey_compressed, int testnet) {
    unsigned char hash160_result[HASH160_SIZE];
    hash160(pubkey_compressed, PUBKEY_COMPRESSED_SIZE, hash160_result);

    unsigned char versioned[21];
    versioned[0] = testnet ? 0x6F : 0x00; // Testnet or mainnet prefix
    memcpy(versioned + 1, hash160_result, HASH160_SIZE);

    char *address = base58_encode(versioned, 21);
    OPENSSL_cleanse(hash160_result, HASH160_SIZE);

    return address;
}

// High-level wallet creation
BitcoinWallet *bitcoin_wallet_create(void) {
    if (global_ctx == NULL) {
        fprintf(stderr, "Error: Bitcoin library not initialized. Call bitcoin_init() first.\n");
        return NULL;
    }

    BitcoinWallet *wallet = malloc(sizeof(BitcoinWallet));
    if (!wallet) return NULL;

    // Initialize to zero
    memset(wallet, 0, sizeof(BitcoinWallet));

    // Generate private key
    if (!generate_priv_key(wallet->privkey)) {
        free(wallet);
        return NULL;
    }

    // Derive public key
    if (!derive_pubkey(wallet->privkey, wallet->pubkey)) {
        OPENSSL_cleanse(wallet->privkey, PRIVKEY_SIZE);
        free(wallet);
        return NULL;
    }

    // Convert to hex strings
    for (int i = 0; i < PRIVKEY_SIZE; i++) {
        sprintf(wallet->privkey_hex + i * 2, "%02x", wallet->privkey[i]);
    }
    wallet->privkey_hex[64] = '\0';

    for (int i = 0; i < PUBKEY_COMPRESSED_SIZE; i++) {
        sprintf(wallet->pubkey_hex + i * 2, "%02x", wallet->pubkey[i]);
    }
    wallet->pubkey_hex[66] = '\0';

    // Generate WIF
    if (!private_key_to_wif(wallet->privkey, wallet->wif)) {
        OPENSSL_cleanse(wallet->privkey, PRIVKEY_SIZE);
        OPENSSL_cleanse(wallet->privkey_hex, 64);
        free(wallet);
        return NULL;
    }

    // Generate addresses
    wallet->mainnet_address = generate_address(wallet->pubkey, 0);
    wallet->testnet_address = generate_address(wallet->pubkey, 1);

    if (!wallet->mainnet_address || !wallet->testnet_address) {
        bitcoin_wallet_free(wallet);
        return NULL;
    }

    return wallet;
}

// Free wallet memory securely
void bitcoin_wallet_free(BitcoinWallet *wallet) {
    if (wallet) {
        // Securely wipe sensitive data
        OPENSSL_cleanse(wallet->privkey, PRIVKEY_SIZE);
        OPENSSL_cleanse(wallet->privkey_hex, 64);
        OPENSSL_cleanse(wallet->wif, WIF_LENGTH);

        free(wallet->mainnet_address);
        free(wallet->testnet_address);
        free(wallet);
    }
}

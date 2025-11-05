#ifndef BITCOIN_H
#define BITCOIN_H
#include <stddef.h>


typedef struct {
    unsigned char privkey[32];
    unsigned char pubkey[33];
    char privkey_hex[65];
    char pubkey_hex[67];
    char wif[53];
    char *mainnet_address;
    char *testnet_address;
} BitcoinWallet;

void generate_priv_key(unsigned char *privkey);
int derive_pubkey(const unsigned char *privkey, unsigned char *pubkey_compressed);
void private_key_to_wif(const unsigned char *privkey, char *wif_out);
char *generate_address(const unsigned char *pubkey_compressed, int testnet);

// Helper functions
void hash160(const unsigned char *input, size_t len, unsigned char *output);
char *base58_encode(const unsigned char *data, size_t len);

// High-level wallet generation
BitcoinWallet *bitcoin_wallet_create(void);
void bitcoin_wallet_free(BitcoinWallet *wallet)

#endif

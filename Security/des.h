#ifndef DES_H
#define DES_H

#include <stdint.h>

/* DES шифрование */
//==================================================================================

class DES {
 private:
  static void initialPermutation(const uint8_t in[8], uint32_t out[2]);
  static void finalPermutation(const uint32_t in[2], uint8_t out[8]);
  static void generateIterationSubkeys(const uint8_t key[8],
                                       uint8_t subkeys[16][8]);
  static uint32_t f(const uint32_t* R, const uint8_t subkey[8]);

 public:
  // DES шифрование
  static void encryption(const uint8_t val[8],
                         const uint8_t key[8],
                         uint8_t result[8]);

  // DES дешифрование
  static void decryption(const uint8_t val[8],
                         const uint8_t key[8],
                         uint8_t result[8]);
};

//==================================================================================

/* TDES шифрование */
//==================================================================================

class TDES {
 public:
  // TDES шифрование типа EDE2
  static void EDE2_encryption(const uint8_t val[8],
                              const uint8_t key1[8],
                              const uint8_t key2[8],
                              uint8_t result[8]);

  // TDES дешифрование типа EDE2
  static void EDE2_decryption(const uint8_t val[8],
                              const uint8_t key1[8],
                              const uint8_t key2[8],
                              uint8_t result[8]);
};

//==================================================================================

#endif  // DES_H

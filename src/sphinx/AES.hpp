/*
# AES encryption

    We are using Intel's AES-NI instruction set. This code is based on the official guide from Intel!
    For our purposes we only care about AES128, ECB - which should be perfectly adequate for small
    strings such as passwords, since those are not big enough to be easily defeated with repeating
    pattern analysis.
*/

namespace sphinx {

    // We are doing AES128 (EBC).
    constexpr int AES_BLOCK_SIZE         = 16;
    constexpr int AES_KEY_SCHEDULE_COUNT = 11;

    /*
        Expand the user key to a key schedule (11 keys).
    */
    void
    aes128_expand_key(const uint8_t *userkey, uint8_t *key);

}

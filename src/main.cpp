
#include <sphinx/AES.hpp>

int main() {
    using namespace sphinx;

    // Initialize AES context
    AES128_Context ctx;
    aes128_init(&ctx, "My key");

    // Test data (all 16 byte):
    u8 original[BLOCK_SIZE] = "Hello, World!42";
    u8 encrypted[BLOCK_SIZE];
    u8 decrypted[BLOCK_SIZE];

    // Encrypt:
    aes128_encrypt(&ctx, encrypted, original);

    // Decrypt:
    aes128_decrypt(&ctx, decrypted, encrypted);

    cout << "Decrypted:\n";
    for (u8 c : decrypted) {
        cout << c;
    }
    cout << "\n";

    return 0;
}

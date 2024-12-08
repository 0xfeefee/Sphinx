
#include <sphinx/AES.hpp>

int main() {
    using namespace sphinx;

    // Initialize AES context
    AES128 aes("My key");

    AES_String text("Lorem Ipsum, sit Dolor Amet, bla bla bla, long string and stuff!");
    AES_String text2(text);

    text.print();

    // Test data (all 16 byte):
    u8 original[BLOCK_SIZE] = "Hello, World!42";
    u8 encrypted[BLOCK_SIZE];
    u8 decrypted[BLOCK_SIZE];

    // Encrypt:
    aes.encrypt(original, encrypted);

    // Decrypt:
    aes.decrypt(encrypted, decrypted);

    cout << "Decrypted:\n";
    for (u8 c : decrypted) {
        cout << c;
    }
    cout << "\n";

    return 0;
}

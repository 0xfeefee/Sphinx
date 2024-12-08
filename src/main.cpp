
#include <sphinx/AES.hpp>

int main() {
    using namespace sphinx;

    // Initialize AES context
    AES128 aes("Harpocrates");
    AES_String text(
        R"(The world is indeed full of peril, and in it there are many dark places;
        but still there is much that is fair, and though in all lands love is now mingled with grief,
        it grows perhaps the greater.)"
    );

    AES_String cipher(text);
    AES_String output(text.block_count*BLOCK_SIZE);

    aes.encrypt(text, cipher);
    aes.decrypt(cipher, output);

    cout << "Plain text: ";
    text.print();
    cout << "Cipher text: ";
    cipher.print();
    cout << "Output text: ";
    output.print();

    return 0;
}

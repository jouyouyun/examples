#include <iostream>
#include <cryptopp/sha.h>

using namespace std;
using namespace CryptoPP;

string sha256_encode(const string& data);
bool sha256_verify(const string& data, const string& digest);
string string_to_hex(const string& input);

int main()
{
    string data("Hello, world");
    string digest = sha256_encode(data);
    cout<<"Result: "<<string_to_hex(digest)<<endl;
    cout<<"Verify: "<<sha256_verify(data, digest)<<endl;
    return 0;
}

string sha256_encode(const string& data)
{
    SHA256 sha;
    const byte* in_digest = (byte*)data.data();
    byte digest[SHA256::DIGESTSIZE];
    sha.CalculateDigest(digest, in_digest, SHA256::DIGESTSIZE);
    // return string((char*)digest);
    return string((char*)digest, SHA256::DIGESTSIZE);
}

bool sha256_verify(const string& data, const string& digest)
{
    SHA256 sha;
    const byte* by_digest = (byte*)digest.data();
    const byte* in_digest = (byte*)data.data();
    return sha.VerifyDigest(by_digest, in_digest, SHA256::DIGESTSIZE);
}

string string_to_hex(const string& input)
{
    static const char* const lut = "0123456789abcdef";
    size_t len = input.length();

    string output;
    output.reserve(2 * len);
    for (size_t i = 0; i < len; ++i)
    {
        const unsigned char c = input[i];
        // the first 4 bytes
        output.push_back(lut[c >> 4]);
        output.push_back(lut[c & 15]);
    }
    return output;
}

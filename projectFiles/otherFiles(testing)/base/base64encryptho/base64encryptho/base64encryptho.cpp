#include <vector>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

vector<unsigned char> readBinaryFile(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Failed to open file: " << filename << endl;
        return {};
    }

    return vector<unsigned char>((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

string base64Encode(const vector<unsigned char>& data) {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    string result;
    int val = 0, valb = -6;

    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(table[(val >> valb) & 0x3f]);
            valb -= 6;
        }
    }

    if (valb > -6) result.push_back(table[((val << 8) >> (valb + 8)) & 0x3F]);
    while (result.size() % 4) result.push_back('=');

    return result;
}

void writeToFile(const string& baseStr, const string& path) {
    ofstream wFile(path);
    if (!wFile) {
        cout << "Failed to open write File: " << path << endl;
    }
    else {
        wFile << baseStr;
        cout << "Wrote to file";
    }
}

int main() {
    string path = "C:\\Users\\Bryson Blakney\\Desktop\\boyscoutFinal.png";
    string writePath = "C:\\Users\\Bryson Blakney\\Desktop\\base.txt";

    auto imageData = readBinaryFile(path);
    if (imageData.empty()) {
        cerr << "No image data read.\n";
        return 1;
    }

    cout << "Read " << imageData.size() << " bytes from image.\n";

    string baseStr = base64Encode(imageData);

    cout << "Base64 string starts with:\n" << baseStr.substr(0, 100) << "...\n";
    cout << "Total length: " << baseStr.length() << " characters\n";

    writeToFile(baseStr, writePath);

    return 0;
}

#include <map>
#include <iostream>

using namespace std;

int main() {
    map<string, string> word_count;
    word_count["pedro"] = "hello";
    // 判断元素是否存在，最好使用 find API
    auto it = word_count.find("pedro");
    if (it != word_count.end()) {
        cout << "pedro\n";
    }
    it = word_count.find("mike");
    if (it != word_count.end()) {
        cout << "mike\n";
    }
    return 0;
}


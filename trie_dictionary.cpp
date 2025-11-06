#include <bits/stdc++.h>
using namespace std;

class Dictionary {
private:
    static const int ALPHABET = 26;

    struct TrieNode {
        bool isEnd = false;
        TrieNode* next[ALPHABET];

        TrieNode() {
            memset(next, 0, sizeof(next));
        }
    };

    TrieNode* root;
    unordered_map<string, string> meanings;

    inline int toIndex(char c) const {
        return c - 'a';
    }

    inline char toChar(int i) const {
        return 'a' + i;
    }

    // Insert a word into the Trie
    void trieInsert(const string& word) {
        TrieNode* cur = root;
        for (char c : word) {
            int idx = toIndex(c);
            if (!cur->next[idx])
                cur->next[idx] = new TrieNode();
            cur = cur->next[idx];
        }
        cur->isEnd = true;
    }

    // Move to prefix node
    TrieNode* trieGoto(const string& prefix) const {
        TrieNode* cur = root;
        for (char c : prefix) {
            int idx = toIndex(c);
            if (!cur->next[idx]) return nullptr;
            cur = cur->next[idx];
        }
        return cur;
    }

    // DFS collect in lexicographic order
    void collectWords(TrieNode* node, string& path, int n, vector<string>& out) const {
        if (!node || out.size() >= (size_t)n) return;

        if (node->isEnd) {
            out.push_back(path);
            if (out.size() >= (size_t)n) return;
        }

        for (int i = 0; i < ALPHABET; i++) {
            if (node->next[i]) {
                path.push_back(toChar(i));
                collectWords(node->next[i], path, n, out);
                path.pop_back();

                if (out.size() >= (size_t)n) return;
            }
        }
    }

    // Wildcard DFS for pattern with '.' characters
    bool existsDFS(TrieNode* node, const string& pat, int idx) const {
        if (!node) return false;

        if (idx == pat.size())
            return node->isEnd;

        char c = pat[idx];

        if (c == '.') {
            // Try all 26 options
            for (int i = 0; i < ALPHABET; i++) {
                if (node->next[i] &&
                    existsDFS(node->next[i], pat, idx + 1))
                    return true;
            }
            return false;
        }

        // exact match
        int id = toIndex(c);
        return existsDFS(node->next[id], pat, idx + 1);
    }

public:
    Dictionary() {
        root = new TrieNode();
    }

    // 1) Insert or update a word
    void storeWord(const string& word, const string& meaning) {
        if (meanings.find(word) == meanings.end()) {
            trieInsert(word);
        }
        meanings[word] = meaning;  // overwrite/update
    }

    // 2) Exact meaning
    string getMeaning(const string& word) const {
        auto it = meanings.find(word);
        return it == meanings.end() ? "" : it->second;
    }

    // 3) Prefix search
    vector<string> searchWords(const string& prefix, int n) const {
        vector<string> result;
        TrieNode* node = trieGoto(prefix);
        if (!node) return result;

        string path = prefix;
        collectWords(node, path, n, result);
        return result;
    }

    // 4) Wildcard exists
    bool exists(const string& pattern) const {
        return existsDFS(root, pattern, 0);
    }
};

/* ----------- Demo (remove for interviews) --------------- */
int main() {
    Dictionary dict;

    dict.storeWord("apple", "a fruit");
    dict.storeWord("apply", "to request formally");
    dict.storeWord("apt", "suitable");
    dict.storeWord("cat", "animal");

    cout << dict.getMeaning("apple") << endl;        // a fruit
    cout << boolalpha << dict.exists("ap.le") << endl; // true
    cout << dict.exists("a....") << endl;              // true (apple, apply)
    cout << dict.exists("c.t") << endl;                // true
    cout << dict.exists(".....") << endl;              // true (apple/apply)

    auto v = dict.searchWords("ap", 5);
    for (auto& w : v) cout << w << endl;             // apple, apply, apt

    return 0;
}

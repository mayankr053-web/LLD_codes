#include <bits/stdc++.h>
using namespace std;

class SearchAutocomplete {
private:
    struct TrieNode {
        unordered_map<char, TrieNode*> children;
        unordered_map<string, int> freqMap;   // sentence -> frequency
    };

    TrieNode* root;
    string currentInput;

public:
    SearchAutocomplete(vector<string>& phrases, vector<int>& counts) {
        root = new TrieNode();
        currentInput = "";

        for (int i = 0; i < phrases.size(); i++) {
            insert(phrases[i], counts[i]);
        }
    }

    // Insert sentence into Trie, updating frequency
    void insert(const string& sentence, int count) {
        TrieNode* node = root;

        for (char c : sentence) {
            if (!node->children.count(c)) {
                node->children[c] = new TrieNode();
            }
            node = node->children[c];

            node->freqMap[sentence] += count;
        }
    }

    // Get top 3 sentences from freqMap
    vector<string> top3(unordered_map<string, int>& freqMap) {
        // Min-heap: lowest freq at top
        auto cmp = [&](const string& a, const string& b) {
            if (freqMap[a] != freqMap[b])
                return freqMap[a] > freqMap[b];  // smaller freq first
            return a < b; // reverse ASCII for tie
        };

        priority_queue<string, vector<string>, decltype(cmp)> pq(cmp);

        for (auto& p : freqMap) {
            pq.push(p.first);
            if (pq.size() > 3)
                pq.pop();
        }

        vector<string> result;
        while (!pq.empty()) {
            result.push_back(pq.top());
            pq.pop();
        }
        reverse(result.begin(), result.end());  // highest freq first
        return result;
    }

    vector<string> getSuggestions(char ch) {
        if (ch == '#') {
            insert(currentInput, 1);
            currentInput = "";
            return {};
        }

        currentInput.push_back(ch);

        TrieNode* node = root;
        for (char c : currentInput) {
            if (!node->children.count(c))
                return {};  // No matches
            node = node->children[c];
        }

        return top3(node->freqMap);
    }
};

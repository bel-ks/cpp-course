#ifndef HUF_TREE_H
#define HUF_TREE_H

#include <vector>
#include <queue>
#include <cstdint>
#include <algorithm>

struct huf_tree {
    struct Node {
        uint8_t sym;
        Node *left, *right;

        Node(): sym(0), left(nullptr), right(nullptr) {}
        Node(uint8_t _sym): sym(_sym), left(nullptr), right(nullptr) {}
        Node(uint8_t _sym, Node *_left, Node *_right): sym(_sym), left(_left), right(_right) {}
        bool isTerm() const;
    };

    huf_tree(std::vector<uint64_t> const& cntSym);

    uint8_t getSym() const;
    uint64_t getNum() const;
    Node* getRoot() const;
    bool go(bool to);
    void reset();

private:
    Node *root;
    Node *curNode;
    size_t numOfSymb;
};


#endif //HUF_TREE_H

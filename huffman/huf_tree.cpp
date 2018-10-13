#include "huf_tree.h"

bool cmp(std::pair<uint64_t, huf_tree::Node*> &firN, std::pair<uint64_t, huf_tree::Node*> &secN) {
    return firN.first < secN.first || (firN.first == secN.first && firN.second->sym < secN.second->sym);
}

std::pair<uint64_t, huf_tree::Node*> getMin(size_t &indL, size_t &indNew,
        std::vector<std::pair<uint64_t, huf_tree::Node*> > &leaves,
        std::vector<std::pair<uint64_t, huf_tree::Node*> > &newNodes) {
    if (leaves[indL].first == newNodes[indNew].first) {
        if (leaves[indL].second->sym < newNodes[indNew].second->sym) {
            ++indL;
            return leaves[indL - 1];
        } else {
            ++indNew;
            return newNodes[indNew - 1];
        }
    }
    if (leaves[indL].first < newNodes[indNew].first) {
        ++indL;
        return leaves[indL - 1];
    } else {
        ++indNew;
        return newNodes[indNew - 1];
    }
}

huf_tree::huf_tree(std::vector<uint64_t> const &cntSym) {
    std::vector<std::pair<uint64_t, Node*> > leaves(257), newNodes(256);
    size_t sizeL = 0, sizeNew = 0, indL = 0, indNew = 0;
    numOfSymb = 2;
    leaves[sizeL] = { cntSym[0], new Node(0) };
    ++sizeL;
    leaves[sizeL] = { cntSym[1], new Node(1) };
    ++sizeL;
    for (uint8_t ind = 2; ind != 0; ++ind) {
        if (cntSym[ind]) {
            leaves[sizeL] = { cntSym[ind], new Node(ind) };
            ++sizeL;
            ++numOfSymb;
        }
    }
    std::sort(leaves.begin(), leaves.begin() + sizeL, cmp);
    leaves[sizeL] = { (uint64_t)-1, nullptr };
    newNodes[sizeNew] = { (uint64_t)-1, nullptr };
    while (sizeL - indL + sizeNew - indNew > 1) {
        auto firNode = getMin(indL, indNew, leaves, newNodes);
        auto secNode = getMin(indL, indNew, leaves, newNodes);
        newNodes[sizeNew] = { firNode.first + secNode.first, new Node(firNode.second->sym, firNode.second, secNode.second) };
        ++sizeNew;
        newNodes[sizeNew] = { (uint64_t)-1, nullptr };
    }
    root = newNodes[indNew].second;
    size_t ind = 0;
    for (; ind < 256; ++ind) {
        if (!leaves[ind].second) {
            delete leaves[ind].second;
        }
        if (!newNodes[ind].second) {
            delete newNodes[ind].second;
        }
    }
    if (!leaves[ind].second) {
        delete leaves[ind].second;
    }
}

bool huf_tree::Node::isTerm() const {
    return !left;
}

huf_tree::Node* huf_tree::getRoot() const {
    return root;
}

uint8_t huf_tree::getSym() const {
    return curNode->sym;
}

uint64_t huf_tree::getNum() const {
    return numOfSymb;
}

bool huf_tree::go(bool to) {
    if (!curNode->left) {
        return false;
    }
    if (to) {
        curNode = curNode->right;
    } else {
        curNode = curNode->left;
    }
    return true;
}

void huf_tree::reset() {
    curNode = root;
}
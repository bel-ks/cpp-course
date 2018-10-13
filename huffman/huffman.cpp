#include "huffman.h"

void huffman::encode(std::istream &in, std::ostream &out) {
    buffer_reader reader(in);
    buffer_writer writer(out);
    std::vector<uint64_t> cntSym(256, 0);
    uint8_t cur = 0;
    while (reader.readChar(cur)) {
        ++cntSym[cur];
    }
    reader.reset();
    huf_tree hufTree(cntSym);
    std::vector<std::pair<uint64_t, uint8_t> > codes = getCodes(hufTree.getRoot());
    writer.writeCnt(hufTree.getNum());
    uint8_t ind = 0, step = 64;
    uint64_t len = 0;
    do {
        if (codes[ind].first != (uint64_t)(-1)) {
            writer.writeChar(ind);
            writer.writeCnt(cntSym[ind]);
            len += cntSym[ind] * (uint64_t)codes[ind].second;
        }
        ++ind;
    } while (ind != 0);
    step = (step - (uint8_t)(len % 64)) % (uint8_t)64;
    writer.writeChar(step);
    if (len % (uint64_t)64) {
        len /= (uint64_t)64;
        ++len;
    } else {
        len /= (uint64_t)64;
    }
    writer.writeCnt(len);
    uint64_t text = 0, shift = 0;
    while (reader.readChar(cur)) {
        if (shift + (uint64_t)codes[cur].second > (uint64_t)64) {
            text <<= ((uint64_t)64 - shift);
            shift = (uint64_t)codes[cur].second - ((uint64_t)64 - shift);
            text += (codes[cur].first >> shift);
            writer.writeCnt(text);
            text = codes[cur].first & ((1 << shift) - 1);
        } else {
            text <<= (uint64_t)codes[cur].second;
            text += codes[cur].first;
            shift += (uint64_t)codes[cur].second;
        }
    }
    if (len) {
        writer.writeCnt(text << (uint64_t)step);
    }
}

bool huffman::decode(std::istream &in, std::ostream &out) {
    buffer_reader reader(in);
    buffer_writer writer(out);
    size_t num = 0;
    if (!reader.readCnt(num)) {
        return false;
    }
    std::vector<uint64_t> cntSym(256, 0);
    uint8_t cur = 0, step = 0;
    while (num) {
        if (!reader.readChar(cur) || cntSym[cur] || !reader.readCnt(cntSym[cur])) {
            return false;
        }
        --num;
    }
    huf_tree hufTree(cntSym);
    if (!reader.readChar(step) || !reader.readCnt(num)) {
        return false;
    }
    uint64_t text = 0;
    huf_tree::Node *curNode = hufTree.getRoot();
    while (num > 1) {
        if (!reader.readCnt(text)) {
            return false;
        }
        for (uint64_t sh = 64; sh > 0; --sh) {
            if (text & ((uint64_t)1 << (sh - 1))) {
                curNode = curNode->right;
            } else {
                curNode = curNode->left;
            }
            if (curNode->isTerm()) {
                writer.writeChar(curNode->sym);
                curNode = hufTree.getRoot();
            }
        }
        text = 0;
        --num;
    }
    if (num) {
        if (!reader.readCnt(text)) {
            return false;
        }
        for (uint64_t sh = 64; sh > step; --sh) {
            if (text & ((uint64_t) 1 << (sh - 1))) {
                curNode = curNode->right;
            } else {
                curNode = curNode->left;
            }
            if (curNode->isTerm()) {
                writer.writeChar(curNode->sym);
                curNode = hufTree.getRoot();
            }
        }
    }
    return (curNode == hufTree.getRoot()) && !reader.readChar(cur);
}

void dfs(huf_tree::Node *curNode, uint64_t &code, uint8_t &len, std::vector<std::pair<uint64_t, uint8_t> > &codes) {
    if (curNode->isTerm()) {
        codes[curNode->sym] = { code, len };
        return;
    }
    code <<= 1;
    ++len;
    dfs(curNode->left, code, len, codes);
    ++code;
    dfs(curNode->right, code, len, codes);
    code >>= 1;
    --len;
}

std::vector<std::pair<uint64_t, uint8_t> > huffman::getCodes(huf_tree::Node* const &root) {
    std::vector<std::pair<uint64_t, uint8_t> > codes(256, {(uint64_t)(-1), 0} );
    huf_tree::Node *curNode = root;
    uint64_t code = 0;
    uint8_t len = 0;
    dfs(curNode, code, len, codes);
    return codes;
}

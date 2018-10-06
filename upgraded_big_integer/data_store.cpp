#include "data_store.h"

void storage::is_unique() {
    if (!large.unique()) {
        large = std::make_shared<std::vector<uint32_t> >(*large);
    }
}

storage::storage(): small(0), _len(0) {}

storage::~storage() {
    clear();
}

void storage::clear() {
    resize(0);
}

bool storage::empty() const {
    return _len == 0;
}

uint32_t storage::back() const {
    if (_len > 1) {
        return large->operator[](_len - 1);
    }
    return small;
}

uint32_t &storage::back() {
    if (_len > 1) {
        is_unique();
        return large->operator[](_len - 1);
    }
    return small;
}

size_t storage::size() const {
    return _len;
}

void storage::push_back(uint32_t val) {
    if (_len > 1) {
        is_unique();
        large->push_back(val);
    } else if (_len) {
        new(&large) std::shared_ptr<std::vector<uint32_t> >(new std::vector<uint32_t>(1, small));
        large->push_back(val);
    } else {
        small = val;
    }
    ++_len;
}

void storage::pop_back() {
    if (_len <= 1) {
        small = 0;
    } else if (_len == 2) {
        auto t = large->operator[](0);
        large.reset();
        small = t;
    } else {
        is_unique();
        large->pop_back();
    }
    --_len;
}

void storage::resize(size_t len) {
    if (_len > 1) {
        if (len < 2) {
            auto t = operator[](0);
            large.reset();
            small = t;
        } else {
            is_unique();
            large->resize(len);
        }
    } else if (len > 1) {
        new(&large) std::shared_ptr<std::vector<uint32_t> >(new std::vector<uint32_t>(len, small));
    }
    _len = len;
}

uint32_t storage::operator[](size_t ind) const {
    if (_len > 1) {
        return large->operator[](ind);
    }
    return small;
}

uint32_t &storage::operator[](size_t ind) {
    if (_len > 1) {
        is_unique();
        return large->operator[](ind);
    }
    return small;
}

storage &storage::operator=(storage const &other) {
    clear();
    if (other._len > 1) {
        new(&large) std::shared_ptr<std::vector<uint32_t> >(other.large);
    } else {
        small = other.small;
    }
    _len = other._len;
    return *this;
}

bool operator==(storage const &a, storage const &b) {
    if ((a._len > 1) ^ (b._len > 1)) {
        return false;
    }
    if (a._len > 1) {
        return *a.large == *b.large;
    }
    return a.small == b.small;
}
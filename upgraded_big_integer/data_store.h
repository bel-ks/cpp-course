#ifndef DATA_STORE_H
#define DATA_STORE_H

#include <memory>
#include <vector>

struct storage {
    storage();
    ~storage();

    void clear();
    bool empty() const;
    uint32_t back() const;
    uint32_t &back();
    size_t size() const;
    void push_back(uint32_t val);
    void pop_back();
    void resize(size_t len);

    uint32_t operator[](size_t ind) const;
    uint32_t &operator[](size_t ind);
    storage &operator=(storage const &other);

    friend bool operator==(storage const &a, storage const &b);

private:
    union {
        uint32_t small;
        std::shared_ptr<std::vector<uint32_t > > large;
    };
    size_t _len;

    void is_unique();
};

bool operator==(storage const& a, storage const& b);

#endif //DATA_STORE_H

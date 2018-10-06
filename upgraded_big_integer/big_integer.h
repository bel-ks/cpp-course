#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include "data_store.h"
#include <functional>
#include <algorithm>

struct big_integer {
    big_integer();
    big_integer(big_integer const& other);
    big_integer(int a);
    big_integer(uint32_t a);
    explicit big_integer(std::string const& str);
    ~big_integer();

    big_integer& operator+=(big_integer const& rhs);
    big_integer& operator-=(big_integer const& rhs);
    big_integer& operator*=(big_integer const& rhs);
    big_integer& operator/=(big_integer const& rhs);
    big_integer& operator%=(big_integer const& rhs);

    big_integer& anyBitOp(big_integer const& rhs, std::function<uint32_t(uint32_t, uint32_t)> fun);
    big_integer& operator&=(big_integer const& rhs);
    big_integer& operator|=(big_integer const& rhs);
    big_integer& operator^=(big_integer const& rhs);

    big_integer& operator<<=(int rhs);
    big_integer& operator>>=(int rhs);

    big_integer operator+() const;
    big_integer operator-() const;
    big_integer operator~() const;

    big_integer& operator++();
    big_integer const operator++(int);

    big_integer& operator--();
    big_integer const operator--(int);

    friend bool operator==(big_integer const& a, big_integer const& b);
    friend bool operator!=(big_integer const& a, big_integer const& b);
    friend bool operator<(big_integer const& a, big_integer const& b);
    friend bool operator>(big_integer const& a, big_integer const& b);
    friend bool operator<=(big_integer const& a, big_integer const& b);
    friend bool operator>=(big_integer const& a, big_integer const& b);

    friend big_integer operator*(big_integer const& a, uint32_t const& b);

    friend void abs(big_integer &num, bool const& sign);
    friend std::pair<big_integer, uint32_t> divLongShort(big_integer const& a, uint32_t const& b);
    friend uint32_t getTrial(uint32_t const& a, uint32_t const& b, uint32_t const& c);
    friend std::string to_string(big_integer const& a);

private:
    storage data;

    bool getSign() const;
    bool getPossibleSign() const;
    uint32_t getNull() const;
    void normalize();
    void shift(int rhs);
    size_t len() const;
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
std::istream& operator>>(std::istream& s, big_integer& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);

#endif //BIG_INTEGER_H

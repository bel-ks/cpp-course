#include "big_integer.h"

const uint32_t logBase = 32;
const uint64_t base = ((uint64_t)1 << logBase);

bool big_integer::getSign() const {
    uint32_t bit = data.back() & ((uint32_t)1 << (logBase - 1));
    return !data.empty() && bit;
}

bool big_integer::getPossibleSign() const {
    size_t size = len();
    uint32_t bit = data[size - 2] & ((uint32_t)1 << (logBase - 1));
    return size > 1 && bit;
}

uint32_t big_integer::getNull() const {
    if (getSign()) {
        return (uint32_t)(base - 1);
    }
    return 0;
}

void big_integer::normalize() {
    while (!data.empty() && data.back() == getNull() && getSign() == getPossibleSign()) {
        data.pop_back();
    }
}

size_t big_integer::len() const {
    return data.size();
}

void abs(big_integer &num, bool const &sign) {
    if (sign) {
        num = -num;
    }
}

big_integer::big_integer() {
    data.clear();
}

big_integer::big_integer(big_integer const &other) {
    data = other.data;
}

big_integer::big_integer(int a) {
    data.clear();
    if (a) {
        data.push_back((uint32_t) a);
    }
}

big_integer::big_integer(uint32_t a) {
    data.clear();
    if (a) {
        data.push_back(a);
    }
    if (getSign()) {
        data.push_back(0);
    }
}

big_integer::big_integer(std::string const &str) {
    data.clear();
    if (str.empty()) {
        *this = 0;
        return;
    }
    bool sign = (str[0] == '-');
    auto ind = (size_t)sign;
    uint64_t cur = 0;
    uint32_t pow10 = 1;
    for (; ind < str.size(); ++ind) {
        int8_t num = str[ind] - '0';
        if (pow10 < (uint32_t)1e9 && cur * 10 + num < base) {
            cur *= 10;
            cur += num;
            pow10 *= 10;
        } else {
            *this *= pow10;
            *this += (uint32_t) cur;
            cur = (uint64_t)num;
            pow10 = 10;
        }
    }
    *this *= pow10;
    *this += (uint32_t)cur;
    if (getSign()) {
        data.push_back(0);
    }
    abs(*this, sign);
}

big_integer::~big_integer() {
    data.clear();
}

big_integer operator*(big_integer const &a, uint32_t const &b) {
    big_integer mul = a;
    bool sign = mul.getSign();
    abs(mul, sign);
    mul.data.push_back(0);
    uint32_t carry = 0;
    size_t size = mul.len();
    for (size_t ind = 0; ind < size; ++ind) {
        uint64_t add = (uint64_t) mul.data[ind] * b + carry;
        mul.data[ind] = (uint32_t)(add & (base - 1));
        carry = (uint32_t)(add >> logBase);
    }
    mul.normalize();
    abs(mul, sign);
    return mul;
}

std::pair<big_integer, uint32_t> divLongShort(big_integer const &a, uint32_t const &b) {
    big_integer res = a;
    uint64_t cur, mod = 0;
    for (size_t ind = a.len(); ind > 0; --ind) {
        cur = (mod << logBase) + res.data[ind - 1];
        res.data[ind - 1] = (uint32_t)(cur / b);
        mod = (uint32_t)(cur % b);
    }
    res.normalize();
    return { res, mod };
}

big_integer &big_integer::operator+=(big_integer const &rhs) {
    size_t oldSize = len(),
            newSize = std::max(oldSize, rhs.len()) + 1;
    uint32_t zero = getNull(), add = 0;
    bool carry = false;
    for (size_t ind = 0; ind < newSize; ++ind) {
        if (ind == oldSize) {
            data.push_back(zero);
            ++oldSize;
        }
        add = rhs.getNull();
        if (ind < rhs.len()) {
            add = rhs.data[ind];
        }
        uint64_t posNum = (uint64_t)data[ind] + add + carry;
        carry = (posNum >= base);
        data[ind] = (uint32_t)posNum;
    }
    normalize();
    return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    return *this += -rhs;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    big_integer mul, t, fir = *this, sec = rhs;
    bool firSign = fir.getSign(), secSign = sec.getSign();
    abs(fir, firSign);
    abs(sec, secSign);
    size_t secSize = rhs.len();
    for (size_t ind = 0; ind < secSize; ++ind) {
        uint32_t sh = (uint32_t)ind * logBase;
        t = fir * sec.data[ind];
        t <<= sh;
        mul += t;
    }
    mul.normalize();
    abs(mul, firSign ^ secSign);
    return *this = mul;
}

uint32_t getNumber(storage const &v, size_t const &ind) {
    if (ind < v.size()) {
        return v[ind];
    }
    return 0;
}

uint32_t getTrial(uint32_t const &a, uint32_t const &b, uint32_t const &c) {
    uint64_t tr = ((uint64_t)a << logBase) + b;
    tr /= c;
    return (uint32_t)std::min(tr, base - 1);
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    big_integer fir = *this, sec = rhs, res;
    bool firSign = fir.getSign(), secSign = sec.getSign();
    abs(fir, firSign);
    abs(sec, secSign);
    if (fir < sec) {
        return *this = 0;
    }
    if (sec.len() == 1) {
        res = divLongShort(fir, sec.data[0]).first;
    } else {
        size_t secSize = sec.len();
        if (sec.data[secSize - 1] == 0) {
            --secSize;
        }
        auto f = (uint32_t)(base / (sec.data[secSize - 1] + 1));
        fir *= f;
        fir.normalize();
        sec *= f;
        sec.normalize();
        secSize = sec.len();
        if (sec.data[secSize - 1] == 0) {
            --secSize;
        }
        uint32_t div = sec.data[secSize - 1];
        size_t size = fir.len() - secSize + 1;
        big_integer cur = fir >> (logBase * size), mul;
        res.data.resize(size);
        for (size_t ind = size; ind > 0; --ind) {
            cur = (cur << logBase) + fir.data[ind - 1];
            uint32_t dig = getTrial(getNumber(cur.data, secSize), getNumber(cur.data, secSize - 1), div);
            mul = sec * dig;
            while (dig > 0 && cur < mul) {
                --dig;
                mul = sec * dig;
            }
            cur -= mul;
            res.data[ind - 1] = dig;
        }
    }
    res.normalize();
    abs(res, firSign ^ secSign);
    return *this = res;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    return *this -= *this / rhs * rhs;
}

big_integer &big_integer::anyBitOp(big_integer const &rhs, std::function<uint32_t(uint32_t, uint32_t)> fun) {
    size_t oldSize = len(),
           newSize = std::max(oldSize, rhs.len());
    uint32_t zero = getNull(), t;
    for (size_t ind = 0; ind < newSize; ++ind) {
        if (ind == oldSize) {
            data.push_back(zero);
            ++oldSize;
        }
        t = rhs.getNull();
        if (ind < rhs.len()) {
            t = rhs.data[ind];
        }
        data[ind] = fun(data[ind], t);
    }
    normalize();
    return *this;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    return anyBitOp(rhs, [](uint32_t a, uint32_t b) {
        return a & b;
    });
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    return anyBitOp(rhs, [](uint32_t a, uint32_t b) {
        return a | b;
    });
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    return anyBitOp(rhs, [](uint32_t a, uint32_t b) {
        return a ^ b;
    });
}

void big_integer::shift(int rhs) {
    size_t size = len();
    if (rhs < 0) {
        for (auto ind = (size_t)-rhs; ind < size; ++ind) {
            data[ind + rhs] = data[ind];
        }
        for (size_t ind = size; ind > size + rhs; --ind) {
            data[ind - 1] = getNull();
        }
        normalize();
    } else {
        data.resize(size + rhs);
        for (size_t ind = size; ind > 0; --ind) {
            data[ind + rhs - 1] = data[ind - 1];
        }
        for (auto ind = (size_t)rhs; ind > 0; --ind) {
            data[ind - 1] = 0;
        }
    }
}

big_integer &big_integer::operator<<=(int rhs) {
    if (rhs < 0) {
        return *this >>= -rhs;
    }
    int sh = rhs / logBase;
    shift(sh);
    sh = rhs - sh * logBase;
    if (sh) {
        data.push_back(getNull());
        size_t size = len();
        for (size_t ind = size; ind > 0; --ind) {
            if (ind != size) {
                data[ind] += (data[ind - 1] >> (logBase - sh));
            }
            data[ind - 1] <<= sh;
        }
    }
    normalize();
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    if (rhs < 0) {
        return *this <<= -rhs;
    }
    int sh = rhs / logBase;
    shift(-sh);
    sh = rhs - sh * logBase;
    if (sh) {
        uint32_t cur = getNull();
        size_t size = len();
        for (size_t ind = 0; ind < size; ++ind) {
            if (ind) {
                data[ind - 1] += (data[ind] << (logBase - sh));
            }
            data[ind] >>= sh;
        }
        data.back() += (cur << (logBase - sh));
    }
    normalize();
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    return ~*this + 1;
}

big_integer big_integer::operator~() const {
    big_integer res;
    if (data.empty()) {
        res.data.push_back(~getNull());
    }
    size_t size = len();
    for (size_t ind = 0; ind < size; ++ind) {
        res.data.push_back(~data[ind]);
    }
    res.normalize();
    return res;
}

big_integer &big_integer::operator++() {
    return *this += 1;
}

big_integer const big_integer::operator++(int) {
    big_integer old = *this;
    ++*this;
    return old;
}

big_integer &big_integer::operator--() {
    return *this -= 1;
}

big_integer const big_integer::operator--(int) {
    big_integer old = *this;
    --*this;
    return old;
}

bool operator==(big_integer const &a, big_integer const &b) {
    return a.data == b.data;
}

bool operator!=(big_integer const &a, big_integer const &b) {
    return !(a == b);
}

bool operator<(big_integer const &a, big_integer const &b) {
    if (a.getSign() ^ b.getSign()) {
        return a.getSign();
    }
    if (a.len() != b.len()) {
        return (a.len() < b.len()) ^ a.getSign();
    }
    for (size_t ind = a.len(); ind > 0; --ind) {
        if (a.data[ind - 1] != b.data[ind - 1]) {
            return a.data[ind - 1] < b.data[ind - 1];
        }
    }
    return false;
}

bool operator>(big_integer const &a, big_integer const &b) {
    return b < a;
}

bool operator<=(big_integer const &a, big_integer const &b) {
    return !(a > b);
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return !(a < b);
}

big_integer operator+(big_integer a, big_integer const &b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const &b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const &b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const &b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const &b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const &b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const &b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const &b) {
    return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}

std::string to_string(big_integer const &a) {
    if (a.data.empty()) {
        return "0";
    }
    std::string str;
    big_integer copy = a;
    bool sign = copy.getSign();
    abs(copy, sign);
    while (copy > 0) {
        auto t = divLongShort(copy, 10);
        copy = t.first;
        str += std::to_string(t.second);
    }
    if (sign) {
        str.push_back('-');
    }
    reverse(str.begin(), str.end());
    return str;
}

std::istream &operator>>(std::istream &s, big_integer &a) {
    std::string num;
    s >> num;
    a = big_integer(num);
    return s;
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    return s << to_string(a);
}
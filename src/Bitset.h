#ifndef BITSET_H
#define BITSET_H

#include <cassert>
#include <cstddef>
#include <ostream>
#include <cstring>

using u64 = unsigned long long;
using u8 = unsigned char;

template<int N,typename T>
T alignup(T t){
    return (t + N -1)/N *N;
}


class __BoolRef{
    u64* _place;
    u8 _pos;
public:
    inline __BoolRef(u64* place,u8 pos){
        assert(pos < 64);
        _place = place;
        _pos = pos;
    }
    inline __BoolRef operator=(bool b){
        if(b){
            *_place |= (1L << _pos);
        }
        else{
            *_place &= ~(1L << _pos);
        }
        return *this;
    }
    inline operator bool() const{
        return (*_place >> _pos) & 1;
    }
};

/**
   @brief This class is a bitset of dynamic size.
 */
class Bitset{
    u64* _data;
    size_t _size;
public:
    /// Create empty bitset, all other operation are UB until @ref init.
    Bitset() : _data(nullptr), _size(0) {}

    /// Create bitset manipulating data with size (in bits) size.
    explicit Bitset(size_t size) : _data(new u64[(size+63)/64]), _size(size){}

    ~Bitset(){
        if(_data) delete[] _data;
    }

    /// Access bit i
    bool get(size_t i) const {
        assert(i < _size);
        return _data[i/64] >> (i%64) & 1;
    }

    /// Set the bit i to 1
    void set(size_t i){
        assert(i < _size);
        _data[i/64] |= (1L << (i%64));
    }

    /// Set range of bit starting at i of length len to 1
    void set(size_t i,size_t len){
        assert(i + len <= _size);
        size_t mod = i % 64;
        if(mod){
            if(len <= 64 - mod){
                _data[i/64] |= (((1ull << (len)) -1ull) << mod);
                return;
            }
            _data[i/64] |= u64(-1ull) - ((1ull << i) - 1ull);
        }
        i = alignup<64>(i);
        len -= (64 -mod);
        while(len >= 64){
            _data[i/64] = u64(-1ull);
            i += 64;
            len -= 64;
        }
        _data[i/64] |= ((1ull << len) -1ull);
    }

    /// unset the bit i
    void unset(size_t i){
        assert(i < _size);
        _data[i/64] &= ~(1L << (i%64));

    }

    /// Unset range of bit starting at i of length len to 1
    void unset(size_t i,size_t len){
        assert(i + len <= _size);
        size_t mod = i % 64;
        if(mod){
            if(len <= 64 - mod){
                _data[i/64] &= ~(((1ull << (len)) -1ull) << mod);
                return;
            }
            _data[i/64] &= ~(u64(-1ull) - ((1ull << i) - 1ull));
        }
        i = alignup<64>(i);
        len -= (64 -mod);
        while(len >= 64){
            _data[i/64] = 0;
            i += 64;
            len -= 64;
        }
        _data[i/64] &= ~((1ull << len) -1ull);
    }
     /// Access to specific boolean.
    __BoolRef operator[](size_t i){
        assert(i < _size);
        return __BoolRef(_data + i/64, i % 64);
    }

    /// Access to specific boolean.
    const __BoolRef operator[](size_t i) const{
        assert(i < _size);
        return __BoolRef(_data + i/64, i % 64);
    }


    /*/// Get the rightmost bit set.
    size_t bsr()const { // -1 if out
        size_t i;
        u64 pos = 0;
        for(i = _size/64; i != (size_t)(-1); --i) {
            if(_data[i]) {
                asm("bsr %1,%0" : "=r"(pos) : "r"(_data[i]));
                return pos + i *64;
            }
        }
        return (-1);
    }

    /// Get the rightmost bit unset
    size_t usr() const{
        size_t i;
        u64 pos = 0;
        for(i = _size/64 -1; i != (size_t)(-1); --i) {
            if(_data[i] != u64(-1)) {
                asm("bsr %1,%0" : "=r"(pos) : "r"(~_data[i]));
                return pos + i *64;
            }
        }
        return (-1);
        }*/

    /// Get the leftmost bit set.
    size_t bsf()const { // -1 if out
        size_t i;
        u64 pos = 0;
        for(i = 0; i < _size/64; ++i) {
            if(_data[i]) {
                asm("bsf %1,%0" : "=r"(pos) : "r"(_data[i]));
                //if(pos + i *64 >= _size) return -1;
                return pos + i *64;
            }
        }
        size_t mod = _size % 64;
        if(mod){
            if(_data[i] & ((1L << mod) -1)){
                asm("bsf %1,%0" : "=r"(pos) : "r"(_data[i]));
                if(pos >= mod) return -1;
                else return pos + i *64;
            }
        }
        return (-1);
    }

    /// Get the leftmost bit unset.
    size_t usf()const { // -1 if out
        size_t i;
        u64 pos = 0;
        for(i = 0; i < _size/64; ++i) {
            if(~_data[i]) {
                asm("bsf %1,%0" : "=r"(pos) : "r"(~_data[i]));
                //if(pos + i *64 >= _size) return -1;
                return pos + i *64;
            }
        }
        size_t mod = _size % 64;
        if(mod){
            if(~_data[i] & ((1L << mod) -1)){
                asm("bsf %1,%0" : "=r"(pos) : "r"(~_data[i]));
                if(pos >= mod) return -1;
                else return pos + i *64;
            }
        }
        return (-1);
    }



    /// Get size of bitset
    size_t size() const {
        return _size;
    }

    /// Clear the bitset to 0.
    void clear(){
        memset(_data,0,(_size+7)/8);
    }

    /// Fill the bitset to 1.
    void fill(){
        memset(_data,-1,(_size+7)/8);
    }
    friend std::ostream& operator<<(std::ostream& out, const Bitset& b);

    bool operator==(const Bitset& oth) const{
        if(this == &oth) return true;
        size_t i;
        for(i = 0 ; i < _size/64 ; ++i){
            if(_data[i] != oth._data[i]) return false;
        }
        size_t mod = _size % 64;
        if(mod){
            if((_data[i] ^ oth._data[i]) & ((1L << mod) - 1)) return false;
        }
        return true;
    }

    bool operator!=(const Bitset& oth) const{
        return !(*this == oth);
    }
};

inline std::ostream& operator<<(std::ostream& out, const Bitset& b){
    for(size_t i = 0 ; i < (b.size() +7)/8 ; ++i){
        for(size_t j =0 ; j < 7  and 8*i+j < b.size() ; ++j){
            out << b[8*i+j];
        }
        out << " ";
    }
    return out;
}

#endif

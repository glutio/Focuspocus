#ifndef __BLIST_H__
#define __BLIST_H__

#include <limits.h>
#include <new>

namespace Buratino {

template<typename T>
class BList {
protected:
  T* _array;
  unsigned _count;
  unsigned _capacity;
  bool _owner;
public:
  // Non-owning view over an existing fixed-size array (e.g. a stack-allocated
  // BRootView*[N]). Never resized or deleted by this BList.
  BList(T* array, unsigned count, unsigned capacity)
    : _array(array), _count(count), _capacity(capacity), _owner(false)  {}

  BList(unsigned capacity)
    : _array(0), _count(0), _capacity(0), _owner(true) { Resize(capacity); }

  BList()
    : _array(0), _count(0), _capacity(0), _owner(true) {}

  // BList owns _array when _owner is true; copying it would shallow-copy the
  // pointer and lead to a double delete[] when both copies are destroyed.
  // Nothing currently needs to copy a BList, so copying is disabled rather
  // than given (unused, untested) deep-copy semantics.
  BList(const BList&) = delete;
  BList& operator = (const BList&) = delete;

  ~BList() {
    if (_owner) {
      delete[] _array;
    }
  }

  T& operator[](unsigned index) {
    return _array[index];
  }

  bool Add(T item) {
    if (_count == _capacity) {
      if (!_owner) {
        return false;
      }
      if (_capacity > UINT_MAX / 2 || !Resize(_capacity ? _capacity * 2 : 2)) {
        return false;
      }
    }

    _array[_count++] = item;
    return true;
  }

  void Remove(unsigned index) {
    for (unsigned i = index; i < _count - 1; ++i) {
      _array[i] = _array[i + 1];
    }
    --_count;
  }

  void Insert(unsigned index, T item) {
    if (index == _count) {
      Add(item);
    } else if (index < _count) {
      if (_count == _capacity) {
        if (!_owner) {
          return;
        }
        if (_capacity > UINT_MAX / 2 || !Resize(_capacity * 2)) {
          return;
        }
      }
      for(unsigned i = _count; i > index; --i) {
        _array[i] = _array[i - 1];
      }
      _array[index] = item;
      ++_count;
    }
  }

  unsigned Length() {
    return _count;
  }

  unsigned Capacity() {
    return _capacity;
  }

  bool Resize(unsigned capacity) {
    if (!_owner) {
      return false;
    }
    if (capacity > static_cast<size_t>(-1) / sizeof(T)) return false;
    T* array = capacity ? new (std::nothrow) T[capacity] : nullptr;
    if (capacity && !array) return false;
    unsigned count = capacity < _count ? capacity : _count;
    for (unsigned i = 0; i < count; ++i) {
      array[i] = _array[i];
    }
    delete[] _array;
    _array = array;
    _capacity = capacity;
    _count = count;
    return true;
  }

  void Compact(T filter) {
    for (auto i = 0; i < _count; ++i) {
      if (_array[i] == filter) {
        for (auto j = i + 1; j < _count; j++) {
          if (_array[j] != filter) {
            _array[i++] = _array[j];
          }
        }
        _count = i;
        break;
      }
    }
  }
};

}
#endif
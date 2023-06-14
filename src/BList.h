#ifndef __BLIST_H__
#define __BLIST_H__

namespace Buratino {

template<typename T>
class BList {
protected:
  T* _array;
  unsigned _count;
  unsigned _capacity;
  bool _owner;
public:
  BList(T* array, unsigned count, unsigned capacity)
    : _array(array), _count(count), _capacity(capacity), _owner(false)  {}

  BList(unsigned capacity)
    : _array(new T[capacity]), _count(0), _capacity(capacity), _owner(true)  {}

  BList()
    : _array(0), _count(0), _capacity(0), _owner(true) {}

  ~BList() {
    if (_owner) {
      delete[] _array;
    }
  }

  T& operator[](unsigned index) {
    return _array[index];
  }

  void Add(T item) {
    if (_count == _capacity) {
      if (!_owner) {
        return;
      }

      if (!_capacity) {
        _capacity = 1;
      }
      Resize(_capacity * 2);
    }

    _array[_count++] = item;
  }

  void Remove(T item) {
    for (unsigned i = 0; i < _count; ++i) {
      if (_array[i] == item) {
        Remove(i);
        break;
      }
    }
    --_count;
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
        Resize(_capacity * 2);
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

  void Resize(unsigned capacity) {
    if (!_owner) {
      return;
    }
    T* array = new T[capacity];
    for (unsigned i = 0; i < min(capacity, _capacity); ++i) {
      array[i] = _array[i];
    }
    delete[] _array;
    _array = array;
    _capacity = capacity;
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
#ifndef __BNULLABLE_H__
#define __BNULLABLE_H__

template <typename T>
class BNullable {
private:
    T _value;
    bool _hasValue;

public:
    BNullable() : _hasValue(false) {}

    BNullable(const T& value) : _value(value), _hasValue(true) {}

    BNullable(const BNullable& other) : _value(other._value), _hasValue(other._hasValue) {}

    BNullable& operator=(const BNullable& other) {
      if (this != &other) {
          _value = other._value;
          _hasValue = other._hasValue;
      }
      return *this;
    }

    BNullable& operator=(const T& newValue) {
      _value = newValue;
      _hasValue = true;
      return *this;
    }

    operator T() const {
      return _value;
    }

    bool hasValue() const {
      return _hasValue;
    }

    void clear() {
      _value = T();
      _hasValue = false;
    }
};

#endif
#pragma once

#include <functional>

template <typename T>
class StateManager
{
private:
  T value;
  std::function<void(const T&)> _callback = nullptr;

public:
  explicit StateManager(T value) = default;

  StateManager& operator=(const T& new_val)
  {
    if (value != new_val) {
      value = new_val;
      if (_callback) {
        _callback(value);
      }
    }
    return *this;
  }

  T get() const{
    return value;
  }

  explicit operator T() const{
    return value;
  }

  void bind(std::function<void(const T&)> callback) 
  {
    _callback=callback;
  }


};

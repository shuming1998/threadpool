#ifndef ANY_H
#define ANY_H
#include <memory>
#include <iostream>

#include "noncopyable.h"

// Any 可接受任意数据类型
class Any {
public:
	Any() = default;
	~Any() = default;
	Any(const Any &) = delete;
	Any &operator=(const Any &) = delete;
	Any(Any&&) = default;
	Any &operator=(Any &&) = default;

  // Any(Any &&any) {
  //   *this = std::move(any);
  // }
  // Any &operator=(Any &&any) {
  //   *this = std::move(any);
  //   return *this;
  // }

  // 该构造函数可以让 Any 类型接收任意其他类型的数据
  template<typename T>
  Any(T data) : base_(std::make_unique<Derive<T> >(data)) {}

  // 将 Any 对象中存储的数据提取出来
  template<typename T>
  T cast() {
    // 必须先将基类指针转成派生类指针，并且在转换时要确
    // 定该基类指针指向的是派生类对象(RTTI==>dynamic_cast)
    Derive<T> *pd = dynamic_cast<Derive<T> *>(base_.get());
    if (!pd) {
      throw "Type in <...> is unmach!";
    }
    return pd->getDate();
  }

private:
  // 基类，可以借助基类指针指向任何派生类类型
  class Base {
  public:
    virtual ~Base() = default;
  };

  // 派生类，借助模板，存储任意类型的数据
  template<typename T>
  class Derive : public Base {
  public:
    Derive(T data) : data_(data) {}
    T getDate() { return data_; }
  private:
    T data_;
  };

private:
  // 基类指针
  std::unique_ptr<Base> base_;
};

#endif
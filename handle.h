#pragma once

#include <memory>

template <auto release> struct Releaser {
    template <class Object> void operator()(Object* object) const { release(object); }
};
template <class Object, auto release> using UniqueHandle = std::unique_ptr<Object, Releaser<release>>;

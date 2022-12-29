#include <concepts>
#include <functional>

template <typename S>
concept funcs = requires (S &s) {
    {s.test()};
};

template <typename... Args>
class rule : public Args... {
public:
    rule(Args... args) : args(args...) {}
    bool test() {
        return (... && funcs<Args>);
    }

};

// CRTP
template<class T>
struct Base {
    void interface() {
        // ...
        static_cast<T *>(this)->implementation();
        // ...
    }

    static void static_func() {
        // ...
        T::static_sub_func();
        // ...
    }
};

struct Derived: Base<Derived> {
    void        implementation();
    static void static_sub_func();
};

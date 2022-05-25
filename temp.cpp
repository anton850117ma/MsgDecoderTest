#include <bit>
#include <concepts>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

/* ====================================================================== */

// TODO     Concept
// *        1. form: requires (parameter list) {requirement sequence}
//          2. parameter list is optional
//          3. 4 types of requirements:
//              a. simple requirment
//              b. type requirements
//              c. compound requirements
//              d. nested requirements

/* ====================================================================== */

// * Simple: check whether expressions that will compile
template<typename T>
concept Incrementable = requires(T t) { ++t; t++;};

template<typename T>
concept Decrementable = requires(T t) { --t; t--;};

/* ====================================================================== */

// * Using the concept

// (1)
template<Incrementable T> 
void Foo(T t);

// (2)
template<typename T> 
requires Incrementable<T>
void Foo(T t);

// (3)
template<typename T>
void Foo(T t) requires Incrementable<T>;

// (4)
void Foo(Incrementable auto t);

/* ====================================================================== */

// * Type: check named types with keyword 'typename'

template<typename T> using Ref = T&;
template<typename T> class S {};
template<typename T> concept Concept1 = requires {
    typename T::inner;    // required nested member name
    typename S<T>;        // required class template specialization
    typename Ref<T>;      // required alias template substitution
};

/* ====================================================================== */

// * Compound: { expression } noexcept return-type-requirement;
//   noexcept and return-type-requirement are optional

// Example: from cppcon
template<typename T>
concept Concept2 = requires(T &x, T &y) {
    // requires a non-throwing swap() method
    { x.swap(y) } noexcept;

    // requires a size() method returning a size_t
    { x.size() } -> std::convertible_to<std::size_t>;
};

/* ====================================================================== */

// Example: from cpluscplus
template<typename T>
concept Concept3 = requires(T x) {

    // the expression *x must be valid
    // AND the type T::inner must be valid
    // AND the result of *x must be convertible to T::inner
    { *x } -> std::convertible_to<typename T::inner>;

    // the expression x + 1 must be valid
    // AND std::same_as<decltype((x + 1)), int> must be satisfied
    // i.e., (x + 1) must be a prvalue of type int
    { x + 1 } -> std::same_as<int>;

    // the expression x * 1 must be valid
    // AND its result must be convertible to T
    { x * 1 } -> std::convertible_to<T>;
};

/* ====================================================================== */

// * Nested: specify additional constraints in terms of local parameters

template<class T>
concept Concept4 = requires(T a, size_t n) {
    requires std::same_as<T *, decltype(&a)>; 
    requires std::same_as<T *, decltype(new T)>;
    requires std::same_as<T *, decltype(new T[n])>;
};

/* ====================================================================== */

// * Combining multiple concepts:

// (1)
template<typename T> 
requires Incrementable<T> && Decrementable<T>
void Foo(T t);

// (2)
template<typename T>
concept Concept5 = Incrementable<T> || Decrementable<T>;
void Foo(Concept5 auto t);

/* ====================================================================== */

// * Before c++20

// (1) implementation deduction
template<typename T>
struct Incrementable_impl {
    template<typename U>
    static auto test(U &&u) -> decltype(++u, u++, std::true_type {});
    static auto test(...) -> std::false_type;
    using type = decltype(test(std::declval<T>()));
};

// (2) inherit type from the instance
template<typename T>
struct Incrementable_cpp17: Incrementable_impl<T>::type {};

// (3) check the member 'value' of std::true_type or std::false_type
template<typename T>
void do_increment_twice(T &&t) {
    // requires c++17 
    if constexpr (Incrementable_cpp17<T>::value) { ++t; t++; }
}

/* ====================================================================== */

// * Since C++20
//   1. easier to write constraints of custom templates
//   2. easier to read template error messages

template<typename T>
concept Incrementable_cpp20 = requires(T x) { ++x; x++; };

void Boo(Incrementable_cpp20 auto t) { t++; }
class Bar {};

// int main() {
//     Bar b;
//     // Boo(b);
// }

/* ====================================================================== */

// TODO         Lambda expression changes
//              1. before c++20, [=] campture 'this' implicitly
// !            2. since c++20, you need to be implicit, so: [=, this]

/* ====================================================================== */

// * Templated Lambda Expressions
//   use familiar template syntax with lambda expressions

auto lambda1 = [] <typename T> (T t) { /*...*/ };
auto lambda2 = [] <typename T, int N> (T (&t)[N]) { /*...*/ };

/* ====================================================================== */

// * Case 1: want to know T of vector<T>

// before c++20
auto lambda3 = [](const auto &vec) {
    using V = std::decay_t<decltype(vec)>;
    using T = typename V::value_type;
    T x {};
    /*...*/
};

// since c++20
auto lambda4 = []<typename T>(const std::vector<T> &vec) {
    T x {};
    /*...*/
};

/* ====================================================================== */

// * Case 2: perfect forwarding

// before c++20
auto lambda5 = [](auto &&...args) {
    return foo(std::forward<decltype(args)>(args)...);
};

// since c++20
auto lambda6 = []<typename... T>(T &&...args) {
    foo(std::forward<T>(args)...);
};

/* ====================================================================== */

// * Pack Expansion in Lambda Captures

// before c++20:
//  a simple-capture followed by an ellipsis is a pack expansion => ok
template<class F, class... Args>
auto delay_invoke(F f, Args... args) {
    return
        [f, args...]() -> decltype(auto) { return std::invoke(f, args...); }
}

//  an init-capture followed by an ellipsis is ill-formed (well-formed since C++20)
template<class F, class... Args>
auto delay_invoke(F f, Args... args) {
    return
        [f = std::move(f), args = std::move(args)...]() -> decltype(auto) {
            return std::invoke(f, args...);
        }
}

//  use the following alternative before c++20
template<class F, class... Args>
auto delay_invoke(F f, Args... args) {
    return [f   = std::move(f),
            tup = std::make_tuple(std::move(args)...)]() -> decltype(auto) {
        return std::apply(f, tup);
    };
}

/* ====================================================================== */

// TODO         constexpr & consteval & constinit

// * constexpr changes
//   1. constexpr virtual functions
//   2. constexpr function can now:
//      (1) use dynamic_cast and typeid
//      (2) do dynamic memory allocations, global new/delete
//      (3) must be transient constexpr allocations
//      (4) contain try/catch blocks
// !          but still cannot throw exceptions
//   3. in constexpr function, std::bit_cast can be used as reinterpret_cast to some extent
//   4. std::string and std::vector are now constexpr

/* ====================================================================== */

// (1)
constexpr int maxElement() {
    std::vector myVec = {1, 2, 4, 3}; 
    std::sort(myVec.begin(), myVec.end());
    return myVec.back();
}

// (2)
constexpr auto correctRelease() {
    auto *p = new int[2020];
    delete[] p;
    return 2020;
}

// (3)
constexpr auto forgottenRelease() {
    auto *p = new int[2020]; 
    try {
        /* ... */
        return 2022;
    } catch (...) {
        delete[] p;
        return 2022;
    }
}

int main() {
    constexpr int res1 = correctRelease();
    constexpr int res2 = forgottenRelease();
}

/* ====================================================================== */

// * Immediate function - consteval
//   constexpr function - might be called at compile time, but not a hard requirement

constexpr auto InchToMm(double inch) { return inch * 25.4; }

const double const_inch {6};
const auto   mm1 {InchToMm(const_inch)};    // at compile time

double     dynamic_inch {8};
const auto mm2 {InchToMm(dynamic_inch)};    // at run time

/* ====================================================================== */

// * Consteval function
//  1. required to always produce a constant at compile time
//  2. implies inline as constexpr 

consteval auto InchToMm2(double inch) { return inch * 25.4; }

const double const_inch {6};
const auto   mm1 {InchToMm2(const_inch)};    // Fine, everything is constant

double     dynamic_inch {8};
const auto mm2 {InchToMm2(dynamic_inch)};    // Compilation error: not constant

/* ====================================================================== */

// * Constinit
//   1. asserts that a variable has static initialization, i.e. zero initialization and 
//      constant initialization, otherwise the program is ill-formed
//   2. declares variable with static or thread storage duration, e.g. static and thread_local 
//   3. cannot be used with constexpr and consteval 
//   4. when the declared variable is a reference, constinit is equivalent to constexpr 
//   5. can be used with object which has constexpr constructors and no constexpr destructor
// ! 6. prevent the static initialization order fiasco

constinit const char *a = {/*...*/};

/* ====================================================================== */

// * Before c++20

// source.cpp
int quad(int n) { return n * n;}
auto staticA = quad(5);

// main.cpp
extern int staticA;
auto       staticB = staticA;
int        main() { std::cout << "staticB: " << staticB << std::endl;}

// staticA and staticB belong to different translation units.
// The initialization of staticB depends on the initialization of staticA.
// The issue is that there is no guarantee in which order staticA or staticB are initialized at run time.
// You have a 50 : 50 chance that staticB is 0 (main -> source) or 25 (source -> main).

/* ====================================================================== */

// * Prevent the static initialization order fiasco before c++20
//   Lazy Initialization of static with local scope

// source.cpp
int quad(int n) { return n * n; }

int &static_value() {
    static auto staticA = quad(5);
    return staticA;
}

// main.cpp

int &static_value();
auto staticB = static_value();

int main() {
    std::cout << "staticB: " << staticB << std::endl;
}

/* ====================================================================== */

// * Since C++20

// source.cpp
constexpr int  const_quad(int n) { return n * n; }
constinit auto init_staticA = const_quad(5);

// main.cpp
extern constinit int init_staticA;
auto                 init_staticB = init_staticA;
int main() { std::cout << "staticB: " << init_staticB << std::endl; }

// Force constant initialization
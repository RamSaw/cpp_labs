#include "linq.h"
#include <gtest/gtest.h>
#include <math.h>
#include <assert.h>
#include <vector>
#include <sstream>
#include <iterator>

void example1() {
    int xs[] = { 1, 2, 3, 4, 5 };

    std::vector<int> res =
            from(xs, xs + 5)  // Взять элементы xs
                    .select([](int x) { return x * x; })  // Возвести в квадрат
                    .where_neq(25)    // Оставить только значения != 25
                    .where([](int x) { return x > 3; })   // Оставить только значения > 3
                    .drop(2)          // Убрать два элемента из начала
                    .to_vector();     // Преобразовать результат в вектор

    std::vector<int> expected = { 16 };
    assert(res == expected);
}

void example2() {
    std::stringstream ss("1 2 3 -1 4");
    std::istream_iterator<int> in(ss), eof;

    std::vector<int> res =
            from(in, eof)  // Взять числа из входного потока
                    .take(4)       // Не более четырёх чисел
                    .until_eq(-1)  // Перестать читать после прочтения -1
                    .to_vector();  // Получить список считанных чисел

    std::vector<int> expected = { 1, 2, 3 };
    assert(expected == res);

    int remaining;
    assert(ss >> remaining);
    assert(remaining == 4);
}

void example3() {
    int xs[] = { 1, 2, 3, 4, 5 };

    std::vector<double> res =
            from(xs, xs + 5)  // Взять элементы xs
                    .select<double>([](int x) { return sqrt(x); })  // Извлечь корень
                    .to_vector();     // Преобразовать результат в вектор

    assert(res.size() == 5);
    for (std::size_t i = 0; i < res.size(); i++) {
        assert(fabs(res[i] - sqrt(xs[i])) < 1e-9);
    }
}

void example4() {
    std::stringstream iss("4 16");
    std::stringstream oss;
    std::istream_iterator<int> in(iss), eof;
    std::ostream_iterator<double> out(oss, "\n");

    from(in, eof)    // Взять числа из входного потока
            .select([](int x) { return static_cast<int>(sqrt(x) + 1e-6); })  // Извлечь из каждого корень
            .copy_to(out);  // Вывести на экран

    assert(oss.str() == "2\n4\n");
}

void from_to_vector() {
    std::vector<int> xs = { 1, 2, 3 };
    std::vector<int> res = from(xs.begin(), xs.end()).to_vector();
    assert(res == xs);
}

void from_select() {
    const int xs[] = { 1, 2, 3 };
    std::vector<int> res = from(xs, xs + 3).select([](int x) { return x + 5; }).to_vector();
    std::vector<int> expected = { 6, 7, 8 };
    assert(res == expected);
}

void from_drop_select() {
    const int xs[] = {1, 2, 3};
    std::vector<int> res = from(xs, xs + 3).drop(1).select([](int x) { return x + 5; }).to_vector();
    std::vector<int> expected = {7, 8};
    assert(res == expected);
}

int main_test() {
    from_to_vector();
    from_select();
    from_drop_select();
    example1();
    example2();
    example3();
    example4();

    return 0;
}

/* TESTS */
TEST(until, until_predicate) {
    std::vector<int> xs = {1, 2, 3};
    std::vector<int> res(2);
    std::vector<int> ans = {1, 2};

    from(xs.begin(), xs.end())
            .until([](int x) { return x % 3 == 0; })
            .copy_to(res.begin());
    ASSERT_EQ(res, ans);
}

TEST(until, until_neq) {
    std::vector<int> xs = {4, 4, 4, 2, 1};
    std::vector<int> ans = {4, 4, 4};

    std::vector<int> res = from(xs.begin(), xs.end())
            .until_neq(4)
            .to_vector();
    ASSERT_EQ(res, ans);
}

TEST(until, empty_result) {
    std::vector<int> xs = {1, 4, 4, 4, 2, 1};
    std::vector<int> ans = {};

    std::vector<int> res = from(xs.begin(), xs.end())
            .until_eq(1)
            .to_vector();
    ASSERT_EQ(res, ans);
}

TEST(until, until_eq) {
    std::vector<int> xs = {1, 2, 3, 4, 4};
    std::vector<int> ans = {1, 2, 3};

    std::vector<int> res = from(xs.begin(), xs.end())
            .until_eq(4)
            .to_vector();
    ASSERT_EQ(res, ans);
}

TEST(take, take) {
    std::vector<int> xs = {1, 2, 3, 4, 4};
    std::vector<int> ans = {1, 2};

    std::vector<int> res = from(xs.begin(), xs.end())
            .take(2)
            .to_vector();
    ASSERT_EQ(res, ans);
    std::vector<int> res2 = from(xs.begin(), xs.end())
            .take(0)
            .to_vector();
    ASSERT_EQ(res2, std::vector<int>());
}

TEST(drop, drop) {
    std::vector<int> xs = {1, 2, 3, 4, 4};
    std::vector<int> ans = {3, 4, 4};

    std::vector<int> res = from(xs.begin(), xs.end())
            .drop(2)
            .to_vector();
    ASSERT_EQ(res, ans);
    std::vector<int> res2 = from(xs.begin(), xs.end())
            .drop(0)
            .to_vector();
    ASSERT_EQ(res2, xs);
}

TEST(where, where_predicate) {
    std::vector<int> xs = {1, 2, 3, 4, 4};
    std::vector<int> ans = {2, 4, 4};

    std::vector<int> res = from(xs.begin(), xs.end())
            .where([](int x) { return x % 2 == 0; })
            .to_vector();
    ASSERT_EQ(res, ans);
}

TEST(where, where_eq) {
    std::vector<int> xs = {4, 2, 3, 4, 4, 5};
    std::vector<int> ans = {4, 4, 4};

    std::vector<int> res = from(xs.begin(), xs.end())
            .where_eq(4)
            .to_vector();
    ASSERT_EQ(res, ans);
}

TEST(where, where_neq) {
    std::vector<int> xs = {4, 2, 3, 4, 4, 5};
    std::vector<int> ans = {2, 3, 5};

    std::vector<int> res = from(xs.begin(), xs.end())
            .where_neq(4)
            .to_vector();
    ASSERT_EQ(res, ans);
}

TEST(select, factorial) {
    class Factorial{
    public:
        int operator()(int x){
            if (x == 1)
                return 1;
            return x * Factorial()(x - 1);
        }
    };
    std::vector<int> xs = {1, 2, 3, 4, 5};
    std::vector<int> ans = {1, 2, 6, 24, 120};

    std::vector<int> res = from(xs.begin(), xs.end())
            .select<int>(Factorial())
            .to_vector();
    ASSERT_EQ(res, ans);
}

TEST(select, convert_to_bool) {
    std::vector<int> xs = {-1, -2, 0, 4, 5};
    std::vector<bool> ans = {1, 1, 0, 1, 1};

    std::vector<bool> res = from(xs.begin(), xs.end())
            .select<bool>([](int x) { return x;})
            .to_vector();
    ASSERT_EQ(res, ans);
}
template <class T>
class My{
public:
    My() { x = 0; }
    T x;
    bool operator==(My<T> other) const { return x == other.x; }
};

TEST(select, my_class) {
    My<int> my;
    std::vector<My<int>> xs = {my, my, my};
    std::vector<My<int>> ans = {my, my, my};

    std::vector<My<int>> res = from(xs.begin(), xs.end())
            .select<My<int>>([](My<int> i) { return i; })
            .to_vector();
    ASSERT_EQ(res, ans);
}

TEST(main__Test, main__Test_main_Test) {
    main_test();
}
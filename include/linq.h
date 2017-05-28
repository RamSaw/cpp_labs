#ifndef LINQ_H_
#define LINQ_H_

#include <vector>

template<typename T, typename Iter>
class range_enumerator;
template<typename T>
class drop_enumerator;
template<typename T>
class take_enumerator;
template<typename T, typename U, typename F>
class select_enumerator;
template<typename T, typename F>
class until_enumerator;
template<typename T, typename F>
class where_enumerator;

template<typename T>
class enumerator {
public:
    virtual const T& operator*() = 0; // Получает текущий элемент.
    virtual enumerator<T>& operator++() = 0;  // Переход к следующему элементу
    virtual operator bool() = 0;  // Возвращает true, если есть текущий элемент

    auto drop(int count) {
        return drop_enumerator<T>(*this, count);
    }

    auto take(int count) {
        return take_enumerator<T>(*this, count);
    }

    template<typename U = T, typename F>
    auto select(F func) {
        return select_enumerator<U, T, F>(*this, func);
    }

    template<typename F>
    auto until(F func) {
        return until_enumerator<T, F>(*this, func);
    }

    auto until_eq(T value) {
        return until([value](T x) { return x == value; });
    }

    auto until_neq(T value) {
        return until([value](T x) { return x != value; });
    }

    template<typename F>
    auto where(F func) {
        return where_enumerator<T, F>(*this, func);
    }

    auto where_eq(T value) {
        return where([value](T x) { return x == value; });
    }

    auto where_neq(T value) {
        return where([value](T x) { return x != value; });
    }

    std::vector<T> to_vector() {
        std::vector<T> ans;
        while ((bool)*this) {
            ans.push_back(std::move(*(*this)));
            ++(*this);
        }

        return std::move(ans);
    }

    template<typename Iter>
    void copy_to(Iter it) {
        while ((bool)*this) {
            *it = std::move(*(*this));
            it++;
            ++(*this);
        }
    }
};

template<typename T, typename Iter>
class range_enumerator : public enumerator<T> {
public:
    operator bool() override {
        return begin_ != end_;
    }

    enumerator<T>& operator++() override {
        begin_++;
        return *this;
    }

    const T& operator*() override {
        return *begin_; /// Safe or not safe, raise exception?
    }

    range_enumerator(Iter begin, Iter end) : begin_(begin), end_(end) {
    }

private:
    Iter begin_, end_;
};

template<typename T>
auto from(T begin, T end) {
    return range_enumerator<typename std::iterator_traits<T>::value_type, T>(begin, end);
}

template<typename T>
class drop_enumerator : public enumerator<T> {
public:
    drop_enumerator(enumerator<T> &parent, int count) : parent_(parent), count_(count) {
        while (count_ > 0 && bool(*this)) {
            ++parent_;
            count_--;
        }
    }

    operator bool() override {
        return bool(parent_);
    }

    enumerator<T>& operator++() override {
        ++parent_;
        return *this;
    }

    const T& operator*() override {
        return *parent_;
    }

private:
    enumerator<T> &parent_;
    int count_;
};

template<typename T>
class take_enumerator : public enumerator<T> {
public:
    take_enumerator(enumerator<T> &parent, int count) : parent_(parent), count_(count), is_end_(false) {
        if (!count_)
            is_end_ = true;
    }

    operator bool() override {
        return !is_end_ && bool(parent_);
    }

    enumerator<T>& operator++() override {
        ++parent_;
        count_--;
        if (count_ == 0)
            is_end_ = true;
        return *this;
    }

    const T& operator*() override {
        return *parent_;
    }

private:
    enumerator<T> &parent_;
    int count_;
    bool is_end_;
};

template<typename T, typename U, typename F>
class select_enumerator : public enumerator<T> {
public:
    select_enumerator(enumerator<U> &parent, F func) : parent_(parent), func_(func), is_calculated_(false) {
    }

    operator bool() override {
        return bool(parent_);
    }

    enumerator<T>& operator++() override {
        ++parent_;
        is_calculated_ = false;
        return *this;
    }

    const T& operator*() override {
        if (!is_calculated_) {
            calculated_value_ = std::move(func_(*parent_));
            is_calculated_ = true;
        }
        return calculated_value_;
    }

private:
    enumerator<U> &parent_;
    F func_;
    bool is_calculated_;
    T calculated_value_;
};

template<typename T, typename F>
class until_enumerator : public enumerator<T> {
public:
    until_enumerator(enumerator<T> &parent, F predicate) : parent_(parent), predicate_(predicate), is_end_(false) {
        if (!parent_ || predicate_(*parent))
            is_end_ = true;
    }

    operator bool() override {
        return !is_end_ && bool(parent_);
    }

    enumerator<T>& operator++() override {
        ++parent_;
        if (parent_ && predicate_(*parent_)) /// No need for parent
            is_end_ = true;
        return *this;
    }

    const T& operator*() override {
        return *parent_;
    }

private:
    enumerator<T> &parent_;
    F predicate_;
    bool is_end_;
};

template<typename T, typename F>
class where_enumerator : public enumerator<T> {
public:
    where_enumerator(enumerator<T> &parent, F predicate) : parent_(parent), predicate_(predicate) {
        while (parent_ && !predicate_(*parent_))
            ++parent_;
    }

    operator bool() override {
        return bool(parent_);
    }

    enumerator<T>& operator++() override {
        while (++parent_ && !predicate_(*parent_))
            ;
        return *this;
    }

    const T& operator*() override {
        return *parent_;
    }

private:
    enumerator<T> &parent_;
    F predicate_;
};

#endif
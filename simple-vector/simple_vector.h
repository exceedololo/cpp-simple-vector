// вставьте сюда ваш код для класса SimpleVector
// внесите необходимые изменения для поддержки move-семантики
#include "array_ptr.h"

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <iterator>
#include <iostream>
#include <utility>

using namespace std;

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity_to_reserve = 0) : capacity_(capacity_to_reserve) {
    }
    size_t GetСapacity() {
        return capacity_;
    }
    size_t capacity_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
    friend class ReserveProxyObj;
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;
    
     // конструктор копирования
    SimpleVector(SimpleVector&& other) noexcept {
        size_ = other.GetSize();
        capacity_ = other.GetCapacity();
        SimpleVector tmp(other.size_);
        std::move(other.begin(), other.end(), tmp.begin());

        auto size_tmp = other.size_;
        auto capacity_tmp = other.capacity_;
        data_.swap(tmp.data_);
        size_ = size_tmp;
        capacity_ = capacity_tmp;
        other.Clear();
    }

    // конструктор копирования const
    SimpleVector(const SimpleVector& other) noexcept {
        size_ = other.GetSize();
        capacity_ = other.GetCapacity();
        SimpleVector tmp(other.size_);
        std::copy(other.begin(), other.end(), tmp.begin());

        auto size_tmp = other.size_;
        auto capacity_tmp = other.capacity_;

        data_.swap(tmp.data_);
        size_ = size_tmp;
        capacity_ = capacity_tmp;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (rhs.IsEmpty()) {
            Clear();
        }
        else if (this != &rhs) {
            SimpleVector temp(std::move(rhs));
            swap(temp);
        }
        return *this;
    }

    SimpleVector& operator=(const SimpleVector& rhs) noexcept {
        if (rhs.IsEmpty()) {
            Clear();
        }
        else if (this != &rhs) {
            SimpleVector temp(std::move(rhs));
            swap(temp);
        }
        return *this;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            auto size = size_;
            Resize(new_capacity);
            capacity_ = new_capacity;
            size_ = size;
        }
    }

  // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : data_(init.size()), size_(init.size()), capacity_(init.size()) {
        std::copy(init.begin(), init.end(), begin());
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, Type&& value=0) : data_(size), size_(size), capacity_(size) {
        int s = 0;
        for (auto i = std::make_move_iterator(begin()); i < std::make_move_iterator(end()); i++) {
            data_[s] = std::move(value);
            s++;
        }
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type value) : data_(size), size_(size), capacity_(size) {
        std::fill(begin(), end(), value);
    }

    explicit SimpleVector(ReserveProxyObj capacity) {
        Reserve(capacity.capacity_);
    }

    void PushBack(Type&& item) {
        if (size_ == 0) {
            Resize(1);
            size_ = 0;
        }
        else if (size_ == capacity_) {
            auto oldsize = size_;
            Resize(size_ * 2);
            size_ = oldsize;
        }
        data_[size_++] = std::move(item);
    }

    void PushBack(const Type&& item) {
        if (size_ == 0) {
            Resize(1);
            size_ = 0;
        }
        else if (size_ == capacity_) {
            auto oldsize = size_;
            Resize(size_ * 2);
            size_ = oldsize;
        }
        data_[size_++] = std::move(item);
    }

    // Вставляет значение value в позицию pos.
        // Возвращает итератор на вставленное значение
        // Если перед вставкой значения вектор был заполнен полностью,
        // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert((pos >= cbegin()) && (pos <= cend()));
        if (size_ == capacity_) {
            auto size = size_ + 1;
            SimpleVector tmp(size);
            size_t size_t_pos = pos - begin();
            std::move_backward(std::make_move_iterator(begin() + size_t_pos), std::make_move_iterator(end()), std::make_move_iterator(tmp.end()));
            tmp.begin()[size_t_pos] = std::move(value);
            std::move(std::make_move_iterator(begin()), std::make_move_iterator(begin() + size_t_pos), std::make_move_iterator(tmp.begin()));
            swap(tmp);
            size_ = size;
            capacity_ = size * 2;
            return begin() + size_t_pos;
        }
        else {
            size_++;
            size_t size_t_pos = pos - begin();
            std::move_backward(std::make_move_iterator(begin() + size_t_pos), std::make_move_iterator(end() - 1), std::make_move_iterator(end()));
            *(begin() + size_t_pos) = std::move(value);
            std::move(begin(), begin() + size_t_pos, begin());
            return std::make_move_iterator(begin() + size_t_pos);
        }
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert((pos >= cbegin()) && (pos <= cend()));
        if (size_ == capacity_) {
            auto size = size_ + 1;
            SimpleVector tmp(size);
            size_t size_t_pos = pos - begin();
            std::move_backward(begin() + size_t_pos, end(), tmp.end());
            tmp.begin()[size_t_pos] = std::move(value);
            std::move(begin(), begin() + size_t_pos, tmp.begin());
            swap(tmp);
            size_ = size;
            capacity_ = size * 2;
            return begin() + size_t_pos;
        }
        else {
            size_++;
            size_t size_t_pos = pos - begin();
            std::move_backward(begin() + size_t_pos, end() - 1, end());
            *(begin() + size_t_pos) = std::move(value);
            std::move(begin(), begin() + size_t_pos, begin());
            return begin() + size_t_pos;
        }
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(!IsEmpty());
        size_--;
    }
  
    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert((pos >= cbegin()) && (pos <cend()));
        auto size_t_pos = pos - begin();
        std::move(begin() + size_t_pos + 1, end(), begin() + size_t_pos);
        size_--;
        return std::move(begin() + size_t_pos);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        SimpleVector tmp(std::move(other.GetSize()));

        tmp.data_.swap(other.data_);
        other.data_.swap(data_);
        data_.swap(tmp.data_);

        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return (size_ == 0);
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        rassert(index < size_);
        eturn data_[index];
    }

    // Возвращает ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("invalid index");
        }
        return data_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("invalid index");
        }
        return data_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) noexcept {
    if (new_size > size_) {
        if (new_size > capacity_) {
            while (new_size > capacity_) {
                capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
            }
            ArrayPtr<Type> temp(capacity_);
            if (begin() != nullptr) {
                std::move(begin(), end(), temp.Get());
            }
            data_.swap(temp);
        }
        Iterator first = begin() + size_;
        Iterator last = begin() + new_size;
        for (auto it = first; it != last; ++it) {
            *it = Type();
        }
    }
    size_ = new_size;
}
        

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return &data_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return begin() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return &data_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return begin() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return &data_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return cbegin() + size_;
    }
private:
    ArrayPtr<Type> data_;
    size_t size_ = 0;
    size_t capacity_ = 0;

};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin()));
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin()));;
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend()));
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(std::lexicographical_compare(rhs.cbegin(), rhs.cend(), lhs.cbegin(), lhs.cend()));
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (std::lexicographical_compare(rhs.cbegin(), rhs.cend(), lhs.cbegin(), lhs.cend()));
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend()));
}

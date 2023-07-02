#pragma once
 
#include <iostream>
#include <cassert>
#include <initializer_list>
#include <utility>
 
#include "array_ptr.h"
 
class ReserveProxyObj {
public:
    ReserveProxyObj(size_t new_capacity) : new_capacity_(new_capacity) {}
 
    size_t GetCapacity() const {
        return new_capacity_;
    }
 
private:
    size_t new_capacity_ = 0;
};
 
ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}
 
 
template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;
 
    SimpleVector() noexcept = default;

 
    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : vec_(size) {
        SimpleVectorInit(size, value);
    }
    
     
    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : vec_(size) {
        SimpleVectorInit(size);
    }
 
    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : vec_(init.size()), size_(init.size()), capacity_(init.size()) {
        if (capacity_ > 0) {
            std::copy(init.begin(), init.end(), vec_.Get());
        }
    }
 
    SimpleVector(const SimpleVector& other) {
        try {
            size_ = other.GetSize();
            capacity_ = other.GetCapacity();
            ArrayPtr<Type> temp(capacity_);
            std::copy(other.begin(), other.end(), temp.Get());
            for (auto it = temp.Get() + size_; it != temp.Get() + capacity_; ++it) {
                *it = std::move(Type());
            }
 
            vec_.swap(temp);
        }
        catch (const std::bad_alloc&) {
            capacity_ = size_ = 0;
            throw;
        }
    }
 
    SimpleVector(SimpleVector&& vec) noexcept : size_(std::exchange(vec.size_, 0)), capacity_(std::exchange(vec.capacity_, 0)) {
        vec_.swap(vec.vec_);
    }
 
    SimpleVector(const ReserveProxyObj& obj) : capacity_(obj.GetCapacity()) {
        if (capacity_ > 0) {
            std::fill(begin(), end(), Type());
        }
    }
 
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            auto rhs_copy(rhs);
            swap(rhs_copy);
        }
 
        return *this;
    }
 
    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            vec_.swap(rhs.vec_);
            size_ = std::exchange(rhs.size_, 0);
            capacity_ = std::exchange(rhs.capacity_, 0);
        }
 
        return *this;
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
        return (GetSize() == 0);
    }
 
    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return vec_[index];
    }
 
    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return vec_[index];
    }
 
    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        return (index >= size_) ? throw std::out_of_range("out of range") : vec_[index];
    }
 
    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        return (index >= size_) ? throw std::out_of_range("out of range") : vec_[index];
    }
 
    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }
 
    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t size) {
        if (size <= size_) {
            size_ = size;
        }
        else if (size <= capacity_) {
            for (auto it = begin() + size_; it != begin() + size; ++it) {
                *it = std::move(Type());
            }
 
            size_ = size;
        }
        else {
            ArrayPtr<Type> temp(size);
 
            for (auto it = temp.Get(); it != temp.Get() + size_; ++it) {
                *it = std::move(*(begin() + (it - temp.Get())));
            }
 
            for (auto it = temp.Get() + size_; it != temp.Get() + size; ++it) {
                *it = std::move(Type());
            }
 
            vec_.swap(temp);
 
            capacity_ = size_ = size;
        }
    }
 
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return Iterator(vec_.Get());
    }
 
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return Iterator(vec_.Get() + size_);
    }
 
    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return ConstIterator(vec_.Get());
    }
 
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return ConstIterator(vec_.Get() + size_);
    }
 
    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return ConstIterator(vec_.Get());
    }
 
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return ConstIterator(vec_.Get() + size_);
    }
 
    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            vec_[size_++] = item;
        }
        else {
            ArrayPtr<Type> temp((capacity_ != 0) ? 2 * capacity_ : 1);
            std::copy(begin(), end(), temp.Get());
            std::fill(temp.Get() + size_, temp.Get() + capacity_, Type());
            temp[size_++] = item;
            vec_.swap(temp);
            capacity_ = (capacity_ != 0) ? 2 * capacity_ : 1;
        }
    }
 
    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            vec_[size_++] = std::move(item);
        }
        else {
            ArrayPtr<Type> temp((capacity_ != 0) ? 2 * capacity_ : 1);
 
            for (size_t i = 0; i < size_; ++i) {
                temp[i] = std::move(vec_[i]);
            }
 
            for (size_t i = size_; i < capacity_; ++i) {
                temp[i] = std::move(Type());
            }
 
            temp[size_++] = std::move(item);
 
            vec_.swap(temp);
 
            capacity_ = (capacity_ != 0) ? 2 * capacity_ : 1;
        }
    }
 
    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        if (size_ < capacity_) {
            Iterator it = Iterator(pos);
            std::copy_backward(Iterator(pos), end(), end() + 1);
            *it = value;
            ++size_;
            return Iterator(pos);
        }
        else {
            auto number = pos - vec_.Get();
            ArrayPtr<Type> temp((capacity_ != 0) ? 2 * capacity_ : 1);
            std::copy(begin(), end(), temp.Get());
            std::fill(temp.Get() + size_, temp.Get() + capacity_, Type());
            std::copy_backward(temp.Get() + number, temp.Get() + capacity_, temp.Get() + capacity_ + 1);
            temp[number] = value;
            vec_.swap(temp);
            ++size_;
            capacity_ = (capacity_ != 0) ? 2 * capacity_ : 1;
            return Iterator(vec_.Get() + number);
        }
    }
 
    Iterator Insert(ConstIterator pos, Type&& value) {
        if (size_ < capacity_) {
            auto it = Iterator(pos);
            for (int i = static_cast<int>(end() - begin() + 1); i > Iterator(pos) - begin(); --i) {
                vec_[i] = std::move(vec_[i - 1]);
            }
            vec_[it - begin()] = std::move(value);
            ++size_;
            return Iterator(pos);
        }
        else {
            auto number = pos - vec_.Get();
            ArrayPtr<Type> temp((capacity_ != 0) ? 2 * capacity_ : 1);
 
            for (size_t i = 0; i < size_; ++i) {
                temp[i] = std::move(vec_[i]);
            }
 
            for (size_t i = size_; i < capacity_; ++i) {
                temp[i] = std::move(Type());
            }
 
            for (int i = static_cast<int>(capacity_ + 1); i > number; --i) {
                temp[i] = std::move(temp[i - 1]);
            }
 
            temp[number] = std::move(value);
 
            vec_.swap(temp);
            ++size_;
            capacity_ = (capacity_ != 0) ? 2 * capacity_ : 1;
 
            return Iterator(vec_.Get() + number);
        }
    }
 
    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        --size_;
    }
 
    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        auto aim = std::distance(cbegin(), pos);
        if (pos == begin() + size_) {    
            size_--;            
        }
 
        else {
            ArrayPtr<Type> temp(size_);
            temp[size_ - 1] = std::move(Type{});
            for (auto i = 0; i < aim; ++i) {
                temp[i] = std::move(vec_[i]);
            }
            for (auto i = static_cast<int>(size_ - 1); i > aim; --i) {
                temp[i - 1] = std::move(vec_[i]);
            }
            vec_.swap(temp);
            size_--;
        }
        return Iterator(cbegin() + aim);
    }
 
    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        vec_.swap(other.vec_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
 
    void Reserve(size_t capacity) {
        if (capacity_ < capacity) {
            ArrayPtr<Type> temp(capacity);
 
            std::copy(begin(), end(), temp.Get());
            std::fill(temp.Get() + size_, temp.Get() + capacity, Type());
 
            vec_.swap(temp);
 
            capacity_ = capacity;
        }
    }
 
private:
    ArrayPtr<Type> vec_;
    size_t size_ = 0;
    size_t capacity_ = 0;
    
    void SimpleVectorInit(size_t size, const Type& value = Type()){
        capacity_ = size_ = size;
        if (size > 0) {
            std::fill(begin(), end(), value);
        }
    }
};
 
template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize()) ?
        std::equal(lhs.begin(), lhs.end(), rhs.begin()) : false;
}
 
template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
        return !(lhs == rhs);
}
 
template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}
 
template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs > rhs);
}

 
template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
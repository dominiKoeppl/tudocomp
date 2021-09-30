#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <climits>

#include <tudocomp/ds/IntRepr.hpp>
#include <tudocomp/ds/IntPtr.hpp>
#include <tudocomp/ds/BitPackingVector.hpp>
#include <tudocomp/ds/uint_t.hpp>
#include <tudocomp/ds/dynamic_t.hpp>
#include <tudocomp/util/IntegerBase.hpp>

#include <glog/logging.h>

namespace tdc {
namespace int_vector {
    /// \cond INTERNAL

    template<typename T>
    struct IntVectorStdVectorRepr {
        typedef typename std::vector<T>::value_type             value_type;

        typedef typename std::vector<T>::reference              reference;
        typedef typename std::vector<T>::const_reference        const_reference;

        typedef typename std::vector<T>::pointer                pointer;
        typedef typename std::vector<T>::const_pointer          const_pointer;

        typedef typename std::vector<T>::iterator               iterator;
        typedef typename std::vector<T>::const_iterator         const_iterator;

        typedef typename std::vector<T>::reverse_iterator       reverse_iterator;
        typedef typename std::vector<T>::const_reverse_iterator const_reverse_iterator;

        typedef typename std::vector<T>::difference_type        difference_type;
        typedef typename std::vector<T>::size_type              size_type;

        typedef          std::vector<T>                         backing_data;
        typedef          T                                      internal_data_type;

        inline static uint64_t bit_size(const backing_data& self) {
            return sizeof(T) * CHAR_BIT * self.size();
        }

        inline static uint64_t bit_capacity(const backing_data& self) {
            return sizeof(T) * CHAR_BIT * self.capacity();
        }

        inline static constexpr ElementStorageMode element_storage_mode() {
            return ElementStorageMode::Direct;
        }

        inline static uint8_t width(const backing_data& self) {
            return sizeof(T) * CHAR_BIT;
        }

        inline static backing_data with_width(size_type n, const value_type& val, uint8_t width) {
            return backing_data(n, val, width);
        }

        inline static void width(backing_data& self, uint8_t w) {
        }

        inline static void resize(backing_data& self, size_type n, const value_type& val, uint8_t w) {
            self.resize(n, val);
        }

        inline static void bit_reserve(backing_data& self, uint64_t n) {
            // TODO: Should this round up to the size of element, and then reserve normally?
        }

        inline static size_t stat_allocation_size_in_bytes(backing_data const& self) {
            return self.capacity() * sizeof(T);
        }
    };

    template<typename T>
    struct IntVectorBitPackingVectorRepr {
        typedef typename BitPackingVector<T>::value_type             value_type;

        typedef typename BitPackingVector<T>::reference              reference;
        typedef typename BitPackingVector<T>::const_reference        const_reference;

        typedef typename BitPackingVector<T>::pointer                pointer;
        typedef typename BitPackingVector<T>::const_pointer          const_pointer;

        typedef typename BitPackingVector<T>::iterator               iterator;
        typedef typename BitPackingVector<T>::const_iterator         const_iterator;

        typedef typename BitPackingVector<T>::reverse_iterator       reverse_iterator;
        typedef typename BitPackingVector<T>::const_reverse_iterator const_reverse_iterator;

        typedef typename BitPackingVector<T>::difference_type        difference_type;
        typedef typename BitPackingVector<T>::size_type              size_type;

        typedef          BitPackingVector<T>                         backing_data;
        typedef typename BitPackingVector<T>::internal_data_type     internal_data_type;

        inline static uint64_t bit_size(const backing_data& self) {
            return self.size() * self.width();
        }

        inline static uint64_t bit_capacity(const backing_data& self) {
            return self.capacity() * self.width();
        }

        inline static constexpr ElementStorageMode element_storage_mode() {
            return ElementStorageMode::BitPacked;
        }

        inline static uint8_t width(const backing_data& self) {
            return self.width();
        }

        inline static backing_data with_width(size_type n, const value_type& val, uint8_t width) {
            return backing_data(n, val, width);
        }

        inline static void width(backing_data& self, uint8_t w) {
            self.width(w);
        }

        inline static void resize(backing_data& self, size_type n, const value_type& val, uint8_t w) {
            self.resize(n, val, w);
        }

        inline static void bit_reserve(backing_data& self, uint64_t n) {
            self.bit_reserve(n);
        }

        inline static size_t stat_allocation_size_in_bytes(backing_data const& self) {
            return self.stat_allocation_size_in_bytes();
        }
    };

    template<class T, class X = void>
    struct IntVectorTrait:
        public IntVectorStdVectorRepr<T> {};

    template<size_t N>
    struct IntVectorTrait<uint_impl_t<N>, typename std::enable_if_t<(N % 8 != 0) && (N > 1)>>:
        public IntVectorBitPackingVectorRepr<uint_t<N>> {};

    template<>
    struct IntVectorTrait<dynamic_t>:
        public IntVectorBitPackingVectorRepr<dynamic_t> {};

    template<>
    struct IntVectorTrait<bool>:
        public IntVectorBitPackingVectorRepr<uint_t<1>> {};

    ///\endcond

    /// A vector over arbitrary unsigned integer types.
    ///
    /// The API behaves mostly identical to `std::vector<T>`.
    ///
    /// Only divergences are the following specializations:
    ///
    /// - `IntVector<uint_t<N>>` where `N % 8 != 0`
    /// - `IntVector<dynamic_t>`
    ///
    /// In both cases, the bits of each integer will be packed efficiently next
    /// to each other, as opposed to the padding introduced if stored
    /// in a `std::vector`.
    ///
    /// In the `dynamic_t` case, the bit with of an integer can be changed at
    /// runtime, in all other cases the corresponding methods will throw.
    // TODO ^ Could just not offer the methods for the non-dynamic case
    template<class T>
    class IntVector {
        // TODO: Add custom allocator support
    public:
        typedef typename IntVectorTrait<T>::value_type             value_type;
        typedef typename IntVectorTrait<T>::reference              reference;
        typedef typename IntVectorTrait<T>::const_reference        const_reference;
        typedef typename IntVectorTrait<T>::pointer                pointer;
        typedef typename IntVectorTrait<T>::const_pointer          const_pointer;
        typedef typename IntVectorTrait<T>::iterator               iterator;
        typedef typename IntVectorTrait<T>::const_iterator         const_iterator;
        typedef typename IntVectorTrait<T>::reverse_iterator       reverse_iterator;
        typedef typename IntVectorTrait<T>::const_reverse_iterator const_reverse_iterator;
        typedef typename IntVectorTrait<T>::difference_type        difference_type;
        typedef typename IntVectorTrait<T>::size_type              size_type;

        /// The element type of the internal data buffer accessed with data()
        typedef typename IntVectorTrait<T>::internal_data_type     internal_data_type;

        static constexpr ElementStorageMode element_storage_mode() {
            return IntVectorTrait<T>::element_storage_mode();
        }
    private:
        typename IntVectorTrait<T>::backing_data m_data;
    public:
        // default
        inline explicit IntVector() {}

        // fill
        explicit IntVector(size_type n): m_data(n) {}
        inline IntVector(size_type n, const value_type& val): m_data(n, val) {}
        inline IntVector(size_type n, const value_type& val, uint8_t width):
            m_data(IntVectorTrait<T>::with_width(n, val, width)) {}

        // range
        template <class InputIterator>
        inline IntVector(InputIterator first, InputIterator last): m_data(first, last) {}

        // copy
        inline IntVector(const IntVector& other): m_data(other.m_data) {}

        // move
        inline IntVector(IntVector&& other): m_data(std::move(other.m_data)) {}

        // initializer list
        inline IntVector(std::initializer_list<value_type> il): m_data(il) {}

        inline IntVector& operator=(const IntVector& other) {
            m_data = other.m_data;
            return *this;
        }

        inline IntVector& operator=(IntVector&& other) {
            m_data = std::move(other.m_data);
            return *this;
        }

        inline IntVector& operator=(std::initializer_list<value_type> il) {
            m_data = il;
            return *this;
        }

        inline iterator begin() {
            return m_data.begin();
        }

        inline iterator end() {
            return m_data.end();
        }

        inline reverse_iterator rbegin() {
            return m_data.rbegin();
        }

        inline reverse_iterator rend() {
            return m_data.rend();
        }

        inline const_iterator begin() const {
            return m_data.begin();
        }

        inline const_iterator end() const {
            return m_data.end();
        }

        inline const_reverse_iterator rbegin() const {
            return m_data.rbegin();
        }

        inline const_reverse_iterator rend() const {
            return m_data.rend();
        }

        inline const_iterator cbegin() const {
            return m_data.cbegin();
        }

        inline const_iterator cend() const {
            return m_data.cend();
        }

        inline const_reverse_iterator crbegin() const {
            return m_data.crbegin();
        }

        inline const_reverse_iterator crend() const {
            return m_data.crend();
        }

        inline size_type size() const {
            return m_data.size();
        }

        inline uint64_t bit_size() const {
            return IntVectorTrait<T>::bit_size(m_data);
        }

        inline size_type max_size() const {
            return m_data.max_size();
        }

        inline uint8_t width() const {
            return IntVectorTrait<T>::width(m_data);
        }

        inline void width(uint8_t w) {
            IntVectorTrait<T>::width(m_data, w);
        }

    private:
        static const bool do_resize_check = false;

        /// Check that the capacity does not grow during the operation F
        ///
        /// If this fails you need to add an reserve() call before the resize()
        /// call
        template<typename F>
        void check_for_growth(F f) {
            auto old_cap = this->capacity();
            f();
            auto new_cap = this->capacity();

            if (do_resize_check) {
                DCHECK_EQ(old_cap, new_cap)
                    << "\nresize() call grew the capacity!"
                    << "\nThis does not cause a single reallocation, but dynamical growth"
                    " with overallocation!"
                    << "\nConsider calling reserve() beforehand.";
            }
        }
    public:

        inline void resize(size_type n) {
            check_for_growth([&]() {
                m_data.resize(n);
            });
        }

        inline void resize(size_type n, const value_type& val) {
            check_for_growth([&]() {
                m_data.resize(n, val);
            });
        }

        inline void resize(size_type n, const value_type& val, uint8_t w) {
            check_for_growth([&]() {
                IntVectorTrait<T>::resize(m_data, n, val, w);
            });
        }

        inline size_type capacity() const {
            return m_data.capacity();
        }

        inline uint64_t bit_capacity() const {
            return IntVectorTrait<T>::bit_capacity(m_data);
        }

        inline bool empty() const {
            return m_data.empty();
        }

        inline void reserve(size_type n) {
            m_data.reserve(n);
        }

        inline void reserve(size_type n, uint8_t w) {
            IntVectorTrait<T>::bit_reserve(m_data, n * w);
        }

        inline void bit_reserve(uint64_t n) {
            IntVectorTrait<T>::bit_reserve(m_data, n);
        }

        inline void shrink_to_fit() {
            m_data.shrink_to_fit();
        }

        inline reference operator[](size_type n) {
            return m_data[n];
        }

        inline const_reference operator[](size_type n) const {
            return m_data[n];
        }

        inline reference at(size_type n) {
            return m_data.at(n);
        }

        inline const_reference at(size_type n) const {
            return m_data.at(n);
        }

        inline reference front() {
            return m_data.front();
        }

        inline const_reference front() const {
            return m_data.front();
        }

        inline reference back() {
            return m_data.back();
        }

        inline const_reference back() const {
            return m_data.back();
        }

        inline internal_data_type* data() noexcept {
            return m_data.data();
        }

        inline const internal_data_type* data() const noexcept {
            return m_data.data();
        }

        template <class InputIterator>
        inline void assign(InputIterator first, InputIterator last) {
            m_data.assign(first, last);
        }

        inline void assign(size_type n, const value_type& val) {
            m_data.assign(n, val);
        }

        inline void assign(std::initializer_list<value_type> il) {
            m_data.assign(il);
        }

        inline void push_back(const value_type& val) {
            m_data.push_back(val);
        }

        inline void push_back(value_type&& val) {
            m_data.push_back(std::move(val));
        }

        inline void pop_back() {
            m_data.pop_back();
        }

        inline iterator insert(const_iterator position, const value_type& val) {
            return m_data.insert(position, val);
        }

        inline iterator insert(const_iterator position, size_type n, const value_type& val) {
            return m_data.insert(position, n, val);
        }

        template <class InputIterator>
        inline iterator insert(const_iterator position, InputIterator first, InputIterator last) {
            return m_data.insert(position, first, last);
        }

        inline iterator insert(const_iterator position, value_type&& val) {
            return m_data.insert(position, std::move(val));
        }

        inline iterator insert(const_iterator position, std::initializer_list<value_type> il) {
            return m_data.insert(position, il);
        }

        inline iterator erase(const_iterator position) {
            return m_data.erase(position);
        }

        inline iterator erase(const_iterator first, const_iterator last) {
            return m_data.erase(first, last);
        }

        inline void swap(IntVector& other) {
            m_data.swap(other.m_data);
        }

        inline void clear() {
            m_data.clear();
        }

        template <class... Args>
        inline iterator emplace(const_iterator position, Args&&... args) {
            return m_data.emplace(position, std::forward<Args...>(args)...);
        }

        template <class... Args>
        inline void emplace_back(Args&&... args) {
            m_data.emplace_back(std::forward<Args...>(args)...);
        }

        template<class U>
        friend bool operator==(const IntVector<U>& lhs, const IntVector<U>& rhs);
        template<class U>
        friend bool operator!=(const IntVector<U>& lhs, const IntVector<U>& rhs);
        template<class U>
        friend bool operator<(const IntVector<U>& lhs, const IntVector<U>& rhs);
        template<class U>
        friend bool operator<=(const IntVector<U>& lhs, const IntVector<U>& rhs);
        template<class U>
        friend bool operator>(const IntVector<U>& lhs, const IntVector<U>& rhs);
        template<class U>
        friend bool operator>=(const IntVector<U>& lhs, const IntVector<U>& rhs);
        template<class U>
        friend void swap(IntVector<U>& lhs, IntVector<U>& rhs);

        inline size_t stat_allocation_size_in_bytes() const {
            return IntVectorTrait<T>::stat_allocation_size_in_bytes(m_data);
        }
    };

    template<class T>
    bool operator==(const IntVector<T>& lhs, const IntVector<T>& rhs) {
        return lhs.m_data == rhs.m_data;
    }

    template<class T>
    bool operator!=(const IntVector<T>& lhs, const IntVector<T>& rhs) {
        return lhs.m_data != rhs.m_data;
    }

    template<class T>
    bool operator<(const IntVector<T>& lhs, const IntVector<T>& rhs) {
        return lhs.m_data < rhs.m_data;
    }

    template<class T>
    bool operator<=(const IntVector<T>& lhs, const IntVector<T>& rhs) {
        return lhs.m_data <= rhs.m_data;
    }

    template<class T>
    bool operator>(const IntVector<T>& lhs, const IntVector<T>& rhs) {
        return lhs.m_data > rhs.m_data;
    }

    template<class T>
    bool operator>=(const IntVector<T>& lhs, const IntVector<T>& rhs) {
        return lhs.m_data >= rhs.m_data;
    }

    template<class T>
    void swap(IntVector<T>& lhs, IntVector<T>& rhs) {
        using std::swap;
        swap(lhs.m_data, rhs.m_data);
    }
}

/// \copydoc int_vector::IntVector
template<class T>
using IntVector = int_vector::IntVector<T>;

/// \brief Represents a bit vector, alias for \ref IntVector with a fixed
///        bit width of 1.
using BitVector = IntVector<uint_t<1>>;

/// \brief Represents an integer vector with unspecified (dynamic) bit
///        width.
///
/// The bit width defaults to 64 bits, but it can be changed at will via the
/// constructor, or later during runtime using the
/// \ref int_vector::IntVector::width(uint8_t) method.
using DynamicIntVector = IntVector<dynamic_t>;

}


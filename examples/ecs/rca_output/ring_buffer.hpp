
#include <utility>
#include <cstddef>
#include <new>
#include <initializer_list>
#include <algorithm>

//STL compatible container
//only supports the member functions I need.
template <typename T, std::size_t N>
class ring_buffer {
public:

    class iterator {
    public:

        iterator(ring_buffer* buffer, size_t index, size_t count)
            : buffer_(buffer), index_(index), remaining_(count) {}

        T& operator*() { return buffer_->data()[index_]; }
        T* operator->() { return &buffer_->data()[index_]; }

        iterator& operator++() {
            index_ = (index_ + 1) % N;
            --remaining_;
            return *this;
        }

        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const iterator& other) const {
            return buffer_ == other.buffer_ && remaining_ == other.remaining_;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        ring_buffer* buffer_;
        size_t index_;
        size_t remaining_;
    };
    class const_iterator {
    public:
        
        const_iterator(ring_buffer const* buffer, size_t index, size_t count)
            : buffer_(buffer), index_(index), remaining_(count) {}

        T const& operator*() { return buffer_->data()[index_]; }
        T const* operator->() { return &buffer_->data()[index_]; }

        const_iterator& operator++() {
            index_ = (index_ + 1) % N;
            --remaining_;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const const_iterator& other) const {
            return buffer_ == other.buffer_ && remaining_ == other.remaining_;
        }
        
        bool operator<(const const_iterator& other) const {
            return remaining_ > other.remaining_;
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }
        
        int operator-(const_iterator const& other) const {
            return -( remaining_ - other.remaining_);
        }

    private:
        ring_buffer const* buffer_;
        size_t index_;
        size_t remaining_;
    };

    ring_buffer() : start_(0), end_(0), count_(0) {}
    ring_buffer(std::initializer_list<T> values) : ring_buffer() {
        for(auto v : values) {
            push_back(std::move(v));
        }
    }
    ~ring_buffer() { clear(); }

    bool empty() const { return count_ == 0; }
    size_t size() const { return count_; }
    size_t count() const { return count_; }
    size_t capacity() const { return N; }

    void clear() {
        for (size_t i = 0; i < count_; ++i) {
            data()[(start_ + i) % N].~T();
        }
        count_ = 0;
        start_ = 0;
        end_ = 0;
    }

    void push_back(const T& value) {
        if (count_ == N) {
            data()[start_].~T();
            start_ = (start_ + 1) % N;
        } else {
            ++count_;
        }
        new (&data()[end_]) T(value);
        end_ = (end_ + 1) % N;
    }

    ring_buffer& operator=(const ring_buffer& other) {
        if (this != &other) {
            clear();
            for (const auto& item : other) {
                push_back(item);
            }
        }
        return *this;
    }

    const_iterator begin() const { return const_iterator(this, start_, count_); }
    const_iterator end() const { return const_iterator(this, end_, 0); }
    iterator begin() { return iterator(this, start_, count_); }
    iterator end() { return iterator(this, end_, 0); }
	T& operator[](size_t idx) {
		return data()[(start_ + idx) % N];
	}
	T const& operator[](size_t idx) const {
		return data()[(start_ + idx) % N];
	}
	T const& back() const {
		return (*this)[count_-1];
	}
	T& back() {
		return (*this)[count_-1];
	}
    bool operator<(ring_buffer const& rhs) const {
        if(size() != rhs.size()){
            return size() < rhs.size();
        }
        for(int i = 0; i < count_; ++i) {
            if((*this)[i] < rhs[i]) return true;
            if(rhs[i] < (*this)[i]) return false;
        }
        return false;
    }
private:
    alignas(T) char buffer_[N * sizeof(T)];
    size_t start_;
    size_t end_;
    size_t count_;

    T* data() { return reinterpret_cast<T*>(buffer_); }
    const T* data() const { return reinterpret_cast<const T*>(buffer_); }
};


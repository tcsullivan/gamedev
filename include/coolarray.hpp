#ifndef COOLARRAY_H_
#define COOLARRAY_H_

#include <memory>
#include <initializer_list>

template<class T>
class CoolArray {
private:
	T *buffer;
	size_t _size, _capacity;
public:
	CoolArray(void) {
		buffer = nullptr;
		_size = 0;
		_capacity = 0;
	}

	CoolArray(size_t n, const T& value=0) {
		buffer = new T[n];
		_size = n;
		_capacity = n;
		std::fill(buffer, buffer + _size, value);
	}

	~CoolArray(void) {
		delete[] buffer;
	}

	T& operator[](size_t idx) {
		return buffer[idx];
	}

	void operator=(std::initializer_list<T> a) {
		if (buffer != nullptr)
			delete[] buffer;

		_size = a.size();
		buffer = new T[_size];
		_capacity = _size;
		std::copy(a.begin(), a.end(), buffer);
	}

	template<class Func>
	void remove_if(Func f) {
		for (size_t i = 0; i < _size; ++i) {
			if (f(buffer[i]))
				std::move(buffer + i + 1, buffer + _size--, buffer + i);
		}
	}

	void reserve(size_t n) {
		if (buffer != nullptr) {
			T *newbuf = new T[n];
			std::copy(buffer, buffer + (_size < n ? _size : n), newbuf);
			_capacity = n;
			delete[] buffer;
			buffer = newbuf;
		} else {
			buffer = new T[n];
			_capacity = n;
		}
	}
		
	void resize(size_t n) {
		reserve(n);
		_size = n;
	}

	void clear(void) {
		delete[] buffer;
		_size = 0;
		_capacity = 0;
	}

	size_t size(void) const {
		return _size;
	}

	size_t capacity(void) const {
		return _capacity;
	}

	T& front(void) {
		return buffer[0];
	}

	T& back(void) {
		return buffer[_size - 1];
	}

	T* begin(void) {
		return buffer;
	}

	T* end(void) {
		return buffer + _size;
	}

	void push_back(const T& x) {
		if (_size >= _capacity)
			reserve(_capacity + 5);

		buffer[_size++] = x;
	}

	void pop_back(void) {
		--_size;
	}
};


#endif // COOLARRAY_H_

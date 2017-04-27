#ifndef VECTOR2_HPP_
#define VECTOR2_HPP_

#include <string>
#include <type_traits>

template<typename T>
struct vector2 {
	static_assert(std::is_arithmetic<T>::value, "vector2 members must be an arithmetic type (i.e. numbers)");

	T x, y;

	vector2(T _x = 0, T _y = 0)
		: x(_x), y(_y) {}

	// format: "3, 5"
	vector2(const std::string& s) {
		*this = s;
	}

	vector2<T>& operator=(const T& value) {
		x = y = value;
		return *this;
	}

	vector2<T>& operator=(const std::string& s) {
		auto comma = s.find(',');
		x = std::stoi(s.substr(0, comma));
		y = std::stoi(s.substr(comma + 1));
		return *this;
	}

	// addition
	vector2<T> operator+(const vector2<T>& v) const {
		return vector2<T>(x + v.x, y + v.y);
	}

	template<typename T2>
	vector2<T> operator+(const vector2<T2>& v) const {
		return vector2<T>(x + v.x, y + v.y);
	}

	vector2<T> operator+(const T& n) const {
		return vector2<T>(x + n, y + n);
	}

	vector2<T> operator+=(const vector2<T>& v) {
		x += v.x, y += v.y;
		return *this;
	}

	// subtraction
	vector2<T> operator-(const vector2<T>& v) const {
		return vector2<T>(x - v.x, y - v.y);
	}

	vector2<T> operator-(const T& n) const {
		return vector2<T>(x - n, y - n);
	}

	vector2<T> operator-=(const vector2<T>& v) {
		x -= v.x, y -= v.y;
		return *this;
	}

	// multiplication
	vector2<T> operator*(const vector2<T>& v) const {
		return vector2<T>(x * v.x, y * v.y);
	}

	vector2<T> operator*(const T& n) const {
		return vector2<T>(x * n, y * n);
	}

	vector2<T> operator*=(const vector2<T>& v) {
		x *= v.x, y *= v.y;
		return *this;
	}

	vector2<T> operator*=(const T& n) {
		x *= n, y *= n;
		return *this;
	}

	// division
	vector2<T> operator/(const vector2<T>& v) const {
		return vector2<T>(x / v.x, y / v.y);
	}

	vector2<T> operator/(const T& n) const {
		return vector2<T>(x / n, y / n);
	}

	vector2<T> operator/=(const vector2<T>& v) {
		x /= v.x, y /= v.y;
		return *this;
	}

	vector2<T> operator/=(const T& n) {
		x /= n, y /= n;
		return *this;
	}

	// compare
	bool operator==(const vector2<T>& v) const {
		return (x == v.x) && (y == v.y);
	}

	bool operator>(const vector2<T>& v) const {
		return (x > v.x) && (y > v.y);
	}

	bool operator<(const vector2<T>& v) const {
		return (x < v.x) && (y < v.y);
	}

	bool operator<=(const T& n) const {
		return (x <= n) && (y <= n);
	}

	// other functions
	std::string toString(void) const {
		return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
	}
};

using vec2 = vector2<float>;
using dim2 = vector2<int>;

#endif // VECTOR2_HPP_

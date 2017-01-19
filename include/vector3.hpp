#ifndef VECTOR3_HPP_
#define VECTOR3_HPP_

/**
 * A structure for three-dimensional points.
 */
template<typename T>
struct vector3 {
	T x; /**< The x coordinate */
	T y; /**< The y coordinate */
	T z; /**< The z coordinate */

	vector3(T _x = 0, T _y = 0, T _z = 1)
		: x(_x), y(_y), z(_z) {}
};

using vec3 = vector3<float>;

#endif // VECTOR3_HPP_

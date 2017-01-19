#ifndef COLOR_HPP_
#define COLOR_HPP_

/**
 * Keeps track of an RGBA color.
 */
class Color{
public:
	float red;   /**< The amount of red, 0-255 or 0.0-1.0 depending on usage */
	float green; /**< The amount of green */
	float blue;  /**< The amount of blue */
	float alpha; /**< Transparency */

	Color(float r = 0, float g = 0, float b = 0, float a = 255)
		: red(r), green(g), blue(b), alpha(a) {}

	Color operator-(const float& a) {
		return Color(red - a, green - a, blue - a, alpha);
	}

	Color operator+(const float& a) {
		return Color(red + a, green + a, blue + a, alpha);
	}
};

#endif // COLOR_HPP_

#ifndef WEATHER_HPP_
#define WEATHER_HPP_

#include <string>

#include <entityx/entityx.h>

#include <vector2.hpp>

extern vec2 offset;

/**
 * The weather type enum.
 * This enum contains every type of weather currently implemented in the game.
 * Weather is set by the world somewhere.
 */
enum class Weather : unsigned char {
	Sunny = 0, /**< Sunny */
	Rainy,     /**< Rain */
	Snowy,     /**< Snow */
	count
};

class WeatherSystem : public entityx::System<WeatherSystem> {
private:
	static Weather weather;

public:
	WeatherSystem(Weather w = Weather::Sunny);

	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override;

	static void setWeather(const std::string& w);
};

#endif // WEATHER_HPP_

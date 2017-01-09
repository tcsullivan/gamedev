#ifndef WEATHER_HPP_
#define WEATHER_HPP_

#include <entityx/entityx.h>

#include <common.hpp>
#include <particle.hpp>

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

constexpr const char *weatherStrings[] = {
	"Sunny",
	"Rainy",
	"Snowy"
};

class WeatherSystem : public entityx::System<WeatherSystem> {
private:
	Weather weather;

public:
	WeatherSystem(Weather w = Weather::Sunny)
		: weather(w) {}

	void update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt) override {
		(void)en;
		(void)ev;
		(void)dt;

		static auto& partSystem = *game::engine.getSystem<ParticleSystem>();
		static int newPartDelay = 0; // TODO no

		switch (weather) {
		case Weather::Sunny:
			break;
		case Weather::Rainy:
			if (newPartDelay++ == 4) {
				newPartDelay = 0;
				partSystem.add(vec2(offset.x - game::SCREEN_WIDTH / 2 + randGet() % game::SCREEN_WIDTH,
					offset.y + game::SCREEN_HEIGHT / 2 + 100),
					ParticleType::Drop, 3000, 3);
			}
			break; // TODO
		case Weather::Snowy:
			if (newPartDelay++ == 4) {
				newPartDelay = 0;
				partSystem.add(vec2(offset.x - game::SCREEN_WIDTH / 2 + randGet() % game::SCREEN_WIDTH,
					offset.y + game::SCREEN_HEIGHT / 2 + 100),
					ParticleType::Confetti, 6000, 0);
			}
			break; // TODO
		default:
			break;
		}
	}

	inline void setWeather(const std::string& w) {
		for (int i = 0; i < static_cast<int>(Weather::count); i++) {
			if (w == weatherStrings[i]) {
				weather = static_cast<Weather>(i);
				return;
			}
		}
	}
};

#endif // WEATHER_HPP_

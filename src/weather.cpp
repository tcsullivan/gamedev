#include <weather.hpp>

#include <config.hpp>
#include <random.hpp>
#include <particle.hpp>

constexpr const char *weatherStrings[] = {
	"Sunny",
	"Rainy",
	"Snowy"
};

Weather WeatherSystem::weather;

WeatherSystem::WeatherSystem(Weather w)
{
	weather = w;
}

void WeatherSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;

	static int newPartDelay = 0; // TODO no

	switch (weather) {
	case Weather::Sunny:
		break;
	case Weather::Rainy:
		if (newPartDelay++ == 4) {
			newPartDelay = 0;
			ParticleSystem::add(vec2(offset.x - game::SCREEN_WIDTH / 2 + randGet() % game::SCREEN_WIDTH,
				offset.y + game::SCREEN_HEIGHT / 2 + 100),
				ParticleType::Drop, 3000, 3);
		}
		break;
	case Weather::Snowy:
		if (newPartDelay++ == 6) {
			newPartDelay = 0;
			ParticleSystem::add(vec2(offset.x - game::SCREEN_WIDTH + randGet() % game::SCREEN_WIDTH * 2,
				offset.y + game::SCREEN_HEIGHT / 2 + 50),
				ParticleType::Confetti, 10000, 0);
		}
		break;
	default:
		break;
	}
}

void WeatherSystem::setWeather(const std::string& w)
{
	for (int i = 0; i < static_cast<int>(Weather::count); i++) {
		if (w == weatherStrings[i]) {
			weather = static_cast<Weather>(i);
			return;
		}
	}
}


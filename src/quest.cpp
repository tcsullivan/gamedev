#include <quest.hpp>

#include <algorithm>

#include <tokens.hpp>

void QuestSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;
}

int QuestSystem::assign(std::string title, std::string desc, std::string req)
{
	for (auto s : tokens(req, ',')) {
		s.erase(std::remove_if(s.begin(), s.end(),
			[](char c) { return isspace(c); }), s.end());

		std::cout << s << '\n';
	}

	current.emplace_back(title, desc);
	return 0;
}

int QuestSystem::drop(std::string title)
{
	current.erase(std::remove_if(std::begin(current), std::end(current),
		[&title](const Quest& q) { return (q.name == title); }));
	return 0;
}

int QuestSystem::finish(std::string title)
{
	auto quest = std::find_if(std::begin(current), std::end(current),
		[&title](const Quest& q) { return (q.name == title); });

	if (quest == std::end(current))
		return -1;

	// TODO requirements

	drop(title);

	return 0;
}

bool QuestSystem::hasQuest(std::string title)
{
	return (std::find_if(std::begin(current), std::end(current),
		[&title](const Quest& q) { return (q.name == title); }) != std::end(current));
}

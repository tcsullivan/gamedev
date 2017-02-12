#include <quest.hpp>

#include <algorithm>

#include <engine.hpp>
#include <inventory.hpp>
#include <tokens.hpp>

std::string& trim(std::string& s)
{
	auto start = std::find_if(s.begin(), s.end(),
		[](char c) { return !isspace(c); }) - s.begin();
	auto end = std::find_if(s.rbegin(), s.rend(),
		[](char c) { return !isspace(c); }).base() - s.begin();
	s = s.substr(start, end - start);
	return s;
}

void QuestSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;
}

int QuestSystem::assign(std::string title, std::string desc, std::string req)
{
	current.emplace_back(title, desc);

	auto* list = &current.back().reqs;
	std::string reqTitle;
	for (auto s : tokens(req, ',')) {
		trim(s);

		if (s == "Reward") {
			list = &current.back().rewards;
		} else {
			if (!reqTitle.empty()) {
				list->emplace_front(reqTitle, std::stoi(s));
				reqTitle.clear();
			} else {
				reqTitle = s;
			}
		}
	}

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

	for (const auto& r : quest->reqs) {
		if (!game::engine.getSystem<InventorySystem>()->take(r.first, r.second))
			return -1;
	}

	for (const auto& r : quest->rewards)
		game::engine.getSystem<InventorySystem>()->add(r.first, r.second);

	drop(title);

	return 0;
}

bool QuestSystem::hasQuest(std::string title)
{
	return (std::find_if(std::begin(current), std::end(current),
		[&title](const Quest& q) { return (q.name == title); }) != std::end(current));
}

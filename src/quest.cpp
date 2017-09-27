#include <quest.hpp>

#include <algorithm>
#include <iostream>
#include <fstream>

#include <engine.hpp>
#include <error.hpp>
#include <fileio.hpp>
#include <inventory.hpp>
#include <tokens.hpp>

std::vector<Quest> QuestSystem::current;

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
		if (!InventorySystem::take(r.first, r.second))
			return -1;
	}

	for (const auto& r : quest->rewards)
		InventorySystem::add(r.first, r.second);

	drop(title);

	return 0;
}

bool QuestSystem::hasQuest(std::string title)
{
	return (std::find_if(std::begin(current), std::end(current),
		[&title](const Quest& q) { return (q.name == title); }) != std::end(current));
}

void QuestSystem::save(void)
{
	std::ofstream s (game::config::xmlFolder + "quest.dat");

	// signature?
	s << "831998\n";

	for (const auto& q : current) {
		s << q.name << '\n' << q.desc << '\n';
		for (const auto& r : q.reqs)
			s << r.first << ',' << std::to_string(r.second) << ',';
		s << "Reward,";
		for (const auto& r : q.rewards)
			s << r.first << ',' << std::to_string(r.second) << ',';
		s << '\n';
	}
}

void QuestSystem::load(void)
{
	std::ifstream sf (game::config::xmlFolder + "quest.dat");
	if (sf.good()) {
		sf.close();
		auto lines = readFileA(game::config::xmlFolder + "quest.dat");

		// check signature
		if (std::stoi(lines[0]) != 831998)
			UserError("Quest save file signature is invalid... (delete it)");

		for (unsigned int i = 1; i < lines.size(); i += 3)
			assign(lines[i], lines[i + 1], lines[i + 2]);
	}
}

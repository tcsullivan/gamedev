#include <quest.hpp>

#include <algorithm>

std::vector<std::string> StringTokenizer(const std::string& str, char delim);

std::vector<std::string> split(std::string s, const std::string& delim)
{
	std::vector<std::string> res;

	while (!s.empty()) {
		auto pos = s.find(delim);
		res.emplace_back(s.substr(0, pos));
		s = s.substr(pos + 1);
	}

	return res;
}

void QuestSystem::update(entityx::EntityManager &en, entityx::EventManager &ev, entityx::TimeDelta dt)
{
	(void)en;
	(void)ev;
	(void)dt;
}

int QuestSystem::assign(std::string title, std::string desc, std::string req)
{
	const auto& reqs = StringTokenizer(req, ',');
	for (const auto& s : reqs)
		std::cout << s << '\n';

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

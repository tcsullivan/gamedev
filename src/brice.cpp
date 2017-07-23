#include <brice.hpp>

#include <unordered_map>
#include <string>
#include <fstream>
#include <istream>
#include <sstream>

#include <common.hpp>
#include <error.hpp>
#include <fileio.hpp>

static std::unordered_map<std::string, std::string> brice;

std::vector<std::string> StringTokenizer(const std::string& str, char delim);

namespace game {
	bool canJump;
	bool canSprint;

	std::string getValue(const std::string& id) {
		auto item = brice.find(id);
		
		if (item == std::end(brice))
			return "";

		return item->second;
	}

	bool setValue(const std::string& id, const std::string& value) {
		auto item = brice.find(id);
		if (item == std::end(brice)) {
			brice.emplace(std::make_pair(id, value));
			return false;
		} else {
			item->second = value;
			return true;
		}
	}

	void briceClear(void) {
		std::ofstream out ("brice.dat", std::ios::out);
		const std::string defaultt = "1\nSlow\n1\n";
		out.write(defaultt.data(), defaultt.size());
		out.close();
		briceLoad();
	}

	void briceSave(void) {
		std::ofstream out ("brice.dat", std::ios::out | std::ios::binary);
		std::string data = std::to_string(brice.size()) + '\n';

		UserAssert(out.is_open(), "Cannot open brice data file");

		for (const auto& i : brice) {
			data.append(i.first  + '\n');
			data.append(i.second + '\n');
		}

		out.write(data.data(), data.size());
		out.close();
	}

	void briceLoad(void) {
		if (!fileExists("brice.dat"))
			briceClear();
		auto data = readFile("brice.dat");

		if (data.empty()) {
			briceClear();
			data = readFile("brice.dat");
		}

		auto datas = StringTokenizer(data, '\n');

		if (datas.size() != 0) {
			const unsigned int count = datas[0][0] - '0';

			for (unsigned int i = 1; i <= count * 2; i += 2) {
				brice.emplace(std::make_pair(datas[i], datas[i + 1]));
			}
		}
	}

	void briceUpdate(void) {
		auto getIntValue = [&](const std::string& id) {
			int val;
			try {
				val = std::stoi(getValue(id));
			} catch (const std::invalid_argument &e) {
				val = 0;
			}
			return val;
		};
	
		// set default values
		canJump = false;
		canSprint = false;

		// attempt to load actual values
		canJump = getIntValue("canJump");
		canSprint = getIntValue("canSprint");

		// re-save values
		setValue("canJump", std::to_string(canJump));
		setValue("canSprint", std::to_string(canSprint));
	}
}

std::vector<std::string> StringTokenizer(const std::string& str, char delim)
{
	std::vector<std::string> tokens;
	std::istringstream is (str);
	std::string token;

	while (getline(is, token, delim))
		tokens.emplace_back(token);

	return tokens;
}


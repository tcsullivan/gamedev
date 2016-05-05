#include <brice.hpp>

#include <unordered_map>
#include <string>
#include <fstream>
#include <istream>

#include <common.hpp>

static std::unordered_map<std::string, std::string> brice;

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

	void briceSave(void) {
		std::ofstream out ("brice.dat", std::ios::out | std::ios::binary);
		std::string data = std::to_string(brice.size()) + '\n';

		if (!out.is_open())
			UserError("Cannot open brice data file");

		for (const auto& i : brice) {
			data.append(i.first  + ',' );
			data.append(i.second + '\n');
		}

		out.write(data.data(), data.size());
		out.close();
	}

	void briceLoad(void) {
		const std::string data = readFile("brice.dat");
		auto datas = StringTokenizer(data, ',');

		for (const auto& d : datas)
			std::cout << d << '\n';
	}

	void briceUpdate(void) {
		auto getIntValue = [&](const std::string& id) {
			int val;
			try {
				val = std::stoi(getValue(id));
			} catch (std::invalid_argument &e) {
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
	}
}

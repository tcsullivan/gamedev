#ifndef BRICE_H_
#define BRICE_H_

#include <unordered_map>
#include <string>
#include <istream>
#include <fstream>

#include <common.hpp>

class Brice {
private:
	std::unordered_map<std::string, std::string> ice;
public:
	Brice(void){}
	~Brice(void){}

	std::string getValue(const std::string& id) const {
		auto item = ice.find(id);
		return (item == std::end(ice)) ? "" : item->second;
	}

	void addValue(const std::string &id, const std::string& value) {
		ice.emplace(std::make_pair(id, value));
	}

	void save(void) const {
		std::ofstream out ("brice.dat", std::ios::out | std::ios::binary);
		std::string data = std::to_string(ice.size()) + '\n';

		if (!out.is_open())
			UserError("Cannot open brice data file");

		for (const auto& i : ice) {
			data.append(i.first  + ',' );
			data.append(i.second + '\n');
		}

		out.write(data.data(), data.size());
		out.close();
	}

	void load(void) {
		const std::string data = readFile("brice.dat");
	}
};

#endif // BRICE_H_

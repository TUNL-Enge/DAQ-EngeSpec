#include "toml.hpp"
#include "mdpp_module_config.h"
#include <iostream>

MDPPConfig read_from_toml(const std::string& filename) {
	// make sure we can parse the file
  try {
		toml::table module_table = toml::parse_file(filename);

		MDPPConfig module_map; // map that will associate bank names with module type and channel numbers

		// iterate over the module array and collect the relevant information
		toml::array& arr = *module_table.get_as<toml::array>("modules");
		std::cout << "Found configuration for " << arr.size() << " modules." << std::endl;
		for(auto&& el : arr) {
			// initialize
			MDPPModule module;

			// save some typing on the conditionals, * just unwraps the std::option
			std::string temp_name = *el.at_path("name").value<std::string>();
			std::string temp_mod_type = *el.at_path("mod_type").value<std::string>();
		
			if (temp_mod_type == "scp") {
				module.mod_type = scp;
			} else if (temp_mod_type == "qdc") {
				module.mod_type = qdc;
			}
		
			module.nchannels = *el.at_path("nchannels").value<int>();
			// put the info into the map
			module_map[temp_name] = module;	
		}
		
		return module_map;
	}
  catch (const toml::parse_error& err){
    std::cerr << "Parsing failed:\n" << err << "\n";
  }
}


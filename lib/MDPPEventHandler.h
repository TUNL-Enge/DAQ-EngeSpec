#ifndef MDPP_EVT_HAND_H
#define MDPP_EVT_HAND_H

#include "mdpp_module_config.h"
#include "manalyzer.h"
#include "midasio.h"
#include <vector>

using vec_u32 = std::vector<uint32_t>;

enum bitmasks {
	one_bit = 0x1,
	two_bit = 0x3,
	three_bit = 0x7,
	four_bit = 0xF,
	five_bit = 0x1F,
	seven_bit = 0x7F,
	eight_bit = 0xFF,
	ten_bit = 0x3FF,
	sixteen_bit = 0xFFFF,
	thirty_bit = 0x3FFFFFFF,
};


// This struct that holds the data for calling by the sort routine.

struct MDPPBank {
	MDPPBank(int nchannels);
	void clear_data();
	vec_u32 adc;
	vec_u32 tdc;
};


class MDPPEvent {

 public:

	MDPPEvent();
	~MDPPEvent(){};

	// 
	void read_banks(TMEvent* event);
	// return reference to the pared bank data
	MDPPBank& get_data(const std::string& bank_name);
	// free up the banks for the next event
	void clear_data();
	
 private:
	MDPPConfig module_map;
	// we store the banks in the vector
	std::vector<MDPPBank> data_vector;
	// banks can still be referenced by mapping names to the vector location;
	std::map<std::string, int> data_map;
	void parse_data(int module_num, int nchannels, uint32_t* dMDPP, int nMDPP);
	int nmodules;
	
};

#endif

#include "manalyzer.h"
#include "midasio.h"
#include "mdpp_module_config.h"
#include "MDPPEventHandler.h"
#include <vector>
#include <iostream>

MDPPBank::MDPPBank(int nchannels)
{
	// initialize the adc and tdc vectors
	this->adc.resize(nchannels);
	std::fill(this->adc.begin(), this->adc.end(), 0);

	this->tdc.resize(nchannels);
	std::fill(this->tdc.begin(), this->tdc.end(), 0);
}

void MDPPBank::clear_data()
{
	std::fill(this->adc.begin(), this->adc.end(), 0);
	std::fill(this->tdc.begin(), this->tdc.end(), 0);
}

/*
MDPPEvent code. This class wraps up the logic for the parser and module layout into one.
It will be passed

 */

MDPPEvent::MDPPEvent()
{
	// hard coded for now
	this->module_map = read_from_toml(
		"/home/jam/caleb-stuff/setup-files/ecr-beamline-mdpp-setup.toml");
	// go ahead and set up the data vector and map
	int i = 0;
	for (auto m : this->module_map) {
		std::cout << "Scanning module: " << m.first << std::endl;
		MDPPBank temp(m.second.nchannels);
		this->data_vector.push_back(temp);
		this->data_map[m.first] = i;
		i += 1;
	}
	this->nmodules = i;
}

// Turn a set of banks into a single vector that can be sorted.
void MDPPEvent::read_banks(TMEvent *event)
{
	// loop through the modules and see what we have
	for (auto m : this->module_map) {
		TMBank *midas_bank = event->FindBank(m.first.c_str());
		// FindBank throws NULL if it doesn't find anything.
		if (midas_bank) {
			// pull the data out
			uint32_t *dMDPP =
				(uint32_t *)event->GetBankData(midas_bank);
			int nMDPP = (midas_bank->data_size) / 4;
			// skip junk
			if (nMDPP == 1) {
				continue;
			}
			// get the MDPPBank information
			int module_num = this->data_map[m.first];
			this->parse_data(module_num, m.second.nchannels, dMDPP,
					 nMDPP);
		}
	}
}

void MDPPEvent::parse_data(int module_num, int nchannels, uint32_t *dMDPP,
			   int nMDPP)
{
	// taken from the old sort routine
	int chan_mask;
	if (nchannels == 16) {
		chan_mask = bitmasks::five_bit;
	} else if (nchannels == 32) {
		chan_mask = bitmasks::seven_bit;
	}
	// retrieve the bank
	MDPPBank &mdpp_bank = this->data_vector[module_num];

	for (int i = 0; i < nMDPP; i++) {
		uint32_t data_word = dMDPP[i];

		// just grab the data events and ignore extended timestamps
		if ((((data_word >> 30) & bitmasks::two_bit) == 0) &&
		    (((data_word >> 28) & bitmasks::two_bit) == 1)) {
			int raw_channel = (dMDPP[i] >> 16) & chan_mask;

			// gets rounded down to give the case statement
			int chan_case = raw_channel / nchannels;

			switch (chan_case) {
			case 0:
				// ADC or Long Integration value
				mdpp_bank.adc[raw_channel] =
					(data_word & bitmasks::sixteen_bit);
				break;

			case 1:
				// TDC value
				mdpp_bank.tdc[(raw_channel - nchannels)] =
					(data_word & bitmasks::sixteen_bit);
				break;

			default:
				// pass for now, probably should raise an error.
				break;
			}
		}
	}
}

MDPPBank &MDPPEvent::get_data(const std::string &bank_name)
{
	int bank_num = this->data_map[bank_name];
	return this->data_vector[bank_num];
}

void MDPPEvent::clear_data()
{
	// ready the banks for more data
	for (int i = 0; i < this->nmodules; i++) {
		this->data_vector[i].clear_data();
	}
}

#ifndef MDPP_MOD_CONFIG_H                                       
#define MDPP_MOD_CONFIG_H                                       
#include "toml.hpp"                                  
#include <string>                                    
#include <map>                                       


enum ModuleType { qdc, scp };                                                   
                                                     
struct MDPPModule {                                  
  ModuleType mod_type;                               
  int nchannels;                                     
};                                                   

using MDPPConfig = std::map<std::string, MDPPModule>;

MDPPConfig read_from_toml(const std::string& filename);
                                                     
#endif                                               

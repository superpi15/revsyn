#ifndef ntk_hpp
#define ntk_hpp

#include <iostream>
#include <vector>
#include <utility>
#include <string>
#include <cstring>

#include "lembit.hpp"


class Rev_Ntk_t {
	typedef std::vector<vLembit_t<1> * > vLevel_t;
	vLevel_t _vLevel;
public:
	;
};

extern Rev_Ntk_t * Rev_Gbd( const Rev_Ttb_t& ttb );

#endif
#pragma once

#include "id_generator.h"
typedef CID_Generator<
	u32,		// time identifier type
	u8,			// compressed id type 
	u16,		// id type
	u8,			// block id type
	u16,		// chunk id type
	0,			// min value
	u16(-2),	// max value
	256,		// block size
	u16(-1)		// invalid id
> id_generator_type;
id_generator_type m_tID_Generator; 

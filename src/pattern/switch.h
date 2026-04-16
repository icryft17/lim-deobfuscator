#ifndef PATTERN_SWITCH
#define PATTERN_SWITCH


#include "acprotect.h"
#include "global.h"

namespace pattern
{
	bool Go(type::VirtualInstruction instruction, type::PatternContext *pattern_ctx);

	bool FinishGo(type::VirtualInstruction instruction, type::PatternContext* pattern_ctx);
	// go block // instruction


	bool TemplatePattern(type::PatternContext* pattern_ctx);
	// decoder -> nop //  o n


	// decode with not nop 
} // namespace pattern
#endif

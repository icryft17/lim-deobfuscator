#ifndef ACPROTECT_H
#define ACPROTECT_H

namespace pattern
{
	namespace ac
	{
		bool JB_JAE_JNP_JB_JL_JGE_JMP_MEM(type::PatternContext *pattern_ctx);

		bool CLD_CLD_STC_STC_CLC_STC(type::PatternContext *pattern_ctx);

		bool MOV_XOR_SUB(type::PatternContext *pattern_ctx);

		bool INC_DEC(type::PatternContext* pattern_ctx);

		bool PUSH_POP(type::PatternContext *pattern_ctx);

		bool CALL_MEM(type::PatternContext *pattern_ctx);
	} // namespace ac
} // namespace pattern

#endif

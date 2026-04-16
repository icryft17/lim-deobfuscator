#include "switch.h"

namespace pattern
{
	bool FinishGo(type::VirtualInstruction instruction, type::PatternContext *pattern_ctx)
	{
		ZydisDisassembledInstruction current_instruction = instruction.Get();
		switch (current_instruction.info.meta.category)
		{
		case ZYDIS_CATEGORY_PUSH:
		case ZYDIS_CATEGORY_POP:
		{
			if (pattern::ac::PUSH_POP(pattern_ctx))
				return true;

			break;
		}

		case ZYDIS_CATEGORY_UNCOND_BR:
		case ZYDIS_CATEGORY_COND_BR:
		{
			if (pattern::global::JB_JAE_JNP_JB_JL_JGE_JMP_NOP(pattern_ctx))
				return true;
			break;
		}
		default:
		{
			if (instruction.Is(ZYDIS_MNEMONIC_MOV))
				if (pattern::ac::MOV_XOR_SUB(pattern_ctx))
					return true;
		}
		}
		return false;
	}

} // namespace pattern

#include "switch.h"

namespace pattern
{
	bool Go(type::VirtualInstruction instruction, type::PatternContext *pattern_ctx)
	{
		type::VirtualBlock *current_block = pattern_ctx->GetBlock();

		ZydisDisassembledInstruction disassm_instruction = instruction.Get();

		switch (disassm_instruction.info.meta.category)
		{
		case ZYDIS_CATEGORY_PUSH:
		case ZYDIS_CATEGORY_POP:
		{
			if (pattern::ac::PUSH_POP(pattern_ctx))
				return true;

			break;
		}

		case ZYDIS_CATEGORY_CALL:
		{
			if (pattern::ac::CALL_MEM(pattern_ctx))
				return true;

			break;
		}
		case ZYDIS_CATEGORY_COND_BR:
		case ZYDIS_CATEGORY_UNCOND_BR:
		{
			if (pattern::ac::JB_JAE_JNP_JB_JL_JGE_JMP_MEM(pattern_ctx))
				return true;

			if (pattern::global::JB_JAE_JNP_JB_JL_JGE_JMP_NOP(pattern_ctx))
				return true;

			break;
		}
		default:
			if (instruction.Is(ZYDIS_MNEMONIC_CLD) || instruction.Is(ZYDIS_MNEMONIC_STC) ||
				instruction.Is(ZYDIS_MNEMONIC_CLC))
				pattern::ac::CLD_CLD_STC_STC_CLC_STC(pattern_ctx);

			if (instruction.Is(ZYDIS_MNEMONIC_DEC) || instruction.Is(ZYDIS_MNEMONIC_INC))
				pattern::ac::INC_DEC(pattern_ctx);

			if (instruction.Is(ZYDIS_MNEMONIC_XOR) || instruction.Is(ZYDIS_MNEMONIC_SUB) ||
				instruction.Is(ZYDIS_MNEMONIC_ADD))
				pattern::ac::MOV_XOR_SUB(pattern_ctx);
		}

		/*

			push

			pop
			block

		*/

		return false;
	}

	bool TemplatePattern(type::PatternContext *pattern_ctx)
	{
		type::Context *ctx = pattern_ctx->GetContext();
		type::VirtualBlock *current_block = pattern_ctx->GetBlock();
		type::ControlFlowGraph *graph = pattern_ctx->Graph();

		return false;
	}
} // namespace pattern

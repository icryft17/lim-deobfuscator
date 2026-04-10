#include "global.h"

namespace pattern
{
	namespace global
	{
		bool JB_JAE_JNP_JB_JL_JGE_JMP_NOP(type::PatternContext *pattern_ctx)
		{
			type::Context *ctx = pattern_ctx->GetContext();
			IMAGE_NT_HEADERS *header = ctx->current_file->GetPEHeader();

			type::VirtualBlock *block = pattern_ctx->GetBlock();
			type::VirtualInstruction branch_instruction = block->LastInstruction();
			ZydisDisassembledInstruction branch_disass_instruction = branch_instruction.Get();

			if (branch_instruction.IsCatagory(ZYDIS_CATEGORY_RET))
				return false;

			bool is_delete = false;

			std::size_t address =
				ABSOLUTE_ADDRESS(branch_instruction.RuntimeAddress(), branch_disass_instruction.operands[0].imm.value.u,
								 branch_instruction.Size()) -
				1;
			std::uint8_t *buffer =
				&ctx->current_file
					 ->Buffer()[ctx->current_file->GetRAWValue(address - header->OptionalHeader.ImageBase)];

			std::int32_t count = branch_disass_instruction.operands[0].imm.value.s;
			if (!count)
			{
				return false;
			}

			while (true)
			{
				if (!count)
				{
					LOG_DEBUG("remover branch with NOP {}", branch_disass_instruction.text);
					is_delete = true;
					break;
				}

				type::VirtualInstruction instruction =
					util::DecodeInstruction(branch_disass_instruction.info.machine_mode, buffer, address);

				ZydisDisassembledInstruction disassm_instruction = instruction.Get();

				if (!instruction.Is(ZYDIS_MNEMONIC_NOP))
					break;

				count = count - 1;
				buffer = buffer - 1;
				address = address - 1;
			}

			if (is_delete)
			{
				std::memset(branch_instruction.PhysicalAddress(), 0x90, branch_instruction.Size());

				return true;
			}
			return false;
		}
	} // namespace global
} // namespace pattern

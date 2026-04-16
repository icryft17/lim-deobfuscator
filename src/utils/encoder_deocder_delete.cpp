#include "encoder_decoder_delete.h"

namespace util
{
	type::VirtualInstruction DecodeInstruction(ZydisMachineMode mode, std::uint8_t *object, std::size_t runtime_address)
	{
		ZydisDisassembledInstruction local_instruction;
		if (ZYAN_SUCCESS(
				ZydisDisassembleIntel(mode, runtime_address, object, ZYDIS_MAX_INSTRUCTION_LENGTH, &local_instruction)))
		{
			return type::NewInstruction(local_instruction, object);
		}
		return {};
	}

	type::VirtualInstruction DecodeWithNotNOPInstruction(ZydisMachineMode mode, std::uint8_t *object,
														 std::size_t runtime_address)
	{
		while (*object == 0x90)
			object++;

		return DecodeInstruction(mode, object, runtime_address);
	}

	bool EncoderInstruction(ZydisMachineMode mode, std::uint8_t *object, type::VirtualInstruction instruction)
	{
		ZydisDisassembledInstruction disassm_instruction = instruction.Get();
		ZydisEncoderRequest req{};

		if (!ZYAN_SUCCESS(
				ZydisEncoderDecodedInstructionToEncoderRequest(&disassm_instruction.info, disassm_instruction.operands,
															   disassm_instruction.info.operand_count_visible, &req)))
			return false;
		std::size_t lenght = sizeof(req);

		return ZYAN_SUCCESS(ZydisEncoderEncodeInstruction(&req, object, &lenght));
	}

	void DeleteInstruction(type::VirtualBlock *current_block, type::VirtualInstruction instruction)
	{
		if (!current_block)
			return;

		type::List<type::VirtualInstruction> instructions = current_block->List();
		for (auto it = instructions.Begin(); it != instructions.End()->next; it = it->Next())
		{
			type::VirtualInstruction virtual_instruction = it->Value();
			if (virtual_instruction.RuntimeAddress() == instruction.RuntimeAddress())
				current_block->DeleteInstruction(it);
		}
	}
} // namespace util

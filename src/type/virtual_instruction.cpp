#include "virtual_instruction.h"

namespace type
{
	ZydisDisassembledInstruction VirtualInstruction::Get() { return instruction; }

	bool VirtualInstruction::Is(ZydisMnemonic mnemonic) { return instruction.info.mnemonic == mnemonic; }

	std::uint8_t *VirtualInstruction::PhysicalAddress() { return physical_address; }

	std::size_t VirtualInstruction::RuntimeAddress() { return instruction.runtime_address; }

	std::size_t VirtualInstruction::Size() noexcept { return instruction.info.length; }

	VirtualInstruction::VirtualInstruction(ZydisDisassembledInstruction instruction, std::uint8_t *physical_address)
		: instruction(instruction), physical_address(physical_address)
	{
	}

	bool VirtualInstruction::IsCatagory(ZydisInstructionCategory category)
	{
		return instruction.info.meta.category == category;
	}

	void VirtualInstruction::SetOperand(std::int16_t i, ZydisDecodedOperand opernad)
	{
		instruction.operands[i] = opernad;
#ifdef HARDCORE_DEOBFUSCATION
		util::EncoderInstruction(instruction.info.machine_mode, physical_address, *this);
#endif //  HARDCORE_DEOBFUSCATION
	}

	VirtualInstruction NewInstruction(ZydisDisassembledInstruction instruction, std::uint8_t *physical_address)
	{
		return VirtualInstruction(instruction, physical_address);
	}
} // namespace type

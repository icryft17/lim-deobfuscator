#ifndef VIRTUAL_INSTRUCTION_H
#define VIRTUAL_INSTRUCTION_H

namespace type
{
	struct VirtualInstruction
	{
	  public:
		VirtualInstruction() = default;

		VirtualInstruction(ZydisDisassembledInstruction instruction, std::uint8_t *physical_address);

	  public:
		ZydisDisassembledInstruction Get();

		bool Is(ZydisMnemonic mnemonic);

		bool IsCatagory(ZydisInstructionCategory category);

		std::size_t RuntimeAddress();

		std::uint8_t *PhysicalAddress();
		// RuntimeAddress
		std::size_t Size() noexcept;

		void SetOperand(std::int16_t i, ZydisDecodedOperand opernad);

	  private:
		std::uint8_t *physical_address = nullptr;

		ZydisDisassembledInstruction instruction;
	};

	VirtualInstruction NewInstruction(ZydisDisassembledInstruction instruction, std::uint8_t *physical_address);
} // namespace type
#endif // !VIRTUAL_INSTRUCTION_H

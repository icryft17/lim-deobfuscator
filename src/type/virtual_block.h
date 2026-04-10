#ifndef VIRTUAL_BLOCK_H
#define VIRTUAL_BLOCK_H

#include "list.h"
#include "math.h"

#include "virtual_edge.h"
#include "virtual_instruction.h"

namespace type
{

	typedef enum block_type
	{
		EntryBlock,
		ExitBlock,
		DefaultBlock,

		LoopHeader,
		LoopPreHeader,
	} BlockType;

	class VirtualBlock
	{
	  public:
		VirtualBlock() = default;

		VirtualBlock(block_type type, std::uint8_t *physical_address, std::size_t runtime_address);

	  public:
		void Insert(VirtualInstruction instruction);

		bool IsInstruction(ZydisMnemonic menmonic) noexcept;

		bool IsWrite() noexcept;

		std::size_t RuntimeAddress() noexcept;

		std::size_t Size() noexcept;

		std::uint8_t *PhysicalAddress() noexcept;

		type::List<VirtualInstruction> &List();


		type::VirtualInstruction LastInstruction();

		type::VirtualInstruction FirstInstruction();

		BlockType Type();

		void SetInstruction(Node<type::VirtualInstruction> *it, type::VirtualInstruction instruction);

		void SetIsWrite() noexcept;


		void DeleteInstruction(Node<type::VirtualInstruction> *it);

		void InsertPresente(VirtualEdge edge);

		void InsertEdge(VirtualEdge edge);

		void Clear();

		tsl::robin_map<std::size_t, VirtualEdge> Presentes();

		tsl::robin_map<std::size_t, VirtualEdge> Edges();

	  private:
		block_type type;

		tsl::robin_map<std::size_t, VirtualEdge> presentes;
		tsl::robin_map<std::size_t, VirtualEdge> edges;

		std::uint8_t *physical_address = nullptr;
		std::size_t runtime_address = 0;
		std::size_t size = 0;
		std::bitset<ZYDIS_MNEMONIC_MAX_VALUE> instructions_map;

		bool is_write = false;

		type::List<VirtualInstruction> instructions;
	};

	VirtualBlock *NewBasicBlock(block_type type, std::uint8_t *physical_address, std::size_t runtime_address);
} // namespace type
#endif // !VIRTUAL_BLOCK_HG

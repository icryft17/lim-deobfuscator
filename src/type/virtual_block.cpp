#include "virtual_block.h"

namespace type
{

	VirtualBlock::VirtualBlock(block_type type, std::uint8_t *physical_address, std::size_t runtime_address)
		: physical_address(physical_address), runtime_address(runtime_address), type(type)
	{
	}
	bool VirtualBlock::IsInstruction(ZydisMnemonic menmonic) noexcept { return instructions_map[menmonic]; }

	bool VirtualBlock::IsWrite() noexcept { return is_write; }

	bool VirtualBlock::Empty() noexcept { return size == 0; }

	bool VirtualBlock::IsPacked() noexcept
	{
		// select all bytes
		tsl::robin_map<std::uint8_t, std::int8_t> bytes;
		for (std::size_t i = 0; i < size; i++)
			bytes[physical_address[i]]++;

		float entropy = 0;
		for (auto pair : bytes)
		{
			float l = (float)pair.second / (float)size;
			if (l > 0)
				entropy -= l * log2f(l);
		}
		return entropy > 7.f;
	}

	void VirtualBlock::SetIsWrite() noexcept { this->is_write = !this->is_write; }
	std::size_t VirtualBlock::RuntimeAddress() noexcept { return runtime_address; }

	std::size_t VirtualBlock::Size() noexcept { return size; }

	void VirtualBlock::InsertPresente(VirtualEdge edge) { edges[edge.From()] = edge; }

	void VirtualBlock::InsertEdge(VirtualEdge edge) { edges[edge.To()] = edge; }

	std::uint8_t *VirtualBlock::PhysicalAddress() noexcept { return physical_address; }

	type::VirtualInstruction VirtualBlock::LastInstruction() { return instructions.Last()->Value(); }

	type::VirtualInstruction VirtualBlock::FirstInstruction() { return instructions.First()->Value(); }

	type::List<VirtualInstruction> &VirtualBlock::List() { return instructions; }

	void SetInstruction(Node<type::VirtualInstruction> *it, type::VirtualInstruction instruction)
	{
		if (!it)
			return;
		it->SetValue(instruction);
	}
	void VirtualBlock::DeleteInstruction(Node<type::VirtualInstruction> *it)
	{
		if (!it)
			return;

		size -= it->Value().Size();
		instructions.Remove(it);

#ifdef HARDCORE_DEOBFUSCATION
		std::memset(it->value.PhysicalAddress(), 0x90, it->value.Size());
#endif
	}

	void VirtualBlock::Insert(VirtualInstruction instruction)
	{
		instructions.Push(instruction);
		size += instruction.Size();
		instructions_map.set(instruction.Get().info.mnemonic);
	}

	void VirtualBlock::Clear()
	{
		instructions.Clear();
		instructions_map.reset();
		size = 0;
	}

	tsl::robin_map<std::size_t, VirtualEdge> VirtualBlock::Presentes() { return presentes; }

	tsl::robin_map<std::size_t, VirtualEdge> VirtualBlock::Edges() { return edges; }

	BlockType VirtualBlock::Type() { return type; }

	VirtualBlock *NewBasicBlock(block_type type, std::uint8_t *physical_address, std::size_t runtime_address)
	{
		return new VirtualBlock(type, physical_address, runtime_address);
	}
} // namespace type

#include "control_flow_graph.h"

namespace type
{
	ControlFlowGraph::ControlFlowGraph(VirtualBlock *entry_block) : entry_block(entry_block) {}

	ControlFlowGraph *NewControlFlowGraph(VirtualBlock *entry_block) { return new ControlFlowGraph(entry_block); }

	bool ControlFlowGraph::Empty() noexcept { return map_blocks.empty(); }

	void ControlFlowGraph::InsertNode(VirtualBlock *block)
	{
		if (Exists(block->RuntimeAddress()))
		{
			block->Clear();
			free(block);
			return;
		}

		size++;
		std::size_t runtime_address = block->RuntimeAddress();
		for (std::size_t i = 0; i < (block->Size()); i++)
			map_blocks[runtime_address + i] = block;

		// map_blocks[block->RuntimeAddress()] = block;
	}

	void ControlFlowGraph::InsertEdge(VirtualEdge edge)
	{
		if (!Exists(edge.From()) || !Exists(edge.To()))
		{
			// LOG_DEBUG("Error init edge from {} to {}", edge.From(), edge.To());
			return;
		}

		map_blocks[edge.From()]->InsertEdge(edge);
		map_blocks[edge.To()]->InsertPresente(edge);
	}

	VirtualBlock *ControlFlowGraph::EntryBlock() noexcept { return entry_block; }

	std::size_t ControlFlowGraph::Size() noexcept { return size; }

	VirtualBlock *ControlFlowGraph::Get(std::size_t address) noexcept { return map_blocks[address]; }

	bool ControlFlowGraph::Exists(std::size_t runtime_address)
	{
		return !(map_blocks.find(runtime_address) == map_blocks.end());
	}
} // namespace type

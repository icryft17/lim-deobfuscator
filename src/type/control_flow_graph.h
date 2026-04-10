#ifndef CONTROL_FLOW_GRAPH
#define CONTROL_FLOW_GRAPH

#include "virtual_block.h"

namespace type
{
	class ControlFlowGraph
	{
	  public:
		ControlFlowGraph() = default;
		ControlFlowGraph(VirtualBlock *entry_block);

	  public:
		bool Empty() noexcept;

		std::size_t Size() noexcept;
		
		void InsertNode(VirtualBlock *block);

		void InsertEdge(VirtualEdge edge);

		bool Exists(std::size_t runtime_address);

		VirtualBlock* EntryBlock() noexcept;

		VirtualBlock* Get(std::size_t address) noexcept;

	  public:
		tsl::robin_map<std::size_t, VirtualBlock *> map_blocks;
		std::size_t size = 0;
		VirtualBlock *entry_block = nullptr;
	};
	ControlFlowGraph *NewControlFlowGraph(VirtualBlock *entry_block);
} // namespace type

#endif

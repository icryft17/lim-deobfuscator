#ifndef CONTEXT_H
#define CONTEXT_H

#include "type/control_flow_graph.h"
#include "type/virtual_block.h"

#include "utils/encoder_decoder_delete.h"
#include "utils/virtual_file.h"

namespace type
{
	enum ProtectorType
	{
		ACProtect
	};

	struct Context
	{
		util::file::VirtualFileBuilder *current_file = nullptr;
	};

	class PatternContext
	{
	  public:
		PatternContext() = default;
		PatternContext(Context *ctx, type::VirtualBlock *virtual_block, type::ControlFlowGraph *graph, bool is_pred);

	  public:
		void SetContext(Context *ctx) noexcept;
		void SetVirtualBlock(type::VirtualBlock *virtual_block) noexcept;
		void SetVirtualGraph(type::ControlFlowGraph *graph) noexcept;
		void SetIsPred(bool pred) noexcept;

		bool IsPred() noexcept;

		ProtectorType Protector();

		Context *GetContext() noexcept;

		type::VirtualBlock *GetBlock() noexcept;

		type::ControlFlowGraph *Graph() noexcept;

		// graph
	  private:
		ProtectorType protector_type;

		bool is_pred = false;

		type::Context *ctx = nullptr;
		type::VirtualBlock *virtual_block = nullptr;
		type::ControlFlowGraph *graph = nullptr;
	};

	type::Context *NewContext(util::file::VirtualFileBuilder *current_file);

	PatternContext *NewPatternContext(type::Context *ctx, type::VirtualBlock *virtual_block,
									  type::ControlFlowGraph *graph, bool is_pred);
} // namespace type
#endif

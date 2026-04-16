#include "context.h"

namespace type
{
	Context *NewContext(util::file::VirtualFileBuilder *current_file)
	{
		return new Context{
			current_file,
		};
	}
	PatternContext::PatternContext(Context *ctx, type::VirtualBlock *virtual_block, type::ControlFlowGraph *graph,
								   bool is_pred)
		: ctx(ctx), virtual_block(virtual_block), graph(graph), is_pred(is_pred)
	{
	}

	void PatternContext::SetContext(Context *ctx) noexcept { this->ctx = ctx; }

	void PatternContext::SetVirtualGraph(type::ControlFlowGraph *graph) noexcept { this->graph = graph; }

	void PatternContext::SetVirtualBlock(type::VirtualBlock *virtual_block) noexcept
	{
		this->virtual_block = virtual_block;
	}

	void PatternContext::SetIsPred(bool pred) noexcept { this->is_pred = pred; }

	bool PatternContext::IsPred() noexcept { return is_pred; }

	ProtectorType PatternContext::Protector() { return protector_type; }

	Context *PatternContext::GetContext() noexcept { return this->ctx; }

	type::VirtualBlock *PatternContext::GetBlock() noexcept { return this->virtual_block; }

	type::ControlFlowGraph *PatternContext::Graph() noexcept { return graph; }

	PatternContext *NewPatternContext(Context *ctx, type::VirtualBlock *virtual_block, type::ControlFlowGraph *graph,
									  bool is_pred)
	{
		return new PatternContext{ctx, virtual_block, graph, is_pred};
	}
} // namespace type

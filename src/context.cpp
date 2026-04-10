#include "context.h"

namespace type
{
	Context *NewContext(util::file::VirtualFileBuilder *current_file)
	{
		return new Context{
			current_file,
		};
	}
	PatternContext::PatternContext(Context *ctx, type::VirtualBlock *virtual_block)
		: ctx(ctx), virtual_block(virtual_block)
	{
	}

	void PatternContext::SetContext(Context *ctx) noexcept { this->ctx = ctx; }

	void PatternContext::SetVirtualBlock(type::VirtualBlock *virtual_block) noexcept
	{
		this->virtual_block = virtual_block;
	}

	Context *PatternContext::GetContext() noexcept { return this->ctx; }

	type::VirtualBlock *PatternContext::GetBlock() noexcept { return this->virtual_block; }

	PatternContext *NewPatternContext(Context *ctx, type::VirtualBlock *virtual_block)
	{
		return new PatternContext{
			ctx,
			virtual_block,
		};
	}
} // namespace type

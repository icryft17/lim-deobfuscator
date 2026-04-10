#include "virtual_edge.h"

namespace type
{
	VirtualEdge::VirtualEdge(EdgeType type, std::size_t from, std::size_t to) : type(type), to(to), from(from) {}

	EdgeType VirtualEdge::Type() noexcept { return type; }

	std::size_t VirtualEdge::From() noexcept { return from; }

	std::size_t VirtualEdge::To() noexcept { return to; }

	VirtualEdge NewEdge(EdgeType type, std::size_t from, std::size_t to) { return VirtualEdge(type, from, to); }
} // namespace type

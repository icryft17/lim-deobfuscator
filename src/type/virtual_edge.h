#ifndef VIRTUAL_EDGE_H
#define VIRTUAL_EDGE_H

namespace type
{
	typedef enum edge_type
	{
		NegativeEdge,
		PositiveEdge,
	} EdgeType;

	class VirtualEdge
	{
	  public:
		VirtualEdge() = default;

		VirtualEdge(EdgeType type, std::size_t from, std::size_t to);

	  public:
		EdgeType Type() noexcept;

		std::size_t From() noexcept;
		std::size_t To() noexcept;

	  private:
		edge_type type;

		std::size_t from = 0;
		std::size_t to = 0;
	};

	VirtualEdge NewEdge(EdgeType type, std::size_t from, std::size_t to);
} // namespace type
#endif

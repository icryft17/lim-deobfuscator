#ifndef TYPE_LIST
#define TYPE_LIST

namespace type
{
	template <typename T> class Allocator
	{
	  public:
		Allocator() = default;

	  public:
		T *New()
		{
			T *ptr = new T;
			allocate_address.push_back((void *)ptr);
			return ptr;
		}

		std::size_t Size() { return size; }

		std::size_t Capacity() { return capacity; }

		void Free()
		{
			for (std::size_t i = 0; i < allocate_address.size(); i++)
				free(allocate_address[i]);
			allocate_address.clear();
		}

	  private:
		std::vector<void *> allocate_address;
		std::size_t size = 0;
		std::size_t capacity = 0;
	};

	template <typename T> class Node
	{
	  public:
		Node() = default;

	  public:
		Node<T> *Next() noexcept { return next; }

		Node<T> *Prev() noexcept { return prev; }

		T Value() noexcept { return value; }

		void SetValue(T value) { this->value = value; }

		void SetNext(Node<T> *elemnet) { this->next = elemnet; }

		void SetPrev(Node<T> *elemnet) { this->prev = elemnet; }

	  public:
		T operator*() { return value; }

	  public:
		Node *next = nullptr;
		Node *prev = nullptr;
		T value;
	};

	template <typename T> class List
	{
	  public:
		List() = default;

	  public:
		std::size_t Size() noexcept { return size; }

		bool Empty() { return size; }

		Node<T> *Begin() { return node; }

		Node<T> *End() { return current_node; }

		Node<T> *First() { return node; }

		Node<T> *Last() { return current_node->prev; }

		void Push(T value)
		{
			if (!node)
			{
				node = allocator.New();
				current_node = node;
			}

			current_node->value = value;
			current_node->next = allocator.New();
			current_node->next->prev = current_node;
			current_node = current_node->next;

			size++;
		}

		void Remove(Node<T> *element)
		{
			if (!element)
				return;

			if (element->prev)
				element->prev->next = element->next;

			if (element->next)
				element->next->prev = element->prev;

			size--;
		}

		void Clear()
		{
			allocator.Free();

			size = 0;
			node = nullptr;
			current_node = nullptr;
		}

	  private:
		Node<T> *node = nullptr;
		Node<T> *current_node = nullptr;

		Allocator<Node<T>> allocator = Allocator<Node<T>>{};

		std::size_t size = 0;
	};
} // namespace type
#endif

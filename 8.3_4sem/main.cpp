#include <iostream>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

template <typename T>
class Safe_Priority_Queue
{
public:

	Safe_Priority_Queue() = default;

	Safe_Priority_Queue(const Safe_Priority_Queue& other)
	{
		std::lock_guard < std::mutex > lock(other.m_mutex);
		pr_queue = other.pr_queue;
	}

	Safe_Priority_Queue& operator=(const Safe_Priority_Queue& other)
	{
		std::scoped_lock lock(m_mutex, other.m_mutex);
		pr_queue = other.pr_queue;
	}

public:

	void push(T value)
	{
		std::lock_guard < std::mutex > lock(m_mutex);
		pr_queue.push(value);
		m_condition_variable.notify_one();
	}

	void wait_and_pop(T& value)
	{
		std::unique_lock < std::mutex > lock(m_mutex);

		m_condition_variable.wait(lock, [this] {return !pr_queue.empty(); });
		value = pr_queue.top();
		pr_queue.pop();
	}

	std::shared_ptr <T> wait_and_pop()
	{
		std::unique_lock < std::mutex > lock(m_mutex);

		m_condition_variable.wait(lock, [this] {return !pr_queue.empty(); });
		auto result = std::make_shared <T>(pr_queue.top());
		pr_queue.pop();

		return result;
	}

	bool try_pop(T& value)
	{
		std::lock_guard <std::mutex> lock(m_mutex);

		if (pr_queue.empty())
		{
			return false;
		}

		value = pr_queue.top();
		pr_queue.pop();

		return true;
	}

	std::shared_ptr <T> try_pop()
	{
		std::lock_guard <std::mutex> lock(m_mutex);

		if (pr_queue.empty())
		{
			return std::shared_ptr <T>();
		}

		auto result = std::make_shared <T>(pr_queue.top());
		pr_queue.pop();

		return result;
	}

	bool empty() const
	{
		std::lock_guard <std::mutex> lock(m_mutex);
		return pr_queue.empty();
	}

private:

	std::priority_queue	<T> pr_queue;
	std::condition_variable m_condition_variable;

private:

	mutable std::mutex m_mutex;
};

int main()
{

	Safe_Priority_Queue<int> priority_queue;

	priority_queue.push(56);

	auto ptr = priority_queue.wait_and_pop();

	std::cout << "We could get " <<*ptr << " out of our priority queue. \n";

	int value;

	bool result = priority_queue.try_pop(value);

	std::cout << "Can we pop something else? " << result << "\n";

	system("pause");
	return 0;
}
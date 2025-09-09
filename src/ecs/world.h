#pragma once

#include <memory>
#include <vector>
#include <type_traits>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <typeindex>
#include <map>
#include <functional>
#include <tuple>

#include "utils/function_traits.h"
#include "utils/remove_smart_ptr.h"
#include "utils/type_map.h"
#include "utils/vec_to_tuple.h"
#include "component.h"
#include "query.h"

class World
{
public:
	World() = default;
	~World() = default;

	template <typename... Args>
	void addEntity(Args... args)
	{
		static_assert(std::conjunction_v<std::is_base_of<Component, Args>...>, "All arguments must be Component");
		auto entity_id = entityCounter++;
		std::apply([this, entity_id](auto &&...args_pack)
			{ (([&]
				{
					using T = std::decay_t<decltype(args_pack)>;
					auto& compVec = components.get<T>();
					compVec.push_back(std::make_shared<T>(args_pack));
					entityToComponentIndex.get<T>()[entity_id] = compVec.size() - 1; }()),
				...); }, std::forward_as_tuple(args...));
	}

	template <typename Func>
	void addSystem(Func&& system)
	{
		using FuncTraits = function_traits<std::decay_t<Func>>;
		addSystemImpl(std::forward<Func>(system),
			std::make_index_sequence<FuncTraits::arity>{});
	}

	void runSystems()
	{
		for (auto system : systems)
		{
			system();
		}
	}

private:
	int entityCounter = 0;
	TypeMap<Component, std::vector<std::shared_ptr<Component>>> components;
	TypeMap<Component, std::unordered_map<int, size_t>> entityToComponentIndex;
	std::vector<std::function<void()>> systems;

	template <typename Func, std::size_t... Is>
	void addSystemImpl(Func&& system, std::index_sequence<Is...>)
	{
		using FuncTraits = function_traits<std::decay_t<Func>>;
		static_assert((std::is_base_of_v<QueryBase,
			typename FuncTraits::template arg<Is>> &&
			...),
			"All arguments must be Queries");

		systems.emplace_back([this, system = std::forward<Func>(system)]() mutable
			{ system(getQuery<typename FuncTraits::template arg<Is>>()...); });
	}

	template <typename Query>
	Query getQuery()
	{
		static_assert(std::is_base_of_v<QueryBase, Query>, "Argument must be a Query");
		using ComponentTuple = typename Query::Types;
		bool first = true;
		std::vector<int> intersection;
		[this, &intersection, &first] <typename... ComponentTypes>(std::tuple<ComponentTypes...> *)
		{
			(([this, &intersection, &first]()
				{
					using Comp = remove_smart_ptr_t<ComponentTypes>;
					auto& entityMap = entityToComponentIndex.get<Comp>();

					if (first) {
						intersection.reserve(entityMap.size());
						for (const auto& p : entityMap)
							intersection.push_back(p.first);
						first = false;
					}
					else {
						std::vector<int> next;
						next.reserve(std::min(intersection.size(), entityMap.size()));
						for (int id : intersection)
						{
							if (entityMap.find(id) != entityMap.end())
								next.push_back(id);
						}
						intersection.swap(next);
					} }()),
				...);
		}(static_cast<ComponentTuple*>(nullptr));

		auto results = std::vector<typename Query::ComponentTuple>();
		for (auto entity : intersection)
		{
			auto componentTuple = [this, &entity]<typename... ComponentTypes>(std::tuple<ComponentTypes...> *)
			{
				return std::make_tuple([this, &entity]() -> std::shared_ptr<remove_smart_ptr_t<ComponentTypes>>
					{
						using Comp = remove_smart_ptr_t<ComponentTypes>;
						auto& componentIndex = entityToComponentIndex.get<Comp>()[entity];
						auto& component = components.get<Comp>()[componentIndex];
						return std::static_pointer_cast<Comp>(component); }()...);
			}(static_cast<ComponentTuple*>(nullptr));
			results.push_back(componentTuple);
		}
		return Query{ results };
	}
};
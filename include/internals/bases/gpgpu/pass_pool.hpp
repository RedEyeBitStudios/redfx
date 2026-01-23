#pragma once
#include <vulkan/vulkan.h>
#include <memory>
#include <string_view>
#include <unordered_map>

namespace nxcraft::intern::subsystems
{
	class GPGPU_PassPool
	{
	public:
		class PassStorage
		{
		public:
			PassStorage() = default;
			virtual ~PassStorage() = default;
		};
	protected:
		std::unordered_map<std::string_view, std::shared_ptr<PassStorage>> pool;
	public:
		PassStorage* getStorage(std::string_view name);
		void alloc(const std::pair<std::string_view, GPGPU_PassPool::PassStorage*>& storage_pair);
	};

	class GPGPU_PassBase
	{
	protected:
		std::string pass_name;
	public:
		GPGPU_PassBase(std::string_view name);
		virtual ~GPGPU_PassBase() = default;

		virtual std::pair<std::string_view, GPGPU_PassPool::PassStorage*> allocateResources() = 0;
	};
}
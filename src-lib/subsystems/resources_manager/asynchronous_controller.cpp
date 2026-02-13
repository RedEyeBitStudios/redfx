#include <internals/subsystems/resources_manager/resources_manager.hpp>
#include <subsystems.hpp>
#include <unordered_set>

using ClassImpl = nxcraft::intern::subsystems::ResourcesManagerRoot;

void ClassImpl::transfer(AsyncTransferUnit& transfer_unit)
{
	transfer_unit.status = AsyncTransferUnitStatus::IDLE;
	transfer_unit.pipe.notify_one();

	while (transfer_unit.status != AsyncTransferUnitStatus::SHUTDOWN)
	{
		std::unique_lock lock(transfer_unit.m);
		transfer_unit.pipe.wait(lock, [&transfer_unit]() -> bool { return transfer_unit.status != AsyncTransferUnitStatus::IDLE; });

		if (transfer_unit.status == AsyncTransferUnitStatus::ACTIVE)
		{
			for (auto& entry : transfer_unit.data)
			{
				std::visit
				(
					[&entry](auto&& ext)
					{
						using T = std::decay_t<decltype(ext)>;
					
						if constexpr(std::is_same_v<T, ResourceManifest::Extensions::Extension_Font>)
						{
							entry->data = std::move(std::make_unique<ResourceData_Font>(*entry->manifest_ptr));
						}
						else if constexpr(std::is_same_v<T, ResourceManifest::Extensions::Extension_RedFXUI>)
						{
							entry->data = std::move(std::make_unique<ResourceData_RedFXUI>(*entry->manifest_ptr));
						}
						else
						{
							static_assert(false, "Unimplemented branch.");
						}
					},
					entry->manifest_ptr->extension
				);
			}
			transfer_unit.status = AsyncTransferUnitStatus::IDLE;
		}
	}
	transfer_unit.status = AsyncTransferUnitStatus::INACTIVE;
	transfer_unit.pipe.notify_all();
}

void ClassImpl::submitQueue()
{
	NXC_LOG_HELPER("Submit asynchronous queue called.");
	
	// Prepare workload.
	std::vector<std::vector<ResourceCache*>> workload;
	uint64_t io_thread_size_summary = 0;	
	std::vector<ResourceCache*> unit_work;

	for (auto& entry : this->queue)
	{
		auto& resource_entry = this->resources[entry];
		const auto file_size = resource_entry.manifest_ptr->general.file_size;

		io_thread_size_summary += file_size;
		unit_work.push_back(&resource_entry);

		if (io_thread_size_summary > 131072 || entry == this->queue.back())
		{
			workload.push_back(std::move(unit_work));			
			io_thread_size_summary = 0;
		}
	}

	// Search for free workers.
	std::unordered_set<AsyncTransferUnit*> free_units;
	while (free_units.size() != workload.size())
	{
		for (auto& t : this->transfer_controller.transfer_units)
		{
			if (free_units.size() == workload.size()) break;
			if (t->status == AsyncTransferUnitStatus::IDLE)
			{
				free_units.insert(t.get());
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	// Dispatch.
	auto workload_iterator = workload.begin();
	uint32_t workload_index = 0;
	for (auto& unit : free_units)
	{
		unit->data = std::move(workload[workload_index]);
		unit->status = AsyncTransferUnitStatus::ACTIVE;
		unit->pipe.notify_one();
		NXC_LOG_HELPER(std::format("Asynchronous transfer submitted: {}.", unit->thread_id));
		workload_index++;
	}
	this->queue.clear();
}

void ClassImpl::waitForTransfers()
{
	NXC_LOG_HELPER("Waiting for all transfer units to finish...");
	for (auto& t_unit : this->transfer_controller.transfer_units)
	{
		if (t_unit->status != AsyncTransferUnitStatus::IDLE)
		{
			while (t_unit->status != AsyncTransferUnitStatus::IDLE)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
		}
	}
}
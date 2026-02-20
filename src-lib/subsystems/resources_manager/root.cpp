#include <internals/subsystems/resources_manager/resources_manager.hpp>
#include <subsystems.hpp>
#include <tinyxml2.h>
#include <filesystem>
#include <regex>
#include <cassert>
#include <mutex>
#include <algorithm>

using ClassImpl = nxcraft::intern::subsystems::ResourcesManagerRoot;

ClassImpl::ResourcesManagerRoot()
{
	nxcraft::Subsystems::getSubsystem_Logger().registerHeader(this, "SubsystemResourcesManager");

	NXC_LOG_HELPER("Initialization started.");
	this->transfer_controller.transfer_units.resize(std::thread::hardware_concurrency() * 2);
	uint32_t thread_id = 0;
	for (auto& t_unit : this->transfer_controller.transfer_units)
	{
		t_unit = std::make_unique<AsyncTransferUnit>();
		t_unit->thread_handle = std::thread(&ClassImpl::transfer, std::ref(*t_unit.get()));
		t_unit->thread_id = thread_id;
		t_unit->thread_handle.detach();
		thread_id++;

		while (t_unit->status != AsyncTransferUnitStatus::IDLE)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	}
	NXC_LOG_HELPER(std::format("Transfer units created: {}", std::thread::hardware_concurrency() * 2));
	this->refresh();

	for (auto& res : this->resources)
	{
		std::visit
		(
			[this, &res](auto&& ext)
			{
				using T = std::decay_t<decltype(ext)>;

				if constexpr (std::is_same_v<T, ResourceManifest::Extensions::Extension_RedFXUI>)
				{
					this->appendAsynchronousQueue(res.first);
				}
			},
			res.second.manifest_ptr->extension
		);
	}

	this->submitQueue();
	this->waitForTransfers();
}
ClassImpl::~ResourcesManagerRoot()
{
	this->waitForTransfers();
	for (auto& t_unit : this->transfer_controller.transfer_units)
	{
		t_unit->status = AsyncTransferUnitStatus::SHUTDOWN;
		t_unit->pipe.notify_one();

		while (t_unit->status != AsyncTransferUnitStatus::INACTIVE)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	}
	NXC_LOG_HELPER(std::format("Transfer units exited: {}", this->transfer_controller.transfer_units.size()));
}
void ClassImpl::searchForManifests()
{
	NXC_LOG_HELPER("Scanning for manifests...");

	for (const auto& f : std::filesystem::directory_iterator("assets"))
	{
		if (f.is_regular_file())
		{
			const std::regex r("(\\S+)\\.manifest\\.xml");
			std::smatch m;

			const auto manifest_filename = f.path().filename().generic_string();

			if (std::regex_match(manifest_filename, m, r))
			{
				NXC_LOG_HELPER(std::format("Manifest: {}", manifest_filename));
				const std::string manifest_name = m[1].str();
				
				tinyxml2::XMLDocument doc;
				doc.LoadFile(f.path().generic_string().c_str());

				const auto root = doc.FirstChildElement();
				const auto general_header = root->FirstChildElement("general");
				const auto extension_header = root->FirstChildElement("extension");

				// Firstly read general section.
				ResourceManifest manifest
				{
					.general
					{
						.path = general_header->FirstChildElement("path")->GetText(),
						.file_size = general_header->FirstChildElement("size")->UnsignedText()
					}
				};

				std::string format;
				if (auto fmt = general_header->FirstChildElement("format"); fmt != nullptr)
				{
					format = general_header->FirstChildElement("format")->GetText();
				}
				
				// Determine what kind of manifest extension is.
				std::unordered_map<std::string_view, AssetType> type_pairs
				{
					{ "REDFX_FONT", AssetType::REDFX_FONT },
					{ "REDFX_UI", AssetType::REDFX_UI }
				};

				if (type_pairs.contains(format))
				{
					const AssetType type = type_pairs[format];

					if (type == AssetType::REDFX_FONT)
					{
						manifest.extension = ResourceManifest::Extensions::Extension_Font
						{
							.name = extension_header->FirstChildElement("name")->GetText()
						};
					}
					else if (type == AssetType::REDFX_UI)
					{
						manifest.extension = ResourceManifest::Extensions::Extension_RedFXUI
						{
							.active_on_init = extension_header->FirstChildElement("active_on_init")->BoolText()
						};
					}
					else
					{
						assert(false); // Unimplemented asset type handling.
					}

					if (this->validateManifest(std::move(manifest)))
					{
						this->manifests[type].push_back(manifest);
					}
					else
					{
						// TODO: Handle this error.
						NXC_LOG_HELPER(std::format("Invalid manifest: {}.", f.path().generic_string()), nxcraft::Subsystems::LogRoot::Message::MARK_AS_CRITICAL_ERROR | nxcraft::Subsystems::LogRoot::Message::SHOW_MESSAGE_BOX);
					}
				}
				else
				{
					// TODO: Handle this error.
					NXC_LOG_HELPER(std::format("Invalid asset type/format in manifest: {}.", f.path().generic_string()), nxcraft::Subsystems::LogRoot::Message::MARK_AS_CRITICAL_ERROR | nxcraft::Subsystems::LogRoot::Message::SHOW_MESSAGE_BOX);
				}
			}
		}
	}
}
bool ClassImpl::validateManifestSection(std::vector<uint64_t>&& parameters, const bool validation_result)
{
	for (auto& param : parameters)
	{
		if (param == 0)
		{
			return false;
		}
	}
	return validation_result;
}
bool ClassImpl::validateManifest(const ResourceManifest& m)
{
	bool validation_result = true;

	// Validate general section.
	validation_result = this->validateManifestSection
	(
		std::vector<uint64_t>
		{
			m.general.file_size,
			!m.general.path.empty()
		},
		validation_result
	);

	// Validate extension section.
	std::visit
	(
		[&validation_result, this](auto&& extension)
		{
			using T = std::decay_t<decltype(extension)>;
			using Ext = ResourceManifest::Extensions;
			std::vector<uint64_t> validation_values{};

			if constexpr (std::is_same_v<T, Ext::Extension_Font>)
			{
				validation_values = 
				{
					extension.name.length()
				};
			}
			else if constexpr (std::is_same_v<T, Ext::Extension_RedFXUI>)
			{}
			else
			{
				static_assert(false, "Unimplemented branch.");
			}

			validation_result = this->validateManifestSection(std::move(validation_values), validation_result);
		},
		m.extension
	);

	return validation_result;
}
void ClassImpl::preCacheAssets()
{
	NXC_LOG_HELPER("Prepare resource table...");
	constexpr const char* fmt = "Setup completed: '{}'.";

	for (auto& manifest : this->manifests[AssetType::REDFX_FONT])
	{
		const auto& ext = std::get<ResourceManifest::Extensions::Extension_Font>(manifest.extension);
		this->resources[ext.name] = ResourceCache
		{
			.manifest_ptr = &manifest,
			.data = nullptr
		};
		NXC_LOG_HELPER(std::format(fmt, ext.name));
	}
	for (auto& manifest : this->manifests[AssetType::REDFX_UI])
	{
		const auto name = manifest.general.path.filename().replace_extension("").generic_string();
		this->resources[name] = ResourceCache
		{
			.manifest_ptr = &manifest,
			.data = nullptr
		};
		NXC_LOG_HELPER(std::format(fmt, name));
	}
}
void ClassImpl::refresh()
{
	NXC_LOG_HELPER("Refreshing resources list requested...");
	this->searchForManifests();
	this->preCacheAssets();
}
void ClassImpl::appendAsynchronousQueue(std::string_view name)
{
	if 
	(
		this->resources.contains(name.data()) && 
		this->resources[name.data()].data == nullptr && 
		[this, &name]() -> bool
		{
			for (auto& unit : this->transfer_controller.transfer_units)
			{
				if (std::ranges::contains(unit->data, &this->resources[name.data()]))
				{
					return false;
				}
			}
			return true;
		}()
	)
	{
		NXC_LOG_HELPER(std::format("Asset '{}' appended into queue.", name));
		this->queue.push_back(name.data());
	}
}
bool ClassImpl::checkAssetIsLoaded(std::string_view name)
{
	return this->resources[name.data()].data != nullptr;
}
const ClassImpl::ResourceCache* ClassImpl::retrieveResourceView(std::string_view name)
{
	return &this->resources[name.data()];
}
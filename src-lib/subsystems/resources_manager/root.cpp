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

				if constexpr (std::is_same_v<T, ResourceManifestClasses::Manifest_RedFXUI>)
				{
					this->appendAsynchronousQueue(res.first);
				}
			},
			res.second.manifest_ptr->manifest
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
ClassImpl::ResourceManifest readManifestXML(const std::filesystem::path& path, const ClassImpl::AssetType type, ClassImpl::ResourceManifest&& manifest)
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile(path.c_str());

	const auto root = doc.FirstChildElement();

	ClassImpl::ResourceManifestClasses::ManifestHeader header
	{
		.fmt_version = static_cast<uint16_t>(root->FirstChildElement("fmt_v")->UnsignedText(0)),
		.title = root->FirstChildElement("title")->GetText()
	};

	if (type == ClassImpl::AssetType::REDFX_FONT)
	{
		manifest.manifest = ClassImpl::ResourceManifestClasses::Manifest_RedFXFont
		{
			{ std::move(header) }
		};
		manifest.implicits.asset_path = path.relative_path().replace_extension(".bin");
	}
	else if (type == ClassImpl::AssetType::REDFX_UI)
	{
		ClassImpl::ResourceManifestClasses::Manifest_RedFXUI meta_cache
		{
			{ std::move(header) }
		};
		meta_cache.init = root->FirstChildElement("init")->BoolText();
		manifest.implicits.asset_path = path.relative_path().replace_extension(".bin");
		manifest.manifest = meta_cache;
	}
	else if (type == ClassImpl::AssetType::REDFX_LANG)
	{
		manifest.implicits.asset_path = path.relative_path();
	}
	else if (type == ClassImpl::AssetType::TEXTURE)
	{
		ClassImpl::ResourceManifestClasses::Manifest_Texture meta_cache
		{
			{ std::move(header) }
		};
		manifest.implicits.asset_path = path.relative_path().replace_extension(".tga");
		//manifest.manifest = meta_cache;
	}
	else
	{
		assert(false && "Unimplemented branch.");
	}

	assert(std::filesystem::exists(manifest.implicits.asset_path) && "File must exist.");
	manifest.implicits.file_size = std::filesystem::file_size(manifest.implicits.asset_path);
	return manifest;
}
void ClassImpl::searchForManifests()
{
	NXC_LOG_HELPER("Scanning for manifests...");
	std::unordered_map<std::filesystem::path, AssetType> directories
	{
		{ "assets/fonts", AssetType::REDFX_FONT },
		{ "assets/langs", AssetType::REDFX_LANG },
		{ "assets/textures", AssetType::TEXTURE },
		{ "assets/ui", AssetType::REDFX_UI }
	};

	for (auto dir : directories)
	{
		const auto resource_type = dir.second;
		for (const auto f : std::filesystem::directory_iterator(dir.first))
		{
			if (f.is_regular_file() && std::string_view(f.path().extension().generic_string()) == std::string_view(".xml"))
			{
				const auto relative_path = f.path().relative_path().generic_string();
				NXC_LOG_HELPER(std::format("Found: {}", relative_path));

				this->manifests[resource_type].push_back(readManifestXML(relative_path, resource_type, ClassImpl::ResourceManifest()));
			}
		}
	}

	/*

	for (const auto& f : std::filesystem::directory_iterator("assets"))
	{
		if (f.is_regular_file())
		{
			const std::regex r("(\\S+)\\.manifest\\.xml");
			std::smatch m;

			const auto manifest_filename = f.path().filename().generic_string();

			if (std::regex_match(manifest_filename, m, r))
			{
				
				const std::string manifest_name = m[1].str();
				
				

				const auto root = doc.FirstChildElement();
				
				const auto extension_header = root->FirstChildElement("extension");

				// Firstly read general section.
				ResourceManifest manifest
				{
					.general
					{
						.path = general_header->FirstChildElement("path")->GetText(),
						.file_size = std::filesystem::file_size(general_header->FirstChildElement("path")->GetText())
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
					{ "REDFX_UI", AssetType::REDFX_UI },
					{ "IMAGE_TGA", AssetType::IMAGE_TGA }
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
		*/
}
void ClassImpl::preCacheAssets()
{
	NXC_LOG_HELPER("Prepare resource table...");
	constexpr const char* fmt = "Setup completed: '{}'.";

	for (auto& manifest_group : std::views::values(this->manifests))
	{
		for (auto& manifest : manifest_group)
		{
			std::visit
			(
				[this, &manifest, &fmt](auto&& metadata)
				{
					using T = std::decay_t<decltype(metadata)>;

					if constexpr(std::is_base_of_v<ResourceManifestClasses::ManifestHeader, T>)
					{
						printf("METADATA: %s\n", metadata.title.c_str());
						this->resources[metadata.title] = ResourceCache
						{
							.manifest_ptr = &manifest,
							.data = nullptr
						};
						NXC_LOG_HELPER(std::format(fmt, metadata.title));
					}
				},
				manifest.manifest
			);
		}
	}

	/*
	for (auto& manifest : this->manifests[AssetType::REDFX_FONT])
	{
		const auto& meta = std::get<>(manifest.manifest);
		this->resources[meta.title] = ResourceCache
		{
			.manifest_ptr = &manifest,
			.data = nullptr
		};
		NXC_LOG_HELPER(std::format(fmt, meta.title));
	}
	for (auto& manifest : this->manifests[AssetType::REDFX_UI])
	{
		const auto& meta = std::get<ResourceManifestClasses::Manifest_RedFXUI>(manifest.manifest);
		this->resources[meta.title] = ResourceCache
		{
			.manifest_ptr = &manifest,
			.data = nullptr
		};
		NXC_LOG_HELPER(std::format(fmt, meta.title));
	}
		*/
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
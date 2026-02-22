#pragma once
#include "../../bases/subsystem.hpp"
#include <unordered_map>
#include <string>
#include <nx-utils/math/vec2.hpp>
#include <vector>
#include <span>
#include <variant>
#include <thread>
#include "../../../ui_class.hpp"
#include <atomic>
#include <memory>
#include <filesystem>
#include <condition_variable>

namespace nxcraft::intern::subsystems
{
	class ResourcesManagerRoot final : public SubsystemBase
	{
	public:
		enum class AssetType
		{
			TEXTURE,
			//IMAGE_KTX_BPTC,
			//IMAGE_KTX_ASTC_4X4,
			//IMAGE_KTX_ASTC_8X8,
			//IMAGE_KTX_ASTC_12X12,
			REDFX_UI,
			REDFX_LANG,
			REDFX_FONT
		};

		class ResourceData
		{
		public:
			ResourceData() = default;
			virtual ~ResourceData() = default;
		};

		struct ResourceManifest;
	public:
		class ResourceData_Font : public ResourceData
		{
		public:
			struct CharacterData
			{
				std::vector<nexora_utils::math::f16vec2> vertices;
				std::float16_t width;
			};
		private:
			mutable std::unordered_map<uint32_t, CharacterData> characters;
			std::string font_name;
		public:
			ResourceData_Font(const ResourceManifest& manifest);
			virtual ~ResourceData_Font() = default;
			[[nodiscard]] const CharacterData* operator[](const uint32_t utf32_code) const;
			[[nodiscard]] std::string_view getFontName() const;
			const std::unordered_map<uint32_t, CharacterData>& getCharacters() const;
		};

		class ResourceData_RedFXUI : public ResourceData
		{
		private:
			uint8_t base_layer_id = 0;

			mutable std::unordered_map<std::string, UI::ColorBox> boxes;
			mutable std::unordered_map<std::string, UI::TextBox> text_boxes;
		public:
			ResourceData_RedFXUI(const ResourceManifest& manifest);

			void fill(UI& ui) const;
		};

		struct ResourceManifestClasses
		{
			struct ManifestHeader
			{
				uint16_t fmt_version;
				std::string title;
			};

			struct Manifest_RedFXFont : public ManifestHeader
			{

			};
			struct Manifest_RedFXUI : public ManifestHeader
			{
				bool init;
			};
			struct Manifest_RedFXLang : public ManifestHeader
			{
				struct Entry
				{
					struct Address
					{
						std::string page;
						std::string box_name;
					} address;
					std::string text;
				};

				std::vector<Entry> content;
			};
			struct Manifest_Texture : public ManifestHeader
			{
			};
		};

		struct ResourceManifest
		{
			using AssetManifest = std::variant
			<
				ResourceManifestClasses::Manifest_RedFXFont,
				//ResourceManifestClasses::Manifest_RedFXLang,
				ResourceManifestClasses::Manifest_RedFXUI
				//ResourceManifestClasses::Manifest_Texture
			>;
			struct
			{
				std::filesystem::path asset_path;
				uint64_t file_size;
			} implicits;
			AssetManifest manifest;
		};

		struct ResourceCache
		{
			ResourceManifest* manifest_ptr;
			std::unique_ptr<ResourceData> data;
		};
		
	private:

		std::unordered_map<AssetType, std::vector<ResourceManifest>> manifests;
		std::unordered_map<std::string, ResourceCache> resources;
		void searchForManifests();
		void preCacheAssets();

		void loadFont(const ResourceManifest& manifest);

		std::vector<std::string> queue;

		enum class AsyncTransferUnitStatus
		{
			IDLE,
			ACTIVE,
			SHUTDOWN,
			INACTIVE
		};

		struct AsyncTransferUnit
		{
			std::thread thread_handle;
			uint32_t thread_id;
			std::mutex m;
			std::condition_variable pipe;
			AsyncTransferUnitStatus status = AsyncTransferUnitStatus::INACTIVE;
			std::vector<ResourceCache*> data;
		};		

		struct
		{
			std::vector<std::unique_ptr<AsyncTransferUnit>> transfer_units;
		} transfer_controller;

		static void transfer(AsyncTransferUnit&);
		void waitForTransfers();
	public:
		ResourcesManagerRoot();
		~ResourcesManagerRoot();

		void refresh();

		[[nodiscard]] const ResourceCache* retrieveResourceView(std::string_view name);
		void appendAsynchronousQueue(std::string_view name);
		void submitQueue();
		[[nodiscard]] bool checkAssetIsLoaded(std::string_view name);
	};
}
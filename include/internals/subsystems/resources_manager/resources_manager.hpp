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
			FONT_TTF,
			IMAGE_PNG,
			IMAGE_KTX,
			REDFX_UI
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
				nexora_utils::math::ui8vec2 size_px;
				nexora_utils::math::i8vec2 bearing_px;
				uint16_t offset_px;
				std::vector<uint8_t> pixels;
			};
		private:
			mutable std::unordered_map<uint32_t, CharacterData> characters;
			std::string font_name;
		public:
			ResourceData_Font(const ResourceManifest& manifest);
			virtual ~ResourceData_Font() = default;
			[[nodiscard]] const CharacterData* operator[](const uint32_t utf8_code) const;
			[[nodiscard]] std::string_view getFontName() const;
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

		struct ResourceManifest
		{
			class Extensions
			{
			public:
				struct Extension_Font
				{
					std::string name;
				};
				struct Extension_RedFXUI
				{
					bool active_on_init;
				};
			};
			
			struct General
			{
				std::filesystem::path path;
				uint64_t file_size;
			} general;

			using Extension = std::variant
			<
				Extensions::Extension_Font,
				Extensions::Extension_RedFXUI
			>;
			Extension extension;
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
		[[nodiscard]] bool validateManifest(const ResourceManifest& m);
		[[nodiscard]] bool validateManifestSection(std::vector<uint64_t>&& parameters, const bool validation_result);
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
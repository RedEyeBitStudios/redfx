#include <internals/bases/gpgpu/root_base.hpp>

using ClassImpl = nxcraft::intern::subsystems::GPGPU_RootBase;

const std::optional<ClassImpl::TextureFormatCompression> ClassImpl::getCompressionFormat() const
{
	return this->compression_format;
}
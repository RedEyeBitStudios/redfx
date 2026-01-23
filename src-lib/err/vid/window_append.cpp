#include <internals/bases/err.hpp>
#include <format>

using ClassImpl = nxcraft::err::Vid_WindowAppend;

ClassImpl::Vid_WindowAppend() : ErrorBase("VidRoot-VidWindows-append-info-001: handle, surf_handle and accel_internals members must be nullptr.")
{

}
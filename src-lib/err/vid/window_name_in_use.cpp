#include <internals/bases/err.hpp>
#include <format>

using ClassImpl = nxcraft::err::Vid_WindowNameInUse;

ClassImpl::Vid_WindowNameInUse() : ErrorBase("VidRoot-VidWindows-append-wnd_name-001: wnd_name must be unique name, which does not exist in this->wnd_pool.")
{

}
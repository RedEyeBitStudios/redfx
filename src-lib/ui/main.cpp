#include "ui_processor.hpp"

using ClassImpl = nxcraft::intern::ProcessorUI;

void ClassImpl::process()
{
	// TODO: Limit processing frequency only into max display refresh frequency.
	ClassImpl::analyzeWindows();
}
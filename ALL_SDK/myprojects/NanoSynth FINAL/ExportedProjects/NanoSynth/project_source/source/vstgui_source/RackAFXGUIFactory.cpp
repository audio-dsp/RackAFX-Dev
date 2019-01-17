#include "plugin.h"

// --- NOTE: the GUI Factory is NOT used in Make VST or Make AU Projects! 
//           So, this factory only returns NULL;
void* CRafxViewFactory::createGUI(VSTGUI_VIEW_INFO* info, CPlugIn* pPlugIn)
{
	return NULL;
}

void* CRafxViewFactory::destroyGUI()
{
	return NULL;
}

void* CRafxViewFactory::timerPing()
{
	return NULL;
}

void* CRafxViewFactory::initControls()
{
	return NULL;
}

void* CRafxViewFactory::syncGUI()
{
	return NULL;
}


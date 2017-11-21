#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <wx/wx.h>
#include <wx/app.h>
#include <wx/log.h>
#include <wx/filename.h>
#include <wx/imaglist.h>
#include <wx/intl.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/url.h>

#include "testlistctrl.h"

class TestApp : public wxApp
{
	public:
		virtual bool OnInit();
		virtual int OnExit();
	private:
		wxFrame* m_frame;
		TestListCtrl*	m_tlist ;

};

DECLARE_APP(TestApp)

IMPLEMENT_APP(TestApp)


static wxLog* oldlog;
bool TestApp::OnInit()
{

	oldlog = wxLog::SetActiveTarget(new wxLogStderr());
	m_frame = new wxFrame(nullptr, wxID_ANY, _T("TestApp"), wxDefaultPosition, wxDefaultSize);
	m_tlist = new TestListCtrl(m_frame, wxID_ANY);


  	m_frame->Show(TRUE);
	SetTopWindow(m_frame);
  	SetExitOnFrameDelete(true);

  	return TRUE;
}


int TestApp::OnExit() 
{
	
	//wxLogDebug(_T("TestApp OnExit"));

	//delete m_tlist;
	//delete wxLog::SetActiveTarget(oldlog);
	 
	
	return 0;
}










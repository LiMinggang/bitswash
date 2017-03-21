#include "torrentproperty.h"

//(*InternalHeaders(TorrentProperty)
#include <wx/settings.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "mainframe.h"
#include "torrentsetting.h"

#include "functions.h"

namespace lt = libtorrent;

//(*IdInit(TorrentProperty)
const long TorrentProperty::ID_STATICTEXT1 = wxNewId();
const long TorrentProperty::ID_PANEL1 = wxNewId();
const long TorrentProperty::ID_STATICTEXT2 = wxNewId();
const long TorrentProperty::ID_STATICTEXT3 = wxNewId();
const long TorrentProperty::ID_STATICTEXT4 = wxNewId();
const long TorrentProperty::ID_TEXTCTRL1 = wxNewId();
const long TorrentProperty::ID_STATICTEXT5 = wxNewId();
const long TorrentProperty::ID_STATICTEXT6 = wxNewId();
const long TorrentProperty::ID_STATICTEXT7 = wxNewId();
const long TorrentProperty::ID_STATICTEXT8 = wxNewId();
const long TorrentProperty::ID_NOTEBOOK1 = wxNewId();
const long TorrentProperty::ID_BUTTONDOWNLOADNONE = wxNewId();
const long TorrentProperty::ID_BUTTONDOWNLOADALL = wxNewId();
//*)

BEGIN_EVENT_TABLE(TorrentProperty,wxDialog)
	//(*EventTable(TorrentProperty)
	//*)
END_EVENT_TABLE()

TorrentProperty::TorrentProperty(shared_ptr<torrent_t>& pTorrent, wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	wxASSERT( pTorrent );

	m_pMainFrame = (wxFrame*)parent;

	m_pTorrent = pTorrent;
	//(*Initialize(TorrentProperty)
	wxButton* ButtonSelectAll;
	wxBoxSizer* m_pane_boxsizer;
	wxButton* ButtonSelectNone;
	wxFlexGridSizer* fgSizer11;
	wxButton* ButtonOK;
	wxButton* ButtonCancel;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* m_torrentsettings_sizer;
	int w,h;

	Create(parent, wxID_ANY, _("Torrent Setting"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	SetClientSize(wxDefaultSize);
	GetClientSize(&w, &h);
	m_torrentsettings_sizer = new wxFlexGridSizer(0, 1, 3, 3);
	m_pane_info = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	m_pane_info->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	m_pane_boxsizer = new wxBoxSizer(wxVERTICAL);
	m_pane_label_info = new wxStaticText(m_pane_info, ID_STATICTEXT1, _("Torrent Info"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	m_pane_boxsizer->Add(m_pane_label_info, 1, wxALL|wxEXPAND, 5);
	m_pane_info->SetSizer(m_pane_boxsizer);
	m_pane_boxsizer->Fit(m_pane_info);
	m_pane_boxsizer->SetSizeHints(m_pane_info);
	m_torrentsettings_sizer->Add(m_pane_info, 1, wxALL|wxEXPAND, 5);
	fgSizer11 = new wxFlexGridSizer(0, 2, 2, 2);
	m_label_name = new wxStaticText(this, ID_STATICTEXT2, _("Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	fgSizer11->Add(m_label_name, 0, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	lt::torrent_info const& torrent_info = *(m_pTorrent->info);
 	wxString t_name = wxString(wxConvUTF8.cMB2WC(torrent_info.name().c_str()));
	m_label_torrentname = new wxStaticText(this, ID_STATICTEXT3, t_name, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	fgSizer11->Add(m_label_torrentname, 0, wxALL|wxEXPAND, 5);
	m_label_comment = new wxStaticText(this, ID_STATICTEXT4, _("Comment:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	fgSizer11->Add(m_label_comment, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	wxString t_comment = wxString(wxConvUTF8.cMB2WC(torrent_info.comment().c_str()));

	int cw, ch;
	m_label_comment->GetClientSize(&cw, &ch);
	m_text_torrentcomment = new wxTextCtrl(this, ID_TEXTCTRL1, t_comment, wxDefaultPosition, wxSize(w-50 - cw , 80), 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	fgSizer11->Add(m_text_torrentcomment, 1, wxALL|wxEXPAND, 5);
	m_label_size = new wxStaticText(this, ID_STATICTEXT5, _("Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	fgSizer11->Add(m_label_size, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	
	wxString t_size = HumanReadableByte((wxDouble)torrent_info.total_size());
	m_label_torrentsize = new wxStaticText(this, ID_STATICTEXT6, t_size, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	fgSizer11->Add(m_label_torrentsize, 1, wxALL|wxEXPAND, 5);
	m_label_freespace = new wxStaticText(this, ID_STATICTEXT7, _("Disk Free Space:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	fgSizer11->Add(m_label_freespace, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	
	m_label_diskfreespace = new wxStaticText(this, ID_STATICTEXT8, _("0 Byte(s)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	fgSizer11->Add(m_label_diskfreespace, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	m_torrentsettings_sizer->Add(fgSizer11, 0, wxALL|wxEXPAND, 5);
	m_notebook_property = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	m_notebook_property->SetMinSize( wxSize( 600,-1 ) );
	
	m_filespane= new FileListCtrl( m_notebook_property, wxEmptyString, wxID_ANY );

	m_filespane->SetStaticHandle(pTorrent); 
	m_filespane->SetItemCount(pTorrent->info->num_files()); 
	
	m_filespane->UpdateSwashList();
	m_notebook_property->AddPage( m_filespane, _("Files"), false );

	m_panel_settings = new TorrentSettingPane( m_notebook_property, pTorrent);

	m_notebook_property->AddPage( m_panel_settings, _("Settings"), false );
	
	m_notebook_property->SetSelection(0);
	m_torrentsettings_sizer->Add( m_notebook_property, 1, wxEXPAND | wxALL, 5 );

	if(m_panel_settings->GetDownloadPath() != wxEmptyString)
	{
		wxFileName fdir;
		fdir.AssignDir(m_panel_settings->GetDownloadPath()); // make sure we got path seperator at the end
		fdir.MakeAbsolute();
		size_t dcount = fdir.GetDirCount( );
		while(dcount-- > 0)
		{
			if(!fdir.DirExists()) fdir.RemoveLastDir();
			else break;
		}
		if(fdir.DirExists())
		{
			wxLongLong free(0);  
			if(wxGetDiskSpace( fdir.GetPathWithSep(), NULL, &free))
			{
				m_label_diskfreespace->SetLabelText(HumanReadableByte(wxDouble(free.ToDouble())));
			}
		}
	}

	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	ButtonSelectNone = new wxButton(this, ID_BUTTONDOWNLOADNONE, _("Select None"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONSELECTNONE"));
	BoxSizer1->Add(ButtonSelectNone, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonSelectAll = new wxButton(this, ID_BUTTONDOWNLOADALL, _("Select All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONSELECTALL"));
	BoxSizer1->Add(ButtonSelectAll, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonOK = new wxButton(this, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_OK"));
	ButtonOK->SetDefault();
	BoxSizer1->Add(ButtonOK, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonCancel = new wxButton(this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_CANCEL"));
	BoxSizer1->Add(ButtonCancel, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_torrentsettings_sizer->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	m_torrentsettings_sizer->Fit(this);
	m_torrentsettings_sizer->SetSizeHints(this);
	SetSizer(m_torrentsettings_sizer);
	Layout();
	Center();

	Bind( wxEVT_SET_FOCUS,  &TorrentProperty::OnFocus, this );
	//*)

	Bind( wxEVT_COMMAND_BUTTON_CLICKED,  &TorrentProperty::OnOK, this, wxID_OK );
	Bind( wxEVT_COMMAND_BUTTON_CLICKED,  &TorrentProperty::OnCancel, this, wxID_CANCEL );
	Bind( wxEVT_COMMAND_BUTTON_CLICKED,  &TorrentProperty::OnDownloadNone, this, ID_BUTTONDOWNLOADNONE );
	Bind( wxEVT_COMMAND_BUTTON_CLICKED,  &TorrentProperty::OnDownloadAll, this, ID_BUTTONDOWNLOADALL );
}

TorrentProperty::~TorrentProperty()
{
	//(*Destroy(TorrentProperty)
	//*)
}


void TorrentProperty::OnOK(wxCommandEvent& event)
{
	wxLogDebug(_T("OnOk"));

	shared_ptr<TorrentConfig> pConfig = m_pTorrent->config;
	Configuration *pMainConfig = wxGetApp().GetConfig();

	wxFileName fdir;
	fdir.AssignDir(m_panel_settings->GetDownloadPath()); // make sure we got path seperator at the end
	pConfig->SetDownloadPath(fdir.GetPathWithSep());

	//pConfig->SetTorrentCompactAlloc(m_panel_settings->GetCompactAlloc());
	pConfig->SetTorrentStorageMode(m_panel_settings->GetStorageMode());
	

	pConfig->SetTorrentState(m_panel_settings->GetStartTorrent()?TORRENT_STATE_QUEUE:TORRENT_STATE_STOP);

	pMainConfig->SetUseDefault(m_panel_settings->GetUseDefault());

	pConfig->SetTorrentDownloadLimit(m_panel_settings->GetDownloadRate());
	pConfig->SetTorrentUploadLimit(m_panel_settings->GetUploadRate());
	pConfig->SetTorrentMaxConnections(m_panel_settings->GetMaxConnections());
	pConfig->SetTorrentMaxUploads(m_panel_settings->GetMaxUpload());

	pConfig->SetTorrentRatio(m_panel_settings->GetRatio());

	pConfig->Save();
	EndModal(wxID_OK);
	event.Skip();

}

void TorrentProperty::OnCancel(wxCommandEvent& event)
{
	EndModal (wxID_CANCEL);
	event.Skip();
}

void TorrentProperty::OnFocus(wxFocusEvent& event)
{
	wxLogDebug(_T("TorrentProperty OnFocus %d"), m_notebook_property->GetSelection());
	if(m_notebook_property->GetSelection() == 0)
	{
		m_filespane->UpdateSwashList();
	}
}

void TorrentProperty::OnDownloadAll(wxCommandEvent& event)
{
	shared_ptr<TorrentConfig> pConfig = m_pTorrent->config;
	std::vector<int> filespriority(m_pTorrent->info->num_files(), BITTORRENT_FILE_NORMAL);
	pConfig->SetFilesPriority(filespriority);
	m_filespane->UpdateSwashList();
	m_filespane->Refresh(false);
}

void TorrentProperty::OnDownloadNone(wxCommandEvent& event)
{
	shared_ptr<TorrentConfig> pConfig = m_pTorrent->config;
	std::vector<int> filespriority(m_pTorrent->info->num_files(), BITTORRENT_FILE_NONE);
	pConfig->SetFilesPriority(filespriority);
	m_filespane->UpdateSwashList();
	m_filespane->Refresh(false);
}


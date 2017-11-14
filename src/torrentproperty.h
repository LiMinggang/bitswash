#ifndef TORRENTPROPERTY_H
#define TORRENTPROPERTY_H

//(*Headers(TorrentProperty)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include "bittorrentsession.h"
#include "mainframe.h"
#include "torrentconfig.h"
#include "torrentsetting.h"
#include "filelistctrl.h"

class TorrentProperty: public wxDialog
{
public:

	TorrentProperty(std::shared_ptr<torrent_t>& pTorrent, wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
	virtual ~TorrentProperty();

	//(*Declarations(TorrentProperty)
	wxStaticText* m_label_comment;
	wxStaticText* m_label_diskfreespace;
	wxPanel* m_pane_info;
	wxPanel* Panel1;
	wxStaticText* m_label_torrentsize;
	wxStaticText* m_label_freespace;
	wxStaticText* m_label_torrentname;
	wxNotebook* m_notebook_property;
	wxStaticText* m_label_size;
	wxPanel* Panel2;
	wxStaticText* m_label_name;
	wxStaticText* m_pane_label_info;
	wxTextCtrl* m_text_torrentcomment;
	//*)
	TorrentSettingPane* m_panel_settings;
	FileListCtrl* m_filespane;
protected:

	//(*Identifiers(TorrentProperty)
	//*)

private:

	//(*Handlers(TorrentProperty)
	void OnFocus(wxFocusEvent& event);
	//*)
	void OnOK(wxCommandEvent& event);
	void OnCancel(wxCommandEvent& event);
	void OnFileButton(wxCommandEvent& event);
    void OnDownloadAll(wxCommandEvent& event);
    void OnDownloadNone(wxCommandEvent& event);

	wxFrame* m_pMainFrame;
	std::shared_ptr<torrent_t> m_pTorrent;
};

#endif

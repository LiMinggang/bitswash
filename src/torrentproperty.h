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

		TorrentProperty(torrent_t* pTorrent, wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~TorrentProperty();

		//(*Declarations(TorrentProperty)
		wxStaticText* m_label_comment;
		wxPanel* m_pane_info;
		wxPanel* Panel1;
		wxStaticText* m_label_torrentsize;
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
		static const long ID_STATICTEXT1;
		static const long ID_PANEL1;
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT6;
		static const long ID_NOTEBOOK1;
		static const long ID_BUTTONDOWNLOADNONE;
		static const long ID_BUTTONDOWNLOADALL;
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
		torrent_t *m_pTorrent;

		DECLARE_EVENT_TABLE()
};

#endif

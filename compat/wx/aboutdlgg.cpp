///////////////////////////////////////////////////////////////////////////////
// Name:        src/generic/aboutdlgg.cpp
// Purpose:     implements wxGenericAboutBox() function
// Author:      Vadim Zeitlin
// Created:     2006-10-08
// RCS-ID:      $Id: aboutdlgg.cpp,v 1.14 2006/11/12 21:43:27 VZ Exp $
// Copyright:   (c) 2006 Vadim Zeitlin <vadim@wxwindows.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// for compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_ABOUTDLG

#ifndef WX_PRECOMP
    #include "wx/sizer.h"
    #include "wx/statbmp.h"
    #include "wx/stattext.h"
    #include "wx/textctrl.h"
    #include "wx/button.h"
#endif //WX_PRECOMP

#include "wx/aboutdlg.h"
#include "wx/generic/aboutdlgg.h"

#if wxUSE_HYPERLINKCTRL
#include "wx/hyperlink.h"
#endif
#if wxUSE_COLLPANE
#include "wx/collpane.h"
#else
#include "wx/panel.h"
#endif

// ============================================================================
// implementation
// ============================================================================

// helper function: returns all array elements in a single comma-separated and
// newline-terminated string
static wxString AllAsString(const wxArrayString& a)
{
    wxString s;
    const size_t count = a.size();
    s.reserve(20*count);
    for ( size_t n = 0; n < count; n++ )
    {
        //s << a[n] << (n == count - 1 ? _T("\n") : _T(", "));
		s << a[n] << _T("\n");
    }

    return s;
}

// ----------------------------------------------------------------------------
// wxAboutDialogInfo
// ----------------------------------------------------------------------------

wxString wxAboutDialogInfo::GetDescriptionAndCredits() const
{
    wxString s = GetDescription();
    if ( !s.empty() )
        s << _T('\n');

    if ( HasDevelopers() )
        s << _T('\n') << _T("Developed by ") << AllAsString(GetDevelopers());

    if ( HasDocWriters() )
        s << _T('\n') << _T("Documentation by ") << AllAsString(GetDocWriters());

    if ( HasArtists() )
        s << _T('\n') << _T("Graphics art by ") << AllAsString(GetArtists());

    if ( HasTranslators() )
        s << _T('\n') << _T("Translations by ") << AllAsString(GetTranslators());

    return s;
}

wxIcon wxAboutDialogInfo::GetIcon() const
{
    wxIcon icon = m_icon;
    if ( !icon.Ok() && wxTheApp )
    {
        const wxTopLevelWindow * const
            tlw = wxDynamicCast(wxTheApp->GetTopWindow(), wxTopLevelWindow);
        if ( tlw )
            icon = tlw->GetIcon();
    }

    return icon;
}

// ----------------------------------------------------------------------------
// wxGenericAboutDialog
// ----------------------------------------------------------------------------

bool wxGenericAboutDialog::Create(const wxAboutDialogInfo& info)
{
    // TODO: should we use main frame as parent by default here?
    if ( !wxDialog::Create(NULL, wxID_ANY, _T("About ") + info.GetName(),
                           wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER|wxDEFAULT_DIALOG_STYLE) )
        return false;

    m_sizerText = new wxBoxSizer(wxVERTICAL);
    wxString nameAndVersion = info.GetName();
    if ( info.HasVersion() )
        nameAndVersion << _T(' ') << info.GetVersion();
    wxStaticText *label = new wxStaticText(this, wxID_ANY, nameAndVersion);
    wxFont fontBig(*wxNORMAL_FONT);
    fontBig.SetPointSize(fontBig.GetPointSize() + 2);
    fontBig.SetWeight(wxFONTWEIGHT_BOLD);
    label->SetFont(fontBig);

    m_sizerText->Add(label, wxSizerFlags().Centre().Border());
    m_sizerText->AddSpacer(5);

    AddText(info.GetCopyright());
    AddText(info.GetDescription());

    if ( info.HasWebSite() )
    {
#if wxUSE_HYPERLINKCTRL
        AddControl(new wxHyperlinkCtrl(this, wxID_ANY,
                                       info.GetWebSiteDescription(),
                                       info.GetWebSiteURL()));
#else
        AddText(info.GetWebSiteURL());
#endif // wxUSE_HYPERLINKCTRL/!wxUSE_HYPERLINKCTRL
    }

#if wxUSE_COLLPANE
    if ( info.HasLicence() )
        AddCollapsiblePane(wxT("License"), info.GetLicence());

    if ( info.HasDevelopers() )
        AddCollapsiblePane(wxT("Developers"),
                           AllAsString(info.GetDevelopers()));

    if ( info.HasDocWriters() )
        AddCollapsiblePane(wxT("Documentation writers"),
                           AllAsString(info.GetDocWriters()));

    if ( info.HasArtists() )
        AddCollapsiblePane(wxT("Artists"),
                           AllAsString(info.GetArtists()));

    if ( info.HasTranslators() )
        AddCollapsiblePane(wxT("Translators"),
                           AllAsString(info.GetTranslators()));

#else
	notebook = new wxNotebook(this, -1);
    if ( info.HasLicence() )
        AddNotebookPane(wxT("License"), info.GetLicence());
	
    if ( info.HasDevelopers() )
        AddNotebookPane(wxT("Developers"),
                           AllAsString(info.GetDevelopers()));

    if ( info.HasDocWriters() )
        AddNotebookPane(wxT("Documentation writers"),
                           AllAsString(info.GetDocWriters()));

    if ( info.HasArtists() )
        AddNotebookPane(wxT("Artists"),
                           AllAsString(info.GetArtists()));

    if ( info.HasTranslators() )
        AddNotebookPane(wxT("Translators"),
                           AllAsString(info.GetTranslators()));
	

    m_sizerText->Add(notebook, wxSizerFlags().Centre().Border());
#endif // wxUSE_COLLPANE

    DoAddCustomControls();


    wxSizer *sizerIconAndText = new wxBoxSizer(wxHORIZONTAL);
#if wxUSE_STATBMP
    wxIcon icon = info.GetIcon();
    if ( icon.Ok() )
    {
        sizerIconAndText->Add(new wxStaticBitmap(this, wxID_ANY, icon),
                                wxSizerFlags().Border(wxRIGHT));
    }
#endif // wxUSE_STATBMP
    sizerIconAndText->Add(m_sizerText, wxSizerFlags(1).Expand());

    wxSizer *sizerTop = new wxBoxSizer(wxVERTICAL);
    sizerTop->Add(sizerIconAndText, wxSizerFlags(1).Expand().Border());

    wxSizer *sizerBtns = CreateButtonSizer(wxOK);
    if ( sizerBtns )
    {
        sizerTop->Add(sizerBtns, wxSizerFlags().Expand().Border());
    }

    SetSizerAndFit(sizerTop);

    CentreOnScreen();

    return true;
}

void wxGenericAboutDialog::AddControl(wxWindow *win, const wxSizerFlags& flags)
{
    wxCHECK_RET( m_sizerText, _T("can only be called after Create()") );
    wxASSERT_MSG( win, _T("can't add NULL window to about dialog") );

    m_sizerText->Add(win, flags);
}

void wxGenericAboutDialog::AddControl(wxWindow *win)
{
    AddControl(win, wxSizerFlags().Border(wxDOWN).Centre());
}

void wxGenericAboutDialog::AddText(const wxString& text)
{
    if ( !text.empty() )
        AddControl(new wxStaticText(this, wxID_ANY, text));
}

#if wxUSE_COLLPANE
void wxGenericAboutDialog::AddCollapsiblePane(const wxString& title,
                                              const wxString& text)
{
    wxCollapsiblePane *pane = new wxCollapsiblePane(this, wxID_ANY, title);
    wxStaticText *txt = new wxStaticText(pane->GetPane(), wxID_ANY, text,
                                         wxDefaultPosition, wxDefaultSize,
                                         wxALIGN_CENTRE);

    // don't make the text unreasonably wide
    static const int maxWidth = wxGetDisplaySize().x/3;
    txt->Wrap(maxWidth);

    // NB: all the wxCollapsiblePanes must be added with a null proportion value
    m_sizerText->Add(pane, wxSizerFlags(0).Expand().Border(wxBOTTOM));
}
#else
void wxGenericAboutDialog::AddNotebookPane(const wxString& title,
                                              const wxString& text)
{
    wxPanel *pane = new wxPanel(notebook, -1 );
    wxTextCtrl *txt = new wxTextCtrl(pane, wxID_ANY, text,
                                         wxDefaultPosition, wxDefaultSize,
                                         wxALIGN_LEFT | wxTE_READONLY | wxTE_MULTILINE | wxTE_WORDWRAP);

    // don't make the text unreasonably wide
    static const int maxWidth = wxGetDisplaySize().x/3;
	txt->SetSize(wxSize(maxWidth, 250));

	notebook->AddPage(pane, title);
}
#endif

// ----------------------------------------------------------------------------
// public functions
// ----------------------------------------------------------------------------

void wxGenericAboutBox(const wxAboutDialogInfo& info)
{
    wxGenericAboutDialog dlg(info);
    dlg.ShowModal();
}

// currently wxAboutBox is implemented natively only under these platforms, for
// the others we provide a generic fallback here
#if !defined(__WXMSW__) && !defined(__WXMAC__) && !defined(__WXGTK26__)

void wxAboutBox(const wxAboutDialogInfo& info)
{
    wxGenericAboutBox(info);
}

#endif // platforms without native about dialog


#endif // wxUSE_ABOUTDLG

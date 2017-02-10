
#include <wx/wx.h>
#include <wx/defs.h>
#include <wx/dcbuffer.h>

#include "swashgauge.h"

BEGIN_EVENT_TABLE(SwashGauge, wxWindow)
	EVT_PAINT(SwashGauge::OnPaint)
END_EVENT_TABLE()
		
SwashGauge::SwashGauge(wxWindow* parent, 
					const wxWindowID id, 
					int max,
					const wxPoint& pos,
					const wxSize& size,
					long style)
			: wxGauge(parent, id, max, pos, size, style)
{
	wxLogMessage(_T("SwashGauge Creator"));	
	wxBufferedPaintDC dc(this);

	wxRect rect = GetRect();

	dc.SetPen(GetBackgroundColour());

	// Get character height so that we can center the text vertically.

	// Clear background
	dc.SetPen(*wxBLACK_PEN);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	dc.DrawRectangle(0, 0, rect.GetWidth(), rect.GetHeight());

	wxString text = wxString::Format(_T("%d %"), GetValue());
	wxCoord w, h;

	dc.GetTextExtent(text, &w, &h);
	dc.DrawText(text, rect.GetWidth()/2 - w/2, rect.GetHeight()/2 - h/2);

}

SwashGauge::~SwashGauge()
{

}

void SwashGauge::OnPaint(wxPaintEvent &event)
{
	wxLogMessage(_T("SwashGauge::OnPaint"));	
	wxBufferedPaintDC dc(this);

	wxRect rect = GetRect();

	dc.SetPen(GetBackgroundColour());

	// Get character height so that we can center the text vertically.

	// Clear background
	dc.SetPen(*wxBLACK_PEN);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	dc.DrawRectangle(0, 0, rect.GetWidth(), rect.GetHeight());

	wxString text = wxString::Format(_T("%d %"), 20);
	wxCoord w, h;

	dc.GetTextExtent(text, &w, &h);
	dc.DrawText(text, rect.GetWidth()/2 - w/2, rect.GetHeight()/2 - h/2);

}

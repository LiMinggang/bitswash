#ifndef _SWASHGAUGE_H_
#define _SWASHGAUGE_H_

#include <wx/wx.h>
#include <wx/gauge.h>

class SwashGauge: public wxGauge
{
public:
	SwashGauge(wxWindow* parent, 
				const wxWindowID id = wxID_ANY,
				int max = 100,
				const wxPoint& pos = wxDefaultPosition,
				const wxSize& size = wxDefaultSize,
				long style = wxGA_HORIZONTAL);
	~SwashGauge();


private:
	void OnPaint(wxPaintEvent &event);
};



#endif

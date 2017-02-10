

#include "swashlistctrl.h"
#include "swashgauge.h"


class TestListCtrl: public SwashListCtrl
{

public:
	TestListCtrl(wxWindow* parent,
					const wxWindowID id = wxID_ANY,
             		const wxPoint& pos = wxDefaultPosition,
               		const wxSize& size = wxDefaultSize,
               		long style = wxLC_VIRTUAL | wxLC_REPORT | wxSUNKEN_BORDER);

	virtual ~TestListCtrl();

private:

	wxString OnGetItemText(long item, long column) const;
	void OnPaint( wxPaintEvent &WXUNUSED(event));

	std::vector<wxGauge*> m_gauge_list;
	DECLARE_EVENT_TABLE()


};

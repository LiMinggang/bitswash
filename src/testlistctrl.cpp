

#include "testlistctrl.h"


BEGIN_EVENT_TABLE(TestListCtrl, SwashListCtrl)
	EVT_PAINT(TestListCtrl::OnPaint)
END_EVENT_TABLE()

static SwashColumnItem testcols[] = {
	{ id: 0, name: _T("col0"), title: _T("title0"), tooltip:_T("tip0"), width: 0 },
	{ id: 1, name: _T("col1"), title: _T("title1"), tooltip:_T("tip1"), width: 110 },
	{ id: 2, name: _T("col2"), title: _T("title2"), tooltip:_T("tip2"), width: 130 },
	{ id: 3, name: _T("col3"), title: _T("title3"), tooltip:_T("tip3"), width: 150 },
};


TestListCtrl::TestListCtrl(wxWindow *parent,
               	const wxWindowID id,
               	const wxPoint& pos,
               	const wxSize& size,
               	long style)
		: SwashListCtrl(parent, SWASHLISTCOL_SIZE(testcols), testcols, wxEmptyString, id, pos, size, style)
{
	int i=0;
	for(i=0;i<4;i++)
	{
		wxGauge *tmpGauge = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxDefaultSize);
		m_gauge_list.push_back(tmpGauge);
	}
	SetItemCount(4);
}

TestListCtrl::~TestListCtrl()
{
#if 1
	std::vector<wxGauge*>::const_iterator git = m_gauge_list.begin();
	for (git; git != m_gauge_list.end(); ++git)
	{
		wxGauge* tmpGauge = *git;
		tmpGauge->Show(0);
		delete *git;
	}
	m_gauge_list.clear();
	m_gauge_list.swap(m_gauge_list);
#endif
#if _DEBUG
	wxLogMessage(_T("TestListCtrl::Descructor"));
#endif
}

void TestListCtrl::OnPaint( wxPaintEvent &WXUNUSED(event))
{
	wxPaintDC dc(this);

	int topItem = GetTopItem();
	int bottomItem = topItem + GetCountPerPage();

	int i;

	wxLogMessage(_T("Paint %d - %d"), topItem, bottomItem);
	
	for(i=topItem; i<bottomItem; i++)
	{
		if (i > 3) break;
		std::vector<wxGauge*>::const_iterator git = m_gauge_list.begin();
		wxGauge *tmpGauge = git[i];

		int gaugecol = 3;
		int gx, gy, gw, gh;
		wxRect rect;
		int j=0;
		GetItemRect(i,rect);
		gy = rect.y;
		gh = rect.height;
		gx = rect.x;
		while(j < gaugecol)
		{
			gx += GetColumnWidth(j++);	

		}
		gw = GetColumnWidth(j);
		tmpGauge->SetSize(gx, gy, gw, gh);
		tmpGauge->SetValue(tmpGauge->GetValue()+1);
		tmpGauge->Show(1);

	}
	

#if _DEBUG
	wxLogMessage(_T("OnPaint"));
#endif
}

wxString TestListCtrl::OnGetItemText(long item, long column) const
{
	//TestListCtrl* pThis = const_cast<TestListCtrl*>(this);
#if _DEBUG
    wxRect rect;
	//GetItemRect(item, rect);

	wxLogMessage(_T("TestListCtrl:: GetItemText %d, %d Rect x %d y %d w %d h %d"), item, column, rect.x, rect.y, rect.width, rect.height );
#endif

	/*
	wxGauge *tmpGauge = new wxGauge(pThis, wxID_ANY, 100, wxPoint(rect.x + 4, rect.y + 2), wxSize(rect.width -15, rect.height -4));
	tmpGauge->SetValue(40);
	tmpGauge->Show(1);
	*/

	return wxString::Format(_T("%d:%d"), item, column);

}

/*
void TestListCtrl::UpdateListLine()
{
	int topItem = GetTopItem();
	int bottomItem = topItem + GetCountPerPage();
}
*/

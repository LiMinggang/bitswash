/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


//
// Class: BitSwash
//
// Created by: lim k. c. <admin@bitswash.org>
// Created on: Mon Feb  1 01:23:21 2007
//
#ifdef HAVE_CONFIG_H
	#  include <config.h>
#endif

#ifdef __WXMSW__
	#include <WinSock2.h>
	#define PREFIX "."
#endif

#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/url.h>
#include <wx/cmdline.h>
#include <wx/mstream.h>

#include "icons.h"
#include "bitswash.h"
#include "mainframe.h"
#include "functions.h"

const wxString g_BitSwashServerStr = wxT( "BitSwashMainApp" );
const wxString g_BitSwashTopicStr = wxT( "single-instance" );

wxConnectionBase *BitSwashAppSrv::OnAcceptConnection( const wxString& topic )
{
	if( topic.Lower() == g_BitSwashTopicStr )
	{
		// Check that there are no modal dialogs active
		wxWindowList::Node* node = wxTopLevelWindows.GetFirst();

		while( node )
		{
			wxDialog* dialog = wxDynamicCast( node->GetData(), wxDialog );

			if( dialog && dialog->IsModal() )
			{
				return NULL;
			}

			node = node->GetNext();
		}

		return new BitSwashAppConn();
	}
	else
		return NULL;
}

// Opens a file passed from another instance
bool BitSwashAppConn::OnExecute( const wxString& topic,
#if wxMAJOR_VERSION < 2 || (wxMAJOR_VERSION == 2 && wxMINOR_VERSION < 9)
	wxChar* data,
	int WXUNUSED( size ),
#else
	const void * data,
	size_t WXUNUSED( size ),
#endif
	wxIPCFormat WXUNUSED( format ) )
{
	MainFrame* frame = wxDynamicCast( wxGetApp().GetTopWindow(), MainFrame );
	wxString filenames( ( wxChar* )data );

	if( filenames.IsEmpty() )
	{
		// Just raise the main window
		if( frame )
		{
			frame->Restore();    // for minimized frame
			frame->Raise();
		}
	}
	else
	{
		// Check if the filename is already open,
		// and raise that instead.
		MainFrame::ReceiveTorrent(filenames);
	}

	return true;
}

IMPLEMENT_APP( BitSwash )

extern MainFrame * g_BitSwashMainFrame;

static struct AppIcons_t
{
	enum appicon_id id;
	wxImage *image;
	const unsigned char * filebuf;
	size_t filesize;
} AppIcons[] =
{
	{ BITSWASH_ICON_APP, NULL, &bitswash_bin[0], sizeof(bitswash_bin)/sizeof(bitswash_bin[0])},
	{ BITSWASH_ICON_APP256, NULL, &bitswash_256x256_bin[0], sizeof(bitswash_256x256_bin)/sizeof(bitswash_256x256_bin[0])},
	{ BITSWASH_ICON_TORRENT_OPEN, NULL, &torrent_open_bin[0], sizeof(torrent_open_bin)/sizeof(torrent_open_bin[0])},
	{ BITSWASH_ICON_TORRENT_CREATE, NULL, &torrent_create_bin[0], sizeof(torrent_create_bin)/sizeof(torrent_create_bin[0])},
	{ BITSWASH_ICON_TORRENT_OPENURL, NULL, &torrent_open_url_bin[0], sizeof(torrent_open_url_bin)/sizeof(torrent_open_url_bin[0])},
	{ BITSWASH_ICON_TORRENT_OPENDIR, NULL, &torrent_open_dir_bin[0], sizeof(torrent_open_dir_bin)/sizeof(torrent_open_dir_bin[0])},
	{ BITSWASH_ICON_TORRENT_START, NULL, &torrent_start_bin[0], sizeof(torrent_start_bin)/sizeof(torrent_start_bin[0])},
	{ BITSWASH_ICON_TORRENT_PAUSE, NULL, &torrent_pause_bin[0], sizeof(torrent_pause_bin)/sizeof(torrent_pause_bin[0])},
	{ BITSWASH_ICON_TORRENT_STOP, NULL, &torrent_stop_bin[0], sizeof(torrent_stop_bin)/sizeof(torrent_stop_bin[0])},
	{ BITSWASH_ICON_TORRENT_PROPERTIES, NULL, &torrent_properties_bin[0], sizeof(torrent_properties_bin)/sizeof(torrent_properties_bin[0])},
	{ BITSWASH_ICON_TORRENT_REMOVE, NULL, &torrent_remove_bin[0], sizeof(torrent_remove_bin)/sizeof(torrent_remove_bin[0])},
	{ BITSWASH_ICON_TORRENT_REMOVEDATA, NULL, &torrent_remove_data_bin[0], sizeof(torrent_remove_data_bin)/sizeof(torrent_remove_data_bin[0])},
	{ BITSWASH_ICON_TORRENT_MOVEUP, NULL, &torrent_move_up_bin[0], sizeof(torrent_move_up_bin)/sizeof(torrent_move_up_bin[0])},
	{ BITSWASH_ICON_TORRENT_MOVEDOWN, NULL, &torrent_move_down_bin[0], sizeof(torrent_move_down_bin)/sizeof(torrent_move_down_bin[0])},
	{ BITSWASH_ICON_OPTION_PREFERENCE, NULL, &option_preference_bin[0], sizeof(option_preference_bin)/sizeof(option_preference_bin[0])},
	{ BITSWASH_ICON_MAX, NULL, NULL, 0 }
};

static struct CountryFlags_t
{
	const wxChar code[4];
	wxString name;
    const unsigned char * filebuf;
    size_t filesize;
	int imgidx;             /* index in the imagelist */
} CountryFlags[] =
{
	{ _T( "--" ), _( "Unknown" ), 0, 0, -1,},
	{ _T( "AD" ), _( "Andorra" ), &ad_bin[0], sizeof(ad_bin)/sizeof(ad_bin[0]), -1,},
	{ _T( "AE" ), _( "United Arab Emirates" ), &ae_bin[0], sizeof(ae_bin)/sizeof(ae_bin[0]), -1,},
	{ _T( "AF" ), _( "Afghanistan" ), &af_bin[0], sizeof(af_bin)/sizeof(af_bin[0]), -1,},
	{ _T( "AG" ), _( "Antigua and Barbuda" ), &ag_bin[0], sizeof(ag_bin)/sizeof(ag_bin[0]), -1,},
	{ _T( "AI" ), _( "Anguilla" ), &ai_bin[0], sizeof(ai_bin)/sizeof(ai_bin[0]), -1,},
	{ _T( "AL" ), _( "Albania" ), &al_bin[0], sizeof(al_bin)/sizeof(al_bin[0]), -1,},
	{ _T( "AM" ), _( "Armenia" ), &am_bin[0], sizeof(am_bin)/sizeof(am_bin[0]), -1,},
	{ _T( "AN" ), _( "Netherlands Antilles" ), &an_bin[0], sizeof(an_bin)/sizeof(an_bin[0]), -1,},
	{ _T( "AO" ), _( "Angola" ), &ao_bin[0], sizeof(ao_bin)/sizeof(ao_bin[0]), -1,},
	{ _T( "AR" ), _( "Argentina" ), &ar_bin[0], sizeof(ar_bin)/sizeof(ar_bin[0]), -1,},
	{ _T( "AS" ), _( "American Samoa" ), &as_bin[0], sizeof(as_bin)/sizeof(as_bin[0]), -1,},
	{ _T( "AT" ), _( "Austria" ), &at_bin[0], sizeof(at_bin)/sizeof(at_bin[0]), -1,},
	{ _T( "AU" ), _( "Australia" ), &au_bin[0], sizeof(au_bin)/sizeof(au_bin[0]), -1,},
	{ _T( "AX" ), _( "Aland Islands" ), &aw_bin[0], sizeof(aw_bin)/sizeof(aw_bin[0]), -1,},
	{ _T( "AW" ), _( "Aruba" ), &ax_bin[0], sizeof(ax_bin)/sizeof(ax_bin[0]), -1,},
	{ _T( "AZ" ), _( "Azerbaijan" ), &az_bin[0], sizeof(az_bin)/sizeof(az_bin[0]), -1,},
	{ _T( "BA" ), _( "Bosnia and Herzegovina" ), &ba_bin[0], sizeof(ba_bin)/sizeof(ba_bin[0]), -1,},
	{ _T( "BB" ), _( "Barbados" ), &bb_bin[0], sizeof(bb_bin)/sizeof(bb_bin[0]), -1,},
	{ _T( "BD" ), _( "Bangladesh" ), &bd_bin[0], sizeof(bd_bin)/sizeof(bd_bin[0]), -1,},
	{ _T( "BE" ), _( "Belgium" ), &be_bin[0], sizeof(be_bin)/sizeof(be_bin[0]), -1,},
	{ _T( "BF" ), _( "Burkina Faso" ), &bf_bin[0], sizeof(bf_bin)/sizeof(bf_bin[0]), -1,},
	{ _T( "BG" ), _( "Bulgaria" ), &bg_bin[0], sizeof(bg_bin)/sizeof(bg_bin[0]), -1,},
	{ _T( "BH" ), _( "Bahrain" ), &bh_bin[0], sizeof(bh_bin)/sizeof(bh_bin[0]), -1,},
	{ _T( "BI" ), _( "Burundi" ), &bi_bin[0], sizeof(bi_bin)/sizeof(bi_bin[0]), -1,},
	{ _T( "BJ" ), _( "Benin" ), &bj_bin[0], sizeof(bj_bin)/sizeof(bj_bin[0]), -1,},
	{ _T( "BM" ), _( "Bermuda" ), &bm_bin[0], sizeof(bm_bin)/sizeof(bm_bin[0]), -1,},
	{ _T( "BN" ), _( "Brunei Darussalam" ), &bn_bin[0], sizeof(bn_bin)/sizeof(bn_bin[0]), -1,},
	{ _T( "BO" ), _( "Bolivia" ), &bo_bin[0], sizeof(bo_bin)/sizeof(bo_bin[0]), -1,},
	{ _T( "BR" ), _( "Brazil" ), &br_bin[0], sizeof(br_bin)/sizeof(br_bin[0]), -1,},
	{ _T( "BS" ), _( "Bahamas" ), &bs_bin[0], sizeof(bs_bin)/sizeof(bs_bin[0]), -1,},
	{ _T( "BT" ), _( "Bhutan" ), &bt_bin[0], sizeof(bt_bin)/sizeof(bt_bin[0]), -1,},
	{ _T( "BV" ), _( "Bouvet Island Bouvetoya" ), &bv_bin[0], sizeof(bv_bin)/sizeof(bv_bin[0]), -1,},
	{ _T( "BW" ), _( "Botswana" ), &bw_bin[0], sizeof(bw_bin)/sizeof(bw_bin[0]), -1,},
	{ _T( "BY" ), _( "Belarus" ), &by_bin[0], sizeof(by_bin)/sizeof(by_bin[0]), -1,},
	{ _T( "BZ" ), _( "Belize" ), &bz_bin[0], sizeof(bz_bin)/sizeof(bz_bin[0]), -1,},
	{ _T( "CA" ), _( "Canada" ), &ca_bin[0], sizeof(ca_bin)/sizeof(ca_bin[0]), -1,},
	{ _T( "CC" ), _( "Cocos Keeling Islands" ), &cc_bin[0], sizeof(cc_bin)/sizeof(cc_bin[0]), -1,},
	{ _T( "CD" ), _( "Congo" ), &cd_bin[0], sizeof(cd_bin)/sizeof(cd_bin[0]), -1,},
	{ _T( "CF" ), _( "Central African Republic" ), &cf_bin[0], sizeof(cf_bin)/sizeof(cf_bin[0]), -1,},
	{ _T( "CG" ), _( "Congo" ), &cg_bin[0], sizeof(cg_bin)/sizeof(cg_bin[0]), -1,},
	{ _T( "CH" ), _( "Switzerland" ), &ch_bin[0], sizeof(ch_bin)/sizeof(ch_bin[0]), -1,},
	{ _T( "CI" ), _( "Cote D'Ivoire" ), &ci_bin[0], sizeof(ci_bin)/sizeof(ci_bin[0]), -1,},
	{ _T( "CK" ), _( "Cook Islands" ), &ck_bin[0], sizeof(ck_bin)/sizeof(ck_bin[0]), -1,},
	{ _T( "CL" ), _( "Chile" ), &cl_bin[0], sizeof(cl_bin)/sizeof(cl_bin[0]), -1,},
	{ _T( "CM" ), _( "Cameroon" ), &cm_bin[0], sizeof(cm_bin)/sizeof(cm_bin[0]), -1,},
	{ _T( "CN" ), _( "China" ), &cn_bin[0], sizeof(cn_bin)/sizeof(cn_bin[0]), -1,},
	{ _T( "CO" ), _( "Colombia" ), &co_bin[0], sizeof(co_bin)/sizeof(co_bin[0]), -1,},
	{ _T( "CR" ), _( "Costa Rica" ), &cr_bin[0], sizeof(cr_bin)/sizeof(cr_bin[0]), -1,},
	{ _T( "CS" ), _( "Serbia and Montenegro" ), &cs_bin[0], sizeof(cs_bin)/sizeof(cs_bin[0]), -1,},
	{ _T( "CU" ), _( "Cuba" ), &cu_bin[0], sizeof(cu_bin)/sizeof(cu_bin[0]), -1,},
	{ _T( "CV" ), _( "Cape Verde" ), &cv_bin[0], sizeof(cv_bin)/sizeof(cv_bin[0]), -1,},
	{ _T( "CX" ), _( "Christmas Island" ), &cx_bin[0], sizeof(cx_bin)/sizeof(cx_bin[0]), -1,},
	{ _T( "CY" ), _( "Cyprus" ), &cy_bin[0], sizeof(cy_bin)/sizeof(cy_bin[0]), -1,},
	{ _T( "CZ" ), _( "Czech Republic" ), &cz_bin[0], sizeof(cz_bin)/sizeof(cz_bin[0]), -1,},
	{ _T( "DE" ), _( "Germany" ), &de_bin[0], sizeof(de_bin)/sizeof(de_bin[0]), -1,},
	{ _T( "DJ" ), _( "Djibouti" ), &dj_bin[0], sizeof(dj_bin)/sizeof(dj_bin[0]), -1,},
	{ _T( "DK" ), _( "Denmark" ), &dk_bin[0], sizeof(dk_bin)/sizeof(dk_bin[0]), -1,},
	{ _T( "DM" ), _( "Dominica" ), &dm_bin[0], sizeof(dm_bin)/sizeof(dm_bin[0]), -1,},
	{ _T( "DO" ), _( "Dominican Republic" ), &do_bin[0], sizeof(do_bin)/sizeof(do_bin[0]), -1,},
	{ _T( "DZ" ), _( "Algeria" ), &dz_bin[0], sizeof(dz_bin)/sizeof(dz_bin[0]), -1,},
	{ _T( "EC" ), _( "Ecuador" ), &ec_bin[0], sizeof(ec_bin)/sizeof(ec_bin[0]), -1,},
	{ _T( "EE" ), _( "Estonia" ), &ee_bin[0], sizeof(ee_bin)/sizeof(ee_bin[0]), -1,},
	{ _T( "EG" ), _( "Egypt" ), &eg_bin[0], sizeof(eg_bin)/sizeof(eg_bin[0]), -1,},
	{ _T( "EH" ), _( "Western Sahara" ), &eh_bin[0], sizeof(eh_bin)/sizeof(eh_bin[0]), -1,},
	{ _T( "ER" ), _( "Eritrea" ), &er_bin[0], sizeof(er_bin)/sizeof(er_bin[0]), -1,},
	{ _T( "ES" ), _( "Spain" ), &es_bin[0], sizeof(es_bin)/sizeof(es_bin[0]), -1,},
	{ _T( "ET" ), _( "Ethiopia" ), &et_bin[0], sizeof(et_bin)/sizeof(et_bin[0]), -1,},
	{ _T( "EU" ), _( "Europe" ), &eu_bin[0], sizeof(eu_bin)/sizeof(eu_bin[0]), -1,},
	{ _T( "FI" ), _( "Finland" ), &fi_bin[0], sizeof(fi_bin)/sizeof(fi_bin[0]), -1,},
	{ _T( "FJ" ), _( "Fiji" ), &fj_bin[0], sizeof(fj_bin)/sizeof(fj_bin[0]), -1,},
	{ _T( "FK" ), _( "Falkland Islands Malvinas" ), &fk_bin[0], sizeof(fk_bin)/sizeof(fk_bin[0]), -1,},
	{ _T( "FM" ), _( "Micronesia" ), &fm_bin[0], sizeof(fm_bin)/sizeof(fm_bin[0]), -1,},
	{ _T( "FO" ), _( "Faeroe Islands" ), &fo_bin[0], sizeof(fo_bin)/sizeof(fo_bin[0]), -1,},
	{ _T( "FR" ), _( "France" ), &fr_bin[0], sizeof(fr_bin)/sizeof(fr_bin[0]), -1,},
	{ _T( "GA" ), _( "Gabon" ), &ga_bin[0], sizeof(ga_bin)/sizeof(ga_bin[0]), -1,},
	{ _T( "GB" ), _( "United Kingdom of Great Britain & N. Ireland" ), &gb_bin[0], sizeof(gb_bin)/sizeof(gb_bin[0]), -1,},
	{ _T( "GD" ), _( "Grenada" ), &gd_bin[0], sizeof(gd_bin)/sizeof(gd_bin[0]), -1,},
	{ _T( "GE" ), _( "Georgia" ), &ge_bin[0], sizeof(ge_bin)/sizeof(ge_bin[0]), -1,},
	{ _T( "GF" ), _( "French Guiana" ), &gf_bin[0], sizeof(gf_bin)/sizeof(gf_bin[0]), -1,},
	{ _T( "GH" ), _( "Ghana" ), &gh_bin[0], sizeof(gh_bin)/sizeof(gh_bin[0]), -1,},
	{ _T( "GI" ), _( "Gibraltar" ), &gi_bin[0], sizeof(gi_bin)/sizeof(gi_bin[0]), -1,},
	{ _T( "GL" ), _( "Greenland" ), &gl_bin[0], sizeof(gl_bin)/sizeof(gl_bin[0]), -1,},
	{ _T( "GM" ), _( "Gambia" ), &gm_bin[0], sizeof(gm_bin)/sizeof(gm_bin[0]), -1,},
	{ _T( "GN" ), _( "Guinea" ), &gn_bin[0], sizeof(gn_bin)/sizeof(gn_bin[0]), -1,},
	{ _T( "GP" ), _( "Guadaloupe" ), &gp_bin[0], sizeof(gp_bin)/sizeof(gp_bin[0]), -1,},
	{ _T( "GQ" ), _( "Equatorial Guinea" ), &gq_bin[0], sizeof(gq_bin)/sizeof(gq_bin[0]), -1,},
	{ _T( "GR" ), _( "Greece" ), &gr_bin[0], sizeof(gr_bin)/sizeof(gr_bin[0]), -1,},
	{ _T( "GS" ), _( "South Georgia and the South Sandwich Islands" ), &gs_bin[0], sizeof(gs_bin)/sizeof(gs_bin[0]), -1,},
	{ _T( "GT" ), _( "Guatemala" ), &gt_bin[0], sizeof(gt_bin)/sizeof(gt_bin[0]), -1,},
	{ _T( "GU" ), _( "Guam" ), &gu_bin[0], sizeof(gu_bin)/sizeof(gu_bin[0]), -1,},
	{ _T( "GW" ), _( "Guinea-Bissau" ), &gw_bin[0], sizeof(gw_bin)/sizeof(gw_bin[0]), -1,},
	{ _T( "GY" ), _( "Guyana" ), &gy_bin[0], sizeof(gy_bin)/sizeof(gy_bin[0]), -1,},
	{ _T( "HK" ), _( "Hong Kong" ), &hk_bin[0], sizeof(hk_bin)/sizeof(hk_bin[0]), -1,},
	{ _T( "HM" ), _( "Heard and McDonald Islands" ), &hm_bin[0], sizeof(hm_bin)/sizeof(hm_bin[0]), -1,},
	{ _T( "HN" ), _( "Honduras" ), &hn_bin[0], sizeof(hn_bin)/sizeof(hn_bin[0]), -1,},
	{ _T( "HR" ), _( "Hrvatska Croatia" ), &hr_bin[0], sizeof(hr_bin)/sizeof(hr_bin[0]), -1,},
	{ _T( "HT" ), _( "Haiti" ), &ht_bin[0], sizeof(ht_bin)/sizeof(ht_bin[0]), -1,},
	{ _T( "HU" ), _( "Hungary" ), &hu_bin[0], sizeof(hu_bin)/sizeof(hu_bin[0]), -1,},
	{ _T( "ID" ), _( "Indonesia" ), &id_bin[0], sizeof(id_bin)/sizeof(id_bin[0]), -1,},
	{ _T( "IE" ), _( "Ireland" ), &ie_bin[0], sizeof(ie_bin)/sizeof(ie_bin[0]), -1,},
	{ _T( "IL" ), _( "Israel" ), &il_bin[0], sizeof(il_bin)/sizeof(il_bin[0]), -1,},
	{ _T( "IN" ), _( "India" ), &in_bin[0], sizeof(in_bin)/sizeof(in_bin[0]), -1,},
	{ _T( "IO" ), _( "British Indian Ocean Territory Chagos Archipelago" ), &io_bin[0], sizeof(io_bin)/sizeof(io_bin[0]), -1,},
	{ _T( "IQ" ), _( "Iraq" ), &iq_bin[0], sizeof(iq_bin)/sizeof(iq_bin[0]), -1,},
	{ _T( "IR" ), _( "Iran" ), &ir_bin[0], sizeof(ir_bin)/sizeof(ir_bin[0]), -1,},
	{ _T( "IS" ), _( "Iceland" ), &is_bin[0], sizeof(is_bin)/sizeof(is_bin[0]), -1,},
	{ _T( "IT" ), _( "Italy" ), &it_bin[0], sizeof(it_bin)/sizeof(it_bin[0]), -1,},
	{ _T( "JM" ), _( "Jamaica" ), &jm_bin[0], sizeof(jm_bin)/sizeof(jm_bin[0]), -1,},
	{ _T( "JO" ), _( "Jordan" ), &jo_bin[0], sizeof(jo_bin)/sizeof(jo_bin[0]), -1,},
	{ _T( "JP" ), _( "Japan" ), &jp_bin[0], sizeof(jp_bin)/sizeof(jp_bin[0]), -1,},
	{ _T( "KE" ), _( "Kenya" ), &ke_bin[0], sizeof(ke_bin)/sizeof(ke_bin[0]), -1,},
	{ _T( "KG" ), _( "Kyrgyz Republic" ), &kg_bin[0], sizeof(kg_bin)/sizeof(kg_bin[0]), -1,},
	{ _T( "KH" ), _( "Cambodia" ), &kh_bin[0], sizeof(kh_bin)/sizeof(kh_bin[0]), -1,},
	{ _T( "KI" ), _( "Kiribati" ), &ki_bin[0], sizeof(ki_bin)/sizeof(ki_bin[0]), -1,},
	{ _T( "KM" ), _( "Comoros" ), &km_bin[0], sizeof(km_bin)/sizeof(km_bin[0]), -1,},
	{ _T( "KN" ), _( "St. Kitts and Nevis" ), &kn_bin[0], sizeof(kn_bin)/sizeof(kn_bin[0]), -1,},
	{ _T( "KP" ), _( "Korea" ), &kp_bin[0], sizeof(kp_bin)/sizeof(kp_bin[0]), -1,},
	{ _T( "KR" ), _( "Korea" ), &kr_bin[0], sizeof(kr_bin)/sizeof(kr_bin[0]), -1,},
	{ _T( "KW" ), _( "Kuwait" ), &kw_bin[0], sizeof(kw_bin)/sizeof(kw_bin[0]), -1,},
	{ _T( "KY" ), _( "Cayman Islands" ), &ky_bin[0], sizeof(ky_bin)/sizeof(ky_bin[0]), -1,},
	{ _T( "KZ" ), _( "Kazakhstan" ), &kz_bin[0], sizeof(kz_bin)/sizeof(kz_bin[0]), -1,},
	{ _T( "LA" ), _( "Lao People's Democratic Republic" ), &la_bin[0], sizeof(la_bin)/sizeof(la_bin[0]), -1,},
	{ _T( "LB" ), _( "Lebanon" ), &lb_bin[0], sizeof(lb_bin)/sizeof(lb_bin[0]), -1,},
	{ _T( "LC" ), _( "St. Lucia" ), &lc_bin[0], sizeof(lc_bin)/sizeof(lc_bin[0]), -1,},
	{ _T( "LI" ), _( "Liechtenstein" ), &li_bin[0], sizeof(li_bin)/sizeof(li_bin[0]), -1,},
	{ _T( "LK" ), _( "Sri Lanka" ), &lk_bin[0], sizeof(lk_bin)/sizeof(lk_bin[0]), -1,},
	{ _T( "LR" ), _( "Liberia" ), &lr_bin[0], sizeof(lr_bin)/sizeof(lr_bin[0]), -1,},
	{ _T( "LS" ), _( "Lesotho" ), &ls_bin[0], sizeof(ls_bin)/sizeof(ls_bin[0]), -1,},
	{ _T( "LT" ), _( "Lithuania" ), &lt_bin[0], sizeof(lt_bin)/sizeof(lt_bin[0]), -1,},
	{ _T( "LU" ), _( "Luxembourg" ), &lu_bin[0], sizeof(lu_bin)/sizeof(lu_bin[0]), -1,},
	{ _T( "LV" ), _( "Latvia" ), &lv_bin[0], sizeof(lv_bin)/sizeof(lv_bin[0]), -1,},
	{ _T( "LY" ), _( "Libyan Arab Jamahiriya" ), &ly_bin[0], sizeof(ly_bin)/sizeof(ly_bin[0]), -1,},
	{ _T( "MA" ), _( "Morocco" ), &ma_bin[0], sizeof(ma_bin)/sizeof(ma_bin[0]), -1,},
	{ _T( "MC" ), _( "Monaco" ), &mc_bin[0], sizeof(mc_bin)/sizeof(mc_bin[0]), -1,},
	{ _T( "MD" ), _( "Moldova" ), &md_bin[0], sizeof(md_bin)/sizeof(md_bin[0]), -1,},
	{ _T( "ME" ), _( "Montenegro" ), &me_bin[0], sizeof(me_bin)/sizeof(me_bin[0]), -1,},
	{ _T( "MG" ), _( "Madagascar" ), &mg_bin[0], sizeof(mg_bin)/sizeof(mg_bin[0]), -1,},
	{ _T( "MH" ), _( "Marshall Islands" ), &mh_bin[0], sizeof(mh_bin)/sizeof(mh_bin[0]), -1,},
	{ _T( "MK" ), _( "Macedonia" ), &mk_bin[0], sizeof(mk_bin)/sizeof(mk_bin[0]), -1,},
	{ _T( "ML" ), _( "Mali" ), &ml_bin[0], sizeof(ml_bin)/sizeof(ml_bin[0]), -1,},
	{ _T( "MM" ), _( "Myanmar" ), &mm_bin[0], sizeof(mm_bin)/sizeof(mm_bin[0]), -1,},
	{ _T( "MN" ), _( "Mongolia" ), &mn_bin[0], sizeof(mn_bin)/sizeof(mn_bin[0]), -1,},
	{ _T( "MO" ), _( "Macao" ), &mo_bin[0], sizeof(mo_bin)/sizeof(mo_bin[0]), -1,},
	{ _T( "MP" ), _( "Northern Mariana Islands" ), &mp_bin[0], sizeof(mp_bin)/sizeof(mp_bin[0]), -1,},
	{ _T( "MQ" ), _( "Martinique" ), &mq_bin[0], sizeof(mq_bin)/sizeof(mq_bin[0]), -1,},
	{ _T( "MR" ), _( "Mauritania" ), &mr_bin[0], sizeof(mr_bin)/sizeof(mr_bin[0]), -1,},
	{ _T( "MS" ), _( "Montserrat" ), &ms_bin[0], sizeof(ms_bin)/sizeof(ms_bin[0]), -1,},
	{ _T( "MT" ), _( "Malta" ), &mt_bin[0], sizeof(mt_bin)/sizeof(mt_bin[0]), -1,},
	{ _T( "MU" ), _( "Mauritius" ), &mu_bin[0], sizeof(mu_bin)/sizeof(mu_bin[0]), -1,},
	{ _T( "MV" ), _( "Maldives" ), &mv_bin[0], sizeof(mv_bin)/sizeof(mv_bin[0]), -1,},
	{ _T( "MW" ), _( "Malawi" ), &mw_bin[0], sizeof(mw_bin)/sizeof(mw_bin[0]), -1,},
	{ _T( "MX" ), _( "Mexico" ), &mx_bin[0], sizeof(mx_bin)/sizeof(mx_bin[0]), -1,},
	{ _T( "MY" ), _( "Malaysia" ), &my_bin[0], sizeof(my_bin)/sizeof(my_bin[0]), -1,},
	{ _T( "MZ" ), _( "Mozambique" ), &mz_bin[0], sizeof(mz_bin)/sizeof(mz_bin[0]), -1,},
	{ _T( "NA" ), _( "Namibia" ), &na_bin[0], sizeof(na_bin)/sizeof(na_bin[0]), -1,},
	{ _T( "NC" ), _( "New Caledonia" ), &nc_bin[0], sizeof(nc_bin)/sizeof(nc_bin[0]), -1,},
	{ _T( "NE" ), _( "Niger" ), &ne_bin[0], sizeof(ne_bin)/sizeof(ne_bin[0]), -1,},
	{ _T( "NF" ), _( "Norfolk Island" ), &nf_bin[0], sizeof(nf_bin)/sizeof(nf_bin[0]), -1,},
	{ _T( "NG" ), _( "Nigeria" ), &ng_bin[0], sizeof(ng_bin)/sizeof(ng_bin[0]), -1,},
	{ _T( "NI" ), _( "Nicaragua" ), &ni_bin[0], sizeof(ni_bin)/sizeof(ni_bin[0]), -1,},
	{ _T( "NL" ), _( "Netherlands" ), &nl_bin[0], sizeof(nl_bin)/sizeof(nl_bin[0]), -1,},
	{ _T( "NO" ), _( "Norway" ), &no_bin[0], sizeof(no_bin)/sizeof(no_bin[0]), -1,},
	{ _T( "NP" ), _( "Nepal" ), &np_bin[0], sizeof(np_bin)/sizeof(np_bin[0]), -1,},
	{ _T( "NR" ), _( "Nauru" ), &nr_bin[0], sizeof(nr_bin)/sizeof(nr_bin[0]), -1,},
	{ _T( "NU" ), _( "Niue" ), &nu_bin[0], sizeof(nu_bin)/sizeof(nu_bin[0]), -1,},
	{ _T( "NZ" ), _( "New Zealand" ), &nz_bin[0], sizeof(nz_bin)/sizeof(nz_bin[0]), -1,},
	{ _T( "OM" ), _( "Oman" ), &om_bin[0], sizeof(om_bin)/sizeof(om_bin[0]), -1,},
	{ _T( "PA" ), _( "Panama" ), &pa_bin[0], sizeof(pa_bin)/sizeof(pa_bin[0]), -1,},
	{ _T( "PE" ), _( "Peru" ), &pe_bin[0], sizeof(pe_bin)/sizeof(pe_bin[0]), -1,},
	{ _T( "PF" ), _( "French Polynesia" ), &pf_bin[0], sizeof(pf_bin)/sizeof(pf_bin[0]), -1,},
	{ _T( "PG" ), _( "Papua New Guinea" ), &pg_bin[0], sizeof(pg_bin)/sizeof(pg_bin[0]), -1,},
	{ _T( "PH" ), _( "Philippines" ), &ph_bin[0], sizeof(ph_bin)/sizeof(ph_bin[0]), -1,},
	{ _T( "PK" ), _( "Pakistan" ), &pk_bin[0], sizeof(pk_bin)/sizeof(pk_bin[0]), -1,},
	{ _T( "PL" ), _( "Poland" ), &pl_bin[0], sizeof(pl_bin)/sizeof(pl_bin[0]), -1,},
	{ _T( "PM" ), _( "St. Pierre and Miquelon" ), &pm_bin[0], sizeof(pm_bin)/sizeof(pm_bin[0]), -1,},
	{ _T( "PN" ), _( "Pitcairn Island" ), &pn_bin[0], sizeof(pn_bin)/sizeof(pn_bin[0]), -1,},
	{ _T( "PR" ), _( "Puerto Rico" ), &pr_bin[0], sizeof(pr_bin)/sizeof(pr_bin[0]), -1,},
	{ _T( "PS" ), _( "Palestinian Territory" ), &ps_bin[0], sizeof(ps_bin)/sizeof(ps_bin[0]), -1,},
	{ _T( "PT" ), _( "Portugal" ), &pt_bin[0], sizeof(pt_bin)/sizeof(pt_bin[0]), -1,},
	{ _T( "PW" ), _( "Palau" ), &pw_bin[0], sizeof(pw_bin)/sizeof(pw_bin[0]), -1,},
	{ _T( "PY" ), _( "Paraguay" ), &py_bin[0], sizeof(py_bin)/sizeof(py_bin[0]), -1,},
	{ _T( "QA" ), _( "Qatar" ), &qa_bin[0], sizeof(qa_bin)/sizeof(qa_bin[0]), -1,},
	{ _T( "RE" ), _( "Reunion" ), &re_bin[0], sizeof(re_bin)/sizeof(re_bin[0]), -1,},
	{ _T( "RO" ), _( "Romania" ), &ro_bin[0], sizeof(ro_bin)/sizeof(ro_bin[0]), -1,},
	{ _T( "RS" ), _( "Serbia" ), &rs_bin[0], sizeof(rs_bin)/sizeof(rs_bin[0]), -1,},
	{ _T( "RU" ), _( "Russian Federation" ), &ru_bin[0], sizeof(ru_bin)/sizeof(ru_bin[0]), -1,},
	{ _T( "RW" ), _( "Rwanda" ), &rw_bin[0], sizeof(rw_bin)/sizeof(rw_bin[0]), -1,},
	{ _T( "SA" ), _( "Saudi Arabia" ), &sa_bin[0], sizeof(sa_bin)/sizeof(sa_bin[0]), -1,},
	{ _T( "SB" ), _( "Solomon Islands" ), &sb_bin[0], sizeof(sb_bin)/sizeof(sb_bin[0]), -1,},
	{ _T( "SC" ), _( "Seychelles" ), &sc_bin[0], sizeof(sc_bin)/sizeof(sc_bin[0]), -1,},
	{ _T( "SD" ), _( "Sudan" ), &sd_bin[0], sizeof(sd_bin)/sizeof(sd_bin[0]), -1,},
	{ _T( "SE" ), _( "Sweden" ), &se_bin[0], sizeof(se_bin)/sizeof(se_bin[0]), -1,},
	{ _T( "SG" ), _( "Singapore" ), &sg_bin[0], sizeof(sg_bin)/sizeof(sg_bin[0]), -1,},
	{ _T( "SH" ), _( "St. Helena" ), &sh_bin[0], sizeof(sh_bin)/sizeof(sh_bin[0]), -1,},
	{ _T( "SI" ), _( "Slovenia" ), &si_bin[0], sizeof(si_bin)/sizeof(si_bin[0]), -1,},
	{ _T( "SJ" ), _( "Svalbard & Jan Mayen Islands" ), &sj_bin[0], sizeof(sj_bin)/sizeof(sj_bin[0]), -1,},
	{ _T( "SK" ), _( "Slovakia Slovak Republic" ), &sk_bin[0], sizeof(sk_bin)/sizeof(sk_bin[0]), -1,},
	{ _T( "SL" ), _( "Sierra Leone" ), &sl_bin[0], sizeof(sl_bin)/sizeof(sl_bin[0]), -1,},
	{ _T( "SM" ), _( "San Marino" ), &sm_bin[0], sizeof(sm_bin)/sizeof(sm_bin[0]), -1,},
	{ _T( "SN" ), _( "Senegal" ), &sn_bin[0], sizeof(sn_bin)/sizeof(sn_bin[0]), -1,},
	{ _T( "SO" ), _( "Somalia" ), &so_bin[0], sizeof(so_bin)/sizeof(so_bin[0]), -1,},
	{ _T( "SR" ), _( "Suriname" ), &sr_bin[0], sizeof(sr_bin)/sizeof(sr_bin[0]), -1,},
	{ _T( "ST" ), _( "Sao Tome and Principe" ), &st_bin[0], sizeof(st_bin)/sizeof(st_bin[0]), -1,},
	{ _T( "SV" ), _( "El Salvador" ), &sv_bin[0], sizeof(sv_bin)/sizeof(sv_bin[0]), -1,},
	{ _T( "SY" ), _( "Syrian Arab Republic" ), &sy_bin[0], sizeof(sy_bin)/sizeof(sy_bin[0]), -1,},
	{ _T( "SZ" ), _( "Swaziland" ), &sz_bin[0], sizeof(sz_bin)/sizeof(sz_bin[0]), -1,},
	{ _T( "TC" ), _( "Turks and Caicos Islands" ), &tc_bin[0], sizeof(tc_bin)/sizeof(tc_bin[0]), -1,},
	{ _T( "TD" ), _( "Chad" ), &td_bin[0], sizeof(td_bin)/sizeof(td_bin[0]), -1,},
	{ _T( "TF" ), _( "French Southern Territories" ), &tf_bin[0], sizeof(tf_bin)/sizeof(tf_bin[0]), -1,},
	{ _T( "TG" ), _( "Togo" ), &tg_bin[0], sizeof(tg_bin)/sizeof(tg_bin[0]), -1,},
	{ _T( "TH" ), _( "Thailand" ), &th_bin[0], sizeof(th_bin)/sizeof(th_bin[0]), -1,},
	{ _T( "TJ" ), _( "Tajikistan" ), &tj_bin[0], sizeof(tj_bin)/sizeof(tj_bin[0]), -1,},
	{ _T( "TK" ), _( "Tokelau Tokelau Islands" ), &tk_bin[0], sizeof(tk_bin)/sizeof(tk_bin[0]), -1,},
	{ _T( "TL" ), _( "Timor-Leste" ), &tl_bin[0], sizeof(tl_bin)/sizeof(tl_bin[0]), -1,},
	{ _T( "TM" ), _( "Turkmenistan" ), &tm_bin[0], sizeof(tm_bin)/sizeof(tm_bin[0]), -1,},
	{ _T( "TN" ), _( "Tunisia" ), &tn_bin[0], sizeof(tn_bin)/sizeof(tn_bin[0]), -1,},
	{ _T( "TO" ), _( "Tonga" ), &to_bin[0], sizeof(to_bin)/sizeof(to_bin[0]), -1,},
	{ _T( "TR" ), _( "Turkey" ), &tr_bin[0], sizeof(tr_bin)/sizeof(tr_bin[0]), -1,},
	{ _T( "TT" ), _( "Trinidad and Tobago" ), &tt_bin[0], sizeof(tt_bin)/sizeof(tt_bin[0]), -1,},
	{ _T( "TV" ), _( "Tuvalu" ), &tv_bin[0], sizeof(tv_bin)/sizeof(tv_bin[0]), -1,},
	{ _T( "TW" ), _( "Taiwan" ), &tw_bin[0], sizeof(tw_bin)/sizeof(tw_bin[0]), -1,},
	{ _T( "TZ" ), _( "Tanzania" ), &tz_bin[0], sizeof(tz_bin)/sizeof(tz_bin[0]), -1,},
	{ _T( "UA" ), _( "Ukraine" ), &ua_bin[0], sizeof(ua_bin)/sizeof(ua_bin[0]), -1,},
	{ _T( "UG" ), _( "Uganda" ), &ug_bin[0], sizeof(ug_bin)/sizeof(ug_bin[0]), -1,},
	{ _T( "UM" ), _( "United States Minor Outlying Islands" ), &um_bin[0], sizeof(um_bin)/sizeof(um_bin[0]), -1,},
	{ _T( "US" ), _( "United States of America" ), &us_bin[0], sizeof(us_bin)/sizeof(us_bin[0]), -1,},
	{ _T( "UY" ), _( "Uruguay" ), &uy_bin[0], sizeof(uy_bin)/sizeof(uy_bin[0]), -1,},
	{ _T( "UZ" ), _( "Uzbekistan" ), &uz_bin[0], sizeof(uz_bin)/sizeof(uz_bin[0]), -1,},
	{ _T( "VA" ), _( "Holy See Vatican City State" ), &va_bin[0], sizeof(va_bin)/sizeof(va_bin[0]), -1,},
	{ _T( "VC" ), _( "St. Vincent and the Grenadines" ), &vc_bin[0], sizeof(vc_bin)/sizeof(vc_bin[0]), -1,},
	{ _T( "VE" ), _( "Venezuela" ), &ve_bin[0], sizeof(ve_bin)/sizeof(ve_bin[0]), -1,},
	{ _T( "VG" ), _( "British Virgin Islands" ), &vg_bin[0], sizeof(vg_bin)/sizeof(vg_bin[0]), -1,},
	{ _T( "VI" ), _( "US Virgin Islands" ), &vi_bin[0], sizeof(vi_bin)/sizeof(vi_bin[0]), -1,},
	{ _T( "VN" ), _( "Viet Nam" ), &vn_bin[0], sizeof(vn_bin)/sizeof(vn_bin[0]), -1,},
	{ _T( "VU" ), _( "Vanuatu" ), &vu_bin[0], sizeof(vu_bin)/sizeof(vu_bin[0]), -1,},
	{ _T( "WF" ), _( "Wallis and Futuna Islands" ), &wf_bin[0], sizeof(wf_bin)/sizeof(wf_bin[0]), -1,},
	{ _T( "WS" ), _( "Samoa" ), &ws_bin[0], sizeof(ws_bin)/sizeof(ws_bin[0]), -1,},
	{ _T( "YE" ), _( "Yemen" ), &ye_bin[0], sizeof(ye_bin)/sizeof(ye_bin[0]), -1,},
	{ _T( "YT" ), _( "Mayotte" ), &yt_bin[0], sizeof(yt_bin)/sizeof(yt_bin[0]), -1,},
	{ _T( "ZA" ), _( "South Africa" ), &za_bin[0], sizeof(za_bin)/sizeof(za_bin[0]), -1,},
	{ _T( "ZM" ), _( "Zambia" ), &zm_bin[0], sizeof(zm_bin)/sizeof(zm_bin[0]), -1,},
	{ _T( "ZW" ), _( "Zimbabwe" ), &zw_bin[0], sizeof(zw_bin)/sizeof(zw_bin[0]), -1,},
	{ _T( "" ), _T( "" ), 0, 0, -1}
};

std::map<wxString, int> CountryCodeIndexMap;
wxString g_BitSwashAppDir;
wxString g_BitSwashHomeDir;

static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
	{
		wxCMD_LINE_SWITCH, "h", "help", "Displays help on the command line parameters",
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP
	},
	{ wxCMD_LINE_PARAM, NULL, NULL, "File(s) to be opened", wxCMD_LINE_VAL_STRING,  wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },

	{ wxCMD_LINE_NONE }
};

bool BitSwash::OnInit()
{
	m_SigleAppChecker = 0;
	m_AppServer = 0;

	//==========================================================================
	if( !wxApp::OnInit() )
			return false;

	wxString name = wxString::Format( wxT( "BitSwash-%s" ), wxGetUserId().GetData() );
	m_SigleAppChecker = new wxSingleInstanceChecker( name );

	// If using a single instance, use IPC to
	// communicate with the other instance
	if( !m_SigleAppChecker->IsAnotherRunning() )
	{
		// Create a new server
		m_AppServer = new BitSwashAppSrv;

		if( !m_AppServer->Create( g_BitSwashServerStr ) )
		{
			wxLogDebug( wxGetTranslation(wxT( "Failed to create an IPC service." ) ));
		}
	}
	else
	{
		wxLogNull logNull;
		// OK, there IS another one running, so try to connect to it
		// and send it any filename before exiting.
		BitSwashAppClnt* client = new BitSwashAppClnt;
		// ignored under DDE, host name in TCP/IP based classes
		wxString hostName = wxT( "localhost" );
		// Create the connection
		wxConnectionBase* connection = client->MakeConnection( hostName, g_BitSwashServerStr, g_BitSwashTopicStr );

		if( connection )
		{
			// Only file names would be send to the instance, ignore other switches, options
			// Ask the other instance to open a file or raise itself
			wxString fnames;

			for( size_t i = 0; i < m_FileNames.GetCount(); ++i )
			{
				//The name is what follows the last \ or /
				fnames +=  m_FileNames[i] + wxT( ' ' );
			}

			connection->Execute( fnames );
			connection->Disconnect();
			delete connection;
		}
		else
		{
			wxMessageBox( wxGetTranslation(wxT( "Sorry, the existing instance may be too busy too respond.\nPlease close any open dialogs and retry." )),
						   APPNAME, wxICON_INFORMATION | wxOK );
		}

		delete client;
		return false;
	}

	wxFileName filename( GetExecutablePath() );
	filename.MakeAbsolute();
	g_BitSwashAppDir = filename.GetPath( wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR );
#ifdef __WXMSW__
	g_BitSwashHomeDir = g_BitSwashAppDir;
#else //linux: ~/.madedit
	g_BitSwashHomeDir = wxStandardPaths::Get().GetUserDataDir() + wxFILE_SEP_PATH;

	if( !wxDirExists( g_BitSwashHomeDir ) )
	{
		wxLogNull nolog; // no error message
		wxMkdir( g_BitSwashHomeDir );
	}
#endif

	wxHandleFatalExceptions();
	//==========================================================================

	int initcountdown = 1;
	SetVendorName( APPNAME );
	SetAppName( APPBINNAME );
	m_locale = NULL;
	m_btinitdone = false;
	//log to stderr
	m_logold = wxLog::SetActiveTarget( new wxLogStderr() );
	wxChar dirsep = wxFileName::GetPathSeparator( wxPATH_NATIVE );
#ifdef USERDATADIR
	wxString confDirName = _T( USERDATADIR );
#elif defined(__WXMSW__)
	wxString confDirName = wxStandardPaths::Get().GetDataDir();
#else
	wxString confDirName = wxStandardPaths::Get().GetUserDataDir();
#endif
	if( !wxFileName::DirExists( confDirName ) )
	{
		bool b = wxFileName::Mkdir( confDirName );
		if( !b ) wxLogError( _T( "Failed to create directory " ) + confDirName );
	}
	// change working directory to ~/.bitswash
	wxSetWorkingDirectory( confDirName );
	m_datapath = confDirName;
	m_configpath = confDirName + dirsep + APPBINNAME + _T( ".conf" );
	if( !wxFileName::FileExists( m_configpath ) )
	{
		wxFile *f = new wxFile();
		bool b = f->Create( m_configpath.c_str() );
		if( !b ) wxLogError( _T( "Failed to create file " ) + m_configpath );
	}
	m_config = new Configuration( APPNAME );
	/* workaround for libtorrent unable to resolve our ip address */
	wxIPV4address remote;
	remote.Hostname( _T( "www.google.com" ) );
	remote.Service( 80 );
	wxIPV4address local;
	wxSocketClient client;
	if( client.Connect( remote, false ) ) client.GetLocal( local );
	wxString ipAddr = local.IPAddress();
	wxLogDebug( _T( "Local ip %s\n" ), ipAddr.c_str() );
	m_config->m_local_ip = local;
	/* end workaround */
	SetLogLevel();
	SetLocale( m_config->GetLanguage() );
	m_dhtstatepath = confDirName + dirsep + APPBINNAME + _T( ".dhtstate" );
	m_savetorrentspath = confDirName + dirsep + _T( "torrents" ) ;
	if( !wxFileName::DirExists( m_savetorrentspath ) )
	{
		bool b = wxFileName::Mkdir( m_savetorrentspath );
		if( !b ) wxLogError( _T( "Failed to create directory " ) + m_savetorrentspath );
	}
	m_logpath = confDirName + dirsep + _T( "logs" ) ;
	if( !wxFileName::DirExists( m_logpath ) )
	{
		bool b = wxFileName::Mkdir( m_logpath );
		if( !b ) wxLogError( _T( "Failed to create directory " ) + m_logpath );
	}

	wxInitAllImageHandlers();

	LoadIcons();
	LoadFlags();
	LoadSettingIcons();
	m_btsession = new BitTorrentSession( this, m_config );
	if( m_btsession->Create() != wxTHREAD_NO_ERROR )
	{
		wxLogError( _T( "Error creating bit torrent session thread\n" ) );
		exit( -1 );
	}
	m_btsession->Run();
	//XXX shows some splash
	while( !m_btinitdone || ( initcountdown-- > 0 ) )
		wxSleep( 1 );
	g_BitSwashMainFrame = new MainFrame( 0L, APPNAME );
	if(!( m_config->GetUseSystray() && m_config->GetHideTaskbar() ))
	{
		g_BitSwashMainFrame->Show( TRUE );
		SetTopWindow( g_BitSwashMainFrame );
	}
	for( size_t i = 0; i < m_FileNames.GetCount(); ++i )
	{
		MainFrame::ReceiveTorrent(m_FileNames[i]);
	}
	SetExitOnFrameDelete( true );
	return TRUE;
}

int BitSwash::OnExit()
{
#if 0
	int i;
	wxLogDebug( _T( "BitSwash OnExit\n" ) );
	// kill bittorrent session
	//XXX lock
#endif
	wxLogDebug( _T( "Awaiting BitTorrent session exit ...\n" ) );
	if( m_btsession->IsAlive() )
	{
		m_btsession->Delete();
		wxLogDebug( _T( "BitTorrent session exited with code %ld\n" ),
					( long )m_btsession->Wait() );
	}
#if 0
	for( i = 0; i < BITSWASH_ICON_MAX; i++ )
	{
		delete AppIcons[i].image;
	}
	delete m_imglist_ctryflags;
	//restore log handle
#endif
	wxLog::SetActiveTarget( m_logold );
	if( m_SigleAppChecker )
		delete m_SigleAppChecker;

	if( m_AppServer )
		delete m_AppServer;

	return 0;
}

#define N_COUNTRY sizeof(CountryFlags) / sizeof(struct CountryFlags_t)
void BitSwash::LoadFlags()
{
	int i;
	wxChar dirsep = wxFileName::GetPathSeparator( wxPATH_NATIVE );
    wxMemoryInputStream fstream(&un_bin[0], sizeof(un_bin)/sizeof(un_bin[0]));
	wxImage empty(fstream);
	int emptyidx = 0;
	m_imglist_ctryflags = new wxImageList( 16, 11, false );
	emptyidx = m_imglist_ctryflags->Add( wxBitmap( empty ) );
	for( i = 0; i < N_COUNTRY ; ++i )
	{
		CountryCodeIndexMap[wxString( CountryFlags[i].code )] = i;
		if( ( wxIsEmpty( CountryFlags[i].code ) ) || ( ! wxStricmp( CountryFlags[i].code, _T( "--" ) ) ) )
		{
			CountryFlags[i].imgidx = emptyidx;
			continue;
		}

        wxMemoryInputStream stream(CountryFlags[i].filebuf, CountryFlags[i].filesize);
        wxImage flag( stream );
        CountryFlags[i].imgidx = m_imglist_ctryflags->Add( wxBitmap( flag ) );
	}
}

int BitSwash::GetCountryFlag( const wxString& code )
{
	if( code.IsEmpty() )
		return 0;
	std::map<wxString, int>::iterator it = CountryCodeIndexMap.find( code );
	if( it != CountryCodeIndexMap.end() )
	{
		wxASSERT( it->second < N_COUNTRY );
		return CountryFlags[it->second].imgidx;
	}
	/*for (i=0; i< N_COUNTRY ; i++)
	{
	    strcode2.sprintf(_T("%s"), CountryFlags[i].code);

	    if ( ! (strcode2.CmpNoCase(strcode.Mid(0,2)) ) )
	        return CountryFlags[i].imgidx;
	}*/
	return -1;
}
#undef N_COUNTRY

void BitSwash::LoadSettingIcons()
{
	int i;
	wxChar dirsep = wxFileName::GetPathSeparator( wxPATH_NATIVE );
	wxImage empty( 16, 16, true );
	m_imglist_settingicons = new wxImageList( 16, 16, false );
	static struct 
    {
        const unsigned char * filebuf;
        size_t filesize;
    } settingfiles[] =
	{
		&settinggeneral_bin[0], sizeof(settinggeneral_bin)/sizeof(settinggeneral_bin[0]),
		&settingconnection_bin[0], sizeof(settingconnection_bin)/sizeof(settingconnection_bin[0]),
		&settingtorrent_bin[0], sizeof(settingtorrent_bin)/sizeof(settingtorrent_bin[0]),
		&settingextension_bin[0], sizeof(settingextension_bin)/sizeof(settingextension_bin[0]),
	};
	for( i = 0; i < sizeof(settingfiles)/sizeof(settingfiles[0]); i++ )
	{
        wxMemoryInputStream stream(settingfiles[i].filebuf, settingfiles[i].filesize);
        wxImage flags(stream);
        m_imglist_settingicons->Add( wxBitmap( flags ) );
	}
}

bool BitSwash::SetLocale( wxString lang )
{
	const wxLanguageInfo * const langinfo = wxLocale::FindLanguageInfo( lang );
	int langid = wxLANGUAGE_DEFAULT;
	if( lang != _T( "" ) )
	{
		langid = langinfo->Language;
	}
	if( m_locale != NULL )
	{
		delete m_locale;
	}
	m_locale = new wxLocale( langid/*, wxLOCALE_CONV_ENCODING*/ );
	if( !m_locale->IsOk() )
	{
		wxLogError( _T( "Language %s is not supported by your system." ), langinfo->CanonicalName.c_str() );
	}
	else
	{
#ifdef  __WXMSW__
		wxString localedir =  wxStandardPaths::Get().GetDataDir() + wxFileName::GetPathSeparator( wxPATH_NATIVE ) + _T( "locale" );
#else
#ifndef PACKAGE_LOCALE_DIR
		wxString localedir =  wxString::FromAscii( PREFIX ) + _T( "/share/locale" );
#else
		wxString localedir = wxString::FromAscii( PACKAGE_LOCALE_DIR );
#endif
#endif
		m_locale->AddCatalogLookupPathPrefix( _T( "." ) );
		wxLocale::AddCatalogLookupPathPrefix( localedir );
		m_locale->AddCatalog( APPBINNAME );
		m_locale->AddCatalog( _T( "wxstd" ) );
	}
	return true;
}

void BitSwash::SetLogLevel()
{
	int wx_loglevel = wxLOG_Debug - m_config->GetLogSeverity();
	if( wx_loglevel >= wxLOG_Info )
		wxLog::SetVerbose( true );
	else
		wxLog::SetVerbose( false );
	wxLog::SetLogLevel( wx_loglevel );
}

void BitSwash::LoadIcons()
{
	int i;
	wxChar dirsep = wxFileName::GetPathSeparator( wxPATH_NATIVE );

	for( i = 0; i < BITSWASH_ICON_MAX; ++i )
	{
        wxMemoryInputStream stream(AppIcons[i].filebuf, AppIcons[i].filesize);
        AppIcons[i].image = new wxImage( stream );
	}
}

wxImage & BitSwash::GetAppIcon( enum appicon_id id )
{
	wxASSERT( id < BITSWASH_ICON_MAX );
	
	static wxImage empty( 16, 16, true );
	//wxLogMessage(_T("size %dx%d\n"), AppIcons[id].image->GetHeight(), AppIcons[id].image->GetWidth());
	if(AppIcons[id].image)
		return *AppIcons[id].image;
	else
		return empty;
}

void BitSwash::OnInitCmdLine( wxCmdLineParser& cmdParser )
{
	cmdParser.SetDesc( g_cmdLineDesc );
	// must refuse '/' as parameter starter or cannot use "/path" style paths
	cmdParser.SetSwitchChars( wxT( "-" ) );
}

bool BitSwash::OnCmdLineParsed( wxCmdLineParser& cmdParser )
{
	wxFileName filename;

	// parse commandline to filenames, every file is with a trailing char '|', ex: filename1|filename2|
	m_FileNames.Empty();
	// to get at your unnamed parameters use GetParam
	int flags = wxDIR_FILES | wxDIR_HIDDEN;
	wxString fname;

	for( size_t i = 0; i < cmdParser.GetParamCount(); i++ )
	{
		fname = cmdParser.GetParam( i );
		fname.Replace(wxT("\\\\"), wxT("\\"));
		filename = fname;

		filename.MakeAbsolute();
		fname = filename.GetFullName();

		//WildCard
		wxArrayString files;
		size_t nums = wxDir::GetAllFiles(filename.GetPath(), &files, fname, flags);

		for (size_t i = 0; i < nums; ++i)
		{
			m_FileNames.Add(files[i]);
		}
	}

	// and other command line parameters
	// then do what you need with them.
	return true;
}


#if (wxUSE_ON_FATAL_EXCEPTION == 1) && (wxUSE_STACKWALKER == 1)
#include <wx/longlong.h>
void BitSwashStackWalker::OnStackFrame( const wxStackFrame & frame )
{
	if( m_DumpFile )
	{
		wxULongLong address( ( size_t )frame.GetAddress() );
#if defined(__x86_64__) || defined(__LP64__) || defined(_WIN64)
		wxString fmt( wxT( "[%02u]:[%08X%08X] %s(%i)\t%s%s\n" ) );
#else
		wxString fmt( wxT( "[%02u]:[%08X] %s(%i)\t%s%s\n" ) );
#endif
		wxString paramInfo( wxT( "(" ) );
#if defined(_WIN32)
		wxString type, name, value;
		size_t count = frame.GetParamCount(), i = 0;

		while( i < count )
		{
			frame.GetParam( i, &type, &name, &value );
			paramInfo += type + wxT( " " ) + name + wxT( " = " ) + value;

			if( ++i < count ) paramInfo += wxT( ", " );
		}

#endif
		paramInfo += wxT( ")" );
		m_DumpFile->Write( wxString::Format( fmt,
											 ( unsigned )frame.GetLevel(),
#if defined(__x86_64__) || defined(__LP64__) || defined(_WIN64)
											 address.GetHi(),
#endif
											 address.GetLo(),
											 frame.GetFileName().c_str(),
											 ( unsigned )frame.GetLine(),
											 frame.GetName().c_str(),
											 paramInfo.c_str() )
						 );
	}
}

void BitSwash::OnFatalException()
{
	wxString name = g_BitSwashHomeDir + wxString::Format(
						wxT( "%s_%s_%lu.dmp" ),
						wxTheApp ? ( const wxChar* )wxTheApp->GetAppDisplayName().c_str()
						: wxT( "wxwindows" ),
						wxDateTime::Now().Format( wxT( "%Y%m%dT%H%M%S" ) ).c_str(),
#if defined(__WXMSW__)
						::GetCurrentProcessId()
#else
						( unsigned )getpid()
#endif
					);
	wxFile dmpFile( name.c_str(), wxFile::write );

	if( dmpFile.IsOpened() )
	{
		m_StackWalker.SetDumpFile( &dmpFile );
		m_StackWalker.WalkFromException();
		dmpFile.Close();
	}
}
#endif



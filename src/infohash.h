/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2015  Vladimir Golovnev <glassez@yandex.ru>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If you
 * modify file(s), you may extend this exception to your version of the file(s),
 * but you are not obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#ifndef BITTORRENT_INFOHASH_H
#define BITTORRENT_INFOHASH_H
#include <wx/wx.h>
#include <wx/string.h>

#include <libtorrent/sha1_hash.hpp>

class InfoHash
{
public:
    InfoHash();
    explicit InfoHash(const libtorrent::sha1_hash &nativeHash);
    explicit InfoHash(const wxString &hashString);
    InfoHash(const InfoHash &other);

    bool isValid() const;

    operator libtorrent::sha1_hash() const;
    operator wxString() const;
	InfoHash& operator=( const InfoHash& other ) {
	    m_valid      = other.m_valid;
	    m_nativeHash = other.m_nativeHash;
	    m_hashString = other.m_hashString;
		return *this;
	}
    bool operator==(const InfoHash &other) const;
    bool operator!=(const InfoHash &other) const;

private:
    bool m_valid;
    libtorrent::sha1_hash m_nativeHash;
    wxString m_hashString;
};

#endif // BITTORRENT_INFOHASH_H

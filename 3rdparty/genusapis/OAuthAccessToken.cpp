/*
 * Copyright (c) 2008 Kilian Marjew <kilian@marjew.nl>
 *               2008 Arend van Beelen jr. <arendjr@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * @author Kilian Marjew (kilian@marjew.nl)
 * @url http://genusapis.marjew.nl/
 */

#include "OAuthAccessToken.h"

namespace GenusApis {

OAuthAccessToken::OAuthAccessToken(QString key, QString secret, QString userId,
                                   QString methods, int expireDate) :
	OAuthBase(key, secret),
	m_userId(userId),
	m_methods(methods),
	m_expireDate(expireDate) {
}


QString OAuthAccessToken::userId() const {
	
	return m_userId;
}

QString OAuthAccessToken::methods() const {
	
	return m_methods;
}

int OAuthAccessToken::expireDate() const {
	
	return m_expireDate;
}

}

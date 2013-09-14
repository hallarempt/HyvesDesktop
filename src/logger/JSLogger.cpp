/*
 * Hyves Desktop, Copyright (C) 2008-2009 Hyves (Startphone Ltd.)
 * http://www.hyves.nl/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  US
 */

#include "JSLogger.h"
#include "extender/Extender.h"
//#include "config.h"

namespace Logger {

JSLogger::JSLogger(QObject *parent) : QObject(parent), Logger("JSLogger") {
	
	Extender::Extender::instance()->registerObject("jslogger", this);
}

JSLogger::~JSLogger() {
	
	Extender::Extender::instance()->unregisterObject("jslogger");
}

void JSLogger::debug(const QString &className, const QString &message) {
	
	log("JS-" + className, Debug, message);
}

void JSLogger::usage(const QString &className, const QString &message) {
	
	log("JS-" + className, Usage, message);
}

void JSLogger::notice(const QString &className, const QString &message) {
	
	log("JS-" + className, Notice, message);
}

void JSLogger::warning(const QString &className, const QString &message) {
	
	log("JS-" + className, Warning, message);
}

void JSLogger::error(const QString &className, const QString &message) {
	
	log("JS-" + className, Error, message);
}

void JSLogger::test(const QString &className, const QString &message) {
	
	log("JS-" + className, Test, message);
}

QString JSLogger::anonymize(const QString &data) {
	
	return Logger::anonymize(data, Remove);
}

QString JSLogger::anonymizeReplace(const QString &data) {
	
	return Logger::anonymize(data, Replace);
}


QString JSLogger::anonymizeEMail(const QString &data) {
	
	return Logger::anonymize(data, EMail);
}


QString JSLogger::anonymizeIPAddr(const QString &data) {
	
	return Logger::anonymize(data, IPAddr);
}

QString JSLogger::anonymizeFile(const QString &data) {
	return Logger::anonymize(data, File);
}

} // namespace Logger

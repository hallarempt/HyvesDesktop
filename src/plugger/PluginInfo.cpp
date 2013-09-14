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

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDomNodeList>
#include <QFile>

#include "PluginInfo.h"
#include "Util.h"

namespace Plugger {

PluginInfo::PluginInfo() :
	name("unknown"),
	displayName("Unknown Plugin"),
	vendor(""),
	version(),
	type(""),
	path(""),
	entry(""),
	registerEntryPoint(false),
	autoStart(false),
	loader(0) {
}
	
PluginInfo::PluginInfo(const PluginInfo &info) :
	name(info.name),
	displayName(info.displayName),
	vendor(info.vendor),
	version(info.version),
	type(info.type),
	path(info.path),
	entry(info.entry),
	registerEntryPoint(info.registerEntryPoint),
	autoStart(info.autoStart),
	loader(info.loader) {
}

bool PluginInfo::loadManifest(const QString &pluginPath) {
	
	QFile manifestFile(pluginPath + "/manifest.xml");
	bool result = manifestFile.open(QIODevice::ReadOnly);
	if (!result) {
		return false;
	}
	
	QByteArray content = manifestFile.readAll();
	result = loadManifestData(content);
	
	if (result) {
		path = pluginPath;
	}
	
	return result;
}
	
bool PluginInfo::loadManifestData(const QByteArray &content) {
	
	QDomDocument manifestDom;
	manifestDom.setContent(content);
	
	QDomNodeList nodeList;
	nodeList = manifestDom.elementsByTagName("metadata");
	if (nodeList.size() < 1) {
		return false;
	}
	
	for (QDomNode node = nodeList.at(0).firstChild(); !node.isNull();
	     node = node.nextSibling()) {
		if (!node.isElement()) {
			continue;
		}
		
		QDomElement el = node.toElement();
		if (el.tagName() == "name") {
			name = el.text();
		} else if (el.tagName() == "displayname") {
			displayName = el.text();
		} else if (el.tagName() == "vendor") {
			vendor = el.text();
		} else if (el.tagName() == "version") {
			version = el.text();
		} else if (el.tagName() == "autoStart") {
			autoStart = (el.text() != "false");
		} else if (el.tagName() == "type") {
			type = el.text();
		} else if (el.tagName() == "entry") {
			entry = Util::platformPath(el.text());
		} else if (el.tagName() == "registerEntryPoint") {
			registerEntryPoint = (el.text() != "false");
		}
	}
	
	nodeList = manifestDom.elementsByTagName("file");
	if (nodeList.size() < 1) {
		return false;
	}
	
	for (int i = 0; i < nodeList.size(); i++) {
		QDomElement fileElement = nodeList.at(0).toElement();
		
		PluginFile file;
		for (QDomNode node = fileElement.firstChild(); !node.isNull();
			     node = node.nextSibling()) {
			if (!node.isElement()) {
				continue;
			}
			
			QDomElement el = node.toElement();
			if (el.tagName() == "path") {
				file.path = Util::platformPath(el.text());
			} else if (el.tagName() == "md5") {
				file.md5 = el.text();
			}
		}
		
		if(file.path.isEmpty()) {
			return false;
		}
		
		files.append(file);
	}
	
	return true;
}

bool PluginInfo::saveManifest() const {
	
	QFile manifestFile(path + "/manifest.xml");
	bool result = manifestFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
	if (result == false) {
		return false;
	}
	
	QDomDocument manifestDom;
	QDomElement packageNode = manifestDom.createElement("package");
	manifestDom.appendChild(packageNode);
	
	QDomElement metaDataNode = manifestDom.createElement("metadata");
	packageNode.appendChild(metaDataNode);
	
	{
		QDomElement node;
		node = manifestDom.createElement("name");
		metaDataNode.appendChild(node);
		node.appendChild(manifestDom.createTextNode(name));
		
		node = manifestDom.createElement("displayname");
		metaDataNode.appendChild(node);
		node.appendChild(manifestDom.createTextNode(displayName));
		
		node = manifestDom.createElement("vendor");
		metaDataNode.appendChild(node);
		node.appendChild(manifestDom.createTextNode(vendor));
		
		node = manifestDom.createElement("version");
		metaDataNode.appendChild(node);
		node.appendChild(manifestDom.createTextNode(version.toString()));
		
		node = manifestDom.createElement("autoStart");
		metaDataNode.appendChild(node);
		node.appendChild(manifestDom.createTextNode(autoStart ? "true" : "false"));
		
		node = manifestDom.createElement("type");
		metaDataNode.appendChild(node);
		node.appendChild(manifestDom.createTextNode(type));
		
		node = manifestDom.createElement("entry");
		metaDataNode.appendChild(node);
		node.appendChild(manifestDom.createTextNode(entry));
		
		node = manifestDom.createElement("registerEntryPoint");
		metaDataNode.appendChild(node);
		node.appendChild(manifestDom.createTextNode(registerEntryPoint ? "true" : "false"));
	}
	
	QDomElement contentsNode = manifestDom.createElement("contents");
	packageNode.appendChild(contentsNode);
	
	for (QList<PluginFile>::ConstIterator it = files.begin();
	     it != files.end(); ++it) {
		
		QDomElement fileNode = manifestDom.createElement("file");
		packageNode.appendChild(fileNode);
		
		QDomElement node;
		node = manifestDom.createElement("path");
		fileNode.appendChild(node);
		node.appendChild(manifestDom.createTextNode((*it).path));
		
		node = manifestDom.createElement("md5");
		fileNode.appendChild(node);
		node.appendChild(manifestDom.createTextNode((*it).md5));
	}
	
	QByteArray byteArray = manifestDom.toByteArray();
	
	manifestFile.write(byteArray);
	manifestFile.close();
	
	return true;
}

PluginInfo &PluginInfo::operator=(const PluginInfo &info) {
	
	name = info.name;
	displayName = info.displayName;
	vendor = info.vendor;
	version = info.version;
	autoStart = info.autoStart;
	type = info.type;
	path = info.path;
	entry = info.entry;
	registerEntryPoint = info.registerEntryPoint;
	files = info.files;
	return *this;
}

bool PluginInfo::operator==(const PluginInfo &info) const {
	
	if (name != info.name ||
	    displayName != info.displayName ||
	    vendor != info.vendor ||
	    version != info.version ||
	    autoStart != info.autoStart ||
	    type != info.type ||
	    path != info.path ||
	    entry != info.entry ||
	    registerEntryPoint != info.registerEntryPoint ||
	    files.size() != info.files.size()) {
		return false;
	}
	
	for (int i = 0; i < files.size(); i++) {
		if (files.at(i).path != info.files.at(i).path ||
		    files.at(i).md5 != info.files.at(i).md5) {
			return false;
		}
	}
	
	return true;
}

} // namespace Plugger

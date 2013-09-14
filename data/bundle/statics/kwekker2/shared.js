
var CHAT_UI_MODE_WEB = 0;
var CHAT_UI_MODE_DESKTOP = 1;
var CHAT_UI_MODE_XUL = 2;
var CHAT_UI_MODE_DEVNULL = 3;
var CHAT_UI_MODE_KWEKKER2 = 4;

var ClientState = {
	Disconnected: 0,
	Connecting: 1,
	Authenticating: 2,
	Active: 3
}
	
var ClientResource = {
	SITE: 'siteClient',
	WEB: 'webClient',
	DESKTOP: 'desktopClient',
	OTHER: 'otherClient'
};

var ClientResourceBits = {
	'siteClient': 0x01,
	'webClient': 0x02,
	'desktopClient': 0x04,
	'otherClient': 0x08
};

/**
 * Parses a kwekker version info string into its
 * components and return as a dictionary with keys majorVersion, minorVersion, release and build.
 * return undefined if versionInfo could not be parsed
 */
function parseVersionInfo(versionInfo) {
	
	logging.debug("parsing version info: " + versionInfo);
	try {
		var vi = versionInfo.split('.');
		return {majorVersion: parseInt(vi[0]),
				minorVersion: parseInt(vi[1]),
				release: parseInt(vi[2]),
				build: parseInt(vi[3])};
	} catch(e) {
		logging.error('' + e, e);
	}
}

function jidresource(jid) {
	
	var split = jid.split('/');
	if(split.length != 2) {
		return null;
	} else {
		return split[1];
	}
}

function jidbare(jid) {
	
	var split = jid.split('/');
	if(split.length == 1 || split.length == 2) {
		return split[0];
	} else {
		return null;
	}
}

function jiduser(jid) {
	
	var bare = jidbare(jid);
	if(bare == null) {
		return null;
	} else {
		var splt = jid.split('@');
		if(splt.length == 2) {
			return splt[0];
		} else {
			return null;
		}
	}
}

/*
* removes all non id chars from jid (so that jid becomes usable as an id
*/
function jidid(jid) {
	
	return jid.replace(/[^0-9a-zA-Z_]/g, "");
}

/**
 * Gets the resource bit from the given jid
 */						  				 	  
function clientResourceBitFromJid(jid) {
	
	var resource = jidresource(jid);
	if(ClientResourceBits[resource]) {
		return ClientResourceBits[resource];
	} else {
		return ClientResourceBits[ClientResource.OTHER];
	}
}
					  				 	  
/**
 * Converts a long integer representing a date, to a Date object.
 */
function dateToObject(date) {
	
	var dateObject = new Date();
	dateObject.setDate(date);
	return dateObject;
}

/**
 * Parses the date format that is used in www updates and headlines.
 * This is a 14 digit string containing date and time.
 */
function parseDateString14(dateString) {
	
	var date = new Date();
	date.setFullYear(dateString.substring(0, 4));
	date.setMonth(dateString.substring(4, 6) - 1); // month is zero based
	date.setDate(dateString.substring(6, 8));
	date.setHours(dateString.substring(8, 10));
	date.setMinutes(dateString.substring(10, 12));
	date.setSeconds(dateString.substring(12, 14));
	return date;
}

/**
 * Parses the date format used by the history keeper, which has the form
 * "YYYY-MM-DD hh:mm", where the time part is optional.
 */
function parseHistoryDateString(dateString) {
	
	var date = new Date();
	date.setFullYear(dateString.substring(0, 4));
	date.setMonth(dateString.substring(5, 7) - 1);
	date.setDate(dateString.substring(8, 10));
	if (dateString.length > 10) {
		date.setHours(dateString.substring(11, 13));
		date.setMinutes(dateString.substring(14, 16));
	}
	return date;
}

//1-deep clone object what
function cloneObject(what) {
    for (i in what) {
        this[i] = what[i];
    }
}

//create a new array from a with 1-deep clones of the objects in a
function cloneObjectArray(a) {
	var c = new Array();
	a.each(function(v, i) {
		c.push(new cloneObject(v));
	});
	return c;
}

/**
 * Filters a message for display.
 *
 * E.g. it removes special codes that should never be displayed.
 */
function filterMessageForDisplay(message) {
	
	return message.replace('__buzz__', '');
}

/**
 * Returns whether a specified key code is an Enter key.
 */
function isEnterKey(keyCode) {
	
	return (keyCode == 3 || // Numpad Enter on Mac
		keyCode == 13); // Regular Enter key
}

/**
 * Returns whether a specified key code is a Backspace key.
 */
function isBackspaceKey(keyCode) {
	
	return (keyCode == 8);
}

/**
 * Returns whether a specified key code is a control character.
 */
function isControlCharacter(keyCode) {
	
	return (keyCode < 32 && keyCode > 0);
}

/**
 * Returns an object containing global variables used to initialize newly
 * created windows.
 */
function globalVars() {
	
	return {
		global_member_id: global_member_id,
		global_member_fullname: global_member_fullname,
		global_member_nickname: typeof(global_member_nickname) != 'undefined' ? global_member_nickname : null,
		global_member_smileypack: global_member_smileypack,
		global_wsa_key: global_wsa_key,
		global_wsa_member_id: global_wsa_member_id,
		global_wsa_url: global_wsa_url,
		hyves_base_url: hyves_base_url,
		hyves_cache_url: hyves_cache_url,
		hyves_smiley_path: hyves_smiley_path,
		member_domainname: member_domainname,
		messageSendPostman: messageSendPostman,
		messageTypeEmail: messageTypeEmail,
		messageTypeMemberToMember: messageTypeMemberToMember,
		messageTypeScrap: messageTypeScrap,
		messageTypeSms: messageTypeSms,
		personalisedItemsSubdir: personalisedItemsSubdir,
		scrapObjectSubtype: scrapObjectSubtype,
		scrapObjectType: scrapObjectType,
		scrapSendPostman: scrapSendPostman,
		MAX_SMS_LENGTH: MAX_SMS_LENGTH
	};
}

if(window.jsloaded) {jsloaded('/statics/kwekker2/shared.js')}

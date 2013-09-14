//just for logging the dom ready event
function urchinTracker() {
	//empty, will be overwritten later by google urchin tracker, prevents errors on dev
}


function GATrackEvent(groupsString, eventName)
{
	if (groupsString == '') {
		// we do not log empty groups ! 
		return;
	}
	if (typeof hyvesPageTracker == "undefined") {
		// tracker is not initialized yet... or this domain is using old version (urchin)
		// we schedule an event to be submited when tracker is initialized
		if (typeof googleAnalyticsScheduledEvents == "undefined") {
			googleAnalyticsScheduledEvents = new Array();
		}
		googleAnalyticsScheduledEvents.push(new Array(groupsString, eventName));
		return;
	}
	hyvesPageTracker._trackEvent(groupsString, eventName);
}

function GATracker(trackerParam) {
	// a wrapper for urchin tracker or a new tracking interface
	if (typeof _udn == "undefined")
	{
		return;
	}
	if (_udn == "hyves.nl")
	{
    	// in this domain we use a new tracker 
    	
		hyvesPageTracker = _gat._getTracker(_uacct);
		if (typeof trackerParam != "undefined"){
			hyvesPageTracker._trackPageview(trackerParam);
		}
		else {
			hyvesPageTracker._trackPageview();
		}
		// now submiting scheduled events
		if (typeof googleAnalyticsScheduledEvents != "undefined")
		{
			for (i=0; i<googleAnalyticsScheduledEvents.length; i++)
			{
				GATrackEvent(googleAnalyticsScheduledEvents[i][0], googleAnalyticsScheduledEvents[i][1]);
			}
			googleAnalyticsScheduledEvents = new Array();
		}
	}
	else
	{
		if (typeof trackerParam != "undefined"){
			urchinTracker(trackerParam);
		}
		else {
			if(typeof urchinTracker != 'undefined') {
				urchinTracker();
			}
		}
	}
}

function logError(sMsg, sUrl, sLine) {
	try {
		var msg = sUrl + "@" + sLine + ":" + sMsg;
		if(msg.indexOf('log.js') == -1) { 
			//this error not caused by logging itself
			logging.error("javascript: " + msg);
		}
		//also save errors on the window object so that unittest can find them
		//if anybody changes this, remember ieunit
		try {
			if(!window.jsErrors) {
				window.jsErrors = new Array();
			}
			window.jsErrors.push(msg);
		}
		catch(e) {}
	} catch(e) {}

	return window.suppressSiteErrors;
}
window.onerror = logError;

//IE
isIE = (navigator.userAgent.indexOf('MSIE') != -1);
isSafari = (navigator.userAgent.indexOf('Safari') != -1);
isWindows = (navigator.userAgent.indexOf('Windows') != -1);
isFF = (navigator.userAgent.toLowerCase().indexOf("firefox") != -1);

//DOMParser work arround
if (typeof DOMParser == "undefined") {
   DOMParser = function () {}
	
   DOMParser.prototype.parseFromString = function (str, contentType) {
      if (typeof ActiveXObject != "undefined") {    	// IE
         var d = new ActiveXObject("MSXML.DomDocument");
         d.loadXML(str);
         return d;
      }	else if (window.XMLHttpRequest)	{
         var req = new XMLHttpRequest;
         req.open("GET", "data:" + (contentType || "application/xml") +
                         ";charset=utf-8," + encodeURIComponent(str), false);
         if (req.overrideMimeType) {
            req.overrideMimeType(contentType);
         }
         req.send(null);
         return req.responseXML;
      } else { // ugly hack for KHTML browsers, will not work in other browsers.
	      var div = document.createElement('div');
	      div.innerHTML = xml;
	      return div;
	   }      
   }
}

function RemoveChildsFromDom(DomElement) {
	while (DomElement.hasChildNodes())
	{
		DomElement.removeChild(DomElement.firstChild);
	}	
}

function getTopLevelElement() {
    if ($('spread_img_container')) {
   		return $('spread_img_container').previousSiblings()[0]
    } else {
    	return document.getElementsByTagName('body')[0]; 
    }
}

function translateStringToDom(string) {
	var parser = new DOMParser(); 
	doc = parser.parseFromString(string, "text/xml"); 
	
	if (doc)
	{
		return doc;
	}
	else
	{
		alert('Operation not supported by browser.');
	}
}

function popUp(URL) {
	width = 700;
	height = 450;
	
	for (var i = 0; i < arguments.length; i++) {
        if(i==1) {
        	width = arguments[i];	
        }
        if(i==2) {
        	height = arguments[i];	
        }
    }
    window.open(URL, "Hyves", "toolbar=no,location=no,directories=no,status=yes,menubar=no,scrollbars=yes,resizable=no,copyhistory=no,width=" + width + ",height=" + height + ",screenX=0,screenY=0,top=0,left=0");
}


function popUpText(text, title) {
	var w = window.open("", title, "toolbar=no,location=no,directories=no,status=yes,menubar=no,scrollbars=yes,resizable=no,copyhistory=no,width=500,height=450,screenX=0,screenY=0,top=0,left=0");
	w.document.write(text);
	w.document.close();
}


function newTDWindow(URL) {
	var options = "toolbar=no,location=no,directories=no,status=yes,menubar=no,scrollbars=yes,resizable=yes,copyhistory=no,width=600,height=550";
	var newwidth 	= (self.screen.width / 2) - (550 / 2);
	var newheight 	= (self.screen.height / 2) - (450 / 2);
	popup = window.open(URL, "Hyves", options);
}



/** setActiveTab dynamically loads layers of content and manages the related menu.
 * To each menu item corresponds a layer.
 * The function displays the chosen (index) utility layer and hightlights the selected tab.
 *
 *  @param index - the index of the layer to be set visible
 *  @param highlighter - optional; a sign that hightlights the selected tab in the menu
 *  @param elementId - the common part of the id of the menu elements(<TD id=elementIdX>)
**/ 
function setActiveTab(index, highlighter, elementId){ 
	var isIE=document.all?true:false;
	var layers = isIE?document.all.tags("DIV"):null;
	var utilityLayers = new Array();
	for(var i=0;i<layers.length;i++) {
		var layerid = layers[i].id;
		if  (layerid.indexOf("utility") >= 0)
		utilityLayers.push(layers[i]);
	}
	 for(var i=0;i<utilityLayers.length;i++) {
 		var tabName = elementId + index + i;
	    var tab = document.getElementById(tabName);
	    if (i == index)  {
		   	utilityLayers[i].style.display = 'block';
		   	tab.className="MiniMenuActive";
		} else  {
			utilityLayers[i].style.display = 'none';
			tab.className="MiniMenuPassive";
		} 
		
	}
	return false;
}



/**
* Adds another upload field in case all fields are being used already
**/
function checkToAddMoreFileUploadFields(thisform, wheretoadd)
{
	needonemore = true;
	for(i=0;i<thisform.elements.length;i++)
	{
		if (thisform.elements[i].type=='file')
			if (thisform.elements[i].value == "")
				needonemore = false;
	}
	if (needonemore)
	{
		row = wheretoadd.insertRow(wheretoadd.rows.length-1);
		td1 = row.insertCell(0);
		td2 = row.insertCell(1);
		td1.innerHTML='<input type="file" name="file[]" onchange="checkToAddMoreFileUploadFields(this.form,document.getElementById(\'fileuploadtable\'))">';
		td2.innerHTML='<input type="text" name="comment[]"></td>\n';
	}
}

_base_url = null;
function base_url()
{
	if(_base_url == null) {
		if(window.location.protocol == "https:") {
			var unsecure_base_url = getCookie('UBU');
			if(unsecure_base_url != null) {
				_base_url = unsecure_base_url;	
			} 	
			else {
				_base_url = 'hyves.nl'; //safe default
			}
		}
		else {
			_base_url = hyves_base_url;	
		}
	}
	
	return _base_url;
}


/**
 * escapes the argument for use in html
 **/
function htmlspecialchars(s)
{
	s = s.replace(/&/g,"&amp;");
	s = s.replace(/>/g,"&gt;");
	s = s.replace(/</g,"&lt;");
	s = s.replace(/"/g,"&quot;");
	return s;
}

/**
 * This function does the same as escape() except it also correctly escapes the
 * plus sign (+) to %2B.
 */
function escapePlus(s) {
	return escape(s).replace(/\+/g, '%2B');
}

/**
 * Removes all Unicode characters from a string. This will leave only 7-bit
 * ASCII characters.
 */
function stripUnicodeCharacters(widgetHtml) {
	widgetHtml = widgetHtml.replace(/&#[0-9a-f]{4};/gi, "");
	for(var i = 0; i < widgetHtml.length; i++) {
		if(widgetHtml.charCodeAt(i) > 127) {
			widgetHtml = widgetHtml.substr(0, i) + widgetHtml.substr(i + 1);
			i--;
		}
	}
	return widgetHtml;
}

/**
 * Dumps an object/array/etc
 * dumping is done dynamically, so the next dump level is only calculated when requested - this allows for dumping of looping references
 * @param mixed v the variable to be dumped
 * @param object location is an element, the output is put in that element, else if loaction = "return" the output is retuned, else the output is written to the document
 */
debug_dump_run = -1;
debug_dump_variable = [];
function Debug_dump(v, location)
{
	debug_dump_run++;
	debug_dump_variable[debug_dump_run] = v;
	if (typeof v == "object" || ""+v == "[object HTMLEmbedElement]")
	{
		html = "<table border=1>";
		for (i in v)
		{
			if (i == "SelectAll")
			{
				v[i]();
			}
			try{
				if (typeof v[i] == "function")
					html+="<tr><td style=\"font: 12px Arial,Helvetica; cursor: pointer;\" onclick=\"if (this.nextSibling.innerHTML =='') {opener.Debug_dump(opener.debug_dump_variable["+debug_dump_run+"]['"+i+"'],this.nextSibling); this.style.fontStyle='italic';}else{this.nextSibling.innerHTML=''; this.style.fontStyle='normal';}\" valign=\"top\">"+i+"()</td><td></td></tr>";
				else
					html+="<tr><td style=\"font: 12px Arial,Helvetica; cursor: pointer;\" onclick=\"if (this.nextSibling.innerHTML =='') {opener.Debug_dump(opener.debug_dump_variable["+debug_dump_run+"]['"+i+"'],this.nextSibling); this.style.fontStyle='italic';}else{this.nextSibling.innerHTML=''; this.style.fontStyle='normal';}\" valign=\"top\">"+i+"</td><td></td></tr>";
			} catch(e) {}
		}
		html +="</table>";
	}
	else
		html = "[type: "+(typeof v)+"]"+escape_html(v);
	
	if (location)
	{
		if (location == "return")
			return html;
		else
			location.innerHTML = html;
		
	}
	else
	{
		var w = window.open("", "Hyves", "toolbar=no,location=no,directories=no,status=yes,menubar=no,scrollbars=yes,resizable=no,copyhistory=no,width=500,height=450,screenX=0,screenY=0,top=0,left=0");
		w.document.open;
		w.document.write(html);
		w.document.close();
	}
}

/**
 * Escapes string, so that each char other than a letter is replaced by a 3 number representation being the ascii code of that number
 **/
function escapeHyves(s)
{
	newstring = "";
	strlen = s.length;
	for (i=0; i<strlen; i++)
	{
		c = s.charAt(i);
		if (c.match(/[a-zA-Z]/))
		{
			newstring += c;
		}
		else
		{
			int_c = c.charCodeAt(0);
			string_c = (1000+int_c).toString().substring(1); //makes sure the result is a number of 3 chars
			newstring += string_c;
		}
	}
	return newstring;
}

/**
 * Deescapes the string
 **/
function unescapeHyves(s)
{
	newstring = "";
	strlen = s.length;
	for (i=0; i<strlen; i++)
	{
		c = s.charAt(i);
		if (c.match(/[^0-9]/))
		{
			newstring += c;
		}
		else
		{
			int_c = s.substr(i,3);
			i+=2;
			newstring += String.fromCharCode(int_c);
		}
	}
	return newstring;
}

/**
* See php checkdate funtion
**/
function checkdate(month, day, year)
{
	var monthLength = new Array(31,28,31,30,31,30,31,31,30,31,30,31);
	// leap year (every 4 years, not at whole centuries unless century dividable by 4)
	if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
	{
		monthLength[1] = 29;
	}
	 return (day > 0 && month > 0 && month <=12 && day <= monthLength[month-1]);
}

/**
* Since IE and firefox use different methods, we abstact that here
*/
function createInputDo(type, name)
{
	if (isIE) // defined earlier as global
	{
		var dob = document.createElement('<input type="'+type+'" name="'+name+'">');
	}
	else
	{
		var dob = document.createElement('input');
		dob.name = name;
		dob.type = type;
	}
	return dob;
}

function changeLanguage(languageId)
{
	url = "/a2a.php?method=setlanguage&languageId="+languageId;
	var myAjax = new Ajax.Request( url,
     	{
     		method: 'get',
     		onComplete: function (response) {
     			var url = window.location.toString();
     			url = url.replace(/&current_language_id=\d*/, ""); //fix for #3707
     			window.location = url;
			}
		}
	);
}

function getWindowizableTagNames() {
	
	var aTagNames = new Array('embed', 'object', 'iframe');
	if(ie6) {
		aTagNames.push('select');
	}
	return aTagNames;
}

/**
 * Contains all the elements which are floating over windowizable components.
 * Elements are placed in this array when they are passed to
 * hideWindowizableComponentsUnder().
 */
aFloatingElements = new Array();

/**
 * Can contain a single element which should be considered modal, while a grey
 * mist is shown over the rest of the page.  
 */
modalElement = null;

/**
 * Contains all elements of windowizable components which are visible.
 */
aVisibleWindowizableElements = new Array();

/**
 * Contains all elements of windowizable components which have been hidden by
 * these functions.
 */
aHiddenWindowizableElements = new Array();

/**
 * Hides all "windowizable" components which are under the given element.
 */
function hideWindowizableComponentsUnder(element) {
	
	if(aFloatingElements.indexOf(element) != -1) {
		return;
	}
	
	aFloatingElements.push(element);
	
	findWindowizableElements();
	if(aVisibleWindowizableElements.length == 0) {
		return;
	}
	
	doUpdateWindowizableComponentsHiddenStatus();
}

/**
 * Shows all "windowizable" components which are under the given element.
 */
function showWindowizableComponentsUnder(element) {
	
	aFloatingElements = aFloatingElements.without(element);
	
	findWindowizableElements();
	
	doUpdateWindowizableComponentsHiddenStatus();
}

/**
 * Hides all "windowizable" components which are not a descendant of the given
 * elment.
 * 
 * This function can be applied to only one element at a time.
 */
function hideWindowizableComponentsNotIn(element) {
	
	modalElement = element;
	
	findWindowizableElements();
	if(aVisibleWindowizableElements.length == 0) {
		return;
	}
	
	doUpdateWindowizableComponentsHiddenStatus();
}

/**
 * Shows all "windowizable" components which are not a descendant of the given
 * elment.
 */
function showWindowizableComponentsNotIn(element) {
	
	modalElement = null;
	
	findWindowizableElements();
	
	doUpdateWindowizableComponentsHiddenStatus();
}

function findWindowizableElements() {
	
	var aTagNames = getWindowizableTagNames();
	aVisibleWindowizableElements = new Array();
	for(var i = 0; i < aTagNames.length; i++) {
		var aElements = $A(document.getElementsByTagName(aTagNames[i]));
		for(var j = 0; j < aElements.length; j++) {
			var element = aElements[j];
			if(element.style.visibility == '' || element.style.visibility == 'visible') {
				if(isWindows &&
				   (element.tagName == 'EMBED' && element.getAttribute('wmode') == 'transparent')) {
					continue;
				}
				aVisibleWindowizableElements.push(element);
			}
		}
	}
}

function doUpdateWindowizableComponentsHiddenStatus() {
	
	// hide elements which are not yet hidden and intersect with any of the
	// floating elements
	for(var i = 0; i < aVisibleWindowizableElements.length; i++) {
		var element = aVisibleWindowizableElements[i];
		if(!isInFloatingElements(element) && intersectsWithFloatingElements(element)) {
			element.style.visibility = 'hidden';
			aHiddenWindowizableElements.push(element);
			aVisibleWindowizableElements = aVisibleWindowizableElements.without(element);
			i--;
		}
	}
	
	// unhide elements which were hidden, but no longer intersect
	for(var i = 0; i < aHiddenWindowizableElements.length; i++) {
		var element = aHiddenWindowizableElements[i];
		if(!intersectsWithFloatingElements(element)) {
			element.style.visibility = 'visible';
			aHiddenWindowizableElements = aHiddenWindowizableElements.without(element);
			aVisibleWindowizableElements.push(element);
			i--;
		}
	}
}

function intersectsWithFloatingElements(element) {
	
	if(modalElement) {
		return true;
	}
	
	for(var i = 0; i < aFloatingElements.length; i++) {
		if(elementsIntersect(element, aFloatingElements[i])) {
			return true;
		}
	}
	return false;
}

function isInFloatingElements(element) {
	
	if(modalElement) {
		var parentNode = element.parentNode;
		while(parentNode) {
			if(parentNode == modalElement) {
				return true;
			}
			parentNode = parentNode.parentNode;
		}
		return false;
	}
	
	for(var i = 0; i < aFloatingElements.length; i++) {
		var floatingElement = aFloatingElements[i];
		
		var parentNode = element.parentNode;
		while(parentNode) {
			if(parentNode == floatingElement) {
				return true;
			}
			parentNode = parentNode.parentNode;
		}
	}
	return false;
}

/**
 * Returns whether element and element2 intersect with each other.
 */
function elementsIntersect(element, element2) {
	
	var s1 = getBorderCoordinates(element);
	var s2 = getBorderCoordinates(element2);
	return (s1.right >= s2.left && s1.left <= s2.right &&
	        s1.bottom >= s2.top && s1.top <= s2.bottom);
}

function getBorderCoordinates(element) {
	
	var aCumulativeOffset = Position.cumulativeOffset(element);
	return {
		left: aCumulativeOffset[0],
		top: aCumulativeOffset[1],
		right: aCumulativeOffset[0] + element.offsetWidth,
		bottom: aCumulativeOffset[1] + element.offsetHeight
	};
}

/**
* quote s so that it is suitable for use as content of xml element
*/
function quoteElement(s)
{
	s = s.replace(/&/g, "&amp;");
	s = s.replace(/</g, "&lt;");
    s = s.replace(/>/g, "&gt;"); 
    return s;
}

/**
* quote s so that it is suitable for use as attribute
* including the correct surrounding quotes
*/
function quoteAttr(s)
{
	s = this.quoteElement(s);
    if(s.indexOf('"') != -1) {
        if(s.indexOf("'") != -1) {
            s = '"' + s.replace(/"/g, "&quot;") + '"';
        }
        else {
			s = "'" + s + "'";
        }
    }
    else {
		s = '"' + s + '"';
    }
   	return s;
}

function dhtmlLoadScript(url)
{
	FastInit.addOnLoad( function () {
		var e = document.createElement("script");
		e.defer = true;
		e.src = url;
		e.type="text/javascript";
		document.getElementsByTagName("head")[0].appendChild(e);
	})
}

/**
 * Pops up a message, disabling the rest of the page
 **/
popupMessageSaver = {};
function popupMessage(html, popupType, callback, style)
{

	if (popupType == "alert")
	{
		html+='<table width="100%" border="0"><tr><td align="center"><input type="button" value="OK" style="display: block;" onclick="closePopupMessage()" /></td></tr></table>'
	}
	
	var documentbody = document.getElementsByTagName('body')[0];
	var blackmist = document.createElement('div');
	var sOffset = Position.realOffset(documentbody);
	var alligner = document.createElement('table');

	popupMessageSaver = {
		blackmist: blackmist,
		alligner: alligner,
		oldonscroll: window.onscroll
	};
	
	alligner.style.position = 'absolute';
	alligner.style.zIndex = 101;
	alligner.style.width = "100%";
	alligner.style.height = "100%";
	alligner.style.top = sOffset[1]+'px';
	alligner.style.left = sOffset[0]+'px';
	var row = alligner.insertRow(0);
	var cell = row.insertCell(0);
	cell.valign="center";
	cell.align="center";
	
	var messageContainer = document.createElement('div');
	messageContainer.style.border = '1px black solid';
	messageContainer.style.backgroundColor = 'white';
	messageContainer.style.textAlign = 'center';
	messageContainer.style.padding = '20px';
	messageContainer.style.width = '640px';
	if (style)
	{
		for (i in style)
		{
			messageContainer.style[i] = style[i];
		}
	}
	cell.appendChild(messageContainer);
	
	documentbody.appendChild(alligner);
	messageContainer.innerHTML = html;		// since the appendshild above will evaluate all javascript firefox (and we don't want that, we'll do it later ourselves), we only add the HTML after the appendchild
	
	hideWindowizableComponentsNotIn(messageContainer);

	blackmist.style.height = '300%';
	blackmist.style.width = '100%';
	blackmist.style.backgroundColor = 'black';
	blackmist.style.display = 'none';
	blackmist.style.opacity = 0;
	blackmist.style.position = 'absolute';
	blackmist.style.top = sOffset[1]+'px';
	blackmist.style.left = sOffset[0]+'px';
	blackmist.style.zIndex = 100;
	documentbody.appendChild(blackmist);
	Effect.Appear(blackmist, {from: 0, to: 0.7});
	
	html.evalScripts();
}

function closePopupMessage()
{
	//window.onscroll = popupMessageSaver.oldonscroll;

	var documentbody = document.getElementsByTagName('body')[0];
	documentbody.removeChild(popupMessageSaver.blackmist);
	documentbody.removeChild(popupMessageSaver.alligner);
	showWindowizableComponentsNotIn(popupMessageSaver.alligner.firstChild.firstChild.firstChild);
}

function showGreyPopUp(url, popupType, callback, style) {
	var myAjax = new Ajax.Request(url, 
		{ 
		method: 'get',
		evalScripts: true,
		onComplete: function (originalRequest) { popupMessage(''+originalRequest.responseText, popupType, callback, style); },
	    onFailure: function(request, error) {
        	logging.debug('Error ' + request + ' -- ' + error);
    		},
		onException: function (request, exception) {
        	logging.debug('Exception ' + request + ' -- ' + exception);
    		}
    	}
    );
}

function showPopupDialogFromUrl(id, title, url, sProp, aRequire) {
	var sDefaultprop = {
		y: 100,
		width: 700,
		alignTop: true,
		modal: true,
		draggable: true,
		dialogTemplate: 'default',
		dialogContentUrl: url,
		dialogMessage: title
	}
	if(!aRequire) {
		aRequire = [];
	}
	requires(['module:PopupDialogManager'].concat(aRequire), function () {
	      popupDialogManager.createDialog(id, Object.extend(sDefaultprop, sProp));
	});
}

/**
 * Platform independent way to get tru window dimension (width, height)
 */
function getWindowDimension() {
  var myWidth = 0, myHeight = 0;
  if( typeof( window.innerWidth ) == 'number' ) {
    //Non-IE
    myWidth = window.innerWidth;
    myHeight = window.innerHeight;
  } else if( document.documentElement && ( document.documentElement.clientWidth || document.documentElement.clientHeight ) ) {
    //IE 6+ in 'standards compliant mode'
    myWidth = document.documentElement.clientWidth;
    myHeight = document.documentElement.clientHeight;
  } else if( document.body && ( document.body.clientWidth || document.body.clientHeight ) ) {
    //IE 4 compatible
    myWidth = document.body.clientWidth;
    myHeight = document.body.clientHeight;
  }

  var dimension = new Object();
  dimension.width = myWidth;
  dimension.height = myHeight;
  
  return dimension;
}

/**
 * Gets the scrollOffset (x,y) of the current window as an array.
 * Takes into account the dock if present
 */
function getScrollOffset() {
	var offsets = document.viewport.getScrollOffsets();
	return [offsets.left, offsets.top];
}
 
function gotoNewBlog() {
	window.location = 'http://' + member_domainname + '/index.php?l3=bl&amp;l4=edit';
}

/**
* Sets the global variables _udn, _uacct and _uff, needed by google analytics
**/
function setGoogleInfo() {
	var host = new String(document.location.host);
	if (host.match(/hyves\.nl$/)){
		_uacct="UA-288839-1";
		_udn="hyves.nl";
		_uff=0;
	} else if (host.match(/hyves\.net$/)){
		_uacct="UA-288839-2";
		_udn="hyves.net";
		_uff=0;
	} else if (host.match(/hyves\.be$/)){
		_uacct="UA-288839-3";
		_udn="hyves.be";
		_uff=0;
	} else if (host.match(/hyves\.org$/)){
		_uacct="UA-288839-4";
		_udn="hyves.org";
		_uff=0;
	} else if (host.match(/hyves\.de$/)){
		_uacct="UA-288839-5";
		_udn="hyves.de";
		_uff=0;
	} else if (host.match(/hyves\.co\.uk$/)){
		_uacct="UA-288839-6";
		_udn="hyves.co.uk";
		_uff=0;
	} else if (host.match(/hyves\.es$/)){
		_uacct="UA-288839-7";
		_udn="hyves.es";
		_uff=0;
	} else if (host.match(/hyves\.it$/)){
		_uacct="UA-288839-8";
		_udn="hyves.it";
		_uff=0;
	} else {
		_uacct="UA-288839-1";
		_udn="hyves.nl";
		_uff=0;
	}
}

function detectNorton()
{
	if(typeof(SymWinOpen) != 'undefined' || typeof(SymError) != 'undefined') {
		return true;
	}
	else {
		return false;
	}
}

function detectNortonPopupBlocker()
{
	if(typeof(SymWinOpen) != 'undefined') {
		return true;
	}
	else {
		return false;
	}
}

/**
 * Performs a WSA Hyves API request using Ajax. Wraps around JSONRequest.
 */
function WSARequest() {
	this.callbackVar = "ha_callback";
	this.base_url = global_wsa_url + "/"; // include trailing slash
	
	// Default params, probably only ha_fancylayout is sometimes manually overridden
	this.extraParams = {
		ha_version: "1.1",
		ha_format: "json",
		ha_responsecode_always_200: "true",
		ha_fancylayout: "false",
		wsasecret: global_wsa_key
	}
}

WSARequest.prototype.log = logging.getLogger("WSARequest");

WSARequest.prototype.get = function(method, callback, extraParams) {
	Object.extend(this.extraParams, extraParams);
	var url = this.base_url + "?ha_method=" + method + "&" + Object.toQueryString(this.extraParams);
	var options = {
		callbackVar: this.callbackVar
	};
	
	var request = new JSONRequest();
	request.get(url, callback, options);
}

//global variables for the JSONRequest class
var _jsreqId = 0;
var _jsreqCb = {};

function JSONRequest_FlashResponse(response, userData)
{
	//logging.debug("json flash resp: " + response + ", userData:" + userData);
	eval(response);	
}

/**
 * Performs a JSON request using the dynamic script insertion method
 * good for cross-domain AJAX calls.
 * Uses the JSONP pattern of providing the name of the callback method
 * from the client.
 */
function JSONRequest()
{
	this.options = {
		callbackVar: "jsonp"
	}
}

JSONRequest.prototype.log = logging.getLogger("JSONRequest");

JSONRequest.prototype.get = function(url, callback, options) {
	try {
		var self = this;

		Object.extend(this.options, options);
		
		//this.log.debug("get:" + url);
		//remove any extra & chars from the end
		if(url.charAt(url.length - 1) == '&') {
			url = url.substring(0, url.length - 1);
		};
		
		_jsreqId++;
		var src = url + "&" + this.options.callbackVar + "=_jsreqCb.cb_" + _jsreqId;
		
		//use dynamic js include trick
		var scriptElt = document.createElement("script");
		var scriptId = "_jsreq" + _jsreqId;
		scriptElt.id = scriptId;
		scriptElt.src = src;
		var jsreqId = _jsreqId;
		_jsreqCb["cb_" + _jsreqId] = function(response) {
			try {
				callback(response);
			} catch(e) {
				self.log.error("error in json request callback: " + e, e);
			}
			try {
				//now remove the callback and the script again
				delete _jsreqCb["cb_" +jsreqId];
				Element.remove(scriptId);
			} catch(e) {
				self.log.error("could not remove callback element: " + e, e);	
			}
		}
		var head = document.getElementsByTagName("head").item(0);        	    
		head.appendChild(scriptElt);
	} catch(e) {
		this.log.error("unknown error in jsonrequest" + e, e);
	}
}

/**
 * determines whether we are currently in given
 * environment
 */
function inEnvironment(environment)
{
	if(hyves_base_url.indexOf('startpda.net') != -1 && environment == 'development') {
		return true;
	} else {
		return false;
	}
}

/**
 * finds the index of ch in s from the right e.g.
 * it returns the first position from the right where ch lives
 * or -1 when it is not found in s
 */
function rightIndexOf(s, ch)
{
	for(var i = s.length - 1; i >= 0; i--) {
		if(s.charAt(i) == ch) {
			return i;
		}
	}
	return -1;
}

function startsWith(haystack, needle)
{
	return haystack.substring(0, needle.length) == needle;
}

/**
 * Returns a string containing a function that overrides
 * document.write() Instead of doing a document.write(), it will
 * do an innerHTML on the node given by nodeId.
 */
function overrideDocumentWrite(id) {
	//return 'document.write=function(s){alert('+id+');};';
	return 'tmp=document.write;document.write=function(s){alert('+id+');document.getElementById(\''+id+'\').innerHTML=s;};document.write=tmp;';	
}

/**
 * for given src returns the url on the static cache
 * e.g. this is exactly the same url that you get
 * from the {cachablefile} smarty directive.
 * note that you must include an entry in headerTemplates.tpl for this
 * method to work (add entry to _dynjsload mapping)
 * @return cache url from src or null if mapping not available
 */
function cachablefile(src)
{
	if(!window._dynjsload) {
		logging.error("no _dynjsload, was headerTemplates.js correctly included?");
		return;
	}
	
	if(_dynjsload[src]) {
		return cachedcontent_url(_dynjsload[src]);
	}
	else {
		logging.error("could not map to cached url: " + src + " please add mapping to _dynjsload in headerTemplates.tpl");
		return null;
	}
}

function cachedcontent_url(src){
		if(window.location.protocol == "http:" || hyves_cache_url != "") {
			return "http://" + hyves_cache_url + src;
		} else {
			/* relative if we're on https and no hyves_cache_url is set (for example on secure.hyves.org) 
			 * to prevent MSIE complaining about mixed content.
			 */
			return src;
		}
}

var _dynjsloaded = {}; //keeps track of which js source files were already loaded
var _dynjsloading = {}; //js files currently being loaded
var _dynjscheck = {}; //outstanding requests for requires calls that we need to check for completion
var _dynjscheckid = 0; //just an id used to stam each request to requires

//this method is called from the end of every js file 
//to signal that is has succesfully loaded and has become
//available
function jsloaded(src)
{
    logging.debug("!!JSLOADED: " + src);
	_dynjsloaded[src] = true;
	delete _dynjsloading[src];
    checkjsloaded();
}

function checkjsloaded()
{
    $H(_dynjscheck).each(function(pair) {
        var id = pair[0];
        var tocheck = pair[1][0];
        var callback = pair[1][1];
        
        var loaded = 0;
        //count how many srcs already loaded for requires request with id
        tocheck.each(function(src, j) {
            if(_dynjsloaded[src]) {
                loaded += 1;
            }
        });
        
        if(loaded == tocheck.length) {
            //everything complete, we dont need
            //to check anymore and can call the callback
            delete _dynjscheck[id];
            setTimeout(function() {
               callback();
            }, 0);
        }
    });
}

//an attempt to put some functions in a different namespace:
var Hyves = {
	log: logging.getLogger("Hyves"),
	domReadyFired: false,
	domReadyListeners: [],
	onDomReady: function(f) {
		if(this.domReadyFired) {
			setTimeout(f, 0);
		}
		else {
			this.domReadyListeners.push(f);
		}
	},
	fireOnDomReady: function() {
		if(this.domReadyFired) {
			return;
		}
		else {
			this.log.debug("** HYVES DOM READY **");
			this.domReadyFired = true;
			this.domReadyListeners.each(function(listener, i) {
				try {
					listener();
				}
				catch(e) {
				}			
			});
		}
	},
	onAvailable: function(fnName, callback){
		new PeriodicalExecuter(function(pe){
			if(eval("typeof " + fnName + " == 'function'")){
				pe.stop();
				callback();
			}	
		}, 0.3);
	},	
	
	stripNortonRE: null,
	
	//strips string from norton inserted meuk
	stripNorton: function(s) {
        if(!this.stripNortonRE) {
            this.stripNortonRE = new RegExp("\\s<script language=\"JavaScript\">\\s<!--[\\s\\S]*?\\s\/\/-->\\s<\/script>\\s\\s", 'g');
		}
		if(s.indexOf("Sym") != -1) {
			//could contain norton meuk
			logging.debug("norton meuk found, stripping");
            return s.replace(this.stripNortonRE, "", "g");			
		}
		else {
			return s;
		}
	},
	
	isKwekker: function() {
		if(window.kwekker) {
			return true;
		}
		else {
			return false;
		}
	},
	
	hasDock: function() {
		if(!$('dock_container')) {
			return false; //dock not present
		}
		else {
			return true;
		}
	},
	
	// custom events wrapper
	cachedHandlers: {},
	
	CustomEvent: function(name, obj, options){
		var defaultOptions = {
			clearOthers: false
		};
		
		var options = Object.extend(defaultOptions, options);

		this.name = name;
		this.rand = Math.random();
		this.observers = [];

		var self = this;
		
		if(Hyves.cachedHandlers[name] && options.clearOthers){
			Event.stopObserving(document, name);
//			Hyves.cachedHandlers[name].each(function(handler){
//				document.stopObserving(name, handler);
//			});
			delete Hyves.cachedHandlers[name];
			
			// TODO: (optimization) delete CustomEvents with same name themselves
			// need to store references to CustomEvents
		}

		if(!Hyves.cachedHandlers[name]){
			Hyves.cachedHandlers[name] = [];
		}
		
		var fn = function(event){
			// only fire on subscriptions of same origin
			if(event.memo._origin == self.rand){
				self.observers.each(function(fn){
					fn.apply(this, [event]);
				}.bind(this));
			}
		}.bind(obj);
		
		Hyves.cachedHandlers[name].push(fn);
		
		document.observe(name, fn);
		
		// return the actual object
		return {
			observe: function(fn) {
				Hyves.log.debug('Observing '+name);
				self.observers.push(fn);
			},
			fire: function(parameters){
				if(!parameters){
					parameters = {};
				}
				// add identity (origin)
				parameters['_origin'] = self.rand;
				Hyves.log.debug('Firing '+self.name);
				document.fire(self.name, parameters);
			}
		}
	}
}

/**
 * dynamically include given javascript and css sources and calls callback when
 * all of them have been loaded.
 */
function requires(srcs, callback)
{
    //can only function when both headerTemplates.js is loaded and DOM ready event
    //occurred

	//check if dom is ready, otherwise delay (because of dom mods below, and use of cachablefile)
	if(!Hyves.domReadyFired) {
		Hyves.onDomReady(function() {
			requires(srcs, callback);
		});
		return;
	}
	
	//if there are modules required, import them first
	//then continue
	var moduleCount = 0;
	var requiredModules = [];
	var requiredSrcs = []
	srcs.each(function(src, i) {
		if(startsWith(src, 'module:')) {
			requiredModules.push(src.substring('module:'.length));
		}
		else {
			requiredSrcs.push(src);
		}
	});
	 
	if(requiredModules.length > 0) {
	   //logging.debug("need to import modules first: " + requiredModules.length);
	   //yes there are some modules that need to be imported,
	   //we will import them first and the require the remaining srcs
	   var moduleCount = requiredModules.length;
	   var importedModules = {};
	   while(requiredModules.length > 0) {
		 var requiredModule = requiredModules.pop();
		 withModule(requiredModule, function(moduleInstance, importedModule) {
            moduleCount--;
            importedModules[importedModule] = moduleInstance;
            if(moduleCount == 0) {
            	//require the other requirements and finally call callback
            	//logging.debug("finished importing required modules, continuing with other sources");
            	requires(requiredSrcs, function() {
            		callback(importedModules);
            	});
            }            		 	
	     });
	   }    
	   return;
	}

    var tocheck = [];
	srcs.each(function(src, i) {
		try {
			//what we willdo depends on extension (js, css etc)
			if(!_dynjsloaded[src] && !_dynjsloading[src]) {
				//we still need todo something
				
				
				var srcUrl = cachablefile(src); //maps src to versioned source
				
				if(!srcUrl) {
					//dont know how to expand source
					logging.error("trying to include " + src + " but not declared in headerTemplates.tpl");
					//setTimeout(onload, 0);
					return;
				}
	
				//logging.debug("including: " + src);
			    //guess mime type on extension
			    var ext = src.substring(rightIndexOf(src, '.') + 1);
			    
			    if(ext == 'js' || ext == 'tpl') { //javascript or client side template
			    	var headElt = document.createElement("script");
				    headElt.src = srcUrl;
				    headElt.type = "text/javascript";
			        _dynjsloading[src] = true;
			    }
			    else if(ext == 'css') {
			    	var headElt = document.createElement("link");
			    	headElt.rel = "stylesheet";
			    	headElt.type = "text/css";
			    	headElt.href = srcUrl;
                    //we will not wait for css to load, it will just be
                    //ready when it is ready
                    _dynjsloaded[src] =  true;
			    }
			    else if(ext == 'vbs') {
			    	var headElt = document.createElement("script");
				    headElt.src = srcUrl;
				    headElt.type = "text/vbscript";
                    headElt.onreadystatechange = function() {
                        if(headElt.readyState == 'complete') {
                            jsloaded(src);
                        }
                    }
			    }
			    else {
					logging.error('unknown script language for include: ' + src + ', assuming js');
			    	var headElt = document.createElement("script");
				    headElt.src = srcUrl;
				    headElt.type = "text/javascript";
			    }
			    
			    var head = document.getElementsByTagName("head").item(0);
			    head.appendChild(headElt);
			    //logging.debug("include appended: " + srcUrl);
			}
            if(!_dynjsloaded[src]) {
                tocheck.push(src);
            }
		}
		catch(e) {
			logging.error('' + e, e);
		}
	});
    _dynjscheckid += 1; //generate a new id to identity this call to requires
    _dynjscheck[_dynjscheckid] = [tocheck, callback]; //note that we must still check these for completion
    //something could already have been loaded, so we need to check here as well
    checkjsloaded();
}

/**
 * removes chars from s that are not valid for a javascript identifier
 */
function identifier(s) {
	return s.replace(/[^0-9a-zA-Z_]/g, "");
}

/**
 * Shakes the current window, this is used in 'buzzing' functions
 */
function shakeWindow()
{
	if (parent.moveBy) {
		for (i = 15; i > 0; i--) {
			for (j = 2; j > 0; j--) {
				parent.moveBy(0,i);
				parent.moveBy(i,0);
				parent.moveBy(0,-i);
				parent.moveBy(-i,0);
			}
		}
	}
}

var _onidle = {}
/**
 * implements an idle timer, e.g. use this if you want some operation
 * to be performed some time after the last of a number of other operations.
 * This function will call f after timeout timeout, but if another call is made
 * to this function with the same id within the timeout, the timeout is rescheduled.
 */
function onidle(id, timeout, f)
{
	if(_onidle[id]) {
		clearTimeout(_onidle[id]);
	}
	_onidle[id] = setTimeout(function() {
		delete _onidle[id];
		f();
	}, timeout);
}

/**
 * Obtains the global instance of the module with given name and calls the provided
 * callback function f with this instance as an argument.
 * A module is just a javascript class and there will only every be 1 instance of it.
 * The module class can provide a list of requirements that will be dynamically included
 * before the module is instantiated like this:
 * 
 * MyFirstModule.prototype.requires = 
 *			['/statics/yui-ext/resources/css/ext-all.css',
 * 		     '/statics/yui-ext/resources/css/ytheme-aero.css',
 *		     '/statics/yui-ext/adapter/yui/yui-utilities.js',
 * 			 '/statics/yui-ext/adapter/yui/ext-yui-adapter.js',
 *			 '/statics/yui-ext/ext-all.js'];
 * 
 * All of these js and css files will be included into the current page before the
 * single instance of MyFirstModule is created
 * 
 * @note this method is asynchronous, e.g. f will be called at some time in the future and not before
 * this function returns
 * @note modules that can be returned by this function must be declared in headerTemplates.tpl
 */
var _moduleInstances = {};
var _moduleInstantiating = {};

function withModule(name, f)
{
    //if not ready for dynamic includes, delay until we are:
    if(!Hyves.domReadyFired) {
        Hyves.onDomReady(function() {
            withModule(name, f);
        });
        return;
    }

	if(_moduleInstances[name]) {
		//logging.debug('module with name ' + name + ' already initialized, returning instance');
		f(_moduleInstances[name], name);
		return;
	}
	
	if(_moduleInstantiating[name]) {
		//logging.debug('module with name ' +  name + ' already instantiating, waiting for instance');
		_moduleInstantiating[name].push(f);
		return;
	}
	
	if(!_modules[name]) {
		//logging.error('module with name ' + name + ' is unknown, edit headerTemplates.tpl');
		return;
	}

	//include the javascript that contains the service
	//and create a new instance after including all of the module
	//own requirements

	//logging.debug("including module: " + name);
	
	_moduleInstantiating[name] = [f];
	
	requires([_modules[name]], function() {
        //logging.debug("module source included for: " + name);
		var instantiateModuleAndCallback = function(importedModules) {
			//logging.debug("instantiating module: " + name);
			var instance = eval('new ' + name + '()');

            //if there were modules required by this module, we will
            //set them as properties of this module instance.
            //logging.debug("imported modules: " + importedModules);
            if(importedModules != undefined) {
                $H(importedModules).each(function(item) {
                	var propertyName = item[0].substring(0,1).toLowerCase() + item[0].substring(1);
                	var propertyValue = item[1];
                	//add the instance to the importing module
                    instance[propertyName] = propertyValue;   
                });
            }
			
			//add module instance to global list of module instances
			_moduleInstances[name] = instance;
			//and to global window scope
			var moduleInstanceName = name.substring(0,1).toLowerCase() + name.substring(1);
			window[moduleInstanceName] = instance;
			//let everybody know that instance was loaded
			_moduleInstantiating[name].each(function(callback, i) {
				callback(_moduleInstances[name], name);
			});
			delete _moduleInstantiating[name];
		}
		
		//logging.debug("checking module requirements for module: " + name);
		var moduleRequirements = eval(name + '.prototype.requires');
		if(moduleRequirements) {
			//logging.debug("including module requirements:");
			requires(moduleRequirements, instantiateModuleAndCallback);
		}
		else {
			instantiateModuleAndCallback();
		}
	});
}


/**
 * returns the current time in seconds (as a float)
 * since the unix epoch (e.g. 1-jan 1970 00:00)
 */
function currentTimeSeconds() 
{
	return new Date().getTime() / 1000.0;	
}

PIMP_SETTING_EXPIRE_TIMEOUT = 1000 * 60 * 60 * 24 * 365 * 20; //20 years

function togglePimp(nowPimping, postman)
{
	
	if (postman) {
		params=postman+'&viewpimp='+(nowPimping?1:0);
		new Ajax.Request('/', {postBody: params});
	}
	
	for (i in document.styleSheets)
	{
		if (document.styleSheets[i].title=="pimp")
		{	
			document.styleSheets[i].disabled = !nowPimping;
		}
	}

	if (nowPimping)
	{
		if ($('turnPimpOffButton')) {
			$('turnPimpOffButton').style.display = 'inline';
		}
		if ($('turnPimpOnButton')) {
			$('turnPimpOnButton').style.display = 'none';
		}
	}
	else
	{
		if ($('turnPimpOffButton')) {
			$('turnPimpOffButton').style.display = 'none';
		}
		if ($('turnPimpOnButton')) {
			$('turnPimpOnButton').style.display = 'inline';
		}
	}
}

/**
 * Selects initial presence and adds handler to set a cookie when it changes.
 */
_attachedInitialPresenceSelectors = [];
function attachInitialPresenceSelector(element)
{
	initialPresenceSelector = $(element);
	_attachedInitialPresenceSelectors.push(initialPresenceSelector);
	//if initial presence cookie is set, initialize all selectors in the page
	if(getCookie('chatInitialPresence')) {
		initialPresenceSelector.value = getCookie('chatInitialPresence');
	}
	//if any of the selectors changes, update all the other selectors
	//first remove any existing events
	Event.stopObserving(initialPresenceSelector, 'change');
	Event.observe(initialPresenceSelector, 'change', function(e) {
		var value = this.value;
		//set the cookie
		var expires = new Date(new Date().getTime() + 1000 * 60 * 60 * 24 * 365);
		setCookie('chatInitialPresence', value, expires, '/', hyves_base_url, false);
		//inform all other selectors
		_attachedInitialPresenceSelectors.each(function(x, i) {
			x.value = value;
		});
	});
}

function addToFriendgroup(fromMemberId, toMemberId) {
	
	withModule('AddToFriendGroupDialog', function(addToFriendGroupDialog) {
		addToFriendGroupDialog.showDialog(fromMemberId, toMemberId);
	});
}



function editRelation(fromMemberId, toMemberId, config)
{
	withModule('RelationEditor', function(relationEditor) {
		relationEditor.editRelation(fromMemberId, toMemberId, config);	
	});
}

function deleteRelation(fromMemberId, toMemberId)
{
	withModule('RelationEditor', function(relationEditor) {
		relationEditor.deleteRelation(fromMemberId, toMemberId);	
	});
}

function confirmRelation(fromMemberId, toMemberId)
{
    withModule('RelationEditor', function(relationEditor) {
        relationEditor.confirmRelation(fromMemberId, toMemberId);   
    });
}

function showRelation(fromMemberId, toMemberId, config)
{
	withModule('RelationEditor', function(relationEditor) {
		relationEditor.showRelation(fromMemberId, toMemberId, config);
	});
}

function editHubModerator(hubId, memberId) {
	withModule('HubModeratorEditDialog', function(hubModeratorEditDialog) {
		hubModeratorEditDialog.showDialog(hubId, memberId);
	});
}

/**
 * Sets a Cookie with the given name and value.
 *
 * name       Name of the cookie
 * value      Value of the cookie
 * [expires]  Expiration date of the cookie (default: end of current session)
 * [path]     Path where the cookie is valid (default: path of calling document)
 * [domain]   Domain where the cookie is valid
 *              (default: domain of calling document)
 * [secure]   Boolean value indicating if the cookie transmission requires a
 *              secure transmission
 */
function setCookie(name, value, expires, path, domain, secure) {
    document.cookie = name + "=" + escape(value) +
        ((expires) ? "; expires=" + expires.toGMTString() : "") +
        ((path) ? "; path=" + path : "") +
        ((domain) ? "; domain=" + domain : "") +
        ((secure) ? "; secure" : "");
}

/**
 * Gets the value of the specified cookie.
 *
 * name  Name of the desired cookie.
 *
 * Returns a string containing value of specified cookie,
 *   or null if cookie does not exist.
 */
function getCookie(name) {
    var dc = document.cookie;
    var prefix = name + "=";
    var begin = dc.indexOf("; " + prefix);
    if (begin == -1) {
        begin = dc.indexOf(prefix);
        if (begin != 0) return null;
    } else {
        begin += 2;
    }
    var end = document.cookie.indexOf(";", begin);
    if (end == -1) {
        end = dc.length;
    }
    return unescape(dc.substring(begin + prefix.length, end));
}

/**
 * Deletes the specified cookie.
 *
 * name      name of the cookie
 * [path]    path of the cookie (must be same as path used to create cookie)
 * [domain]  domain of the cookie (must be same as domain used to create cookie)
 */
function deleteCookie(name, path, domain) {
    if (getCookie(name)) {
        document.cookie = name + "=" +
            ((path) ? "; path=" + path : "") +
            ((domain) ? "; domain=" + domain : "") +
            "; expires=Thu, 01-Jan-70 00:00:01 GMT";
    }
}

/**
* Wrapper for document.write, since write has to be done in an external js file, to avoid eolas patent.....
**/
function documentwrite(html) {
	document.write(html);
}

/**
 * Adds a auto disable handler to given form, e.g. when form is submitted it 
 * is disabled to prevent multiple submits
 */
function autoDisableForm(id) {
	Event.observe(id, "submit", function() {
		setTimeout(function() {
			$(id).disable();
		}, 10);
	});
}

function fixFireFoxRenderBug(){
	if (isFF){
		var cc = $('center_container') || $('centercontents');
		if(cc){
			var width = parseInt(cc.getStyle('width')); 
			cc.setStyle({width: (width+0.1)+'px'});
			window.setTimeout(function(){
				cc.setStyle({width: width+'px'})
			},10);
		}
		else{
			logging.debug('Error: the #div.center_container could not be found');
		}
	}
}

/**
 * FF has a render bug that causes HTML content that was inserted by JavaScript (WebAds/DoubleClick) to fall
 * through the layout. Setting with main width ('center_container') 1 pixel wider for 100ms circumvents this bug.
 * Should be called onDomReady.
 */
function ffRenderBug(){
	[100,1000,3000,6000,10000].each(function(interval){
		window.setTimeout(fixFireFoxRenderBug,interval);
	});
}

/**
 * replaces the contents of element e with an ajax activity image
 */
function actionate(e)
{
	var oldContent = $(e).innerHTML;
	Element.update(e, "<img src='http://" + hyves_cache_url + "/images/ajax_action.gif'/>");
	return oldContent;
}

/**
 * Checks for printability of given media or album and if printable
 * goes to the appropriate order page
 */
function printMedia(mediaId, albumId, contextElement)
{
	var oldContent = null;
	if(contextElement) {
		oldContent = actionate(contextElement);
	}

	if(mediaId) {
		var isPrintableUrl = "/index.php?module=Media&action=isPrintable";
		var isPrintableParameters = {mediaId: mediaId};
		var orderUrl = "/index.php?module=OrderItemPhoto&action=edit&mediaId=" + mediaId + "&photoFunId=PHOTO&albumId="+albumId;
		var noRightText = templates.translate('PRINT_MEDIA_NO_RIGHT_TO_PRINT');
	}
	else if(albumId) 
	{
		var isPrintableUrl = "/index.php?module=Album&action=isPrintable";
		var isPrintableParameters = {albumId: albumId};
		var orderUrl = "/index.php?module=OrderItemPhoto&action=pickMediaForPrint&autoOpenAlbumId=" + albumId;
		var noRightText = templates.translate('PRINT_ALBUM_NO_RIGHT_TO_PRINT');
	}
	
	new Ajax.Request(isPrintableUrl, 
	{ 
		parameters: isPrintableParameters,
		method: 'get',
		evalScripts: true,
		onComplete: function (request) {
			var result = request.responseText.parseJSON();
			if(result.success && result.printable) {
				window.location = orderUrl;
			}
			else {
				alert(noRightText);
				Element.update(contextElement, oldContent);
			}
		},
	    onFailure: function(request, error) {
	    	logging.debug('Error ' + request + ' -- ' + error);
	    	Element.update(contextElement, oldContent);
			},
		onException: function (request, exception) {
	    	logging.debug('Exception ' + request + ' -- ' + exception);
	    	Element.update(contextElement, oldContent);
			}
	});
	
}

sSearchTerm = new Object();
currentsearch = undefined;
displayoverridden = false;
function doMemberSearch(searchwords) {
	if (searchwords == "") {
		searchwords = "SHOW_SEARCH_PAGE"
	}
    stateManager.changeState("searchhyver", searchwords);
    doMemberSearchHelper(searchwords);
}
	
function doMemberSearchHelper(searchwords) {
	doMemberSearchHelper.callcount++;
	if (doMemberSearchHelper.callcount == 1 && searchwords == "") {
		return;	//initial call, do nothing
	}

	var target=$('contenttable');
	var membersearchdone = function (mysearchwords, result) {
		if (currentsearch == mysearchwords) {
			setTimeout( function() {
				$('form_search_statusline').enable();
			}, 10);
			togglePimp(0);
			target.innerHTML = result.stripScripts();
			displayoverridden = true;
			result.evalScripts();
		} else {
			logging.warn("got result for "+mysearchwords+", but want result for "+currentsearch);
		}
	}
	
	currentsearch = searchwords;

	if (searchwords == "") {
		//back to the original page
		if (displayoverridden) {
			document.location.reload();
		}
		return;
	}
	
	if (searchwords == "SHOW_SEARCH_PAGE") {
		searchwords = "";
		currentsearch = searchwords;
	}
	if (sSearchTerm[searchwords] == "busy") {
		//we are already waiting for this search
		return;
	}
	if (sSearchTerm[searchwords] != undefined) {
		membersearchdone(searchwords, sSearchTerm[searchwords]);
		return;
	}
	
	sSearchTerm[searchwords] = "busy";
	
	var url="/search/hyver?searchterms=" + escape(searchwords) + "&fullpageAjax=1";
	var myAjax = new Ajax.Request(url, {
		method: 'get',
		onComplete: function(response) {
			var result = response.responseText;
			sSearchTerm[searchwords] = result;
			membersearchdone(searchwords, result);
			removeLoadingScreen();
		},
		onLoading: function() {
			var offset = Position.cumulativeOffset(target);
			var div = $(document.createElement('div'));
			div.id = 'loadingscreen';
			div.setStyle({
				width: target.clientWidth,
				height: target.clientHeight,
				backgroundColor: '#cfdfef',
				position: 'absolute',
				left: offset[0],
				top: offset[1],
				opacity: 0.6,
				textAlign: 'left',
				padding: '10px'
			});
			div.innerHTML = '<img src="http://' + hyves_cache_url + '/images/ajax_action_big.gif" width="32" height="32">';
			document.body.appendChild(div);
		},
		onException: function (request, exception) { logging.error("" + exception, exception); }
	});
}
doMemberSearchHelper.callcount=0;

function removeLoadingScreen() {
	if($('loadingscreen')) {
		$('loadingscreen').remove();
	} else {
		setTimeout(removeLoadingScreen, 10);
	}
}

function urchinTrackerClickHandler(event) {
	GATracker(this.getAttribute('urchinPath'));
}

function addUrchinTrackerToIds(ids) {
	ids.each(function(id){
		var ext_id = $(id);
		if(ext_id){ // need to check if they really exist
			Event.observe(ext_id, 'click', urchinTrackerClickHandler.bindAsEventListener(ext_id));
		}
	});
}

/**
 * Attaches member mouseover preview when hovering over a link to a member.
 * Takes an array of objects containing the 'elemId' and 'memberId' fields.
 */
function attachMemberPreviewListeners( previewLinks ) {
	for( var i = 0; i < previewLinks.length; i++ ) {
		var previewLink = previewLinks[i];
		new AjaxTip( (previewLink['elemId']),
					 '/index.php?l1=fr&l2=it&l3=info&xmlHttp=1&memberId='+previewLink['memberId'],
				 	 {hideOn:false, delay:0.3, hideAfter:0.25, className: 'hyvestip', effect: 'appear', hook:{target: 'bottomRight', tip:'topLeft'}}
				   );
	}
}

/**
 * Same as YAHOO util Dom.getElementsByClassName
 * but implemented on top of prototype
 */
function getElementsByClassName(className, tagName, rootNode)
{
	// don't forget the .
	var className = '.'+className;
	if(!tagName && !rootNode) {
		return $$(className); 
	}

    if(rootNode) {
        var elements  = $(rootNode).select(className);
    }
    else {
    	var elements  = $$(className);    	
    }  
   
   if(!tagName) {
   	    return elements;
   }
   else {
        //filter by tagname
        var filtered = []
        elements.each(function(element, i) {
        	if(element.tagName.toLowerCase() == tagName) {
        		filtered.push(element);
        	}
        });
        return filtered;            	
   }
}


/**
 * operates on 2 dimensional array [[x1,x2],[x3,x4]...]
 * given a list of names [n1,n2...] returns a new array of objects
 * [{n1:x1,n2:x2},{n1:x3,n2:x4}]
 */
Array.prototype.decorate = function(names)
{
	var r = [];
	var il = this.length;
	var nl = names.length;
	for(var i = 0; i < il; i++) {
		var o = {};
		for(var j = 0; j < nl; j++) {
			o[names[j]] = this[i][j];
		}
		r.push(o);
	}
	return r;
}

Array.prototype.prefix = function(p)
{
	var i = 0;
	for(i = 0; i < this.length; i++) {
		this[i] = p + this[i];
	}
	return this;
}

function isFormElement(e)
{
	try{
    	return " button textarea input select option ".indexOf(" "+ e.tagName.toLowerCase()) != -1;
	}
	catch(ex){ //if e doesn't support tagName. like if e is Document element
		return false;
	}
}

//override parseJSON to get rid of any norton inserted crap
//this caused krabbels to fail randomly
if(String.prototype.parseJSON != undefined) {
    String.prototype._parseJSON = String.prototype.parseJSON;
    String.prototype.parseJSON = function () {
        return Hyves.stripNorton(this)._parseJSON();
    }
}

/**
 * Called from the dart iframe child (dartproxy.html)
 **/
 
function dartcallback(command, id) {
	if (command == 'has_ad') {
		var idregex = /^[0-9]+$/
		if (! id.match(idregex)) {
			return false;
		}
		var el;
		if (el = $('dartholder_'+id)) {
			el.show();
			fixFireFoxRenderBug();
		}
	} 
}

/**
 * formats an amount in cents
 */
function formatAmountInCents(amountInCents) {
	amount = String(amountInCents/100);
	return amount.replace('.', ',');
}

defer = function (myobject, myfunc, myargs, mycallback) {
	for (var i=0;i<myargs.length;i++) {
		if (myargs[i] == defer.CALLBACK) {
			myargs[i] = mycallback
		}
	}
	return myfunc.apply(myobject, myargs)
}
/**
 * Needed because IE thinks setTimeout is not a regular function :(
 **/
hyvesSetTimeout = function(callback, timeout) {
	return window.setTimeout(callback, timeout);
}

defer.CALLBACK = "Some UNIQUE callback String TYERWFVD75%G"

/**
 * helper function for createing square imgs (crops and scales to size)
 */
function sqi(the_img, size)
{
	var width = the_img.width;
	var height = the_img.height;
	if(width > height) { 
		//landscape
		the_img.style.height = size + 'px';
		margin = -Math.floor((((size / height) * width) - size) / 2.0);
		the_img.style.margin = '0px 0 0 ' + margin + 'px';
	}
	else {
		//portrait
		the_img.style.width = size + 'px';
		margin = -Math.floor((((size / width) * height) - size) / 2.0);
		the_img.style.margin = margin + 'px 0 0 0';
	}
}

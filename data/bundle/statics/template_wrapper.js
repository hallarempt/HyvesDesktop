/**
* A wrapper class around jst templates
* (http://trimpath.com/project/wiki/JavaScriptTemplates)
* the wrapper includes special methods for quoting xml correctly and hyves fancy layout
* requires fancy_layout.js and header.js
*/

//global translation dict, is added to by jstitem tag
var _translation = {};
var _constant = {};

function _translate(key, data) { 
	if(_translation[key] != undefined) {
		if(typeof(_translation[key]) == 'string') {
			return _translation[key];
		}
		else {
			return _translation[key].process(data);
		}
	}
	else  {
		return '@' + key + '@';
	}
}

translateModifier = function(key)
{
	data = {};
	for(var i = 1; i < arguments.length; i+=2) {
		var arg = arguments[i];
		var val = arguments[i+1];
		data[arg] = val;
	}
	return _translate(key, data);
}

truncateModifier = function(s, len)
{
	if(s.length > len) {
		return s.substring(0, len) + "...";
	}
	else {
		return s;
	}
}

window.softHyphenRegExp = [];
softHyphen = function(string, length) {
	
	if (!window.softHyphenRegExp[length]) {
		window.softHyphenRegExp[length] = new RegExp('([^\\s<>&]{' + length + '})([^\\s<>])(?![^<]*>)', 'g');
	}
	return string.replace(window.softHyphenRegExp[length], '$1&shy;$2');
}

var fancyLayoutCache = new Object();

fancyLayout = function(s, type)
{
	var noCache = false;
	var key = "";
	for(var i = 0; i < arguments.length; i++) {
		if(arguments[i]) {
			key += arguments[i].toString();
		}
	}

    if(startsWith(s, '[gadget]')) { //skip cache
        noCache = true;
    }
    
	
	if(noCache || !fancyLayoutCache[key]) {
		if(type) {
			type = type.replace("!", "|");
		}
		fancyLayoutCache[key] = fancy_layout(s, type);
	}
		
	return fancyLayoutCache[key];
}

function _Template(name, code) {
	this.name = name;
	this.code = code;
	this.template = null;
}

_Template.prototype._MODIFIERS = null;

_Template.prototype.process = function(data)
{
	if(this.template == null) {
		try {
			this.template = TrimPath.parseTemplate(this.code);
		}
		catch(e) {
			logging.error("error while parsing template:" + this.name + ": " + e, e);
			return "";
		}
	}

	if(data == undefined) {
		data = {}
	}

	if(!this._MODIFIERS) {
		this._MODIFIERS = {
			quoteElement: quoteElement,
			quoteAttr: quoteAttr,
			fancyLayout: fancyLayout,
			fancy_layout: fancyLayout,
			softHyphen: softHyphen,
			translate: translateModifier,
			truncate: truncateModifier
		}
	}
	data._MODIFIERS = this._MODIFIERS;
	
	data.throwExceptions = true;
	try {
		return this.template.process(data, data);
	}
	catch(e) {
		logging.error("error while processing template:" + this.name + ": " + e, e);
		return "";
	}
}


var templateCache = new Object();
/**
 * do not use this method it is prives, use the namespace below
 */
function _getTemplate(id)
{
	if(!templateCache[id]) {
		templateCache[id] = new _Template(id, id);
	}
	return templateCache[id];
}

/**
 * if delay is true, delays execution of func until target exists in DOM
 * checks twice for target, once after 10ms and if not exist
 * a second time after 200ms
 */
function _delay(delay, target, func)
{
	if(delay) {
		setTimeout(function() {
			if($(target)) {
				func();
			}
			else {
				setTimeout(function() {
					if($(target)) {
						func();
					}
				}, 200);
			}
		}, 10);
	}
	else {
		func();
	}
}

/**
 * Please use only function from the following namespace:
 */
templates = {
	get: function(id)
	{
		return _getTemplate(id);
	},
	addFromString: function(id, content)
	{
		templateCache[id] = new _Template(id, content);
	},
	addTranslation: function(key, content) {
		_translation[key] = new _Template(key, content);
	},
	addConstant: function(key, content) {
		_constant[key] = content;
	},
	/**
	 * updates the given target DOM element by executing given
	 * template on given data. 
	 * If delay is true, the updating will wait until element target
	 * is available in DOM. optionally func will be called at that time
	 */
	updateElement: function(target, template, data, delay, func) {
		_delay(delay, target, function() {
			try {
				$(target).innerHTML = templates.get(template).process(data);
				if(func) {
					func();
				}
			}
			catch(e) {
				logging.error('error processing template ' + template + ': ' + e, e);
			}
		});
	},
	insertBottom: function(target, template, data) {
		new Insertion.Bottom(target, templates.get(template).process(data));
	},
	insertTop: function(target, template, data) {
		new Insertion.Top(target, templates.get(template).process(data));
	},
	insertBefore: function(target, template, data) {
		new Insertion.Before(target, templates.get(template).process(data));
	},
	process: function(template, data) {
		return templates.get(template).process(data);
	},
	translate: function(key, data) {
		return _translate(key, data);
	},
	exists: function(key) {
		if(templateCache[key]) {
			return true;
		}
		else {
			return false;
		}
	},
	constant: function(key) {
		if(_constant[key] != undefined) {
			return _constant[key];
		}
		else  {
			return '#' + key + '#';
		}
	},
	//recompile/reload a template (development only)
	reload: function(id) {
		var url = "/"
		var pars = "module=Utility&action=compileTemplate&id=" + id;
	
		var myAjax = new Ajax.Request(url, 
		{
			asynchronous: false,
			method: 'get', 
			parameters: pars,
			onSuccess: function(transport) {
				eval(transport.responseText);
			}
		});
	}
}







/**
 * Translates the given @p string.
 */
function tr(string) {
	
	if (!window.translator) {
		return string;
	}
	
	return window.translator.tr(string);
}

function trHtml(string) {
	
	if (!window.translator) {
		return string;
	}
	
	return window.translator.trHtml(string);
}

function trHtmlInsert(string, obj) {
	
	if (!window.translator) {
		return;
	}
	
	eval(window.translator.trHtmlJs(string, "obj"));
}

function trAccessKey(string) {
	
	if (!window.translator) {
		return "";
	}
	
	return window.translator.trAccessKey(string);
}

function trInsert(string, htmlParent, accessKeyObject) {
	
	trHtmlInsert(string, htmlParent);
	if (accessKeyObject) {
		accessKeyObject.accessKey = trAccessKey(string);
	}
}

/**
 * Replaces the lowest numbered occurence of %1, %2, etc... in the string with
 * the given @p argument.
 */
String.prototype.arg = function(argument) {
	
	var num = 1;
	while (num < 10) {
		if (this.indexOf('%' + num) > -1) {
			return this.replace(new RegExp('%' + num, 'g'), argument);
		}
		num++;
	}
}

/**
 * Translates all strings in the login form since the form cannot know which
 * language we are using.
 */
function translateLoginForm() {
	
	try {
		$('loginLabel').innerHTML = tr('Log in');
		$('usernameLabel').innerHTML = tr('Username:');
		$('passwordLabel').innerHTML = tr('Password:');
		$('myOnlineStatusLabel').innerHTML = tr('My online status:');
		$('rememberLabel').innerHTML = tr('Remember');
		
		$('login_button').value = tr('Login');
		
		$('onlineOption').innerHTML = tr('Online');
		$('busyOption').innerHTML = tr('Busy');
		$('berightbackOption').innerHTML = tr('Be Right Back');
		$('awayOption').innerHTML = tr('Away');
		$('phoneOption').innerHTML = tr('In a call');
		$('lunchOption').innerHTML = tr('Out to Lunch');
		$('offlineOption').innerHTML = tr('Show Offline');
	} catch(e) {
		logging.debug('Could not translate login form: ' + e, e);
	}
}

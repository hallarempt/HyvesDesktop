
//constructor for the hyvesense
function Hyvesense2() {
}

//start the banner selection process by loading a schipt from the hyvesense servers
Hyvesense2.runBannerSelection = function(server, zone, targets, memberId, unique) {
		
	window.hyveSenseUrl = 'http://' + server + '/view?zone=' + zone + '&memberId=' + memberId + '&unique=' + unique + '&' + $H(targets).toQueryString();
	dhtmlLoadScript(window.hyveSenseUrl);
}

Hyvesense2.clearBanner = function(unique, pars) {
	if ($('hyvesense_' + unique)) {
		$('hyvesense_' + unique).innerHTML = '';
	}
}	


Hyvesense2.showBannerFromObjectIdAndType = function(unique, objectId, type) {
	
	var url = '/';
	var pars = 'module=hyveSense2&action=showBannerFromObjectIdAndType&objectId=' + objectId + '&objectType=' + type;
	
		
	new Ajax.Updater( 'hyvesense_' + unique, url, 
			{
				evalScripts: true,
				method: 'get' ,
				parameters: pars
			}); 
}

Hyvesense2.showBannerFromId = function(unique, targetId, type, t_s) {
	
	var url = '/';
	var pars = 'module=hyveSense2&action=showBannerFromId&id=' + targetId + '&type=' + type + '&s=' + t_s;
	
		
	new Ajax.Updater( 'hyvesense_' + unique, url, 
			{
				evalScripts: true,
				method: 'get' ,
				parameters: pars
			}); 
}

Hyvesense2.showBannerFromData = function(unique, type, pars, onComplete) {
	
	pars['type'] = type;
    var url = '?module=hyveSense2&action=showBannerFromData';
	
	new Ajax.Updater( 'hyvesense_' + unique, url, 
			{
				evalScripts: true,
				method: 'get' ,
				parameters: pars,
				onComplete: function(transport) {
						if (onComplete) {
							onComplete();	
						}
					}
			}); 
}

Hyvesense2.setBannerTitleListener = function() {
	var objectBannerTitleElement = $('objectBannerTitle');
	if (objectBannerTitleElement) {
		
		var replaceFunction = function() {objectBannerTitleElement.innerHTML = $F('idCreateTitle').replace('{$name}', global_member_nickname);};
		replaceFunction();
		
		Event.observe('idCreateTitle', 'keyup', function(e){ 
			replaceFunction();	
		});
	}
}






if(window.jsloaded) {jsloaded('/statics/hyvesense/hyvesense2.js')}



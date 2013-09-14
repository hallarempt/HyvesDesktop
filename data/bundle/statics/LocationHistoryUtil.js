function changeVisibility(id, visibility, postman, secret){
	var url = '/index.php';
	var pars = 'visibility='+visibility+'&id='+id+'&'+postman+'&edit_secret='+secret;
	var selector = $('v_'+id);
	var feedback = $('f_'+id);
	if(!feedback){
		var feedbackBox = '<span id="f_'+id+'"></span>';
		new Insertion.After(selector, feedbackBox);
		feedback = $('f_'+id);
	}
	
	var myAjax = new Ajax.Request(
		url,
		{
			method: 'get',
			parameters: pars,
			evalScripts: true,
			onComplete: function(response){
				result = response.responseText.parseJSON();
				feedback.innerHTML = result.content;
				if(result.success){
					if(window.invalidatePagers){
						invalidatePagers();
					}
				}
			},
			onLoading: function(){
				feedback.innerHTML = '<img src="http://'+hyves_cache_url+'/images/ajax_action.gif">';				
			}
		}
	);
}

function deleteLocationHistory(id, postman, secret){
	var url = '/index.php';
	var pars = 'id='+id+'&'+postman+'&delete_secret='+secret;
	var feedback = $('fd_'+id);
	if(!feedback){
		var feedbackBox = '<span id="fd_'+id+'"></span>';
		new Insertion.Bottom($('d_'+id), feedbackBox);
		feedback = $('fd_'+id);
	}	
	var myAjax = new Ajax.Request(
		url,
		{
			method: 'get',
			parameters: pars,
			evalScripts: true,
			onComplete: function(response){
				result = response.responseText.parseJSON();
				if(result.success){
					deleteElement = $('l_'+id);
					new Effect.Fade(deleteElement);
					if(window.invalidatePagers){
						invalidatePagers();
					}
				}
				else{
					feedback.innerHTML = "Failed";
				}
			},
			onLoading: function(){
				feedback.innerHTML = '<img src="http://'+hyves_cache_url+'/images/ajax_action.gif">';
			}
		}
	);		
}
var LocationHistoryCompleterModule = Class.create({
	requires:  ['/LocationHistory/locationhistory_client_popup.tpl'],
	
	getInstance: function() {	
		return new LocationHistoryCompleter();	
	}
});

var LocationHistoryCompleter = Class.create({
	
	initialize: function()  {
		this.locationSelected = new Hyves.CustomEvent('locationHistoryCompleter:locationSelected', this);
		this.pulldownMenuShown = new Hyves.CustomEvent('locationHistoryCompleter:pulldownMenuShown', this);
		this.pulldownMenuHidden = new Hyves.CustomEvent('locationHistoryCompleter:pulldownMenuHidden', this);
		this.url = '/index.php?module=LocationHistory&action=getHubsAndSpotsJSON';
		this.value = '';
		this.cache = {};
		this.matches = [];
		this.firstrequest = true;
		this.pulldown = false;
		this.store = false;
		this.input = false;
		this.addToHub = false;
		this.log = logging.getLogger("LocationHistory");
		this.selectedIndex = -1;
	}, 
	init: function(hiddenformfield, inputfield, addToHubField) {
		var tempDiv = Builder.node('div');
		tempDiv = $(tempDiv);
		tempDiv.update(templates.process("locationhistory.popupContainer", {}));
		this.pulldown = tempDiv.childNodes[0];
		document.body.appendChild(tempDiv);
		this.store = $(hiddenformfield);
		this.input = $(inputfield);
		this.addToHub = addToHubField;
		Event.observe(this.pulldown, 'click', this.select.bindAsEventListener(this));
		Event.observe(this.pulldown, 'mouseover', this.mouseover.bindAsEventListener(this));
		Event.observe(this.input, 'keyup', this.fire.bindAsEventListener(this));  // Not a nice fix, but until we make a patch for control.js
		Event.observe(this.input, 'keydown', this.fireDown.bindAsEventListener(this));
		Event.observe(this.input, 'blur', this.onblur.bindAsEventListener(this)); // superceding pulldown click event
		Event.observe(this.input, 'click', this.fire.bindAsEventListener(this));
	},
	fire: function(event) {
		if(!this.specialkeys(event)){
			return;
		}
		this.store.value = '';
		var element = Event.element(event);
		this.value = element.value.toLowerCase();
		this.search();
	},
	fireDown: function(event) {
		switch(event.keyCode) {
			case Event.KEY_RETURN:
				if (this.selectedIndex >= 0) {
					this.setValue($(this.matches[this.selectedIndex]));
					this.finish();
					Event.stop(event);
				}
			break;
		}
	},
	mouseover: function(event) {
		var element = Event.findElement(event, 'div');
		if (element.id) {
			var index = this.matches.indexOf(element.id);
			if (index != -1) {
				this.setSelectedIndex(index);
			}
		}
	},
	specialkeys: function(event){
		switch(event.keyCode){
			case Event.KEY_ESC:
				this.finish();
				return false;
			break;
			
			case Event.KEY_UP:
				if (this.matches.length != 0) {
					this.setSelectedIndex(this.selectedIndex-1);
					return false;
				}
			break;
			
			case Event.KEY_DOWN:
				if (this.matches.length != 0) {
					this.setSelectedIndex(this.selectedIndex+1);
					return false;
				}
			break;
			
			case Event.KEY_RETURN:
				return false;
			break;
			
			default:
				return true;
		}		
	},
	search: function(){
		/*if(this.value.length == 0){
			this.finish();
			return;
		}*/
		var self = this;
		/*
		if($H(this.cache).values().size() == 0 && this.firstrequest){
			this.request();
			return;
		}
		this.matches = [];
		$H(this.cache).each(function(entry){
			if(entry.value.toLowerCase().indexOf(self.value) > -1){
				self.matches.push(entry.key);
			}
		});
		*/
		this.matchText(false);
		if(this.matches.length > 0){
			this.clearSelection();
			this.display();
		}
		else{
			this.finish();
		}
	},
	matchText: function(bMatchExact){
		var self = this;
		if($H(this.cache).values().size() == 0 && this.firstrequest){
			this.request();
			return;
		}
		
		this.matches = [];
		
		if (bMatchExact){
			$H(this.cache).each(function(entry){
				if(entry.value.name.toLowerCase() == self.value){
					self.matches.push(entry.key);
				}
			});
		}else{
			$H(this.cache).each(function(entry){
				if(entry.value.name.toLowerCase().indexOf(self.value) > -1){
					self.matches.push(entry.key);
				}
			});
		}
	},
	request: function(){
		this.firstrequest = false;
		var self = this;
		var myAjax = new Ajax.Request(self.url, 
			{
				method: 'get', 
				parameters: {},			
				onSuccess: function(response) {
					result = response.responseText.parseJSON();
					if(!result.success){
						this.log.debug("could not retrieve favorite locations");
					}
					else {
						if(Object.isArray(result.locations)) {
							//this happens when locations is empty
							self.cache = {};
						}
						else {
							//this should be an hash(object)
							self.cache = result.locations;
						}
						self.search();
					}
				}
			});		
	},
	display: function(){
		var self = this;
		var favoriteLocations = [];
		var spots = [];
		var hubs = [];
		var favoriteLocationIds = [];
		var spotIds = [];
		var hubIds = [];
		var maxResultPerCategory = 5;
		this.matches.each(function(match){
			if (self.cache[match].type === 3) {
				if(hubs.length < maxResultPerCategory) {
					hubIds.push(match);
					hubs.push(self.cache[match]);
				}
			}
			else if (self.cache[match].type === 2) {
				if(spots.length < maxResultPerCategory) {
					spotIds.push(match);
					spots.push(self.cache[match]);
				}
			}
			else if (self.cache[match].type === 1) {
				if(favoriteLocations.length < maxResultPerCategory) {
					favoriteLocationIds.push(match);
					favoriteLocations.push(self.cache[match]);
				}
			}
		});
		this.matches = favoriteLocationIds.concat(spotIds, hubIds);
		this.pulldown.innerHTML = templates.process("locationhistory.menuItems", {'favoriteLocations' : favoriteLocations, 'hangouts' : spots, 'otherHubs' : hubs });
		if(!this.pulldown.visible()) {
			this.pulldown.style.zIndex = 3000; //make sure pulldown is visible even in popupdialogs
		}
		this.updatePosition();
		this.pulldown.show();
		this.pulldownMenuShown.fire();
	},
	clearSelection: function() {
		this.selectedIndex = -1;
	},
	setSelectedIndex: function(index) {
		
		var oldDiv = (this.selectedIndex >= 0) ? $(this.matches[this.selectedIndex]) : null;
		if (oldDiv) {
			oldDiv.style.background = '#EBF2FA';
		}
		
		if (index < 0) {
			// do nothing
		}
		else if (index < this.matches.length) {
			this.selectedIndex = index;
		}
		else{
			this.selectedIndex = this.matches.length - 1;
		}
		
		var newDiv = $(this.matches[this.selectedIndex]);
		if (newDiv) {
			newDiv.style.background = '#CFDFEF';
		}
	},
	finish: function(){
		this.pulldown.hide();
		this.pulldownMenuHidden.fire();
	},
	onblur: function(event){
		//console.log(event);
		var self = this;
		cTimeout = setTimeout(function() {
			if (self.selectedIndex !== -1) {
				var selectedItem = $(self.matches[self.selectedIndex]);
				self.setValue(selectedItem);
				self.finish();
			}
			else {
				self.typed();
			} 
		}, 300);
	},
	typed: function(){
		//console.log("typed");
		if(this.value.length == 0){
			this.finish();
			return;
		}
		
		var self = this;
		self.matchText(true);
		
		if (self.matches[0]){
			this.setValue($(self.matches[0])); // picking up the first element; in case of duplicated entries
		}

		this.finish();
		return;
	},
	select: function(event){
		//console.log(event);
		clearTimeout(cTimeout);
		this.setValue(Event.element(event));
		this.finish();
	},
	/**
	 * sets the selected value in the input field.
	 * @param element - div element representing menuitem. It may have one child element as data or span followed by data.
	 * 					div.childNodes[span, data] or div.childNodes[data] span node repesents favorite locations by private
	 */
	setValue: function(element){
		if (!element) {
			return;
		}
		
		if (element.id) {
			this.log.debug('element id: ' + element.id);
			
			if (this.addToHub !== undefined) {
				if (element.id.startsWith('56_')) {
					$(this.addToHub).hide();
				}
				else {
					$(this.addToHub).show();
				}
			}
			
			var text = element.childNodes[0].nodeName == "SPAN" ? element.childNodes[1].data : element.childNodes[0].data;
			this.input.value = text; 
			this.store.value = element.id;
			
			this.locationSelected.fire({value: element.id, name: text});
		}
	},
	/**
	 * moves the pulldown menu to the right of the input box
	 * resizing: menu expands to the top until no more space is left
	 *           it then continues expanding towards the bottom of the page
	 *           (wwwDialog.js takes care about resizing the page for really large menues)
	 */
	updatePosition: function() {
		var self = this;
		var avartarImage = $("avatarImage");
		var menuOffsetLeft = self.input.getDimensions().width;
		// vertically align menu with the inputbox and don't expand beyond the top of the avatar image (suggested by Hanno)
		var menuOffsetTop = self.input.getDimensions().height - self.pulldown.getDimensions().height;
		if ((self.input.offsetTop - avartarImage.offsetTop + menuOffsetTop) < 0) {
			menuOffsetTop = avartarImage.offsetTop - self.input.offsetTop;
		}

		self.pulldown.clonePosition(self.input, {setLeft: true, setTop: true, setWidth: true, setHeight: false, offsetTop: menuOffsetTop, offsetLeft: menuOffsetLeft});
	},

	getMapForLocation: function(whereId, callback) {	
		new Ajax.Request('/index.php?module=LocationHistory&action=getGoogleMapForLocation', {
			method: 'get',
			parameters: {whereId: whereId},
			onSuccess: function(response) {
				callback(response.responseText);
			}
		});
	}
});


/**
 * A function to show a www input form dynamically on any page
 * use the callback to get the new www when the form is (ajax)posted
 */
function showLocationHistoryFormPopup(id, title, callback, url) {
	
	//a function attached to the submit handler
	//on the www form, turns normal submit into an ajax submit
	var rte_name = 'locationhistory_form_' + id;
	var form_id = 'locationhistory_form_' + id;
	
	var attachSubmitHandler = function() {
		
		dialog = popupDialogManager.findDialog(id);
		
		$(form_id).observe('submit', function(event) {
			Event.stop(event);//prevent normal submit
			dialog.setBusy(true); //instead do an ajax submit
			RichTextEditor.getInstance(rte_name).hidePreview();

			//save the www, we will inform callback later
			var www = {
				wwwEmotion: $F('richtextcontrol_' + rte_name + '_code'),
				wwwLocation: $F('www_box_' + rte_name),
				wwwTime: new Date()
			}
			
			//do the ajax submit
			window.aError = false; //will be set if form post fails
			
			var parms = $(form_id).serialize(true);
			var querystring = '';
			$H(parms).each(function (parm) {
				querystring += parm.key  + '=' + escapePlus(parm.value) + '&'
			});
			
			new Ajax.Request($(form_id).action, {
				
				postBody: querystring,
				encoding: 'ISO-8859-1',
				onComplete: function(request) {
					dialog.setBusy(false);
					dialog.updateBody(request.responseText);
					//make sure that we again override the normal submit logic
					attachSubmitHandler();
					//wait a bit because otherwise window.aError is not available
					setTimeout(function() {
						if(!window.aError) {						
							if(callback) {
								//inform callback of the www update
								callback(www);
							}
							setTimeout(function() {
								dialog.close();
								if(url) {
									window.location = url;
								}
							}, 2500);
						}
					}, 10);
				}
			});			
		});
	}

	//show popup with www form
	showPopupDialogFromUrl(id, title, 
						  '/index.php?module=LocationHistory&action=showLocationHistoryFormPopup&id=' + id, {
		modal: false,
		onComplete: function(request) {
			attachSubmitHandler();
		},
		onClose: function(dialog) {
			RichTextEditor.getInstance(rte_name).hidePreview();
			var locationHistoryPopup = window._moduleInstances['LocationHistoryCompleter']; 
			if (locationHistoryPopup) {
				locationHistoryPopup.finish();
			}
		}		
    });
}

if(window.jsloaded) {jsloaded('/statics/LocationHistoryUtil.js')}


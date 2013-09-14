	//smilie picker
aSmilieCategoryName = {};

aCategory = {'love': 1, 'happy': 2, 'angry': 3, 'cool': 4, 'party': 5,
	   	     'confused': 6, 'types': 7, 'activity': 8, 'general': 9, 'tongue': 10, 'animal': 11};

aCategoryOrder = ['general', 'happy', 'angry', 'cool', 'party', 'love',
	   	     	  'confused', 'types', 'activity', 'tongue', 'animal'];
				
aFastInit = ["(h)*bril", "(6)*duivels", ";-)*knipoog", ":-$*schamen", ":'(*cry", ":)*lach", ":-(*verdrietig", 
			 ":-@*boos", ":-d*groot", ":-o*surprised", ":-p*tong2", ":s*indewar", ":-|*geschokt", "^o)*neutraal"];

$H(aCategory).each(function(item) {
	aSmilieCategoryName[item.value] = templates.translate('CONTROL_RICHTEXTEDIT_' + item.key.toUpperCase());
});

function RTE_SmiliePickerController() {
}

RTE_SmiliePickerController.prototype.init = function() {
	this.firstLoad = true;
	
	this.SMILEY_ALL = 'all';
	this.SMILEY_FIRST_LOAD = 'first';
	this.SMILEY_TYPE_GENERAL = 'general';
	
	this.eltPlane = $("smiliepicker_" + this.name + "_pickplane");
	
	//listen for clicks on smilies and or optins
	var self = this;
	Event.observe(this.eltPlane, "click", function(event) {
		var target = $(event.target);
		if(target) {
			if(target.hasClassName('rte_smilie')) {
				fancylayout_inserttext(escape_html_decode(event.target.title.replace("'","\'")), 'richtextcontrol_' + self.name);
				
				var sendButton = $('uiButtonSend');
				if (sendButton) {
					sendButton.removeClassName('disabled');
				}
				
				event.stop();
			}
			else if(target.hasClassName('rte_smilie_optin')) {
				self.openEnableOptinWindow();
				event.stop();
			}
			else if(target.hasClassName('rte_smilie_extra_premium')) {
				window.open(templates.process('template.rte.smiley.premium_enable_url'), "hyves_premium_window");
			}
		}
	});

	//listen for clicks on smiley categories
	var eltCategoryPicker = $("smiliepicker_" + this.name + "_category_picker");
	Event.observe(eltCategoryPicker, "click", function(event) {
		var target = $(event.target);
		if(target && target.hasClassName('rte_smilie_category')) {
			var category = target.id.split('_').last();
			if(category) {
				self.showSmilies(category);
			}
			event.stop();
		}		
	});
}

RTE_SmiliePickerController.prototype.load = function() {
	if (this.firstLoad == true) {
		this.showSmilies(this.SMILEY_FIRST_LOAD);	
	}
		
	this.firstLoad = false;
}

RTE_SmiliePickerController.prototype.checkEnableOptin = function() {
	optin = false;
	for(i in this.smileyPacks)
	{
		if (this.smileyPacks[i] == SMILEY_EXTRA_OPTIN)
		{
			optin = true;
		}
	}
	return optin;
}

RTE_SmiliePickerController.prototype.checkPremium = function() {
	premium = false;
	for(i in this.smileyPacks)
	{
		if (this.smileyPacks[i] == SMILEY_EXTRA_PREMIUM)
		{
			premium = true;
		}
	}
	return premium;
}

RTE_SmiliePickerController.prototype.showSmilies = function(type) {
	
    this.eltPlane.update("");
    
	if (type == this.SMILEY_ALL)
	{
		if (!this.checkEnableOptin())
		{
			this.eltPlane.insert(this.getExtraSmileyAdvertisement());
		}
		
		var self = this;
		aCategoryOrder.each(function(category, i) {
			self.eltPlane.insert(self.getSmilieDom(aCategory[category]));	
		});
	}
    else if (type == this.SMILEY_FIRST_LOAD) {
    	//fast init of initial smiley page, fast because we don't init the smiletree
    	var category = 9;
		var args = {'rte_name': this.name,
					'type': category};
    	
    	var smileysHtml = [];
    	aFastInit.each(function(namecode, i) {
			namecode = namecode.split('*');
			args['id'] = i;
			args['name'] = namecode[1];
			args['title'] = namecode[0];
    		smileysHtml.push(templates.process("template.rte.smiley", args));
    	});
    	
		var html = templates.process('template.rte.smileys', {
			category: aSmilieCategoryName[category],
			smileys: smileysHtml.join('')
		});
    	
    	this.eltPlane.insert(html);
    }
	else {
		this.eltPlane.insert(this.getSmilieDom(aCategory[type]));
    }
    
    if (type != this.SMILEY_ALL) {
		
		if (!this.checkPremium())
		{
			this.eltPlane.insert(this.getPremiumSmileyAdvertisement());
		}
		if (!this.checkEnableOptin())
		{
			this.eltPlane.insert(this.getExtraSmileyAdvertisement());
		}
	}
}
	
RTE_SmiliePickerController.prototype.getSmilieDom = function(type) {
	// main span
	
	var oaoSmilie = SmileyUtil.createSmiliesArray();
	var args = {'rte_name': this.name,
				'type': type};
	
	var smileysHtml = [];
	
	for (i_type = this.smileyPacks.length-1; i_type >= 0; i_type--)
	{
		extra_type = this.smileyPacks[i_type];
		for (i = 0; i < oaoSmilie[type].length; i++)
		{
			smilie = oaoSmilie[type][i];
			if (smilie.getExtra() == extra_type)
			{
				args['id'] = i;
				args['title'] = smilie.code;
				args['name'] = smilie.name;
				smileysHtml.push(templates.process("template.rte.smiley", args));
			}
		}
	}
	
	var html = templates.process('template.rte.smileys', {
		category: aSmilieCategoryName[type],
		smileys: smileysHtml.join('')
	});

	return html;
}

RTE_SmiliePickerController.prototype.getExtraSmileyAdvertisement = function() 
{
	return templates.process('template.rte.smiley_optin');
}

RTE_SmiliePickerController.prototype.getPremiumSmileyAdvertisement = function() 
{
	return templates.process('template.rte.smiley_extra_premium');
}


RTE_SmiliePickerController.prototype.openEnableOptinWindow = function() {
	if (this.oRTESmileyOptinCon)
	{
		this.oRTESmileyOptinCon.openWindow();
	}
	// Preview is always on top, prevent overlapping by hiding it.
	RichTextEditor.getInstance(this.name).hidePreview();
}


function RTE_EnableSmileyOptInController() {
}

RTE_EnableSmileyOptInController.prototype.init = function() {
	this.domDiv = $("rte_smiley_extra_div_" + this.name);
	this.domIFrame = $("rte_smiley_extra_iframe_" + this.name);
	this.domClosebutton = $("rte_smiley_extra_closebutton_" + this.name);

	// adds all events
	this.EventHandler = new RTE_EnableSmileyOptInEventHandler(this);    
}

RTE_EnableSmileyOptInController.prototype.openWindow = function() {
	var url = "/index.php";
	var pars = "xmlHttp=1&module=Smiley&action=showRichTextEditEnableSmileyOptIn&name=" + this.name;
	var myAjax = new Ajax.Request(
								url,
								{
									parameters: pars,
									onComplete: function(originalRequest){ 
										reply = originalRequest.responseText;
										popupMessage( reply );
										}
								}
								);	
}

RTE_EnableSmileyOptInController.prototype.closeWindow = function() {
	this.domDiv.hide();
}

RTE_EnableSmileyOptInController.prototype.enableOptIn = function() {
	this.richTextEditor.changeSmileyPacks([SMILEY_EXTRA_OPTIN, SMILEY_EXTRA_DEFAULT]);
}

function RTE_EnableSmileyOptInEventHandler(Controller) {
	this.Controller = Controller;
	this.init();
}


RTE_EnableSmileyOptInEventHandler.prototype.init = function() {
	EventHandler.getInstance().addFakeEvent("rte_smiley_extra_optinenabled_event_" + this.Controller.name, this, "eventOptInEnabled");
	EventHandler.getInstance().addFakeEvent("rte_smiley_extra_optinenabled_event_close_" + this.Controller.name, this, "eventCloseWindow");
	EventHandler.getInstance().addOnClickEvent(this.Controller.domClosebutton, this, "eventCloseWindow");
}


RTE_EnableSmileyOptInEventHandler.prototype.eventOptInEnabled = function(e, params) {
	this.Controller.enableOptIn();
}

RTE_EnableSmileyOptInEventHandler.prototype.eventCloseWindow = function(e, params) {
	this.Controller.closeWindow();
}


if(window.jsloaded) {jsloaded('/statics/controls/RTE_Smileypicker.js')}


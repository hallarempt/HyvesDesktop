/*	RichTextEditor
*
*	The following properties need to be set:
*
*	RichTextEditor.name						// name of the richtextedit
*	RichTextEditor.open_picker				// defaulttab to open (e.g. smilies)
*	RichTextEditor.fancy_layout_type		// fancylayout type of richtextedit
*	RichTextEditor.mediasize				// the image size
*	RichTextEditor.text_RICHTEXT_LINKTO		// text element for links
*	RichTextEditor.text_SMILEY_EXTRA_ERROR_ENABLE_OPTIN_FIRST
*	RichTextEditor.text_SMILEY_EXTRA_ERROR_NOT_PREMIUM
*
*	[ ] = optional
*/
var _richtexteditors = {}; //map of rte instances keyed by name

function RichTextModule()
{
    //just a stub for module system
}

RichTextModule.prototype.requires = ['/rte/rteTemplates.tpl'];

RichTextModule.RTE_DEFAULT_CONFIG = {
    name: 'RT_EDIT_NAME',
    open_picker: 'RT_EDIT_OPEN_PICKER',
    fancy_layout_type: 'RT_EDIT_FANCY_LAYOUT_TYPE',
    fancy_layout_replace_name: 'RT_EDIT_FANCY_LAYOUT_REPLACE_NAME',
    mediasize: 'RT_EDIT_MEDIASIZE',
    showpreview: 'RT_EDIT_SHOWPREVIEW',
    previewWidth: 'RT_EDIT_PREVIEW_WIDTH',
    previewElement: 'RT_EDIT_PREVIEWELEMENT',
    picker_leftmenu_width: 'RT_EDIT_PICKER_LEFTMENU_WIDTH',
    picker_height: 'RT_EDIT_PICKER_HEIGHT',
    picker_width: 'RT_EDIT_PICKER_WIDTH',
    textarea_width: 'RT_EDIT_TEXTAREA_WIDTH',
    textarea_height: 'RT_EDIT_TEXTAREA_HEIGHT',
    textarea_name: 'RT_EDIT_TEXTAREA_NAME',
    showcontrolsonstart: 'RT_EDIT_SHOWCONTROLSONSTART',
    submit_value: 'RT_EDIT_SUBMIT_VALUE',
    submit_name: 'RT_EDIT_SUBMIT_NAME',
    content: 'RT_EDIT_CONTENT',
    allowlinks: 'RT_EDIT_ALLOWLINKS',
    allowgadgets: 'RT_EDIT_ALLOWGADGETS',
    allowmedia: 'RT_EDIT_ALLOWMEDIA'
};
   
/**
*	Constructor
*
*	Assign the sub-controllers.
**/
function RichTextEditor(config) {
    
    //updateing config defaults
    $H(RichTextModule.RTE_DEFAULT_CONFIG).each(function(item) {
	    if(config[item[0]] == undefined) {
	        config[item[0]] = templates.constant(item[1]);
	    }
    });
        
    this.name = config.name;
    this.open_picker = config.open_picker;
    this.fancy_layout_type = config.fancy_layout_type;
    this.fancy_layout_replace_name = config.fancy_layout_replace_name;
    this.mediasize = config.mediasize;
    this.showpreview = config.showpreview;
    this.previewWidth = config.previewWidth;
    this.previewElement = config.previewElement;
    this.picker_height = config.picker_height;
    this.picker_width = config.picker_width;
    this.picker_leftmenu_width = config.picker_leftmenu_width;
    this.smileyPacks = global_member_smileypack;

	
	
    this.config = config;
	
    this.currentTab = "";
	this.previousTab = "";
	
	this.oRTESmilieCon = null;
	this.oRTEMediaCon = null;
	this.oRTEWidgetCon = null;
	
	this.oRTEMemberCon = null;
	
	this.text_SMILEY_EXTRA_ERROR_ENABLE_OPTIN_FIRST = templates.translate('SMILEY_EXTRA_ERROR_ENABLE_OPTIN_FIRST'); 
	this.text_SMILEY_EXTRA_ERROR_NOT_PREMIUM = templates.translate('SMILEY_EXTRA_ERROR_NOT_PREMIUM'); 
	
	this.MEDIA_SIZE_ICON_SMALL =  templates.constant('MEDIA_SIZE_ICON_SMALL');
	
    _richtexteditors[this.name] = this;

    this.checkSmileyRightsTimeout;
}

RichTextEditor.prototype.write = function(divId)
{
    templates.updateElement(divId, 'template.rte.main', this.config);
    return this;
}

RichTextEditor.prototype.init = function()
{
    // assign all dom elements
    this.domCode = $("richtextcontrol_" + this.name + "_code");

	this.domTabToolbar = $("richtextcontrol_" + this.name + "_tab_toolbar");
	this.domTabContainer = $("richtextcontrol_" + this.name + "_tab_container");
	this.domControlToolbar = $("richtextcontrol_" + this.name + "_control_toolbar");

    this.domTextBoldButton = $("richtextcontrol_" + this.name + "_text_bold_button");
    this.domTextItalicsButton = $("richtextcontrol_" + this.name + "_text_italics_button");
    this.domTextUrlButton = $("richtextcontrol_" + this.name + "_text_url_button");
	this.domTextStrikeButton = $("richtextcontrol_" + this.name + "_text_strike_button");
	
    this.domTabSmilieButton = $("richtextcontrol_" + this.name + "_tab_smilie_button");
    this.domTabMediaButton = $("richtextcontrol_" + this.name + "_tab_media_button");
	this.domTabWidgetButton = $("richtextcontrol_" + this.name + "_tab_widget_button");
    this.domTabAllButton = $("richtextcontrol_" + this.name + "_tab_all_button");
    
    this.domSmilies = $("richtextcontrol_" + this.name + "_smilies");
    this.domMedia = $("richtextcontrol_" + this.name + "_media");
	this.domWidget = $("richtextcontrol_" + this.name + "_widget");
	
	this.domSubmitButton = $("richtextcontrol_" + this.name + "_submit_button");
        
    this.domErrorBox = $("richtextcontrol_" + this.name + "_errorbox");
	
	this.tabLookup = {
		"smilies": [this.domTabSmilieButton, this.domSmilies],
		"media": [this.domTabMediaButton, this.domMedia],
		"widget": [this.domTabWidgetButton, this.domWidget]
	};
        
    // assign the form
    this.form = this.domCode.form;
    
    // select default tab
	if (this.open_picker && this.open_picker != 'none') {
		this.toggleTabs(this.open_picker);
	} else {
		// all tabs are closed, change style
		this.domTabToolbar.addClassName('rte_add_tabs_closed');
		if (!this.domSubmitButton) {
			this.hideDomTabContainer();
		}
	}
	
	// Attach events
	this.attachEvents();
}

RichTextEditor.prototype.attachEvents = function() {
	var _self = this;
	
	Event.observe(this.domErrorBox, "click", function(e) {
		_self.closeErrorBox();
	});
	
	// Textarea
	Event.observe(this.domCode, "beforedeactivate", function(e) {
		// smilies_fancylayout, to put the smiley in the correct location in MSIE.
		textarealoosesfocus("richtextcontrol_" + _self.name);
	});
	Event.observe(this.domCode, "focus", function(e) {
		_self.previewInit();
		_self.showControlsContainers();
	});
	Event.observe(this.domCode, "keypress", function(e) {
		if(e.ctrlKey) {
			// Bold shortcut, ctrl+b(98)
		    if (e.keyCode == 66 || e.charCode == 98) {
	            Event.stop(e); // prevents opening bookmarks tab in browser
		    	_self.insertTextBold();
		    }
			// Italic shortcut, ctrl+i(105)
		    else if (e.keyCode == 73 || e.charCode == 105) {
	            Event.stop(e); //prevent default event
		    	_self.insertTextItalics();
		    }
		    // URL, ctrl+l(108)
            else if (e.keyCode == 76 || e.charCode == 108) {
                Event.stop(e); //prevent default event
                _self.insertTextUrl();
            }
		}
	});
	
	// Options toolbar
	Event.observe(this.domTextBoldButton, "click", function(e) {
		_self.insertTextBold();
	});
	Event.observe(this.domTextItalicsButton, "click", function(e) {
		_self.insertTextItalics();
	});
	Event.observe(this.domTextUrlButton, "click", function(e) {
		Event.stop(e);
		_self.insertTextUrl();
	});
	Event.observe(this.domTextStrikeButton, "click", function(e) {
		_self.insertTextStrike();
	});
	
	// Tabs toolbar
	Event.observe(this.domTabSmilieButton, "click", function(e) {
		_self.toggleTabs("smilies");
	});
	Event.observe(this.domTabMediaButton, "click", function(e) {
		_self.toggleTabs("media");
	});
	Event.observe(this.domTabWidgetButton, "click", function(e) {
		_self.toggleTabs("widget");
	});
	Event.observe(this.domTabAllButton, "click", function(e) {
		_self.toggleTabs("all");
	});
}

RichTextEditor.getInstance = function(name)
{
    return _richtexteditors[name];
}

RichTextEditor.prototype.log=logging.getLogger("RichTextEditor");

/**
*	FUNCTIONS
**/
RichTextEditor.prototype.showError = function(msg) {
	this.domErrorBox.style.display = "block";
	this.domErrorBox.innerHTML = msg;
}

RichTextEditor.prototype.checkEnableOptin = function() {
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

RichTextEditor.prototype.checkPremium = function() {
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

/**
* Tab control of the RTE
* 
* @param string tab
**/
RichTextEditor.prototype.toggleTabs = function(tab) {
	this.log.debug('toggleTabs called for: ' + tab);
	
	if(tab == this.currentTab) {
		// Toggle (if a tab is open, close it)
		this.closeTab(this.currentTab);
	
	} else if(tab == "all") {
		// Toggle all tabs
		if(this.currentTab) {
			this.closeTab(this.currentTab);
		} else {
			if (!this.previousTab) {
				// When no default tab is present and all tabs are closed on load, open smilies tab
				this.previousTab = "smilies";
			}
			this.openTab(this.previousTab);
		}
		
	} else {
		// Another tab is selected, close current, then open other
		if(this.currentTab) {
			this.closeTab(this.currentTab); 
		}
		this.openTab(tab);
	}
}

/**
 * Opens a tab by opening the element and loading tab content
 * 
 * @param string tab
 */
RichTextEditor.prototype.openTab = function(tab) {
	// Show container (in case it was hidden)
	this.showDomTabContainer();
	
	// Update style classes
	this.tabLookup[tab][0].addClassName('current'); // tab-button style
	this.domTabToolbar.removeClassName('rte_add_tabs_closed');
	this.domTabToolbar.addClassName('rte_add_tabs_open');

	// Loading of the content of the selected tab
	var _self = this;
	switch (tab)
	{
		case "smilies":
			requires(['/statics/controls/RTE_Smileypicker.js'], function() {
					_self.onSmileypickerLoaded();
				}
			);
		break;
		case "media":
			requires(['/statics/controls/RTE_Mediapicker.js'], function() {
					_self.onMediapickerLoaded();
				}
			);
		break;
		case "widget":
			requires(['/statics/controls/RTE_Widgetpicker.js'], function() {
					_self.onWidgetpickerLoaded();
				}
			);
		break;
	}
	
	// Show contents
	this.tabLookup[tab][1].show(); // tab contents

	this.currentTab = tab;
}

RichTextEditor.prototype.showDomTabContainer = function() {
	
	if (this.domTabContainer.style.display == 'none') {
		if (window.xmppSession && window.xmppSession.windowId) {
			var windowInfo = window.windowManager.windowInfoJSON(window.xmppSession.windowId).parseJSON();
			window.windowManager.windowExecute(window.xmppSession.windowId, 'update',
				ObjectToJSONString({ height: windowInfo.height + 243 })
			);
			if (window.xmppSession.updateSessionHeight) {
				window.xmppSession.updateSessionHeight();
			}
			window.xmppSession.scrollToBottomOfSession();
		}
		if (window.wwwDialog) {
			window.wwwDialog.updateDialogHeight();
		}
		this.domTabContainer.show();
	}
}

RichTextEditor.prototype.hideDomTabContainer = function() {

	if (this.domTabContainer.style.display != 'none') {
		this.domTabContainer.hide();
		if (window.xmppSession && window.xmppSession.windowId) {
			var windowInfo = window.windowManager.windowInfoJSON(window.xmppSession.windowId).parseJSON();
			if (windowInfo.innerHeight > 570) {
				window.windowManager.windowExecute(window.xmppSession.windowId, 'update',
					ObjectToJSONString({ height: windowInfo.height - 243 })
				);
			}
			if (window.xmppSession.updateSessionHeight) {
				window.xmppSession.updateSessionHeight();
			}
			window.xmppSession.scrollToBottomOfSession();
		}
		if (window.wwwDialog) {
			window.wwwDialog.updateDialogHeight();
		}
	}
}

/**
 * Closes a tab
 * 
 * @param string tab
 */
RichTextEditor.prototype.closeTab = function(tab) {
	// Hide tabcontent-container if there is no submit button
	if (!this.domSubmitButton) {
		this.hideDomTabContainer();
	}
	
	// Update toolbar styles
	this.tabLookup[tab][0].removeClassName('current');
	this.domTabToolbar.removeClassName('rte_add_tabs_open');
	this.domTabToolbar.addClassName('rte_add_tabs_closed');
	
	// Hide tab contents
	this.tabLookup[tab][1].hide();
	
	this.previousTab = tab;
	this.currentTab = '';
}

RichTextEditor.prototype.onSmileypickerLoaded = function() {

	if (!this.oRTESmileyCon) {
		var htmlOut = templates.get('template.rte.smileypicker').process({'name':this.name,
					'picker_leftmenu_width': this.picker_leftmenu_width, 'height': this.picker_height, 
					'width_pickplane': this.picker_width-this.picker_leftmenu_width});
		this.domSmilies.innerHTML = htmlOut;
		
		this.oRTESmileyCon = new RTE_SmiliePickerController();
		this.oRTESmileyCon.name = this.name;
		this.oRTESmileyCon.oRTESmileyOptinCon = this.name;
		this.oRTESmileyCon.smileyPacks = this.smileyPacks;
		
		this.oRTESmileyCon.init();
	}
		
	oRTESmileyOptinCon = new RTE_EnableSmileyOptInController();
	oRTESmileyOptinCon.name = this.name;
	oRTESmileyOptinCon.richTextEditor = this;
	oRTESmileyOptinCon.init(); 
	this.oRTESmileyCon.oRTESmileyOptinCon = oRTESmileyOptinCon;
	
	this.oRTESmileyCon.load();
}
			
RichTextEditor.prototype.onMediapickerLoaded = function() {
	
	if (!this.oRTEMediaCon) {
		this.log.debug('setting root member');
		var rootMember = new Object();
		rootMember['id'] = global_member_id;
		rootMember['fullName'] = global_member_fullname;

		var htmlOut = templates.get('template.rte.mediapicker').process({'name':this.name,
					'picker_leftmenu_width': this.picker_leftmenu_width, 'height': this.picker_height, 'oRootMember': rootMember , 
					'width_pickplane': this.picker_width-this.picker_leftmenu_width});
		
		this.domMedia.innerHTML = htmlOut;

		this.oRTEMediaCon = new RTE_MediaPickerController();
		this.oRTEMediaCon.name = this.name;

		this.oRTEMediaCon.MEDIA_SIZE_ICON_SMALL = this.MEDIA_SIZE_ICON_SMALL;
		this.oRTEMediaCon.init();
	}
	this.oRTEMediaCon.load();
}

RichTextEditor.prototype.onWidgetpickerLoaded = function() {	
	if (!this.oRTEWidgetCon) {
		var htmlOut = templates.get('template.rte.widgetpicker').process({'name':this.name,
					'picker_leftmenu_width': this.picker_leftmenu_width, 'height': this.picker_height, 
					'width_pickplane': this.picker_width-this.picker_leftmenu_width});
		
		this.domWidget.innerHTML = htmlOut;
		
		this.oRTEWidgetCon = new RTE_WidgetPickerController();
		this.oRTEWidgetCon.name = this.name;
		this.oRTEWidgetCon.init();
	}
	this.oRTEWidgetCon.load();
}

RichTextEditor.prototype.changeSmileyPacks = function(pack) {
	this.smileyPacks = pack;
	this.oRTESmileyCon.smileyPacks = pack;
	this.oRTESmileyCon.showSmilies(this.oRTESmileyCon.SMILEY_TYPE_GENERAL);
}

RichTextEditor.prototype.doFormSubmit = function() {
	var returnvalue = true;
	
	if (this.form.onsubmit)
		returnvalue = this.form.onsubmit();
	if (returnvalue)
		this.form.submit();
	return false;	
}

RichTextEditor.prototype.hidePreview = function() {
	
	if (this.oPreview) {
		this.oPreview.hidePreview();
	}
}

RichTextEditor.prototype.onPreviewHide = function() {
	
	Event.observe(this.domCode, 'keydown', this.scheduleSmileySecurityCheck);
}

RichTextEditor.prototype.previewInit = function() {
	
	if (this.showpreview && !this.oPreview) {
		var self = this;
		requires(['module:PreviewModule'], function() {
			var smileyErrorCallback = function(smileyExtra) {
				var message;
				if (smileyExtra == SMILEY_EXTRA_OPTIN) {
					message = self.text_SMILEY_EXTRA_ERROR_ENABLE_OPTIN_FIRST;
				} else {
					message = self.text_SMILEY_EXTRA_ERROR_NOT_PREMIUM;
				}
				self.showError(message);
			};
			self.oPreview = new Preview($('richtextcontrol_' + self.name + '_code'), self.previewWidth,
			                            self.smileyPacks, smileyErrorCallback, self.previewElement,
			                            self.fancy_layout_type, self.fancy_layout_replace_name);
			
			self.oPreview.onHide = function() { self.onPreviewHide(); };
			self.oPreview.onShow = function() { self.onPreviewShow(); };
			if (window._pagers !== undefined) {
				var aKeys = Object.keys(_pagers);
				for (var i = 0; i < aKeys.length; i++) {
					_pagers[aKeys[i]].pagePlacedEvent.subscribe(function(type, args, me) {
						self.oPreview.repositionPreview();
					}, this);
				}
			}
			self.oPreview.showPreview();
		});
	} else if(this.showpreview && this.oPreview) {
		this.oPreview.repositionPreview();
	}
}

RichTextEditor.prototype.onPreviewShow = function() {
	
	Event.stopObserving(this.domCode, 'keydown', this.scheduleSmileySecurityCheck);
	this.stopSmileySecurityCheck();
}

RichTextEditor.prototype.scheduleSmileySecurityCheck = function() {
	this.log.debug('scheduleSmileySecurityCheck()');
	
	this.stopSmileySecurityCheck();
	var self = this;
	this.checkSmileyRightsTimeout = setTimeout(function() {
		self.startSmileySecurityCheck();
	}, 1000);
}

RichTextEditor.prototype.startSmileySecurityCheck = function() {
	
	self = this;
	this.domCode.value = SmileyUtil.removeSmilies(this.domCode.value, this.smileyPacks, function(smileyExtra) {
		var message;	
		if (smileyExtra == SMILEY_EXTRA_OPTIN) {
			message = self.text_SMILEY_EXTRA_ERROR_ENABLE_OPTIN_FIRST;
		} else {
			message = self.text_SMILEY_EXTRA_ERROR_NOT_PREMIUM;
		}
		self.showError(message);
	});
}

RichTextEditor.prototype.getValue = function() {
    return this.domCode.value;	
}

RichTextEditor.prototype.setValue = function(value) {
    this.domCode.value = value;  
}

RichTextEditor.prototype.stopSmileySecurityCheck = function() {

	//this.log.debug('stopping smiley security check');
	
	if (this.checkSmileyRightsTimeout) {
    	clearTimeout(this.checkSmileyRightsTimeout);
    }
}

RichTextEditor.prototype.insertTextBold = function() {
	fancylayout_blocktag("[b]","[/b]","richtextcontrol_" + this.name);
}

RichTextEditor.prototype.insertTextItalics = function() {
	fancylayout_blocktag("[i]","[/i]","richtextcontrol_" + this.name);
}

RichTextEditor.prototype.insertTextUrl = function() {
	var _self = this;
	
	var insertLinkAndCloseDialog = function(dialog) 
    {
        var url = $(dialog.mkid('link_input')).getValue();
        if(fancylayout_getselection("richtextcontrol_" + _self.name)) {
            fancylayout_blocktag("[url=" + url + "]","[/url]", "richtextcontrol_" + _self.name);
        } else {
            fancylayout_inserttext("[url=" + url + "]" + url + "[/url]", "richtextcontrol_" + _self.name);
        }
        dialog.close();
	}
	
	withModule('PopupDialogManager', function(popupDialogManager) {
		popupDialogManager.createDialog("rte_promptdialog_" + _self.name, {
			width: 400,
			positionCenter: true,
			draggable: true,
			alwaysOnTop: true,
			dialogTemplate: "default",
			dialogMessage: "Link",
			contentTemplate: "template.rte.linepromptdialog",
			dialogButtons: {
				cancel: 'GENERAL_CANCEL',
				confirm: 'GENERAL_RTE_CAPTION_INSERT_LINK'
			},
			onContentReady: function(dialog) {
				if(ie) {
                    Event.observe(dialog.mkid('link_input'), "keypress", function(e) {
                        if(e.keyCode == 13) {
                        	Event.stop(e);
                        	insertLinkAndCloseDialog(dialog);
                        }
                    });
				}
				else {
                    Event.observe(dialog.mkid('link_input'), "change", function() {
                        insertLinkAndCloseDialog(dialog);
                    });
				}
			},
			onConfirm: function(dialog) {
				insertLinkAndCloseDialog(dialog);
			},
			onAfterShow: function(dialog) {
				var inputElement = $(dialog.mkid('link_input'));
				inputElement.value = "http://";
				setTimeout(function() {
					inputElement.select();
					inputElement.focus();
				}, 100);
			}
		});
	});
}

RichTextEditor.prototype.insertTextStrike = function() {
	fancylayout_blocktag("[strike]","[/strike]","richtextcontrol_" + this.name);
}

RichTextEditor.prototype.closeErrorBox = function(e) {
	this.domErrorBox.hide();
}

RichTextEditor.prototype.showControlsContainers = function(e){
	this.domControlToolbar.show();
	this.domTabToolbar.show();
}

/**
 * cleanup div's to allow opening a new RTE with the same name
 */
RichTextEditor.prototype.destroy = function(){
	var container = $('rte_container_'+this.name);
	if(container){
		container.previous().remove();
		Element.remove(container);
	}
}

if(window.jsloaded) {jsloaded('/statics/controls/RichTextEditor.js')}


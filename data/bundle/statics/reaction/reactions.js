function ReactionModule()
{
    this.ajaxReactions = {}; //keyed by pager id	
}

ReactionModule.prototype.requires = ['/reaction/reactionTemplates.tpl',
                                     'module:RichTextModule'];
                                     
ReactionModule.prototype.log = logging.getLogger('ReactionModule');

ReactionModule.getInstance = function()
{
	return _moduleInstances["ReactionModule"];
}

/**
 * returns the ajax reaction corresponding to the given pager
 */
ReactionModule.getAjaxReaction = function(pager)
{
    logging.debug("get ajax reaction for pager: " + pager);
    return ReactionModule.getInstance().ajaxReactions[pager];
}


ReactionModule.prototype.showMassDeleteScrapsDialog = function() {

	var self = this;
	withModule('PopupDialogManager', function(popupDialogManager) {
		popupDialogManager.createDialog("scrap_promptdialog", {
			width: 400,
			positionCenter: true,
			draggable: true,
			dialogTemplate: "default",
			dialogMessage: templates.translate('FR_IT_HM_SCRAPS_MASSDELETE_TITLE'),
			contentTemplate: 'reaction.massDeleteScrapsDialog',
			dialogButtons: {
				cancel: 'GENERAL_CANCEL',
				confirm: 'GENERAL_BUTTON_DELETE'
			},
			onBeforeShow: function(dialog) {
				dialog.updateButtons();
				dialog.updateContent();
			},
			onConfirm: function(dialog) {
				logging.debug('onConfirm');
				dialog.disableButtons(true);
				dialog.setBusy(true);
				Effect.Appear(dialog.id + '_ajaxprogress');
				self.requestMassDeleteScraps(dialog);
			}
		});
	});
}

ReactionModule.prototype.requestMassDeleteScraps = function(dialog) {
	var url = "/"
	var pars = scrapsMassDeletePostman;
	
	var self = this;

	var myAjax = new Ajax.Request(
		url,
		{
			method: 'post', 
			parameters: pars,
			onComplete: function(response) {
				self.afterMassDeleteScraps(response);
				dialog.disableButtons(false);
				dialog.setBusy(false);
				dialog.close();
				},
			onException: function (response, exception) {
				self.log.error("" + exception, exception);
			}
		});
}

ReactionModule.prototype.afterMassDeleteScraps = function(originalRequest) {
	
	$('massDeleteScrapsReply').innerHTML = '';
	var responseText = originalRequest.responseText;
	var requestReply = responseText.parseJSON();
	if( requestReply == false ) {
		this.log.error('could not parse ' +  responseText);
		
		withModule('ErrorManager', function(errorManager) {
        	$('massDeleteScrapsReply').innerHTML = errorManager.getError('Unexpected error');
        });
		return;
	}
	
	this.log.debug('succesfully parsed scraps delete response, result = ' + requestReply);
	 
	withModule('ErrorManager', function(errorManager) {
		if (requestReply["success"]) {
			$('massDeleteScrapsReply').innerHTML = errorManager.getSuccess(requestReply["content"]);
		} else {
			$('massDeleteScrapsReply').innerHTML = errorManager.getError(requestReply["content"]);
		}
	});
}


function Reaction(deleteSecret, editSecret) {
	this.deleteSecret = deleteSecret;
	this.editSecret = editSecret;
}

Reaction.prototype.getId = function() {
	return this.reaction_id;	
}

Reaction.prototype.getContent = function() {
	return this.reaction_content;	
}

Reaction.prototype.getDeleteSecret = function() {
	return this.deleteSecret;	
}

Reaction.prototype.getEditSecret = function() {
	return this.editSecret;	
}
	
function AjaxReaction(options) {
	Object.extend(this, options);
	this.topReactionDiv = 'reactionTop';	
	this.createdReactions = new Object();
	ReactionModule.getInstance().ajaxReactions[this.pager] = this;
}

//log for this instance
AjaxReaction.prototype.log = logging.getLogger('AjaxReaction'); 

AjaxReaction.prototype.getRte = function(id) {
    if(!id) {
        id = this.rte;
    }
		
    if(window._richtexteditors && window._richtexteditors[id]) {
        return window._richtexteditors[id];
    }
    else {
        return null;
    }
	   
}

//returns the javascript instance for the richtexteditor corresponding
//to the given reactionId
AjaxReaction.prototype.getRteByReactionId = function(reactionId) {
    return this.getRte(this.rte + '_' + reactionId);
}

AjaxReaction.prototype.getPager = function() {
	if(this.pager && window._pagers && window._pagers[this.pager]) {
		return window._pagers[this.pager];
	}
}

AjaxReaction.prototype.createReaction = function(originalRequest) {
	this.log.debug("create reaction");
	
	var reactionContent = escapePlus(this.getRte().getValue());
	
	if(!reactionContent) {
		withModule('ErrorManager', function(errorManager) {
               errorManager.showError('reactionSendReply', 'ERROR_REACTION_NOCONTENT');
		});
        return false;
	}
	
	if(reactionContent.length > templates.constant('MAX_MESSAGE_SIZE')) {
		withModule('ErrorManager', function(errorManager) {
               errorManager.showError('reactionSendReply', 'ERROR_REACTION_TOOLONG');
		});
		return false;
	}

	$('reactionSendReply').innerHTML = "<img src=\"http://" + hyves_cache_url + "/images/ajax_action.gif\">";

	var url = "/";

	var pars = this.reactionCreatePostman + "&returnType=none&reactionaddsecret=" + escape(this.reactionAddSecret)
	+ "&instanceName=" + this.pager  
	+ "&objectId=" + this.objectId  + "&objecttype=" + this.objectType + "&objectsubtype=" + this.objectSubType + "&type=1&send=verstuur&content=" + reactionContent
	+ "&xmlHttp=1";
	if (this.columnClass) {
		pars+="&return_include_html=true&column_class="+escape(this.columnClass);
	}
	
	var keepInformedType = $('keepInformedType');
	if (keepInformedType) {
		pars = pars + "&keepInformedType=" +  keepInformedType.value;	
	}

	var self = this;
	var myAjax = new Ajax.Request(
			url, 
			{
				method: 'post', 
				postBody: pars,
				onComplete: function(originalRequest, messageText) { self.afterReactionSent(originalRequest, reactionContent) ; },
				onException: function (request, exception) { self.log.error("" + exception, exception); }
			});		
			
	return false;
} 


AjaxReaction.prototype.afterReactionSent = function(originalRequest, reactionContent) {
	var self = this;
	withModule('ErrorManager', function(errorManager) {
        self._afterReactionSent(originalRequest, reactionContent, errorManager);
	});
}

AjaxReaction.prototype._afterReactionSent = function(originalRequest, reactionContent, errorManager) {	
	
	$('reactionSendReply').innerHTML = '';
	
	var responseText = originalRequest.responseText;
	var reactionReply = responseText.parseJSON();
	if( reactionReply == false ) {
		this.log.error('could not parse ' +  responseText);
        $('reactionSendReply').innerHTML = errorManager.getError('unexpected error');
		return;
	}
	
	this.log.debug('succesfully parsed reaction response, result = ' + reactionReply);
	 
    var reactionSendReplyHtml = '';
    	 
	if( reactionReply['aError'] ) {
		var aErrors = reactionReply['aError'].toArray();  
		this.log.debug('found errors ' + aErrors.inspect());		
		aErrors.each( function(value, index) {
			reactionSendReplyHtml += errorManager.getError(value) + '<br/>';
		});
	}
	
	if( reactionReply['aSuccess'] ) {
		this.getRte().setValue('');
		var aSuccess = reactionReply['aSuccess'].toArray(); 
		aSuccess.each(function(value, index) {
				var last = (aSuccess.length - 1) == index;
			    
			    if(last && reactionReply.feedbackCampagne) {
				    reactionSendReplyHtml += errorManager.getSuccess(value, reactionReply.feedbackCampagne);				    
			    }
			    else {
			    	reactionSendReplyHtml += errorManager.getSuccess(value);
			    }
				
				if(!last) {
					reactionSendReplyHtml += '<br/>';
				}
		});
			
		var deleteSecret =  reactionReply['deleteSecret'];;
		var editSecret =  reactionReply['editSecret'];;
		
		var oReaction = new Reaction(deleteSecret, editSecret);
		Object.extend(oReaction, reactionReply['oReaction']);
		this.createdReactions[oReaction.getId()] = oReaction;
		
		this.log.debug('reaction created ' + oReaction.getId() + ' content = ' + oReaction.getContent());
		
		if (reactionReply['html']) {
			var htmlOut = reactionReply['html'];
		} else {
			trigger_error("not used anymore");
		}
			
		this.updateRteAndPager(htmlOut, 'reaction_div_' + oReaction.getId());
	}
	
    Element.update('reactionSendReply', reactionSendReplyHtml);
}

AjaxReaction.prototype.editReaction = function(reactionId, objectId, editSecret) {
	var reactionContent = $('richtextcontrol_reaction_add_' + reactionId + '_code').value;
	var url = "/"
	var pars = this.reactionEditPostman + "&returnType=none&editsecret=" + editSecret + "&reactionId=" + reactionId 
			+ "&objectId=" + objectId + "&content=" + escapePlus(reactionContent);

	this.log.debug("hiding preview");
	var rte = this.getRteByReactionId(reactionId);
	if(rte) {
		rte.hidePreview();
	}
	else {
		this.log.debug("rte instance not found");
	}
	
	var self = this;

	var myAjax = new Ajax.Request(
			url, 
			{
				method: 'post', 
				postBody: pars,
				onComplete: function(originalRequest) { 
					var divNameFancy = 'reaction_content_' + reactionId + "_fancy";
					self.log.debug('setting ' + divNameFancy + ' to new content');
					
					$(divNameFancy).innerHTML = fancy_layout(reactionContent, 'body'); 
					$('reaction_edit_button_' + reactionId).style.display = 'block';
					
					if(self.getPager()) {
						self.getPager().addChangedItem('reaction_div_' + reactionId);
					}
				},
				onException: function (request, exception) { self.log.error("" + exception, exception); }
			});		
			
	return false;	
}

AjaxReaction.prototype.deleteReaction = function(reactionId, objectId, deletesecret) {
	
	var url = "/"
	var pars = this.reactionDeletePostman + "&returnType=none&reactionId=" + reactionId + "&objectId=" + objectId + '&deletesecret=' + deletesecret;
	
	var self = this;
	var myAjax = new Ajax.Request(
			url, 
			{
				method: 'post', 
				parameters: pars,
				onComplete: function(response) { 
						self.log.debug('unshowing diff reaction_div_' + reactionId);
						Effect.Shrink('reaction_div_' + reactionId);
						if(self.getPager()) {
						  self.getPager().addDeletedItem('reaction_div_' + reactionId);
						}
					} ,
				onException: function (response, exception) { self.log.error("" + exception, exception); }
			});		
			
	return false;	
}

AjaxReaction.prototype.showRTE = function(reactionId, objectId, editSecret) {
	
	var url = "/";
	var pars = "module=reaction&action=showRTE&name=reaction_add_" + reactionId + "&reaction_id=" + reactionId + "&object_id=" + objectId + "&editsecret=" + editSecret;
	$('reaction_content_' + reactionId + '_fancy').innerHTML = "<img src=\"http://" + hyves_cache_url + "/images/ajax_action.gif\">";
	
	var self = this;
	var myAjax = new Ajax.Request(
			url, 
			{
				method: 'GET', 
				parameters: pars,
				onException: function (response, exception) { self.log.error("" + exception, exception); },
				onComplete: function(result) {
					var formDo = document.createElement("form");
					formDo.onsubmit=function() {return ReactionModule.getAjaxReaction(self.pager).editReaction(reactionId, objectId , editSecret)}
					result.responseText.evalScripts();
					formDo.innerHTML=result.responseText.stripScripts();;
					$('reaction_content_' + reactionId + '_fancy').innerHTML="";
					$('reaction_content_' + reactionId + '_fancy').appendChild(formDo);
				}
			});		
			
	return false;	
	
}

AjaxReaction.prototype.getReaction = function(reactionId) {
	return this.createdReactions[reactionId];
}

AjaxReaction.prototype.updateRteAndPager = function(item, divName) {
	
	this.log.debug('updating ' + divName);
	
	var rte = this.getRte();
	
	if (rte) {
		rte.hidePreview();
	}
	
	this.log.debug('preview visibility set to none');
	
	if (this.getPager()) {
		//there are already reaction, insert to top or bottom depending on sort order
		this.log.debug('sortOrder ' + this.sortOrder);
		if(this.sortOrder == templates.constant('MEMBER_GROUPTHREADSORT_DESC')) {
			this.log.debug('sortOrder ' + templates.constant('MEMBER_GROUPTHREADSORT_DESC'));
			this.getPager().showFirstPageAndAdd(item, divName);	
		}
		else {		
			this.getPager().showLastPageAndAdd(item, divName);	
		}
	}
	else {
		if(this.sortOrder == templates.constant('MEMBER_GROUPTHREADSORT_DESC')) {
			//this is the first reaction
			var topDivName = 'pagerTop_show_reactions'; 
			if(!$(topDivName)) {
				logging.debug('top div ' + topDivName + ' not found');
			}
			else {
				new Insertion.After(topDivName, item);
				logging.debug("sliding pagerTop");
				Effect.SlideDown(divName);
			}
		}
		else {
			var bottomDivName = 'pagerBottom_show_reactions'; 
			if(!$(bottomDivName)) {
				//first reaction
				bottomDivName = 'pagerTop_show_reactions';
			}
			else {
				new Insertion.Before(bottomDivName, item);
				this.log.debug("sliding pagerBottom");
				Effect.SlideDown(bottomDivName);
			}
		}
	}
}

if(window.jsloaded) {jsloaded('/statics/reaction/reactions.js')}


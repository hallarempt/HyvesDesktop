/*	RichTextEditSmiliePickerController
*
*	The following properties need to be set:
*
*	RTE_MemberPickerController.name								// name of the richtextedit
*	RTE_MemberPickerController.text_PLEASE_WAIT_BEING_LOADED	// text element
*	RTE_MemberPickerController.logged_in_member_id				// logged in member id
*	RTE_MemberPickerController.MEDIA_SIZE_ICON_SMALL			// image size
*
*	[ ] = optional
*/
function RTE_MemberPickerController() {
}

RTE_MemberPickerController.prototype.log = logging.getLogger("MemberPicker");

RTE_MemberPickerController.prototype.init = function() {
	this.firstLoad = true;
	
	this.text_PLEASE_WAIT_BEING_LOADED =  templates.translate('PLEASE_WAIT_BEING_LOADED');
	
	this.domPickplane = $("memberpicker_" + this.name + "_pickplane");
}

RTE_MemberPickerController.prototype.load = function() {
	if (this.firstLoad == true) {
		this.loadFriends();
	}
		
	this.firstLoad = false;
}

RTE_MemberPickerController.prototype.loadFriends = function() {
	
	this.domPickplane.innerHTML = this.text_PLEASE_WAIT_BEING_LOADED;
	
	var _self = this;
	var request = new WSARequest();
	var extraParams = {
		ha_responsefields: "username"
	};
	request.get("users.getFriendsByLoggedinAlphabetically", function(response) {
		_self.renderFriends(response);
	}, extraParams);
}
	
RTE_MemberPickerController.prototype.renderFriends = function(data) {	
	RemoveChildsFromDom(this.domPickplane);
	
	if (data.method == "error") {
		this.domPickplane.innerHTML = templates.translate('RTE_ERROR_API_GET_FRIENDS') +  " (code: " + data.error_code + ")";
		return;
	}
	
	var aoFriend = data.user;
	var htmlOut = templates.get('template.rte.memberpicker.results').process({
		'aoFriend': aoFriend,
		'member_id': global_member_id,
		'member_nickname': global_member_nickname,
		'name':this.name
	});
	this.domPickplane.innerHTML = htmlOut;

	// Attach events
	var _self = this;
	
	Event.observe("memberpicker_" + this.name + "_member_" + global_member_id + "_add", "click", function(e) {
		_self.insertTextMember(global_member_username, global_member_nickname);
	});
	
	$A(aoFriend).each(function(oFriend) {
		var friendPickerId = "memberpicker_" + _self.name + "_member_" + oFriend.userid + "_add";
		Event.observe(friendPickerId, "click", function(e) {
			_self.insertTextMember(oFriend.username, oFriend.displayname);
		});
	});
}

RTE_MemberPickerController.prototype.insertTextMember = function(username, displayname) {
	if (fancylayout_getselection("richtextcontrol_" + this.name) == "")	{
		fancylayout_inserttext("[hyver=" + username + "]" + displayname + "[/hyver]", "richtextcontrol_" + this.name);
	} else {
		fancylayout_blocktag("[hyver=" + username + "]", "[/hyver]", "richtextcontrol_" + this.name);
	}
}

if(window.jsloaded) {jsloaded('/statics/controls/RTE_Memberpicker.js')}


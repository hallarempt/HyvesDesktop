/*	RichTextEditSmiliePickerController
*
*	The following properties need to be set:
*
*	RTE_MediaPickerController.name							// name of the richtextedit
*	RTE_MediaPickerController.text_PLEASE_WAIT_BEING_LOADED	// text element
*	RTE_MediaPickerController.MEDIA_SIZE_ICON_SMALL			// image size
*
*	[ ] = optional
*/
function RTE_MediaPickerController() {
}

RTE_MediaPickerController.prototype.log = logging.getLogger('MediaPicker'); 

RTE_MediaPickerController.prototype.init = function() {
	
	try {
		this.firstLoad = true;
		
		this.domPickplane = $("mediapicker_" + this.name + "_pickplane");
		this.domAlbumsButton = $("mediapicker_" + this.name + "_albums_button");
		this.domAlbums = $("mediapicker_" + this.name + "_memberalbums");
		this.domAlbumFolderImage = $("mediapicker_" + this.name + "_album_folderimage"); // not used yet...!
		this.domAlbumCount = $("mediapicker_" + this.name + "_album_count");
		//no upload button so far
		//this.domUploadNewButton = $("mediapicker_" + this.name + "_uploadnew_button");
		this.text_PLEASE_WAIT_BEING_LOADED =  templates.translate('PLEASE_WAIT_BEING_LOADED');
		
		// Adds events
		var _self = this;
		Event.observe(this.domAlbumsButton, "click", function(e) {
			_self.domAlbums.toggle();
		});

		//no upload button so far
		/*Event.observe(this.domUploadNewButton, "click", function(e) {
			_self.openUploadPopup();
		});*/
	} catch(e) {
		logging.error('RTE_MediaPickerController::init(): ' + e);
	}
}

RTE_MediaPickerController.prototype.load = function() {
	if (this.firstLoad == true) {
		this.loadMemberAlbums(global_member_id);
	}

	this.firstLoad = false;
}

/**
*	Reset right content tab
**/
RTE_MediaPickerController.prototype.clearPicker = function() {
	this.domPickplane.innerHTML = "";
}

RTE_MediaPickerController.prototype.loadMemberAlbums = function(member_id) {
	this.domPickplane.innerHTML = this.text_PLEASE_WAIT_BEING_LOADED;

	var self = this;
	new Ajax.Request('/a2a.php', {
		method: 'get',
		parameters: { 
			method: 'getallalbumsformember', 
			member_id: global_member_id
		},
		onSuccess: function(response) {
			self.renderAlbumsForMember(response.responseXML.documentElement);
			self.domAlbums.toggle();
		}
	});
}

/**
 * Loads the albums into the mediapicker.
 */
RTE_MediaPickerController.prototype.renderAlbumsForMember = function(reply) {

	var _self = this;
	RemoveChildsFromDom(this.domAlbums);
	aAlbum = reply.getElementsByTagName("album");
	this.domAlbumCount.update(aAlbum.length);
	$A(aAlbum).each(function(album) {
		var albumName = album.getElementsByTagName("name")[0].firstChild.nodeValue;
		var albumSize = album.getElementsByTagName("size")[0].firstChild.nodeValue
		var albumId = album.getElementsByTagName("id")[0].firstChild.nodeValue;
		var albumSecret = album.getElementsByTagName("secret")[0].firstChild.nodeValue;
		var nod = document.createElement("div");
		nod.className = "personal_fakelink fakelink";
		nod.innerHTML = htmlspecialchars(albumName) + " (" + albumSize + ")";
		nod.id = "mediapicker_" + _self.name + "_memberalbums_" + albumId + "_loadalbum";
		_self.domAlbums.appendChild(nod);
		
		Event.observe(nod, "click", function(e) {
			_self.loadAlbum(albumId, albumSecret);
		});
    });
   	RemoveChildsFromDom(this.domPickplane);
}

RTE_MediaPickerController.prototype.loadAlbum = function(album_id, secret) {
	
	this.domPickplane.innerHTML = this.text_PLEASE_WAIT_BEING_LOADED;
	
	var self = this;
	new Ajax.Request('/a2a.php', {
		method: 'get',
		parameters: {
			method: 'getallmediafromalbum', 
			album_id: album_id, 
			album_secret: escape(secret)
		},
		onSuccess: function(response) {
			self.renderMediaFromAlbum(response.responseXML.documentElement);
		}
	});
}

/**
 * Loads the media into the mediapicker_pickerplane.
 */
RTE_MediaPickerController.prototype.renderMediaFromAlbum = function(reply) {
	
	var _self = this;

	//sometimes the dom reference seems to go stale, and the media picker does not show
	//the album, refetch it here prevents this problem
	this.domPickplane = $("mediapicker_" + this.name + "_pickplane");

	this.clearPicker();
	
	aMediaResult = reply.getElementsByTagName("media");
	
	// Prepare results	
	albumTitle = htmlspecialchars(reply.getElementsByTagName("name")[0].firstChild.nodeValue);
	var aMedia = [];
	$A(aMediaResult).each(function(mediaResult) {
		var media = {
			id: mediaResult.getElementsByTagName("id")[0].firstChild.nodeValue,
			secret: mediaResult.getElementsByTagName("secret")[0].firstChild.nodeValue,
			url: mediaResult.getElementsByTagName("url").item(0).firstChild.nodeValue,
			type: mediaResult.getElementsByTagName("type")[0].firstChild.nodeValue
		};
		aMedia.push(media);
	});
	
	// Output results
	var htmlOut = templates.get('template.rte.mediapicker.results').process({
		'aMedia': aMedia,
		'albumTitle': albumTitle,
		'name':this.name
	});
	this.domPickplane.innerHTML = htmlOut;
	
	// Attach actions
	$A(aMedia).each(function(media) {
		imageDomId = "mediapicker_" + _self.name + "_memberalbums_" + media.id + "_add";
		Event.observe(imageDomId, "click", function(e) {
			_self.insertMedia(media.id + media.secret, media.type);
		});
    });
}

/**
 * params:
 * [media_id_secret, media_type]
 */
RTE_MediaPickerController.prototype.insertMedia = function(media_id, media_type) {
	media_icon_size_val = $("media_icon_size").value;
	if( (media_type == templates.constant('FILEUTIL_TYPE_VIDEO') || media_type == "video") && (media_icon_size_val == "xlarge") ) {
		media_icon_size_val = "large";
	}

	fancylayout_inserttext("[media id=" + media_id + " size=" + media_icon_size_val + "]", "richtextcontrol_" + this.name);
	
	var sendButton = $('uiButtonSend');
	if (sendButton) {
		sendButton.removeClassName('disabled');
	}
}

RTE_MediaPickerController.prototype.openUploadPopup = function() {
	RichTextEditor.getInstance(this.name).hidePreview();
	showPopupDialogFromUrl(
		'rtemediaupload', 
		templates.translate('RTE_UPLOAD_PHOTO'),
		'/?module=Media&action=uploadPopup&rtename=' + this.name, 
		{width: "800px"}, 
		['/statics/mediaUploadPicker.js','/statics/TabPage.js']
	);
}

if(window.jsloaded) {jsloaded('/statics/controls/RTE_Mediapicker.js')}


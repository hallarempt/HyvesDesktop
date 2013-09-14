var MEDIA_REQUEST_TIMEOUT = 1000;

function MediaLocator() {
	
	this.mediaRequests = []; // requests for media (request objects)
	this.pendingElements = {}; // elements that are still waiting for their image,
	                           // "mediaId/mediaSecret" => array of [ element ID, width, height ] arrays
	this.requestTimeoutId = null;
	
	window.extender.connect('mediaCache', 'mediaLoaded(QString)', 'window.mediaLocator.onMediaLoaded');
	
	window.mediaLocator = this;
}

/**
 * Sets media on an <img> element. The <img> element should already have a fixed
 * size. The media will be loaded so that it will fit this size.<b> 
 *
 * @param elementOrId Element or ID of the element on which to set the image.
 * @param mediaId ID of the media to set.
 * @param mediaSecret Secret of the media to set.
 */
MediaLocator.prototype.setMedia = function(elementOrId, mediaId, mediaSecret) {
	
	var idString = mediaId + '/' + mediaSecret;
	var element = $(elementOrId);
	var width = element.width;
	var height = element.height;
	
	if (window.mediaCache.isInCache(idString)) {
		element.src = window.mediaCache.mediaUrl(idString, width, height);
		return;
	}
	
	this.mediaRequests.push('m@' + mediaId + '@' + mediaSecret + '@4');
	
	if (!this.pendingElements.hasOwnProperty(idString)) {
		this.pendingElements[idString] = [];
	}
	this.pendingElements[idString].push([ element, width, height ]);
	
	this.scheduleRequest();
}

/**
 * Cancels outstanding waiting request and schedules a new one
 * some time in the future. Schedules an immidiate request when now is true.
 */
MediaLocator.prototype.scheduleRequest = function(now) {
	
	if (this.mediaRequests.length == 0) {
		return; // nothing to request
	}
	
	if(this.requestTimeoutId != null) {
		clearTimeout(this.requestTimeoutId);
	}
	
	var self = this;
	var timeout = now ? 0 : MEDIA_REQUEST_TIMEOUT;
	this.requestTimeoutId = setTimeout(function() {
		self.doRequest();
	}, timeout);
}

/**
 * Do the actual AJAX request to resolve the media URLs.
 */
MediaLocator.prototype.doRequest = function() {
	
	var done = {}; // keep track of mediaKeys already in this request
	var postPars = ''; // build up post request vars
	var numMedia = 0;
	while (numMedia < 100 && this.mediaRequests.length > 0) {
		var mediaKey = this.mediaRequests.pop();
		if (!done[mediaKey]) {
			postPars += mediaKey + ".";
			done[mediaKey] = true;
			numMedia++;
		}
	}
	
	// send the AJAX request to the resolver
	if (postPars) {
		var self = this;
		var request = new Ajax.Request('/locateMedia.php', {
			method: 'post',
			asynchronous: true,
			postBody: 'i=' + postPars,
			onComplete: function(request) { self.onMediaUrlResponse(request); },
			onFailure: function(request) { if (logging) { logging.error('mediaLocator.js:doRequest(): Error locating media.'); } }
		});
	}
	this.scheduleRequest(false);
}

/**
 * The locateMedia script did its work and returned
 * all the resolved mediaUrls to us
 */
MediaLocator.prototype.onMediaUrlResponse = function(request) {
	
	var result = request.responseText.parseJSON();
	
	for (var i = 0; i < result.length; i += 2) {
		var mediaKey = result[i];
		var mediaKeyParts = mediaKey.split('@');
		var mediaId = mediaKeyParts[1];
		var mediaSecret = mediaKeyParts[2];
		var url = result[i + 1];
		if (url.substr(0, 7) == 'http://') {
			// make sure we only insert valid URLs (in some cases we can get a "not found" result)
			window.mediaCache.insertUrl(mediaId + '/' + mediaSecret, url);
		}
	}
}

/**
 * Called when the media is loaded in the cache and is ready to be set on an
 * image element. 
 */
MediaLocator.prototype.onMediaLoaded = function(id) {
	
	if (!this.pendingElements.hasOwnProperty(id)) {
		return;
	}
	
	while (this.pendingElements[id].length > 0) {
		var pendingElement = this.pendingElements[id].pop();
		var element = pendingElement[0];
		var width = pendingElement[1];
		var height = pendingElement[2];
		element.src = window.mediaCache.mediaUrl(id, width, height);
	}
}

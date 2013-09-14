var BUZZ_DECREASE_TIMEOUT = 2 * 60; // decrease the buzz count every X seconds
var MAX_BUZZ_COUNT = 3; // maximum number somebody may be buzzed per BUZZ_DECREASE_TIMEOUT period.

// this module implements the logic of buzzing
// e.g. mostly rules how often somebody may be buzzed and or may buzz
// this code is shared by site, web and desktop clients
function BuzzManager() {
	
	this.log.debug('init');
	var self = this;
	
	// keep track how many times I have been buzzed by jid, map of jid -> (times)
	// if more than MAX_BUZZ_COUNT, it will take BUZZ_RESET_TIME before jid can buzz me again
	// buzzCount is stored in a cookie so it will survive sessions
	this.receiveBuzzCount = {};
	this.sendBuzzCount = {};
		
	// try to load it from the cookie if present
	if (getCookie('chatBuzzCount')) {
		var cookie = getCookie('chatBuzzCount').parseJSON();
		if(cookie) {
			this.receiveBuzzCount = cookie[0];
			this.sendBuzzCount = cookie[1];
		}
	}
	
	Event.observe(window, 'beforeunload', function(event) {
		try {
			self.log.debug("storing buzz counts");
			var cookie = ArrayToJSONString([self.receiveBuzzCount, self.sendBuzzCount, self.lastUpdateSeconds]);
			setCookie('chatBuzzCount', cookie, null, '/', hyves_base_url, false);
			clearInterval(self.updateBuzzCountInterval);
		} catch(e) {
			this.log.error('' + e, e);
		}
	});
	
	this.updateBuzzCountInterval = setInterval(function() {
		self._updateBuzzCount();
	}, BUZZ_DECREASE_TIMEOUT * 1000);
}

BuzzManager.prototype.log = logging.getLogger("BuzzManager");

/**
 * Updates the receiving and sending buzzcounts, e.g.
 * every BUZZ_DECREASE_TIMEOUT period, we subtract 1 from the count
 * so that member can send and or receive more buzzes. 
 */
BuzzManager.prototype._updateBuzzCount = function() {
	
	var decreaseCounts = function(buzzCount) {
		$H(buzzCount).each(function(pair) {
			var jid = pair.key;
			var count = pair.value;
			
			count--;
			if (count < 1) {
				delete buzzCount[jid];
			} else {
				buzzCount[jid] = count;
			}
		});
	}
	
	decreaseCounts(this.receiveBuzzCount);
	decreaseCounts(this.sendBuzzCount);
}

/**
 * Internal method to check whether we are still allowed to
 * send or receive to the given jid (e.g. if MAX_BUZZ_COUNT has not
 * yet been reached).
 */
BuzzManager.prototype._maySendOrReceiveBuzz = function(buzzCount, jid) {
	
	if (!buzzCount[jid] || buzzCount[jid] < MAX_BUZZ_COUNT) {
		return true;
	} else {
		return false;
	}
}

/**
 * Determines if fromJid may still buzz me
 */
BuzzManager.prototype.mayReceiveBuzzFrom = function(fromJid) {
	
	fromJid = jidbare(fromJid);
	return this._maySendOrReceiveBuzz(this.receiveBuzzCount, fromJid);
}

/**
 * Determines if i may still buzz toJid.
 */
BuzzManager.prototype.maySendBuzzTo = function(toJid) {
	
	toJid = jidbare(toJid);
	return this._maySendOrReceiveBuzz(this.sendBuzzCount, toJid);
}

/**
 * Increase the buzz count with 1
 */
BuzzManager.prototype._incBuzzCount = function(buzzCount, jid) {
	this.log.debug("old buzz count: " + buzzCount[jid]);
	
	if (buzzCount[jid]) {
		var count = buzzCount[jid];
		buzzCount[jid] = count + 1;
	} else {
		buzzCount[jid] = 1;
	}
	
	if (buzzCount[jid] > MAX_BUZZ_COUNT) {
		buzzCount[jid] = MAX_BUZZ_COUNT;
	}
	
	this.log.debug("new buzz count: " + buzzCount[jid]);
}

/**
 * Receive a buzz from fromJid, if this buzz is allowed.
 */
BuzzManager.prototype.receiveBuzz = function(fromJid) {
	
	fromJid = jidbare(fromJid);
	if (this.mayReceiveBuzzFrom(fromJid)) {
		this._incBuzzCount(this.receiveBuzzCount, fromJid);
		return true;
	} else {
		this.log.debug("not allowed to receive buzz from: " + this.log.anonymizeEMail(fromJid));
		return false;
	}
}

/**
 * Send a buzz to toJid, if this is still allowed.
 */
BuzzManager.prototype.sendBuzz = function(toJid) {
	
	toJid = jidbare(toJid);
	if (this.maySendBuzzTo(toJid)) {
		this._incBuzzCount(this.sendBuzzCount, toJid);
		return true;
	} else {
		this.log.debug("not allowed to send buzz to: " + this.log.anonymizeEMail(toJid));
		return false;
	}
}

if(window.jsloaded) {jsloaded('/statics/kwekker2/buzzer.js')}

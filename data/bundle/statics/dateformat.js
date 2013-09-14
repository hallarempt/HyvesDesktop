
function DateFormat() {
	
	this.DATE_TIME_LONG = new SimpleDateFormat("d MMM, HH:mm");
	this.DATE_LONG = new SimpleDateFormat("d MMM yyyy");
	this.DATE_MEDIUM = new SimpleDateFormat("dd-MM-yyyy");
	this.DATE_SMALL = new SimpleDateFormat("dd-MM");
	this.TIME_LONG = new SimpleDateFormat("HH:mm:ss");
	this.TIME_SMALL = new SimpleDateFormat("HH:mm");
	this.MONTH_LONG = new SimpleDateFormat("MMM yyyy");
	this.HOUR = new SimpleDateFormat("HH");
}
	
DateFormat.getInstance = function() {
	
	return _moduleInstances['DateFormat'];
}

DateFormat.prototype.requires = ['/statics/simpledateformat.js'];

DateFormat.prototype.formatDateTimeLong = function(date) {
	
	return this.DATE_TIME_LONG.format(date);
}

DateFormat.prototype.formatDateLong = function(date) {
	
	return this.DATE_LONG.format(date);
}

DateFormat.prototype.formatDateSmall = function(date) {
	
	return this.DATE_SMALL.format(date);
}

DateFormat.prototype.formatTimeLong = function(date) {
	
	return this.TIME_LONG.format(date);
}

DateFormat.prototype.formatMonthLong = function(date) {
	
	return this.MONTH_LONG.format(date);
}

DateFormat.prototype.formatTimeSmall = function(date) {
	
	if (!date) {
		date = new Date(); // now
	}
	return this.TIME_SMALL.format(date);
}

if(window.jsloaded) {jsloaded('/statics/dateformat.js')}

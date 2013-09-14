var _rootLogger = log4javascript.getLogger("root");
_addAppenders(_rootLogger);

logging = {
	getLogger: function(name)
	{
		logger = log4javascript.getLogger(name); 
		_addAppenders(logger);
		return logger;
	},
	debug: function() { _rootLogger.debug.apply(_rootLogger, arguments)},
	info: function() { _rootLogger.info.apply(_rootLogger, arguments)},
	warn: function() { _rootLogger.warn.apply(_rootLogger, arguments)},
	error: function() { _rootLogger.error.apply(_rootLogger, arguments)},
	fatal: function() { _rootLogger.fatal.apply(_rootLogger, arguments)},
	log: function() {}
}

//this prevents yui-ext from popping up it's own console and
//javascript errors on IE
if(typeof console == "undefined")
{
	console = logging;
}

function FlashLogger_Debug()
{
	logging.debug.apply(logging.debug, arguments);
}


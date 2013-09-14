

// the eventhandler, singleton.
function EventHandler() {
	this.init();
}

EventHandler.prototype.init = function() {
	this.traceBackEvents = new Array();
}

EventHandler.prototype.addOnClickEvent = function(DomElement, controller, controllerOperation, params) {
	this.addEvent(DomElement, "click", controller, controllerOperation, params);
}

EventHandler.prototype.addOnKeyUpEvent = function(DomElement, controller, controllerOperation, params) {
	this.addEvent(DomElement, "keyup", controller, controllerOperation, params);
}

EventHandler.prototype.addOnKeyPressEvent = function(DomElement, controller, controllerOperation, params) {
	this.addEvent(DomElement, "keypress", controller, controllerOperation, params);
}

EventHandler.prototype.addOnFocusEvent = function(DomElement, controller, controllerOperation, params) {
	this.addEvent(DomElement, "focus", controller, controllerOperation, params);
}

EventHandler.prototype.addOnBeforeDeactivateEvent = function(DomElement, controller, controllerOperation, params) {
	this.addEvent(DomElement, "beforedeactivate", controller, controllerOperation, params);
}

EventHandler.prototype.addOnMouseDownEvent = function(DomElement, controller, controllerOperation, params) {
	this.addEvent(DomElement, "mousedown", controller, controllerOperation, params);
}

EventHandler.prototype.addOnMouseMoveEvent = function(DomElement, controller, controllerOperation, params) {
	this.addEvent(DomElement, "mousemove", controller, controllerOperation, params);
}

EventHandler.prototype.addOnMouseUpEvent = function(DomElement, controller, controllerOperation, params) {
	this.addEvent(DomElement, "mouseup", controller, controllerOperation, params);
}

EventHandler.prototype.addOnMouseOutEvent = function(DomElement, controller, controllerOperation, params) {
	this.addEvent(DomElement, "mouseout", controller, controllerOperation, params);
}


EventHandler.prototype.addFakeEvent = function(name, controller, controllerOperation, params) {
	fakeElement = new Object();
	fakeElement["id"] = name;
	this.addTraceBackEvent(fakeElement, "fake_event", controller, controllerOperation, params);
}

EventHandler.prototype.addEvent = function(DomElement, eventType, controller, controllerOperation, params) {
//register few event handlers, for compatibility
	DomElement["on" + eventType] = handleEvent;
//	DomElement.addEventListener(eventType,handleEventCapture,false);
//	DomElement.addEventListener(eventType,handleEventBubble,true);
	this.addTraceBackEvent(DomElement, eventType, controller, controllerOperation, params);
}

EventHandler.prototype.addTraceBackEvent = function(DomElement, eventType, controller, controllerOperation, params) {
	this.traceBackEvents[EventHandler.getEventMapping(DomElement, eventType)] = [controller, controllerOperation, params];
}

EventHandler.prototype.getTraceBackEvent = function(DomElement, eventType) {
	return this.traceBackEvents[EventHandler.getEventMapping(DomElement, eventType)];
}

EventHandler.autoIdCounter = 0;
EventHandler.getEventMapping = function(DomElement, eventType) {
	if (!DomElement.id)
	{
		var id = "EventHandlerAutoId_"+EventHandler.autoIdCounter;
		EventHandler.autoIdCounter++;
		DomElement.id = id;
	}
	return DomElement.id + ';' + eventType;
}

EventHandler.prototype.addParamsToFakeEvent = function(name, params) {
	fakeElement = new Object();
	fakeElement["id"] = name;
	aFunction = this.getTraceBackEvent(fakeElement, "fake_event");
	if (aFunction)
	{
		this.addTraceBackEvent(fakeElement, "fake_event", aFunction[0], aFunction[1], params);
	}
}

EventHandler.prototype.triggerFakeEvent = function(name) {
	fakeElement = new Object();
	fakeElement["id"] = name;
	aFunction = this.getTraceBackEvent(fakeElement, "fake_event");
	if (aFunction)
	{
		oEventHandler = aFunction[0];
		oEventHandler[aFunction[1]](aFunction[2]);
	}
}

EventHandler.getEvent = function(e) {
	if (isIE) {
		return event;
	} else {
		return e;
	}
}

EventHandler.getDomElementFromEvent = function(e) {
	if (isIE) {
		return e.srcElement;
	} else {
		return e.target;
	}
}

EventHandler.prototype.handleEvent = function(e) {
	aFunction = null;
	e = EventHandler.getEvent(e);
	eventType = e.type;
	DomElement = EventHandler.getDomElementFromEvent(e);
	// defeat Safari bug
	if (DomElement.nodeType == 3) 
		DomElement = DomElement.parentNode;
		
	aFunction = this.getTraceBackEvent(DomElement, eventType);

	while(!aFunction && DomElement.parentNode){
		DomElement = DomElement.parentNode;
		aFunction = this.getTraceBackEvent(DomElement, eventType);
	}
		
	if (aFunction)
	{
		oController = aFunction[0];
		eventName = aFunction[1];
		if(eventName) {
			return oController[aFunction[1]](e, aFunction[2]);
		}
	}
}

EventHandler.prototype.handleEventByDom = function(DomElement, eventType) {
	// defeat Safari bug
	if (DomElement.nodeType == 3) 
		DomElement = DomElement.parentNode;
	aFunction = this.getTraceBackEvent(DomElement, eventType);
	if (aFunction)
	{
		oController = aFunction[0];
		oController[aFunction[1]](null, aFunction[2]);
	}
}

EventHandler.oInstance = null;

EventHandler.getInstance = function() {
	if (!EventHandler.oInstance)
	{
		EventHandler.oInstance = new EventHandler();
	}	
	return EventHandler.oInstance;
}



//Event helper
function handleEvent(e)
{
	EventHandler.getInstance().handleEvent(e);
	return true;
}

function handleEventByDom(DomElement, eventType)
{
	EventHandler.getInstance().handleEventByDom(DomElement, eventType);
}
//end eventhandler.



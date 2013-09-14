/*
    json.js
    2006-04-28

    This file adds these methods to JavaScript:

        object.toJSONString()

            This method produces a JSON text from an object. The
            object must not contain any cyclical references.

        array.toJSONString()

            This method produces a JSON text from an array. The
            array must not contain any cyclical references.

        string.parseJSON()

            This method parses a JSON text to produce an object or
            array. It will return false if there is an error.
*/
(function () {
    var m = {
            '\b': '\\b',
            '\t': '\\t',
            '\n': '\\n',
            '\f': '\\f',
            '\r': '\\r',
            '"' : '\\"',
            '\\': '\\\\'
        },
        s = {
            array: function (x) {
                var a = ['['], b, f, i, l = x.length, v;
                for (i = 0; i < l; i += 1) {
                    v = x[i];
                    f = s[typeof v];
                    if (f) {
                        v = f(v);
                        if (typeof v == 'string') {
                            if (b) {
                                a[a.length] = ',';
                            }
                            a[a.length] = v;
                            b = true;
                        }
                    }
                }
                a[a.length] = ']';
                return a.join('');
            },
            'boolean': function (x) {
                return String(x);
            },
            'null': function (x) {
                return "null";
            },
            number: function (x) {
                return isFinite(x) ? String(x) : 'null';
            },
            object: function (x) {
                if (x) {
                    if (x instanceof Array) {
                        return s.array(x);
                    }
                    var a = ['{'], b, f, i, v;
                    for (i in x) {
                        v = x[i];
                        f = s[typeof v];
                        if (f) {
                            v = f(v);
                            if (typeof v == 'string') {
                                if (b) {
                                    a[a.length] = ',';
                                }
                                a.push(s.string(i), ':', v);
                                b = true;
                            }
                        }
                    }
                    a[a.length] = '}';
                    return a.join('');
                }
                return 'null';
            },
            string: function (x) {
                if (/["\\\x00-\x1f]/.test(x)) {
                    x = x.replace(/([\x00-\x1f\\"])/g, function(a, b) {
                        var c = m[b];
                        if (c) {
                            return c;
                        }
                        c = b.charCodeAt();
                        return '\\u00' +
                            Math.floor(c / 16).toString(16) +
                            (c % 16).toString(16);
                    });
                }
                return '"' + x + '"';
            }
        };

    ObjectToJSONString = function (object) {
        return s.object(object);
    };

    ArrayToJSONString = function (string) {
        return s.array(string);
    };
})();

String.prototype.parseJSON = function () {
    try {
        return !(/[^,:{}\[\]0-9.\-+Eaeflnr-u \n\r\t]/.test(
                this.replace(/"(\\.|[^"\\])*"/g, ''))) &&
            eval('(' + this + ')');
    } catch (e) {
        return false;
    }
};



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


/**
 * TrimPath Template. Release 1.0.38.
 * Copyright (C) 2004, 2005 Metaha.
 * 
 * TrimPath Template is licensed under the GNU General Public License
 * and the Apache License, Version 2.0, as follows:
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed WITHOUT ANY WARRANTY; without even the 
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
var TrimPath;

// TODO: Debugging mode vs stop-on-error mode - runtime flag.
// TODO: Handle || (or) characters and backslashes.
// TODO: Add more modifiers.

(function() {               // Using a closure to keep global namespace clean.
    if (TrimPath == null)
        TrimPath = new Object();
    if (TrimPath.evalEx == null)
        TrimPath.evalEx = function(src) { return eval(src); };

    var UNDEFINED;
    if (Array.prototype.pop == null)  // IE 5.x fix from Igor Poteryaev.
        Array.prototype.pop = function() {
            if (this.length === 0) {return UNDEFINED;}
            return this[--this.length];
        };
    if (Array.prototype.push == null) // IE 5.x fix from Igor Poteryaev.
        Array.prototype.push = function() {
            for (var i = 0; i < arguments.length; ++i) {this[this.length] = arguments[i];}
            return this.length;
        };

    TrimPath.parseTemplate = function(tmplContent, optTmplName, optEtc) {
        if (optEtc == null)
            optEtc = TrimPath.parseTemplate_etc;
        var funcSrc = parse(tmplContent, optTmplName, optEtc);
        var func = TrimPath.evalEx(funcSrc, optTmplName, 1);
        if (func != null)
            return new optEtc.Template(optTmplName, tmplContent, funcSrc, func, optEtc);
        return null;
    }
    
    try {
        String.prototype.process = function(context, optFlags) {
            var template = TrimPath.parseTemplate(this, null);
            if (template != null)
                return template.process(context, optFlags);
            return this;
        }
    } catch (e) { // Swallow exception, such as when String.prototype is sealed.
    }
    
    TrimPath.parseTemplate_etc = {};            // Exposed for extensibility.
    TrimPath.parseTemplate_etc.statementTag = "forelse|for|if|elseif|else|var|macro";
    TrimPath.parseTemplate_etc.statementDef = { // Lookup table for statement tags.
        "if"     : { delta:  1, prefix: "if (", suffix: ") {", paramMin: 1 },
        "else"   : { delta:  0, prefix: "} else {" },
        "elseif" : { delta:  0, prefix: "} else if (", suffix: ") {", paramDefault: "true" },
        "/if"    : { delta: -1, prefix: "}" },
        "for"    : { delta:  1, paramMin: 3, 
                     prefixFunc : function(stmtParts, state, tmplName, etc) {
                        if (stmtParts[2] != "in")
                            throw new etc.ParseError(tmplName, state.line, "bad for loop statement: " + stmtParts.join(' '));
                        var iterVar = stmtParts[1];
                        var listVar = "__LIST__" + iterVar;
                        return [ "var ", listVar, " = ", stmtParts[3], ";",
                             // Fix from Ross Shaull for hash looping, make sure that we have an array of loop lengths to treat like a stack.
                             "var __LENGTH_STACK__;",
                             "if (typeof(__LENGTH_STACK__) == 'undefined' || !__LENGTH_STACK__.length) __LENGTH_STACK__ = new Array();", 
                             "__LENGTH_STACK__[__LENGTH_STACK__.length] = 0;", // Push a new for-loop onto the stack of loop lengths.
                             "if ((", listVar, ") != null) { ",
                             "var ", iterVar, "_ct = 0;",       // iterVar_ct variable, added by B. Bittman     
                             "for (var ", iterVar, "_index in ", listVar, ") { ",
                             iterVar, "_ct++;",
                             "if (typeof(", listVar, "[", iterVar, "_index]) == 'function') {continue;}", // IE 5.x fix from Igor Poteryaev.
                             "__LENGTH_STACK__[__LENGTH_STACK__.length - 1]++;",
                             "var ", iterVar, " = ", listVar, "[", iterVar, "_index];" ].join("");
                     } },
        "forelse" : { delta:  0, prefix: "} } if (__LENGTH_STACK__[__LENGTH_STACK__.length - 1] == 0) { if (", suffix: ") {", paramDefault: "true" },
        "/for"    : { delta: -1, prefix: "} }; delete __LENGTH_STACK__[__LENGTH_STACK__.length - 1];" }, // Remove the just-finished for-loop from the stack of loop lengths.
        "var"     : { delta:  0, prefix: "var ", suffix: ";" },
        "macro"   : { delta:  1, 
                      prefixFunc : function(stmtParts, state, tmplName, etc) {
                          var macroName = stmtParts[1].split('(')[0];
                          return [ "var ", macroName, " = function", 
                                   stmtParts.slice(1).join(' ').substring(macroName.length),
                                   "{ var _OUT_arr = []; var _OUT = { write: function(m) { if (m) _OUT_arr.push(m); } }; " ].join('');
                     } }, 
        "/macro"  : { delta: -1, prefix: " return _OUT_arr.join(''); };" }
    }
    TrimPath.parseTemplate_etc.modifierDef = {
        "eat"        : function(v)    { return ""; },
        "escape"     : function(s)    { return String(s).replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;"); },
        "capitalize" : function(s)    { return String(s).toUpperCase(); },
        "default"    : function(s, d) { return s != null ? s : d; }
    }
    TrimPath.parseTemplate_etc.modifierDef.h = TrimPath.parseTemplate_etc.modifierDef.escape;

    TrimPath.parseTemplate_etc.Template = function(tmplName, tmplContent, funcSrc, func, etc) {
        this.process = function(context, flags) {
            if (context == null)
                context = {};
            if (context._MODIFIERS == null)
                context._MODIFIERS = {};
            if (context.defined == null)
                context.defined = function(str) { return (context[str] != undefined); };
            for (var k in etc.modifierDef) {
                if (context._MODIFIERS[k] == null)
                    context._MODIFIERS[k] = etc.modifierDef[k];
            }
            if (flags == null)
                flags = {};
            var resultArr = [];
            var resultOut = { write: function(m) { resultArr.push(m); } };
            try {
                func(resultOut, context, flags);
            } catch (e) {
                if (flags.throwExceptions == true)
                    throw e;
                var result = new String(resultArr.join("") + "[ERROR: " + e.toString() + (e.message ? '; ' + e.message : '') + "]");
                result["exception"] = e;
                return result;
            }
            return resultArr.join("");
        }
        this.name       = tmplName;
        this.source     = tmplContent; 
        this.sourceFunc = funcSrc;
        this.toString   = function() { return "TrimPath.Template [" + tmplName + "]"; }
    }
    TrimPath.parseTemplate_etc.ParseError = function(name, line, message) {
        this.name    = name;
        this.line    = line;
        this.message = message;
    }
    TrimPath.parseTemplate_etc.ParseError.prototype.toString = function() { 
        return ("TrimPath template ParseError in " + this.name + ": line " + this.line + ", " + this.message);
    }
    
    var parse = function(body, tmplName, etc) {
        body = cleanWhiteSpace(body);
        var funcText = [ "var TrimPath_Template_TEMP = function(_OUT, _CONTEXT, _FLAGS) { with (_CONTEXT) {" ];
        var state    = { stack: [], line: 1 };                              // TODO: Fix line number counting.
        var endStmtPrev = -1;
        while (endStmtPrev + 1 < body.length) {
            var begStmt = endStmtPrev;
            // Scan until we find some statement markup.
            begStmt = body.indexOf("{", begStmt + 1);
            while (begStmt >= 0) {
                var endStmt = body.indexOf('}', begStmt + 1);
                var stmt = body.substring(begStmt, endStmt);
                var blockrx = stmt.match(/^\{(cdata|minify|eval)/); // From B. Bittman, minify/eval/cdata implementation.
                if (blockrx) {
                    var blockType = blockrx[1]; 
                    var blockMarkerBeg = begStmt + blockType.length + 1;
                    var blockMarkerEnd = body.indexOf('}', blockMarkerBeg);
                    if (blockMarkerEnd >= 0) {
                        var blockMarker;
                        if( blockMarkerEnd - blockMarkerBeg <= 0 ) {
                            blockMarker = "{/" + blockType + "}";
                        } else {
                            blockMarker = body.substring(blockMarkerBeg + 1, blockMarkerEnd);
                        }                        
                        
                        var blockEnd = body.indexOf(blockMarker, blockMarkerEnd + 1);
                        if (blockEnd >= 0) {                            
                            emitSectionText(body.substring(endStmtPrev + 1, begStmt), funcText);
                            
                            var blockText = body.substring(blockMarkerEnd + 1, blockEnd);
                            if (blockType == 'cdata') {
                                emitText(blockText, funcText);
                            } else if (blockType == 'minify') {
                                emitText(scrubWhiteSpace(blockText), funcText);
                            } else if (blockType == 'eval') {
                                if (blockText != null && blockText.length > 0) // From B. Bittman, eval should not execute until process().
                                    funcText.push('_OUT.write( (function() { ' + blockText + ' })() );');
                            }
                            begStmt = endStmtPrev = blockEnd + blockMarker.length - 1;
                        }
                    }                        
                } else if (body.charAt(begStmt - 1) != '$' &&               // Not an expression or backslashed,
                           body.charAt(begStmt - 1) != '\\') {              // so check if it is a statement tag.
                    var offset = (body.charAt(begStmt + 1) == '/' ? 2 : 1); // Close tags offset of 2 skips '/'.
                                                                            // 10 is larger than maximum statement tag length.
                    if (body.substring(begStmt + offset, begStmt + 10 + offset).search(TrimPath.parseTemplate_etc.statementTag) == 0) 
                        break;                                              // Found a match.
                }
                begStmt = body.indexOf("{", begStmt + 1);
            }
            if (begStmt < 0)                              // In "a{for}c", begStmt will be 1.
                break;
            var endStmt = body.indexOf("}", begStmt + 1); // In "a{for}c", endStmt will be 5.
            if (endStmt < 0)
                break;
            emitSectionText(body.substring(endStmtPrev + 1, begStmt), funcText);
            emitStatement(body.substring(begStmt, endStmt + 1), state, funcText, tmplName, etc);
            endStmtPrev = endStmt;
        }
        emitSectionText(body.substring(endStmtPrev + 1), funcText);
        if (state.stack.length != 0)
            throw new etc.ParseError(tmplName, state.line, "unclosed, unmatched statement(s): " + state.stack.join(","));
        funcText.push("}}; TrimPath_Template_TEMP");
        return funcText.join("");
    }
    
    var emitStatement = function(stmtStr, state, funcText, tmplName, etc) {
        var parts = stmtStr.slice(1, -1).split(' ');
        var stmt = etc.statementDef[parts[0]]; // Here, parts[0] == for/if/else/...
        if (stmt == null) {                    // Not a real statement.
            emitSectionText(stmtStr, funcText);
            return;
        }
        if (stmt.delta < 0) {
            if (state.stack.length <= 0)
                throw new etc.ParseError(tmplName, state.line, "close tag does not match any previous statement: " + stmtStr);
            state.stack.pop();
        } 
        if (stmt.delta > 0)
            state.stack.push(stmtStr);

        if (stmt.paramMin != null &&
            stmt.paramMin >= parts.length)
            throw new etc.ParseError(tmplName, state.line, "statement needs more parameters: " + stmtStr);
        if (stmt.prefixFunc != null)
            funcText.push(stmt.prefixFunc(parts, state, tmplName, etc));
        else 
            funcText.push(stmt.prefix);
        if (stmt.suffix != null) {
            if (parts.length <= 1) {
                if (stmt.paramDefault != null)
                    funcText.push(stmt.paramDefault);
            } else {
                for (var i = 1; i < parts.length; i++) {
                    if (i > 1)
                        funcText.push(' ');
                    funcText.push(parts[i]);
                }
            }
            funcText.push(stmt.suffix);
        }
    }

    var emitSectionText = function(text, funcText) {
        if (text.length <= 0)
            return;
        var nlPrefix = 0;               // Index to first non-newline in prefix.
        var nlSuffix = text.length - 1; // Index to first non-space/tab in suffix.
        while (nlPrefix < text.length && (text.charAt(nlPrefix) == '\n'))
            nlPrefix++;
        while (nlSuffix >= 0 && (text.charAt(nlSuffix) == ' ' || text.charAt(nlSuffix) == '\t'))
            nlSuffix--;
        if (nlSuffix < nlPrefix)
            nlSuffix = nlPrefix;
        if (nlPrefix > 0) {
            funcText.push('if (_FLAGS.keepWhitespace == true) _OUT.write("');
            var s = text.substring(0, nlPrefix).replace('\n', '\\n'); // A macro IE fix from BJessen.
            if (s.charAt(s.length - 1) == '\n')
            	s = s.substring(0, s.length - 1);
            funcText.push(s);
            funcText.push('");');
        }
        var lines = text.substring(nlPrefix, nlSuffix + 1).split('\n');
        for (var i = 0; i < lines.length; i++) {
            emitSectionTextLine(lines[i], funcText);
            if (i < lines.length - 1)
                funcText.push('_OUT.write("\\n");\n');
        }
        if (nlSuffix + 1 < text.length) {
            funcText.push('if (_FLAGS.keepWhitespace == true) _OUT.write("');
            var s = text.substring(nlSuffix + 1).replace('\n', '\\n');
            if (s.charAt(s.length - 1) == '\n')
            	s = s.substring(0, s.length - 1);
            funcText.push(s);
            funcText.push('");');
        }
    }
    
    var emitSectionTextLine = function(line, funcText) {
        var endMarkPrev = '}';
        var endExprPrev = -1;
        while (endExprPrev + endMarkPrev.length < line.length) {
            var begMark = "${", endMark = "}";
            var begExpr = line.indexOf(begMark, endExprPrev + endMarkPrev.length); // In "a${b}c", begExpr == 1
            if (begExpr < 0)
                break;
            if (line.charAt(begExpr + 2) == '%') {
                begMark = "${%";
                endMark = "%}";
            }
            var endExpr = line.indexOf(endMark, begExpr + begMark.length);         // In "a${b}c", endExpr == 4;
            if (endExpr < 0)
                break;
            emitText(line.substring(endExprPrev + endMarkPrev.length, begExpr), funcText);                
            // Example: exprs == 'firstName|default:"John Doe"|capitalize'.split('|')
            var exprArr = line.substring(begExpr + begMark.length, endExpr).replace(/\|\|/g, "#@@#").split('|');
            for (var k in exprArr) {
                if (exprArr[k].replace) // IE 5.x fix from Igor Poteryaev.
                    exprArr[k] = exprArr[k].replace(/#@@#/g, '||');
            }
            funcText.push('_OUT.write(');
            emitExpression(exprArr, exprArr.length - 1, funcText); 
            funcText.push(');');
            endExprPrev = endExpr;
            endMarkPrev = endMark;
        }
        emitText(line.substring(endExprPrev + endMarkPrev.length), funcText); 
    }
    
    var emitText = function(text, funcText) {
        if (text == null ||
            text.length <= 0)
            return;
        text = text.replace(/\\/g, '\\\\');
        text = text.replace(/\n/g, '\\n');
        text = text.replace(/"/g,  '\\"');
        funcText.push('_OUT.write("');
        funcText.push(text);
        funcText.push('");');
    }
    
    var emitExpression = function(exprArr, index, funcText) {
        // Ex: foo|a:x|b:y1,y2|c:z1,z2 is emitted as c(b(a(foo,x),y1,y2),z1,z2)
        var expr = exprArr[index]; // Ex: exprArr == [firstName,capitalize,default:"John Doe"]
        if (index <= 0) {          // Ex: expr    == 'default:"John Doe"'
            funcText.push(expr);
            return;
        }
        var parts = expr.split(':');
        funcText.push('_MODIFIERS["');
        funcText.push(parts[0]); // The parts[0] is a modifier function name, like capitalize.
        funcText.push('"](');
        emitExpression(exprArr, index - 1, funcText);
        if (parts.length > 1) {
            funcText.push(',');
            funcText.push(parts[1]);
        }
        funcText.push(')');
    }

    var cleanWhiteSpace = function(result) {
        result = result.replace(/\t/g,   "    ");
        result = result.replace(/\r\n/g, "\n");
        result = result.replace(/\r/g,   "\n");
        result = result.replace(/^(\s*\S*(\s+\S+)*)\s*$/, '$1'); // Right trim by Igor Poteryaev.
        return result;
    }

    var scrubWhiteSpace = function(result) {
        result = result.replace(/^\s+/g,   "");
        result = result.replace(/\s+$/g,   "");
        result = result.replace(/\s+/g,   " ");
        result = result.replace(/^(\s*\S*(\s+\S+)*)\s*$/, '$1'); // Right trim by Igor Poteryaev.
        return result;
    }

    // The DOM helper functions depend on DOM/DHTML, so they only work in a browser.
    // However, these are not considered core to the engine.
    //
    TrimPath.parseDOMTemplate = function(elementId, optDocument, optEtc) {
        if (optDocument == null)
            optDocument = document;
        var element = optDocument.getElementById(elementId);
        var content = element.value;     // Like textarea.value.
        if (content == null)
            content = element.innerHTML; // Like textarea.innerHTML.
        content = content.replace(/&lt;/g, "<").replace(/&gt;/g, ">");
        return TrimPath.parseTemplate(content, elementId, optEtc);
    }

    TrimPath.processDOMTemplate = function(elementId, context, optFlags, optDocument, optEtc) {
        return TrimPath.parseDOMTemplate(elementId, optDocument, optEtc).process(context, optFlags);
    }
}) ();

/**
* A wrapper class around jst templates
* (http://trimpath.com/project/wiki/JavaScriptTemplates)
* the wrapper includes special methods for quoting xml correctly and hyves fancy layout
* requires fancy_layout.js and header.js
*/

//global translation dict, is added to by jstitem tag
var _translation = {};
var _constant = {};

function _translate(key, data) { 
	if(_translation[key] != undefined) {
		if(typeof(_translation[key]) == 'string') {
			return _translation[key];
		}
		else {
			return _translation[key].process(data);
		}
	}
	else  {
		return '@' + key + '@';
	}
}

translateModifier = function(key)
{
	data = {};
	for(var i = 1; i < arguments.length; i+=2) {
		var arg = arguments[i];
		var val = arguments[i+1];
		data[arg] = val;
	}
	return _translate(key, data);
}

truncateModifier = function(s, len)
{
	if(s.length > len) {
		return s.substring(0, len) + "...";
	}
	else {
		return s;
	}
}

var fancyLayoutCache = new Object();

fancyLayout = function(s, type)
{
	var noCache = false;
	var key = "";
	for(var i = 0; i < arguments.length; i++) {
		if(arguments[i]) {
			key += arguments[i].toString();
		}
	}

    if(startsWith(s, '[gadget]')) { //skip cache
        noCache = true;
    }
    
	
	if(noCache || !fancyLayoutCache[key]) {
		if(type) {
			type = type.replace("!", "|");
		}
		fancyLayoutCache[key] = fancy_layout(s, type);
	}
		
	return fancyLayoutCache[key];
}

function _Template(name, code) {
	this.name = name;
	this.code = code;
	this.template = null;
}

_Template.prototype._MODIFIERS = null;

_Template.prototype.process = function(data)
{
	if(this.template == null) {
		try {
			this.template = TrimPath.parseTemplate(this.code);
		}
		catch(e) {
			logging.error("error while parsing template:" + this.name + ": " + e, e);
			return "";
		}
	}

	if(data == undefined) {
		data = {}
	}

	if(!this._MODIFIERS) {
		this._MODIFIERS = {
			quoteElement: quoteElement,
			quoteAttr: quoteAttr,
    		fancyLayout: fancyLayout,
    		fancy_layout: fancyLayout,
    		translate: translateModifier,
    		truncate: truncateModifier
		}
	} 	
	data._MODIFIERS = this._MODIFIERS;
                      
	data.throwExceptions = true;
	try {
		return this.template.process(data, data);
	}
	catch(e) {
		logging.error("error while processing template:" + this.name + ": " + e, e);
		return "";
	}
}


var templateCache = new Object();
/**
 * do not use this method it is prives, use the namespace below
 */
function _getTemplate(id)
{
	if(!templateCache[id]) {
		templateCache[id] = new _Template(id, id);
	}
	return templateCache[id];
}

/**
 * if delay is true, delays execution of func until target exists in DOM
 * checks twice for target, once after 10ms and if not exist
 * a second time after 200ms
 */
function _delay(delay, target, func)
{
	if(delay) {
		setTimeout(function() {
			if($(target)) {
				func();
			}
			else {
				setTimeout(function() {
					if($(target)) {
						func();
					}
				}, 200);
			}
		}, 10);
	}
	else {
		func();
	}
}

/**
 * Please use only function from the following namespace:
 */
templates = {
	get: function(id)
	{
		return _getTemplate(id);
	},
	addFromString: function(id, content)
	{
		templateCache[id] = new _Template(id, content);
	},
	addTranslation: function(key, content) {
		_translation[key] = new _Template(key, content);
	},
	addConstant: function(key, content) {
		_constant[key] = content;
	},
	/**
	 * updates the given target DOM element by executing given
	 * template on given data. 
	 * If delay is true, the updating will wait until element target
	 * is available in DOM. optionally func will be called at that time
	 */
	updateElement: function(target, template, data, delay, func) {
		_delay(delay, target, function() {
			try {
				$(target).innerHTML = templates.get(template).process(data);
				if(func) {
					func();
				}
			}
			catch(e) {
				logging.error('error processing template ' + template + ': ' + e, e);
			}
		});
	},
	insertBottom: function(target, template, data) {
		new Insertion.Bottom(target, templates.get(template).process(data));
	},
	insertTop: function(target, template, data) {
		new Insertion.Top(target, templates.get(template).process(data));
	},
	insertBefore: function(target, template, data) {
		new Insertion.Before(target, templates.get(template).process(data));
	},
	process: function(template, data) {
		return templates.get(template).process(data);
	},
	translate: function(key, data) {
		return _translate(key, data);
	},
	exists: function(key) {
		if(templateCache[key]) {
			return true;
		}
		else {
			return false;
		}
	},
	constant: function(key) {
		if(_constant[key] != undefined) {
			return _constant[key];
		}
		else  {
			return '#' + key + '#';
		}
	},
	//recompile/reload a template (development only)
	reload: function(id) {
		var url = "/"
		var pars = "module=Utility&action=compileTemplate&id=" + id;
	
		var myAjax = new Ajax.Request(url, 
		{
			asynchronous: false,
			method: 'get', 
			parameters: pars,
			onSuccess: function(transport) {
				eval(transport.responseText);
			}
		});
	}
}





/*
Macromedia(r) Flash(r) JavaScript Integration Kit License


Copyright (c) 2005 Macromedia, inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

3. The end-user documentation included with the redistribution, if any, must
include the following acknowledgment:

"This product includes software developed by Macromedia, Inc.
(http://www.macromedia.com)."

Alternately, this acknowledgment may appear in the software itself, if and
wherever such third-party acknowledgments normally appear.

4. The name Macromedia must not be used to endorse or promote products derived
from this software without prior written permission. For written permission,
please contact devrelations@macromedia.com.

5. Products derived from this software may not be called "Macromedia" or
"Macromedia Flash", nor may "Macromedia" or "Macromedia Flash" appear in their
name.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MACROMEDIA OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.

--

This code is part of the Flash / JavaScript Integration Kit:
http://www.macromedia.com/go/flashjavascript/

Created by:

Christian Cantrell
http://weblogs.macromedia.com/cantrell/
mailto:cantrell@macromedia.com

Mike Chambers
http://weblogs.macromedia.com/mesh/
mailto:mesh@macromedia.com

Macromedia
*/

/**
 * Create a new Exception object.
 * name: The name of the exception.
 * message: The exception message.
 */
function Exception(name, message)
{
    if (name)
        this.name = name;
    if (message)
        this.message = message;
}

/**
 * Set the name of the exception. 
 */
Exception.prototype.setName = function(name)
{
    this.name = name;
}

/**
 * Get the exception's name. 
 */
Exception.prototype.getName = function()
{
    return this.name;
}

/**
 * Set a message on the exception. 
 */
Exception.prototype.setMessage = function(msg)
{
    this.message = msg;
}

/**
 * Get the exception message. 
 */
Exception.prototype.getMessage = function()
{
    return this.message;
}

/**
 * Generates a browser-specific Flash tag. Create a new instance, set whatever
 * properties you need, then call either toString() to get the tag as a string, or
 * call write() to write the tag out.
 *
 * The properties src, width, height and version are required when creating a new
 * instance of the FlashTag, but there are setters to let you change them, as well.
 * That way, if you want to render more than one piece of Flash content, you can do
 * so without creating a new instance of the tag.
 *
 * For more information on supported parameters, see:
 * http://www.macromedia.com/cfusion/knowledgebase/index.cfm?id=tn_12701
 */


/**
 * Creates a new instance of the FlashTag.
 * src: The path to the SWF file.
 * width: The width of your Flash content.
 * height: the height of your Flash content.
 * version: the required version of the Flash Player for the specified content.
 *
 * These are the only 4 properites that are required.
 */
function FlashTag(src, width, height, version)
{
    if (arguments.length < 4)
    {
        throw new Exception('RequiredParameterException',
                            'You must pass in a src, width, height, and version when creating a FlashTag.');
    }

    // Required
    this.src            =  src;
    this.width          =  width;
    this.height         =  height;
    this.version        =  version;

    this.id             =  null;
    this.flashVars      =  null;
    this.flashVarsStr   =  null;
    this.genericParam   = new Object();
    this.ie = (navigator.appName.indexOf ("Microsoft") != -1) ? 1 : 0;
	this.useEmbedTagOnly = false;	
}

/**
 * Set this to true if you use an swf with Stage set to noScale. MSIE has a bug that causes the flash movie
 * not to show for these conditions when you use the object tag. Don't ask :/
 * 
 * @param {Object} value
 */
FlashTag.prototype.setUseEmbedTagOnly = function(value)
{
	this.useEmbedTagOnly = value;
}

/**
 * Specifies the location (URL) of the Flash content to be loaded.
 */
FlashTag.prototype.setSource = function(src)
{
    this.src = src; 
}

/**
 * Specifies the width of the Flash content in either pixels or percentage of browser window. 
 */
FlashTag.prototype.setWidth = function(w)
{
    this.width = width; 
}

/**
 * Specifies the height of the Flash content in either pixels or percentage of browser window. 
 */
FlashTag.prototype.setHeight = function(h)
{
    this.h = height; 
}

/**
 * The required version of the Flash Player for the specified content. 
 */
FlashTag.prototype.setVersion = function(v)
{
    this.version = v;
}

/**
 * Identifies the Flash content to the host environment (a web browser, for example) so that
 * it can be referenced using a scripting language. This value will be used for both the 'id'
 * and 'name' attributes depending on the client platform and whether the object or the embed
 * tag are used.
 */
FlashTag.prototype.setId = function(id)
{
    this.id = id;
}

/**
 * Specifies the background color of the Flash content. Use this attribute to override the background
 * color setting specified in the Flash file. This attribute does not affect the background
 * color of the HTML page. 
 */
FlashTag.prototype.setBgcolor = function(bgc)
{
    if (bgc.charAt(0) != '#')
    {
        bgc = '#' + bgc;
    }
    this.genericParam['bgcolor'] = bgc;
}

/**
 * Allows you to set multiple Flash vars at once rather than adding them one at a time. The string
 * you pass in should contain all your Flash vars, properly URL encoded. This function can be used in
 * conjunction with addFlashVar.
 */
FlashTag.prototype.addFlashVars = function(fvs)
{
    this.flashVarsStr = fvs;
}

/**
 * Used to send root level variables to the Flash content. You can add as many name/value pairs as
 * you want. The formatting of the Flash vars (turning them into a query string) is handled automatically.
 */
FlashTag.prototype.addFlashVar = function(n, v)
{
    if (this.flashVars == null)
    {
        this.flashVars = new Object();
    }

    this.flashVars[n] = v;
}

/**
 * Used to remove Flash vars. This is primarily useful if you want to reuse an instance of the FlashTag
 * but you don't want to send the same variables to more than one piece of Flash content. 
 */
FlashTag.prototype.removeFlashVar = function(n)
{
    if (this.flashVars != null)
    {
        this.flashVars[n] = null;
    }
}

/**
 * (true, false) Specifies whether the browser should start Java when loading the Flash Player for the first time.
 * The default value is false if this property is not set. 
 */
FlashTag.prototype.setSwliveconnect = function(swlc)
{
    this.genericParam['swliveconnect'] = swlc;
}

/**
 * (true, false) Specifies whether the Flash content begins playing immediately on loading in the browser.
 * The default value is true if this property is not set. 
 */
FlashTag.prototype.setPlay = function(p)
{
    this.genericParam['play'] = p;
}

/**
 * (true, false) Specifies whether the Flash content repeats indefinitely or stops when it reaches the last frame.
 * The default value is true if this property is not set. 
 */
FlashTag.prototype.setLoop = function(l)
{
    this.genericParam['loop'] = l;
}

/**
 * (true,false) Whether or not to display the full Flash menu. If false, displays a menu that contains only
 * the Settings and the About Flash options. 
 */
FlashTag.prototype.setMenu = function(m)
{
    this.genericParam['menu'] = m;
}

/**
 * 
 * Set allownetworking
 * Different values
 * all: No networking restrictions. Flash Player behaves normally. This is the default.
 * internal: SWF files cannot call browser navigation or browser interaction APIs (such as the ExternalInterface.call(), fscommand(), and navigateToURL() methods), but can call other networking APIs.
 * none: SWF files cannot call networking or SWF-to-SWF file communication APIs. In addition to the APIs restricted by the internal value, these include other methods such as URLLoader.load(), Security.loadPolicyFile(), and SharedObject.getLocal().
 */
FlashTag.prototype.setAllowNetworking = function(m)
{
    this.genericParam['AllowNetworking'] = m;
}

/**
 * (low, high, autolow, autohigh, best) Sets the quality at which the Flash content plays.
 */
FlashTag.prototype.setQuality = function(q)
{
    if (q != 'low' && q != 'high' && q != 'autolow' && q != 'autohigh' && q != 'best')
    {
        throw new Exception('UnsupportedValueException',
                            'Supported values are "low", "high", "autolow", "autohigh", and "best".');
    }
    this.genericParam['quality'] = q;
}

/**
 * (showall, noborder, exactfit) Determins how the Flash content scales.
 */
FlashTag.prototype.setScale = function(sc)
{
    if (sc != 'showall' && sc != 'noborder' && sc != 'exactfit')
    {
        throw new Exception('UnsupportedValueException',
                            'Supported values are "showall", "noborder", and "exactfit".');
    }
    this.genericParam['scale'] = sc;
}

/**
 * (l, t, r, b) Align the Flash content along the corresponding edge of the browser window and crop
 * the remaining three sides as needed.
 */
FlashTag.prototype.setAlign= function(a)
{
    if (a != 'l' && a != 't' && a != 'r' && a != 'b')
    {
        throw new Exception('UnsupportedValueException',
                            'Supported values are "l", "t", "r" and "b".');
    }
    this.genericParam['align'] = a;
}

/**
 * (l, t, r, b, tl, tr, bl, br) Align the Flash content along the corresponding edge of the browser
 * window and crop the remaining three sides as needed.
 */
FlashTag.prototype.setSalign= function(sa)
{
    if (sa != 'l' && sa != 't' && sa != 'r' && sa != 'b' && sa != 'tl' && sa != 'tr' && sa != 'bl' && sa != 'br')
    {
        throw new Exception('UnsupportedValueException',
                            'Supported values are "l", "t", "r", "b", "tl", "tr", "bl" and "br".');
    }
    this.genericParam['salign'] = sa;
}

/**
 * (window, opaque, transparent) Sets the Window Mode property of the Flash content for transparency,
 * layering, and positioning in the browser. 
 */
FlashTag.prototype.setWmode = function(wm)
{
    if (wm != 'window' && wm != 'opaque' && wm != 'transparent')
    {
        throw new Exception('UnsupportedValueException',
                            'Supported values are "window", "opaque", and "transparent".');
    }
    this.genericParam['wmode'] = wm;
}

/**
 * Specifies the base directory or URL used to resolve all relative path statements in your Flash content.
 */
FlashTag.prototype.setBase = function(base)
{
    this.genericParam['base'] = base;
}

/**
 * (never, always) Controls the ability to perform outbound scripting from within Flash content. 
 */
FlashTag.prototype.setAllowScriptAccess = function(sa)
{
    if (sa != 'never' && sa != 'always' && sa != 'samedomain')
    {
        throw new Exception('UnsupportedValueException',
                            'Supported values are "never" and "always".');
    }
    this.genericParam['allowScriptAccess'] = sa;
}

/**
 * Get the Flash tag as a string. 
 */
FlashTag.prototype.toString = function()
{
    var flashTag = new String();
    if (this.ie && !this.useEmbedTagOnly)
    {
        flashTag += '<object classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000" ';
        if (this.id != null)
        {
            flashTag += 'id="'+this.id+'" ';
        }
        //HP 13-03-2008 added correct protocol to keep ie from complaining about mixed content
        flashTag += 'codebase="' + window.location.protocol + '//download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version='+this.version+'" ';
        flashTag += 'width="'+this.width+'" ';
        flashTag += 'height="'+this.height+'">';
        flashTag += '<param name="movie" value="'+this.src+'"/>';

        for (var n in this.genericParam)
        {
            if (this.genericParam[n] != null)
            {
                flashTag += '<param name="'+n+'" value="'+this.genericParam[n]+'"/>';
            }
        }

        if (this.flashVars != null)
        {
            var fv = this.getFlashVarsAsString();
            if (fv.length > 0)
            {
                flashTag += '<param name="flashvars" value="'+fv+'"/>';
            }
        }
        flashTag += '</object>';
    }
    else
    {
        flashTag += '<embed src="'+this.src+'"';
        flashTag += ' width="'+this.width+'"';
        flashTag += ' height="'+this.height+'"';
        flashTag += ' type="application/x-shockwave-flash"';
        if (this.id != null)
        {
            flashTag += ' name="'+this.id+'"';
            flashTag += ' id="'+this.id+'"';
        }

        for (var n in this.genericParam)
        {
            if (this.genericParam[n] != null)
            {
                flashTag += (' '+n+'="'+this.genericParam[n]+'"');
            }
        }

        if (this.flashVars != null)
        {
            var fv = this.getFlashVarsAsString();
            if (fv.length > 0)
            {
                flashTag += ' flashvars="'+fv+'"';
            }
        }
        flashTag += ' pluginspage="http://www.macromedia.com/go/getflashplayer">';
        flashTag += '</embed>';
    }
    return flashTag;
}

/**
 * Write the Flash tag out. Pass in a reference to the document to write to. 
 */
FlashTag.prototype.write = function(doc)
{
	if (doc.write)
    	doc.write(this.toString());
    else
    	doc.innerHTML = this.toString();
}

/**
 * Write the Flash tag out. Pass in a reference to the document to write to. 
 */
FlashTag.prototype.getFlashVarsAsString = function()
{
    var qs = new String();
    for (var n in this.flashVars)
    {
        if (this.flashVars[n] != null)
        {
            qs += (encodeURL(n)+'='+encodeURL(this.flashVars[n])+'&');
        }
    }

    if (this.flashVarsStr != null)
    {
        return qs + this.flashVarsStr;
    }

    return qs.substring(0, qs.length-1);
}

/*  Function Equivalent to java.net.URLEncoder.encode(String, "UTF-8")

	published at http://www.cresc.co.jp/tech/java/URLencoding/JavaScript_URLEncoding.htm
    
    Copyright (C) 2002, Cresc Corp.

    Version: 1.0

*/
function encodeURL(str) {
	str = str.toString();
    var s0, i, s, u;
    s0 = "";                // encoded str
    for (i = 0; i < str.length; i++){   // scan the source
        s = str.charAt(i);
        u = str.charCodeAt(i);          // get unicode of the char
        if (s == " ") {
        	s0 += "+";
        }       // SP should be converted to "+"
        else {
            if ( u == 0x2a || u == 0x2d || u == 0x2e || u == 0x5f || 
            	((u >= 0x30) && (u <= 0x39)) || 
            	((u >= 0x41) && (u <= 0x5a)) || 
            	((u >= 0x61) && (u <= 0x7a)))  {       // check for escape
                s0 = s0 + s;            // don't escape
            }
            else {                  // escape
                if ((u >= 0x0) && (u <= 0x7f)){     // single byte format
                    s = "0"+u.toString(16);
                    s0 += "%"+ s.substr(s.length-2);
                }
                else if (u > 0x1fffff){     // quaternary byte format (extended)
                    s0 += "%" + (oxf0 + ((u & 0x1c0000) >> 18)).toString(16);
                    s0 += "%" + (0x80 + ((u & 0x3f000) >> 12)).toString(16);
                    s0 += "%" + (0x80 + ((u & 0xfc0) >> 6)).toString(16);
                    s0 += "%" + (0x80 + (u & 0x3f)).toString(16);
                }
                else if (u > 0x7ff){        // triple byte format
                    s0 += "%" + (0xe0 + ((u & 0xf000) >> 12)).toString(16);
                    s0 += "%" + (0x80 + ((u & 0xfc0) >> 6)).toString(16);
                    s0 += "%" + (0x80 + (u & 0x3f)).toString(16);
                }
                else {                      // double byte format
                    s0 += "%" + (0xc0 + ((u & 0x7c0) >> 6)).toString(16);
                    s0 += "%" + (0x80 + (u & 0x3f)).toString(16);
                }
            }
        }
    }
    return s0;
}

 
 
 

/*  Function Equivalent to java.net.URLDecoder.decode(String, "UTF-8")

	published at http://www.cresc.co.jp/tech/java/URLencoding/JavaScript_URLEncoding.htm
	
    Copyright (C) 2002, Cresc Corp.

    Version: 1.0

*/

function decodeURL(str) {
    var s0, i, j, s, ss, u, n, f;
    s0 = "";                // decoded str
    for (i = 0; i < str.length; i++) {   // scan the source str
        s = str.charAt(i);
        if (s == "+") {
        	s0 += " ";
        }       // "+" should be changed to SP
        else {
            if (s != "%"){
            	s0 += s;
            }     // add an unescaped char
            else{               // escape sequence decoding
                u = 0;          // unicode of the character
                f = 1;          // escape flag, zero means end of this sequence
                while (true) {
                    ss = "";        // local str to parse as int
                    for (j = 0; j < 2; j++ ) {  // get two maximum hex characters for parse
                        sss = str.charAt(++i);
                        if (((sss >= "0") && (sss <= "9")) || 
                        	((sss >= "a") && (sss <= "f")) || 
                        	((sss >= "A") && (sss <= "F")))  {
                                ss += sss;      // if hex, add the hex character
                            } 
                            else {
                                --i; 
                                break;
                            }    // not a hex char., exit the loop
                        }
                    n = parseInt(ss, 16);           // parse the hex str as byte
                    if (n <= 0x7f)                  {u = n; f = 1;}   // single byte format
                    if ((n >= 0xc0) && (n <= 0xdf)) {u = n & 0x1f; f = 2;}   // double byte format
                    if ((n >= 0xe0) && (n <= 0xef)) {u = n & 0x0f; f = 3;}   // triple byte format
                    if ((n >= 0xf0) && (n <= 0xf7)) {u = n & 0x07; f = 4;}   // quaternary byte format (extended)
                    if ((n >= 0x80) && (n <= 0xbf)) {u = (u << 6) + (n & 0x3f); --f;} // not a first, shift and add 6 lower bits
                    if (f <= 1){break;}         // end of the utf byte sequence
                    if (str.charAt(i + 1) == "%"){ i++ ;} // test for the next shift byte
                    else {break;}                   // abnormal, format error
                }
            s0 += String.fromCharCode(u);           // add the escaped character
            }
        }
    }
    return s0;

}

if(window.jsloaded) {jsloaded('/statics/FlashTag.js')}

if(window.jsloaded) {jsloaded('/precompiled/misc.js'); }
var heightSearchPane = 20;
var heightCheckPane = 20;


var global_amazon_categories = new Object();
var global_search_categories = new Object();

var javascriptGatewayCFName = 'http://cache.hyves-static.net/statics/JavaScriptFlashGateway.a26e49afa654ad1169b3c4e66a887f59.swf';
var xmlGatewayCFName = 'http://cache.hyves-static.net/statics/XMLRetreiver.797cc47a82e432e5035d95a65f9f2d50.swf';

global_amazon_categories[3] = "Music";
global_amazon_categories[4] = "Movie";
global_amazon_categories[5] = "Books";
global_amazon_categories[6] = "Games";

global_search_categories[3] = "Music";
global_search_categories[4] = "Video";
global_search_categories[5] = "Books";
global_search_categories[6] = "VideoGames";

global_amazon_country_url = new Object();
global_amazon_country_url[213] = "http://webservices.amazon.com/onca/xml?Service=AWSECommerceService";
global_amazon_country_url[212] = "http://webservices.amazon.co.uk/onca/xml?Service=AWSECommerceService";
global_amazon_country_url[77] = "http://webservices.amazon.de/onca/xml?Service=AWSECommerceService";
global_amazon_country_url[101] = "http://webservices.amazon.co.jp/onca/xml?Service=AWSECommerceService";
global_amazon_country_url[71] = "http://webservices.amazon.fr/onca/xml?Service=AWSECommerceService";
global_amazon_country_url[37] = "http://webservices.amazon.ca/onca/xml?Service=AWSECommerceService";

AMAZON_DEFAULT_SEARCHCATEGORY = 5;


function RTE_AmazonPickerEventHandler(Controller) {
	this.Controller = Controller;
	this.init();
}

RTE_AmazonPickerEventHandler.prototype.init = function() {
	EventHandler.getInstance().addFakeEvent("amazonpicker_timeoutxml_" + this.Controller.name, this, "eventTimeoutXml");
}

RTE_AmazonPickerEventHandler.prototype.eventTimeoutXml = function(event, params) {
	this.Controller.timeoutXML();
}

/*	RTE_AmazonPickerController
*
*	The following properties need to be set:
*
*	RTE_AmazonPickerController.name					// name of the richtextedit
*	RTE_AmazonPickerController.text_AMAZONPICKER_PROCESSING					// "Processing request, one moment please."
*	RTE_AmazonPickerController.text_AMAZONPICKER_NORESULTS						// "No results.";
*	RTE_AmazonPickerController.text_AMAZONPICKER_TIMEOUT					// "Timeout";
*	[ ] = optional
*/
var AMAZONPICKER_STYLE_BLOCK = "block";
var AMAZONPICKER_STYLE_LAYOVER = "layover";
var AMAZONPICKER_TIMEOUT = 10000;


function RTE_AmazonPickerController() {
}

RTE_AmazonPickerController.prototype.log=logging.getLogger("Amazon");

RTE_AmazonPickerController.prototype.init = function() {
	this.firstLoad = true;
	var _self = this;

	//amazon picker
	this.text_AMAZONPICKER_PROCESSING = templates.translate('AMAZONPICKER_PROCESSING');	//"Processing request, one moment please."
	this.text_AMAZONPICKER_NORESULTS =  templates.translate('AMAZONPICKER_NORESULTS'); // "No results.";
	this.text_AMAZONPICKER_TIMEOUT = templates.translate('AMAZONPICKER_TIMEOUT'); // "Timeout.";
	
	this.amazonItems = new Object();
	
	this.domSearchBox = $("amazonpicker_" + this.name + "_searchbox");
	this.domSearchButton = $("amazonpicker_" + this.name + "_searchbutton");
	this.domResultPane = $("amazonpicker_" + this.name + "_resultpane");
	this.domCheckBox = $("amazonpicker_" + this.name + "_checkbox");
	this.domPagecontrolTop = $("amazonpicker_" + this.name + "_pagecontrol_top");
	this.domPagecontrolBottom = $("amazonpicker_" + this.name + "_pagecontrol_bottom");

	$H(global_amazon_categories).each(function(category) {
		var catid = category.key;
		_self["domCategory"+catid] = $("amazonpicker_"+_self.name+"_category_"+catid);
		
		Event.observe(_self["domCategory"+catid], "click", function(e) {
			_self.currentCategory = catid;
			_self.categoryChanged();
			_self.currentPage = 1;
			_self.doSearch();
		});
	});

	this.keywords = "";
	this.currentPage = 1;
	this.subscriptionId = "0GTB1SESH84HR12FT382";
	this.currentCategory = AMAZON_DEFAULT_SEARCHCATEGORY;
	this.currentType = AMAZONPICKER_STYLE_BLOCK;
	this.currentASIN = "";
	
	this.waitingXMLCall = false;
	
	this.domCheckBox.checked = true; // default currentType is block, so make checkbox checked.

	this.categoryChanged();
	
	// adds all events
	this.EventHandler = new RTE_AmazonPickerEventHandler(this);
	
	// Disable submitting of form on enter and executes search
	Event.observe(this.domSearchBox, "keypress", function(e) {
	    if (e.keyCode == Event.KEY_RETURN) {
            Event.stop(e);
	    	_self.doSearch();
	    }
	});
	
	Event.observe(this.domSearchButton, "click", function(e) {
		_self.doSearch();
	});
	
	var callbackName = 'amazonpicker_xmlgateway_loadXML_parseresult_' + this.name;
	
	var divName = "xmlretreiverdiv"

    var self = this;
	window[callbackName] = function (xml){
		self.XMLloaded(translateStringToDom(xml));
		// delete the item after a while, to avoid IE crashing on it on unload
		setTimeout(function() {
			$(divName).innerHTML="";
		} , 1000);
	}
	
	window['amazonpicker_xmlgateway_loadXML_' + this.name] = function (url) {
		xmlgateway_tag=new FlashTag(xmlGatewayCFName,1, 1, 8);
		xmlgateway_tag.addFlashVar('callbackname', callbackName);
		xmlgateway_tag.addFlashVar('url', url);
		xmlgateway_tag.setId('xmlretreiver');
		xmlgateway_tag.setAllowScriptAccess('always');
		
		if (!$(divName)) {
			var newTarget = document.createElement("div");
	        newTarget.id = divName;
	        document.body.appendChild(newTarget);
		}
		
		xmlgateway_tag.write($(divName));
	}

	
//end 
}

RTE_AmazonPickerController.prototype.load = function() {
	if (this.firstLoad == true) {
		//nothing to do.
	}
		
	this.firstLoad = false;
}

RTE_AmazonPickerController.prototype.doSearch = function() {
	this.amazonItems = [];
	RemoveChildsFromDom(this.domResultPane);
	if (this.domSearchBox.value.length == 0) {
		return;
	}
	this.domResultPane.innerHTML = this.text_AMAZONPICKER_PROCESSING;
	var searchIndex = global_search_categories[this.currentCategory];
	var country_url = global_amazon_country_url[this.currentCountry];
	var url = country_url + "&SubscriptionId="+this.subscriptionId+"&Operation=ItemSearch&SearchIndex="+searchIndex+"&Keywords="+this.domSearchBox.value+"&ResponseGroup=Small,Images&ItemPage="+this.currentPage;
	eval("amazonpicker_xmlgateway_loadXML_"+this.name+"(\""+url+"\");");
	this.waitingXMLCall = true;
	setTimeout("EventHandler.getInstance().triggerFakeEvent(\"amazonpicker_timeoutxml_" + this.name + "\")",AMAZONPICKER_TIMEOUT);	
}

RTE_AmazonPickerController.prototype.XMLloaded = function(xml_doc) {
	this.waitingXMLCall = false;
	RemoveChildsFromDom(this.domResultPane);
	oAmazonUtil = new AmazonSearchUtil(xml_doc);
	var numResults = oAmazonUtil.getTotalResultsFromXML();
	if (numResults < 1)
	{
		this.domResultPane.innerHTML = this.text_AMAZONPICKER_NORESULTS;
	}
	else
	{
		var numPages = oAmazonUtil.getTotalPagesFromXML();
		var aoItem = oAmazonUtil.getItemsFromXML();
		this.renderResults(aoItem);
		this.renderPageControl(numPages);
	}
}

RTE_AmazonPickerController.prototype.renderPageControl = function(numPages) {
	var _self = this;
	if (numPages > 1)
	{
		var nod = document.createElement("span");
	   	nod.style.display = "block";
	   	nod.textAlign = 'center';
	   	nod.width = "100%";
			var nod_back = document.createElement("span");
			nod_back.id = "amazonpicker_pagecontrol_back_"+this.name;
			nod_back.innerHTML = "<<";
			if (this.currentPage > 1)
			{
  				nod_back.className = "personal_fakelink fakelink";
  				Event.observe(nod_back, "click", function(e) {
  					_self.currentPage--;
					_self.doSearch();
  				});
			}

			var nod_pages = document.createElement("span");
			nod_pages.innerHTML = " ("+this.currentPage+" / "+numPages+") ";
	
			var nod_forward = document.createElement("span");
			nod_forward.id = "amazonpicker_pagecontrol_forward_"+this.name;
			nod_forward.innerHTML = ">>";
			if (this.currentPage < numPages)
			{
  				nod_forward.className = "personal_fakelink fakelink";
				Event.observe(nod_forward, "click", function(e) {
					_self.currentPage++;
					_self.doSearch();
  				});
			}
		nod.appendChild(nod_back);
		nod.appendChild(nod_pages);
		nod.appendChild(nod_forward);
		this.domResultPane.appendChild(nod);
	}
}


RTE_AmazonPickerController.prototype.renderResults = function(aoItem) {
	var _self = this;
	
	// Output results
	var htmlOut = templates.get('template.rte.amazon.results').process({
		'aoItem': aoItem, 
		'name':this.name
	});
	this.domResultPane.update(htmlOut);
	
	// Add actions
	$A(aoItem).each(function(oItem) {
		_self.amazonItems[oItem.asin] = oItem;
		
		var itemImage = $("amazonpicker_results_image_" + _self.name + "_" + oItem.asin);
		Event.observe(itemImage, "click", function(e) {
			_self.addAmazonItem(oItem.asin);
		});
		
		var itemTitle = $("amazonpicker_results_title_" + _self.name + "_" + oItem.asin);
		Event.observe(itemTitle, "click", function(e) {
			_self.addAmazonItem(oItem.asin);
		});
	});
}

RTE_AmazonPickerController.prototype.categoryChanged = function() {
	this.resetCategoryButtons();
	this["domCategory"+this.currentCategory].innerHTML = this["domCategory"+this.currentCategory].innerHTML;
	this["domCategory"+this.currentCategory].style.fontWeight = "bold";
	this["domCategory"+this.currentCategory].className = "";
}

RTE_AmazonPickerController.prototype.resetCategoryButtons = function() {
	for (catid in global_amazon_categories)
	{
		var catname = global_amazon_categories[catid];
		this["domCategory"+catid].innerHTML = catname;
		this["domCategory"+catid].style.fontWeight = "normal";
		this["domCategory"+catid].className = "personal_fakelink fakelink";
	}	
}

RTE_AmazonPickerController.prototype.addAmazonItem = function(asin) {
	this.currentASIN = asin;
	this.updateType();
		
	var oAmazonItem = this.amazonItems[this.currentASIN];
	
	var tag_code = oAmazonItem.asin+"_"+ this.currentCountry + "_" + this.currentCategory;
	
	if (this.currentType == AMAZONPICKER_STYLE_BLOCK)
	{
		fancylayout_inserttext(this.buildAmazonBlockTag(oAmazonItem, tag_code), "richtextcontrol_" + this.name);
	} else if (this.currentType == AMAZONPICKER_STYLE_LAYOVER) {
		if (fancylayout_getselection("richtextcontrol_" + this.name) == "")
			fancylayout_inserttext("[amznlo=" + tag_code + "]" + oAmazonItem.title + "[/amznlo]", "richtextcontrol_" + this.name);
		else
			fancylayout_blocktag("[amznlo=" + tag_code + "]", "[/amznlo]", "richtextcontrol_" + this.name);
	}
}

RTE_AmazonPickerController.prototype.buildAmazonBlockTag = function(oAmazonItem, tag_code){
	var tag_string = "[amznblk="+ tag_code + "]";
	tag_string += "[b]" + oAmazonItem.title + "[/b]\n";
	if (oAmazonItem.authors)
		tag_string += "Authors: " + oAmazonItem.authors + "\n";
	if (oAmazonItem.actors)
		tag_string += "Actors: " + oAmazonItem.actors + "\n";
	if (oAmazonItem.creators)
		tag_string += "Creators: " + oAmazonItem.creators + "\n";
	if (oAmazonItem.manufacturers)
		tag_string += "Manufacturers: " + oAmazonItem.manufacturers + "\n";
	if (oAmazonItem.directors)
		tag_string += "Directors: " + oAmazonItem.directors + "\n";
	tag_string += "Productgroup: " + oAmazonItem.productgroup + "\n";
	tag_string += "[/amznblk]";
	
	return tag_string;
}

RTE_AmazonPickerController.prototype.updateType = function() {
	if (this.domCheckBox.checked == true)
		this.currentType = AMAZONPICKER_STYLE_BLOCK;
	if (this.domCheckBox.checked == false)
		this.currentType = AMAZONPICKER_STYLE_LAYOVER;
}

RTE_AmazonPickerController.prototype.timeoutXML = function() {
	if (this.waitingXMLCall)
	{
		RemoveChildsFromDom(this.domResultPane);
		this.domResultPane.innerHTML = this.text_AMAZONPICKER_TIMEOUT;	
	}
}



function AmazonSearchUtil(xml_doc) {
	this.xml_doc = xml_doc;
}


AmazonSearchUtil.prototype.getItemsFromXML = function() {
	aItemsXml = this.xml_doc.getElementsByTagName("ItemSearchResponse")[0].getElementsByTagName("Items")[0].getElementsByTagName("Item");
	//make js-objects from the list
	var aoItem = [];
    for (var i=0;i< aItemsXml.length;i++)
    {
    	oItem = new Object();
    	
    	if (aItemsXml[i])
    	{
	    	if (aItemsXml[i].getElementsByTagName("ASIN")[0])
		     	oItem.asin = aItemsXml[i].getElementsByTagName("ASIN")[0].firstChild.nodeValue;
	    	if (aItemsXml[i].childNodes[2])
	    	{
		    	docSmallImage = aItemsXml[i].childNodes[2];
		    	if (docSmallImage.childNodes[0])
			    	oItem.imagesmallurl = docSmallImage.childNodes[0].firstChild.nodeValue;
		    	if (docSmallImage.childNodes[1])
			    	oItem.imagesmallheight = docSmallImage.childNodes[1].firstChild.nodeValue;
		    	if (docSmallImage.childNodes[2])
			    	oItem.imagesmallwidth = docSmallImage.childNodes[2].firstChild.nodeValue;
	    	}
	    	if (aItemsXml[i].getElementsByTagName("ItemAttributes")[0])
	    	{
		    	docItemAttributes = aItemsXml[i].getElementsByTagName("ItemAttributes")[0];
		    	if (docItemAttributes.getElementsByTagName("Title")[0])
			    	oItem.title = docItemAttributes.getElementsByTagName("Title")[0].firstChild.nodeValue;
		    	if (docItemAttributes.getElementsByTagName("ProductGroup")[0])
			    	oItem.productgroup = docItemAttributes.getElementsByTagName("ProductGroup")[0].firstChild.nodeValue;
		    	if (docItemAttributes.getElementsByTagName("Author").length > 0)
			    	oItem.authors = AmazonSearchUtil.getArrayFromXMLArray(docItemAttributes.getElementsByTagName("Author"));
		    	if (docItemAttributes.getElementsByTagName("Actor").length > 0)
			    	oItem.actors = AmazonSearchUtil.getArrayFromXMLArray(docItemAttributes.getElementsByTagName("Actor"));
		    	if (docItemAttributes.getElementsByTagName("Creator").length > 0)
			    	oItem.creators = AmazonSearchUtil.getArrayFromXMLArray(docItemAttributes.getElementsByTagName("Creator"));
		    	if (docItemAttributes.getElementsByTagName("Manufacturer").length > 0)
			    	oItem.manufacturers = AmazonSearchUtil.getArrayFromXMLArray(docItemAttributes.getElementsByTagName("Manufacturer"));
		    	if (docItemAttributes.getElementsByTagName("Director").length > 0)
			    	oItem.directors = AmazonSearchUtil.getArrayFromXMLArray(docItemAttributes.getElementsByTagName("Director"));
	    	}
			  	aoItem[aoItem.length] = oItem;
		}
    }
    return aoItem;
}

AmazonSearchUtil.prototype.getTotalResultsFromXML = function() {
	if (this.xml_doc.getElementsByTagName("ItemSearchResponse"))
		if (this.xml_doc.getElementsByTagName("ItemSearchResponse")[0].getElementsByTagName("Items")[0])
			if (this.xml_doc.getElementsByTagName("ItemSearchResponse")[0].getElementsByTagName("Items")[0].getElementsByTagName("TotalResults")[0])
				return this.xml_doc.getElementsByTagName("ItemSearchResponse")[0].getElementsByTagName("Items")[0].getElementsByTagName("TotalResults")[0].firstChild.nodeValue;
	return 0;
}

AmazonSearchUtil.prototype.getTotalPagesFromXML = function() {
	if (this.xml_doc.getElementsByTagName("ItemSearchResponse"))
		if (this.xml_doc.getElementsByTagName("ItemSearchResponse")[0].getElementsByTagName("Items")[0])
			if (this.xml_doc.getElementsByTagName("ItemSearchResponse")[0].getElementsByTagName("Items")[0].getElementsByTagName("TotalPages")[0])
				return this.xml_doc.getElementsByTagName("ItemSearchResponse")[0].getElementsByTagName("Items")[0].getElementsByTagName("TotalPages")[0].firstChild.nodeValue;
	return 0;
}

AmazonSearchUtil.getArrayFromXMLArray = function(xmlarray) {
	var temp_array = [];
	if (xmlarray.length && xmlarray.length > 0)
	{
	    for (var i=0;i< xmlarray.length;i++)
	    {
	    	temp_array[temp_array.length] = xmlarray[i].firstChild.nodeValue;
	    }
	}
	return temp_array;
}

if(window.jsloaded) {jsloaded('/precompiled/amazon.js')}


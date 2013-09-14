//constructor for the pager
var _pagers = {}; //dict (instanceName->instance) of all pagers in the page

function invalidatePagers()
{
	logging.debug("invalidating pagers");
	$H(_pagers).each(function(item) {
		item[1].invalidatePageCache();
	});
}

//name is jsname of the pager, equal to config name
//instanceName set for multiple pagers with same name on page
//nrPages total number of pages for this pager
//pageDiv element where new pages are placed in
//length number of items to show in page
//config name of the php config name for this set of pagers
function Pager(options) {

    //defaults
    this.extra = false;
    this.useNap = false;
    this.asynchronous = true;

    //extend ourself with options
    Object.extend(this, options);

    this.log.debug('initilizing pager');
    this.log.debug('name=' + this.name);
	this.log.debug('instancename=' + this.instanceName);
	
    this.pageDiv = $('pageDivId_' + this.instanceName); 

	//this.log.debug('instanceName=' + instanceName);
	//this.log.debug('nrPages=' + nrPages);
	//this.log.debug('pageDiv=' + pageDiv);
	//this.log.debug('length=' + length);
	
	this.previousPageNr = null;
	this.currentPageNr = 1;
	//initialize the page cache
	this.pageloaded = new Array();

	//the divs where the page numbers navigator must be shown
	this.pageNrDivs = new Array();
	this.deletedItems = new Array();
	this.changedItems = new Array();
	
    if(this.pageNrAlignTop) {
        this.registerPageNrDiv($('pageNrDivIdTop_' + this.instanceName));
    }
    
    if(this.pageNrAlignBottom) {
        this.registerPageNrDiv($('pageNrDivIdBottom_' + this.instanceName));
    }

    this.showPageNumbers();

    /*must be after setExtra and after addToCache otherwise we get a second ajax call*/
    if(this.useNap) { 
        this.initNap();   
    }
	
    // custom event
	this.pagePlacedEvent = new Hyves.CustomEvent('pager:pagePlaced', this);
	this.pagePlacedEvent.fire({id: this.instanceName});
	
	_pagers[this.instanceName] = this;
}

//log for this instance
Pager.prototype.log = logging.getLogger('Pager'); 

/**
 * Invalidates the cache, any new requested wil be
 * re-retreived from server side
 */
Pager.prototype.invalidatePageCache = function() {
	this.pageloaded = new Array();
}

Pager.prototype.navigate = function(state) {

	this.changeState(state);
}

//nap delegations its's magic
Pager.prototype.changeState = function(pageNr) {

	pageNr = parseInt("" + pageNr);
	if(!pageNr) {
		pageNr = 1;
	}
	if(this.currentPageNr != pageNr) {
		this.previousPageNr = this.currentPageNr;
		this.currentPageNr = pageNr;
		this.showPage();
	}
}

Pager.prototype.initNap = function() {
	this.useNap = true;
}

//all kind of extra info set to render the pager, cotennts depend on the type of pager
Pager.prototype.setExtra = function(extra) {
	this.extra = extra;	
}

//expreimental not used
Pager.prototype.setAsynchronous = function(value) {
	this.asynchronous = value;	
}

//register div, cotnainint the pagenrs, can be more than one(eg on top or below of the apger or both
Pager.prototype.registerPageNrDiv = function(element) {
	this.pageNrDivs[this.pageNrDivs.length] = element;	
}

/**
 * Show the pagenumbers in all the registered elements
 **/
Pager.prototype.showPageNumbers = function()
{
	//this.log.debug('iterating the pagenumberContainers for page ' + this.currentPageNr);
	//this.log.debug('page nr containers=' + this.pageNrDivs.inspect());
	
	if (this.nrPages==0) {
		return;
	}
	
	var self = this;
	this.pageNrDivs.each( function(pageNrElement) {
		
		self.putPageNumbersInElement(pageNrElement);
	});
	
} 

/**
  * Private util function to add a page number dom element
  */
Pager.prototype.addPageNumber = function(container, text, id, linkToPage,print) {
	var sPagenr = new Object();
	sPagenr.text=text;
	sPagenr.id=id;
	sPagenr.linktopage = linkToPage;
	sPagenr.print = print;
	container[container.length] = sPagenr;
}

/**
 * Show the correct pagenumbers in a dom element
 */
Pager.prototype.putPageNumbersInElement = function(element) {
	//console.debug(element);
	//cast to an int by subtracting, smetimtimes the currentpage got seen as a string 13 + 1 = 131??
	var currentPage = this.currentPageNr-0;
	//this.log.debug(element);
	if(!element) {
		return;
	}
	
	//this.log.debug('putting page number in ' + element.id + ' for page ' + currentPage + ' with total pages=' + this.nrPages);
	
	if (this.nrPages == 1) {
		this.log.debug('nr pages was 1 no nubmer drawing required');
		return;
	}
	
		
	if (element.firstChild)	{
		element.removeChild(element.firstChild);
	}
	
	var pagenumbers_do = document.createElement("span");
	pagenumbers_do.style.display = 'block';
	
	// first create page object		
	var asPagenr = new Array();
	
	
	
	this.addPageNumber(asPagenr, "&laquo;", "pb_prev_pager_" + element.id + "_" + this.instanceName , (currentPage == 1 ? false : currentPage-1), true);
	
	if (!this.spread) {
		this.spread = 3;
	}
	
	for (var i = 1; i<=this.nrPages;i++)
	{
		var printThisPageNumber = ( i == 1 || i == this.nrPages) ||	//always display first and last number
							(Math.abs(currentPage - i) == 4 && (i == 2 || i == this.nrPages -1)) ||	// second and penultimate may be 4 away  
							(Math.abs(currentPage - i) <= this.spread); 	// other pages maximum 'this.spread' away
		
		var elementName = i + "_pnr_" + element.id + "_" + this.instanceName + "_pagebutton";
		this.addPageNumber(asPagenr, i, elementName , (currentPage == i ? false : i), printThisPageNumber);
	}
	
	this.addPageNumber(asPagenr, "&raquo;", "pb_next_pager_" + element.id + "_" + this.instanceName , (currentPage == this.nrPages ? false : currentPage+1), true);
	
	//this.log.debug('drwaing numbers, nr = ' + asPagenr.length);
	var lastprinted = false;
	for(var i = 0; i< asPagenr.length;i++)
	{
		//decide whether to print this number
		var sPagenr = asPagenr[i];
		//console.debug(sPagenr);
		if (sPagenr.print)
		{
			var pagenr_do = document.createElement("span");
			pagenr_do.innerHTML = sPagenr.text;
				
			pagenr_do.id = sPagenr.id;
			
			
			if (sPagenr.linktopage !== false)
			{ 
				pagenr_do.className="personal_fakelink fakelink PageNr";
				var pageNr = sPagenr.linktopage;
				var oEventHandler = EventHandler.getInstance();
				oEventHandler.addOnClickEvent(pagenr_do, this, "pagenrClicked", [pageNr]);
			}
			else   
			{
				pagenr_do.className="personal_text PageNr";
				pagenr_do.style.fontWeight="bold";
			}
			//console.debug(pagenr_do);
			pagenumbers_do.appendChild(pagenr_do);
			lastprinted = true;
		}
		else
		{
			if (lastprinted)
			{
				var pagenr_do = document.createElement("span");
				pagenr_do.innerHTML = "...";
				pagenr_do.className="personal_text PageNr";
				pagenumbers_do.appendChild(pagenr_do);
				
				lastprinted = false;  
			}
		}  
	}
	
	//console.debug(element);
	//console.debug(pagenumbers_do);
	element.appendChild(pagenumbers_do);
}
  
//called on click of pagerNr
Pager.prototype.pagenrClicked = function(e, params)
{
	this.log.debug('showing page nr ' + params[0]);
	this.pageNrDivs.each( function(pageNrElement) {
		pageNrElement.innerHTML='<img src="http://' + hyves_cache_url + '/images/ajax_action.gif"/>';
	});
	
	
	
	if (this.useNap) {
		this.navigate(params[0]);
	}
	else {
		this.changeState(params[0]);
	}	
}

Pager.prototype.getCurrentPageDomObject = function()
{
	return document.getElementById('multipageloop_text_' + this.instanceName + '_' + this.currentPageNrnr);
}


/**
 * Show the page for pageNr
 **/
Pager.prototype.showPage = function()
{
	var _self = this;
		
	this.log.debug('showpage current page number ' + this.currentPageNr);
	
	/* We are going to overwrite the content div with a new currentpage (already set)
	 * if we had a previous page, we update our cache of it before overwriting. 
	 * The page could have been modifified by ondomready scripts etc. 
	 * If we later return to the prev page than we want to show the most recent
	 * version, and not the initial html.
	 * 
	 * This is NOT done on first load (for the first page), otherwise the cache of
	 * the first page would not include the ondomready content set by pagerAjax.tpl.
	 * Page 1 is retrieved an extra time when surfing back after clicking 
	 * on another page.
	 */
	if(this.previousPageNr && (this.pageloaded.length != 0)) {
		this.pageloaded[this.previousPageNr] = this.pageDiv.innerHTML;
	}
	
	//console.debug(this.pageDiv);
	if (this.pageloaded[this.currentPageNr]) {
		this.log.debug('page found in cache for pagenumber ' + this.currentPageNr);
		this.log.debug('updating page div=' + this.pageDiv.id);
		
		//also reevals javascript:
		//console.log(this.pageloaded[this.currentPageNr]);
		Element.update(this.pageDiv, this.pageloaded[this.currentPageNr]);
				
		//change the changed items
		this.log.debug('changes the changend items, length = ' + this.changedItems.length);
		this.changedItems.each(function(value, index) {	
					var divName = value['divName'];
					var changedItem = $(divName);
					if (changedItem) {
						_self.log.debug('changing ' + divName);		
						Element.update(changedItem, value['content']);	
					}
					else {
						_self.log.debug('value not found for changing ' + divName);
					}
				})
				this.asynchronous
		//unshow the deleted items, after change for items that have been changed and deleted
		this.log.debug('unshow the deleted items, length = ' + this.deletedItems.length);
		this.deletedItems.each(function(value, index) {	
					var deletedItem = $(value);
					if (deletedItem) {
						_self.log.debug('unshowing ' + value);		
						deletedItem.innerHTML = '';	
					}
					else {
						_self.log.debug('value not found for unshowing ' + value);
					}
				})
		
		this.showPageNumbers();
		this.pagePlacedEvent.fire({id: this.instanceName});
		return;			
	}
	
	
	if (this.currentPageNr>this.nrPages) {
		this.currentPageNr = this.nrPages;	
	}
	
	var url = '/index.php?xmlHttp=1&module=pager&action=showPage';
	var pars = 'name=' + this.name + '&pageNr=' + this.currentPageNr + '&config=' + this.config;
	if (this.extra) {
		pars = pars + '&extra=' + encodeURIComponent(this.extra);	
	}

	// This is not done using an Ajax.Updater, because then the javascript tags would be removed
	// and not be placed in the cache.
	var myAjax = new Ajax.Request( url, 
				{
					method: 'post' ,
					onFailure: function() {
						_self.showPageNumbers(); 
						_self.showFailureDialog();
					}, 
					onException: function (request, exception) {
						_self.log.error("" + exception, exception); _self.showPageNumbers();
					},
					onSuccess: function(xmlHttpRequest) {
						_self.afterPageUpdate(xmlHttpRequest, _self);
						_self.pageDiv.innerHTML = xmlHttpRequest.responseText;
						var html = _self.pageDiv.innerHTML.toString();
						setTimeout(function() {html.evalScripts()}, 10);
						_self.pagePlacedEvent.fire({id: _self.instanceName});
					},
					parameters: pars,
					asynchronous: this.asynchronous 
				});

	// in this case, we also want to do a google count
	if (window.GoogleAnalytics_Pageview)
		GoogleAnalytics_Pageview("/pager/"+this.name);
}

Pager.prototype.showFailureDialog = function(){
	var dialogId = Math.random();
	var elementId = '_popupDialog_'+dialogId+'_button_confirm';
	
	requires(['module:PopupDialogManager'], function() {
		var options = {
		y: 20,
		width: 500,
		alignTop: true,
		modal: true,
		draggable: true,
		dialogTemplate: 'default',
		dialogButtons: {
				'cancel': 'GENERAL_CANCEL',
				'confirm': 'GENERAL_OK'
			},
		dialogContent: function(){ 
			return templates.translate('GENERAL_SOMETHING_WENT_WRONG_RELOAD',{});
			}	
		};

		popupDialogManager.createDialog(dialogId, options);
		Event.observe(elementId, 'click', function(){document.location.reload(true)}); 		
	});
}
//after page result is recieved, page is added to cache and pagenumbers are updated
Pager.prototype.afterPageUpdate = function(xmlHttpRequest, self) {
	this.log.debug('afterPageUpdate ' + self.currentPageNr);
	this.addToCache(self.currentPageNr, xmlHttpRequest.responseText);
	this.showPageNumbers();
}

//add page to cache
Pager.prototype.addToCache = function(pageNr, content) {
	this.pageloaded[pageNr] = content;
}

//get the current page number
Pager.prototype.getPagenr = function()
{
	return(this.pagenrField.value);	
}

//show first page and add an item as the first item on the first page
Pager.prototype.showFirstPageAndAdd = function(itemToAdd, slideName) {
	
	this.log.debug('adding to first page, currentPage = ' + this.currentPageNr);
	if(this.pageloaded[1]) {
		var content = this.pageloaded[1];
		content = itemToAdd + content;
		this.pageloaded[1] = content;
	}
	
	if(this.currentPageNr==1) {
		var topDivName = 'pagerTop_' + this.name; 
		if(!$(topDivName)) {
			this.log.debug('top div ' + topDivName + ' not found');
		}
		else {
			new Insertion.After(topDivName, itemToAdd);
			this.log.debug("sliding pagerTop");
			Effect.SlideDown(slideName);
		}
	}
	else {
		this.navigate(1);
	}
	
}

//show the last page and add an item as the last item on the last page
Pager.prototype.showLastPageAndAdd = function(itemToAdd, slideName) {
	
	this.log.debug('adding to last page, currentPage = ' + this.currentPageNr);
	if(this.pageloaded[this.nrPages]) {
		this.log.debug('last page found in cache now adding');
		var content = this.pageloaded[this.nrPages];
		content = content + itemToAdd;
		this.pageloaded[this.nrPages] = content;
	}
	
	
	if(this.currentPageNr==this.nrPages) {
		var bottomDivName = 'pagerBottom_' + this.name; 
		if(!$(bottomDivName)) {
			this.log.debug('top div ' + bottomDivName + ' not found');
		}
		else {
			new Insertion.Before(bottomDivName, itemToAdd);
			this.log.debug("sliding pagerBottom");
			Effect.SlideDown(slideName);
		}
	}
	else {
		this.navigate(this.nrPages);	
	}
}

/**
 * registers deleted items, if page comes from cache, these are unshown
**/
Pager.prototype.addDeletedItem = function(divName) {
	
	this.log.debug('adding ' + divName + ' to deleted items');
	this.deletedItems.push(divName)	
}


/**
 * registers changed items, if page comes from cache, these are chagned
**/
Pager.prototype.addChangedItem = function(divName) {
	
	this.log.debug('adding ' + divName + ' to changed items');
	newContent = $(divName).innerHTML;
	
	$changed = new Object();
	$changed['divName'] = divName;
	$changed['content'] = newContent;
	
	this.changedItems.push($changed)	
}

 if(window.jsloaded) {jsloaded('/statics/pager.js')}

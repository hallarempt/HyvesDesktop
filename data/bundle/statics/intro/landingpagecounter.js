// defines how often the introduction page is shown to the user
// note: this needs to be changed in plugins/kwekker/src/MainWindow.cpp accordingly
var landingPageCounterMax = 5;

function initShowAgainState() {
	var landingPageCounter = this.settingsManager.intValue('landingPageCounter', landingPageCounterMax);
	document.getElementById('showAgainCheckbox').checked = (landingPageCounter > 0);
	document.getElementById('showAgainLabel').innerHTML = tr("Show this screen the next time Hyves Desktop starts");
}

function saveShowAgainState() {
	this.settingsManager.setValue('landingPageCounter', document.getElementById('showAgainCheckbox').checked ? landingPageCounterMax : 0);
}

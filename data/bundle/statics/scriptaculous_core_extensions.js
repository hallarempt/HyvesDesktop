/**
 * Adds a transitionless toggle to Script.aculo.us' Effect class.
 * Use: Effect.toggle(element, 'show');
 */
 
Effect.PAIRS = Object.extend(Effect.PAIRS, { 'show':  ['show','hide']});

Effect.toggle = function(element, effect) {
    element = $(element);
    effect = (effect || 'appear').toLowerCase();
    if(effect == 'show'){
    	if(element){
    		element.visible() ? element.hide() : element.show();
    	}
    	return;
    }
    
    var options = Object.extend({
      queue: { position:'end', scope:(element.id || 'global'), limit: 1 }
    }, arguments[2] || {});
    Effect[element.visible() ? 
      Effect.PAIRS[effect][1] : Effect.PAIRS[effect][0]](element, options);
  }
  

/**
 * We define the scriptaculous version number here so we don't have 
 * to include scriptaculous.js which cannot be included since it messes
 * up our js versioning and 'require' system.
 * Prototip won't work without the scriptaculous version number.
 */
 var Scriptaculous = {
  Version: '1.8.1'};

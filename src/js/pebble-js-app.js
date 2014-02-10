
Pebble.addEventListener("ready", function(e) {
  console.log("Starting ...");
  sendContents();
});

Pebble.addEventListener("appmessage", function(e) {
  sendContents();
});

function sendContents() {
  var label = localStorage.getItem('label') || 'Set this in Settings...';
  var target = localStorage.getItem('target');
  Pebble.sendAppMessage({ "label": label, "target": target });
}

Pebble.addEventListener('showConfiguration', function(e) {
  var label = localStorage.getItem('label') || '';
  var target = localStorage.getItem('target') || '';
	//var uri = 'https://rawgithub.com/VGraupera/note-to-self-watchface/master/html/configuration.html?' +
	var uri = 'https://vgraupera.s3.amazonaws.com/pebble/configuration.html?' +
				'label=' + encodeURIComponent(label) + "&target=" + encodeURIComponent(label);
	console.log('showing configuration at uri: ' + uri);
	Pebble.openURL(uri);
});

Pebble.addEventListener('webviewclosed', function(e) {
	console.log('configuration closed');
	if (e.response) {
		var options = JSON.parse(decodeURIComponent(e.response));
		console.log('options received from configuration: ' + JSON.stringify(options));
		var target = options['target'];
		localStorage.setItem('target', target);
		var label = options['label'];
		localStorage.setItem('label', label);
    sendContents();
	} else {
		console.log('no options received');
	}
});

// Generated by CoffeeScript 1.7.1
(function() {
  var appMessageQueue, maxTriesForSendingAppMessage, readEvents, sendAppMessageQueue, sendConfiguration, timeoutForAppMessage, timeoutForAppMessageRetry;

  maxTriesForSendingAppMessage = 3;

  timeoutForAppMessageRetry = 3000;

  timeoutForAppMessage = 100;

  appMessageQueue = [];

  sendAppMessageQueue = function() {
    var currentAppMessage;
    if (appMessageQueue.length > 0) {
      currentAppMessage = appMessageQueue[0];
      currentAppMessage.numTries = currentAppMessage.numTries || 0;
      if (currentAppMessage.numTries < maxTriesForSendingAppMessage) {
        console.log("Sending currentAppMessage to Pebble: " + JSON.stringify(currentAppMessage));
        return Pebble.sendAppMessage(currentAppMessage.message, function(e) {
          appMessageQueue.shift();
          return setTimeout((function() {
            return sendAppMessageQueue();
          }), timeoutForAppMessage);
        }, function(e) {
          console.log("Failed sending currentAppMessage for " + (JSON.stringify(currentAppMessage)) + "\n Error: " + e.data.error.message);
          currentAppMessage.numTries++;
          return setTimeout((function() {
            return sendAppMessageQueue();
          }), timeoutForAppMessageRetry);
        });
      } else {
        appMessageQueue.shift();
        return console.log("Failed sending AppMessage bailing. " + JSON.stringify(currentAppMessage));
      }
    } else {
      return console.log("AppMessage queue is empty.");
    }
  };

  sendConfiguration = function() {
    var date, event, events, vibrate, _i, _len;
    console.log("Sending config ...");
    events = readEvents();
    appMessageQueue.push({
      message: {
        reset: 1,
        length: events.length
      }
    });
    for (_i = 0, _len = events.length; _i < _len; _i++) {
      event = events[_i];
      date = new Date(event.date);
      appMessageQueue.push({
        message: {
          name: event.name,
          target: "" + date.getTime() / 1000
        }
      });
    }
    vibrate = false;
    if (localStorage.getItem("vibrate") !== null) {
      vibrate = JSON.parse(localStorage.getItem("vibrate"));
    }
    appMessageQueue.push({
      message: {
        vibrate: vibrate && 1 || 0
      }
    });
    return sendAppMessageQueue();
  };

  readEvents = function() {
    var e, events;
    events = [];
    if (localStorage.getItem("eventList") !== null) {
      try {
        events = JSON.parse(localStorage.getItem("eventList"));
      } catch (_error) {
        e = _error;
        console.log("exception reading " + (localStorage.getItem("eventList")));
      }
    } else {
      events = [
        {
          name: "Christmas",
          date: "2014-12-25"
        }
      ];
    }
    return events;
  };

  Pebble.addEventListener("ready", function(e) {
    console.log("On ready event ...");
    return sendConfiguration();
  });

  Pebble.addEventListener("appmessage", function(e) {
    console.log("Received from Pebble: " + JSON.stringify(e.payload));
    if (e.payload.update) {
      return sendConfiguration();
    }
  });

  Pebble.addEventListener("showConfiguration", function(e) {
    var events, url, vibrate;
    vibrate = localStorage.getItem("vibrate") || false;
    events = readEvents();
    url = "http://countdown-watchface-v3.s3-website-us-west-1.amazonaws.com?events=" + encodeURIComponent(JSON.stringify(events)) + ("&vibrate=" + vibrate);
    console.log("open settings: " + url);
    return Pebble.openURL(url);
  });

  Pebble.addEventListener("webviewclosed", function(e) {
    var params;
    if (e.response) {
      params = JSON.parse(decodeURIComponent(e.response));
      console.log("Params received from settings page: " + JSON.stringify(params));
      localStorage.setItem("eventList", JSON.stringify(params.events));
      localStorage.setItem("vibrate", params.vibrate);
      return sendConfiguration();
    }
  });

}).call(this);

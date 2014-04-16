maxTriesForSendingAppMessage = 3
timeoutForAppMessageRetry = 3000
timeoutForAppMessage = 100

appMessageQueue = []

sendAppMessageQueue = ->
  if appMessageQueue.length > 0
    currentAppMessage = appMessageQueue[0]
    currentAppMessage.numTries = currentAppMessage.numTries or 0
    if currentAppMessage.numTries < maxTriesForSendingAppMessage
      console.log "Sending currentAppMessage to Pebble: " + JSON.stringify(currentAppMessage)
      Pebble.sendAppMessage currentAppMessage.message, (e) ->
        appMessageQueue.shift()
        setTimeout (->
          sendAppMessageQueue()
        ), timeoutForAppMessage
      , (e) ->
        console.log "Failed sending currentAppMessage for #{JSON.stringify(currentAppMessage)}\n
          Error: #{e.data.error.message}"
        currentAppMessage.numTries++
        setTimeout (->
          sendAppMessageQueue()
        ), timeoutForAppMessageRetry
    else
      appMessageQueue.shift()
      console.log "Failed sending AppMessage bailing. " + JSON.stringify(currentAppMessage)
  else
    console.log "AppMessage queue is empty."

sendConfiguration = ->
  console.log "Sending config ..."
  events = readEvents()

  appMessageQueue.push
    message:
      reset: 1
      length: events.length

  for event in events
    date = new Date(event.date)
    appMessageQueue.push
      message:
        name: event.name
        target: "" + date.getTime() / 1000

  vibrate = false
  if localStorage.getItem("vibrate") isnt null
    vibrate = JSON.parse(localStorage.getItem("vibrate"))

  appMessageQueue.push
    message:
      vibrate: vibrate && 1 || 0

  sendAppMessageQueue()

readEvents = () ->
  events = []
  if localStorage.getItem("eventList") isnt null
    try
      events = JSON.parse(localStorage.getItem("eventList"))
    catch e
      console.log "exception reading #{localStorage.getItem("eventList")}"
  else
    events = [
      name: "Christmas"
      date: "2014-12-25"
    ]
  events

Pebble.addEventListener "ready", (e) ->
  console.log "On ready event ..."
  sendConfiguration()

Pebble.addEventListener "appmessage", (e) ->
  console.log "Received from Pebble: " + JSON.stringify(e.payload)
  if e.payload.update
    sendConfiguration()

Pebble.addEventListener "showConfiguration", (e) ->
  vibrate = localStorage.getItem("vibrate") or false
  events = readEvents()
  url = "http://countdown-watchface-v3.s3-website-us-west-1.amazonaws.com?events=" +
    encodeURIComponent(JSON.stringify(events)) + "&vibrate=#{vibrate}"
  console.log "open settings: #{url}"
  Pebble.openURL url

Pebble.addEventListener "webviewclosed", (e) ->
  if e.response
    params = JSON.parse(decodeURIComponent(e.response))
    console.log "Params received from settings page: " + JSON.stringify(params)
    # cache to local storage;
    localStorage.setItem "eventList", JSON.stringify(params.events)
    localStorage.setItem "vibrate", params.vibrate
    sendConfiguration()

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

sendEvents = ->
  console.log "Sending event list ..."
  events = []
  if localStorage.getItem("eventList") isnt null
    events = JSON.parse(localStorage.getItem("eventList")) # get value from localstorage

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

  sendAppMessageQueue()

Pebble.addEventListener "ready", (e) ->
  console.log "On ready event ..."
  sendEvents()

Pebble.addEventListener "appmessage", (e) ->
  console.log "Received from Pebble: " + JSON.stringify(e.payload)
  if e.payload.update
    sendEvents()

Pebble.addEventListener "showConfiguration", (e) ->
  Pebble.openURL "http://countdown-watchface.s3-website-us-west-1.amazonaws.com"

Pebble.addEventListener "webviewclosed", (e) ->
  if e.response
    events = JSON.parse(decodeURIComponent(e.response))
    console.log "Events received from settings page: " + JSON.stringify(events)
    localStorage.setItem "eventList", JSON.stringify(events) # cache to local storage
    sendEvents()

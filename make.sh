coffee --join src/js/pebble-js-app.js --compile src/coffee/*.coffee || { exit 1; }
jshint appinfo.json || { exit 1; }
#jshint src/js/pebble-js-app.js || { exit 1; }
pebble clean
pebble build || { exit 1; }
#rm src/js/pebble-js-app.js || { exit 1; }
if [ "$1" = "install" ]; then
    pebble install --logs --debug
fi

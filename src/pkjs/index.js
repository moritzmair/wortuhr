var Clay = require('./clay');
var buildConfig = require('./config');
var clayCustom = require('./config-custom');
var clay = new Clay(buildConfig('de'), clayCustom, { autoHandleEvents: false });

// Sprache der Einstellungsseite: die fürs Watchface gewählte, sonst die der Uhr.
function configLanguage() {
  try {
    var saved = JSON.parse(localStorage.getItem('clay-settings') || '{}').Language;
    if (saved !== undefined) {
      return String(saved) === '0' ? 'de' : 'en';
    }
  } catch (e) {}
  var info = Pebble.getActiveWatchInfo && Pebble.getActiveWatchInfo();
  var locale = (info && info.language) || navigator.language || '';
  return locale.indexOf('de') === 0 ? 'de' : 'en';
}

Pebble.addEventListener('showConfiguration', function() {
  clay.config = buildConfig(configLanguage());
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) {
    return;
  }
  Pebble.sendAppMessage(clay.getSettings(e.response), null, function(err) {
    console.log('Einstellungen nicht gesendet: ' + JSON.stringify(err));
  });
});

function fetchWeather(fahrenheit) {
  navigator.geolocation.getCurrentPosition(function(pos) {
    var url = 'https://api.open-meteo.com/v1/forecast' +
      '?latitude=' + pos.coords.latitude +
      '&longitude=' + pos.coords.longitude +
      '&current=temperature_2m' +
      (fahrenheit ? '&temperature_unit=fahrenheit' : '');
    var req = new XMLHttpRequest();
    req.onload = function() {
      try {
        var temp = JSON.parse(req.responseText).current.temperature_2m;
        Pebble.sendAppMessage({ 'Temperature': Math.round(temp) });
      } catch (e) {
        console.log('Wetter: unerwartete Antwort ' + req.responseText);
      }
    };
    req.open('GET', url);
    req.send();
  }, function(err) {
    console.log('Wetter: kein Standort (' + err.message + ')');
  }, { timeout: 15000, maximumAge: 60 * 60 * 1000 });
}

// Die Uhr fragt selbst nach, wenn sie die Temperatur braucht (Start, alle 30 Minuten,
// nach dem Speichern der Einstellungen) und schickt die gewünschte Einheit mit.
Pebble.addEventListener('appmessage', function(e) {
  if (e.payload.RequestWeather !== undefined) {
    fetchWeather(!!e.payload.Fahrenheit);
  }
});

// Signalisiert der Uhr, dass die JS-Seite bereit ist; vorher kann sie nichts anfragen.
Pebble.addEventListener('ready', function() {
  Pebble.sendAppMessage({ 'RequestWeather': 1 });
});

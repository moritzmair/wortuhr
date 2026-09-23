var Clay = require('./clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

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

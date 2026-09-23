module.exports = [
  {
    "type": "heading",
    "defaultValue": "Wortuhr"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Farben"
      },
      {
        "type": "color",
        "messageKey": "BackgroundColor",
        "defaultValue": "FFFFFF",
        "label": "Hintergrund"
      },
      {
        "type": "color",
        "messageKey": "TextColor",
        "defaultValue": "AAAAAA",
        "label": "Text"
      },
      {
        "type": "color",
        "messageKey": "HighlightColor",
        "defaultValue": "000000",
        "label": "Hervorgehobener Text"
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Anzeige"
      },
      {
        "type": "toggle",
        "messageKey": "ShowEsIst",
        "label": "\"ES IST\" anzeigen",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "messageKey": "ShowDate",
        "label": "Datum anzeigen",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "messageKey": "ShowTemp",
        "label": "Außentemperatur anzeigen",
        "description": "Wird alle 30 Minuten über den Standort des Handys von open-meteo.com geholt.",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "messageKey": "Fahrenheit",
        "label": "Temperatur in °F",
        "defaultValue": false
      },
      {
        "type": "toggle",
        "messageKey": "InfoGridStyle",
        "label": "Datum/Temperatur im Buchstabenraster",
        "description": "Ein Zeichen pro Feld mit Füllbuchstaben dazwischen, wie der Rest der Uhr. Aus: normale Textzeile.",
        "defaultValue": true
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "id": "InfoColorsHeading",
        "defaultValue": "Farben der Textzeile"
      },
      {
        "type": "toggle",
        "messageKey": "InfoCustomColors",
        "label": "Eigene Farben",
        "description": "Aus: Die Zeile ist automatisch invertiert zum Raster.",
        "defaultValue": false
      },
      {
        "type": "color",
        "messageKey": "InfoBackgroundColor",
        "defaultValue": "000000",
        "label": "Hintergrund"
      },
      {
        "type": "color",
        "messageKey": "InfoTextColor",
        "defaultValue": "FFFFFF",
        "label": "Text"
      },
      {
        "type": "color",
        "messageKey": "InfoLineColor",
        "defaultValue": "AAAAAA",
        "label": "Trennlinie"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Speichern"
  }
];

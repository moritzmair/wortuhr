var TEXT = {
  de: {
    colors: 'Farben',
    background: 'Hintergrund',
    text: 'Text',
    highlight: 'Hervorgehobener Text',
    display: 'Anzeige',
    language: 'Sprache',
    showHeader: '"ES IST" anzeigen',
    showDate: 'Datum anzeigen',
    showTemp: 'Außentemperatur anzeigen',
    showTempDescription: 'Wird alle 30 Minuten über den Standort des Handys von open-meteo.com geholt.',
    fahrenheit: 'Temperatur in °F',
    gridStyle: 'Datum/Temperatur im Buchstabenraster',
    gridStyleDescription: 'Ein Zeichen pro Feld mit Füllbuchstaben dazwischen, wie der Rest der Uhr. Aus: normale Textzeile.',
    infoColors: 'Farben der Textzeile',
    customColors: 'Eigene Farben',
    customColorsDescription: 'Aus: Die Zeile ist automatisch invertiert zum Raster.',
    line: 'Trennlinie',
    save: 'Speichern'
  },
  en: {
    colors: 'Colors',
    background: 'Background',
    text: 'Text',
    highlight: 'Highlighted text',
    display: 'Display',
    language: 'Language',
    showHeader: 'Show "IT IS"',
    showDate: 'Show date',
    showTemp: 'Show outside temperature',
    showTempDescription: 'Fetched every 30 minutes from open-meteo.com using your phone\'s location.',
    fahrenheit: 'Temperature in °F',
    gridStyle: 'Date/temperature in the letter grid',
    gridStyleDescription: 'One character per cell with filler letters in between, like the rest of the clock. Off: plain text bar.',
    infoColors: 'Text bar colors',
    customColors: 'Custom colors',
    customColorsDescription: 'Off: the bar is automatically inverted relative to the grid.',
    line: 'Separator line',
    save: 'Save'
  }
};

// Baut die Einstellungsseite in der Sprache `lang` ('de' oder 'en').
module.exports = function(lang) {
  var t = TEXT[lang] || TEXT.en;
  return [
    {
      "type": "heading",
      "defaultValue": lang === 'de' ? "Wortuhr" : "Word Clock"
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": t.display
        },
        {
          "type": "select",
          "messageKey": "Language",
          "label": t.language,
          "defaultValue": lang === 'de' ? "0" : "1",
          "options": [
            { "label": "Deutsch", "value": "0" },
            { "label": "English", "value": "1" }
          ]
        },
        {
          "type": "toggle",
          "messageKey": "ShowEsIst",
          "label": t.showHeader,
          "defaultValue": false
        },
        {
          "type": "toggle",
          "messageKey": "ShowDate",
          "label": t.showDate,
          "defaultValue": false
        },
        {
          "type": "toggle",
          "messageKey": "ShowTemp",
          "label": t.showTemp,
          "description": t.showTempDescription,
          "defaultValue": false
        },
        {
          "type": "toggle",
          "messageKey": "Fahrenheit",
          "label": t.fahrenheit,
          "defaultValue": false
        },
        {
          "type": "toggle",
          "messageKey": "InfoGridStyle",
          "label": t.gridStyle,
          "description": t.gridStyleDescription,
          "defaultValue": true
        }
      ]
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": t.colors
        },
        {
          "type": "color",
          "messageKey": "BackgroundColor",
          "defaultValue": "FFFFFF",
          "label": t.background
        },
        {
          "type": "color",
          "messageKey": "TextColor",
          "defaultValue": "AAAAAA",
          "label": t.text
        },
        {
          "type": "color",
          "messageKey": "HighlightColor",
          "defaultValue": "000000",
          "label": t.highlight
        }
      ]
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "id": "InfoColorsHeading",
          "defaultValue": t.infoColors
        },
        {
          "type": "toggle",
          "messageKey": "InfoCustomColors",
          "label": t.customColors,
          "description": t.customColorsDescription,
          "defaultValue": false
        },
        {
          "type": "color",
          "messageKey": "InfoBackgroundColor",
          "defaultValue": "000000",
          "label": t.background
        },
        {
          "type": "color",
          "messageKey": "InfoTextColor",
          "defaultValue": "FFFFFF",
          "label": t.text
        },
        {
          "type": "color",
          "messageKey": "InfoLineColor",
          "defaultValue": "AAAAAA",
          "label": t.line
        }
      ]
    },
    {
      "type": "submit",
      "defaultValue": t.save
    }
  ];
};

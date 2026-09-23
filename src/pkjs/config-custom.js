// Läuft auf der Einstellungsseite (nicht im Handy-JS) und blendet die Farben der
// Textzeile nur ein, wenn sie auch benutzt werden.
module.exports = function(minified) {
  var clayConfig = this;

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    var showDate = clayConfig.getItemByMessageKey('ShowDate');
    var showTemp = clayConfig.getItemByMessageKey('ShowTemp');
    var gridStyle = clayConfig.getItemByMessageKey('InfoGridStyle');
    var custom = clayConfig.getItemByMessageKey('InfoCustomColors');
    var colors = ['InfoBackgroundColor', 'InfoTextColor', 'InfoLineColor'].map(function(key) {
      return clayConfig.getItemByMessageKey(key);
    });
    var heading = clayConfig.getItemById('InfoColorsHeading');

    function update() {
      var textRow = (showDate.get() || showTemp.get()) && !gridStyle.get();
      [heading, custom].forEach(function(item) {
        if (textRow) { item.show(); } else { item.hide(); }
      });
      colors.forEach(function(item) {
        if (textRow && custom.get()) { item.show(); } else { item.hide(); }
      });
    }

    [showDate, showTemp, gridStyle, custom].forEach(function(item) {
      item.on('change', update);
    });
    update();
  });
};

jQuery(function($, undefined) {
  $('#term').terminal(function(command, term) {
    term.echo(command);
  }, {
    greetings: 'Javascript Interpreter',
    name: 'js_demo',
    height: 500,
    prompt: 'js> '
  });
});

function WiFiTelnetToSerialViewModel()
{
    var self = this;

    // Data
    self.tab = ko.observable(null);

    // Derived data
    self.isTerminal = ko.pureComputed(function () {
        return this.tab() == 'terminal';
    }, this);
    self.isSettings = ko.pureComputed(function () {
        return this.tab() == 'settings';
    }, this);
    self.isAbout = ko.pureComputed(function () {
        return this.tab() == 'about';
    }, this);

    // Behaviours
    self.goToTab = function (tab) { location.hash = tab; };

    // Client-side routes
    var sammy = Sammy(function ()
    {
        this.get('#:tab', function () {
            self.tab(this.params.tab);
        });

        this.get('', function () {
            this.redirect('#terminal');
        });
    });

    sammy.run();
}

// Activates knockout.js
var htmlTestTool = new WiFiTelnetToSerialViewModel();
ko.applyBindings(htmlTestTool);

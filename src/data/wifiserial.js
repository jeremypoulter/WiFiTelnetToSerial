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

    var socket = new WebSocket("ws://"+window.location.hostname+"/ws");
    socket.onopen = function (ev) {
        console.log(ev);
    };
    socket.onclose = function (ev) {
        console.log(ev);
    };
    socket.onmessage = function (msg) {
        console.log(msg);
        terminal.echo(msg.data.replace(/[\r\n]/g, ''));
    };

    var terminal = $('#term').terminal(function(command, term) {
        socket.send(command);
    }, {
        greetings: 'ESP Serial',
        name: 'espserial',
        height: 500,
        prompt: ''
    });

    sammy.run();
}

// Activates knockout.js
var htmlTestTool = new WiFiTelnetToSerialViewModel();
ko.applyBindings(htmlTestTool);

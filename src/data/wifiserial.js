//var baseHost = window.location.hostname;
//var baseHost = 'espserial.local';
var baseHost = 'test.com';
var baseEndpoint = 'http://'+baseHost;

function WiFiViewModel()
{
    var self = this;

    ko.mapping.fromJS({
        'mac': '00:00:00:00:00:00',
        'localIP': '0.0.0.0',
        'subnetMask': '255.255.255.0',
        'gatewayIP': '0.0.0.0',
        'dnsIP': '0.0.0.0',
        'status': 0,
        'hostname': 'espserial',
        'SSID': 'ssid',
        'BSSID': '00:00:00:00:00:00',
        'RSSI': 0
    }, {}, self);

    self.fetching = ko.observable(false);
    self.scan = ko.observable(new WiFiScanViewModel());

    self.update = function () {
        self.scan().update();
        self.fetching(true);
        $.get(baseEndpoint+'/wifi', function (data) {
            ko.mapping.fromJS(data, self);
        }, 'json').always(function () {
            self.fetching(false);
        });
    };
}

function WiFiScanResultViewModel(data)
{
    var self = this;
    ko.mapping.fromJS(data, {}, self);
}

function WiFiScanViewModel()
{
    var self = this;

    self.results = ko.mapping.fromJS([], {
        key: function(data) {
            return ko.utils.unwrapObservable(data.ssid);
        },
        create: function (options) {
            return new WiFiScanResultViewModel(options.data);
        }
    });

    self.fetching = ko.observable(false);

    self.update = function () {
        self.fetching(true);
        $.get(baseEndpoint+'/wifi/scan', function (data) {
            ko.mapping.fromJS(data, self.results);
        }, 'json').always(function () {
            self.fetching(false);
        });
    };
}

function SerialViewModel()
{
    var self = this;

    ko.mapping.fromJS({
        'baud': 115200,
        'dataBits': 8,
        'parity': 'N',
        'stopBits': 1
    }, {}, self);

    self.fetching = ko.observable(false);

    self.update = function () {
        self.fetching(true);
        $.get(baseEndpoint+'/serial', function (data) {
            ko.mapping.fromJS(data, self);
        }, 'json').always(function () {
            self.fetching(false);
        });
    };
}

function InfoViewModel()
{
    var self = this;

    ko.mapping.fromJS({
        'id': 0,
        'heap': 0,
        'version': '0.0.0'
    }, {}, self);

    self.fetching = ko.observable(false);

    self.update = function () {
        self.fetching(true);
        $.get(baseEndpoint+'/info', function (data) {
            ko.mapping.fromJS(data, self);
        }, 'json').always(function () {
            self.fetching(false);
        });
    };
}

function SettingsViewModel(app)
{
  var self = this;

  self.wifi = ko.observable(new WiFiViewModel());
  self.serial = ko.observable(new SerialViewModel());

  app.isSettings.subscribe(function (selected) {
      if (selected) {
          self.wifi().update();
          self.serial().update();
      }
  });

}

function AboutViewModel(app)
{
  var self = this;

  self.info = ko.observable(new InfoViewModel());

  self.reboot = function () {
    $.post(baseEndpoint+'/reboot', function (data) {
      alert('ESPSerial rebooting');
    }, 'json');
  };

  self.factoryReset = function () {
    $.ajax({
        url: baseEndpoint+'/settings',
        type: 'DELETE'
    });
  };

  app.isAbout.subscribe(function (selected) {
      if (selected) {
          self.info().update();
      }
  });
}

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

    // Settings
    self.settings = ko.observable(new SettingsViewModel(self));

    // Aboud
    self.about = ko.observable(new AboutViewModel(self));

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

    var socket = new WebSocket('ws://'+baseHost+'/ws');
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

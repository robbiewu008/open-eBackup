/*!
 * tabs
 * Tabs switch
 * Depends on jquery-3.5.1
 */
/*!
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */
(function() {
  /*!
Dependent style file
css/jquery-ui.css
*/

  // Label for instantiation plug-in
  var widgetUuid = 1;
  $.fn['uiTabs'] = function(options) {
    // this div#tabs
    this.each(function() {
      // fullName:"ui-tabs"
      var instance = $.data(this, 'ui-tabs');
      if (instance) {
        // already instantiated
        return;
      } else {
        // initialization
        $.data(this, 'ui-tabs', new uiTabs(options, this));
      }
    });
  };

  var uiTabs = function(options, element) {
    this.uuid = widgetUuid++;
    this.eventNamespace = '.tabs' + this.uuid;
    this.element = $(element);
    this.classesElementLookup = {};
    this.bindings = $();
    this.hoverable = $();
    this.focusable = $();
    if (options) {
      this.options = options;
    }
    this._create();
  };
  uiTabs.prototype = {
    options: {
      active: 0,
      // Event Triggering Mode
      event: 'click',
      collapsible: false,
      hide: null,
      show: null,

      // functions
      activate: null,
      // Check Before Tab Switch
      beforeActivate: null,
      beforeLoad: null,
      load: null
    },
    // reload anchor
    _isLocal: (function() {
      var rhash = /#.*$/;

      return function(anchor) {
        var anchorUrl, locationUrl, isLocal;

        anchorUrl = anchor.href.replace(rhash, '');
        locationUrl = location.href.replace(rhash, '');

        // Decoding if the URL isn't UTF-8
        try {
          anchorUrl = decodeURIComponent(anchorUrl);
        } catch (error) {}
        try {
          locationUrl = decodeURIComponent(locationUrl);
        } catch (error) {}
        isLocal = anchor.hash.length > 1 && anchorUrl === locationUrl;
        return isLocal;
      };
    })(),

    _create: function() {
      var that = this;
      var options = this.options;
      this._addClass('ui-tabs', 'ui-widget ui-widget-content');
      this._toggleClass('ui-tabs-collapsible', null, options.collapsible);
      // Add sub-element class and data
      this._assembleTabs();
      // Initialize the selected items and display and hide the contents
      options.active = this._initialActive();

      if (this.anchors.length) {
        this.active = this._findActive(options.active);
      } else {
        this.active = $();
      }
      // Include _initactive
      this._refresh();
      if (this.active.length && this.active.length > 0) {
        this.load(options.active);
      }
    },

    _initialActive: function() {
      var active = this.options.active;
      var collapsible = this.options.collapsible;
      if (active === null) {
        active = this.tabs.length ? 0 : false;
      }
      if (active !== false) {
        var activeIndex = this.tabs.eq(active);
        active = this.tabs.index(activeIndex);
        if (active === -1) {
          collapsible ? (active = false) : (active = 0);
        }
      }
      return active;
    },

    _sanitizeSelector: function(hash) {
      hash = hash
        ? hash.replace(/[!"$%&'()*+,.\/:;<=>?@\[\]\^`{|}~]/g, '\\$&')
        : '';
      return hash;
    },

    _refresh: function() {
      var hiddenObj = { 'aria-hidden': 'true' };
      // Binding the Tabs Switching Event
      this._setupEvents(this.options.event);
      var active = this._getPanelForTab(this.active);
      var domHiden = this.panels.not(active);
      domHiden.hide();
      domHiden.attr(hiddenObj);
      this._initactive();
    },

    _initactive: function() {
      var showObj = { 'aria-hidden': 'false' };
      this._addClass(this.active, 'ui-tabs-active', 'ui-state-active');
      var domShow = this._getPanelForTab(this.active);
      domShow.show();
      domShow.attr(showObj);
    },

    _assembleTabs: function() {
      var that = this;
      var prevTabs = this.tabs;
      var prevAnchors = this.anchors;
      var prevPanels = this.panels;

      this.tablist = this._getList();
      this._addClass(
        this.tablist,
        'ui-tabs-nav',
        'ui-helper-reset ui-helper-clearfix ui-widget-header'
      );

      this.tablist
        .on('mousedown' + this.eventNamespace, '> li', function(event) {
          if ($(this).is('.ui-state-disabled')) {
            event.preventDefault();
          }
        })

        // Support: IE <9
        .on('focus' + this.eventNamespace, '.ui-tabs-anchor', function() {
          if (
            $(this)
              .closest('li')
              .is('.ui-state-disabled')
          ) {
            this.blur();
          }
        });

      this.tabs = this.tablist.find('> li:has(a[href])').attr({
        tabIndex: -1
      });
      this._addClass(this.tabs, 'ui-tabs-tab', 'ui-state-default');

      this.anchors = this.tabs.map(function() {
        return $('a', this)[0];
      });
      this.tabs.attr({
        tabIndex: -1
      });
      // set float
      this._addClass(this.anchors, 'ui-tabs-anchor');
      // Initialize the dom
      this.panels = $();

      this.anchors.each(function(i, anchor) {
        var selector, panel, panelId;
        var anchorId = $(anchor).attr('id');
        var tab = $(anchor).closest('li');

        // Inline tab
        if (that._isLocal(anchor)) {
          selector = anchor.hash;
          panelId = selector.substring(1);
          panel = that.element.find(that._sanitizeSelector(selector));
        }

        if (panel.length) {
          that.panels = that.panels.add(panel);
        }
        tab.attr({
          'aria-controls': panelId,
          'aria-labelledby': anchorId
        });
        // The aria specification adds labels to elements in an invisible manner
        panel.attr('aria-labelledby', anchorId);
      });

      this._addClass(this.panels, 'ui-tabs-panel', 'ui-widget-content');

      // Avoid memory leaks
      if (prevTabs) {
        this._off(prevTabs.not(this.tabs));
        this._off(prevAnchors.not(this.anchors));
        this._off(prevPanels.not(this.panels));
      }
    },

    _getList: function() {
      var List = this.tablist || this.element.find('ol, ul').eq(0);
      return List;
    },

    _setupEvents: function(event) {
      var events = {};
      // Event type event
      if (event) {
        $.each(event.split(' '), function(index, eventName) {
          events[eventName] = '_eventHandler';
        });
      }

      this._off(this.anchors.add(this.tabs).add(this.panels));
      this._on(this.anchors, events);
      this._focusable(this.tabs);
      this._hoverable(this.tabs);
    },
    // Binding anchors event
    _eventHandler: function(event) {
      var options = this.options;
      var active = this.active;
      var anchor = $(event.currentTarget);
      var tab = anchor.closest('li');
      var clickedIsActive = tab[0] === active[0];
      // Exit without switching the tabs
      if (clickedIsActive) {
        return;
      }

      var collapsing = clickedIsActive && options.collapsible;
      var toShow = (this.newPanel = collapsing
        ? $()
        : this._getPanelForTab(tab));
      // Panel to be hidden
      var toHide = (this.oldPanel = !active.length
        ? $()
        : this._getPanelForTab(active));
      this.oldTab = active;
      this.newTab = collapsing ? $() : tab;

      event.preventDefault();

      if (tab.hasClass('disabled')) {
        return;
      }
      // Canceling this Event
      if (this._trigger('beforeActivate', event) === false) {
        return;
      }
      // Record Selected Items
      options.active = collapsing ? false : this.tabs.index(tab);
      this.active = clickedIsActive ? $() : tab;

      if (!toHide.length && !toShow.length) {
        return;
      }

      if (toShow.length) {
        this.load(this.tabs.index(tab), event);
      }
      this._toggle(event);
    },

    // Handles show/hide for selecting tabs
    _toggle: function(event) {
      var that = this;
      var toShow = this.newPanel;
      var toHide = this.oldPanel;
      // is toggle active over
      this.runningMark = true;

      function aftershow() {
        that.runningMark = false;
        that._trigger('activate', event);
      }

      function complete() {
        that.running = false;
        that._trigger('activate', event);
      }

      function show() {
        that._addClass(
          that.newTab.closest('li'),
          'ui-tabs-active',
          'ui-state-active'
        );

        if (toShow.length && that.options.show) {
          that._show(toShow, that.options.show, complete);
        } else {
          toShow.show();
          aftershow();
        }
      }

      function _show(element, options, callback) {
        // add collapsing animate
        element.show();
        callback();
      }

      this._removeClass(
        that.oldTab.closest('li'),
        'ui-tabs-active',
        'ui-state-active'
      );
      toHide.hide();
      show();

      toHide.attr('aria-hidden', 'true');
      toShow.attr('aria-hidden', 'false');
    },

    _findActive: function(index) {
      if (index === false) {
        return $();
      }
      return this.tabs.eq(index);
    },

    _getIndex: function(index) {
      // index return a number
      if (typeof index === 'string') {
        index = this.anchors.index(
          this.anchors.filter("[href$='" + index + "']")
        );
      }
      return index;
    },

    load: function(index, event) {
      index = this._getIndex(index);
      var that = this;
      var tab = this.tabs.eq(index);
      var anchor = tab.find('.ui-tabs-anchor');
      var panel = this._getPanelForTab(tab);
      var eventData = {
        tab: tab,
        panel: panel,
        anchor: anchor
      };
      for (var i in eventData) {
        eventData[i].attr(ondragstart, 'return false');
      }

      // Reload
      if (this._isLocal(anchor[0])) {
        return;
      }
    },

    _getPanelForTab: function(tab) {
      var id = $(tab).attr('aria-controls');
      return this.element.find(this._sanitizeSelector('#' + id));
    },

    _toggleClass: function(element, keys, extra, add) {
      add = typeof add === 'boolean' ? add : extra;
      var shift = typeof element === 'string' || element === null;
      var options = {
        extra: shift ? keys : extra,
        keys: shift ? element : keys,
        element: shift ? this.element : element,
        add: add
      };
      var tgoption = this._classes(options);
      options.element.toggleClass(tgoption, add);
      return this;
    },

    _removeClass: function(element, keys, extra) {
      return this._toggleClass(element, keys, extra, false);
    },

    _addClass: function(element, keys, extra) {
      return this._toggleClass(element, keys, extra, true);
    },

    _classes: function(options) {
      var full = [];
      var that = this;

      var fulloptions = $.extend(
        {
          element: this.element,
          // Default Style Class Name
          classes: this.options.classes || {
            'ui-tabs': 'ui-corner-all',
            'ui-tabs-nav': 'ui-corner-all',
            'ui-tabs-panel': 'ui-corner-bottom',
            'ui-tabs-tab': 'ui-corner-top'
          }
        },
        options
      );

      function processClassString(classes, checkOption) {
        var current;
        for (var i = 0; i < classes.length; i++) {
          current = that.classesElementLookup[classes[i]] || $();
          if (fulloptions.add) {
            current = $(
              $.unique(current.get().concat(fulloptions.element.get()))
            );
          } else {
            current = $(current.not(fulloptions.element).get());
          }
          that.classesElementLookup[classes[i]] = current;
          full.push(classes[i]);
          if (checkOption && fulloptions.classes[classes[i]]) {
            full.push(fulloptions.classes[classes[i]]);
          }
        }
      }

      if (fulloptions.keys) {
        processClassString(fulloptions.keys.match(/\S+/g) || [], true);
      }
      if (fulloptions.extra) {
        processClassString(fulloptions.extra.match(/\S+/g) || []);
      }

      return full.join(' ');
    },

    _on: function(dom, events) {
      var tabs = this;
      if (!events) {
        events = dom;
        dom = this.element;
      } else {
        dom = $(dom);
        this.bindings = this.bindings.add(dom);
      }

      $.each(events, function(event, handler) {
        function _handle() {
          var uiinstance =
            typeof handler === 'string' ? tabs[handler] : handler;
          var handlerapply = uiinstance.apply(tabs, arguments);
          return handlerapply;
        }

        // for _off unbinding works
        if (typeof handler !== 'string') {
          _handle.guid = handler.guid =
            handler.guid || _handle.guid || $.guid++;
        }

        var match = event.match(/^([\w:-]*)\s*(.*)$/);
        var eventName = match[1] + tabs.eventNamespace;
        dom.on(eventName, _handle);
      });
    },

    _off: function(element, eventName) {
      var _eventName =
        (eventName || '').split(' ').join(this.eventNamespace + ' ') +
        this.eventNamespace;
      element.off(_eventName);

      this.bindings = $(this.bindings.not(element).get());
      this.focusable = $(this.focusable.not(element).get());
      this.hoverable = $(this.hoverable.not(element).get());
    },

    _hoverable: function(hoverelement) {
      this.hoverable = this.hoverable.add(hoverelement);
      this._on(hoverelement, {
        mouseenter: function(event) {
          this._addClass($(event.currentTarget), null, 'ui-state-hover');
        },
        mouseleave: function(event) {
          this._removeClass($(event.currentTarget), null, 'ui-state-hover');
        }
      });
    },

    _focusable: function(focuselement) {
      this.focusable = this.focusable.add(focuselement);
      this._on(focuselement, {
        focusin: function(event) {
          this._addClass($(event.currentTarget), null, 'ui-state-focus');
        },
        focusout: function(event) {
          this._removeClass($(event.currentTarget), null, 'ui-state-focus');
        }
      });
    },

    _trigger: function(type, event, data) {
      var callback = this.options[type];
      data = data || {};
      event = $.Event(event);
      // Event Type
      event.type = type.toLowerCase();
      event.target = this.element[0];
      var prop;
      var orig = event.originalEvent;
      if (orig) {
        for (prop in orig) {
          if (!(prop in event)) {
            event[prop] = orig[prop];
          }
        }
      }
      this.element.trigger(event, data);
      var istrigger = !(
        ($.isFunction(callback) &&
          callback.apply(this.element[0], [event].concat(data)) === false) ||
        event.isDefaultPrevented()
      );
      return istrigger;
    }
  };
})($);

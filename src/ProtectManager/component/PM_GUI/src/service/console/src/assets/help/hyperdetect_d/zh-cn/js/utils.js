/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

function isAbsPath(strPath) {
  var strUpper = strPath.toUpperCase();
  return strUpper.indexOf(':') !== -1 || strUpper.indexOf('\\\\') === 0;
}

function isAbsPathToHost(path) {
  return path.indexOf('/') === 0;
}

function getHost(path) {
  var pos = path.indexOf('//');
  if (pos > 0) {
    var posx = path.indexOf('/', pos + 2);
    if (posx > 0) return path.substring(0, posx);
    else return path;
  }
  return path;
}

function parseXML(data) {
  if (!data) {
    return null;
  }
  var xmlDoc = null;
  var trimXmlStr = data.trim();
  if (window.DOMParser) {
    var parser = new DOMParser();
    xmlDoc = parser.parseFromString(trimXmlStr, 'text/xml');
  } else {
    xmlDoc = new ActiveXObject('Microsoft.XMLDOM');
    xmlDoc.async = false;
    var index = trimXmlStr.indexOf('<?xml');
    if (index !== -1) {
      index = trimXmlStr.indexOf('?>', index);
      if (index !== -1) {
        var strXML = trimXmlStr.substr(index + 2);
        xmlDoc.loadXML(strXML);
      }
    } else {
      xmlDoc.loadXML(trimXmlStr);
    }
  }

  return xmlDoc;
}

function getFullPath(sPath, relPath) {
  if (isAbsPath(relPath)) {
    return relPath;
  } else if (isAbsPathToHost(relPath)) {
    return getHost(sPath) + relPath;
  }

  var fullPath = sPath;
  var pathPos = 0;
  while (pathPos !== -1) {
    pathPos = relPath.indexOf('../');
    if (pathPos !== -1) {
      relPath = relPath.substring(pathPos + 3);
      fullPath = fullPath.substring(0, fullPath.length - 1);
      var pos2 = fullPath.lastIndexOf('/');
      if (pos2 !== -1) {
        fullPath = fullPath.substring(0, pos2 + 1);
      } else {
        break;
      }
    }
  }
  fullPath += relPath;
  return fullPath;
}

function getPath(URL) {
  // remove the search and hash string
  var m = 0;
  var m1 = URL.indexOf('#');
  var m2 = URL.indexOf('?');
  if (m1 >= 0) {
    if (m2 >= 0) m = m1 > m2 ? m2 : m1;
    else m = m1;
  } else {
    if (m2 >= 0) m = m2;
    else m = URL.length;
  }
  URL = URL.substring(0, m);

  var sPathPos = URL.lastIndexOf('/');
  if (sPathPos > 0) return URL.substring(0, sPathPos + 1);
  else return '';
}
String.prototype.startsWith =
  String.prototype.startsWith ||
  function(str) {
    var reg = new RegExp('^' + str);
    return reg.test(this);
  };
String.prototype.endsWith =
  String.prototype.endsWith ||
  function(str) {
    var reg = new RegExp(str + '$');
    return reg.test(this);
  };

function loadScript(relativePath, scriptLoadedCallback) {
  var head = document.getElementsByTagName('head')[0];
  var script = document.createElement('script');
  script.type = 'text/javascript';

  // IE 9/10
  if (script.readyState) {
    script.onreadystatechange = function() {
      if (script.readyState === 'loaded' || script.readyState === 'complete') {
        script.onreadystatechange = null;
        scriptLoadedCallback();
      }
    };
    script.onerror = scriptLoadedCallback;
  }
  // Firefox file protocol compatibility
  else if (
    window.navigator.product === 'Gecko' &&
    window.navigator.userAgent.indexOf('KHTML') === -1 &&
    window.navigator.userAgent.indexOf('Trident') === -1 &&
    window.location.protocol === 'file:'
  ) {
    var request = new XMLHttpRequest();
    var sCurrentPath = getPath(decodeURI(document.location.href));
    var scriptURL = getFullPath(sCurrentPath, relativePath);
    request.open('GET', encodeURI(scriptURL), true);
    request.onreadystatechange = function() {
      if (
        request.readyState === 4 &&
        request.status !== 0 &&
        request.status !== 200
      ) {
        scriptLoadedCallback();
        // Script exists. For local files: Firefox < 35 returns 0 on every AJAX call. Firefox >= 35 returns 200 on success.
      }
    };
    try {
      request.send(null);
    } catch (e) {
      scriptLoadedCallback();
      return;
    }
    script.onload = scriptLoadedCallback;
  } else {
    script.onload = scriptLoadedCallback;
    script.onerror = scriptLoadedCallback;
  }

  script.src = relativePath;
  head.appendChild(script);
}

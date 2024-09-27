var tokensMap,
  keyWord,
  titleOrContext,
  fileNameMap,
  searching,
  noKeyWord,
  noResult,
  h1Map,
  searchDataKey;
var fileNameMapPath = "data/file_name_map.js";
var naviReady = "naviReady";
var indexPath = "data/index.js";
var indexReady = "indexReady";
var h1FilePath = "data/h1.js";
var i18n = {
  "zh": {
    "searching": "正在搜索...",
    "noKeyword": "搜索关键字不能为空.",
    "noResult": "未找到主题."
  }, "en": {
    "searching": "Searching...",
    "noKeyword": "Search keywords cannot be empty",
    "noResult": "No topics found."
  }
};
function Map() {
  // noinspection JSValidateTypes
  this.keys = [];
  this.data = {};

  this.set = function (key, value) {
    if (!this.data[key]) {
      if (this.keys.indexOf(key) === -1) {
        this.keys.push(key);
      }
    }
    this.data[key] = value;
  };

  this.get = function (key) {
    return this.data[key];
  };

  this.push = function (map) {
    var self = this;
    map.keys.forEach(function (item) {
      self.set(item, map.get(item));
    })
  };

  this.size = function () {
    return this.keys.length;
  }
}

/**
 * Displaying a message during search
 * @param tip
 */
function tips(tip) {
  document.getElementById("result-div").innerHTML = tip;
}

function initTips() {
  searching = "<span style=\"font-family: 宋体,serif;font-size: 12px;\">" + i18n[language].searching + "</span>";
  noKeyWord = "<span style=\"font-family: 宋体,serif;font-size: 12px;\">" + i18n[language].noKeyword + "</span>";
  noResult = "<span style=\"font-family: 宋体,serif;font-size: 12px;\">" + i18n[language].noResult + "</span>";
}

function Message(type, value) {
  this.type = type;
  this.value = value;
}

function JsInfo(name, baseUrl, ref, callback) {
  this.name = name;
  this.baseUrl = baseUrl;
  this.ref = ref;
  this.callback = callback;
}

function triggerEvent(type) {
  var event;
  if(typeof(Event) === 'function') {
    event = new Event(type);
  }else{
    event = document.createEvent('Event');
    event.initEvent(type, true, true);
  }
  window.dispatchEvent(event);
}

// Letters or digits
var digitalOrChar = new RegExp("[A-Za-z]|[0-9]");

function isDigitalOrChar(char) {
  return digitalOrChar.test(char);
}

function loadIndexFiles(jsInfos) {
  var startPromise = new Promise(function (resolve) {
    resolve();
  });
  function processJsInfo(jsInfo) {
    startPromise = startPromise.then(function () {
      searchDataKey = null;
      return new Promise(function (resolve) {
        loadScript(jsInfo.ref ? jsInfo.ref + "/" + jsInfo.baseUrl : jsInfo.baseUrl, function () {
          var xmlDoc = parseXML(searchDataKey);
          if (xmlDoc) {
            jsInfo.callback(xmlDoc, jsInfo);
          }
          resolve();
        });
      });
    })
  }

  for (var i = 0; i < jsInfos.length; i++) {
    processJsInfo(jsInfos[i]);
  }
  startPromise.then(function () {
    triggerEvent(indexReady);
  });
}

function fileName2Map(dom, jsInfo) {
  var ref = jsInfo.ref;
  var lastDom = dom.lastChild.childNodes;
  if (!fileNameMap) {
    fileNameMap = new Map();
  }
  var item, url, id;
  for (var i = 0; i < lastDom.length; i++) {
    item = lastDom[i];
    if (item.nodeType === 1) {
      id = item.getAttribute("i");
      url = item.getAttribute("f");
      if (ref) {
        fileNameMap.set(ref + "-" + id, ref + "/" + url);
      } else {
        fileNameMap.set(id, url);
      }
    }
  }
}

function handleTokensDom(data, jsInfo) {
  var ref = jsInfo.ref;
  var allTokens = data.lastChild.childNodes;
  if (!tokensMap) {
    tokensMap = new Map();
  }
  var tokenWithoutText = [];
  var paramTo;
  for (var i = 0; i < allTokens.length; i++) {
    paramTo = allTokens[i];

    // Obtains all keywords in the index token.
    if (paramTo.nodeType === 1) {
      tokenWithoutText.push(paramTo);
    }
  }
  tokensMap.set(ref, tokenWithoutText);
}


function h12Map(dom, jsInfo) {
  var ref = jsInfo.ref;
  var h1s = dom.lastChild.childNodes;
  if (!h1Map) {
    h1Map = new Map();
  }
  var item, id;
  for (var i = 0; i < h1s.length; i++) {
    item = h1s[i];
    if (item.nodeType === 1) {
      id = item.getAttribute("i");
      h1Map.set(ref ? ref + "-" + id : id, item.getAttribute("t"));
    }
  }
}

function needLoadFile() {
  var jsInfos = [];
  jsInfos.push(new JsInfo(null, indexPath, null, handleTokensDom));
  jsInfos.push(new JsInfo(null, fileNameMapPath, null, fileName2Map));
  jsInfos.push(new JsInfo(null, h1FilePath, null, h12Map));

  if (mergedProjects) {
    for (var i = 0; i < mergedProjects.length; i++) {
      jsInfos.push(new JsInfo(mergedProjects[i].name, indexPath, mergedProjects[i].ref, handleTokensDom));
      jsInfos.push(new JsInfo(mergedProjects[i].name, fileNameMapPath, mergedProjects[i].ref, fileName2Map));
      jsInfos.push(new JsInfo(mergedProjects[i].name, h1FilePath, mergedProjects[i].ref, h12Map));
    }
  }
  return jsInfos;
}

/**
 * Before searching, load the index file, file name and ID mapping file, and H1 file.
 */
function beforeSearch() {
  var searchOptionSelected = document.getElementById("titleOrContext");
  titleOrContext = searchOptionSelected.options[searchOptionSelected.selectedIndex].value;
  if (!tokensMap && !fileNameMap && !h1Map && titleOrContext === "2") {
    loadIndexFiles(needLoadFile());
  } else {
    triggerEvent(indexReady);
  }
}



function buttonDisabled(sid) {
  document.getElementById(sid).setAttribute("disabled", "disabled");
}



function search() {
  keyWord = document.getElementById("keyWord").value.trim();
  if (!keyWord || keyWord === "") {
    tips(noKeyWord);
    return;
  }
  buttonDisabled("searchButton");
  tips(searching);
  beforeSearch();
}
function _clickLiFunc() {
  clickLiFunc(this, titleOrContext);
}
function displayResult(nodes) {
  if (keyWord && nodes.length > 0) {
    document.getElementById("result-div").innerHTML =
      "<ul id=\"searchResult\" style=\"font-family: 宋体,serif;font-size: 12px;\"></ul>";
    for (var i = 0; i < nodes.length; i++) {
      var liNode = document.createElement("li");
      liNode.setAttribute("class", "sLi");
      liNode.setAttribute("value", nodes[i].id)
      liNode.setAttribute("local", nodes[i].local);
      liNode.onclick = _clickLiFunc
      liNode.appendChild(document.createTextNode(nodes[i].name));

      document.getElementById("searchResult").appendChild(liNode);
    }
  } else {
    tips(noResult);
  }
}


/**
 * Sort by occurrences
 * @param record
 * @param arr
 * @returns {Map}
 */
function hitsSort(record, arr) {
  var hitsNumMap = new Map();
  arr.forEach(function (id) {
    var path = fileNameMap.get(id);
    hitsNumMap.set(record.get(id).length + path, path);
  });
  var sortKeys = hitsNumMap.keys.sort(function (a, b) {
    return b.substring(0, b.lastIndexOf(hitsNumMap.get(b))) - a.substring(0, a.lastIndexOf(hitsNumMap.get(a)));
  });
  var sortedMap = new Map();
  var key;
  sortKeys.forEach(function (item) {
    key = hitsNumMap.get(item);
    sortedMap.set(key, record.get(key));
  });
  return sortedMap;
}



// Check whether keywords exist in the h1 header.
function h1Regexp(innerHtml) {
  var htmlTagPattern = "(<(\"[^\"]*\"|'[^']*'|[^'\">])*>)*";
  var keyWordArr = keyWord.split("");

  escapeCharacter(keyWordArr);
  var regex = keyWordArr.join(htmlTagPattern);
  var pattern = new RegExp(regex, "g");
  return innerHtml.match(pattern);
}

/**
 * Search for articles with keywords appearing in the H1 tag and sort them in descending order of occurrences
 * @param record Search result
 * @returns {Map} Sorting result
 */
function grepH1(record) {
  var h1Html;
  var match;
  var h1Match = record.keys.filter(function (id) {
    h1Html = h1Map.get(id);
    match = h1Regexp(h1Html);
    return typeof match !== "undefined" && match !== null;
  });
  return hitsSort(record, h1Match);
}

function hits(record, scoreMap) {
  var other = record.keys.filter(function (id) {
    return scoreMap.keys.indexOf(fileNameMap.get(id)) === -1;
  });
  if (other === null || typeof other === "undefined" || other.length === 0) {
    return new Map();
  }

  return hitsSort(record, other);
}

// Sort search results
function weightSort(record) {
  if (typeof record === "undefined" || record.size() === 0) {
    return null;
  }
  var scoreMap = new Map();

  // The rule for sorting search results is as follows:
  scoreMap.push(grepH1(record));
  scoreMap.push(hits(record, scoreMap));

  return scoreMap;
}


function getPositions(nextPositionsValue,positionsValue,beforeLength){
  return nextPositionsValue.filter(function (value) {
    return positionsValue.indexOf(String(value - beforeLength)) > -1;
  });
}

/**
 *
 * @param positions Accumulated search results before
 * @param nextPositions Search result of the current character
 * @param beforeLength Length of the previous result, for example,
 * "Description of abc". When "A" is found, the length of the previous result abc is
 * 3. (When word segmentation is performed, a single Chinese character segmentation is counted as a single word,
 * and a combination of letters and digits is counted as a whole.)
 * @param isBegin Indicates whether to search for the first word of a sentence, for example, "Description".
 * When "say" isBegin" is true.
 * @returns {*}
 */
function combineQueue(positions, nextPositions, beforeLength, isBegin) {
  var result = new Map();
  var positionsKeys = positions.keys;
  var nextPositionsKeys = nextPositions.keys;
  var resultKeys = [];
  // Indicates whether the matching is performed for the first time, that is,the first character or a string of digits and letters except for other characters.
  if (isBegin) {
    return nextPositions;
  } 
  // Check whether the file names overlap.
  resultKeys = positionsKeys.filter(function (value) {
    return nextPositionsKeys.indexOf(value) > -1;
  });

  // If no intersection exists, an empty map is returned and the search ends.
  if (resultKeys.length === 0) {
    return result;
  } 
  // If there is an intersection, check whether the character offset is consecutive.
  var positionsValue;
  var nextPositionsValue;
  var resultValue;
  for (var i = 0; i < resultKeys.length; i++) {
    positionsValue = positions.get(resultKeys[i]);
    nextPositionsValue = nextPositions.get(resultKeys[i]);
    // Obtain the intersection set,if The offsets of two search keywords in the same file are consecutive.
    resultValue=getPositions(nextPositionsValue,positionsValue,beforeLength);
    // If the offsets of two search keywords in the same file are consecutive,
    if (resultValue.length > 0) {
      result.set(resultKeys[i], resultValue);
    }
  }  
  return result;
}


function handleSliceEqualToken(token, ref, result) {
  var fileName
  var positions = token.childNodes;
  for (var j = 0; j < positions.length; j++) {
    if (positions[j].nodeType === 1) {
      fileName = positions[j].getAttribute("f");
      result.set(ref ? ref + "-" + fileName : fileName,
        positions[j].getAttribute("o").split(","));
    }
  }
}

// Searches for the matched token in the index.
function binarySearch(slice, matchTokensMap) {
  var result = new Map();
  var ref,
    paramto,
    tokens,
    tokenStr,
    tokensBegin,
    tokensEnd,
    mid;
  for (var i = 0, keys = matchTokensMap.keys; i < keys.length; i++) {
    ref = keys[i];
    tokens = matchTokensMap.get(keys[i]);
    tokensBegin = 0;
    tokensEnd = tokens.length - 1;
    while (tokensBegin <= tokensEnd) {
      mid = Math.floor((tokensBegin + tokensEnd) / 2);
      tokenStr = tokens[mid].getAttribute("n");
      if (slice < tokenStr) {
        tokensEnd = mid - 1;
      } else if (slice > tokenStr) {
        tokensBegin = mid + 1;
      } else {
        paramto = tokens[mid];
        handleSliceEqualToken(paramto, ref, result)
        
        break;
      }
    }
  }
  return result;
}

// Obtain word segmentation based on the search content and determine whether the word is matched based on the offset.
function analysis(keyword, matchTokensMap) {
  var keywordArr = keyword.split("");
  var char;
  var paramstr;
  var positions = new Map();
  // 1: letters or digits; 2: others (Chinese characters, punctuation marks)
  var type = 0;
  // A string consisting of letters and arrays.
  var slice;
  // Indicates whether the character is the first matched character.
  var isBegin = true;
  var beforeLength = 1;
  for (var i = 0, length = keywordArr.length; i < length; i++) {
    char = keywordArr[i];
    if (isDigitalOrChar(char)) {
      if (type !== 1) {
        slice = "";
        type = 1;
      }
      slice += char;

      // If the next character is at the end or is not a character or number, the index search is performed.
      var flag = (i + 1 >= keywordArr.length) || !isDigitalOrChar(keywordArr[i + 1])
      if (flag) {
        paramstr = binarySearch(slice, matchTokensMap);
      }      
      if (flag && paramstr.size() === 0) {
        return undefined
      }
      if (flag) {
        positions = combineQueue(positions, paramstr, beforeLength, isBegin);
        beforeLength = slice.length;
        isBegin = false;
      }
      // If the linked list is empty after being merged, the search fails.
      if (flag && positions.size === 0) {
        return undefined;
      }
    } else {
      type = 2;
      paramstr = binarySearch(char, matchTokensMap);
      if (paramstr.size() === 0) {
        return undefined;
      }
      positions = combineQueue(positions, paramstr, beforeLength, isBegin);
      beforeLength = 1;
      isBegin = false;
      // If the linked list is empty after being merged, the search fails.
      if (positions.size === 0) {
        return undefined;
      }
    }
  }

  return positions;
}

// Obtains all word sets related to keywords.
function getWordRec(keyword) {
  var paramto,
    tokens,
    matchTokens;
  var keys = tokensMap.keys;
  var matchTokensMap = new Map();
  for (var i = 0; i < keys.length; i++) {
    matchTokens = [];
    tokens = tokensMap.get(keys[i]);
    for (var j = 0; j < tokens.length; j++) {
      paramto = tokens[j];
      // Obtains all keywords in the index token.
      if (keyword.indexOf(paramto.getAttribute("n")) !== -1) {
        matchTokens.push(paramto);
      }
    }
    matchTokensMap.set(keys[i], matchTokens);
  }
  return analysis(keyword, matchTokensMap);
}


function buttonEnabled(sid) {
  document.getElementById(sid).removeAttribute("disabled");
}

/**
 * Search keyword main logic
 */
function searchKeyword() {
  var nodes = [];
  if (keyWord && keyWord !== "") {
    // Search Title
    if (titleOrContext === "1") {
      nodes = myTree.getNodesByParamFuzzy("name", keyWord);
      // Searching for Content
    } else if (titleOrContext === "2") {
      var result = getWordRec(keyWord.toLowerCase());
      var positions = weightSort(result);
      if (positions && positions.size() > 0) {
        positions.keys.forEach(function (item) {
          var node = myTree.getNodeByParam("local", item);
          node && nodes.push(node);
        });
      }
    }
    displayResult(nodes);
  }

  if (browser !== "Chrome") {
    changeLiStyle();
  }

  buttonEnabled("searchButton")
}


document.addEventListener('DOMContentLoaded', function () {
  document.getElementById('searchButton')
    .addEventListener('click', search);
});

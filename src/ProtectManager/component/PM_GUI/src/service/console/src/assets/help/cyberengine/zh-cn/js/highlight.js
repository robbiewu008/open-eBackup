var strHlStart = "<span style='color:black; background-color:yellow'>";
var strHlEnd = "</span>";

function regexEncode(str) {
  if (str.length === 0) {
    return "";
  }
  var s = str.replace(/\\/g, "\\\\");
  s = s.replace(/\./g, "\\.");
  s = s.replace(/\^/g, "\\^");
  s = s.replace(/\|/g, "\\|");
  s = s.replace(/\$/g, "\\$");
  s = s.replace(/\[/g, "\\[");
  s = s.replace(/\?/g, "\\?");
  s = s.replace(/\+/g, "\\+");
  s = s.replace(/\*/g, "\\*");
  s = s.replace(/\(/g, "\\(");
  s = s.replace(/\)/g, "\\)");
  return s;
}

function htmlEncode(str) {
  if (str.length === 0) {
    return "";
  };
  return str.replace(/[&<>]/g,function(match){
    switch(match){
      case "&":return "&amp;";
      case "<":return "&lt;";
      case ">":return "&gt;";
      default:
        break
    }
  });
}

function encode(str) {
  return regexEncode(htmlEncode(str));
}

function htmlDecode(str) {
  if (str.length === 0) {
    return "";
  }
  return str.replace(/&\w+;/g,function(match){
    return {"&amp;":"&","&lt;":"<","&gt;":">"}[match];
  });

}

function replaceStr(matchStr) {
  var result;
  var patternStart = "<(?:\"[^\"]*\"|";
  var patternEnd = "'[^']*'|[^'\">])*>";
  var pattern = new RegExp(patternStart + patternEnd, "gi");
  if (pattern.test(matchStr)) {
    var matchStrWithoutTag = htmlDecode(matchStr.replace(pattern, ""));

    // To know where the search starts in the matching string
    var beginIndex = matchStrWithoutTag.indexOf(keyWord);

    // To know where the search ends in the matching string
    var lastLength = keyWord.length;

    // Matches strings without tags in strings.
    var split = matchStr.split(pattern);

    // Matches HTML tags in strings.
    var htmlTagArr = matchStr.match(pattern);
    var resultArr = [];
    var slice;
    var decodeStr;
    for (var i = 0; i < split.length; i++) {
      if (!split[i] || split[i] === "") {
        resultArr.push(htmlTagArr[i]);
        continue;
      }

      // Encoded for comparison with keywords.
      decodeStr = htmlDecode(split[i]);

      // All strings that match keywords must be escaped in HTML format.
      if (i === 0) {
        slice = decodeStr.charAt(0) + htmlEncode(decodeStr.slice(1, beginIndex)) + strHlStart +
          htmlEncode(decodeStr.slice(beginIndex)) + strHlEnd;
        lastLength = lastLength - (decodeStr.length - beginIndex);
      } else if (i === split.length - 1) {
        slice = strHlStart + htmlEncode(decodeStr.slice(0, lastLength)) + strHlEnd +
          htmlEncode(decodeStr.slice(lastLength, decodeStr.length - 1)) +
          decodeStr.charAt(decodeStr.length - 1);
      } else {
        lastLength = lastLength - decodeStr.length;
        slice = strHlStart + split[i] + strHlEnd;
      }
      resultArr.push(slice);
      resultArr.push(htmlTagArr[i]);
    }
    result = resultArr.join("");
  } else {
    result = matchStr.replace(new RegExp(encode(keyWord), "gi"), strHlStart + matchStr.match(new RegExp(encode(keyWord), "gi"))[0] + strHlEnd);
  }
  return result;
}

// Deduplication
function deduplicateArr(matchArr) {
  return matchArr ? matchArr.filter(function (item, index) {
    return matchArr.indexOf(item, 0) === index;
  }) : null;
}

function highlightInnerHtml(matchArr, body) {
  if (!matchArr || matchArr.length === 0) {
    return;
  }
  var innerHtml = body.innerHTML;
  var replace;
  var matchStr;
  var replaceReg;
  for (var i = 0; i < matchArr.length; i++) {
    matchStr = matchArr[i];
    replace = replaceStr(matchStr);
    replaceReg = new RegExp(regexEncode(matchStr), "g");
    innerHtml = innerHtml.replace(replaceReg, replace);
  }
  body.innerHTML = innerHtml;
}

// Special characters in regular expressions and HTML special characters are escaped for regular expression matching.
function escapeCharacter(keyWordArr) {
  for (var j = 0, length = keyWordArr.length; j < length; j++) {
    keyWordArr[j] = encode(keyWordArr[j]);
  }
}

function regexStrings(innerHtml) {
  var htmlTagPattern = "(<(\"[^\"]*\"|'[^']*'|[^'\">])*>)*";
  var regexTag="(([^><])*";
  var regexEnd="<){1}";
  var keyWordArr = keyWord.split("");

  escapeCharacter(keyWordArr);
  var regex = keyWordArr.join(htmlTagPattern);
  regex = "(>[^><]*){1}" + regex + regexTag + regexEnd;
  var pattern = new RegExp(regex, "gi");
  return innerHtml.match(pattern);
}

function highlight(body) {
  if (!body) {
    return;
  }
  var innerHtml = body.innerHTML;

  // Regular expression matching in the body tag
  var matchArr = regexStrings(innerHtml);

  // Deduplication and replacement of the same item at a time
  matchArr = deduplicateArr(matchArr);
  highlightInnerHtml(matchArr, body);
}


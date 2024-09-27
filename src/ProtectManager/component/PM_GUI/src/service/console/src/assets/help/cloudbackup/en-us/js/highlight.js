/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2023. All rights reserved.
 */

var strHlStart = "<span style='color:black; background-color:yellow'>";
var strHlEnd = '</span>';

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

function regexStrings(innerHtml) {
  var htmlTagPattern = '(<("[^"]*"|\'[^\']*\'|[^\'">])*>)*';
  var regexEnd = '<){1}';
  var regexTag = '(([^><])*';
  var keyWordArr = keyWord.split('');

  escapeCharacter(keyWordArr);
  var regex = keyWordArr.join(htmlTagPattern);
  regex = '(>[^><]*){1}' + regex + regexTag + regexEnd;
  var pattern = new RegExp(regex, 'g');
  return innerHtml.match(pattern);
}

// Special characters in regular expressions and HTML special characters are escaped for regular expression matching.
function escapeCharacter(keyWordArr) {
  for (var j = 0, length = keyWordArr.length; j < length; j++) {
    keyWordArr[j] = encode(keyWordArr[j]);
  }
}

function encode(str) {
  return regexEncode(htmlEncode(str));
}

function htmlEncode(str) {
  if (str.length === 0) {
    return '';
  }
  return str.replace(/[&<>]/g, function(match) {
    switch (match) {
      case '<':
        return '&lt;';
      case '&':
        return '&amp;';
      case '>':
        return '&gt;';
    }
  });
}

function htmlDecode(str) {
  if (str.length === 0) {
    return '';
  }
  return str.replace(/&\w+;/g, function(match) {
    return { '&amp;': '&', '&lt;': '<', '&gt;': '>' }[match];
  });
}

function regexEncode(str) {
  if (str.length === 0) {
    return '';
  }
  var s = str.replace(/\\/g, '\\\\');
  s = s.replace(/\|/g, '\\|');
  s = s.replace(/\./g, '\\.');
  s = s.replace(/\^/g, '\\^');
  s = s.replace(/\[/g, '\\[');
  s = s.replace(/\$/g, '\\$');
  s = s.replace(/\?/g, '\\?');
  s = s.replace(/\*/g, '\\*');
  s = s.replace(/\+/g, '\\+');
  s = s.replace(/\)/g, '\\)');
  s = s.replace(/\(/g, '\\(');
  return s;
}

function highlightInnerHtml(matchArr, body) {
  if (!matchArr || matchArr.length === 0) {
    return;
  }
  var innerHtml = body.innerHTML;
  var replace, replaceReg;
  for (var i = 0; i < matchArr.length; i++) {
    replace = replaceStr(matchArr[i]);
    replaceReg = new RegExp(regexEncode(matchArr[i]), 'g');
    innerHtml = innerHtml.replace(replaceReg, replace);
  }
  body.innerHTML = innerHtml;
}

// eg:
// Search for BSC6910 and download the product documentation package.
// Search <span id="ZH-CN_CONCEPT_0137072973__ph1711863125115">BSC6910</span>
function replaceStr(matchStr) {
  var resultStr;
  var patternStart = '<(?:"[^"]*"|';
  var patternEnd = "'[^']*'|[^'\">])*>";
  var pattern = new RegExp(patternStart + patternEnd, 'g');
  if (pattern.test(matchStr)) {
    var matchStrWithoutTag = htmlDecode(matchStr.replace(pattern, ''));

    // To know where the search ends in the matching string
    var lastLength = keyWord.length;

    // To know where the search starts in the matching string
    var beginIndex = matchStrWithoutTag.indexOf(keyWord);

    // Matches strings without tags in strings.
    var matchArr = matchStr.split(pattern);

    // Matches HTML tags in strings.
    var htmlTagArr = matchStr.match(pattern);
    var resultArr = [];
    var slice, decodeStr;
    for (var i = 0; i < matchArr.length; i++) {
      if (!matchArr[i] || matchArr[i] === '') {
        resultArr.push(htmlTagArr[i]);
        continue;
      }

      // Encoded for comparison with keywords.
      decodeStr = htmlDecode(matchArr[i]);

      // All strings that match keywords must be escaped in HTML format.
      if (i === 0) {
        slice =
          decodeStr.charAt(0) +
          htmlEncode(decodeStr.slice(1, beginIndex)) +
          strHlStart +
          htmlEncode(decodeStr.slice(beginIndex)) +
          strHlEnd;
        lastLength = lastLength - (decodeStr.length - beginIndex);
      } else if (i === matchArr.length - 1) {
        slice =
          strHlStart +
          htmlEncode(decodeStr.slice(0, lastLength)) +
          strHlEnd +
          htmlEncode(decodeStr.slice(lastLength, decodeStr.length - 1)) +
          decodeStr.charAt(decodeStr.length - 1);
      } else {
        slice = strHlStart + matchArr[i] + strHlEnd;
        lastLength = lastLength - decodeStr.length;
      }
      resultArr.push(slice);
      resultArr.push(htmlTagArr[i]);
    }
    resultStr = resultArr.join('');
  } else {
    resultStr = matchStr.replace(
      new RegExp(encode(keyWord), 'g'),
      strHlStart + htmlEncode(keyWord) + strHlEnd
    );
  }
  return resultStr;
}

// Deduplication
function deduplicateArr(matchArr) {
  return matchArr.filter(function(item, index, arr) {
    return matchArr.indexOf(item, 0) === index;
  });
}

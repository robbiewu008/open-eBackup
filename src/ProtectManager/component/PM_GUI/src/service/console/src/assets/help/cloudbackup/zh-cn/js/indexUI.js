/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */
var setting = {
    data: {
      simpleData: {
        enable: true,
        idKey: 'id',
        pIdKey: 'parentId',
        rootPId: 0
      }
    },
    key: {
      name: 'name',
      local: 'local'
    },
    view: {
      showLine: false,
      selectedMulti: false,
      showIcon: true
    },
    callback: {
      beforeClick: beforeClickTreeNode,
      beforeExpand: beforeExpandTreeNode,
      onExpand: onExpandTreeNode
    }
  },
  urlParentPath,
  mainPage,
  mainNavi,
  topMainPage,
  src,
  browser = '',
  windowHeight = 0,
  windowWidth = 0,
  naviData,
  timer,
  language,
  mergedProjects,
  topLanguage,
  topMergedProjects,
  naviFilePath = 'data/nav_json.js';

function loadMergedProjects() {
  var jsInfos = [];
  naviData = null;
  var startPromise = new Promise(function(resolve) {
    resolve();
  });
  if (mergedProjects) {
    for (var i = 0; i < mergedProjects.length; i++) {
      jsInfos.push(
        new JsInfo(
          mergedProjects[i].name,
          naviFilePath,
          mergedProjects[i].ref,
          assembleNavi
        )
      );
    }
  }
  for (var i = 0; i < jsInfos.length; i++) {
    (function(jsInfo) {
      startPromise = startPromise.then(function() {
        naviData = null;
        return new Promise(function(resolve) {
          loadScript(
            jsInfo.ref ? jsInfo.ref + '/' + jsInfo.baseUrl : jsInfo.baseUrl,
            function() {
              if (naviData) {
                jsInfo.callback(naviData, jsInfo);
              }
              resolve();
            }
          );
        });
      });
    })(jsInfos[i]);
  }
  startPromise.then(function() {
    triggerEvent(naviReady);
  });
}

function assembleNavi(childNavigation, jsInfo) {
  var name = jsInfo.name;
  var ref = jsInfo.ref;

  //Clear the mergeProject attribute in the subproject navigation tree and change the relative path of each
  // page of the subproject to ref + local.
  function clearChildNaviData(obj, ref) {
    function addRef(child, ref) {
      var local = child.local;
      if (ref && $.trim(local)) {
        child.local = ref + '/' + local;
      }
    }

    if (obj.mergeProject) {
      obj.mergeProject = null;
    } else {
      if (obj.children != null) {
        for (var i = 0; i < obj.children.length; i++) {
          clearChildNaviData(obj.children[i], ref);
          addRef(obj.children[i], ref);
        }
      } else if (obj.length) {
        for (var i = 0; i < obj.length; i++) {
          clearChildNaviData(obj[i], ref);
          addRef(obj[i], ref);
        }
      }
    }
  }

  clearChildNaviData(childNavigation, ref);
  var childJsonStr = JSON.stringify(childNavigation);

  // If the value is jsonArray, delete the brackets ([]) on both sides.
  if (childNavigation instanceof Array) {
    childJsonStr = childJsonStr.substring(1, childJsonStr.length - 1);
  }

  // Locate the subproject in the parent project and replace it with a character string.
  function findAnchor(obj, name, childNavi) {
    if (obj.mergeProject === name) {
      return obj;
    } else {
      var result;
      if (obj.children != null) {
        for (var i = 0; i < obj.children.length; i++) {
          result = findAnchor(obj.children[i], name, childNavi);
          if (result) {
            return result;
          }
        }
      } else if (obj.length) {
        for (var i = 0; i < obj.length; i++) {
          result = findAnchor(obj[i], name, childNavi);
          if (result) {
            return result;
          }
        }
      }
    }
  }

  var matchJsonObj = findAnchor(mainNavi, name, childNavigation);
  var matchJsonStr = JSON.stringify(matchJsonObj);
  var mainNaviStr = JSON.stringify(mainNavi);
  var mainNaviNew = mainNaviStr.replace(matchJsonStr, childJsonStr);
  mainNavi = JSON.parse(mainNaviNew);
}

function init() {
  initArgs();
  initMergedProjects();
  judgeBrowserType();
  initParentPath();
  loadMergedProjects();
  setTitle();
  setFont();
  addListener();
  initTips();
}

function addListener() {
  window.addEventListener(indexReady, searchKeyword, false);
  window.addEventListener(naviReady, initZTree, false);
}

function setTitle() {
  if (language === 'en') {
    $('.search-title').text('Search(S)');
    $('.catalog-title').text('Catalog(C)');
    $('.span-search').text('Please enter the keywords to search(W):');
    $('.span-search-result').text('Search result:');
    $('#keyWord').attr('placeholder', 'Please enter the keywords');
    $('#searchButton').attr('value', 'Search');
    setSelection();
    document.title = 'Online Help';
  }
}

function setFont() {
  if (language === 'en') {
    let style = document.createElement('style');
    style.setAttribute('type', 'text/css');
    style.innerHTML =
      '.tree * {padding:0; margin:0; font-family: Arial, "宋体", Helvetica, AppleGothic, sans-serif}';
    document
      .getElementsByTagName('head')
      .item(0)
      .appendChild(style);
  }
}

function setSelection() {
  var select = $('#titleOrContext');
  select.empty();
  select.append('<option value="1">Title</option>');
  select.append('<option value="2" selected>Content</option>');
}

function initZTree() {
  $.fn.zTree.init($('#zTree'), setting, mainNavi);

  initPage();
  var url = document.location.toString();
  var index = url.indexOf('#');

  if (index >= 0) {
    var param = url.split('#')[1].split('?')[0];
    openNodeByTopicUrl(param);
  } else {
    openNodeById('first');
  }
}

function beforeClickTreeNode(treeId, treeNode) {
  showTopic(treeNode);
}

function beforeExpandTreeNode(treeId, treeNode) {}

function onExpandTreeNode(treeId, treeNode) {}

function openNodeById(treeId, isHighlight) {
  var treeObj = $.fn.zTree.getZTreeObj('zTree');
  var node = '';
  if (treeId === 'first') {
    if (mainPage) {
      node = treeObj.getNodeByParam('local', mainPage);
      // 首页名字忽略大小写
      var ignoreCase = function(mainPage) {
        var nodes = treeObj.getNodes();
        var findkey = 'local';
        if (!nodes || !findkey) return null;
        for (var i = 0, l = nodes.length; i < l; i++) {
          var node = nodes[i];
          if (node[findkey].toLowerCase() == mainPage.toLowerCase()) {
            // 返回同名节点
            return nodes[i];
          }
        }
        return null;
      };
      if (null == node) {
        node = ignoreCase(mainPage);
      }
    } else {
      var nodes = treeObj.getNodes();
      if (nodes.length > 0) {
        node = nodes[0]; //The root node is selected by default.
        while ($.trim(node.local) === '') {
          node = node.children[0];
        }
      }
    }
  } else {
    node = treeObj.getNodeByTId(treeId);
  }

  treeObj.selectNode(node);
  showTopic(node, isHighlight);
}

function openNodeByTopicUrl(url) {
  var treeObj = $.fn.zTree.getZTreeObj('zTree');
  var node = treeObj.getNodeByParam('local', url);
  treeObj.selectNode(node);
  showTopic(node);
}

function getTopic(url, isHighlight) {
  var iframeDom = document.getElementById('iframeContent');
  iframeDom.contentWindow.location.replace(url);
  $('#iframeContent').unbind();
  $('#iframeContent').on('load', { highlight: isHighlight }, function(e) {
    //Add a click event to the a tag in the iFarme.
    var aText = $(this)
      .contents()
      .find('a');

    if (aText && aText.length > 0) {
      for (var i = 0; i < aText.length; i++) {
        $(aText[i]).bind('click', { index: i }, clickHandler);
      }
    }

    function clickHandler(ti) {
      var index = ti.data.index;
      var href = $(aText[index]).attr('href');
      var tmpSrc = src;

      if (href.indexOf('/') < 0) {
        tmpSrc = tmpSrc.substr(0, tmpSrc.lastIndexOf('/'));
      } else {
        var reg = eval(/\.\.\//gi);
        var reIndex = href.match(reg).length;
        href = href.replace(/\.\.\//gi, '');
        for (var i = 0; i <= reIndex; i++) {
          tmpSrc = tmpSrc.substr(0, tmpSrc.lastIndexOf('/'));
        }
      }

      src = tmpSrc + '/' + href;
      var realHref = src.substring(urlParentPath.length + 1, src.length);
      var treeObj = $.fn.zTree.getZTreeObj('zTree');
      // get the node URL
      var hasjinghao = realHref.indexOf('#');
      if (hasjinghao > 0) {
        realHref = realHref.substring(0, hasjinghao);
      }
      var node = treeObj.getNodeByParam('local', realHref);
      treeObj.selectNode(node);
    }

    if (timer) {
      clearInterval(timer);
    }
    var isHighlight = e.data.highlight;
    if (isHighlight === 2) {
      highlight(iframeDom.contentWindow.document.body);
      e.data.highlight = undefined;
    }
    $('#iframeContent').height($('.content_div').height());
    var error = false;
    try {
      error = !('document' in iframeDom.contentWindow);
    } catch (e) {
      error = true;
    }
    if (iframeDom.contentWindow.document.contentType === 'application/pdf') {
      // 防止pdf打不开
    } else if (
      error ||
      iframeDom.contentWindow.document.body.scrollHeight === 0
    ) {
      $('.div-h5').css('display', 'block');
      $('#iframeContent').css('display', 'none');
    }

    // selectSearchNode
    var selectSearchNode = function(url) {
      var local = url;
      var searchNodes = document.querySelectorAll('#searchResult > li');
      var currentSelectNode;
      for (var i = 0; i < searchNodes.length; i++) {
        if (searchNodes[i].getAttribute('local') === local) {
          currentSelectNode = searchNodes[i];
          break;
        }
      }
      if (currentSelectNode) {
        //Remove styles from other sibling elements
        $(currentSelectNode)
          .siblings()
          .removeClass('selected');
        //Adds the style of the current element
        $(currentSelectNode).addClass('selected');
      }
    };
    // selectTreeNode
    var niframeDom = e.target;
    var newurl = niframeDom.contentWindow.location;
    // Prevent Google Errors ,Blocked a frame with origin "null" from accessing a cross-origin frame
    if (Object.keys(newurl).length > 0) {
      // Refresh selectNode after browser returns
      var realHref = newurl.href;
      realHref = getBasename(realHref);
      var hasjinghao = realHref.indexOf('#');
      if (hasjinghao > 0) {
        realHref = realHref.substring(0, hasjinghao);
      }
      var treeObj = $.fn.zTree.getZTreeObj('zTree');
      var node = treeObj.getNodeByParam('local', realHref);
      treeObj.selectNode(node);
      selectSearchNode(realHref);
    }
  });
}

function initParentPath() {
  var url = document.location.toString();

  if (url.indexOf('?') != -1) {
    url = url.substring(0, url.indexOf('?'));
  }

  if (url.indexOf('#') < 0) {
    urlParentPath = url.substring(0, url.lastIndexOf('/'));
  } else {
    var urlNoParam = url.split('#')[0];
    urlParentPath = urlNoParam.substring(0, urlNoParam.lastIndexOf('/'));
  }
}

function showTopic(treeNode, isHighlight) {
  $('.div-h5').css('display', 'none');
  $('#iframeContent').css('display', 'block');

  var sign = true;

  if (treeNode) {
    var local = treeNode.local;
    if ($.trim(local)) {
      src = urlParentPath + '/' + local;
      getTopic(src, isHighlight);
      sign = false;
    }
  }
  if (sign) {
    $('.div-h5').css('display', 'block');
    $('#iframeContent').css('display', 'none');
  }
}

function judgeBrowserType() {
  var explorer = navigator.userAgent;
  if (explorer.indexOf('MSIE 7.0') >= 0) {
    browser = 'ie7';
  } else if (explorer.indexOf('MSIE 8.0') >= 0) {
    browser = 'ie8';
  } else if (explorer.indexOf('MSIE 9.0') >= 0) {
    browser = 'ie9';
  } else if (explorer.indexOf('MSIE 10.0') >= 0) {
    browser = 'ie10';
  } else if (explorer.indexOf('Opera') >= 0) {
    browser = 'Opera';
  } else if (explorer.indexOf('Safari') >= 0) {
    browser = 'Safari';
  } else if (explorer.indexOf('Chrome') >= 0) {
    browser = 'Chrome';
  } else if (explorer.indexOf('Firefox') >= 0) {
    browser = 'Firefox';
  } else if (explorer.indexOf('Netscape') >= 0) {
    browser = 'Netscape';
  } else {
    browser = 'ie';
  }
}

function clickLiFunc(e, isHighlight) {
  var treeId = $(e).attr('value');
  //Remove styles from other sibling elements
  $(e)
    .siblings()
    .removeClass('selected');
  //Adds the style of the current element
  $(e).addClass('selected');
  openNodeById(treeId, isHighlight);
}

function changeTitleStyle() {
  $('#zTree').css('font-size', '12px');
  $('.li-a').css('font-size', '12px');
  $('#zTree').css('font-size', '12px');
  $('#keyWord').css('font-size', '12px');
  $('#keyWord').css('font-family', '宋体');
  $('.li-a').css('font-size', '12px');
  $('.span-search').css('font-size', '12px');
  $('.span-search-result').css('font-size', '12px');
}

function changeLiStyle() {
  $('.sLi').css('white-space', 'nowrap');
  $('.sLi').css('*zoom', '1');
  $('.sLi').css('*display', 'inline');
}

function initPage() {
  //Adapting to the browser style
  if (browser !== 'Chrome') {
    changeTitleStyle();
  } else {
    $('#searchResult').css('width', 'max-content');
  }

  if (browser === 'ie7' || browser === 'ie8') {
    $('html').css('overflow', 'hidden');
    $('.nav_div').css('border-top-width', '0px');
  }

  if (browser === 'ie7') {
    $('.tabs_ul').css('height', '30px');
  }
}

function heightAdjustment() {
  AdjustWindow();
  if (src) {
    getTopic(src);
  }
}

function AdjustWindow() {
  windowHeight = $(window).height();
  windowWidth = $(window).width();

  $('#zTree').css('height', windowHeight - 100 + 'px');
  $('.nav_div').css('height', windowHeight - 61 + 'px');
  $('.content_div').css('height', windowHeight - 52.6 + 'px');
  $('.result-div').css('height', windowHeight - 158 + 'px');

  $('body').css('width', windowWidth + 'px');
  $('.content_div').css('width', windowWidth - 281 + 'px');
}

// This step is required because the system loading is supported. The sub-nav_json.js file is read.
// The file also contains the topLanguage attribute. Do not overwrite this attribute.
// You can assign a value to the sub-nav_json.js file again.
function initArgs() {
  language = topLanguage;
  mainNavi = naviData;
  mainPage = topMainPage;
}

// This step is required because the RAT loading is supported. The nav_json.js sub-file is read.
// The file also contains the topMergedProjects attribute. Do not overwrite this attribute. Set this attribute again.
function initMergedProjects() {
  mergedProjects = topMergedProjects;
}

// Obtain the file name + file extension in the path
function getBasename(tail) {
  // Regex to split the tail part of the above into [*, dir, basename, ext]
  var splitTailRe = /^([\s\S]*?)((?:\.{1,2}|[^\\\/]+?|)(\.[^.\/\\]*|))(?:[\\\/]*)$/;

  // Split the tail into dir, basename and extension
  var result = splitTailRe.exec(tail);
  var basename = result[2];
  return basename;
}

$(document).ready(function() {
  init();

  //Using the Tab Plug-in

  $('#tabs').uiTabs();

  AdjustWindow();
});

//Adapted page size

$(window).resize(heightAdjustment);

var urlParentPath,
  mainPage,
  topMainPage,
  mainNavi,
  src,
  browser = '',
  windowHeight = 0,
  windowWidth = 0,
  timer,
  naviData,
  language,
  topLanguage,
  mergedProjects,
  topMergedProjects,
  myTree;
var naviFilePath = 'data/nav_json.js';
function selectTreeNode(url) {
  if (url.indexOf('#') > 0) {
    url = url.split('#')[0];
  }
  var local = url.substring(urlParentPath.length + 1, url.length);
  var node = myTree.getNodeByParam('local', local);
  if (node) {
    myTree.selectNode(node);
  }
}

function hasClass(elements, cName) {
  return !!elements.className.match(new RegExp('(\\s|^)' + cName + '(\\s|$)'));
}

function removeClass(elements, cName) {
  if (hasClass(elements, cName)) {
    elements.className = elements.className.replace(
      new RegExp('(\\s|^)' + cName + '(\\s|$)'),
      ' '
    );
  }
}

function removeSiblingsSelected(e) {
  var childNodes = e.parentNode.childNodes;
  for (var i = 0; i < childNodes.length; i++) {
    removeClass(childNodes[i], 'selected');
  }
}

function addClass(elements, cName) {
  if (!hasClass(elements, cName)) {
    elements.className = elements.className.trim();
    elements.className += ' ' + cName;
  }
}

function selectSearchNode(url) {
  if (url.indexOf('#') > 0) {
    url = url.split('#')[0];
  }
  var local = url.substring(urlParentPath.length + 1, url.length);
  var searchNodes = document.querySelectorAll('#searchResult > li');
  var currentSelectNode;
  for (var i = 0; i < searchNodes.length; i++) {
    if (searchNodes[i].getAttribute('local') === local) {
      currentSelectNode = searchNodes[i];
      break;
    }
  }
  if (currentSelectNode) {
    removeSiblingsSelected(currentSelectNode);
    addClass(currentSelectNode, 'selected');
  }
}

function changeClassElementStyle(className, style, value) {
  var elementsByClassName = document.getElementsByClassName(className);
  for (var i = 0; i < elementsByClassName.length; i++) {
    elementsByClassName[i].style[style] = value;
  }
}

function addClickHandler(aText) {
  function clickHandler(index) {
    src = aText[index].href;
    selectTreeNode(src);
  }
  if (!aText || aText.length == 0) {
    return;
  }
  for (var i = 0; i < aText.length; i++) {
    if (
      !aText[i].href ||
      !(
        aText[i].href.endsWith('.html') ||
        (aText[i].href.indexOf('#') > 0 &&
          aText[i].href.split('#')[0].endsWith('.html'))
      )
    ) {
      return;
    }
    (function(index) {
      aText[index].onclick = function() {
        clickHandler(index);
      };
    })(i);
  }
}

function getTopic(url, isHighlight) {
  var iframeDom = document.getElementById('iframeContent');
  iframeDom.contentWindow.location.href = url;
  iframeDom.onload = function() {
    // Add a click event to the a tag in the iFarme.
    var iframe = document.getElementById('iframeContent').contentDocument;
    var aText = iframe.querySelectorAll('a');

    addClickHandler(aText);
    selectTreeNode(iframe.location.href);
    selectSearchNode(iframe.location.href);

    if (timer) {
      clearInterval(timer);
    }

    if (isHighlight === '2') {
      highlight(iframeDom.contentWindow.document.body);
    }
    document.getElementById(
      'iframeContent'
    ).style.height = document.getElementsByClassName(
      'content_div'
    )[0].style.height;
    var error;
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
      changeClassElementStyle('div-h5', 'display', 'block');
      document.getElementById('iframeContent').style.display = 'none';
    }
  };
}

function showTopic(treeNode, isHighlight) {
  changeClassElementStyle('div-h5', 'display', 'none');
  document.getElementById('iframeContent').style.display = 'block';

  var sign = true;

  if (treeNode) {
    var local = treeNode.local;
    if (local && local.trim()) {
      src = urlParentPath + '/' + local;
      getTopic(src, isHighlight);
      sign = false;
    }
  }
  if (sign) {
    changeClassElementStyle('div-h5', 'display', 'block');
    document.getElementById('iframeContent').style.display = 'none';
  }
}

function beforeClickTreeNode(treeNode) {
  showTopic(treeNode);
}

var container = document.getElementById('tree');
var treeSetting = {
  container: container,
  callback: {
    beforeClick: beforeClickTreeNode
  }
};

function initArgs() {
  language = topLanguage || 'zh';
  mainNavi = naviData;
  mainPage = topMainPage;
}

// This step is required because the RAT loading is supported. The nav_json.js sub-file is read.
function initMergedProjects() {
  mergedProjects = topMergedProjects;
}

function AdjustWindow() {
  windowHeight = window.innerHeight;
  windowWidth = window.innerWidth;
  var titleHeight = 0;
  var elementById = document.getElementById('cpTitle');
  if (!elementById) {
    titleHeight = 49;
  }
  document.getElementById('tree').style.height =
    windowHeight - 100 + titleHeight + 'px';
  changeClassElementStyle(
    'nav_div',
    'height',
    windowHeight - 61 + titleHeight + 'px'
  );
  changeClassElementStyle(
    'content_div',
    'height',
    windowHeight - 52.6 + titleHeight + 'px'
  );
  changeClassElementStyle('content_div', 'width', windowWidth - 281 + 'px');
  changeClassElementStyle('result-div', 'height', windowHeight - 158 + 'px');
  var elementsByTagName = document.getElementsByTagName('body');
  for (var i = 0; i < elementsByTagName.length; i++) {
    elementsByTagName[i].style.width = windowWidth + 'px';
  }
}

function changeLiStyle() {
  var elementsByClassName = document.getElementsByClassName('sLi');
  for (var i = 0; i < elementsByClassName.length; i++) {
    elementsByClassName[i].style['white-space'] = 'nowrap';
    elementsByClassName[i].style['*zoom'] = '1';
    elementsByClassName[i].style['*display'] = 'inline';
  }
}

function updateParentId(childNavigation, matchJsonObj) {
  if (childNavigation instanceof Array) {
    for (var i = 0; i < childNavigation.length; i++) {
      childNavigation[i].parentId = matchJsonObj.parentId;
    }
  } else {
    childNavigation.parentId = matchJsonObj.parentId;
  }
}

function changeTitleStyle() {
  document.getElementById('tree').style.fontSize = '12px';
  var keyWordEl = document.getElementById('keyWord');
  keyWordEl.style.fontSize = '12px';
  keyWordEl.style.fontFamily = '宋体';
  changeClassElementStyle('li-a', 'font-size', '12px');
  changeClassElementStyle('span-search', 'font-size', '12px');
  changeClassElementStyle('span-search-result', 'font-size', '12px');
}

// Clear the mergeProject attribute in the subproject navigation tree and change the relative path of each
function clearChildNaviData(obj, ref, name) {
  function addRef(child, ref, name) {
    var local = child.local;
    if (ref && local && local.trim()) {
      child.local = ref + '/' + local;
      child.id = name + '_' + child.id;
      child.parentId = name + '_' + child.parentId;
    }
  }

  if (obj.mergeProject) {
    obj.mergeProject = null;
  } else {
    if (obj.children) {
      for (var i = 0; i < obj.children.length; i++) {
        clearChildNaviData(obj.children[i], ref, name);
        addRef(obj.children[i], ref, name);
      }
    } else if (obj.length) {
      for (var i = 0; i < obj.length; i++) {
        clearChildNaviData(obj[i], ref, name);
        addRef(obj[i], ref, name);
      }
    }
  }
}

// Locate the subproject in the parent project and replace it with a character string.
function findAnchor(obj, name) {
  if (obj.mergeProject === name) {
    return obj;
  }
  var result;
  if (obj.children) {
    for (var i = 0; i < obj.children.length; i++) {
      result = findAnchor(obj.children[i], name);
      if (result) {
        return result;
      }
    }
  } else if (obj.length) {
    for (var i = 0; i < obj.length; i++) {
      result = findAnchor(obj[i], name);
      if (result) {
        return result;
      }
    }
  }
}

function assembleNavi(childNavigation, jsInfo) {
  var name = jsInfo.name;
  var ref = jsInfo.ref;

  var matchJsonObj = findAnchor(mainNavi, name);

  clearChildNaviData(childNavigation, ref, name);
  updateParentId(childNavigation, matchJsonObj);

  var childJsonStr = JSON.stringify(childNavigation);
  // If the value is jsonArray, delete the brackets ([]) on both sides.
  if (childNavigation instanceof Array) {
    childJsonStr = childJsonStr.substring(1, childJsonStr.length - 1);
  }
  var matchJsonStr = JSON.stringify(matchJsonObj);
  var mainNaviStr = JSON.stringify(mainNavi);
  var mainNaviNew = mainNaviStr.replace(matchJsonStr, childJsonStr);
  mainNavi = JSON.parse(mainNaviNew);
}
function getPromise(startPromise, jsInfo) {
  return startPromise.then(function() {
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
}
function loadMergedProjects() {
  naviData = null;
  var jsInfos = [];
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
    var jsInfo = jsInfos[i];
    startPromise = getPromise(startPromise, jsInfo);
  }
  startPromise.then(function() {
    triggerEvent(naviReady);
  });
}

function replaceClassText(className, value) {
  var elementsByClassName = document.getElementsByClassName(className);
  for (var i = 0; i < elementsByClassName.length; i++) {
    elementsByClassName[i].innerText = value;
  }
}

function setSelection() {
  var select = document.getElementById('titleOrContext');
  var delOptions = select.options;
  var len = delOptions.length;
  for (var i = 0; i < len; i++) {
    select.removeChild(delOptions[0]);
  }
  var titleOption = document.createElement('option');
  titleOption.value = '1';
  titleOption.appendChild(document.createTextNode('Title'));
  select.appendChild(titleOption);
  var contentOption = document.createElement('option');
  contentOption.value = '2';
  contentOption.selected = true;
  contentOption.appendChild(document.createTextNode('Content'));
  select.appendChild(contentOption);
}

function setTitle() {
  if (language === 'en') {
    replaceClassText('search-title', 'Search(S)');
    replaceClassText('catalog-title', 'Catalog(C)');
    replaceClassText('span-search', 'Please enter the keywords to search(W):');
    replaceClassText('span-search-result', 'Search result:');
    document.getElementById('keyWord').placeholder =
      'Please enter the keywords';
    document.getElementById('searchButton').value = 'Search';
    setSelection();
    document.title = 'Online Help';
  }
}

function setFont() {
  if (language === 'en') {
    var style = document.createElement('style');
    style.setAttribute('type', 'text/css');
    style.innerHTML =
      '.tree * {padding:0; margin:0; font-family: Arial, "宋体", Helvetica, AppleGothic, sans-serif}';
    document
      .getElementsByTagName('head')
      .item(0)
      .appendChild(style);
  }
}

function initPage() {
  // Adapting to the browser style
  if (browser !== 'Chrome') {
    changeTitleStyle();
  } else {
    document.getElementById('searchResult').style.width = 'max-content';
  }

  if (browser === 'ie7' || browser === 'ie8') {
    changeClassElementStyle('html', 'overflow', 'hidden');
    changeClassElementStyle('nav_div', 'border-top-width', '0px');
  }
  if (browser === 'ie7') {
    changeClassElementStyle('tabs_ul', 'height', '30px');
  }
}

// 首页名字忽略大小写
function ignoreCase(mainPage) {
  var nodes = myTree.getNodes();
  var findKey = 'local';
  if (!nodes || !findKey) return null;
  for (var i = 0, l = nodes.length; i < l; i++) {
    var node = nodes[i];
    if (
      node[findKey] &&
      node[findKey].toLowerCase() === mainPage.toLowerCase()
    ) {
      // 返回同名节点
      return nodes[i];
    }
  }
  return null;
}

function calcFirstNode() {
  var node;
  if (mainPage) {
    node = myTree.getNodeByParam('local', mainPage);
    if (null === node) {
      node = ignoreCase(mainPage);
    }
  } else {
    var nodes = myTree.getNodes();
    if (nodes.length > 0) {
      node = nodes[0]; // The root node is selected by default.
      while (node.local.trim() === '') {
        node = node.children[0];
      }
    }
  }
  return node;
}

function openNodeById(treeId, isHighlight) {
  var node = '';
  if (treeId === 'first') {
    node = calcFirstNode(node);
  } else {
    node = myTree.getNodeByTId(treeId);
  }
  myTree.selectNode(node);
  showTopic(node, isHighlight);
}

function openNodeByTopicUrl(url) {
  var node = myTree.getNodeByParam('local', url);
  myTree.selectNode(node);
  showTopic(node);
}

function initTree() {
  myTree = new Tree(treeSetting, mainNavi);

  initPage();
  var url = document.location.toString();
  var index = url.indexOf('#');

  if (index >= 0) {
    var paramWithTimestamp = url.split('#');
    var param = paramWithTimestamp[1].split('?')[0];
    openNodeByTopicUrl(param);
  } else {
    openNodeById('first');
  }
}

function addListener() {
  window.addEventListener(indexReady, searchKeyword, false);
  window.addEventListener(naviReady, initTree, false);
}

function initParentPath() {
  var url = document.location.toString();

  if (url.indexOf('?') !== -1) {
    url = url.substring(0, url.indexOf('?'));
  }

  if (url.indexOf('#') < 0) {
    urlParentPath = url.substring(0, url.lastIndexOf('/'));
  } else {
    var urlNoParam = url.split('#')[0];
    urlParentPath = urlNoParam.substring(0, urlNoParam.lastIndexOf('/'));
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
  } else if (explorer.indexOf('Safari') >= 0) {
    browser = 'Safari';
  } else if (explorer.indexOf('Firefox') >= 0) {
    browser = 'Firefox';
  } else if (explorer.indexOf('Chrome') >= 0) {
    browser = 'Chrome';
  } else if (explorer.indexOf('Opera') >= 0) {
    browser = 'Opera';
  } else if (explorer.indexOf('Netscape') >= 0) {
    browser = 'Netscape';
  } else {
    browser = 'ie';
  }
}

function clickLiFunc(e, isHighlight) {
  var treeId = e.getAttribute('value');

  // Remove styles from other sibling elements
  removeSiblingsSelected(e);
  addClass(e, 'selected');
  openNodeById(treeId, isHighlight);
}

function heightAdjustment() {
  AdjustWindow();
  if (src) {
    getTopic(src);
  }
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

window.onload = function() {
  // 获取元素
  var liTag = document.getElementsByClassName('ulLi');
  var subTabs = document.getElementsByClassName('subTab');

  for (var i = 0; i < subTabs.length; i++) {
    // 存储i的值  相当于oul的第一个子元素等于con的第一个子元素  (通俗来说就是一一对应)
    liTag[i].index = i;
    // 循环设置点击事件
    liTag[i].onclick = function() {
      for (var i = 0; i < subTabs.length; i++) {
        subTabs[i].style.display = 'none';
        removeClass(liTag[i], 'activeLi');
      }
      // this指的是事件前的对象  (ali[i].index=i)
      subTabs[this.index].style.display = 'block';
      addClass(liTag[this.index], 'activeLi');
    };
  }
  init();
  AdjustWindow();
};
window.onresize = heightAdjustment;

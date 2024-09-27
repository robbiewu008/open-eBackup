// 树组件
function Tree(setting, dataSource) {
  this.setting = setting;
  this.dataSource = dataSource;
  this.ItemIdChildrenMap = new Map();
  this.ItemIdItemMap = new Map();
  this.SetItemChildrenMap(dataSource);
  this.Render();
}

function escapeHtml(str) {
  if (typeof str == 'undefined') {
    return '';
  } else if (!str || typeof str != 'string') {
    return str;
  }
  var result = str;
  result = result.replace(/&/g, '&amp;');
  result = result.replace(/\"/g, '&quot;');
  result = result.replace(/'/g, '&apos;');
  result = result.replace(/</g, '&lt;');
  result = result.replace(/>/g, '&gt;');
  result = result.replace(/\\t/g, '&nbsp;&nbsp;');
  result = result.replace(/\\r\\n/g, '<br/>');
  return result;
}

Tree.prototype = {
  // 渲染树组件
  Render: function() {
    var dataSource = this.dataSource;
    var container = this.setting.container;
    container.classList.add('tree');
    this.BuildRootItem(dataSource, container);
  },

  BuildRootItem: function(data, container) {
    if (Array.isArray(data) && data.length) {
      var ul = document.createElement('ul');
      data.forEach(
        function(item) {
          if (!item.isHidden) {
            var liStr =
              '<li class="level_0">' +
              '<span class="button button-close" treenode_switch data-id="' +
              escapeHtml(item.id) +
              '"></span>' +
              '<a class="level_0" title="' +
              escapeHtml(item.name) +
              '" data-id="' +
              escapeHtml(item.id) +
              '">' +
              '<span class="icon"></span>' +
              '<span class="node_name">' +
              escapeHtml(item.name) +
              '</span>' +
              '</a>' +
              '</li>';
            var itemUi = this.CreateElementByStr(liStr);
            ul.appendChild(itemUi);
            var nodeItem = this.getNodeByTId(item.id);
            nodeItem.itemUi = itemUi;
            nodeItem.level = 0;
            // 绑定点击事件
            this.AttachNodeClickEvent(nodeItem);

            this.AttachNodeDblClickEvent(nodeItem);

            this.AttachNodeExpendEvent(nodeItem);

            this.UpdateIcon(nodeItem);
          }
        }.bind(this)
      );
      container.appendChild(ul);
    }
  },

  AttachNodeExpendEvent: function(node) {
    var switchNode = node.itemUi.querySelector('span[treenode_switch]');
    if (switchNode) {
      switchNode.removeEventListener('click', this.ExpandClickHandler);
      switchNode.addEventListener(
        'click',
        this.ExpandClickHandler.bind(this, switchNode),
        false
      );
    }
  },

  ExpandClickHandler: function(expandBtn) {
    var itemId = expandBtn.getAttribute('data-id');
    var nodeItem = this.getNodeByTId(itemId);
    if (!nodeItem.isExpend) {
      this.ExpandItem(nodeItem);
    } else {
      this.CloseItem(nodeItem);
    }
    this.UpdateIcon(nodeItem);
  },

  AttachNodeDblClickEvent: function(item) {
    var aTag = item.itemUi.getElementsByTagName('a')[0];
    if (aTag) {
      aTag.removeEventListener('dblclick', this.ExpandClickHandler);
      aTag.addEventListener(
        'dblclick',
        this.ExpandClickHandler.bind(this, aTag),
        false
      );
    }
  },

  AttachNodeClickEvent: function(item) {
    var aTag = item.itemUi.getElementsByTagName('a')[0];
    if (aTag) {
      aTag.removeEventListener('click', this.NodeClickHandler);
      aTag.addEventListener(
        'click',
        this.NodeClickHandler.bind(this, aTag),
        false
      );
    }
  },

  NodeClickHandler: function(aTag) {
    var itemId = aTag.getAttribute('data-id');
    var nodeItem = this.getNodeByTId(itemId);
    this.selectNode(nodeItem);
    if (
      this.setting.callback &&
      this.setting.callback.beforeClick &&
      Object.prototype.toString.call(this.setting.callback.beforeClick) ===
        '[object Function]'
    ) {
      this.setting.callback.beforeClick.call(this, nodeItem);
    }
  },

  // 更新节点图标状态
  UpdateIcon: function(node) {
    var iconNode = node.itemUi.getElementsByClassName('icon')[0];
    var buttonNode = node.itemUi.getElementsByClassName('button')[0];
    if (!node.children) {
      iconNode.classList.add('icon-nochild');
      buttonNode.classList.add('button-nochild');
    } else {
      iconNode.classList.remove('icon-nochild');
      if (node.isExpend) {
        iconNode.classList.remove('icon-close');
        iconNode.classList.add('icon-open');

        buttonNode.classList.remove('button-close');
        buttonNode.classList.add('button-open');
      } else {
        iconNode.classList.remove('icon-open');
        iconNode.classList.add('icon-close');

        buttonNode.classList.remove('button-open');
        buttonNode.classList.add('button-close');
      }
    }
  },

  // 构建id-children Map

  SetItemChildrenMap: function(dataSource) {
    var self = this;
    dataSource.forEach(function(item) {
      self.ItemIdItemMap.set(item.id, item);
      if (item.children) {
        self.ItemIdChildrenMap.set(item.id, item.children);
        self.SetItemChildrenMap(item.children);
      }
    });
  },

  // 展开节点
  ExpandItem: function(item) {
    if (item.isExpend || !item.children || !item.children.length) {
      return;
    }
    var childrenUi = item.itemUi.childNodes[2];
    if (childrenUi) {
      childrenUi.style.display = 'block';
      item.isExpend = true;
    } else {
      var itemChildList = item.children;
      var childLevel = item.level + 1;
      if (Array.isArray(itemChildList)) {
        var ul = document.createElement('ul');
        itemChildList.forEach(
          function(node) {
            if (!node.isHidden) {
              var liStr =
                '<li class="level_' +
                childLevel +
                '">' +
                '<span class="button" treenode_switch data-id="' +
                escapeHtml(node.id) +
                '"></span>' +
                '<a class="level_' +
                childLevel +
                '" title="' +
                escapeHtml(node.name) +
                '" data-id="' +
                escapeHtml(node.id) +
                '">' +
                '<span class="icon"></span>' +
                '<span class="node_name">' +
                escapeHtml(node.name) +
                '</span>' +
                '</a>' +
                '</li>';
              var itemUi = this.CreateElementByStr(liStr);
              ul.appendChild(itemUi);
              var nodeItem = this.getNodeByTId(node.id);
              nodeItem.itemUi = itemUi;
              nodeItem.level = childLevel;
              // 绑定点击事件
              this.AttachNodeClickEvent(nodeItem);

              this.AttachNodeDblClickEvent(nodeItem);

              this.AttachNodeExpendEvent(nodeItem);

              this.UpdateIcon(nodeItem);
            }
          }.bind(this)
        );
        item.itemUi.appendChild(ul);
        item.isExpend = true;
      }
    }
  },

  // 折叠节点
  CloseItem: function(item) {
    if (item.isExpend) {
      var childrenUi = item.itemUi.childNodes[2];
      if (childrenUi) {
        childrenUi.style.display = 'none';
        item.isExpend = false;
      }
    }
  },

  // 选中一个节点
  selectNode: function(node) {
    this.CancelSelectNodes();
    var nodeList = this.FindParentExpandList(node);
    for (var i = 0; i < nodeList.length; i++) {
      this.ExpandItem(nodeList[i]);
    }
    // dom加载过了
    var aTag = node.itemUi.getElementsByTagName('a')[0];
    if (aTag) {
      aTag.classList.add('a_selected');
    }
    this.ScrollToView(node);
  },

  /**
   * 向上寻找父节点至根节点
   * @param node
   * @returns {[]}
   * @constructor
   */
  FindParentExpandList: function(node) {
    var list = [];
    while (node.level !== 0) {
      var parentNode = this.getNodeByTId(node.parentId);
      list.unshift(parentNode);
      node = parentNode;
    }
    return list;
  },

  // 取消所有的选中
  CancelSelectNodes: function() {
    var nodeList = this.getNodes();
    for (var i = 0; i < nodeList.length; i++) {
      var curUi = nodeList[i].itemUi;
      if (curUi) {
        curUi.getElementsByTagName('a')[0].classList.remove('a_selected');
      }
    }
  },

  ScrollToView: function(node) {
    if (!node.itemUi) {
      return;
    }
    var nodeRect = node.itemUi.getBoundingClientRect();
    var containerRect = this.setting.container.getBoundingClientRect();

    var topGap = nodeRect.top - containerRect.top;
    var bottomGap = nodeRect.bottom - containerRect.bottom;

    if (topGap < 0) {
      this.setting.container.scrollTop += topGap - 34;
    } else if (bottomGap > 0) {
      this.setting.container.scrollTop += bottomGap + 34;
    }
  },

  getNodeByTId: function(id) {
    return this.ItemIdItemMap.get(id);
  },

  // 取到所有节点，包括没挂载dom的
  getNodes: function() {
    var nodeList = [];
    for (var key in this.ItemIdItemMap.data) {
      nodeList.push(this.ItemIdItemMap.data[key]);
    }
    return nodeList;
  },

  GetChildrenById: function(id) {
    return this.ItemIdChildrenMap.get(id);
  },

  /**
   * 1、如无结果，返回 null
   * 2、如有多个节点满足查询条件，只返回第一个匹配到的节点
   * @param attr
   * @param value
   */
  getNodeByParam: function(attr, value) {
    var nodeList = this.getNodes();
    for (var i = 0; i < nodeList.length; i++) {
      if (nodeList[i][attr] === value) {
        return nodeList[i];
      }
    }
    return null;
  },

  /**
   * 根据节点数据的属性搜索，获取条件模糊匹配的节点数据 JSON 对象集合
   * @param attr
   * @param value
   */
  getNodesByParamFuzzy: function(attr, value) {
    var nodeList = this.getNodes();
    return nodeList.filter(function(node) {
      return node[attr].toLowerCase().indexOf(value.toLowerCase()) > -1;
    });
  },

  CreateElementByStr: function(str) {
    var tempDiv = document.createElement('div');
    tempDiv.innerHTML = str;
    var element = tempDiv.firstElementChild.cloneNode(true);
    return element;
  }
};

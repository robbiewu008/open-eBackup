/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
import {
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  Input,
  OnDestroy,
  OnInit,
  SimpleChanges,
  ViewChild,
  ViewEncapsulation
} from '@angular/core';
import {
  DatatableService,
  DateService,
  FilterConfig,
  I18NService,
  LvConfig,
  SortDirective,
  TableFilterConfig,
  TagItem,
  TypeUtils
} from '@iux/live';
import { CommonConsts, GlobalService } from 'app/shared';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import {
  assign,
  assign as _assign,
  cloneDeep as _cloneDeep,
  each,
  filter as _filter,
  includes,
  isArray,
  isEmpty,
  isFunction,
  isString,
  map,
  merge as _merge,
  size,
  toString,
  trim
} from 'lodash';
import { Observable } from 'rxjs';
import { Filters, TableCols, TableConfig, TableData } from './interface';
import { DEFAULT_CONFIG } from './pro-table.config';

@Component({
  selector: 'lv-pro-table',
  templateUrl: './pro-table.component.html',
  styleUrls: ['./pro-table.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush,
  encapsulation: ViewEncapsulation.None,
  providers: [DatatableService],
  host: {
    '[class.lv-pro-table]': 'true',
    '[class.lv-pro-table-noAnim]': '!initConfig.table.showAnimation'
  }
})
export class ProTableComponent implements OnInit, OnDestroy {
  @Input() config: TableConfig;

  @Input() data: TableData;

  @ViewChild('lvTable', { static: false }) table;
  @ViewChild('lvPage', { static: false }) page;
  @ViewChild(SortDirective, { static: false }) tableSort: SortDirective;
  @ViewChild('filterSearch', { static: false }) filterSearch;
  @ViewChild('filterDate', { static: false }) filterDate;

  constructor(
    private cdr: ChangeDetectorRef,
    private datatableService: DatatableService,
    private dateService: DateService,
    private i18n: I18NService,
    private globalService: GlobalService
  ) {}

  typeUtils = TypeUtils;
  pollingTimer;
  initConfig: TableConfig;
  // 数据过滤映射
  filterMap: Filters;
  filterSelectMap: { [key: string]: any } = {};
  casheFilterSelectMap: { [key: string]: any } = {};
  filterTags: TagItem[];
  activeSort: { [key: string]: any };
  tableData = {
    data: [],
    total: 0
  };
  tableCols: TableCols[];
  cacheTableCols: TableCols[];
  displayControlCols: TableCols[];
  displayCols: string[];
  selection: any[];
  tableLoading = false;
  initing = false;
  tableScroll;
  activeItem;

  private _pollingParams; // 轮询的附加参数缓存
  private _isAutoPolling = false;

  _isAllCheck() {
    if (!this.table) {
      return false;
    }

    let renderData = _filter(this.table.renderData, item => !item.disabled);
    if (
      this.initConfig.table.rows &&
      this.initConfig.table.rows.disabledSelectionLogic === 'include'
    ) {
      renderData = this.table.renderData;
    }
    const all = renderData.length,
      selected = this.table.getRenderSelection().filter(item => !item.disabled)
        .length;
    return selected > 0 && all === selected;
  }
  _isHalfCheck() {
    if (!this.table) {
      return false;
    }
    return !this._isAllCheck() && !!this.table.getRenderSelection().length;
  }

  _getCols() {
    let cols: number = this.displayCols.length || 0;
    if (this.initConfig.table.colDisplayControl) {
      cols = cols + 1;
    }
    if (
      this.initConfig.table.rows &&
      this.initConfig.table.rows.selectionMode === 'multiple' &&
      this.initConfig.table.rows.showSelector
    ) {
      cols = cols + 1;
    }
    return cols + 1;
  }

  _isHidePagination() {
    const page = this.initConfig.pagination;

    if (page) {
      return this.tableData.data.length === 0;
    } else {
      return true;
    }
  }

  ngOnInit() {
    this.init();
    this.getDetailModalClose();
  }

  ngOnDestroy() {
    this.stopPolling();
  }

  ngOnChanges(changes: SimpleChanges): void {
    if (changes.data) {
      // 取消loading
      this.tableLoading = false;
      if (changes.data.currentValue) {
        this.tableData = Object.assign(this.tableData, this.data);

        if (this.initConfig && this.initConfig.table) {
          // 表格高度自适应
          if (
            this.initConfig.table.scroll &&
            !this.initConfig.table.scrollFixed
          ) {
            const scrollHeight =
              (this.initConfig.table.scroll &&
                this.initConfig.table.scroll.y) ||
              '0px';
            const infactHeight =
              (this.initConfig.table.virtualItemHeight &&
                this.tableData.data.length *
                  this.initConfig.table.virtualItemHeight) ||
              0;
            const _y =
              infactHeight < parseInt(scrollHeight)
                ? infactHeight + 'px'
                : scrollHeight;
            this.tableScroll = {
              x:
                (this.initConfig.table.scroll &&
                  this.initConfig.table.scroll.x &&
                  this.initConfig.table.scroll.x + 'px') ||
                null,
              y: _y
            };
          }

          // 手动更新数据时列表定位到第一条数据
          if (!this._isAutoPolling) {
            this._scrollToTop();
          }

          // 自动轮询
          if (this.initConfig.table.autoPolling) {
            this.startPolling(this.initConfig.table.autoPolling);
          }
        }
      }
    }
  }

  init(config: TableConfig = this.config) {
    this.initConfig = _merge({}, DEFAULT_CONFIG, config);
    this.tableScroll = this.initConfig.table.scroll;
    this.filterMap = {
      filters: [],
      paginator: {
        pageIndex:
          (this.initConfig &&
            this.initConfig.pagination &&
            this.initConfig.pagination.pageIndex) ||
          0,
        pageSize:
          (this.initConfig &&
            this.initConfig.pagination &&
            this.initConfig.pagination.pageSize) ||
          LvConfig.paginatorOptions.lvPageSize ||
          5
      },
      sort: { key: '', direction: '' }
    };
    this.tableCols = this.initConfig.table.columns;
    each(this.tableCols, (col: any) => {
      // 适配操作列宽度
      if (col.key === 'operation' && !col.useOpWidth) {
        col.width = CommonConsts.TABLE_OPERATION_WIDTH;
      }
      // 适配详情点击事件，激活当前行
      if (col.cellRender?.config?.click) {
        const click = col.cellRender.config.click;
        col.cellRender.config.click = item => {
          this.activeItem = item;
          click.call(this, item);
        };
      }
    });
    this.cacheTableCols = this._getCacheTableCols(
      this.initConfig.table.columns
    );
    this.displayControlCols = this._getDisplayControlCols();
    this.displayCols = this._getDisplayCols();
    this._setFilterSelectMap();
    this._setActiveSort();
  }

  /**
   * 过滤条件变更
   * @param source
   */
  _stateChange(source: any) {
    // 设置过滤条件
    if (source.filterState) {
      this._updateFilterMap({
        filters: source.filterState
      });
    }
    if (source.paginator) {
      this._updateFilterMap({
        paginator: source.paginator
      });
    }
    if (source.sort) {
      this._updateFilterMap({
        sort: source.sort
      });
    }

    // filter 变更时触发异步数据更新
    if (this.initConfig.table.async) {
      this.stopPolling();
      this.fetchData();
    }

    // 更新过滤器popover的位置
    const delay = setTimeout(() => {
      this.filterSearch && this.filterSearch.popover.updatePosition();
      this.filterDate && this.filterDate.popover.updatePosition();
      clearTimeout(delay);
    }, 0);

    // TODO: afterStateChange事件
  }

  /**
   * 设置分页点击标识
   */
  _pageChange() {
    this.filterMap.pageChangeFlag = true;
  }

  /**
   * 行选择变更事件
   */
  _selectionChange(data) {
    // 当前页选中项
    const renderSelection = this.table.getRenderSelection();
    // 所有选中项
    const selection = this.table.getSelection();

    const doChange = this.initConfig.table.selectionChange;
    doChange && doChange(selection, renderSelection);
  }

  /**
   * 设置行checkbox选中状态
   */
  _isRowSeleted(source) {
    return this.table.isSelected(source);
  }

  /**
   * 行点击选择
   */
  _toggleSelect(source) {
    if (
      source.disabled ||
      (this.initConfig.table.rows &&
        (this.initConfig.table.rows.selectionMode === null ||
          this.initConfig.table.rows.selectionTrigger === 'selector'))
    ) {
      return;
    }
    if (
      this.initConfig.table.rows &&
      this.initConfig.table.rows.keepRadioLogic &&
      this.initConfig.table.rows.selectionMode === 'single' &&
      this.getSelections()[0] === source
    ) {
      return;
    } else {
      this.table.toggleSelection(source);
    }
  }

  /**
   * radio点击选择
   */
  _radioSelect(source) {
    if (source.disabled) {
      return;
    }

    if (
      this.initConfig.table.rows &&
      this.initConfig.table.rows.keepRadioLogic &&
      this.getSelections()[0] === source
    ) {
      return;
    } else {
      this.table.toggleSelection(source);
    }
  }

  /**
   * checkbox点击选择
   */
  _toggleCheckboxSelect(source) {
    if (source.disabled) {
      return;
    }
    this.table.toggleSelection(source);
  }

  /**
   * 全选方法
   */
  _toggleAllSelection() {
    const renderData = _filter(this.table.renderData, item => !item.disabled);
    if (
      this.table.getRenderSelection().filter(item => !item.disabled).length ===
      renderData.length
    ) {
      this.table.deleteSelection(renderData);
    } else {
      this.table.bulkSelection(renderData);
    }
  }

  /**
   * 缓存表格列信息
   */
  _getCacheTableCols(cols) {
    const _cacheCols: any[] = [];
    cols.map(item => {
      const obj: any = {
        key: item.key,
        name: this.typeUtils.isRealString(item.name) ? item.name : '',
        auxiliary: item.auxiliary,
        width: item.width,
        hidden: item.hidden
      };
      _cacheCols.push(obj);
    });
    return _cacheCols;
  }

  /**
   * 获取可控制显示/隐藏的列
   */
  _getDisplayControlCols() {
    if (this.initConfig.table && this.initConfig.table.colDisplayControl) {
      return this.cacheTableCols;
    } else {
      return [];
    }
  }

  /**
   * 设置单元格对齐方式
   */
  _setTdAlign(col) {
    if (
      col.cellRender &&
      (col.cellRender.type === 'capacity' ||
        col.cellRender.type === 'number' ||
        col.cellRender.type === 'percent' ||
        (col.cellRender.type === 'text' && col.thAlign === 'right') ||
        (!col.cellRender.type && col.thAlign === 'right'))
    ) {
      return 'right';
    } else {
      return 'left';
    }
  }

  /**
   * 获取当前显示的列
   */
  _getDisplayCols() {
    const arr: string[] = [];
    this.displayControlCols.map(item => {
      if (item.hidden === 'ignoring' || !item.hidden) {
        arr.push(item.key);
      }
    });
    return arr;
  }

  /**
   * 设置类型为select的过滤器的数据
   */
  _setFilterSelectMap() {
    this.tableCols.map(item => {
      if (item.filter && item.filter.type === 'select') {
        if (item.key) {
          if (item.filter.options instanceof Array) {
            this.filterSelectMap[item.key] = item.filter.options;
            this.casheFilterSelectMap[item.key] = item.filter.options;
          } else {
            const mapData = item.filter.options();
            if (mapData instanceof Promise) {
              mapData
                .then(res => {
                  this.filterSelectMap[item.key] = res;
                  this.casheFilterSelectMap[item.key] = res;
                })
                .catch(error => {
                  console.error('Failed to request data.');
                });
            } else if (mapData instanceof Observable) {
              mapData.subscribe(
                res => {
                  this.filterSelectMap[item.key] = res;
                  this.casheFilterSelectMap[item.key] = res;
                },
                error => {
                  console.error('Failed to request data.');
                }
              );
            }
          }
        }
      }
    });
  }

  /**
   * 设置当前排序
   */
  _setActiveSort() {
    if (this.initConfig.table.activeSort) {
      this.activeSort = this.initConfig.table.activeSort;
    } else {
      this.activeSort = {};
    }
  }

  /**
   * 设置过滤标签
   * @param tags
   */
  _setFilterTags(tags: TableFilterConfig[]) {
    const _filterTags =
      tags &&
      Array.from(tags).map(item => {
        const arr = {
          label: this._formatTagItem(item),
          removeable: true
        };
        return _merge(arr, item);
      });
    this.filterTags = _filterTags.filter(item => {
      if (isArray(item.value)) {
        item.value = item.value.filter(
          v => v !== '' && v !== null && v !== undefined
        );
        return item.value.length > 0;
      }
      return true;
    });
  }

  _formatTagItem(item) {
    const col = this.tableCols.filter(i => i.key === item.key)[0];
    let labels: any[] = [];
    if (col.filter && 'options' in col.filter) {
      item.value.map(v => {
        this.casheFilterSelectMap[item.key].map(o => {
          if (v === o.value) {
            labels.push(o.label);
          }
        });
      });
    } else if (col.filter && 'dateRange' in col.filter) {
      if (col.filter.dateRange) {
        let offsetDate: Date[];
        if (col.filter.timezoneOffset) {
          offsetDate = [
            this.dateService.getTimezoneOffsetDate(
              item.value[0],
              col.filter.timezoneOffset,
              true
            ),
            this.dateService.getTimezoneOffsetDate(
              item.value[1],
              col.filter.timezoneOffset,
              true
            )
          ];
        } else {
          offsetDate = item.value;
        }
        const formatDate = [
          this.dateService.format(
            offsetDate[0],
            col.filter.format || 'yyyy-MM-dd'
          ),
          this.dateService.format(
            offsetDate[1],
            col.filter.format || 'yyyy-MM-dd'
          )
        ];
        labels.push(formatDate[0] + this.i18n.get('timeTo') + formatDate[1]);
      } else {
        const offsetDate = col.filter.timezoneOffset
          ? this.dateService.getTimezoneOffsetDate(
              item.value[0],
              col.filter.timezoneOffset,
              true
            )
          : item.value;
        labels.push(
          this.dateService.format(offsetDate, col.filter.format || 'yyyy-MM-dd')
        );
      }
    } else {
      labels = item.value;
    }
    return labels;
  }

  /**
   * 获取Tag名称
   */
  _getTagName(key) {
    const col = this.tableCols.filter(item => item.key === key);
    return (
      col.length > 0 &&
      (this.typeUtils.isRealString(col[0].name)
        ? col[0].name
        : col[0].auxiliary)
    );
  }

  /**
   * 更新过滤条件
   * @param filters
   */
  _updateFilterMap(filters: Filters) {
    this.filterMap = _assign(this.filterMap, filters);
    this._setFilterTags(this.filterMap.filters as TableFilterConfig[]);
  }

  /**
   * 清除过滤标签
   * @param event 无参数时清除所有
   */
  _clearFilterTag(event?) {
    if (event) {
      this.table.removeFilter(event.key);
      this.page.jumpToFisrtPage();
      this.filterMap.filters = this.table.getFilter();
      // 清理对应过滤器的视图
      const _map = _cloneDeep(this.filterSelectMap);
      if (_map[event.key]) {
        _map[event.key].map(item => {
          const isSelected = event.value.some(v => v === item.value);
          if (isSelected) {
            item.selected = !isSelected;
          }
        });
        this.filterSelectMap[event.key] = _cloneDeep(_map[event.key]);
      }
    } else {
      this.table.removeFilter();
      this.page.jumpToFisrtPage();
      this.filterMap.filters = this.table.getFilter();
      this.filterSelectMap = _cloneDeep(this.casheFilterSelectMap);
    }
  }

  /**
   * 清除Search过滤调遣
   * @param name 过滤器名称
   */
  _clearSearchFilter(col) {
    if (isFunction(this.initConfig.table.filterChangeBefore)) {
      this.initConfig.table.filterChangeBefore();
    }
    this.filterMap.filterChangeFlag = true;
    this.table.removeFilter(col.key);
    this.page.jumpToFisrtPage();
    this.filterMap.filters = this.table.getFilter();
  }

  /**
   * 获取 type='search/date' 类型过滤器的当前值
   */
  _getfilterValue(col) {
    const _filter =
      this.filterMap.filters &&
      this.filterMap.filters.filter(item => {
        return item.key === col.key;
      });
    const _value =
      (_filter && _filter.length > 0 && _filter[0].value) || undefined;
    return _value;
  }

  /**
   * type='search/date' 类型过滤器事件
   */
  _doSearch(e, col) {
    const _option: TableFilterConfig = {
      key: col.key,
      value: e,
      customFilter: col.filter.customFilter,
      customItemFilter: col.filter.customItemFilter,
      filterMode: col.filter.filterMode || 'startsWith',
      caseSensitive: col.filter.caseSensitive || false
    };

    // 日期过滤特殊处理
    if (col.filter.type === 'date') {
      _option.value = e.filter(item => item !== null);
      if (!col.filter.customFilter && _option.value.length > 0) {
        // 日期范围
        if (
          col.filter.dateRange === true ||
          col.filter.dateRange === undefined
        ) {
          _option.customFilter = (data, filter): any => {
            const showTime = col.filter.showTime ? true : false;
            const startTime = this._getTime(filter.value[0], showTime);
            const endTime = this._getTime(filter.value[1], showTime, false);
            return _filter(
              data,
              item =>
                filter.key &&
                item[filter.key].getTime() >= startTime &&
                item[filter.key].getTime() <= endTime
            );
          };
        }
        // 单个日期
        else {
          _option.customFilter = (data, filter): any => {
            const showTime = col.filter.showTime ? true : false;
            const timeNum = this._getTime(filter.value[0], showTime);
            return _filter(data, item => {
              return (
                filter.key &&
                this._getTime(item[filter.key], showTime) === timeNum
              );
            });
          };
        }
      }
    }
    this.filterChange(_option);
  }

  _getTime(date, showTime = true, dayStart = true) {
    let _date;
    if (showTime) {
      _date = date.setMilliseconds(0);
      if (!dayStart) {
        _date = date.setMilliseconds(999);
      }
    } else {
      _date = date.setHours(0, 0, 0, 0);
      if (!dayStart) {
        _date = date.setHours(23, 59, 59, 999);
      }
    }

    return _date;
  }

  /**
   * 行展开事件
   */
  _rowExpandChange(expand: boolean, rowData) {
    this.initConfig.table.rows &&
      this.initConfig.table.rows.expandChange &&
      this.initConfig.table.rows.expandChange(expand, rowData);
  }

  /**
   * 更新数据时定位到列表第一条数据
   */
  _scrollToTop() {
    if (this.initConfig.table.virtualScroll) {
      const el = this.table._scrollViewport;
      el && el.scrollToIndex(0);
    } else {
      const el = this.table.elementRef.nativeElement.querySelector(
        '.lv-table-body'
      );
      el && (el.scrollTop = 0);
    }
  }

  /**
   * 重新初始化
   */
  reinit(callback?, config?: TableConfig) {
    this.tableCols = [];
    this.cacheTableCols = [];
    this.displayControlCols = [];
    this.displayCols = [];
    this.initing = true;
    this.init(config);

    // 清空列表数据
    this.tableData = {
      data: [],
      total: 0
    };
    this.filterTags = [];

    // 立即执行变更检测，清除之前的脏数据
    this.cdr.detectChanges();

    // 下一周期渲染表格
    const delay = setTimeout(() => {
      this.initing = false;
      this.cdr.detectChanges();

      // 初始化后的回调
      callback && callback();
      clearTimeout(delay);
    }, 0);
  }

  /**
   * 开始轮询
   */
  startPolling(timer?: number, options?) {
    if (timer && timer !== this.initConfig.table.autoPolling) {
      this.initConfig.table.autoPolling = timer;
    }
    if (options) {
      this._pollingParams = options;
    }

    this.pollingTimer = setTimeout(() => {
      this._isAutoPolling = true;
      this.fetchData(
        { isAutoPolling: this._isAutoPolling },
        this._pollingParams
      );
      clearTimeout(this.pollingTimer);
    }, this.initConfig.table.autoPolling);
  }

  /**
   * 停止轮询
   */

  stopPolling() {
    this._isAutoPolling = false;
    clearTimeout(this.pollingTimer);
  }

  /**
   * 设置表格选中项
   */
  setSelections(selections: any[], markChange = true) {
    this.selection = selections;
    if (markChange) {
      this.cdr.detectChanges();
    }
  }

  /**
   * 获取表格当页选中项
   */
  getSelections() {
    return this.table && this.table.getRenderSelection();
  }

  /**
   * 获取表格全部选中项
   */
  getAllSelections() {
    return this.table && this.table.getSelection();
  }

  /**
   * 外部设置过滤条件
   * @param filters
   */
  setFilterMap(data?: Filters) {
    const filters = data ? data : this.getFilterMap();
    // 清除内部的filtermap, 再重新设置filtermap
    if (filters && filters.filters) {
      this.table.removeFilter();
      this.filterSelectMap = _cloneDeep(this.casheFilterSelectMap);
      filters.filters.map(item => {
        const _source = {
          ...{ filterMode: 'startsWith', caseSensitive: false },
          ...item
        } as FilterConfig;
        this.datatableService.setFilterState(_source);
        this.table.filter(_source);

        // 设置对应过滤器的视图
        if (item.key) {
          const _map = _cloneDeep(this.casheFilterSelectMap);
          if (_map[item.key]) {
            _map[item.key].map(m => {
              const isSelected = item.value.some(v => v === m.value);
              if (isSelected) {
                m.selected = isSelected;
              }
            });
            this.filterSelectMap[item.key] = _cloneDeep(_map[item.key]);
          }
        }
      });
    }

    // 重新设置paginator
    if (filters && filters.paginator) {
      this.datatableService.setPaginatorState(filters.paginator);
      this.page.jumpToPage(filters.paginator.pageIndex + 1);
    }

    // 重新设置排序
    if (filters && filters.sort) {
      const currentSort = Array.from(
        this.datatableService.getSortState().values()
      );
      if (currentSort.length === 0) {
        this.datatableService.clearSortState();
      }

      this.datatableService.setSortState(filters.sort);
      this.tableSort.sort(filters.sort);
    }

    // 获取内部的filter
    this.filterMap.filters = this.table.getFilter();
    this.filterMap.paginator = this.datatableService.getPaginatorState();
    this.filterMap.sort = Array.from(
      this.datatableService.getSortState().values()
    )[0];

    this.table._dataSource.states().next(this.filterMap);
  }

  /**
   * 过滤事件
   */
  filterChange(options: any) {
    if (isFunction(this.initConfig.table.filterChangeBefore)) {
      this.initConfig.table.filterChangeBefore();
    }
    this.filterMap.filterChangeFlag = true;
    this.table.filter(options);
    this.page.jumpToFisrtPage();
  }

  /**
   * 获取过滤映射
   */
  getFilterMap() {
    return this.filterMap;
  }

  /**
   * 设置列显示/隐藏
   * @displayCols 要显示的列，keys数组
   * @manual 是否内部手动触发
   */
  setColsDisplay(displayCols: string[], manual: boolean = false) {
    this.tableCols.map(col => {
      if (col.hidden !== 'ignoring') {
        col.hidden = !displayCols.includes(col.key);
      }
    });

    // 为当前显示的列设置原始宽度
    const _displayCols = this.tableCols.filter(col => col.hidden !== true);
    _displayCols.map(col => {
      const cacheCol = this.cacheTableCols.find(
        originCol => originCol.key === col.key
      );
      cacheCol && cacheCol.width
        ? (col.width = cacheCol.width)
        : (col.width = undefined);
    });
    // 当所有列都为固定宽度时，设置最后一列的宽度为自适应
    const hasFlexCol = _displayCols.some(
      col => col.width === undefined || col.width === 'auto'
    );
    if (!hasFlexCol) {
      _displayCols[_displayCols.length - 1].width = 'auto';
    }

    // 外部设置时更新下拉列表
    if (!manual) {
      this.displayCols = displayCols;
    } else {
      // 内部变更触发回调
      this.initConfig.table.colDisplayChange &&
        this.initConfig.table.colDisplayChange(displayCols);
    }
  }

  /**
   * 获取表格数据
   * @param filters 表格的过滤条件
   */
  fetchData(...args: any[]) {
    // 显示loading
    this.initConfig.table &&
      this.initConfig.table.showLoading &&
      (this.tableLoading = true);

    // 组装过滤、搜索等参数
    const conditions = {};
    const conditions_v2 = {};
    each(this.filterMap.filters, item => {
      if (!item.value) {
        return;
      }
      if (
        item.filterMode === 'contains' &&
        !isEmpty(trim(toString(item.value)))
      ) {
        conditions[item.key] = trim(toString(item.value));
        if (item.key === 'sla_name') {
          conditions_v2['protectedObject'] = {
            ...conditions_v2['protectedObject'],
            slaName: [['~~'], trim(toString(item.value))]
          };
        } else {
          conditions_v2[item.key] = [['~~'], trim(toString(item.value))];
        }
      } else if (
        (item.filterMode === 'in' && !!size(item.value)) ||
        (!isEmpty(item.value) && item.value)
      ) {
        conditions[item.key] = item.value;
        if (item.key === 'protection_status') {
          conditions_v2['protectionStatus'] = [['in'], ...item.value];
        } else if (item.key === 'sla_status') {
          conditions_v2['protectedObject'] = {
            ...conditions_v2['protectedObject'],
            status: [['in'], ...map(item.value, v => +v)]
          };
        } else if (item.key === 'sla_compliance') {
          conditions_v2['protectedObject'] = {
            ...conditions_v2['protectedObject'],
            sla_compliance: [['in'], ...item.value]
          };
        } else {
          conditions_v2[item.key] = [['in'], ...item.value];
        }
      }
    });

    if (!isEmpty(conditions)) {
      assign(this.filterMap, { conditions: JSON.stringify(conditions) });
    } else {
      delete this.filterMap['conditions'];
    }

    if (!isEmpty(conditions_v2)) {
      assign(this.filterMap, { conditions_v2: JSON.stringify(conditions_v2) });
    } else {
      delete this.filterMap['conditions_v2'];
    }

    const orders = [];
    if (this.filterMap.sort.key && this.filterMap.sort.direction) {
      orders.push(
        (this.filterMap.sort.direction === 'asc' ? '+' : '-') +
          this.filterMap.sort.key
      );
    }

    if (!!size(orders)) {
      assign(this.filterMap, { orders });
    } else {
      delete this.filterMap['orders'];
    }

    // 调用外部方法刷新数据
    this.initConfig.table.fetchData &&
      this.initConfig.table.fetchData(this.filterMap, ...args);
    this.cdr.detectChanges();
  }

  setTableScroll(scroll) {
    this.tableScroll = scroll;
    this.cdr.detectChanges();
  }

  isActive(item) {
    const compareKey = isString(this.initConfig.table.compareWith)
      ? this.initConfig.table.compareWith
      : 'uuid';
    return this.activeItem && item[compareKey] === this.activeItem[compareKey];
  }

  getDetailModalClose() {
    this.globalService
      .getState('detailModalClose')
      .subscribe(() => this.setActiveItemEmpty());
  }

  setActiveItemEmpty() {
    this.activeItem = {};
    this.cdr.detectChanges();
  }

  /**
   * 新手引导新增资源
   */
  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      (includes(USER_GUIDE_CACHE_DATA.resource, item?.rootUuid) ||
        includes(USER_GUIDE_CACHE_DATA.resource, item?.uuid))
    );
  }
}

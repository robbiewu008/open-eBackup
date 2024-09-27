import { TemplateRef, TrackByFunction } from '@angular/core';
import {
  BaseSortConfig,
  FilterItem,
  LvDateTimeOption,
  LvFilterMode,
  PageConfig,
  SortConfig,
  TableFilterConfig
} from '@iux/live';
import { Observable } from 'rxjs';
import { ProButton } from '../pro-button/interface';
import { StatusConfig } from '../pro-status';
import { TextConfig } from '../pro-text';

export interface TableConfig {
  /* 在列表上方显示当前过滤器标签 */
  filterTags?:
    | {
        /* 自定义标签模板 */
        tagTemplate?: TemplateRef<any>;
        /* 标签末尾扩展内容模板 */
        extraContent?: TemplateRef<any>;
      }
    | boolean;
  /* 列表分页配置 */
  pagination?: {
    /* 当前页码 */
    pageIndex?: number;
    /* 当前每页显示条数 */
    pageSize?: number;
    /* 是否显示每页条数下拉配置 */
    showPageSizeOptions?: boolean;
    /* 每页显示条数配置可选项 */
    pageSizeOptions?: number[];
    /* 分页器显示模式，复杂模式和简单模式 */
    mode?: 'default' | 'simple';
    /* 是否显示总条数 */
    showTotal?: boolean;
    // 是否固定分页条
    winTablePagination?: boolean;
    showPagination?: boolean;
  };
  table: {
    /* 是否异步数据 */
    async?: boolean;
    /* 列配置 */
    columns: TableCols[];
    /* 行配置 */
    rows?: TableRow;
    /* 数据对比配置，默认按照对象地址比较，如果异步数据必须配置该属性 */
    compareWith?: string | ((o: any) => any);
    /* 自动轮询，值为轮询周期，单位毫秒 */
    autoPolling?: number;
    /* 表格渲染动效 */
    showAnimation?: boolean;
    /* 显示表格局部loading */
    showLoading?: boolean;
    /* 傀儡表格 */
    fake?: boolean;
    /* 列表的行高规格 */
    size?: 'default' | 'large' | 'small';
    /* 列表的 trackBy 配置 */
    trackByFn?: TrackByFunction<any>;
    /* 是否虚拟滚动 */
    virtualScroll?: boolean;
    /* 虚拟滚动行高配置 */
    virtualItemHeight?: number;
    /* 定义表格的高和宽，超出自动滚动 */
    scroll?: { x?: string; y?: string };
    /* 当数据小于滚动高度时，是否随数据变化 */
    scrollFixed?: boolean;
    /* 初始化排序字段 */
    activeSort?: BaseSortConfig;
    /* 是否开启列宽度调整 */
    colResize?: { mode: 'fit' | 'expand' } | boolean;
    /* 是否开启列显示/隐藏 */
    colDisplayControl?:
      | {
          /* 被忽略的列在下拉列表中的处理方式 */
          ignoringColsType: 'hide' | 'disable';
        }
      | boolean;
    /* 获取列表数据 */
    fetchData?: (filter: Filters, ...args: any[]) => any;
    /* 列表选择回调 */
    selectionChange?: (selection: any[], renderSelection: any[]) => any;
    /* 列显示/隐藏变化时的回调函数 */
    colDisplayChange?: (displayCols: string[]) => any;
    /* 表头搜索下发前函数 */
    filterChangeBefore?: () => any;
  };
}

export interface TableCols {
  /* 列的名称，支撑自定义模板 */
  name: string | TemplateRef<any>;
  /* 列的属性 */
  key: string;
  /* 辅助名，当name为空时，用于列显示隐藏的标识 */
  auxiliary?: string;
  /* 列宽度 */
  width?: number | string;
  /* 列名称对齐方式 */
  thAlign?: 'left' | 'right' | 'center';
  // fixLeft?: string | boolean;
  // fixRight?: string | boolean;
  /* 列显示/隐藏初始化配置，'ignoring'为排除项 */
  hidden?: boolean | 'ignoring';
  /* 列过滤器配置 */
  filter?: FilterTypeSelect | FilterTypeSearch | FilterTypeDate;
  /* 列排序配置 */
  sort?:
    | {
        customSort?: <T>(data: T[] | null, sort: SortConfig) => T[] | null;
      }
    | boolean;
  /* 列扩展配置，显示在名称右侧 */
  thExtra?: string | TemplateRef<any>;
  /* 单元格渲染器，默认提供 8 种类型渲染器 */
  cellRender?: CellRenderType;
  /** 业务扩展参数，供业务逻辑处理 */
  extendParameter?: any;
  useOpWidth?: boolean;
}

export interface FilterTypeSelect {
  /* 过滤器类型 */
  type: 'select';
  /* 下拉选项，支持异步返回 */
  options:
    | FilterItem[]
    | (() => Promise<FilterItem[]>)
    | (() => Observable<FilterItem[]>);
  /* 是否为多选 */
  isMultiple?: boolean;
  /* 是否显示全选按钮，配置 isMultiple:true 时有效 */
  showCheckAll?: boolean;
  /* 是否显示搜索功能 */
  showSearch?: boolean;
  /* 选项自定义模板 */
  template?: TemplateRef<any>;
  /* 选中选项后立即执行过滤 */
  filterAuto?: boolean;
  /* 过滤模式，LvFilterMode = 'contains' | 'endsWith' | 'startsWith' | 'equals' | 'in' | 'timeIn' | 'rangeIn'; */
  filterMode?: LvFilterMode;
  /* 是否大小写敏感 */
  caseSensitive?: boolean;
  /* 根据全部数据自定义过滤 */
  customFilter?: <T>(data: T[] | null, filter: TableFilterConfig) => T[] | null;
  /* 根据单项数据自定义过滤 */
  customItemFilter?: <T>(data: T | null, filter: TableFilterConfig) => boolean;
}
export interface FilterTypeSearch {
  /* 过滤器类型 */
  type: 'search';
  /* 过滤模式，LvFilterMode = 'contains' | 'endsWith' | 'startsWith' | 'equals' | 'in' | 'timeIn' | 'rangeIn'; */
  filterMode?: LvFilterMode;
  /* 是否大小写敏感 */
  caseSensitive?: boolean;
  /* 根据全部数据自定义过滤 */
  customFilter?: <T>(data: T[] | null, filter: TableFilterConfig) => T[] | null;
  /* 根据单项数据自定义过滤 */
  customItemFilter?: <T>(data: T | null, filter: TableFilterConfig) => boolean;
}
export interface FilterTypeDate {
  /* 过滤器类型 */
  type: 'date';
  /* 是否为日期范围 */
  dateRange?: boolean;
  /* 日期显示格式化字符串 */
  format?: string;
  /* 时区偏移值设置，偏移值单位为分钟 */
  timezoneOffset?: number;
  /* 是否显示时间 */
  showTime?: boolean | LvDateTimeOption;
  /* 不可选的日期 */
  disabledDate?: (date: Date) => boolean;
  /* 是否只显示当前月日期单元格 */
  onlyShowActiveCell?: boolean;
  /* 是否显示【今天】快捷选项，配置 dateRange: false 时有效 */
  showTodayButton?: boolean;
  /* 是否显示【此刻】快捷选项，配置 dateRange: false 时有效 */
  showNowButton?: boolean;
  /* 预设日期范围快捷选项，配置 dateRange: true 时有效 */
  presetRanges?: { [key: string]: Date[] }[];
  /* 提示信息 */
  placeholder?: string | string[];

  /* 过滤模式，LvFilterMode = 'contains' | 'endsWith' | 'startsWith' | 'equals' | 'in' | 'timeIn' | 'rangeIn'; */
  filterMode?: LvFilterMode;
  /* 是否大小写敏感 */
  caseSensitive?: boolean;
  /* 根据全部数据自定义过滤 */
  customFilter?: <T>(data: T[] | null, filter: TableFilterConfig) => T[] | null;
  /* 根据单项数据自定义过滤 */
  customItemFilter?: <T>(data: T | null, filter: TableFilterConfig) => boolean;
}

export interface TableRow {
  /* 斑马纹背景 */
  showStripe?: boolean;
  /* 选择模式配置 */
  selectionMode?: 'single' | 'multiple';
  /* 选择触发器，整行或仅选择器 */
  selectionTrigger?: 'row' | 'selector';
  /* 全选逻辑是否包含禁用项 */
  disabledSelectionLogic?: 'exclude' | 'include';
  /* 禁用项提示，挂载在选择器上，支持模板配置 */
  disabledTooltip?: string | TemplateRef<any>;
  /* 是否显示选择器checkbox/radio */
  showSelector?: boolean;
  /* 单选模式下是否保留radio的原生逻辑，即一旦选中后不可取消 */
  keepRadioLogic?: boolean;
  /* 是否支持行展开，如需配置初始化行数据展开，需在行数据中增加属性 _lv_expand:true */
  expandable?: boolean;
  /* 展开图标配置，值为 svg symbol id */
  expandToggleIcon?: { true?: string; false?: string } | string;
  /* 行展开内容配置，支持模板配置，模板参数为行数据 */
  expandContent?: string | TemplateRef<any>;
  /* 行展开事件 */
  expandChange?: (expand: boolean, rowData) => any;
}

export interface Filters {
  /**
   * 过滤条件
   */
  filters?: TableFilterConfig[];
  /**
   * 分页
   */
  paginator?: PageConfig;
  /**
   * 排序
   */
  sort?: SortConfig;
  /**
   * 排序字段：uuid
   */
  orders?: Array<string>;

  /**
   * 条件参数：
   */
  conditions?: string;

  /**
   * 条件参数(适配v2接口)：
   */
  conditions_v2?: string;

  pageChangeFlag?: boolean;

  filterChangeFlag?: boolean;
}

export interface TableData {
  data: any[];
  total: number;
}

export type CellRenderType =
  /* 文本/链接渲染器，TextConfig 配置见 ProText 组件 */
  | {
      type: 'text';
      config?: any | TextConfig;
    }
  /* 操作列渲染器 */
  | {
      type: 'operation';
      config?:
        | any
        | {
            /* ProButton 配置见 ProButton 组件 */
            items: ProButton[];
            /* 操作按钮组的最大可见个数，其余项在下拉菜单展示 */
            maxDisplayItems?: number;
            /* 是否始终显示下拉菜单 */
            keepDropdown?: boolean;
            /* 下拉菜单的文本 */
            menuText?: string;
          };
    }
  /* 进度条渲染器 */
  | {
      type: 'progress';
      config?: any;
    }
  /* 容量渲染器，输入值为 Byte 单位的数字 */
  | {
      type: 'capacity';
      config?:
        | any
        | {
            /* 保留小数位数 */
            decimals?: number;
            /* 小数舍入计算方式 */
            method?: 'round' | 'floor' | 'ceil'; // 默认值'round'
          };
    }
  /* 数字渲染器 */
  | {
      type: 'number';
      config?:
        | any
        | {
            /* 保留小数位数 */
            decimals?: number;
            /* 小数舍入计算方式 */
            method?: 'round' | 'floor' | 'ceil'; // 默认值'round'
            /* 是否开启千分位格式化 */
            formatWithComma?: boolean;
          };
    }
  /* 百分比渲染器 */
  | {
      type: 'percent';
      config?:
        | any
        | {
            /* 保留小数位数 */
            decimals?: number;
            /* 小数舍入计算方式 */
            method?: 'round' | 'floor' | 'ceil'; // 默认值'round'
          };
    }
  /* 状态渲染器 */
  | {
      type: 'status';
      config?: any | { [key: string]: StatusConfig };
    }
  /* 日期渲染器 */
  | {
      type: 'date';
      config?:
        | any
        | {
            /* 格式化字符串 */
            format?: string;
            /* 时区偏移值设置，偏移值单位为分钟 */
            timezoneOffset?: number;
          };
    }
  /* 自定义渲染器模板 */
  | TemplateRef<any>;

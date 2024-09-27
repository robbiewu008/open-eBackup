import { CommonConsts } from 'app/shared';

export const DEFAULT_CONFIG = {
  filterTags: false,
  table: {
    async: true,
    rows: {
      selectionMode: null,
      showSelector: true,
      disabledSelectionLogic: 'exclude'
    },
    compareWith: '',
    showLoading: false,
    showAnimation: true,
    size: 'default',
    colResize: {
      mode: 'expand'
    },
    scroll: { x: '100%', y: null },
    scrollFixed: true,
    colDisplayControl: true,
    virtualItemHeight: 48,
    trackByFn: (index: number, item: any) => {
      return index;
    }
  },
  pagination: {
    showTotal: true,
    pageSizeOptions: CommonConsts.PAGE_SIZE_OPTIONS,
    showPageSizeOptions: true,
    pageIndex: CommonConsts.PAGE_START,
    pageSize: CommonConsts.PAGE_SIZE
  }
};

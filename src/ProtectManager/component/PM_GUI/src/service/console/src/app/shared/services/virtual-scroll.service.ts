import { Injectable } from '@angular/core';
import { Table_Size, Page_Size_Options, CookieService, CommonConsts } from '..';

@Injectable({
  providedIn: 'root'
})
export class VirtualScrollService {
  // 滚动参数，用于列表调整滚动高度， 初始化值为default表格20行高度
  scrollParam = { y: '480px' };
  scrollParamMap = {};
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  constructor(private cookieService: CookieService) {}

  /**
   * 获取滚动参数，用于主列表调整滚动高度
   * @param otherHeight 内容区除表格外的其他元素高度
   * @param minRow 最少显示几行数据
   * @param size 表格大小
   */
  getScrollParam(
    otherHeight: number = 300,
    minRow: number = Page_Size_Options.Three,
    size: string = Table_Size.Default,
    tableKey?: string
  ) {
    let height = this.getBrowserHeight();
    if (this.isHcsUser) {
      otherHeight = otherHeight - 65;
    }
    const scrollHeight = this.getScrollHeight(
      height,
      otherHeight,
      minRow,
      size
    );
    if (tableKey) {
      this.scrollParamMap[tableKey] = scrollHeight;
      if (
        tableKey === 'search-resource-table' &&
        parseInt(scrollHeight.y) < 800
      ) {
        this.scrollParamMap[tableKey] = { y: '800px' };
      }
      if (tableKey === 'search-file-table' && parseInt(scrollHeight.y) < 750) {
        this.scrollParamMap[tableKey] = { y: '750px' };
      }
    } else {
      this.scrollParam = scrollHeight;
    }
    window.onresize = () => {
      height = this.getBrowserHeight();
      if (tableKey) {
        this.scrollParamMap[tableKey] = this.getScrollHeight(
          height,
          otherHeight,
          minRow,
          size
        );
      } else {
        this.scrollParam = this.getScrollHeight(
          height,
          otherHeight,
          minRow,
          size
        );
      }
    };
  }

  private getBrowserHeight() {
    return (
      window.innerHeight ||
      document.documentElement.clientHeight ||
      document.body.clientHeight
    );
  }

  private getScrollHeight(height, otherHeight, minRow, size) {
    let minHeight,
      unitHeight = 48;
    if (size) {
      switch (size) {
        case Table_Size.Default:
          unitHeight = 48;
          break;
        case Table_Size.Small:
          unitHeight = 30;
          break;
      }
    }

    minHeight = unitHeight * minRow; // 主列表最小高度

    return height - 160 - otherHeight < minHeight
      ? { y: minHeight + 'px' }
      : { y: height - 160 - otherHeight + 'px' };
  }
}

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

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
import { guid } from '../utils';

// modal常量
export const MODAL_COMMON = {
  gutterModal: 20,
  smallWidth: 400,
  normalWidth: 600,
  largeWidth: 900,
  xLargeWidth: 1124,
  generateDrawerOptions() {
    return {
      ...this.drawerOptions,
      lvModalKey: guid()
    };
  },
  drawerOptions: {
    lvDrawerPositionOffset: [0, 0, 0, 0],
    lvModalKey: 'aui-modal',
    lvType: 'drawer',
    lvWidth: 900,
    lvCloseButtonDisplay: true,
    positionOffset: ['0px', '0px', '0px', '0px'],
    position: 'right'
  },
  xSmallModal: 416,
  smallModal: 608,
  largeModal: 896,
  xLargeModal: 1232
};

// group
export const GROUP_COMMON = {
  xSmallColumnGutter: '12px',
  smallColumnGutter: '20px',
  columnGutter: '24px',
  largeColumnGutter: '40px',
  rowGutter: '8px',
  largeRowGutter: '12px',
  xLargeRowGutter: '16px',
  middleRowGutter: '16px'
};

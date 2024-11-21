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
import { CapacityCalculateLabel } from './capacity.pipe';
import { I18NPipe } from './i18n.pipe';
import { NilPipe } from './nil.pipe';
import { TextMapPipe } from './text-map.pipe';
import { FindPipe } from './find.pipe';
import { TimestampPipe } from './timestamp.pipe';

export * from './capacity.pipe';
export * from './i18n.pipe';
export * from './nil.pipe';
export * from './text-map.pipe';
export * from './find.pipe';

export const PIPES = [
  TextMapPipe,
  I18NPipe,
  CapacityCalculateLabel,
  NilPipe,
  TimestampPipe,
  FindPipe
];

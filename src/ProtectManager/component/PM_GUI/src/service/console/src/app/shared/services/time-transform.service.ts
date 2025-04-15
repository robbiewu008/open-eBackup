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
import { Injectable } from '@angular/core';
import { cloneDeep, size } from 'lodash';
import { DataMapService, I18NService } from '.';

@Injectable({
  providedIn: 'root'
})
export class TimeTransformService {
  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService
  ) {}

  transformTime(item) {
    let data = cloneDeep(item);
    let specifiedTime;
    const isEn = this.i18n.isEn;
    const timeUnits = this.dataMapService.toArray('Detecting_During_Unit');
    let blankSpace = [0, 0, 0, 0, 0, 0, 0];
    let biggestNum = 0;
    while (data > 0) {
      for (let i = 0; i < size(timeUnits); i++) {
        if (data < timeUnits[i].convertSecond) {
          blankSpace[i - 1] = 1;
          if (data === item) {
            biggestNum = i - 1;
          }
          for (let j = biggestNum; j >= i; j--) {
            if (blankSpace[j] === 0) {
              if (isEn) {
                specifiedTime += `0 ${timeUnits[j].label}(s), `;
              } else {
                specifiedTime += `0${timeUnits[j].label}`;
              }
              blankSpace[j] = 1;
            }
          }
          let reduceTime = data - (data % timeUnits[i - 1].convertSecond);
          let newTimeLabel = isEn
            ? `${reduceTime / timeUnits[i - 1].convertSecond} ${this.i18n.get(
                timeUnits[i - 1].label
              )}`
            : `${reduceTime / timeUnits[i - 1].convertSecond}${this.i18n.get(
                timeUnits[i - 1].label
              )}`;
          if (specifiedTime) {
            specifiedTime += newTimeLabel;
          } else {
            specifiedTime = newTimeLabel;
          }
          if (isEn) {
            specifiedTime += `(s), `;
          }
          data = data - reduceTime;
          break;
        }
      }
    }
    if (item === 0) {
      specifiedTime = `0 ${timeUnits[0].label}`;
    }
    if (isEn) {
      specifiedTime = specifiedTime.slice(0, -2);
    }
    return specifiedTime;
  }
}

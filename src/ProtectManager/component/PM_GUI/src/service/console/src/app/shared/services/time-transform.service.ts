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
    if (isEn) {
      specifiedTime = specifiedTime.slice(0, -2);
    }
    return specifiedTime;
  }
}

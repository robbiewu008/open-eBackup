import { Injectable } from '@angular/core';
import { GlobalService } from './store.service';
import { isString, assign, isEmpty } from 'lodash';

@Injectable({
  providedIn: 'root'
})
export class RememberColumnsService {
  constructor() {}
  currentUser;

  setUser(user) {
    this.currentUser = user;
  }

  setColumnsStatus(tableKey, tableParams) {
    if (window.localStorage) {
      let allParams = localStorage.getItem('TABLE_COLUMN_STATUS') || {};
      if (isString(allParams)) {
        allParams = JSON.parse(allParams);
      }
      const currentUserParams = allParams[this.currentUser] || {};
      assign(currentUserParams, {
        [tableKey]: tableParams
      });
      localStorage.setItem(
        'TABLE_COLUMN_STATUS',
        JSON.stringify(
          assign(allParams, {
            [this.currentUser]: currentUserParams
          })
        )
      );
    }
  }

  getColumnsStatus(tableKey) {
    if (window.localStorage) {
      let columnsStatus = localStorage.getItem('TABLE_COLUMN_STATUS') || {};
      if (isString(columnsStatus)) {
        columnsStatus = JSON.parse(columnsStatus);
      }
      return columnsStatus[this.currentUser]
        ? columnsStatus[this.currentUser][tableKey]
        : {};
    }
    return {};
  }
}

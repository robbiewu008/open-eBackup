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
  ChangeDetectorRef,
  Component,
  ElementRef,
  EventEmitter,
  Input,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import { WarningMessageService } from '../../services';
import { GetLabelOptionsService } from '../../services/get-labels.service';
import { CommonConsts } from '../../consts';
import { AppUtilsService } from '../../services/app-utils.service';
import { LabelApiService } from '../../api/services/label-api.service';
import { debounce } from 'lodash';

@Component({
  selector: 'aui-custom-table-filter',
  templateUrl: './custom-table-filter.component.html',
  styleUrls: ['./custom-table-filter.component.less']
})
export class CustomTableFilterComponent implements OnInit {
  @Input() filterTitle: string;
  @Input() value; // 用于传入数据重置
  @Output() filter = new EventEmitter<any>();
  data: any[];
  nameFilterValue: any[] = [];
  nameFilterPanelVisible = false;
  searchValue;
  labelLoading = true;
  @ViewChild('primaryButton', { read: ElementRef }) primaryButton: ElementRef<
    HTMLElement
  >;
  @ViewChild('nameFilterSelect') nameFilterInput: ElementRef<HTMLElement>;

  constructor(
    private cdr: ChangeDetectorRef,
    public warningMessageService: WarningMessageService,
    private appUtilsService: AppUtilsService,
    private labelApiService: LabelApiService
  ) {}

  ngOnInit() {
    this.initData();
  }

  openNameFilterPanel() {
    this.nameFilterPanelVisible = !this.nameFilterPanelVisible;
    setTimeout(() => {
      this.nameFilterInput?.nativeElement?.focus();
    }, 300);
  }

  reset() {
    // 输入框和多选置空
    this.nameFilterValue = [];
    this.filterByName(this.nameFilterValue);
    this.nameFilterPanelVisible = false;
  }

  filterByName(value: any[], isManual = true) {
    this.nameFilterValue = value;
    this.filter.emit(value);
  }
  initData() {
    const extParams = {
      startPage: CommonConsts.PAGE_START_EXTRA,
      akLoading: false
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.labelApiService.queryLabelUsingGET(params),
      res => {
        this.data = res?.map(item => {
          return {
            id: item.uuid,
            label: item.name,
            value: item.uuid,
            isLeaf: true
          };
        });
        this.labelLoading = false;
      }
    );
  }
}

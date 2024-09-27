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
import { Component, OnInit } from '@angular/core';
import { Subject } from 'rxjs';
import { FormBuilder } from '@angular/forms';
import { BaseUtilService, I18NService } from 'app/shared';

@Component({
  selector: 'aui-advanced',
  templateUrl: './advanced.component.html',
  styleUrls: ['./advanced.component.less']
})
export class AdvancedComponent implements OnInit {
  formGroup;
  valid$ = new Subject<boolean>();
  proxyHost = [];

  isBindSla = false;
  slaSelectRadioOption = [
    {
      value: 'overwrite',
      label: this.i18n.get('protection_overwrite_sla_label')
    },
    {
      value: 'unOverwrite',
      label: this.i18n.get('protection_not_overwrite_sla_label')
    }
  ];
  slaSelectCheckboxOption = [
    {
      value: 'selectedHost',
      label: this.i18n.get('protection_apply_sla_in_host_label')
    },
    {
      value: 'newVm',
      label: this.i18n.get('protection_apply_sla_in_new_vm_label')
    },
    {
      value: 'noApply',
      label: this.i18n.get('protection_remove_sla_no_apply_label')
    }
  ];

  exclude = [];
  include = [];
  constructor(
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService,
    private i18n: I18NService
  ) {}

  ngOnInit() {
    this.getProxyHost();
    this.initForm();
  }
  initForm() {
    this.formGroup = this.fb.group({
      proxyHost: [, this.baseUtilService.VALID.required()],
      preScript: [],
      postScript: [],
      slaSelect: [
        this.isBindSla
          ? this.slaSelectRadioOption[0].value
          : [
              this.slaSelectCheckboxOption[0].value,
              this.slaSelectCheckboxOption[1].value,
              this.slaSelectCheckboxOption[2].value
            ]
      ],
      vmFilter: [true],
      excludeCheck: [true],
      excludeInput: [],
      exclude: [],
      includeCheck: [true],
      includeInput: [],
      include: []
    });
    this.formGroup.statusChanges.subscribe(res => {
      this.valid$.next(res === 'VALID');
    });
  }

  getProxyHost() {
    this.proxyHost = [
      {
        key: 'node01',
        label: 'node(10.10.10.10)',
        isLeaf: true
      },
      {
        key: 'node02',
        label: 'node(10.10.10.11)',
        isLeaf: true
      },
      {
        key: 'node03',
        label: 'node(10.10.10.12)',
        isLeaf: true
      }
    ];
  }

  clearExclude() {
    this.formGroup.get('excludeInput').setValue('');
  }

  addExclude() {
    if (this.formGroup.value.excludeInput) {
      this.exclude = [
        ...this.exclude,
        {
          label: this.formGroup.value.excludeInput,
          removeable: true
        }
      ];
      this.formGroup.get('exclude').setValue(this.exclude);
    }
  }

  clearInclude() {
    this.formGroup.get('includeInput').setValue('');
  }

  addInclude() {
    if (this.formGroup.value.includeInput) {
      this.include = [
        ...this.include,
        {
          label: this.formGroup.value.includeInput,
          removeable: true
        }
      ];
      this.formGroup.get('include').setValue(this.include);
    }
  }
}

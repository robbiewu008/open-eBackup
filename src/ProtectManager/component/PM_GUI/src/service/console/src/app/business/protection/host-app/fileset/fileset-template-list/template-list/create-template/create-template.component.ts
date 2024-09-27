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
import { Component, Input, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageService, ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  FilesetTemplatesApiService,
  I18NService,
  MODAL_COMMON
} from 'app/shared';
import {
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import {
  assign,
  filter,
  includes,
  isEmpty,
  map,
  reject,
  size,
  union,
  uniqueId
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { AddPathComponent } from './add-path/add-path.component';
import { cacheGuideResource } from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-create-template',
  templateUrl: './create-template.component.html',
  styleUrls: ['./create-template.component.less']
})
export class CreateTemplateComponent implements OnInit {
  @Input() rowItem;
  osType = DataMap.Fileset_Template_Os_Type.linux.value;
  tableConfig: TableConfig;
  tableData: TableData;
  filterParams = [];
  selection = [];
  formGroup: FormGroup;
  dataMap = DataMap;
  osTypeOptions = this.dataMapService
    .toArray('Fileset_Template_Os_Type')
    .filter(v => {
      return (v.isLeaf = true);
    });
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };

  @ViewChild('resourceFilter', { static: false }) filterComponent;
  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private modal: ModalRef,
    private fb: FormBuilder,
    private i18n: I18NService,
    private message: MessageService,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private drawModalService: DrawModalService,
    private infoMessageService: InfoMessageService,
    private filesetTemplatesApiService: FilesetTemplatesApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initTableConfig();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl(
        this.rowItem?.isClone
          ? `clone_${this.rowItem?.name}`
          : this.rowItem?.name || '',
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(32),
            this.baseUtilService.VALID.name()
          ]
        }
      ),
      os_type: new FormControl(
        this.rowItem?.osType || DataMap.Fileset_Template_Os_Type.linux.value,
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      )
    });

    this.formGroup.get('os_type').valueChanges.subscribe(res => {
      this.osType = res;
      this.tableData = {
        data: [],
        total: 0
      };
      this.disableOkBtn();
    });

    this.formGroup.statusChanges.subscribe(res => this.disableOkBtn());

    if (this.rowItem?.osType) {
      this.osType = this.rowItem?.osType;
    }

    if (this.rowItem?.isClone) {
      setTimeout(() => {
        this.formGroup.get('name').updateValueAndValidity();
        this.formGroup.get('name').markAsTouched();
        this.disableOkBtn();
      });
    }
  }

  initTableConfig() {
    this.tableConfig = {
      table: {
        async: false,
        compareWith: 'path',
        size: 'small',
        columns: [
          { key: 'path', name: this.i18n.get('common_path_label') },
          {
            key: 'operation',
            width: 130,
            hidden: 'ignoring',
            name: this.i18n.get('common_operation_label'),
            cellRender: {
              type: 'operation',
              config: {
                maxDisplayItems: 2,
                items: [
                  {
                    id: 'remove',
                    label: this.i18n.get('common_remove_label'),
                    onClick: data => this.removePath(data)
                  }
                ]
              }
            }
          }
        ],
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: false,
        selectionChange: selection => {
          this.selection = selection;
          this.disableOkBtn();
        }
      },
      pagination: {
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS,
        showPageSizeOptions: false,
        mode: 'simple',
        winTablePagination: true
      }
    };
    if (this.rowItem && this.rowItem.files) {
      this.tableData = {
        data: map(this.rowItem.files, item => {
          return {
            path: item,
            randomId: uniqueId()
          };
        }),
        total: size(this.rowItem.files)
      };
    }
  }

  addPath() {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: this.i18n.get('common_add_label'),
      lvContent: AddPathComponent,
      lvOkDisabled: true,
      lvWidth: MODAL_COMMON.smallModal,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as AddPathComponent;
        const modalIns = modal.getInstance();
        content.formGroup.statusChanges.subscribe(res => {
          modalIns.lvOkDisabled = res !== 'VALID';
        });
      },
      lvComponentParams: {
        osType: this.osType
      },
      lvOk: modal => {
        const content = modal.getContentComponent() as AddPathComponent;
        const paths = content.formGroup.value.paths?.split(',') || [];
        const unionPaths = union(
          map(this.tableData?.data, 'path'),
          filter(paths, item => {
            return !isEmpty(item);
          })
        );
        if (size(unionPaths) > 64) {
          this.message.error(
            this.i18n.get('protection_template_path_number_error_label'),
            {
              lvMessageKey: 'formatErrorKey',
              lvShowCloseButton: true
            }
          );
          return false;
        }
        this.tableData = {
          data: map(unionPaths, item => {
            return {
              path: item,
              randomId: uniqueId()
            };
          }),
          total: size(unionPaths)
        };
        this.disableOkBtn();
      }
    });
  }

  removePath(datas, clearSelection?) {
    const rejectData = reject(this.tableData.data, item => {
      return includes(map(datas, 'randomId'), item.randomId);
    });
    this.tableData = {
      data: rejectData,
      total: size(rejectData)
    };
    if (clearSelection) {
      this.selection = [];
      this.dataTable.setSelections([]);
    }
    this.disableOkBtn();
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled =
      !this.formGroup.valid || isEmpty(this.tableData?.data);
  }

  create(isModify): Observable<void> {
    return new Observable<void>((observer: Observer<any>) => {
      this.filterComponent.collectParams();
      const params = assign({
        files: map(this.tableData.data, 'path'),
        filters: this.filterParams,
        name: this.formGroup.value.name,
        osType: this.formGroup.value.os_type
      });
      if (isModify) {
        if (isModify.isClone) {
          this.filesetTemplatesApiService
            .createUsingPOST({ filesetTemplate: params })
            .subscribe({
              next: res => {
                observer.next(res);
                observer.complete();
              },
              error: err => {
                observer.error(err);
                observer.complete();
              }
            });
        } else {
          assign(params, { uuid: this.rowItem.uuid });
          if (this.rowItem && !!this.rowItem.associated_filesets_num) {
            this.infoMessageService.create({
              content: this.i18n.get('protection_modify_template_tip_label'),
              onOK: () => {
                this.filesetTemplatesApiService
                  .updateUsingPUT({ filesetTemplate: params })
                  .subscribe({
                    next: res => {
                      observer.next(res);
                      observer.complete();
                    },
                    error: err => {
                      observer.error(err);
                      observer.complete();
                    }
                  });
              },
              onCancel: () => {
                observer.error(null);
                observer.complete();
              },
              lvAfterClose: result => {
                if (result && result.trigger === 'close') {
                  observer.error(null);
                  observer.complete();
                }
              }
            });
          } else {
            this.filesetTemplatesApiService
              .updateUsingPUT({ filesetTemplate: params })
              .subscribe({
                next: res => {
                  observer.next(res);
                  observer.complete();
                },
                error: err => {
                  observer.error(err);
                  observer.complete();
                }
              });
          }
        }
      } else {
        this.filesetTemplatesApiService
          .createUsingPOST({ filesetTemplate: params })
          .subscribe({
            next: res => {
              cacheGuideResource(res);
              observer.next(res);
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      }
    });
  }
}

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
import { Component, OnInit, ViewChild, TemplateRef } from '@angular/core';
import {
  FormControl,
  FormGroup,
  FormBuilder,
  ValidatorFn,
  AbstractControl
} from '@angular/forms';
import {
  CommonConsts,
  DataMap,
  I18NService,
  DataMapService,
  BaseUtilService,
  TapeType,
  TapeLibraryApiService,
  LogManagerApiService,
  MediaSetApiService,
  CAPACITY_UNIT
} from 'app/shared';
import {
  includes,
  assign,
  cloneDeep,
  omit,
  isEmpty,
  first,
  union,
  size,
  map,
  trim,
  find
} from 'lodash';
import { OptionItem } from '@iux/live';
import {
  TableConfig,
  TableCols,
  TableData,
  ProTableComponent,
  Filters
} from 'app/shared/components/pro-table';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-create-storage-pool',
  templateUrl: './create-storage-pool.component.html',
  styleUrls: ['./create-storage-pool.component.less']
})
export class CreateStoragePoolComponent implements OnInit {
  data;
  active;
  node;
  libraries = [];
  activeIndex = 'total';
  currentTotalTape = 0;
  totalSelectedTape = 0;
  currentSelect = 0;
  selectedTapes = [];
  formGroup: FormGroup;
  dataMap = DataMap;
  _includes = includes;
  tapeType = TapeType;
  tapeTableData: TableData;
  unitconst = CAPACITY_UNIT;
  tapeTableConfig: TableConfig;
  nodeItems: OptionItem[];
  retentionTypes: OptionItem[];
  blockSizeOptions: OptionItem[];
  retentionValueUnits: OptionItem[];
  validSelection$ = new Subject<boolean>();

  nameLabel = this.i18n.get('common_name_label');
  typeLabel = this.i18n.get('system_tape_type_label');
  tapeLabel = this.i18n.get('system_archive_tape_label');
  basicInfoLabel = this.i18n.get('common_basic_info_label');
  rentainLabel = this.i18n.get('common_retention_policy_label');
  controllerLabel = this.i18n.get('common_home_node_label');
  placeholderLabel = this.i18n.get('common_loading_label');
  selectTapesLabel = this.i18n.get('system_archive_select_tapes_label');
  blockSizeLabel = this.i18n.get('system_block_size_label');
  compressionLabel = this.i18n.get('system_data_compression_label');
  capacityThresholdLabel = this.i18n.get(
    'common_capacity_alarm_threshold_label'
  );
  alarmThresholdLabel = this.i18n.get('common_alarm_threshold_label');
  clearThresholdLabel = this.i18n.get('system_clear_threshold_label');
  insufficientLabel = this.i18n.get('system_archive_insufficient_tape_label');
  nameErrorTip = assign(
    { ...this.baseUtilService.nameErrorTip },
    { invalidName: this.i18n.get('common_valid_name_label') }
  );
  rangeErrorTip = assign(
    {
      ...this.baseUtilService.integerErrorTip,
      ...this.baseUtilService.requiredErrorTip
    },
    { invalidMinSize: this.i18n.get('common_valid_minsize_label', [1]) },
    { invalThreshold: this.i18n.get('common_valid_threshold_label') }
  );
  blockRangeErrorTip = {
    ...this.baseUtilService.rangeErrorTip
  };
  maxLengthErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [256]),
    spaceErrorTip: this.i18n.get('common_valid_space_label')
  };
  @ViewChild('tapeTable', { static: false }) tapeTable: ProTableComponent;
  @ViewChild('capacityTpl', { static: true }) capacityTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private dataMapService: DataMapService,
    private logManagerApiService: LogManagerApiService,
    public baseUtilService: BaseUtilService,
    private tapeLibraryApiService: TapeLibraryApiService,
    private mediaSetApiService: MediaSetApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initTable();
    this.initOptionItems();
  }

  initForm() {
    this.formGroup = this.fb.group({
      mediaSetName: new FormControl(
        { value: '', disabled: !!this.data },
        {
          validators: [this.baseUtilService.VALID.name(CommonConsts.REGEX.name)]
        }
      ),
      blockSize: new FormControl(DataMap.Tape_Block_Size.large.value, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      compressionStatus: new FormControl(
        DataMap.Archive_Compression_Status.enable.value
      ),
      type: new FormControl(this.tapeType.Rw, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      node: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      alarmEnable: new FormControl(true),
      alarmThreshold: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.minSize(0),
          this.validThreshold()
        ]
      }),
      retentionType: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      retentionDuration: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.minSize(0)
        ]
      }),
      retentionUnit: new FormControl(
        this.dataMap.Tape_Retention_Unit.day.value,
        {
          validators: [this.baseUtilService.VALID.required()]
        }
      )
    });
    this.formGroup.get('node').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      this.getLibraries(res);
    });
    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      if (res === this.tapeType.Worm) {
        this.formGroup
          .get('retentionType')
          .setValue(this.dataMap.Tape_Retention_Type.permanent.value);
        this.formGroup.get('retentionType').updateValueAndValidity();
      }
      this.formGroup.get('node').updateValueAndValidity();
    });
    this.formGroup.get('retentionType').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      if (res !== this.dataMap.Tape_Retention_Type.temporary.value) {
        this.formGroup.get('retentionDuration').clearValidators();
      } else {
        this.formGroup
          .get('retentionDuration')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.minSize(0)
          ]);
      }
      this.formGroup.get('retentionDuration').updateValueAndValidity();
    });
    this.formGroup.get('alarmEnable').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('alarmThreshold')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.integer(),
            this.baseUtilService.VALID.minSize(0),
            this.validThreshold()
          ]);
      } else {
        this.formGroup.get('alarmThreshold').clearValidators();
      }
      this.formGroup.get('alarmThreshold').updateValueAndValidity();
    });
    if (!isEmpty(this.data)) {
      this.formGroup.patchValue(this.data);
      if (!this.data.alarmEnable) {
        this.formGroup.get('alarmThreshold').setValue('');
      }
      this.formGroup.get('type').clearValidators();
      this.formGroup.get('node').clearValidators();
      this.formGroup.get('blockSize').clearValidators();
      this.formGroup.get('compressionStatus').clearValidators();
      this.formGroup.updateValueAndValidity();
    }
  }

  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = cloneDeep(omit(this.formGroup.value, ['']));
      assign(params, {
        mediaSetName: this.formGroup.get('mediaSetName')?.value,
        tapes: this.selectedTapes,
        alarmThreshold: parseInt(this.formGroup.value.alarmThreshold, 0),
        retentionDuration: parseInt(this.formGroup.value.retentionDuration, 0),
        blockSize: parseInt(this.formGroup.value.blockSize, 0)
      });
      this.mediaSetApiService
        .createMediaSetUsingPOST({
          mediaPoolCreateRequest: params,
          memberEsn: this.node?.remoteEsn
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }

  modify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = cloneDeep(omit(this.formGroup.value, ['']));
      assign(params, {
        mediaSetName: this.formGroup.get('mediaSetName')?.value,
        tapes: this.selectedTapes,
        alarmThreshold: parseInt(this.formGroup.value.alarmThreshold, 0),
        retentionDuration: parseInt(this.formGroup.value.retentionDuration, 0),
        blockSize: parseInt(this.formGroup.value.blockSize, 0)
      });
      this.mediaSetApiService
        .updateMediaSetUsingPUT({
          mediaSetUpdateRequest: params,
          mediaSetId: this.data.mediaSetId,
          memberEsn: this.node?.remoteEsn
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }

  initOptionItems() {
    this.logManagerApiService
      .collectNodeInfo({ memberEsn: this.node?.remoteEsn })
      .subscribe(res => {
        if (res.data.length) {
          const nodeItems = [];
          res.data.forEach(item => {
            nodeItems.push({
              value: item.nodeName,
              label: item.nodeName,
              isLeaf: true
            });
          });
          this.nodeItems = nodeItems.sort((node1, node2) =>
            node1.label.localeCompare(node2.label)
          );
          this.placeholderLabel = this.i18n.get('common_select_label');
        }
      });
    this.retentionTypes = this.dataMapService
      .toArray('Tape_Retention_Type')
      .filter(v => {
        return (v.isLeaf = true);
      });
    this.retentionValueUnits = this.dataMapService
      .toArray('Tape_Retention_Unit')
      .filter(v => {
        return (v.isLeaf = true);
      });
    this.blockSizeOptions = this.dataMapService
      .toArray('Tape_Block_Size')
      .filter(v => {
        return (v.isLeaf = true);
      });
  }

  initTable() {
    const tapeCols: TableCols[] = [
      {
        key: 'tapeLabel',
        name: this.i18n.get('system_archive_tape_labe_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'writeStatus',
        name: this.i18n.get('system_archive_write_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Tape_Write_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Tape_Write_Status')
        }
      },
      {
        key: 'status',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Library_Tape_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Library_Tape_Status')
        }
      },
      {
        key: 'worm',
        name: this.i18n.get('system_tape_type_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Media_Pool_Type')
        }
      },
      {
        key: 'usedCapacity',
        name: this.i18n.get('common_used_capcity_label'),
        thAlign: 'right',
        cellRender: this.capacityTpl
      }
    ];
    this.tapeTableConfig = {
      table: {
        size: 'small',
        compareWith: 'tapeUUID',
        columns: tapeCols,
        virtualScroll: true,
        colDisplayControl: false,
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        virtualItemHeight: 32,
        scrollFixed: true,
        scroll: { y: '340px' },
        fetchData: (filter: Filters) => {
          this.getTapes(filter);
        },
        selectionChange: (selection, renderSelection) => {
          this.currentSelect = selection.length;
          this.libraries.forEach(e => {
            if (e.id === this.active) {
              e.selectedTapes = selection;
            }
          });
          let selections = [];
          this.libraries.forEach(e => {
            selections = union(selections, e.selectedTapes);
          });
          this.selectedTapes = selections;
          this.totalSelectedTape = selections.length;
          this.validSelection$.next(!!size(selections));
          this.formGroup.get('alarmThreshold').updateValueAndValidity();
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple'
      }
    };
  }

  getLibraries(node) {
    if (!isEmpty(this.data)) {
      this.selectedTapes = this.data.tapes;
    } else {
      this.selectedTapes = [];
      this.totalSelectedTape = 0;
    }
    this.tapeLibraryApiService
      .getTapeLibrariesUsingGET({
        controllerName: node,
        memberEsn: this.node?.remoteEsn
      })
      .subscribe(res => {
        this.libraries = res.records
          .filter(
            item =>
              (this.data &&
                find(this.data.tapes, { tapeLibrarySn: item.serialNo })) ||
              item.status === DataMap.Media_Tape_Status.online.value
          )
          .map(item => {
            return {
              id: item.serialNo,
              label: item.name,
              status: item.status,
              selectedTapes: isEmpty(this.data)
                ? []
                : this.fetchTapeDisabled(this.data.tapes, item).filter(
                    e => e.tapeLibrarySn === item.serialNo
                  )
            };
          });
        if (!size(this.libraries)) {
          this.tapeTableData = {
            data: [],
            total: 0
          };
          return;
        }
        this.active = first(this.libraries).id;
        this.getTapes();
      });
  }

  selectIndexChange(e) {
    if (e === 'selected') {
      const library = this.libraries.find(item => item.id === this.active);
      this.tapeTableData = {
        data: library ? library.selectedTapes : [],
        total: library ? size(library.selectedTapes) : 0
      };
      this.tapeTable.setSelections(library.selectedTapes);
    } else {
      this.tapeTable.fetchData();
    }
  }

  itemClick(e) {
    this.activeIndex = 'total';
    this.tapeTable.fetchData();
  }

  getTapes(filters?) {
    const params = {
      tapeLibrarySn: this.active,
      worms: [this.formGroup.value.type],
      pageSize: filters ? filters.paginator.pageSize : CommonConsts.PAGE_SIZE,
      pageNo: filters
        ? filters.paginator.pageIndex + 1
        : CommonConsts.PAGE_START + 1,
      memberEsn: this.node?.remoteEsn
    };

    if (filters && !isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.tapeLabel) {
        assign(params, { tapeLabel: conditions.tapeLabel });
      }
      if (conditions.writeStatus) {
        assign(params, { writeStatuses: conditions.writeStatus });
      }
      if (conditions.status) {
        assign(params, { statuses: conditions.status });
      }
    }

    this.tapeLibraryApiService
      .getLibraryTapesUsingGET(params)
      .subscribe(res => {
        const library = this.libraries.find(item => item.id === this.active);
        const tapes = this.fetchTapeDisabled(res.records, library);
        this.tapeTableData = {
          data: tapes,
          total: res.totalCount
        };
        const selections = this.libraries.find(item => item.id === this.active)
          .selectedTapes;
        this.tapeTable.setSelections(selections ? selections : []);
        this.currentTotalTape = this.tapeTableData.total;
        this.totalSelectedTape = this.selectedTapes.length;
        this.currentSelect = this.tapeTable.getAllSelections().length;
        this.formGroup.updateValueAndValidity();
      });
  }

  fetchTapeDisabled(data: any, library: any) {
    const tapes = data.filter(item =>
      (library && library.status === DataMap.Media_Tape_Status.offline.value) ||
      (isEmpty(this.data) && !isEmpty(item.mediaSetId)) ||
      (!isEmpty(this.data) &&
        !isEmpty(item.mediaSetId) &&
        item.mediaSetId !== this.data.mediaSetId) ||
      (this.formGroup.value.type === this.tapeType.Rw &&
        includes(
          [
            this.dataMap.Tape_Write_Status.unknown.value,
            this.dataMap.Tape_Write_Status.error.value
          ],
          item.writeStatus
        )) ||
      (this.formGroup.value.type === this.tapeType.Worm &&
        includes(
          [this.dataMap.Tape_Write_Status.error.value],
          item.writeStatus
        )) ||
      this.dataMap.Library_Tape_Status.inLibrary.value !== item.status
        ? assign(item, { disabled: true })
        : item
    );
    if (size(this.selectedTapes)) {
      this.validSelection$.next(true);
      tapes.forEach(item => {
        if (includes(map(this.selectedTapes, 'tapeUUID'), item.tapeUUID)) {
          assign(item, {
            disabled: includes(
              [
                this.dataMap.Tape_Write_Status.full.value,
                this.dataMap.Tape_Write_Status.written.value
              ],
              item.writeStatus
            )
          });
        }
      });
    }
    return tapes;
  }

  validThreshold(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!this.formGroup) {
        return null;
      }

      if (!trim(control.value)) {
        return null;
      }

      return +control.value > size(this.selectedTapes)
        ? {
            invalThreshold: { value: control.value }
          }
        : null;
    };
  }
}

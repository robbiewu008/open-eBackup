import { ChangeDetectorRef, Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { MessageService, ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService
} from 'app/shared';
import {
  USER_GUIDE_CACHE_DATA,
  cacheGuideResource
} from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  each,
  filter,
  find,
  includes,
  isNumber,
  isUndefined,
  map,
  reject,
  size
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-object',
  templateUrl: './register-object.component.html',
  styleUrls: ['./register-object.component.less']
})
export class RegisterObjectComponent implements OnInit {
  formGroup: FormGroup;
  objectOptions = [];
  tableData = [];
  selectedTableData = [];
  modifiedTables = [];
  totalTable;
  selectionTable = [];
  name;
  selectedName;
  displayLeftData = [];
  displayRightData = [];
  tempTableData = [];
  tempSelectedData = [];
  pageIndexS = CommonConsts.PAGE_START;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  dataMap = DataMap;
  rowData;
  filters = [];
  errorTip = this.i18n.get('common_valid_maxlength_label', [1024]);
  prefixTipLabel = this.i18n.get(
    'protection_object_bucket_prefix_all_tip_label'
  );
  error = false;
  isSearch = false;

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip
  };

  prefixError = {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024])
  };

  @ViewChild('lvTable', { static: false }) lvTable;
  @ViewChild('lvSelectTable', { static: false }) lvSelectTable;
  @ViewChild('pageS', { static: false }) pageS;
  @ViewChild('pageA', { static: false }) pageA;
  @ViewChild('popover', { static: false }) popover;
  @ViewChild('objectPopover', { static: false }) objectPopover;
  @ViewChild('selectedObjectPopover', { static: false }) selectedObjectPopover;
  @ViewChild('prefixTipTpl', { static: false }) prefixTipTpl;

  constructor(
    public modal: ModalRef,
    public messageService: MessageService,
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getObjectStorage();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.resource, item.uuid)
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      objectStorage: new FormControl('test', {
        validators: this.baseUtilService.VALID.required()
      }),
      object: new FormControl([], {
        validators: this.baseUtilService.VALID.required()
      })
    });

    this.formGroup.get('objectStorage').valueChanges.subscribe(res => {
      this.selectedTableData = [];
      this.tableData = [];
      this.selectionTable = [];
      this.formGroup.get('object').setValue(this.selectedTableData);
      this.getBucket(res);
    });

    if (this.rowData) {
      this.formGroup.patchValue({
        name: this.rowData.name,
        objectStorage: this.rowData.environment.uuid
      });
    }
  }

  getObjectStorage() {
    const extParams = {
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.ObjectStorage.value
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        const hostArray = [];
        each(resource, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}`,
            isLeaf: true
          });
        });
        this.objectOptions = hostArray;
      }
    );
  }

  getBucket(res, recordsTemp?, startPage?) {
    const params: any = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize * 10,
      envId: res
    };

    this.protectedEnvironmentApiService
      .ListEnvironmentResource(params)
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage ===
            Math.ceil(res.totalCount / (CommonConsts.PAGE_SIZE * 10)) ||
          res.totalCount === 0
        ) {
          const bucketArray = [];
          each(recordsTemp, item => {
            bucketArray.push({
              name: item.name
            });
          });
          this.tableData = bucketArray;
          this.tempTableData = this.tableData;
          this.totalTable = size(this.tableData);
          if (this.rowData) {
            const modified = JSON.parse(this.rowData.extendInfo.bucketList);
            each(this.tableData, item => {
              const exist = find(modified, val => val.name === item.name);
              if (exist) {
                this.selectionTable.push(item);
                if (exist.prefix) {
                  this.filters.push({
                    name: item.name,
                    data: exist.prefix
                  });
                }
              }
            });
            this.selectionTable = [...this.selectionTable];
            this.selectedTableData = [...this.selectionTable];
            each(this.selectedTableData, item => {
              if (find(this.filters, val => val.name === item.name)) {
                item['chosen'] = true;
              }
            });
            this.formGroup.get('object').setValue(this.selectedTableData);
          }
          this.cdr.detectChanges();
          return;
        }
        this.getBucket(res, recordsTemp, startPage);
      });
  }

  trackByName(index: string, list: any) {
    return list.name;
  }

  selectionChange(selection) {
    this.selectionTable = selection;
    this.selectedTableData = [...this.selectionTable];
    this.formGroup.get('object').setValue(this.selectedTableData);
    if (this.isSearch) {
      this.searchSelectedObjectName(this.selectedName);
    }
    this.clearFilter();
    this.cdr.detectChanges();
    this.disableBtn();
  }

  removeSingle(item) {
    this.selectionTable = reject(this.selectionTable, value => {
      return value.name === item.name;
    });
    this.selectedTableData = reject(this.selectedTableData, value => {
      return item.name === value.name;
    });
    this.formGroup.get('object').setValue(this.selectedTableData);
    this.clearFilter();
    this.cdr.detectChanges();
    this.disableBtn();
  }

  clearSelected() {
    if (this.isSearch) {
      this.selectionTable = filter(this.selectionTable, item => {
        return !find(this.selectedTableData, val => val.name === item.name);
      });
      this.tempSelectedData = [...this.selectionTable];
      this.selectedTableData = [];
      this.formGroup.get('object').setValue(this.selectionTable);
    } else {
      this.selectionTable = [];
      this.selectedTableData = [];
      this.formGroup.get('object').setValue(this.selectedTableData);
    }
    this.clearFilter();
    this.disableBtn();
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
  }

  pageChangeS(page) {
    this.pageSize = page.pageSize;
    this.pageIndexS = page.pageIndex;
  }

  addFilter(item) {
    for (let tmp of this.selectedTableData) {
      if (tmp.name === item) {
        tmp['chosen'] = true;
      }
    }
    this.popover.hide();
    this.filters.push({
      name: item,
      data: []
    });
  }

  deleteFilter(item) {
    item.data = [];
    this.filters = this.filters.filter(tmp => {
      return tmp.name !== item.name;
    });
    for (let tmp of this.selectedTableData) {
      if (tmp.name === item.name) {
        tmp['chosen'] = false;
      }
    }
    this.disableBtn();
  }

  clearFilter() {
    this.filters = filter(this.filters, item => {
      return find(this.selectionTable, val => val.name === item.name);
    });
    each(this.selectionTable, item => {
      if (!find(this.filters, val => val.name === item.name)) {
        if (!isUndefined(item.chosen)) {
          delete item.chosen;
        }
      }
    });
    if (this.isSearch) {
      this.tempSelectedData = [...this.selectionTable];
    } else {
      this.selectedTableData = [...this.selectionTable];
    }
    this.disableBtn();
  }

  ngModelChange(e, item) {
    let overlength = find(e, item => item.length > 1024);
    let overAllLength = e.length > 256;
    let prefixLetterValid = false;
    let prefixHeadValid = false;
    let prefixNearValid = false;
    const reg1 = /[|:*?<>"\\]+/;
    if (find(e, item => reg1.test(item))) {
      prefixLetterValid = true;
    }
    if (find(e, item => item.startsWith('/'))) {
      prefixHeadValid = true;
    }
    if (find(e, item => item.indexOf('//') !== -1)) {
      prefixNearValid = true;
    }
    if (overlength || overAllLength) {
      item.error = true;
      item.errorTip = this.i18n.get(
        'protection_object_bucket_prefix_info_label'
      );
    } else if (prefixLetterValid) {
      item.error = true;
      item.errorTip = this.i18n.get(
        'protection_object_bucket_prefix_letter_tip_label'
      );
    } else if (prefixNearValid) {
      item.error = true;
      item.errorTip = this.i18n.get(
        'protection_object_bucket_prefix_near_tip_label'
      );
    } else if (prefixHeadValid) {
      item.error = true;
      item.errorTip = this.i18n.get(
        'protection_object_bucket_prefix_head_tip_label'
      );
    } else {
      item.error = false;
    }
    this.disableBtn();
  }

  searchObjectName(e) {
    if (e !== '') {
      this.objectPopover.hide();
    }
    this.displayLeftData = [];
    if (e === '') {
      this.tableData = this.tempTableData;
    }
    each(this.tempTableData, item => {
      if (item.name.toLowerCase().indexOf(e.toLowerCase()) !== -1) {
        this.displayLeftData.push({
          ...item
        });
      }
    });
    this.tableData = this.displayLeftData;
    this.totalTable = size(this.tableData);
  }

  searchSelectedObjectName(e) {
    if (e !== '') {
      this.isSearch = true;
      this.selectedObjectPopover.hide();
    }
    this.displayRightData = [];
    if (e === '') {
      this.selectedTableData = this.selectionTable;
      this.isSearch = false;
    }
    each(this.selectionTable, item => {
      if (item.name.toLowerCase().indexOf(e.toLowerCase()) !== -1) {
        this.displayRightData.push({
          ...item
        });
      }
    });
    this.selectedTableData = this.displayRightData;
    this.totalTable = size(this.tableData);
  }

  getParams() {
    const parentName = find(
      this.objectOptions,
      item => item.uuid === this.formGroup.value.objectStorage
    ).name;
    const bucketList = map(this.selectedTableData, item => {
      return {
        name: item.name,
        prefix: find(this.filters, tmp => tmp.name === item.name)?.data
      };
    });
    const params: any = {
      name: this.formGroup.value.name,
      type: 'ObjectStorage',
      subType: DataMap.Resource_Type.ObjectSet.value,
      extendInfo: {
        bucketList: JSON.stringify(bucketList),
        storageType: find(this.objectOptions, {
          uuid: this.formGroup.value.objectStorage
        })?.extendInfo?.storageType
      },
      parentUuid: this.formGroup.value.objectStorage,
      parentName: parentName
    };
    return params;
  }

  checkFilter() {
    let rightFilter = false;
    each(this.filters, item => {
      if (item?.error === true) {
        rightFilter = true;
      }
    });
    return rightFilter;
  }

  disableBtn() {
    this.modal.getInstance().lvOkDisabled =
      this.formGroup.invalid || this.checkFilter();
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.rowData) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.rowData.uuid,
            UpdateResourceRequestBody: params
          })
          .subscribe({
            next: res => {
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      } else {
        this.protectedResourceApiService
          .CreateResource({
            CreateResourceRequestBody: params
          })
          .subscribe({
            next: res => {
              cacheGuideResource(res);
              observer.next();
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

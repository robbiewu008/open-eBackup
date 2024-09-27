import { Component, OnInit, ViewChild } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef, MessageService } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  PolicyType,
  PolicyAction,
  LANGUAGE,
  FilesetTemplatesApiService,
  ProtectedResourceApiService,
  ProjectedObjectApiService,
  MODAL_COMMON
} from 'app/shared';
import { SelectSlaComponent } from 'app/shared/components/protect/select-sla/select-sla.component';
import { SystemTimeService } from 'app/shared/services/system-time.service';
import {
  assign,
  map,
  isArray,
  each,
  includes,
  size,
  reject,
  trim,
  cloneDeep,
  find,
  get
} from 'lodash';
import { combineLatest, Observable, Observer } from 'rxjs';
import { CreateFilesetComponent } from '../../create-fileset/create-fileset.component';
import { AdvancedParameterComponent } from '../../advanced-parameter/advanced-parameter.component';
import { DatePipe } from '@angular/common';
import { BackupMessageService } from 'app/shared/services/backup-message.service';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { BatchResultsComponent } from './batch-results/batch-results.component';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
@Component({
  selector: 'aui-create-fileset-template',
  templateUrl: './create-fileset-template.component.html',
  styleUrls: ['./create-fileset-template.component.less'],
  providers: [DatePipe]
})
export class CreateFilesetTemplateComponent implements OnInit {
  rowItem;
  queryName;
  formGroup: FormGroup;
  isEn = this.i18n.language === LANGUAGE.EN;
  hostOptions = [];
  templateOptions = [];
  createFilesetMode = DataMap.Create_Fileset_Mode;
  resourceType = DataMap.Resource_Type;
  activeIndex = 0;
  total = 0;
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  totalHostData = [];
  selectionHost = [];
  selectionHostClone = [];
  osType;
  columns = [
    {
      label: this.i18n.get('protection_host_path_label'),
      key: 'name'
    }
  ];

  hostErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.baseUtilService.requiredLabel
  };

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_valid_name_label')
  };

  @ViewChild(SelectSlaComponent, { static: false })
  slaComponent: SelectSlaComponent;
  @ViewChild(CreateFilesetComponent, { static: false })
  createFilesetComponent: CreateFilesetComponent;
  @ViewChild('advanced', { static: false })
  parameterComponent: AdvancedParameterComponent;
  @ViewChild('pageT', { static: false }) pageT;
  @ViewChild('pageS', { static: false }) pageS;
  @ViewChild('namePopover', { static: false }) namePopover;

  constructor(
    private fb: FormBuilder,
    public i18n: I18NService,
    private modalRef: ModalRef,
    private datePipe: DatePipe,
    private drawModalService: DrawModalService,
    private messageService: MessageService,
    public baseUtilService: BaseUtilService,
    private systemTimeService: SystemTimeService,
    private batchOperateService: BatchOperateService,
    private backupMessageService: BackupMessageService,
    private filesetTemplatesApiService: FilesetTemplatesApiService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private projectedObjectApiService: ProjectedObjectApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.updateData();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.resource, item.uuid)
    );
  }

  showHostGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.environment?.endpoint)
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      mode: new FormControl(''),
      name: new FormControl(''),
      template_id: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      sla: new FormControl(false),
      sla_id: new FormControl('')
    });

    this.listenGroup();
  }

  updateData() {
    setTimeout(() => {
      this.formGroup.patchValue({
        mode: this.rowItem
          ? this.rowItem.extendInfo?.templateId
            ? this.createFilesetMode.applicationTemplate.value
            : this.createFilesetMode.manual.value
          : this.createFilesetMode.manual.value,
        name: this.rowItem ? this.rowItem.name || '' : '',
        template_id: this.rowItem
          ? this.rowItem.extendInfo?.templateId || ''
          : ''
      });
    }, 5);
  }

  removeSelection(item) {
    this.selectionHost = reject(this.selectionHost, value => {
      return value.uuid === item.uuid;
    });
    this.selectionHostClone = reject(this.selectionHostClone, value => {
      return value.uuid === item.uuid;
    });
    this.selectionChange(this.selectionHost);
  }

  resetSelection() {
    this.selectionHost = [];
    this.selectionHostClone = [];
    this.selectionChange([]);
  }

  selectionChange(e) {
    this.selectionHostClone = cloneDeep(this.selectionHost);
    this.enableBtnFn();

    if (size(this.selectionHost) > 64) {
      this.messageService.error(
        this.i18n.get('protection_max_fileset_host_label'),
        {
          lvMessageKey: 'hostErrorKey',
          lvShowCloseButton: true
        }
      );
    }
  }

  vaildPolicy() {
    if (
      find(
        get(this.slaComponent.resourceData, 'slaObject.policy_list'),
        item => item.action === PolicyAction.PERMANENT
      )
    ) {
      if (!!this.parameterComponent.formGroup.value.smallFile) {
        this.parameterComponent.formGroup.get('smallFile').setValue(false);
      }
      this.parameterComponent.disableSmallFile = true;
    } else if (
      find(
        get(this.slaComponent.resourceData, 'slaObject.policy_list'),
        item => item.action === PolicyAction.INCREMENT
      )
    ) {
      if (!this.parameterComponent.formGroup.value.smallFile) {
        this.parameterComponent.formGroup.get('smallFile').setValue(true);
      }
      this.parameterComponent.disableSmallFile = true;
    } else {
      this.parameterComponent.disableSmallFile = false;
    }
  }

  enableBtnFn() {
    this.modalRef.getInstance().lvOkDisabled =
      !size(this.selectionHost) ||
      size(this.selectionHost) > 64 ||
      this.formGroup.invalid ||
      this.parameterComponent.formGroup.invalid ||
      (this.parameterComponent.formGroup.value.smallFile &&
        this.parameterComponent.formGroup.value.fileSize <
          this.parameterComponent.formGroup.value.maxFileSize) ||
      (this.osType === DataMap.Fileset_Template_Os_Type.windows.value &&
        get(
          find(
            get(this.slaComponent, 'resourceData.slaObject.policy_list', []),
            {
              type: PolicyType.BACKUP
            }
          ),
          'ext_parameters.source_deduplication'
        ));
  }

  listenGroup() {
    this.formGroup.get('sla').valueChanges.subscribe(res => {
      if (res) {
        this.getSlaList();
        this.parameterComponent.valid$.subscribe(res => {
          this.enableBtnFn();
        });
        this.slaComponent.valid$.subscribe(isOk => {
          this.vaildPolicy();
          if (isOk) {
            this.formGroup
              .get('sla_id')
              .setValue(this.slaComponent.resourceData.sla_id);
          } else {
            this.formGroup.get('sla_id').setValue('');
          }
          this.enableBtnFn();
        });
        this.formGroup
          .get('sla_id')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('sla_id').updateValueAndValidity();
      } else {
        this.formGroup.get('sla_id').setValue('');
        this.formGroup.get('sla_id').clearValidators();
        this.formGroup.get('sla_id').updateValueAndValidity();
      }
      this.enableBtnFn();
    });

    this.formGroup.get('mode').valueChanges.subscribe(res => {
      if (res === this.createFilesetMode.applicationTemplate.value) {
        this.getTemplates();
        this.formGroup.statusChanges.subscribe(result => {
          if (
            this.formGroup.value.mode !==
            this.createFilesetMode.applicationTemplate.value
          ) {
            return;
          }

          this.modalRef.getInstance().lvOkDisabled = this.rowItem
            ? !(
                result === 'VALID' &&
                this.formGroup.value.mode ===
                  this.createFilesetMode.applicationTemplate.value
              )
            : !(
                result === 'VALID' &&
                !!size(this.selectionHost) &&
                this.formGroup.value.mode ===
                  this.createFilesetMode.applicationTemplate.value
              );
        });
        if (this.rowItem) {
          this.formGroup
            .get('name')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.name(CommonConsts.REGEX.name)
            ]);
          this.formGroup.get('name').updateValueAndValidity();
        }
      } else if (res === this.createFilesetMode.manual.value) {
        this.formGroup.get('name').clearValidators();
        this.formGroup.get('name').updateValueAndValidity();
        const combined: any = combineLatest([
          this.createFilesetComponent.formGroup.statusChanges,
          this.createFilesetComponent.fileValid$
        ]);
        combined.subscribe(latestValues => {
          if (
            this.formGroup.value.mode !== this.createFilesetMode.manual.value
          ) {
            return;
          }
          const [formGroupStatus, fileValid] = latestValues;
          this.modalRef.getInstance().lvOkDisabled = !(
            formGroupStatus === 'VALID' &&
            fileValid &&
            this.formGroup.value.mode === this.createFilesetMode.manual.value
          );
        });
        this.createFilesetComponent.formGroup
          .get('name')
          .updateValueAndValidity();
        this.createFilesetComponent.validPath();
      }
    });

    this.formGroup.get('template_id').valueChanges.subscribe(res => {
      if (!res || !!this.rowItem) {
        return;
      }
      const currentOsType = find(
        this.templateOptions,
        item => item.uuid === res
      ).osType;

      if (currentOsType !== this.osType) {
        this.formGroup.get('sla').setValue(false);
      }
      this.osType = currentOsType;
      this.parameterComponent.updateForm(this.osType);

      this.getHosts(res);
      this.selectionHost = [];
      this.selectionHostClone = [];
    });
  }

  getSlaList() {
    this.slaComponent.initData(
      this.rowItem || {
        sub_type: DataMap.Resource_Type.fileset.value,
        subType: DataMap.Resource_Type.fileset.value,
        environment: {
          osType: this.osType
        }
      }
    );
  }

  getAdvanceParamter() {
    return this.parameterComponent.getParams();
  }

  getHosts(template_id, hostName?) {
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize
    };
    const filterParams = {
      subType: ['FilesetPlugin'],
      environment: {
        osType: [
          this.templateOptions.find(item => item.uuid === template_id)?.osType
        ],
        linkStatus: [DataMap.resource_LinkStatus_Special.normal.value]
      }
    };
    if (!!trim(hostName)) {
      assign(filterParams.environment, {
        name: [['~~'], trim(hostName)]
      });
    }
    this.protectedResourceApiService
      .ListResources({
        ...params,
        conditions: JSON.stringify(filterParams)
      })
      .subscribe(res => {
        this.totalHostData = res.records;
        this.total = res.totalCount;
      });
  }

  pageChange(page) {
    this.pageIndex = page.pageIndex;
    this.pageSize = page.pageSize;
    this.getHosts(this.formGroup.value.template_id);
  }

  searchByName(hostName) {
    if (this.namePopover) {
      this.namePopover.hide();
    }
    this.getHosts(this.formGroup.value.template_id, hostName);
    this.queryName = '';
  }

  getTemplates() {
    const params = {
      pageNo: CommonConsts.PAGE_START + 1,
      pageSize: 200
    };

    if (this.rowItem) {
      assign(params, {
        conditions: JSON.stringify({
          osType: [this.rowItem.environment?.osType]
        })
      });
    }

    this.filesetTemplatesApiService
      .listUsingGET({ ...params })
      .subscribe(res => {
        this.templateOptions = map(res.records, item => {
          return assign(item, {
            isLeaf: true,
            label: item.name
          });
        });
      });
  }

  create(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.value.mode === this.createFilesetMode.manual.value) {
        if (this.createFilesetComponent.formGroup.invalid) {
          observer.next();
          observer.complete();
          return;
        }
        this.createFilesetComponent.createFileset().subscribe({
          next: res => {
            observer.next(res);
            observer.complete();
          },
          error: err => {
            observer.error(err);
            observer.complete();
          }
        });
      } else if (
        this.formGroup.value.mode ===
        this.createFilesetMode.applicationTemplate.value
      ) {
        if (this.formGroup.invalid) {
          observer.next();
          observer.complete();
          return;
        }

        const body = {
          name: '0000',
          type: DataMap.Resource_Type.fileset.value,
          subType: DataMap.Resource_Type.fileset.value,
          extendInfo: {
            templateId: this.formGroup.value.template_id
          }
        };

        let postAction;
        if (this.formGroup.value.sla) {
          this.systemTimeService.getSystemTime().subscribe(sysTime => {
            const slaObject = this.slaComponent.resourceData['slaObject'] || {};
            const startTimeArr = [];
            if (isArray(slaObject.policy_list)) {
              each(slaObject.policy_list, item => {
                if (
                  item &&
                  item.type === PolicyType.BACKUP &&
                  includes(
                    [
                      PolicyAction.FULL,
                      PolicyAction.DIFFERENCE,
                      PolicyAction.INCREMENT
                    ],
                    item.action
                  ) &&
                  item.schedule &&
                  item.schedule.start_time
                ) {
                  startTimeArr.push(
                    Date.parse(
                      this.datePipe.transform(
                        item.schedule.start_time,
                        'yyyy/MM/dd HH:mm:ss'
                      )
                    )
                  );
                }
              });
            }
            const startTime: number =
              startTimeArr.length > 0
                ? Math.min.apply(null, startTimeArr)
                : undefined;
            const systemTime: number = sysTime
              ? Date.parse(sysTime.time.replace(/-/g, '/'))
              : undefined;
            if (startTime < systemTime) {
              this.backupMessageService.create({
                content: this.i18n.get('protection_protect_late_tip_label'),
                onOK: modal => {
                  const component = modal.getContentComponent();
                  postAction = component.status;
                  this.createFileset(body, postAction, observer);
                },
                onCancel: modal => {
                  this.createFileset(body, postAction, observer);
                }
              });
            } else {
              this.createFileset(body, postAction, observer);
            }
          });
        } else {
          this.createFileset(body, postAction, observer);
        }
      }
    });
  }

  createFileset(body, postAction: {}, observer: Observer<void>) {
    this.drawModalService.create({
      lvModalKey: 'batch',
      lvWidth: MODAL_COMMON.normalWidth + 100,
      lvHeader: this.i18n.get('common_execute_result_label'),
      lvContent: BatchResultsComponent,
      lvFooter: [
        {
          id: 'close',
          label: this.i18n.get('common_close_label'),
          onClick: (modal, button) => {
            modal.close();
          }
        }
      ],
      lvAfterClose: res => {
        observer.next();
        observer.complete();
      },
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as BatchResultsComponent;
        content.valid$.subscribe(res => {
          modal.lvOkDisabled = !res;
        });
      },
      lvComponentParams: {
        advanced: this.getAdvanceParamter(),
        body,
        selectionHost: this.selectionHost,
        formGroup: this.formGroup,
        templateOptions: this.templateOptions,
        postaction: postAction
      }
    });
  }

  modify(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.value.mode === this.createFilesetMode.manual.value) {
        if (this.createFilesetComponent.formGroup.invalid) {
          observer.next();
          observer.complete();
          return;
        }
        this.createFilesetComponent.modifyFileset().subscribe(
          res => {
            observer.next(res);
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
      } else if (
        this.formGroup.value.mode ===
        this.createFilesetMode.applicationTemplate.value
      ) {
        if (this.formGroup.invalid) {
          observer.next();
          observer.complete();
          return;
        }

        const body = {
          name: this.formGroup.value.name,
          type: DataMap.Resource_Type.fileset.value,
          subType: DataMap.Resource_Type.fileset.value,
          extendInfo: {
            templateId: this.formGroup.value.template_id
          }
        };
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.rowItem.uuid,
            UpdateResourceRequestBody: body
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
      }
    });
  }

  onOK(): Observable<void> {
    return !this.rowItem ? this.create() : this.modify();
  }
}

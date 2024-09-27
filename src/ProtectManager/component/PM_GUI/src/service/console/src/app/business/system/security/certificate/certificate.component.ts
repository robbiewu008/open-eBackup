import {
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { DatatableComponent } from '@iux/live';
import {
  BackupClustersApiService,
  CommonConsts,
  ComponentRestApiService,
  CookieService,
  DataMap,
  DataMapService,
  getPermissionMenuItem,
  I18NService,
  LANGUAGE,
  MODAL_COMMON,
  OperateItems,
  WarningMessageService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  each,
  extend,
  find,
  get,
  includes,
  isEmpty,
  mapValues,
  now
} from 'lodash';
import { combineLatest } from 'rxjs';
import { AddComponentsComponent } from './add-components/add-components.component';
import { CertificateDetailComponent } from './certificate-detail/certificate-detail.component';
import { ExportRequestFileComponent } from './export-request-file/export-request-file.component';
import { ImportCertificateComponent } from './import-certificate/import-certificate.component';
import { ImportRevocationListComponent } from './import-revocation-list/import-revocation-list.component';
import { ModifyComponent } from './modify/modify.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-certificate',
  templateUrl: './certificate.component.html',
  styleUrls: ['./certificate.component.less']
})
export class CertificateComponent implements OnInit {
  updateFlag = false;
  certificateData: any[];
  certificateSelection: any[];
  componentName: string;
  optOptions;
  COMPONENT_TYPE = this.dataMapService.getConfig('Component_Type');
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  _includes = includes;
  @ViewChild(DatatableComponent, { static: false }) lvTable: DatatableComponent;
  @ViewChild('headerTpl', { static: false }) headerTpl: TemplateRef<any>;
  certificateLabel = this.i18n.get('system_certificate_label');
  exportFileLable = this.i18n.get('system_export_request_file_label');
  importCertificateLabel = this.i18n.get('system_import_certificate_label');
  importRevocationLabel = this.i18n.get('system_import_revocation_list_label');
  modifyLabel = this.i18n.get('system_modify_alarm_days_label');
  componentLabel = this.i18n.get('common_name_label');
  componentTypeLabel = this.i18n.get('common_type_label');
  certificateStatusLabel = this.i18n.get('common_certificate_status_label');
  expirationTimeLabel = this.i18n.get('common_expriration_time_label');
  operationLabel = this.i18n.get('common_operation_label');
  clientNameLabel = this.i18n.get('system_client_name_label');
  areaLabel = this.i18n.get('system_area_label');
  descriptionLabel = this.i18n.get('common_desc_label');
  dependencyTypeLabel = this.i18n.get('system_dependency_type_label');
  updateTimeLabel = this.i18n.get('system_update_time_label');
  enabledStatusLabel = this.i18n.get('system_enable_status_label');
  addExternalComponentLable = this.i18n.get(
    'system_add_external_component_label'
  );
  daysLabel =
    this.i18n.language === 'zh-cn'
      ? ''
      : ' ' + this.i18n.get('common_days_label');
  expiredLabel = this.i18n.get('common_expired_label');
  daysOfWarningLabel = this.i18n.get('system_certificate_early_day_label');
  closeLabel = this.i18n.get('common_close_label');
  deleteLabel = this.i18n.get('common_delete_label');
  downloadCaCertificateLabel = this.i18n.get(
    'system_download_ca_certificate_label'
  );
  leftLabel = this.i18n.get('common_left_label');
  moreThanLabel = this.i18n.get('common_more_than_label');
  expiredDateLabel =
    this.i18n.get('system_component_expiration_time_label') + ' ';
  importInteralCerLabel = this.i18n.get(
    'system_import_internal_certificate_label'
  );
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  currentItem: any;
  isDataBackup = includes(
    [
      DataMap.Deploy_Type.a8000.value,
      DataMap.Deploy_Type.x3000.value,
      DataMap.Deploy_Type.x6000.value,
      DataMap.Deploy_Type.x8000.value,
      DataMap.Deploy_Type.x9000.value
    ],
    this.i18n.get('deploy_type')
  );

  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  constructor(
    public i18n: I18NService,
    public modalService: DrawModalService,
    public certApiService: ComponentRestApiService,
    public dataMapService: DataMapService,
    public warningMessageService: WarningMessageService,
    public backupClustersApiService: BackupClustersApiService,
    private cookieService: CookieService,
    private cdr: ChangeDetectorRef,
    private appUtilsService: AppUtilsService
  ) {}

  addExternalComponent() {
    this.modalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'addExternalComponent',
        lvHeader: this.addExternalComponentLable,
        lvContent: AddComponentsComponent,
        lvWidth: 600,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent();
          const modalIns = modal.getInstance();
          const combined: any = combineLatest(
            content.formGroup.statusChanges,
            content.validCer$
          );
          combined.subscribe(latestValues => {
            const [formGroupStatus, validCer] = latestValues;
            modalIns.lvOkDisabled = !(formGroupStatus === 'VALID' && validCer);
          });
        },
        lvOk: modal => {
          const content = modal.getContentComponent();
          content.registerComponent(() => {
            this.getCertificates();
          });
        }
      })
    );
  }

  downloadRequestFile(data, modal) {
    const content = modal.getContentComponent();
    // 保存文件
    const saveFn = blob => {
      const bf = new Blob([blob], {
        type: 'application/zip'
      });
      this.appUtilsService.downloadFile(`requestFile_${now()}.csr`, bf);
    };
    const params = {
      componentId: data.componentId,
      algorithm: content.formGroup.value.keyAlgorithm
    };
    // 区分OP和OC
    if (this.isCyberEngine) {
      each(
        [
          'country',
          'state',
          'city',
          'organization',
          'organizationUnit',
          'commonName'
        ],
        key => {
          if (content.formGroup.get(key)?.value) {
            assign(params, {
              [key]: content.formGroup.get(key)?.value
            });
          }
        }
      );
    }
    this.certApiService
      .exportCertificateRequestUsingGET(params)
      .subscribe(blob => saveFn(blob));
  }

  exportRequestFile(data) {
    this.modalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'exportRequestFile',
        lvHeader: this.exportFileLable,
        lvContent: ExportRequestFileComponent,
        lvWidth: 600,
        lvOkDisabled: false,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as ExportRequestFileComponent;
          const modalIns = modal.getInstance();
          if (this.isCyberEngine) {
            content.formGroup.statusChanges.subscribe(res => {
              modalIns.lvOkDisabled = res === 'INVALID';
            });
          }
        },
        lvOk: modal => this.downloadRequestFile(data, modal)
      })
    );
  }

  importCertificate(data, detailFlag?) {
    this.modalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'importCertificate',
        lvHeader: includes(
          [
            DataMap.Component_Type.internal.value,
            DataMap.Component_Type.communicationComponent.value,
            DataMap.Component_Type.redisComponent.value
          ],
          data.type
        )
          ? this.importInteralCerLabel
          : this.importCertificateLabel,
        lvWidth:
          this.i18n.language === LANGUAGE.EN &&
          includes(
            [
              DataMap.Component_Type.internal.value,
              DataMap.Component_Type.communicationComponent.value,
              DataMap.Component_Type.redisComponent.value,
              DataMap.Component_Type.protectAgent.value
            ],
            data.type
          )
            ? 650
            : 600,
        lvContent: ImportCertificateComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          currentComponent: data
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as ImportCertificateComponent;
          const modalIns = modal.getInstance();
          if (
            !includes(
              [
                DataMap.Component_Type.internal.value,
                DataMap.Component_Type.protectAgent.value,
                DataMap.Component_Type.communicationComponent.value,
                DataMap.Component_Type.redisComponent.value
              ],
              data.type
            )
          ) {
            content.validCertificate$.subscribe(res => {
              modalIns.lvOkDisabled = !res;
            });
            return;
          }

          if (
            data.type === DataMap.Component_Type.communicationComponent.value ||
            data.type === DataMap.Component_Type.redisComponent.value
          ) {
            const combined: any = combineLatest(
              content.formGroup.statusChanges,
              content.validCertificate$,
              content.valid$
            );
            combined.subscribe(latestValues => {
              const [formGroupStatus, validCertificate, valid] = latestValues;
              modalIns.lvOkDisabled =
                !validCertificate || !valid || formGroupStatus !== 'VALID';
            });
            content.formGroup.updateValueAndValidity();
            return;
          }

          if (this.cookieService.isCloudBackup) {
            const combined: any = combineLatest(
              content.formGroup.statusChanges,
              content.validCertificate$,
              content.valid$,
              content.validDhparam$
            );
            combined.subscribe(latestValues => {
              const [
                formGroupStatus,
                validCertificate,
                valid,
                validDhparam
              ] = latestValues;
              modalIns.lvOkDisabled =
                !validCertificate ||
                !valid ||
                !validDhparam ||
                formGroupStatus !== 'VALID';
            });
          } else {
            if (data.type === DataMap.Component_Type.protectAgent.value) {
              const combined: any = combineLatest(
                content.formGroup.statusChanges,
                content.validCertificate$,
                content.valid$,
                content.privateKey$
              );
              combined.subscribe(latestValues => {
                const [
                  formGroupStatus,
                  validCertificate,
                  valid,
                  privateKey
                ] = latestValues;
                modalIns.lvOkDisabled =
                  !validCertificate ||
                  !valid ||
                  !privateKey ||
                  formGroupStatus !== 'VALID';
              });
            } else {
              const combined: any = combineLatest(
                content.formGroup.statusChanges,
                content.validCertificate$,
                content.valid$
              );
              combined.subscribe(latestValues => {
                const [formGroupStatus, validCertificate, valid] = latestValues;
                modalIns.lvOkDisabled =
                  !validCertificate || !valid || formGroupStatus !== 'VALID';
              });
            }
          }
          content.formGroup.updateValueAndValidity();
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as ImportCertificateComponent;
            content.importCertificate(() => {
              this.getCertificates(data, detailFlag);
            }, resolve);
          });
        }
      })
    );
  }

  importRevocation(data, detailFlag?) {
    this.modalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'importRevocation',
        lvHeader: this.importRevocationLabel,
        lvWidth: 600,
        lvContent: ImportRevocationListComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          currentComponent: data
        },
        lvAfterOpen: modal => {
          const content = modal.getContentComponent();
          const modalIns = modal.getInstance();
          content.valid$.subscribe(res => {
            modalIns.lvOkDisabled = !res;
          });
        },
        lvOk: modal => {
          const content = modal.getContentComponent();
          content.importRevocation(() => {
            this.getCertificates(data, detailFlag);
          });
        }
      })
    );
  }

  modify(data, detailFlag?) {
    this.modalService.create(
      assign({}, MODAL_COMMON.drawerOptions, {
        lvModalKey: 'modify',
        lvHeader: this.modifyLabel,
        lvWidth: 600,
        lvContent: ModifyComponent,
        lvComponentParams: {
          currentComponent: data
        },
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent();
          const modalIns = modal.getInstance();
          content.formGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          const content = modal.getContentComponent();
          content.modify(() => {
            this.getCertificates(data, detailFlag);
          });
        }
      })
    );
  }

  getDetail(item) {
    if (isEmpty(item)) {
      return;
    }
    this.currentItem = item;
    this.componentName = item.name;
    this.optOptions =
      item.type === this.COMPONENT_TYPE.internal.value ||
      item.type === this.COMPONENT_TYPE.communicationComponent.value ||
      item.type === this.COMPONENT_TYPE.protectAgent.value ||
      item.type === this.COMPONENT_TYPE.redisComponent.value ||
      item.type === this.COMPONENT_TYPE.ha.value
        ? this.internalOptsCallback(item, true)
        : this.externalOptsCallback(item, true);
    const modalParams = assign({}, MODAL_COMMON.drawerOptions, {
      lvModalKey: 'certificate-details',
      lvHeader: this.headerTpl,
      lvWidth: 700,
      lvContent: CertificateDetailComponent,
      lvFooter: [
        {
          label: this.closeLabel,
          onClick: (modal, button) => {
            this.getCertificates();
            modal.close();
          }
        }
      ],
      lvComponentParams: {
        currentComponent: assign({}, item, {
          refreshDetail: () => {
            this.getCertificates(item, true);
          }
        })
      }
    });
    if (
      includes(
        mapValues(this.modalService.modals, 'key'),
        'certificate-details'
      )
    ) {
      this.modalService.update('certificate-details', modalParams);
    } else {
      this.modalService.create(modalParams);
    }
  }

  internalOptsCallback = (data, detailFlag?) => {
    const menus = [
      {
        id: 'exportRequestFile',
        label: this.exportFileLable,
        disabled: data.validity === DataMap.Certificate_Status.replacing.value,
        permission: OperateItems.ExportingRequestFile,
        hidden: includes(
          [
            DataMap.Component_Type.protectAgent.value,
            DataMap.Component_Type.ha.value
          ],
          data.type
        ),
        onClick: (d: any) => {
          this.exportRequestFile(data);
        }
      },
      {
        id: 'importCertificate',
        label: this.importInteralCerLabel,
        disabled: data.validity === DataMap.Certificate_Status.replacing.value,
        hidden: includes([DataMap.Component_Type.ha.value], data.type),
        permission: OperateItems.ImportingInternalComponentCertificates,
        onClick: (d: any) => {
          this.importCertificate(data, detailFlag);
        }
      },
      {
        id: 'downloadCaCertificate',
        label: this.downloadCaCertificateLabel,
        disabled: data.validity === DataMap.Certificate_Status.replacing.value,
        permission: OperateItems.DownloadingCertificate,
        onClick: (d: any) => {
          this.certApiService
            .downloadCaUsingGET({
              componentId: data.componentId
            })
            .subscribe(blob => {
              const bf = new Blob([blob], {
                type: 'application/octet-stream'
              });
              this.appUtilsService.downloadFile(
                `caCertificate_${now()}.pem`,
                bf
              );
            });
        }
      },
      // 当证书类型为内部通信或内部数据库证书时，且证书到期剩余天数小于过期告警天数时，露出重新生成证书按钮
      {
        id: 'regenerateCaCertificate',
        label: this.i18n.get('system_regenerate_certificate_label'),
        hidden: !(
          includes(
            [
              DataMap.Component_Type.communicationComponent.value,
              DataMap.Component_Type.redisComponent.value
            ],
            data.type
          ) && data.remainingDays < data.expirationWarningDays
        ),
        permission: OperateItems.RegenerateCertificate,
        onClick: (d: any) => {
          this.warningMessageService.create({
            content: this.i18n.get('system_regenerate_certificate_warn_label'),
            onOK: () => {
              this.certApiService
                .regenerateInternalCert({
                  componentId: data.componentId
                })
                .subscribe(res => {
                  this.getCertificates();
                });
            }
          });
        }
      },
      {
        id: 'modify',
        label: this.modifyLabel,
        disabled: data.validity === DataMap.Certificate_Status.replacing.value,
        permission: OperateItems.ChangingCertificateAlarmThreshold,
        onClick: (d: any) => {
          this.modify(data, detailFlag);
        }
      },
      {
        id: 'update',
        label: this.i18n.get('common_update_label'),
        hidden:
          data.type !== DataMap.Component_Type.ha.value || this.updateFlag,
        permission: OperateItems.UpdateHACertificate,
        onClick: (d: any) => {
          this.warningMessageService.create({
            content: this.i18n.get('system_upgrate_ha_certificate_tips_label'),
            onOK: () => {
              this.certApiService
                .updateHaCertUsingPOST({
                  componentId: data.componentId
                })
                .subscribe(res => {
                  this.getCertificates();
                });
            }
          });
        }
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  getShowOpts(data) {
    const menu = this.externalOptsCallback(data);
    return menu?.filter(item => item.hidden === false);
  }

  externalOptsCallback = (data, detailFlag?) => {
    const menus = [
      {
        id: 'importCertificate',
        label: this.importCertificateLabel,
        permission: OperateItems.ImportingExternalComponentCertificates,
        onClick: (d: any) => {
          this.importCertificate(data, detailFlag);
        }
      },
      {
        id: 'importRevocation',
        label: this.importRevocationLabel,
        hidden: true,
        permission: OperateItems.ImportingRevocationList,
        onClick: (d: any) => {
          this.importRevocation(data, detailFlag);
        }
      },
      {
        id: 'downloadCaCertificate',
        label: this.downloadCaCertificateLabel,
        permission: OperateItems.DownloadingCertificate,
        onClick: (d: any) => {
          this.certApiService
            .downloadCaUsingGET({
              componentId: data.componentId
            })
            .subscribe(blob => {
              const bf = new Blob([blob], {
                type: 'application/octet-stream'
              });
              this.appUtilsService.downloadFile(
                `caCertificate_${now()}.pem`,
                bf
              );
            });
        }
      },
      {
        id: 'modify',
        label: this.modifyLabel,
        permission: OperateItems.ChangingCertificateAlarmThreshold,
        onClick: (d: any) => {
          this.modify(data, detailFlag);
        }
      },
      {
        id: 'delete',
        label: this.deleteLabel,
        permission: OperateItems.DeletingCertificate,
        hidden:
          this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value
            ? false
            : data.name === 'ExternalComponent',
        onClick: (d: any) => {
          this.warningMessageService.create({
            content: this.i18n.get('system_delete_component_warn_label', [
              data.name
            ]),
            onOK: () => {
              this.certApiService
                .deleteComponentUsingDELETE({
                  componentId: data.componentId,
                  sync: true
                })
                .subscribe(res => {
                  this.getCertificates();
                  if (
                    includes(
                      mapValues(this.modalService.modals, 'key'),
                      'certificate-details'
                    )
                  ) {
                    this.modalService.destroyModal('certificate-details');
                  }
                });
            }
          });
        }
      }
    ];
    return getPermissionMenuItem(menus, this.cookieService.role);
  };

  refresh() {
    if (this.isDataBackup) {
      this.getHaInfo();
    } else {
      this.getCertificates();
    }
  }

  getCertificates(data?, detailFlag?) {
    this.certApiService.queryComponentsUsingGET({}).subscribe(res => {
      each(res, (item: any) => {
        const certArr = [];
        each(item.certificateDetails, cer => {
          certArr.push(cer.expirationTime);
        });
        const typeNames = [];
        each(item.type.split(','), value => {
          typeNames.push(this.dataMapService.getLabel('Component_Type', value));
        });
        extend(item, {
          expiredDate: Math.min.apply(null, certArr),
          typeNames: typeNames.join(', ')
        });
      });
      this.certificateData = this.isHyperdetect
        ? res.filter(
            (item: any) =>
              !get(item, 'typeNameList').includes('EXTERNAL_STORAGE')
          )
        : res;
      this.total = res.length;
      if (detailFlag) {
        this.getDetail(find(res, { componentId: data.componentId }));
      }
    });
  }

  getHaInfo() {
    this.backupClustersApiService
      .queryClusterInfo(assign({}))
      .subscribe(res => {
        if (res.roleType === 'PRIMARY') {
          this.updateFlag = false;
        } else {
          this.updateFlag = true;
        }
        this.getCertificates();
      });
  }

  ngOnInit() {
    if (this.isDataBackup) {
      this.getHaInfo();
    } else {
      this.getCertificates();
    }
  }

  onChange() {
    this.ngOnInit();
  }
}

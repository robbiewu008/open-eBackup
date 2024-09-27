import { Component, OnInit, ChangeDetectorRef } from '@angular/core';
import { I18NService, DataMapService, DataMap, MODAL_COMMON } from 'app/shared';
import { each, filter, sortBy, isEmpty, includes, assign } from 'lodash';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { ComponentRestApiService } from 'app/shared/api/services';
import { ImportRevocationListComponent } from '../import-revocation-list/import-revocation-list.component';

import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-certificate-detail',
  templateUrl: './certificate-detail.component.html',
  styles: [
    `
      .tab-container {
        margin-top: 10px;
      }
      .status-badge {
        background: #7adfa0;
      }
      .status-badge-off {
        background: #b8becc;
      }
      .certificate-title > i,
      .info-container i {
        height: 10px;
        width: 10px;
        display: inline-block;
        border-radius: 50%;
        margin-right: 8px;
      }
      .details-container {
        border: 0.01rem solid #b8becc;
        padding: 20px;
        margin-top: 10px;
      }
      .title-border {
        border-bottom: 0.01rem dashed #d4d9e6;
        padding-bottom: 12px;
        margin-bottom: 12px;
      }
      .details-container .lv-form-label {
        padding-bottom: 0.06rem;
      }
      .details-container .lv-form-control {
        padding-bottom: 0.01rem;
      }
      .validity-status {
        display: inline-block;
        margin-right: 10px;
      }
      .performance-no-data {
        text-align: center;
      }
      .performance-no-data i {
        width: 240px;
        height: 240px;
      }
      .import-btn {
        margin-top: 20px;
      }
      .card-opt-btn {
        min-width: 0.4rem;
      }
    `
  ]
})
export class CertificateDetailComponent implements OnInit {
  currentComponent: any;
  clientCertificate = [];
  caCertificate = [];
  cerRevocationList = [];
  componentExpirationTime;
  wordMap = ['一', '二', '三', '四', '五', '六', '七', '八', '九', '十'];
  COMPONENT_TYPE = this.dataMapService.getConfig('Component_Type');
  componentLabel = this.i18n.get('common_name_label');
  componentTypeLabel = this.i18n.get('common_type_label');
  certificateStatusLabel = this.i18n.get('common_certificate_status_label');
  expirationTimeLabel = this.i18n.get(
    'system_certificate_expiration_time_label'
  );
  daysOfWarningLabel = this.i18n.get('system_certificate_early_day_label');
  clientCertificateLabel = this.i18n.get('system_client_certificate_label');
  caCertificateLabel = this.i18n.get('system_ca_certificate_label');
  cerRevocationListLabel = this.i18n.get('system_cer_revocation_list_label');
  importRevocationLabel = this.i18n.get('system_import_revocation_list_label');
  userLabel = this.i18n.get('system_certificate_user_label');
  issuserLabel = this.i18n.get('system_issuser_label');
  algorithmLabel = this.i18n.get('system_key_algorithm_label');
  signatureLabel = this.i18n.get('system_signature_algorithm_label');
  keyLengthLabel = this.i18n.get('system_key_length_label');
  fingerprintLabel = this.i18n.get('system_finger_print_label');
  subjectInfoLabel = this.i18n.get('system_subject_information_label');
  expireDateLabel = this.i18n.get(
    'system_certificate_expiration_time_label',
    [],
    true
  );
  componentExpiredLabel = this.i18n.get('common_expriration_time_label');
  signatureAlgLabel = this.i18n.get('system_signature_algorithm_label');
  isInternalComponent = false;
  isRevocationListShow: boolean;

  constructor(
    public i18n: I18NService,
    private cdr: ChangeDetectorRef,
    public dataMapService: DataMapService,
    private modalService: DrawModalService,
    private appUtilsService: AppUtilsService,
    public cerApiService: ComponentRestApiService
  ) {}

  ngOnInit() {
    this.initCertificate();
    this.initRevocationList();
  }

  typeChange(e) {}

  showDetail(item) {
    item.expand = !item.expand;
  }

  delete(item) {
    this.cerApiService
      .deleteCRLUsingDELETE({
        componentId: this.currentComponent.componentId,
        crlId: item.crlId,
        sync: true
      })
      .subscribe(res => {
        this.initRevocationList();
      });
  }

  download(item) {
    this.cerApiService
      .downloadCrlUsingGET({
        componentId: this.currentComponent.componentId,
        crlId: item.crlId
      })
      .subscribe(blob => {
        const bf = new Blob([blob], {
          type: 'application/octet-stream'
        });
        this.appUtilsService.downloadFile(
          `${this.currentComponent.name}.crl`,
          bf
        );
        this.cdr.detectChanges();
      });
  }

  initCertificate() {
    const hidenRevocationListNames = [
      'innerDatabase',
      'innerComponent',
      'OceanProtect',
      'DataBackup',
      'HA'
    ];
    if (
      this.i18n.get('deploy_type') !== DataMap.Deploy_Type.cyberengine.value
    ) {
      hidenRevocationListNames.push('ExternalComponent');
    }

    this.isRevocationListShow = !includes(
      hidenRevocationListNames,
      this.currentComponent.name
    );
    this.isInternalComponent = includes(
      [
        DataMap.Component_Type.internal.value,
        DataMap.Component_Type.protectAgent.value,
        DataMap.Component_Type.redisComponent.value,
        DataMap.Component_Type.communicationComponent.value,
        DataMap.Component_Type.ha.value
      ],
      this.currentComponent.type
    );
    this.caCertificate = filter(
      this.currentComponent.certificateDetails,
      item => {
        item.expand = false;
        return item.certType === 2 || item.certType === 3;
      }
    );
    this.caCertificate = sortBy(this.caCertificate, item => {
      return item.sortNum;
    });
    this.clientCertificate = filter(
      this.currentComponent.certificateDetails,
      item => {
        item.expand = false;
        return item.certType === 1;
      }
    );
    if (isEmpty(this.clientCertificate)) {
      this.clientCertificate = filter(
        this.currentComponent.certificateDetails,
        item => {
          item.expand = false;
          return item.certType === 3;
        }
      );
    }
    this.clientCertificate = sortBy(this.clientCertificate, item => {
      return item.sortNum;
    });
    const certArr = [];
    each(this.currentComponent.certificateDetails, item => {
      certArr.push(item.expirationTime);
    });
    this.componentExpirationTime = Math.min.apply(null, certArr);
  }

  initRevocationList() {
    const { componentId } = this.currentComponent;
    this.cerApiService.getCRLUsingGET({ componentId }).subscribe(res => {
      this.cerRevocationList = res;
    });
  }

  importRevocation(data) {
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
            this.initRevocationList();
          });
        }
      })
    );
  }
}

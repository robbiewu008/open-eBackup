import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  ApplicationType,
  ApplicationTypeView,
  CommonConsts,
  COPIES_ICONS,
  DataMap,
  I18NService,
  STORAGE_ICONS
} from 'app/shared';
import { BaseUtilService } from 'app/shared/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { ResourceCatalogsService } from 'app/shared/services/resource-catalogs.service';
import { each, find, includes, toString, union } from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { CookieService } from 'app/shared/services/cookie.service';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-select-application',
  templateUrl: './select-application.component.html',
  styleUrls: ['./select-application.component.less']
})
export class SelectApplicationComponent implements OnInit {
  applicationObj = {} as any;
  valid$ = new Subject();
  formGroup: FormGroup;
  applicationType = ApplicationType;
  applicationTypeView = ApplicationTypeView;
  appHostIcons = [];
  vmIcons = [];
  containerIcons = [];
  cloudIcons = [];
  bigDataIcons = [];
  copiesIcons = [];
  storageIcons = [];
  bareMetalIcons = [];
  applicationIcons = [];
  language = this.i18n.language;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isHyperdetect = includes(
    [DataMap.Deploy_Type.hyperdetect.value],
    this.i18n.get('deploy_type')
  );

  constructor(
    public fb: FormBuilder,
    public i18n: I18NService,
    public modal: ModalRef,
    public cookieService: CookieService,
    public baseUtilService: BaseUtilService,
    public appUtilsService: AppUtilsService,
    private resourceCatalogsService: ResourceCatalogsService
  ) {}

  ngOnInit() {
    this.initForm();
    this.updateForm();
  }

  updateForm() {
    this.resourceCatalogsService.getResourceCatalog().subscribe(items => {
      // 数据库
      this.appHostIcons = this.appUtilsService
        .getApplicationConfig()
        .database.filter(icon => {
          icon.id = icon.slaId;
          return includes(items, icon.slaId);
        });
      // 大数据
      this.bigDataIcons = this.appUtilsService
        .getApplicationConfig()
        .bigData.filter(icon => {
          icon.id = icon.slaId;
          return includes(items, icon.slaId);
        });
      // 虚拟化
      this.vmIcons = this.appUtilsService
        .getApplicationConfig()
        .virtualization.filter(icon => {
          icon.id = icon.slaId;
          return includes(items, icon.slaId);
        });
      // 容器
      this.containerIcons = this.appUtilsService
        .getApplicationConfig()
        .container.filter(icon => {
          icon.id = icon.slaId;
          return includes(items, icon.slaId);
        });
      // 云平台
      this.cloudIcons = this.appUtilsService
        .getApplicationConfig()
        .cloud.filter(icon => {
          icon.id = icon.slaId;
          return includes(items, icon.slaId);
        });
      // 文件服务
      if (
        includes(
          [
            DataMap.Deploy_Type.cloudbackup2.value,
            DataMap.Deploy_Type.cloudbackup.value
          ],
          this.i18n.get('deploy_type')
        )
      ) {
        this.storageIcons = STORAGE_ICONS.filter(icon =>
          includes(items, icon.id)
        );
      } else if (
        this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value
      ) {
        //
        this.storageIcons = STORAGE_ICONS.filter(icon =>
          includes(items, icon.id)
        );
      } else {
        this.storageIcons = this.appUtilsService
          .getApplicationConfig()
          .fileService.filter(icon => {
            icon.id = icon.slaId;
            return includes(items, icon.slaId);
          });
        // 由于不是独立的应用，在此新增NDMP SLA
        this.storageIcons.splice(1, 0, {
          id: ApplicationType.Ndmp,
          slaId: ApplicationType.Ndmp,
          label: this.i18n.get('protection_ndmp_protocol_label'),
          prefix: 'N',
          color: '#EBAA44'
        });
        this.storageIcons = [...this.storageIcons];
      }
      this.applicationIcons = this.appUtilsService
        .getApplicationConfig()
        .application.filter(icon => {
          icon.id = icon.slaId;
          return includes(items, icon.slaId);
        });
      // 裸机
      this.bareMetalIcons = [];
      // 复制副本
      this.copiesIcons = COPIES_ICONS.filter(icon => includes(items, icon.id));
      this.formGroup.patchValue({
        type: this.applicationObj.viewType,
        all:
          this.applicationObj.value === this.applicationType.Common
            ? this.applicationType.Common
            : '',
        appHost: find(this.appHostIcons, { id: this.applicationObj.value })
          ? this.applicationObj.value
          : '',
        vm: find(this.vmIcons, { id: this.applicationObj.value })
          ? this.applicationObj.value
          : '',
        bigData: find(this.bigDataIcons, { id: this.applicationObj.value })
          ? this.applicationObj.value
          : '',
        container: find(this.containerIcons, { id: this.applicationObj.value })
          ? this.applicationObj.value
          : '',
        storage: find(this.storageIcons, { id: this.applicationObj.value })
          ? this.applicationObj.value
          : '',
        cloud: find(this.cloudIcons, { id: this.applicationObj.value })
          ? this.applicationObj.value
          : '',
        application: find(this.applicationIcons, {
          id: this.applicationObj.value
        })
          ? this.applicationObj.value
          : '',
        bareMetal: find(this.bareMetalIcons, { id: this.applicationObj.value })
          ? this.applicationObj.value
          : '',
        copies: find(this.copiesIcons, { id: this.applicationObj.value })
          ? this.applicationObj.value
          : ''
      });
    });
  }

  clearForm(rejectKey: string) {
    const appType = [
      'appHost',
      'vm',
      'container',
      'cloud',
      'bigData',
      'storage',
      'bareMetal',
      'copies',
      'application'
    ];
    each(appType, key => {
      if (key !== rejectKey) {
        this.formGroup.get(key).setValue('');
      }
    });
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(this.applicationObj.viewType),
      all: new FormControl(''),
      appHost: new FormControl(''),
      vm: new FormControl(''),
      container: new FormControl(''),
      cloud: new FormControl(''),
      bigData: new FormControl(''),
      storage: new FormControl(''),
      bareMetal: new FormControl(''),
      application: new FormControl(''),
      copies: new FormControl('')
    });
    setTimeout(() => {
      this.modal.getInstance().lvOkDisabled = !toString(
        this.applicationObj.value
      );
    }, 0);

    this.formGroup.valueChanges.subscribe(res => {
      this.modal.getInstance().lvOkDisabled = !(
        toString(res.all) ||
        toString(res.appHost) ||
        toString(res.vm) ||
        toString(res.container) ||
        toString(res.cloud) ||
        toString(res.bigData) ||
        toString(res.storage) ||
        toString(res.bareMetal) ||
        toString(res.application) ||
        toString(res.copies)
      );
    });
    this.formGroup.get('type').valueChanges.subscribe(res => {
      this.formGroup.patchValue({
        all: '',
        appHost: '',
        vm: '',
        container: '',
        cloud: '',
        bigData: '',
        storage: '',
        bareMetal: '',
        application: '',
        copies: ''
      });
    });
    this.formGroup.get('appHost').valueChanges.subscribe(res => {
      if (!toString(res)) {
        return;
      }
      this.clearForm('appHost');
      this.formGroup.updateValueAndValidity();
    });
    this.formGroup.get('vm').valueChanges.subscribe(res => {
      if (!toString(res)) {
        return;
      }
      this.clearForm('vm');
      this.formGroup.updateValueAndValidity();
    });
    this.formGroup.get('cloud').valueChanges.subscribe(res => {
      if (!toString(res)) {
        return;
      }
      this.clearForm('cloud');
      this.formGroup.updateValueAndValidity();
    });
    this.formGroup.get('bigData').valueChanges.subscribe(res => {
      if (!toString(res)) {
        return;
      }
      this.clearForm('bigData');
      this.formGroup.updateValueAndValidity();
    });
    this.formGroup.get('container').valueChanges.subscribe(res => {
      if (!toString(res)) {
        return;
      }
      this.clearForm('container');
      this.formGroup.updateValueAndValidity();
    });
    this.formGroup.get('storage').valueChanges.subscribe(res => {
      if (!toString(res)) {
        return;
      }
      this.clearForm('storage');
      this.formGroup.updateValueAndValidity();
    });
    this.formGroup.get('bareMetal').valueChanges.subscribe(res => {
      if (!toString(res)) {
        return;
      }
      this.clearForm('bareMetal');
      this.formGroup.updateValueAndValidity();
    });
    this.formGroup.get('application').valueChanges.subscribe(res => {
      if (!toString(res)) {
        return;
      }
      this.clearForm('application');
      this.formGroup.updateValueAndValidity();
    });
    this.formGroup.get('copies').valueChanges.subscribe(res => {
      if (!toString(res)) {
        return;
      }
      this.clearForm('copies');
      this.formGroup.updateValueAndValidity();
    });
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const {
        type,
        all,
        appHost,
        vm,
        container,
        cloud,
        bigData,
        storage,
        bareMetal,
        application,
        copies
      } = this.formGroup.value;
      const result = {
        label: '',
        viewType: type,
        checkedUrl: '',
        applicationType: this.applicationType.Common
      } as any;

      if (type === this.applicationTypeView.Specified) {
        const id =
          all ||
          appHost ||
          vm ||
          container ||
          cloud ||
          bigData ||
          storage ||
          bareMetal ||
          application ||
          copies;
        const icons = union(
          this.appHostIcons,
          this.vmIcons,
          this.cloudIcons,
          this.containerIcons,
          this.bigDataIcons,
          this.storageIcons,
          this.bareMetalIcons,
          this.applicationIcons,
          this.copiesIcons
        );
        result.applicationType = id;
        result.checkedUrl = 'aui-sla-' + id;
        result.label = find(icons, {
          id
        }).label;
      }

      observer.next(result);
      observer.complete();
    });
  }

  groupColumns(apps: any[]) {
    if (apps.length > 2) {
      return ['auto', 'auto', 'auto'];
    }
    if (apps.length === 2) {
      return ['272px', '272px'];
    }
    if (apps.length === 1) {
      return ['272px'];
    }
  }

  showAppGuide(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active && USER_GUIDE_CACHE_DATA.slaType === item.id
    );
  }
}

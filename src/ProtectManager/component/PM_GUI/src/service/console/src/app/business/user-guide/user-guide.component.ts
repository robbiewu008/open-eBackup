import { Component, EventEmitter, OnInit, Output } from '@angular/core';
import { Router } from '@angular/router';
import {
  ApplicationType,
  DataMap,
  GlobalService,
  I18NService,
  SUB_APP_REFRESH_FLAG
} from 'app/shared';
import {
  USER_GUIDE_APPLICATION_CONFIG,
  USER_GUIDE_CACHE_DATA,
  USER_GUIDE_PROTECTION_STEPS,
  clearUserGuideCache
} from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  cloneDeep,
  each,
  includes,
  omit,
  reject,
  values
} from 'lodash';

@Component({
  selector: 'aui-user-guide',
  templateUrl: './user-guide.component.html',
  styleUrls: ['./user-guide.component.less']
})
export class UserGuideComponent implements OnInit {
  @Output() close = new EventEmitter();
  // 引导内容（备份、复制、归档）
  guideMap = {
    main: 'main',
    backup: 'backup',
    replication: 'replication',
    archive: 'archive'
  };
  guideSteps = [
    {
      id: this.guideMap.backup,
      title: this.i18n.get('protection_guide_backup_title_label'),
      desc: this.i18n.get('protection_guide_backup_desc_label')
    }
  ];
  applicationConfig = cloneDeep(USER_GUIDE_APPLICATION_CONFIG);
  dataMap = DataMap;

  currentGuide = this.guideMap.main;

  activeStep = 1;

  // 当前激活的资源类型
  activeAppId: string;
  activeApp;
  backupSteps: any = [];
  nasshareMode = [
    {
      value: 'auto',
      title: this.i18n.get('protection_auto_nasshare_label'),
      content: this.i18n.get('protection_auto_nasshare_desc_label')
    },
    {
      value: 'manual',
      title: this.i18n.get('protection_manual_nasshare_label'),
      content: this.i18n.get('protection_manual_nasshare_desc_label')
    }
  ];
  selectedMode = 'auto';
  cacheNasSteps;
  guideTime = this.i18n.get('protection_guide_start_time_label');
  hasGuideFlag = false;

  constructor(
    private router: Router,
    private i18n: I18NService,
    private globalService: GlobalService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {
    this.initApplicationConfig();
  }

  initApplicationConfig() {
    // E1000和E6000没有NAS文件系统和通用共享
    if (this.appUtilsService.isDecouple || this.appUtilsService.isDistributed) {
      each(this.applicationConfig, (item: any) => {
        item.apps = reject(item.apps, app =>
          includes(
            [
              DataMap.Resource_Type.NASFileSystem.value,
              DataMap.Resource_Type.ndmp.value,
              DataMap.Resource_Type.commonShare.value
            ],
            app.subType
          )
        );
      });
    }
  }

  closeGuide() {
    this.close.emit();
  }

  toGuide(step: string) {
    this.currentGuide = step;
  }

  activeClick(app) {
    this.activeAppId = app.id;
    this.activeApp = app;
    const backupSteps = cloneDeep(USER_GUIDE_PROTECTION_STEPS);
    if (this.activeApp.steps?.beforeBackup) {
      assign(backupSteps.beforeBackup, {
        steps: this.activeApp.steps?.beforeBackup
      });
    } else {
      // 没有步骤去掉
      delete backupSteps.beforeBackup;
    }
    if (this.activeApp.steps?.resource) {
      assign(backupSteps.resource, {
        steps: this.activeApp.steps?.resource
      });
    } else {
      delete backupSteps.resource;
    }

    if (this.activeApp.steps?.vmGroup) {
      assign(backupSteps.vmGroup, {
        steps: this.activeApp.steps?.vmGroup
      });
    } else {
      delete backupSteps.vmGroup;
    }

    if (this.activeApp.steps?.storageDevice) {
      assign(backupSteps.storageDevice, {
        steps: this.activeApp.steps?.storageDevice
      });
    } else {
      delete backupSteps.storageDevice;
    }

    if (this.activeApp.steps?.backup) {
      assign(backupSteps.backup, {
        steps: this.activeApp.steps?.backup
      });
    } else {
      delete backupSteps.backup;
    }

    if (this.activeApp.steps?.configNas) {
      assign(backupSteps.configNas, {
        steps: this.activeApp.steps?.configNas
      });
    } else {
      delete backupSteps.configNas;
    }

    if (this.activeApp.steps?.configPermission) {
      assign(backupSteps.configPermission, {
        steps: this.activeApp.steps?.configPermission
      });
    } else {
      delete backupSteps.configPermission;
    }

    // 通用共享去掉客户端
    if (this.activeApp?.subType === DataMap.Resource_Type.commonShare.value) {
      delete backupSteps.installClient;
    }

    // NAS共享
    if (this.activeApp?.subType === DataMap.Resource_Type.NASShare.value) {
      assign(backupSteps.installClient, {
        isOptional: true
      });
      this.cacheNasSteps = cloneDeep(backupSteps);
      this.modeClick();
    } else {
      this.backupSteps = values(backupSteps);
    }

    each(this.backupSteps, item => {
      assign(item, {
        expand: false
      });
    });
    // 清除上一次缓存数据
    clearUserGuideCache();
    USER_GUIDE_CACHE_DATA.active = true;
    USER_GUIDE_CACHE_DATA.appType = this.activeApp.subType;
    USER_GUIDE_CACHE_DATA.slaType =
      this.activeApp.subType === DataMap.Resource_Type.ndmp.value
        ? ApplicationType.NASFileSystem
        : this.activeApp.id;
    this.guideTime = this.i18n.get('protection_guide_start_time_label', [
      this.appUtilsService.convertDateLongToString(new Date().getTime())
    ]);
    this.hasGuideFlag = true;
  }

  // NAS共享模式切换
  modeClick() {
    if (this.selectedMode === 'auto') {
      this.backupSteps = values(omit(this.cacheNasSteps, ['resource']));
    } else {
      this.backupSteps = values(
        omit(this.cacheNasSteps, ['storageDevice', 'configNas'])
      );
    }
  }

  // 跳转联机帮助
  gotoHelp(item) {
    const targetUrl = `/console/assets/help/a8000/${
      this.i18n.isEn ? 'en-us' : 'zh-cn'
    }/index.html#${this.i18n.isEn ? item.enLink : item.link}`;
    window.open(targetUrl, '_blank');
  }

  // 跳转界面
  gotoRouter(item) {
    SUB_APP_REFRESH_FLAG.emit = true;
    // 是否需要弹出提示
    USER_GUIDE_CACHE_DATA.showTips = !item.hideTips;
    // 激活哪个页签
    if (item.activeTab) {
      USER_GUIDE_CACHE_DATA.activeTab = item.activeTab;
    }
    if (this.router.url === item.routerLink) {
      this.globalService.emitStore({
        action: USER_GUIDE_CACHE_DATA.action,
        state: { tab: item.activeTab, showTips: !item.hideTips }
      });
    } else {
      this.router.navigateByUrl(item.routerLink);
    }
  }

  expandBar(step) {
    step.expand = !step.expand;
  }

  next() {
    this.activeStep++;
  }

  previous() {
    this.activeStep--;
  }

  endGuide() {
    if (this.activeStep === 2) {
      this.currentGuide = this.guideMap.main;
    } else {
      this.close.emit();
    }
  }

  continnue() {
    this.currentGuide = this.guideMap.backup;
  }
}

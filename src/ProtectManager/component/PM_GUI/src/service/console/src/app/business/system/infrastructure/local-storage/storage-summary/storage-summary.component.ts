import { combineLatest } from 'rxjs';
import {
  Component,
  EventEmitter,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import {
  CapacityCalculateLabel,
  CAPACITY_UNIT,
  I18NService,
  LANGUAGE,
  LocalStorageApiService,
  MODAL_COMMON,
  StoragesApiService,
  CookieService,
  DataMap
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { filter, toString, assign } from 'lodash';
import { ThresholdModifyComponent } from './threshold-modify/threshold-modify.component';
import { SystemTimeService } from 'app/shared/services/system-time.service';

@Component({
  selector: 'aui-storage-summary',
  templateUrl: './storage-summary.component.html',
  styleUrls: ['./storage-summary.component.less'],
  providers: [CapacityCalculateLabel]
})
export class StorageSummaryComponent implements OnInit {
  unitconst = CAPACITY_UNIT;
  ableJump = false;
  progressBarColor = [[0, '#6C92FA']];
  usedSizeColor = '#b8becc';
  storageInfo = {} as any;
  leftItems = [
    {
      key: 'version',
      label: this.i18n.get('common_version_label')
    },
    {
      key: 'mode',
      label: this.i18n.get('common_model_label')
    },
    {
      key: 'totalCapacity',
      label: this.i18n.get('system_total_disk_capacity_label')
    }
  ];
  rightItems = [
    {
      key: 'esn',
      label: this.i18n.get('ESN')
    },
    {
      key: 'wwn',
      label: this.i18n.get('WWN')
    }
  ];
  isHyperDetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;

  @Output() openDeviceChange = new EventEmitter<any>();

  constructor(
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private storagesApiService: StoragesApiService,
    private capacityCalculateLabel: CapacityCalculateLabel,
    private localStorageApiService: LocalStorageApiService,
    private systemTimeService: SystemTimeService
  ) {}

  ngOnInit() {}

  getData() {
    combineLatest(
      this.localStorageApiService.getStorageInfoUsingGET({}),
      this.storagesApiService.getLocalStorageUsingGET({})
    ).subscribe(res => {
      this.storageInfo = {
        ...res[0],
        ...res[1]
      };

      assign(this.storageInfo, {
        sizePercent: this.getSizePercent(this.storageInfo),
        alarmThreasholdPer: `${(
          this.storageInfo.alarmThreashold * 100
        ).toFixed()}%`
      });

      if (
        this.storageInfo.sizePercent / 100 >=
        this.storageInfo.alarmThreashold
      ) {
        this.progressBarColor = [[0, '#FA8E5A']];
        this.usedSizeColor = '#FA8E5A';
      } else {
        this.progressBarColor = [[0, '#6C92FA']];
        this.usedSizeColor = '#6C92FA';
      }

      this.leftItems = filter(this.leftItems, (item: any) => {
        if (item.key === 'totalCapacity') {
          this.storageInfo[item.key] = this.capacityCalculateLabel.transform(
            this.storageInfo[item.key],
            '1.1-3',
            CAPACITY_UNIT.BYTE,
            true
          );
        }
        return (item['value'] = this.storageInfo[item.key]);
      });

      this.rightItems = filter(
        this.rightItems,
        (item: any) => (item['value'] = this.storageInfo[item.key])
      );
    });
  }

  openDeviceManage() {
    this.localStorageApiService.getStorageTokenUsingGET({}).subscribe(
      res => {
        this.openDeviceChange.emit();
        const language =
          this.i18n.language.toLowerCase() === LANGUAGE.CN ? 'zh' : 'en';
        const url = `https://${encodeURI(res.ip)}:${encodeURI(
          toString(res.port)
        )}/deviceManager/devicemanager/feature/login/crossDomainLogin.html?passphrase=${encodeURIComponent(
          res.token
        )}&language=${encodeURIComponent(language)}`;
        window.open(url, '_blank');
      },
      err => {
        this.openDeviceChange.emit();
      }
    );
  }

  modifyThreshold(data) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader:
        this.i18n.get('common_modify_label') +
        this.i18n.get('common_threshold_label'),
      lvContent: ThresholdModifyComponent,
      lvWidth: MODAL_COMMON.smallWidth,
      lvComponentParams: {
        data
      },
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const component = modal.getContentComponent() as ThresholdModifyComponent;
        const modalIns = modal.getInstance();
        component.formGroup.statusChanges.subscribe(res => {
          modalIns.lvOkDisabled = res !== 'VALID';
        });
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const component = modal.getContentComponent() as ThresholdModifyComponent;
          component.onOK().subscribe(
            res => {
              resolve(true);
              this.getData();
            },
            err => resolve(false)
          );
        });
      }
    });
  }

  getSizePercent(source): string {
    const sizePercent = parseFloat(
      (source.usedSize / source.totalSize) * 100 + ''
    );
    return this.capacityCalculateLabel.formatDecimalPoint(sizePercent, 3);
  }
}

import { Component, OnInit } from '@angular/core';
import {
  CapacityCalculateLabel,
  DataMap,
  DataMapService,
  LANGUAGE,
  LicenseApiService,
  MODAL_COMMON,
  SystemApiService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { I18NService } from 'app/shared/services/i18n.service';
import { assign, each, isNull, now } from 'lodash';
import {
  CAPACITY_UNIT,
  ColorConsts
} from './../../../shared/consts/common.const';
import { ImportLicenseComponent } from './import-license/import-license.component';
import { map } from 'rxjs/operators';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-license',
  templateUrl: './license.component.html',
  styleUrls: ['./license.component.less'],
  providers: [CapacityCalculateLabel]
})
export class LicenseComponent implements OnInit {
  esn;
  tableData = [];
  _isNull = isNull;
  dataMap = DataMap;
  exportBtnDisable = true;
  activateBtnDisable = true;
  unitconst = CAPACITY_UNIT;
  columns = [
    {
      label: this.i18n.get('system_authorization_name_label'),
      show: true,
      key: 'name'
    },
    {
      label: this.i18n.get('system_available_capacity_label'),
      show: true,
      width: '28%',
      key: 'capacity'
    },
    {
      label: this.i18n.get('common_import_time_label'),
      show: true,
      width: '18%',
      key: 'importTime'
    },
    {
      label: this.i18n.get('system_license_end_time_label'),
      show: true,
      width: '18%',
      key: 'swUpgradeDueDate'
    }
  ];
  lessThanLabel = this.i18n.get('common_less_than_label');

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private licenseApiService: LicenseApiService,
    private systemApiService: SystemApiService,
    private capacityCalculateLabel: CapacityCalculateLabel
  ) {}

  ngOnInit() {
    this.getLicense();
  }

  onChange() {
    this.ngOnInit();
  }

  getESNCode() {
    if (this.esn) {
      return;
    }
    this.systemApiService.queryEsnUsingGET({}).subscribe(res => {
      this.esn = res.esn;
    });
  }

  getSizePercent(source): string {
    const sizePercent = parseFloat(
      (source.usedCapacity / source.totalCapacity) * 100 + ''
    );
    return this.capacityCalculateLabel.formatDecimalPoint(sizePercent, 3);
  }

  getLicense() {
    this.licenseApiService
      .queryLicenseUsingGET({})
      .pipe(
        map(res => {
          each(res, item => {
            assign(item, {
              sizePercent: item.usedCapacity ? this.getSizePercent(item) : 0,
              alarmThreasholdPer: '80%',
              progressBarColor:
                item.usedCapacity &&
                item.usedCapacity / item.totalCapacity > 0.8
                  ? [[0, ColorConsts.ABNORMAL]]
                  : [[0, ColorConsts.NORMAL]]
            });
          });
          return res;
        })
      )
      .subscribe(res => {
        this.tableData = res;
        this.exportBtnDisable = !res.length;
      });
  }

  importLicense() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'importLicense',
        lvHeader: this.i18n.get('common_import_label'),
        lvContent: ImportLicenseComponent,
        lvWidth:
          this.i18n.language === LANGUAGE.CN
            ? MODAL_COMMON.normalWidth + 50
            : MODAL_COMMON.normalWidth + 250,
        lvOkDisabled: true,
        lvComponentParams: {},
        lvOk: modal => {
          const content = modal.getContentComponent() as ImportLicenseComponent;
          return new Promise(resolve => {
            content.onOK().subscribe(
              res => {
                resolve(true);
                this.getLicense();
              },
              error => {
                resolve(false);
              }
            );
          });
        }
      })
    );
  }

  exportLicense() {
    this.licenseApiService.exportLicenseUsingGET({}).subscribe(blob => {
      const bf = new Blob([blob], {
        type: 'application/octet-stream'
      });
      this.appUtilsService.downloadFile(`license${now()}.dat`, bf);
    });
  }

  getCapacityPercent(source) {
    const capacityPercent = parseFloat(source + '');
    return this.capacityCalculateLabel.formatDecimalPoint(capacityPercent, 3);
  }
}

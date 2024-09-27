import { Component, Input, OnInit } from '@angular/core';
import { CapacityCalculateLabel, CAPACITY_UNIT, LANGUAGE } from 'app/shared';
import { I18NService, CookieService } from 'app/shared/services';

@Component({
  selector: 'aui-data-reduction',
  templateUrl: './data-reduction.component.html',
  styleUrls: ['./data-reduction.component.less'],
  providers: [CapacityCalculateLabel]
})
export class DataReductionComponent implements OnInit {
  @Input() isHomepage;
  spaceReductionRate;
  logicalUsed;
  logicalUsedUnit;
  physicalUsed;
  physicalUsedUnit;
  isZh: boolean;

  constructor(
    private i18n: I18NService,
    public cookieService: CookieService,
    public capacityCalculateLabel: CapacityCalculateLabel
  ) {}

  ngOnInit() {
    this.isZh = this.i18n.language === LANGUAGE.CN;
  }

  initCapacityInfo(systemCapacityInfo) {
    if (!systemCapacityInfo) {
      return;
    }
    const logicalUsedInfo = this.transformKBUnit(
      systemCapacityInfo.writeCapacity
    );
    const physicalUsedInfo = this.transformKBUnit(
      systemCapacityInfo.consumedCapacity
    );
    const logicalUsedUnit = logicalUsedInfo.slice(-2);
    const physicalUsedUnit = physicalUsedInfo.slice(-2);
    const logicalUsed = logicalUsedInfo.slice(0, -2);
    const physicalUsed = physicalUsedInfo.slice(0, -2);

    if (
      logicalUsed <= 0 ||
      physicalUsed <= 0 ||
      systemCapacityInfo.writeCapacity < systemCapacityInfo.consumedCapacity
    ) {
      this.logicalUsed = 0;
      this.physicalUsed = 0;
      this.logicalUsedUnit = 'KB';
      this.physicalUsedUnit = 'KB';
      this.spaceReductionRate = '--';
    } else {
      this.logicalUsed = logicalUsed;
      this.physicalUsed = physicalUsed;
      this.logicalUsedUnit = logicalUsedUnit;
      this.physicalUsedUnit = physicalUsedUnit;
      this.spaceReductionRate =
        systemCapacityInfo.spaceReductionRate > 0
          ? `${systemCapacityInfo.logic}${systemCapacityInfo.spaceReductionRate}:1`
          : '--';
    }
  }

  private transformKBUnit(capacity) {
    return this.capacityCalculateLabel.transform(
      capacity,
      '1.3-3',
      CAPACITY_UNIT.KB,
      true
    );
  }
}

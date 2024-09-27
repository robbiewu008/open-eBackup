import { Component, OnDestroy, OnInit } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  GlobalService,
  PolicyType
} from 'app/shared';
import { assign, each, find, isArray } from 'lodash';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-replica-advanced-parameter',
  templateUrl: './replica-advanced-parameter.component.html',
  styleUrls: ['./replica-advanced-parameter.component.less']
})
export class ReplicaAdvancedParameterComponent implements OnInit, OnDestroy {
  formGroup: FormGroup;
  resourceData;
  subResourceType;
  dataMap = DataMap;
  valid$ = new Subject<boolean>();
  isSpecialResource = false;
  hasArchive = false;
  slaData; // 由于复制资源的保护原本没有高级参数，所以需要外层传入副本限制组件判断
  DetectionResource = this.dataMapService.toArray('Detecting_Resource_Type');
  destroy$ = new Subject();

  constructor(
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    private globalService: GlobalService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getState();
  }

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  initForm() {
    this.formGroup = this.fb.group({});
    this.formGroup.statusChanges.subscribe(res => {
      this.valid$.next(this.formGroup.valid);
    });
  }

  getState(): void {
    this.globalService
      .getState('slaSelectedEvent')
      .pipe(takeUntil(this.destroy$))
      .subscribe(res => {
        this.hasArchive = !!find(res.policy_list, {
          type: PolicyType.ARCHIVING
        });
        this.slaData = res;
      });
  }

  initData(data: any, resourceType: string) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.subResourceType = this.resourceData.resourceSubType;
    this.isSpecialResource = !!find(this.DetectionResource, {
      value: this.subResourceType
    });
  }

  onOK() {
    const ext_parameters = {};

    each(['enable_security_archive'], key => {
      if (this.formGroup.get(key)) {
        assign(ext_parameters, {
          [key]: this.formGroup.get(key).value
        });
      }
    });

    return {
      ext_parameters
    };
  }
}

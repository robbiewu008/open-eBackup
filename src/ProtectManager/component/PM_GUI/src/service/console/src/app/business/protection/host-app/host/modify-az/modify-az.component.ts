import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  CommonConsts,
  CookieService,
  HcsResourceServiceService,
  OpHcsServiceApiService,
  ProtectedResourceApiService
} from 'app/shared';
import { assign, first, map } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-modify-az',
  templateUrl: './modify-az.component.html',
  styleUrls: ['./modify-az.component.less']
})
export class ModifyAzComponent implements OnInit {
  data;
  formGroup: FormGroup;
  azOptions = [];

  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  constructor(
    private fb: FormBuilder,
    private cookieService: CookieService,
    private hcsResourceService: HcsResourceServiceService,
    private opHcsServiceApiService: OpHcsServiceApiService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getAzOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      az: new FormControl(this.data?.extendInfo?.availableZone || '')
    });
  }

  getAzOptions() {
    if (this.isHcsUser) {
      this.hcsResourceService.GetHcsAz({}).subscribe(res => {
        this.azOptions = map(res.resources, item => {
          return assign(item, {
            value: item.resource_id,
            label: first(item.tags?.display_name),
            isLeaf: true
          });
        });
      });
    } else {
      this.opHcsServiceApiService.getAvailableZones({}).subscribe(res => {
        this.azOptions = map(res.records, item => {
          return assign(item, {
            value: item.azId,
            label: item.name,
            isLeaf: true
          });
        });
      });
    }
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.protectedResourceApiService
        .UpdateResource({
          resourceId: this.data.uuid,
          UpdateResourceRequestBody: {
            extendInfo: {
              availableZone: this.formGroup.value.az || ''
            }
          }
        })
        .subscribe(
          () => {
            observer.next();
            observer.complete();
          },
          () => {
            observer.error(null);
            observer.complete();
          }
        );
    });
  }
}

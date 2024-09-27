import { Component, OnInit } from '@angular/core';
import { Form, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  ClientManagerApiService,
  DataMap,
  DataMapService,
  I18NService
} from 'app/shared';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-config-log-level',
  templateUrl: './config-log-level.component.html',
  styleUrls: ['./config-log-level.component.less']
})
export class ConfigLogLevelComponent implements OnInit {
  data;
  currentLevel = DataMap.Log_Level.info.value;
  formGroup: FormGroup;
  logLevelOptions = this.dataMapService.toArray('Log_Level').filter(item => {
    return (item.isLeaf = true);
  });

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    if (this.data?.extendInfo?.logLeve) {
      this.currentLevel = this.data?.extendInfo?.logLeve;
    }
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      log_level: new FormControl(this.currentLevel)
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const params = {
        logLevel: this.formGroup.value.log_level
      };

      this.clientManagerApiService
        .updateAgentLogConfigurationPUT({
          configParam: params,
          agentId: this.data.uuid
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
    });
  }
}

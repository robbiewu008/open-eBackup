/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
import {
  Component,
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  DataMap,
  DataMapService,
  I18NService,
  LogManagerApiService
} from 'app/shared';
import { ExportFilesService } from 'app/shared/components/export-files/export-files.component';
import { InfoMessageService } from 'app/shared/services/info-message.service';
import { each, includes, isEmpty, map, union, uniq } from 'lodash';
import { Subject, Subscription } from 'rxjs';

@Component({
  selector: 'aui-cyber-debug-log',
  templateUrl: './cyber-debug-log.component.html',
  styleUrls: ['./cyber-debug-log.component.less']
})
export class CyberDebugLogComponent implements OnInit, OnDestroy {
  debugLogFormGroup: FormGroup;
  exportLogFormGroup: FormGroup;
  originalLoglevel;
  nodeNames = [];
  nodeNameOptions = [];
  componentOptions = [];
  debugLogDisable = true;
  components = this.dataMapService
    .toArray('cyberComponentName')
    .filter(v => (v.isLeaf = true));
  levelOptions = this.dataMapService
    .toArray('Log_Level')
    .filter(v => (v.isLeaf = true));
  timeSub$: Subscription;
  destroy$ = new Subject();
  @ViewChild('fileDownloadCompletedTpl', { static: true })
  fileDownloadCompletedTpl: TemplateRef<any>;
  fileDownloadProcessingLabel = this.i18n.get(
    'common_file_download_processing_label'
  );
  fileDownloadCompletedLabel = this.i18n.get(
    'common_file_download_completed_label'
  );
  fileDownloadErrorLabel = this.i18n.get('common_file_download_error_label');
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private baseUtilService: BaseUtilService,
    private infoMessageService: InfoMessageService,
    private logManagerApiService: LogManagerApiService,
    private exportFilesService: ExportFilesService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getDebugLog();
    this.getNodeNames();
  }

  onChange() {
    this.ngOnInit();
  }

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  initForm() {
    this.debugLogFormGroup = this.fb.group({
      log_level: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      })
    });

    this.debugLogFormGroup.get('log_level').valueChanges.subscribe(res => {
      this.debugLogDisable = res === this.originalLoglevel;
    });

    this.exportLogFormGroup = this.fb.group({
      nodeName: new FormControl([], {
        validators: [this.baseUtilService.VALID.minLength(1)]
      }),
      componentName: new FormControl([], {
        validators: [this.baseUtilService.VALID.minLength(1)]
      })
    });

    this.exportLogFormGroup
      .get('nodeName')
      .valueChanges.subscribe(nodeNames => {
        let components = [];
        each(nodeNames, nodeName => {
          const destNode = this.nodeNames.find(node => {
            return node.nodeName === nodeName;
          });

          if (!destNode) {
            return;
          }
          components = union(components, destNode.componentList);
        });

        this.componentOptions = this.components.filter(component => {
          return includes(uniq(components), component.value);
        });
      });
  }

  getDebugLog() {
    this.logManagerApiService.queryCurrentLevel({}).subscribe(res => {
      this.originalLoglevel = res.data.logLevel;
      this.debugLogFormGroup.get('log_level').setValue(res.data.logLevel);
    });
  }

  getNodeNames() {
    this.logManagerApiService.collectNodeInfo({}).subscribe(res => {
      this.nodeNames = res.data;
      this.nodeNameOptions =
        map(res.data, (v: any) => {
          v.isLeaf = true;
          v.label = v.nodeName;
          return v;
        }) || [];
    });
  }

  ok() {
    if (
      this.debugLogFormGroup.value.log_level !== DataMap.Log_Level.debug.value
    ) {
      this.logManagerApiService
        .setLogLevel({ body: this.debugLogFormGroup.value })
        .subscribe(res => this.getDebugLog());
    } else {
      this.infoMessageService.create({
        content: this.i18n.get('system_debug_log_info_label', [
          this.dataMapService.getLabel(
            'Log_Level',
            DataMap.Log_Level.debug.value
          )
        ]),
        onOK: () => {
          this.logManagerApiService
            .setLogLevel({ body: this.debugLogFormGroup.value })
            .subscribe(res => this.getDebugLog());
        },
        onCancel: () => this.getDebugLog()
      });
    }
  }

  export(exportParams?) {
    const params = isEmpty(exportParams)
      ? this.exportLogFormGroup.value
      : exportParams;
    params.timeRange = 0;
    this.exportFilesService.create({
      data: { params, type: DataMap.Export_Query_Type.log.value }
    });
  }
}

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
import { ChangeDetectorRef, ElementRef, Injectable } from '@angular/core';
import { I18NService } from 'app/shared/services/i18n.service';
import { Subject } from 'rxjs';
interface GuideStepItem {
  id: string; // 元素id
  step: number;
  name: string;
  isGroup?: boolean; // 是否分组
  mountOrigin?: ElementRef;
  description: string | any[]; // 纯文字描述或者(图标+文字)
}

interface GuideServiceData {
  guideSteps: Map<number, GuideStepItem>;
  currentStep: number;
}

@Injectable({
  providedIn: 'root'
})
export class GuideService {
  private COMER_GUIDE_DATA: GuideStepItem[];
  private guideStepsMap: Map<number, GuideStepItem> = new Map();
  private mountIdProcessMap: Map<string, number> = new Map();
  private currentStep: number;
  private currentStep_ = new Subject<number>();
  constructor(private i18n: I18NService) {
    this.COMER_GUIDE_DATA = [
      {
        id: 'menuIconTpl',
        step: 1,
        name: this.i18n.get('common_comer_guidance_navigator_label'),
        description: this.i18n.get('common_comer_guidance_navigator_tips_label')
      },
      {
        id: 'globalSearchTpl',
        step: 2,
        name: this.i18n.get('common_global_search_label'),
        description: this.i18n.get(
          'common_comer_guidance_global_search_tips_label'
        )
      },
      {
        id: 'intelliMateIconTpl',
        step: 3,
        name: this.i18n.get('common_comer_guidance_intelli_assistant_label'),
        description: this.i18n.get(
          'common_comer_guidance_guide_assistant_tips_label'
        )
      },
      {
        id: 'shortcutOpTpl',
        step: 4,
        name: this.i18n.get('common_comer_guidance_shortcut_operation_label'),
        isGroup: true,
        description: [
          {
            id: 'cluster',
            icon: 'aui-new-comer-cluster',
            name: this.i18n.get('common_comer_guidance_cluster_change_label'),
            description: this.i18n.get(
              'common_comer_guidance_cluster_change_tips_label'
            )
          },
          {
            id: 'alarm',
            icon: 'aui-new-comer-alarm',
            name: this.i18n.get('common_alarms_label'),
            description: this.i18n.get('common_comer_guidance_alarm_tips_label')
          },
          {
            id: 'jobs',
            icon: 'aui-new-comer-jobs',
            name: this.i18n.get('common_jobs_label'),
            description: this.i18n.get('common_comer_guidance_job_tips_label')
          },
          {
            id: 'language',
            icon: 'aui-new-comer-language',
            name: this.i18n.get('common_comer_guidance_language_change_label'),
            description: this.i18n.get(
              'common_comer_guidance_language_change_tips_label'
            )
          },
          {
            id: 'userCenter',
            icon: 'aui-new-comer-account',
            name: this.i18n.get('common_comer_guidance_user_center_label'),
            description: this.i18n.get(
              'common_comer_guidance_user_center_tips_label'
            )
          },
          {
            id: 'helpCenter',
            icon: 'aui-new-comer-help',
            name: this.i18n.get('common_help_label'),
            description: this.i18n.get(
              'common_comer_guidance_help_center_tips_label'
            )
          }
        ]
      }
    ];
    this.initData();
  }

  initData() {
    this.processGuideData();
    this.currentStep = 1;
    this.setStep(this.currentStep);
  }

  private processGuideData() {
    // 更新guideMap和mountIdMap
    this.COMER_GUIDE_DATA.forEach(item => {
      this.guideStepsMap.set(item.step, item);
      this.mountIdProcessMap.set(item.id, item.step);
    });
  }

  setStep(step: number) {
    this.currentStep = step;
    this.currentStep_.next(step);
  }

  setGuideMountOriginById(id: string, origin: ElementRef) {
    const stepIndex: number = this.mountIdProcessMap.get(id);
    this.guideStepsMap.get(stepIndex).mountOrigin = origin;
  }

  addStepData(data: GuideStepItem[]) {
    // 动态的添加Menu组件的子菜单
    if (!data.length) {
      return;
    }
    this.COMER_GUIDE_DATA.push(...data);
    this.processGuideData(); // 数据插入后，同步更新map
  }

  getGuideCurrentStep(): number {
    return this.currentStep;
  }

  getGuideStep(step: number = this.currentStep): GuideStepItem {
    return this.guideStepsMap.get(step);
  }

  getGuideStepSize(): number {
    return this.guideStepsMap.size;
  }

  nextStep() {
    const nextStep = this.currentStep + 1;
    if (nextStep <= this.getGuideStepSize()) {
      this.setStep(nextStep);
    }
  }

  previousStep() {
    const previousStep = this.currentStep - 1;
    if (previousStep >= 1) {
      this.setStep(previousStep);
    }
  }

  startGuide() {
    this.setStep(1);
    return true;
  }

  endGuide(): boolean {
    this.setStep(1);
    return false;
  }
}

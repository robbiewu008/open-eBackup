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
import { LvConfig } from '@iux/live';
import { of } from 'rxjs';
import { mergeMap } from 'rxjs/operators';
import { ICONS_DEFINE } from './consts';
import { I18NService } from './services';
import { getI18nResource } from './guards/preload-i18n-resolver';
import { WhiteboxService } from './services/whitebox.service';
import { noop } from 'lodash';

export class SharedConfig {
  // 配置入口
  static config(i18n: I18NService, whitebox: WhiteboxService) {
    SharedConfig.lvConfig(i18n);
    SharedConfig.iconConfig();
    SharedConfig.whiteboxConfig(whitebox);

    return () => SharedConfig.I18NConfig(i18n);
  }

  // I18N服务配置
  static I18NConfig(i18n: I18NService) {
    const preloadModules = [
      'common',
      'protection',
      'explore',
      'insight',
      'system',
      'search',
      'params',
      'deploy',
      'alarm/common',
      'alarm/ab',
      'alarm/dorado_v6',
      'operation/common',
      'task/common',
      'error-code/ab',
      'error-code/agent',
      'error-code/common',
      'error-code/dorado_v6',
      'error-code/dorado_616',
      'error-code/pacific'
    ];
    const i18nStream = of(...preloadModules).pipe(
      mergeMap(module => getI18nResource(i18n, module))
    );
    return i18nStream.toPromise();
  }

  // icon配置
  static iconConfig() {
    document.body.insertAdjacentHTML('afterbegin', ICONS_DEFINE);
  }

  // 组件配置
  static lvConfig(i18n: I18NService) {
    LvConfig.language = i18n.language as any;
    LvConfig.messageOptions = {
      lvPosition: 'topRight',
      lvDuration: 5 * 1e3,
      lvMaxWidth: '50%'
    };
    LvConfig.modalOptions.lvDrawerPosition = 'right';
    LvConfig.modalOptions.lvDrawerPositionOffset = ['0px', '0px', '0px', '0px'];
    LvConfig.paginatorOptions.lvPageSize = 20;
    LvConfig.paginatorOptions.lvPageSizeOptions = [20, 50, 100];
    LvConfig.operationMenuOptions.lvMaxShowNum = 1;
    LvConfig.operationMenuOptions.lvMenuText =
      i18n.language === i18n.defaultLanguage ? '更多' : 'More';
    LvConfig.operationMenuOptions.lvShowMode = 'dropdown';
    LvConfig.formOptions.lvLabelColon = false;
    LvConfig.tooltipOptions.lvTheme = 'light';
    LvConfig.overflowOptions.lvTheme = 'light';
    LvConfig.popoverOptions.lvTheme = 'light';
    LvConfig.dataTableOptions = {
      lvDisableSortClear: true
    };
    LvConfig.modalOptions.lvOkLoadingText =
      i18n.language === i18n.defaultLanguage ? '加载中...' : 'Loading...';
  }

  static whiteboxConfig(whitebox: WhiteboxService) {
    whitebox.loadOemFile().subscribe();
  }
}

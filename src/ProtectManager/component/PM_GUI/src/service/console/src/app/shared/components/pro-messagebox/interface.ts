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
import { ModalButton } from '@iux/live';
import { Observable } from 'rxjs';
import { TableConfig } from '../pro-table';

export interface ProMessageboxOptions {
  modalKey?: string;
  width?: number | string;
  height?: number | string;
  title?: string;
  icon?:
    | 'question'
    | 'info'
    | 'success'
    | 'error'
    | 'warning'
    | 'danger'
    | string;
  panelClass?: string;
  content: ProMessageboxContentOptions;
  closeButtonDisplay?: boolean;
  beforeOpen?: ProMessageboxCallback; // 无返回参数
  afterOpen?: ProMessageboxCallback; // 返回参数：modal
  beforeClose?: ProMessageboxCallback; // 返回参数：modal、result
  afterClose?: ProMessageboxCallback; // 返回参数：result

  okButton?: {
    showLoading?: boolean;
    loadingText?: string;
    disabled?: boolean;
    onClick: ProMessageboxCallback; // 返回参数：modal
  };
  cancelButton?: {
    showLoading?: boolean;
    loadingText?: string;
    disabled?: boolean;
    onClick: ProMessageboxCallback; // 返回参数：modal
  };
  customButtons?: ModalButton[] | null;
}

export type ProMessageboxCallback = (
  ...args: any[]
) =>
  | (boolean | void | {})
  | Promise<boolean | void | {}>
  | Observable<boolean | void | {}>;

export interface ProMessageboxContentOptions {
  description?: {
    type?: 'html' | 'text';
    value?: string;
  };
  list?: TableConfig;
  checkbox?: {
    data?: { value: string; label: string; disabled?: boolean }[];
    value?: string[];
    valueChange?: (value, modal, checkboxs) => any;
  };
}

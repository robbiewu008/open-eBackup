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
import { Directive, ElementRef, Input, OnInit } from '@angular/core';
import { includes } from 'lodash';
import { RoleOperationAuth } from '../consts/permission.const';

@Directive({
  selector: '[auiRolePermission]'
})
export class RolePermissionDirective implements OnInit {
  @Input() rolePermission: string;
  roleOperationAuth = RoleOperationAuth;

  constructor(private elementRef: ElementRef) {}

  ngOnInit(): void {
    this.elementRef.nativeElement.style.display = includes(
      this.roleOperationAuth,
      this.rolePermission
    )
      ? ''
      : 'none';
  }
}

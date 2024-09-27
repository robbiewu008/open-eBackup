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
/**
 * 将常用module component directive pipe 等统一导入
 */
import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import {
  AlertModule,
  BadgeModule,
  BreadcrumbModule,
  ButtonModule,
  CardModule,
  CheckboxModule,
  CollapseModule,
  DatatableModule,
  DatePickerModule,
  DropdownModule,
  FormModule,
  GroupModule,
  IconModule,
  InputIPModule,
  InputModule,
  LayoutModule,
  LoadingModule,
  MenuModule,
  ModalModule,
  MultiAutocompleteModule,
  OperationmenuModule,
  OverflowModule,
  PaginatorModule,
  PopoverModule,
  ProgressModule,
  RadioModule,
  SearchModule,
  SelectModule,
  SliderModule,
  SortModule,
  SpinnerModule,
  SwitchModule,
  TabsModule,
  TagModule,
  TimePickerModule,
  TooltipModule,
  TreeModule,
  TreeSelectModule,
  UploadModule,
  WizardModule,
  EmptyModule,
  CascaderModule
} from '@iux/live';
import { AccordionModule } from './components/accordion';
import { AlarmLevelModule } from './components/alarm-level';
import { AutoActiveMenuModule } from './components/auto-active-menu';
import { BackupModule } from './components/backup.component';
import { ColumnFilterTplModule } from './components/column-filter-tpl';
import { FeatureModule } from './components/feature';
import { InfoModule } from './components/info.component';
import { InuptWithEyeModule } from './components/inupt-with-eye/inupt-with-eye.module';
import {
  ProCapacityPipe,
  ProNumberPipe,
  ProPercentPipe
} from './components/pro-core';
import { SlaComplianceTooltipModule } from './components/sla-compliance-tooltip/sla-compliance-tooltip.module';
import { SlaTypeModule } from './components/sla-type';
import { StatusModule } from './components/status';
import { WarningModule } from './components/warning.component';
import { PermissionDirective } from './directive/permission.directive';
import { RolePermissionDirective } from './directive/role-permission.directive';
import { PIPES } from './pipe';
import { DrawModalService } from './services/draw-modal.service';
import { AgentSelectModule } from './components/agent-select/agent-select.module';
@NgModule({
  providers: [DrawModalService],
  declarations: [
    ...PIPES,
    ProCapacityPipe,
    ProPercentPipe,
    ProNumberPipe,
    PermissionDirective,
    RolePermissionDirective
  ],
  exports: [
    CommonModule,
    FormsModule,
    ReactiveFormsModule,
    IconModule,
    StatusModule,
    ModalModule,
    AutoActiveMenuModule,
    DatatableModule,
    PaginatorModule,
    SortModule,
    FormModule,
    SelectModule,
    GroupModule,
    ButtonModule,
    InputModule,
    OverflowModule,
    TooltipModule,
    SwitchModule,
    TabsModule,
    DatePickerModule,
    SearchModule,
    DropdownModule,
    CheckboxModule,
    TreeModule,
    SpinnerModule,
    PopoverModule,
    BadgeModule,
    ProgressModule,
    WizardModule,
    BreadcrumbModule,
    TimePickerModule,
    RadioModule,
    CollapseModule,
    TagModule,
    LayoutModule,
    WarningModule,
    InfoModule,
    BackupModule,
    MenuModule,
    UploadModule,
    OperationmenuModule,
    SliderModule,
    UploadModule,
    CardModule,
    AlertModule,
    ...PIPES,
    ProCapacityPipe,
    ProPercentPipe,
    ProNumberPipe,
    SlaTypeModule,
    AlarmLevelModule,
    PermissionDirective,
    RolePermissionDirective,
    InputIPModule,
    FeatureModule,
    AccordionModule,
    LoadingModule,
    ColumnFilterTplModule,
    TreeSelectModule,
    InuptWithEyeModule,
    SlaComplianceTooltipModule,
    MultiAutocompleteModule,
    EmptyModule,
    CascaderModule,
    AgentSelectModule
  ]
})
export class BaseModule {}

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
import { RouterUrl } from 'app/shared';

export const cardlist = [
  {
    name: 'missionOverview',
    title: 'common_home_job_overview_label',
    index: 1,
    footer: [
      {
        text: 'common_home_view_all_jobs_label',
        isBtn: true,
        navigateInfo: [RouterUrl.InsightJobs]
      }
    ],
    clusterNodesOptions: [],
    selectTime: 3, // 任务统计范围
    selectcluster: undefined // 任务总览选中的集群
  },
  {
    name: 'capacity',
    title: 'common_home_capacity_label',
    subtitle: 'common_home_physical_label',
    index: 2
  },
  {
    name: 'alarm',
    title: 'common_home_alarms_label',
    index: 3,
    footer: [
      {
        text: 'common_home_view_all_warning_label',
        isBtn: true,
        navigateInfo: [RouterUrl.InsightAlarms]
      }
    ]
  },
  {
    title: 'common_home_resource_protection_label',
    index: 4,
    footer: [
      {
        text: 'common_home_view_resource_label',
        isBtn: true,
        navigateInfo: [RouterUrl.ProtectionSummary]
      }
    ]
  },
  {
    title: 'common_home_ransomware_protection_label',
    index: 5,
    footer: [
      {
        text: 'common_home_view_details_label',
        isBtn: true,
        navigateInfo: [RouterUrl.ExploreAntiRansomware]
      }
    ]
  },
  {
    name: 'performance',
    title: 'common_home_performance_label',
    index: 6,
    footer: [
      {
        text: 'common_home_view_details_label',
        isBtn: true,
        navigateInfo: [RouterUrl.InsightPerformance],
        navigateParams: {
          selectTime: null,
          selectNode: null
        }
      }
    ],
    selectTime: 3,
    selectNode: [],
    clusterNodesOptions: [],
    clusterType: undefined
  },
  {
    name: 'capacityDiction',
    title: 'common_home_capacity_diction_label',
    subtitle: 'common_home_physical_label',
    index: 7,
    selectNode: [],
    clusterNodesOptions: [],
    clusterType: undefined
  },
  {
    title: 'common_home_reduction_ratio_label',
    index: 8
  },
  {
    title: 'common_home_resources_label',
    index: 9
  },
  {
    title: 'common_home_sla_compliance_label',
    index: 10,
    slaTooltip: true
  },
  {
    name: 'topFailedTasksSlaProtectionPolicy',
    title: 'common_home_top_job_failures_sla_protection_policy_label',
    index: 11,
    selectTime: 4
  },
  {
    title: 'common_home_top_job_failures_resource_object_label',
    index: 12,
    selectTime: 4
  },
  {
    title: 'common_external_associated_systems_label',
    index: 13,
    footer: [
      {
        text: 'common_view_all_label',
        isBtn: true,
        navigateInfo: [RouterUrl.ExternalAssociatedSystems]
      }
    ]
  }
];

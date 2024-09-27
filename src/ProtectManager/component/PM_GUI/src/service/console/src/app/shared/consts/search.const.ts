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
export enum SearchRange {
  COPIES = 'COPIES',
  RESOURCES = 'RESOURCES',
  LABELS = 'LABELS'
}

export enum FilterType {
  DbbackupAgent = 101,
  VmbackupAgent = 102,
  AbbackupClient = 103,
  Fileset = 201,
  DfsFileset = 202,
  Volume = 203,
  Oracle = 301,
  SqlServer = 302,
  DB2 = 303,
  Mysql = 304,
  Gaussdb = 305,
  SapHana = 306,
  Kingbase = 307,
  Sybase = 308,
  Informix = 309,
  Timesten = 310,
  Gbase = 311,
  Dameng = 312,
  Cassandra = 313,
  Oscardb = 314,
  Exchange = 401,
  Vcenter = 501,
  Esx = 502,
  Esxi = 503,
  Fusionsphere = 505,
  ClusterComputeResource = 506,
  HostSystem = 507,
  Folder = 508,
  ResourcePool = 509,
  Openstack = 601,
  HuaweiCloudStack = 602,
  VimVirtualMachine = 701,
  VimDataCenter = 510,
  VimVirtualApp = 511,
  HwVirtualMachine = 702,
  MsVirtualMachine = 703,
  Hadoop = 801,
  FusionInsight = 802,
  HDFSFileset = 803,
  HBaseBackupset = 2,
  NasFileSystem = 3,
  NasShare = 902,
  Kubernetes = 5,
  LocalFileSystem = 6,
  FusionCompute = 905,
  FusionOneCompute = 912,
  HCSCloudHost = 906,
  ObjectStorage = 907,
  OpenstackCloudServer = 908,
  APSCloudServer = 909,
  CnwareVm = 910,
  HyperV = 911,
  Ndmp = 204
}

export enum NodeType {
  Folder,
  File,
  Link
}

export enum SearchResource {
  Refresh = 'NeedRefresh'
}

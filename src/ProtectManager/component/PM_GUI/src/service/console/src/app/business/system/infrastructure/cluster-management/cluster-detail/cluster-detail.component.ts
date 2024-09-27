import { Component, OnInit, ViewChild } from '@angular/core';
import {
  CommonConsts,
  DataMapService,
  I18NService,
  WarningMessageService
} from 'app/shared';
import { ClustersApiService } from 'app/shared/api/services';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, each, extend, isEmpty, includes } from 'lodash';

@Component({
  selector: 'aui-cluster-detail',
  templateUrl: './cluster-detail.component.html',
  styleUrls: ['./cluster-detail.component.less']
})
export class ClusterDetailComponent implements OnInit {
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  drawData;
  nodesData = [];
  nodeNameSearch;
  theadFilterMap = {};
  currentHostName = location.hostname;
  clusterType = this.dataMapService.getConfig('Cluster_Type');
  basicInfoLabel = this.i18n.get('common_basic_info_label');
  nameLabel = this.i18n.get('common_name_label');
  typeLabel = this.i18n.get('common_type_label');
  statusLabel = this.i18n.get('common_status_label');
  ipLabel = this.i18n.get('system_management_ip_label');
  qosPolicyLabel = this.i18n.get('common_limit_rate_policy_label');
  nodeLabel = this.i18n.get('system_servers_label');
  managementIpLabel = this.i18n.get('system_management_ip_label');
  dataplaneIpLabel = this.i18n.get('common_dataplane_ip_label');
  operationLabel = this.i18n.get('common_operation_label');
  configDataplaneIpLabel = this.i18n.get('system_configure_dataplane_ip_label');
  clearDataplaneIpLabel = this.i18n.get('system_clear_dataplane_ip_label');
  selectQosLabel = this.i18n.get('system_select_qos_label');
  localClusterLabel = this.i18n.get('system_local_cluster_label');
  targetClusterLabel = this.i18n.get('common_target_cluster_label');
  statusFilters = this.dataMapService.toArray('Cluster_Node_Status');
  _includes = includes;

  constructor(
    public i18n: I18NService,
    public drawmodalservice: DrawModalService,
    public warningMessageService: WarningMessageService,
    public dataMapService: DataMapService,
    public clusterApiService: ClustersApiService
  ) {}

  filterChange(e) {
    extend(this.theadFilterMap, {
      [e.key]: e.value
    });
    each(this.theadFilterMap, (value, key) => {
      if (isEmpty(value)) {
        delete this.theadFilterMap[key];
      }
    });
    this.getNodes();
  }

  getNodes() {
    this.clusterApiService
      .queryClusterNodeInfoUsingGET(
        assign(
          {
            clusterId: this.drawData.clusterId,
            startPage: this.pageIndex,
            pageSize: this.pageSize,
            nodeName: this.nodeNameSearch
          },
          this.theadFilterMap
        )
      )
      .subscribe(res => {
        this.nodesData = res.records;
        this.total = res.totalCount;
      });
  }

  clusterPageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getNodes();
  }

  ngOnInit() {
    this.getNodes();
  }
}

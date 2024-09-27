import { Component, OnInit, ViewChild } from '@angular/core';
import { DatatableComponent, PaginatorComponent } from '@iux/live';
import {
  CAPACITY_UNIT,
  DataMap,
  DataMapService,
  ResourceType,
  VmwareService
} from 'app/shared';
import { trim, map } from 'lodash';

@Component({
  selector: 'aui-vm-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  unitconst = CAPACITY_UNIT;
  resourceType = ResourceType;
  dataMap = DataMap;
  source;
  tableData = [];

  queryName;
  queryDeviceId;
  querySlaName;
  queryDatastore;
  filterParams = {};

  @ViewChild(DatatableComponent, { static: false }) lvTable: DatatableComponent;
  @ViewChild(PaginatorComponent, { static: false }) lvPage: PaginatorComponent;

  constructor(private vmwareService: VmwareService) {}

  ngOnInit() {
    if (this.source.sub_type === DataMap.Resource_Type.virtualMachine.value) {
      if (this.source.properties) {
        const properties = JSON.parse(this.source.properties);
        if (
          properties.vmware_metadata &&
          properties.vmware_metadata.disk_info
        ) {
          this.tableData = map(properties.vmware_metadata.disk_info, item => {
            item['name'] = item.NAME;
            item['slot'] = item.BUSNUMBER;
            item['capacity'] = item.SIZE;
            item['datastoreName'] = item.DSNAME;
            return item;
          });
        }
        return;
      }
      this.getDiskData();
    }
  }

  initDetailData(data: any) {
    this.source = data;
  }

  getDiskData() {
    this.vmwareService
      .listVmDiskV1VirtualMachinesVmUuidDisksGet({ vmUuid: this.source.uuid })
      .subscribe(res => {
        this.tableData = [...res];
        this.tableData.forEach(item => {
          item.datastoreName = item.datastore.name;
          item.sla_name = '';
          if (
            this.source.ext_parameters &&
            (this.source.ext_parameters.all_disk ||
              this.source.ext_parameters.disk_info.includes(item.uuid))
          ) {
            item.sla_name = this.source.sla_name;
          }
        });
      });
  }

  sortChange(source) {
    this.lvTable.sort(source);
    this.lvPage.jumpToFisrtPage();
  }

  searchByName(queryName) {
    this.lvTable.filter({
      key: 'name',
      value: trim(queryName),
      filterMode: 'contains'
    });
    this.lvPage.jumpToFisrtPage();
  }

  searchBySlaName(querySlaName) {
    this.lvTable.filter({
      key: 'sla_name',
      value: trim(querySlaName),
      filterMode: 'contains'
    });
    this.lvPage.jumpToFisrtPage();
  }

  searchByDeviceId(queryDeviceId) {
    this.lvTable.filter({
      key: 'slot',
      value: trim(queryDeviceId),
      filterMode: 'contains'
    });
    this.lvPage.jumpToFisrtPage();
  }

  searchByDatastore(queryDatastore) {
    this.lvTable.filter({
      key: 'datastoreName',
      value: trim(queryDatastore),
      filterMode: 'contains'
    });
    this.lvPage.jumpToFisrtPage();
  }
}

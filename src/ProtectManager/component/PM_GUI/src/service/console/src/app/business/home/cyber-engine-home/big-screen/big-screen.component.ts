import { Component, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import { DetectReportsService, I18NService, RouterUrl } from 'app/shared';
import { EnvironmentPojo } from 'app/shared/api/models';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { each, eq, first, get, isNil, set } from 'lodash';

@Component({
  selector: 'aui-big-screen',
  templateUrl: './big-screen.component.html',
  styleUrls: ['./big-screen.component.less']
})
export class BigScreenComponent implements OnInit {
  environmentInfoList: EnvironmentPojo[]; // 节点列表
  normalCount: number; // 正常节点
  exceptionCount: number; // 异常节点
  offlineCount: number; // 离线节点
  totalCount: number; // 总数
  statusArr: string[] = Array(8);
  constructor(
    private detectReportsService: DetectReportsService,
    public i18n: I18NService,
    private router: Router,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.loadNodeInfo();
  }
  loadNodeInfo(loading: boolean = true) {
    this.detectReportsService
      .queryEnvironment({
        akLoading: loading
      })
      .subscribe(res => {
        this.environmentInfoList = isNil(res.environmentInfoList)
          ? []
          : res.environmentInfoList;
        this.totalCount = get(res, 'deviceTotalNum', 0);
        this.normalCount = get(res, 'normalDeviceNum', 0);
        this.exceptionCount = get(res, 'exceptionDeviceNum', 0);
        this.offlineCount = get(res, 'offlineDeviceNum', 0);
        this.setNodeStatus();
      });
  }
  setNodeStatus() {
    const container = first(document.getElementsByClassName('big-screen-box'));
    const setBgImg = (node: Element, imgName: string) =>
      set(
        node,
        'style',
        `background:url('assets/img/${imgName}') no-repeat;background-size:100% 100%;;`
      );

    each(this.environmentInfoList, (node, index) => {
      const nodeDom = first(container.getElementsByClassName(`node-${index}`));
      const nodTitleDom =
        !isNil(nodeDom) &&
        first(nodeDom.getElementsByClassName('big-screen-node-title'));
      if (!nodeDom) {
        return;
      }

      if (nodeDom) {
        nodeDom.classList.add('link-block');
        nodeDom.addEventListener('click', () => {
          if (!node.environmentStatus) {
            this.gotoFileSystem(node.name);
          } else {
            this.gotoDevice();
          }
        });
      }

      // 离线
      if (!eq(node.linkStatus, '1')) {
        this.statusArr[index] = 'offline';
        setBgImg(nodeDom, 'cyberengine_home_node_offline.png');
        if (nodTitleDom) {
          nodTitleDom.innerHTML =
            get(node, 'name') +
            '<br>' +
            this.i18n.get('common_file_system_label') +
            ' --';
        }
        // 离线优先级高于是否正常
        return;
      }

      if (nodTitleDom) {
        nodTitleDom.innerHTML =
          get(node, 'name') +
          '<br>' +
          this.i18n.get('common_file_system_label') +
          ' ' +
          get(node, 'fileSystemTotalNum');
      }

      // 正常
      if (!!node.environmentStatus) {
        this.statusArr[index] = 'normal';
        setBgImg(nodeDom, 'cyberengine_home_node_normal.png');
      }
      // 异常
      if (!node.environmentStatus) {
        this.statusArr[index] = 'error';
        setBgImg(nodeDom, 'cyberengine_home_node_error.png');
      }
    });
  }

  gotoDevice() {
    this.router.navigateByUrl(RouterUrl.ExploreStorageDevice);
  }

  gotoFileSystem(name) {
    this.appUtilsService.setCacheValue('abnormalDeviceName', name);
    this.router.navigateByUrl(RouterUrl.ExploreSnapShotData);
  }
}

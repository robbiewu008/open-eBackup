import {
  Component,
  Input,
  OnChanges,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { MessageService } from '@iux/live';
import {
  DataMap,
  DataMapService,
  I18NService,
  LocalStorageApiService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { cloneDeep, find, get, includes, isEmpty, map } from 'lodash';
@Component({
  selector: 'aui-link',
  templateUrl: './link.component.html',
  styleUrls: ['./link.component.less']
})
export class LinkComponent implements OnInit {
  find = find;
  dataMap = DataMap;
  @Input() detailData;
  @Input() resourceConfigs;
  @Input() activeId;
  @Input() isConfig;
  tableConfig: TableConfig;
  tableData: TableData;
  allNfsLink = [];
  allCifsLink = [];
  whitelistArr = [];
  whitelistStr;
  linkInfo = {
    esn: '',
    filesystemName: '',
    nfs: null,
    sharePath: '',
    cifs: null,
    shareName: '',
    userType: null,
    userNames: [],
    whitelist: []
  };
  nfsEnabled = false;
  cifsEnabled = false;
  shareIps = [];
  userOptions = [];
  userTypeOptions = this.dataMapService
    .toArray('Cifs_Domain_Client_Type')
    .filter(v => {
      v.isLeaf = true;
      return includes(
        [
          DataMap.Cifs_Domain_Client_Type.everyone.value,
          DataMap.Cifs_Domain_Client_Type.windows.value,
          DataMap.Cifs_Domain_Client_Type.windowsGroup.value
        ],
        v.value
      );
    });

  @ViewChild('shareIpTpl', { static: true }) shareIpTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private LocalStorageApiService: LocalStorageApiService,
    private message: MessageService
  ) {}

  ngOnInit() {
    this.initConfig();
    this.getShareIps();
    if (this.detailData) {
      this.initDetailData1();
    }
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'shareIp',
        name: 'IP',
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.shareIpTpl
      }
    ];

    this.tableConfig = {
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      },
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        async: false,
        scroll: { y: '300px' }
      }
    };
  }

  initDetailData(data, config) {
    if (!isEmpty(data)) {
      this.linkInfo = cloneDeep(data?.extendInfo);
      if (this.linkInfo.nfs === 'true') {
        this.nfsEnabled = true;
        this.linkInfo.whitelist = cloneDeep(
          JSON.parse(get(data?.extendInfo, 'whitelist', '[]'))
        );
        this.whitelistStr = this.linkInfo.whitelist?.join();
      }

      if (this.linkInfo.cifs === 'true') {
        this.cifsEnabled = true;
        this.linkInfo.userType = Number(this.linkInfo.userType);
        this.linkInfo.userNames = JSON.parse(
          get(data?.extendInfo, 'userNames', '[]')
        );
      }
    }
  }

  initDetailData1() {
    this.linkInfo.filesystemName = this.detailData?.filesystemName;
    if (!isEmpty(this.detailData?.nfs)) {
      this.nfsEnabled = true;
      this.linkInfo.sharePath = this.detailData?.nfs.sharePath;
      this.linkInfo.whitelist = get(this.detailData?.nfs, 'whitelist', '[]');
      this.whitelistStr = this.linkInfo.whitelist.join();
    }

    if (!isEmpty(this.detailData?.cifs)) {
      this.cifsEnabled = true;
      this.linkInfo.shareName = this.detailData?.cifs.shareName;
      this.linkInfo.userType = Number(this.detailData?.cifs.userType);
      this.linkInfo.userNames = get(this.detailData?.cifs, 'userNames', '[]');
    }
  }

  getShareIps() {
    this.LocalStorageApiService.getLogicPortUsingGET({
      akDoException: false,
      protocol: '2,3'
    }).subscribe(res => {
      this.shareIps = map(res.logicPortList, item => {
        return {
          shareIp: item
        };
      });
      this.tableData = {
        total: this.shareIps.length,
        data: this.shareIps
      };
    });
  }

  // 点击复制
  copyLink(data: any) {
    const oInput = document.createElement('input'); // 创建一个input标签
    oInput.value = data; // 将要复制的值赋值给input

    document.body.appendChild(oInput); // 在页面中插入
    oInput.select(); // 模拟鼠标选中
    document.execCommand('Copy'); // 执行浏览器复制命令（相当于ctrl+c）
    oInput.style.display = 'none'; // 只是用一下input标签的特性，实际并不需要显示，所以这里要隐藏掉

    this.message.success(this.i18n.get('common_copy_success_label'), {
      lvMessageKey: 'msgRight',
      lvPosition: 'topRight',
      lvShowCloseButton: true
    });
  }
}

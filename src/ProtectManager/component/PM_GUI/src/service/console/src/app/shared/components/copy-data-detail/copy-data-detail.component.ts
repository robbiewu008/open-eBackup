import { DatePipe } from '@angular/common';
import {
  Component,
  ElementRef,
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { ModalRef } from '@iux/live';
import {
  CapacityCalculateLabel,
  CommonConsts,
  extendNodeParams
} from 'app/shared';
import {
  AccessPointControllerService,
  CopyControllerService,
  MediaSetApiService
} from 'app/shared/api/services';
import { CopiesService } from 'app/shared/api/services/copies.service';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { CAPACITY_UNIT, DataMap, WormStatusEnum } from 'app/shared/consts';
import {
  CookieService,
  DataMapService,
  I18NService
} from 'app/shared/services';
import { ManualMountService } from 'app/shared/services/manual-mount.service';
import {
  RestoreParams,
  RestoreService
} from 'app/shared/services/restore.service';
import {
  assign,
  cloneDeep,
  each,
  find,
  first,
  get,
  includes,
  isArray,
  isEmpty,
  isNumber,
  isString,
  map,
  reduce,
  reject,
  size,
  trim,
  unionBy
} from 'lodash';
import { Subject, Subscription, timer } from 'rxjs';
import { switchMap, takeUntil } from 'rxjs/operators';
import { AppUtilsService } from '../../services/app-utils.service';
import { DownloadFlrFilesComponent } from '../download-flr-files/download-flr-files.component';
import { ExportFilesService } from '../export-files/export-files.component';

@Component({
  selector: 'aui-copy-data-detail',
  templateUrl: './copy-data-detail.component.html',
  styleUrls: ['./copy-data-detail.component.less'],
  providers: [DatePipe, CapacityCalculateLabel]
})
export class CopyDataDetailComponent implements OnInit, OnDestroy {
  activeFileTab;
  gn;
  activeIndex;
  unitconst = CAPACITY_UNIT;
  data;
  formItems = [];
  verifyItems = [];

  isViewParentCopy = false;
  isClickedFilesetFile = false;
  isIndexCreating = false;
  showIndex;
  verifyStatus: string;

  tableSelection = [];
  tableData = [];

  isSingleRestore = false;
  copyStatus = DataMap.copydata_validStatus;
  fileIndex = DataMap.CopyData_fileIndex;
  copyDataGeneratedType = DataMap.CopyData_generatedType;

  treeTableData = [];
  treeTableSelection = [];
  tableColumns = [
    {
      key: 'name',
      label: this.i18n.get('common_name_label')
    },
    {
      key: 'slot',
      label: this.i18n.get('common_slot_label')
    },
    {
      key: 'capacity',
      label: this.i18n.get('common_capacity_label'),
      align: 'right'
    },
    {
      key: 'name',
      label: this.i18n.get('common_datastore_label')
    }
  ];
  // 主机卷数据
  volumeTree = [];
  dataMap = DataMap;
  // GoldenDB分片数据
  groupTableConfig: TableConfig;
  groupTableData: TableData;

  incrementalData = [];

  @ViewChild('headerTpl', { static: true }) headerTpl: TemplateRef<any>;
  @ViewChild('batchDownloadFoolterTpl', { static: true })
  batchDownloadFoolterTpl: TemplateRef<any>;
  @ViewChild('fileDownloadCompletedTpl', { static: true })
  fileDownloadCompletedTpl: TemplateRef<any>;
  fileDownloadCompletedLabel = this.i18n.get(
    'common_file_download_completed_label'
  );
  @ViewChild('groupDataTable', { static: false })
  groupDataTable: ProTableComponent;

  cloudArchivalDesc = this.i18n.get('explore_cloud_archive_file_desc_label');
  downloadFlrFilesComponent = new DownloadFlrFilesComponent(
    this.exportFilesService
  );
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );

  timeSub$: Subscription;
  destroy$ = new Subject();

  tapeTableConfig: TableConfig;
  tapeTableData: TableData;

  // 是否进入过文件tab页签
  hasActiveFileTab = false;

  constructor(
    public i18n: I18NService,
    private modal: ModalRef,
    private datePipe: DatePipe,
    private elementRef: ElementRef,
    public copiesApiService: CopiesService,
    private cookieService: CookieService,
    public accessPointControllerService: AccessPointControllerService,
    public dataMapService: DataMapService,
    private restoreService: RestoreService,
    public manualMountService: ManualMountService,
    private exportFilesService: ExportFilesService,
    private capacityCalculateLabel: CapacityCalculateLabel,
    private mediaSetApi: MediaSetApiService,
    public appUtilsService: AppUtilsService,
    private copyControllerService: CopyControllerService
  ) {}

  ngOnInit() {
    this.getBasicInfo();
    this.updateModalHeader();
    this.getTableData();
    this.getTreeTableData();
    this.getVolumeTreeData();
    this.getIncrementalData();
    this.initVerifyStatus(this.data);
    this.initTapeTable();
    this.afterModalClose();

    if (
      includes(
        [
          DataMap.Resource_Type.HCSCloudHost.value,
          DataMap.Resource_Type.openStackCloudServer.value,
          DataMap.Resource_Type.KubernetesStatefulset.value,
          DataMap.Resource_Type.FusionCompute.value,
          DataMap.Resource_Type.fusionOne.value,
          DataMap.Resource_Type.cNwareVm.value,
          DataMap.Resource_Type.dbTwoDatabase.value,
          DataMap.Resource_Type.dbTwoTableSet.value
        ],
        this.data.resource_sub_type
      )
    ) {
      this.getCopyVerify();
    }
    if (this.activeFileTab) {
      this.activeIndex = 'file';
      this.hasActiveFileTab = true;
    }
  }

  initTapeTable() {
    if (
      !includes(
        [this.copyDataGeneratedType.tapeArchival.value],
        this.data.generated_by
      )
    ) {
      return;
    }
    this.tapeTableConfig = {
      table: {
        async: false,
        columns: [
          {
            key: 'TapeSetName',
            name: this.i18n.get('protection_tape_set_name_label')
          },
          {
            key: 'TapeLibraryID',
            name: this.i18n.get('protection_tape_id_label')
          },
          {
            key: 'TapeID',
            name: this.i18n.get('protection_tape_tag_label')
          }
        ],
        colDisplayControl: false
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };

    this.mediaSetApi
      .getMediaDetailByArchiveCopyId({
        archiveCopyId: this.data.uuid,
        akDoException: false
      })
      .subscribe(res => {
        this.tapeTableData = {
          data: res,
          total: size(res)
        };
      });
  }

  getGoldenDBInfo() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'id',
        name: this.i18n.get('GroupID')
      }
    ];

    this.groupTableConfig = {
      table: {
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        async: false
      },
      pagination: {
        mode: 'simple'
      }
    };

    const resourceData = JSON.parse(this.data?.resource_properties || '{}');
    const instance = JSON.parse(
      get(resourceData, 'extendInfo.clusterInfo', '{}')
    );
    const group = get(instance, 'group', []);
    const groupNum = size(group);
    assign(this.data, {
      groupNum: groupNum
    });

    this.groupTableData = {
      data: map(group, item => {
        return {
          id: item?.groupId,
          name: `DBGroup${item?.groupId}`
        };
      }),
      total: groupNum
    };
  }

  initVerifyStatus(data) {
    const properties = isString(data.properties)
      ? JSON.parse(data.properties)
      : data.properties;
    this.verifyStatus = get(properties, 'verifyStatus');
  }

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  _isWorm(row) {
    return [
      WormStatusEnum.SETTING,
      WormStatusEnum.SET_SUCCESS,
      WormStatusEnum.SET_FAILED
    ].includes(row?.worm_status);
  }

  getWormLabel(row) {
    const wormStatus = row;
    switch (wormStatus) {
      case WormStatusEnum.UNSET:
        return 'explore_copy_worm_unset_label';
      case WormStatusEnum.SETTING:
        return 'explore_copy_worm_setting_label';
      case WormStatusEnum.SET_SUCCESS:
        return 'explore_copy_worm_setted_label';
      case WormStatusEnum.SET_FAILED:
        return 'explore_copy_worm_set_fail_label';
      default:
        return 'explore_copy_worm_unset_label';
    }
  }

  getWormIcon(row) {
    const wormStatus = row;
    switch (wormStatus) {
      case WormStatusEnum.UNSET:
        return 'aui-copy-data-worm-unset';
      case WormStatusEnum.SETTING:
        return 'aui-icon-loading';
      case WormStatusEnum.SET_SUCCESS:
        return 'aui-copy-data-worm-set-success';
      case WormStatusEnum.SET_FAILED:
        return 'aui-copy-data-worm-set-faild';
      default:
        return 'aui-copy-data-worm-unset';
    }
  }

  getIncrementalData() {
    if (
      !includes(
        [
          DataMap.Resource_Type.NASFileSystem.value,
          DataMap.Resource_Type.NASShare.value,
          DataMap.Resource_Type.ndmp.value
        ],
        this.data.resource_sub_type
      ) ||
      isEmpty(this.data.detail)
    ) {
      return;
    }
    const details = this.data.detail.split('\n');
    each(details, item => {
      this.incrementalData.push({
        label: trim(item.split(':')[0]),
        value: item.split(':')[1] ? trim(item.split(':')[1]) : 0
      });
    });
  }

  afterTabChange = (origin, active) => {
    if (!active.index) {
      return;
    }

    setTimeout(() => {
      if (
        this.copyDataGeneratedType.cloudArchival.value ===
        this.data.generated_by
      ) {
        const cloudArchiveFileDesc = this.elementRef.nativeElement.querySelector(
          'span.cloud-archive-file-desc'
        );
        if (!cloudArchiveFileDesc) {
          return;
        }
        cloudArchiveFileDesc.addEventListener('click', () => {
          this.getVMwareResource();
        });
      }
    }, 0);
  };

  activeIndexChange = active => {
    this.hasActiveFileTab = active === 'file';
    if (
      !(
        !active &&
        DataMap.Resource_Type.fileset.value === this.data.resource_sub_type &&
        this.data.generated_by ===
          this.copyDataGeneratedType.cloudArchival.value
      )
    ) {
      return;
    }

    setTimeout(() => {
      if (
        this.copyDataGeneratedType.cloudArchival.value ===
        this.data.generated_by
      ) {
        const cloudArchiveFileDesc = this.elementRef.nativeElement.querySelector(
          'span.cloud-archive-file-desc'
        );
        if (!cloudArchiveFileDesc) {
          return;
        }
        cloudArchiveFileDesc.addEventListener('click', () => {
          this.isClickedFilesetFile = true;
          this.getFilesetResource();
        });
      }
    }, 0);
  };

  getEsn(): string {
    let esn = '';
    try {
      const properties = JSON.parse(this.data.properties);
      esn = properties.fsRelations.relations[0].oldEsn;
    } catch (error) {
      esn = '';
    }
    return esn;
  }

  getBasicInfo() {
    if (
      [DataMap.Resource_Type.goldendbInstance.value].includes(
        this.data.resource_sub_type
      )
    ) {
      this.getGoldenDBInfo();
    }

    this.formItems = [
      [
        {
          key: 'status',
          label: this.i18n.get('common_status_label'),
          value: this.data.status
        },
        {
          key: 'location',
          label: this.i18n.get('common_location_label'),
          value: this.data.location
        },
        {
          key: 'backup_type',
          label: this.i18n.get('common_copy_type_label'),
          value: this.data?.source_copy_type || this.data.backup_type
        },
        {
          key: 'resource_name',
          label: this.i18n.get('protection_resource_name_label'),
          value: this.data.resource_name,
          hidden:
            this.data.resource_sub_type ===
            DataMap.Resource_Type.virtualMachine.value
        },
        {
          key: 'esn',
          label: this.i18n.get('common_equipment_esn_label'),
          value: this.getEsn(),
          hidden:
            this.data.generated_by !==
            DataMap.CopyData_generatedType.replicate.value
        },
        {
          key: 'cluster_name',
          label: this.i18n.get('system_servers_label'),
          value: this.data.cluster_name,
          hidden: !this.isOceanProtect || this.appUtilsService.isDistributed
        }
      ],
      [
        {
          key: 'worm_status',
          label: this.i18n.get('explore_worm_th_label'),
          value: this.data.worm_status || 1
        },
        {
          key: 'indexed',
          label: this.i18n.get('common_index_label'),
          value: this.data.indexed,
          hidden:
            ![
              DataMap.Resource_Type.virtualMachine.value,
              DataMap.Resource_Type.fileset.value
            ].includes(this.data.resource_sub_type) || this.isHcsUser
        },
        {
          key: 'uuid',
          label: this.i18n.get('protection_id_label'),
          value: this.data.uuid,
          hidden: !includes(
            [DataMap.Deploy_Type.hyperdetect.value],
            this.i18n.get('deploy_type')
          )
        },
        {
          key: 'generated_by',
          label: this.i18n.get('common_generated_type_label'),
          value: this.data.generated_by
        },
        {
          key: 'expiration_time',
          label: this.i18n.get('common_expriration_time_label'),
          value:
            this.data.retention_type === 1
              ? '--'
              : this.datePipe.transform(
                  this.data.expiration_time,
                  'yyyy-MM-dd HH:mm:ss'
                )
        },
        {
          key: 'size',
          label: this.i18n.get('insight_job_databeforereduction_label'),
          value: this.getCopySize()
        },
        {
          key: 'isBackupAcl',
          label: this.i18n.get('explore_acl_backup_label'),
          value: this.getIsBackupAcl(),
          hidden: ![DataMap.Resource_Type.ObjectSet.value].includes(
            this.data.resource_sub_type
          )
        }
      ]
    ];

    const deployType = this.i18n.get('deploy_type');
    if (['d3', 'd4', 'cloudbackup'].includes(deployType)) {
      this.formItems[1] = this.formItems[1].filter(
        item => item.key !== 'worm_status'
      );
    }
  }

  getCopySize() {
    const properties = isString(this.data.properties)
      ? JSON.parse(this.data.properties)
      : this.data.properties;
    if (!properties.size) {
      return '--';
    }
    const sizeStr = `${this.capacityCalculateLabel.transform(
      properties.size,
      '1.3-3',
      CAPACITY_UNIT.KB,
      true
    )}`;
    const size = parseFloat(sizeStr.substring(0, sizeStr.indexOf('.') + 3));
    const unit = trim(sizeStr.replace(/[0-9.]/g, ''));
    return `${size} ${unit}`;
  }

  getIsBackupAcl() {
    const resourceData = JSON.parse(this.data?.resource_properties || '{}');
    if (!resourceData.ext_parameters) {
      return '--';
    }
    return this.dataMapService.getLabel(
      'aclType',
      resourceData.ext_parameters.isBackupAcl
    );
  }

  getCopyVerify() {
    const source_data = JSON.parse(this.data.properties);
    const items = [
      {
        label: this.i18n.get('common_last_verify_complete_time_label'),
        content: source_data.verifyTime,
        key: 'time'
      },
      {
        label: this.i18n.get('common_copy_verify_status_label'),
        content: includes(
          [
            DataMap.CopyData_generatedType.cloudArchival.value,
            DataMap.CopyData_generatedType.tapeArchival.value
          ],
          this.data.generated_by
        )
          ? '--'
          : find(this.dataMapService.toArray('HCSCopyDataVerifyStatus'), {
              value: source_data.verifyStatus
            })?.label,
        key: 'status'
      }
    ];
    this.verifyItems = items;
  }

  getTableData() {
    if (
      DataMap.Resource_Type.virtualMachine.value !== this.data.resource_sub_type
    ) {
      return;
    }
    this.tableData = JSON.parse(this.data.properties).vmware_metadata.disk_info;
  }

  getTreeTableData() {
    if (
      DataMap.Resource_Type.virtualMachine.value === this.data.resource_sub_type
    ) {
      this.getVMwareResource();
    } else if (
      DataMap.Resource_Type.SQLServerDatabase.value ===
      this.data.resource_sub_type
    ) {
      this.getSQLServerResource();
    }
  }

  getSQLServerResource() {
    const resource = isString(this.data.properties)
      ? JSON.parse(this.data.properties)
      : {};
    this.treeTableData = [
      {
        hasChildren: false,
        name: resource.dataPath,
        parent_uuid: null,
        path: ''
      }
    ];
  }

  getFilesetResource() {
    const resource = isString(this.data.resource_properties)
      ? JSON.parse(this.data.resource_properties)
      : {};
    const isWindows =
      resource.environment_os_type ===
      this.dataMapService.getConfig('Os_Type').windows.value;
    this.treeTableData = isWindows
      ? [
          {
            children: [],
            hasChildren: true,
            name: resource.environment_name,
            parent_uuid: null,
            path: '',
            icon: 'aui-icon-host'
          }
        ]
      : [
          {
            children: [
              {
                children: [],
                hasChildren: true,
                name: '/',
                parent_uuid: null,
                path: '',
                icon: 'aui-icon-directory'
              }
            ],
            hasChildren: true,
            name: resource.environment_name,
            parent_uuid: null,
            path: '',
            icon: 'aui-icon-host'
          }
        ];
    if (isWindows) {
      this.getResource(this.treeTableData[0]);
    } else {
      this.getResource(this.treeTableData[0].children[0]);
    }
  }

  getVMwareResource() {
    this.copiesApiService
      .queryResourcesV1CopiesGet({
        akLoading: false,
        pageNo: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE,
        conditions: JSON.stringify({
          uuid: this.data.parent_copy_uuid
        })
      })
      .subscribe(res => {
        this.gn = !!size(res.items) ? first(res.items)['gn'] : '';
      });
    this.treeTableData = [
      {
        name: '',
        nodeName: '',
        children: [],
        path: '/',
        label: this.data?.resource_name,
        absolutePath: '/',
        contentToggleIcon: 'aui-icon-directory'
      }
    ];
  }

  getVolumeTreeData() {
    if (
      DataMap.Resource_Type.ABBackupClient.value !== this.data.resource_sub_type
    ) {
      return;
    }
    this.getVolumes();
  }

  getVolumes(recordsTemp?, startPage?) {
    this.accessPointControllerService
      .queryBackupClustersUsingPOST({
        akOperationTips: false,
        copyId: this.data.uuid,
        count: 100,
        index: startPage || CommonConsts.PAGE_START,
        path: ''
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage === Math.ceil(res.totalCount / 100) ||
          res.totalCount === 0
        ) {
          const volumeData = [];
          const resource = isString(this.data.resource_properties)
            ? JSON.parse(this.data.resource_properties)
            : {};
          each(recordsTemp, (item: any) => {
            volumeData.push({
              label: (() => {
                if (
                  resource.environment_os_type === DataMap.Os_Type.windows.value
                ) {
                  return item.path.endsWith(':')
                    ? item.path.replace('/', '').replace(':', ':\\')
                    : item.path.replace('/', '');
                } else {
                  return item.path.replace('/\\', '/').replace(/\\/g, '/');
                }
              })(),
              contentToggleIcon: 'aui-icon-volume',
              isLeaf: true
            });
          });
          this.volumeTree = [
            {
              label: !isEmpty(resource.environment_endpoint)
                ? `${resource.environment_name}(${resource.environment_endpoint})`
                : resource.environment_name,
              contentToggleIcon: 'aui-icon-host',
              expanded: true,
              children: volumeData
            }
          ];
          return;
        }
        this.getVolumes(recordsTemp, startPage);
      });
  }

  getVmFileDownloadPath(paths) {
    let childPaths = [];
    // 特殊处理根目录
    if (!paths[0].nodeName && paths[0].path === '/' && paths[0].name === '/') {
      paths = paths[0].children;
    }
    paths = reject(paths, { nodeName: '' });
    each(paths, item => {
      if (!!size(item.children)) {
        childPaths = unionBy(childPaths, item.children, 'path');
      }
    });
    return reject(paths, item => {
      return !isEmpty(find(childPaths, { path: item.path }));
    });
  }

  getPath(paths, isTable?) {
    let filterPaths = [];
    let childPaths = [];
    paths = reject(paths, { path: '' });
    paths = reject(paths, { isMoreBtn: true });
    each(paths, item => {
      if (!!size(item.children)) {
        childPaths = unionBy(childPaths, item.children, 'path');
      }
    });
    if (this.isSingleRestore) {
      filterPaths = isTable
        ? paths
        : reject(paths, item => {
            return !isEmpty(item.children);
          });
    } else {
      filterPaths = reject(paths, item => {
        return !isEmpty(find(childPaths, { path: item.path }));
      });
    }

    return filterPaths;
  }

  getVmFilePath(paths, isTable?) {
    let filterPaths = [];
    let childPaths = [];
    if (isTable && !paths[0].nodeName) {
      paths = paths[0].children;
    }
    paths = reject(paths, { nodeName: '' });
    paths = reject(paths, { isMoreBtn: true });
    each(paths, item => {
      if (!!size(item.children)) {
        childPaths = unionBy(childPaths, item.children, 'path');
      }
    });
    filterPaths = reject(paths, item => {
      return !isEmpty(find(childPaths, { path: item.path }));
    });

    return reduce(
      filterPaths,
      (arr, item) => {
        arr.push(
          item.path === '/' ? `/${item.name}` : `${item.path}/${item.name}`
        );
        return arr;
      },
      []
    );
  }

  tableSelectionChange(selection) {
    this.treeTableSelection = selection;
  }

  getVMwareFiles(node?, startPage?) {
    const params = {
      copyId: this.data.uuid,
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.MAX_PAGE_SIZE,
      parentPath:
        node.path === '/'
          ? `${node.path}${node.nodeName}`
          : `${node.path}/${node.nodeName}`,
      akOperationTips: false
    };
    if (!isEmpty(this.data.device_esn)) {
      assign(params, {
        memberEsn: this.data.device_esn
      });
    }
    this.copyControllerService.ListCopyCatalogs(params).subscribe(res => {
      each(res.records, (item: any) => {
        item = extendNodeParams(node, item);
      });
      if (isArray(node.children) && !isEmpty(node.children)) {
        node.children = [
          ...reject(node.children, n => {
            return n.isMoreBtn;
          }),
          ...res.records
        ];
      } else {
        node.children.push(...res.records);
      }
      if (res.totalCount > size(node.children)) {
        const moreClickNode = {
          label: `${this.i18n.get('common_more_label')}...`,
          contentToggleIcon: '',
          isMoreBtn: true,
          hasChildren: false,
          isLeaf: true,
          children: null,
          parent: node,
          startPage: Math.floor(
            size(node.children) / CommonConsts.MAX_PAGE_SIZE
          )
        };
        node.children = [...node.children, moreClickNode];
      }
      this.treeTableData = [...this.treeTableData];
      this.isViewParentCopy =
        this.activeIndex &&
        this.data.generated_by ===
          this.copyDataGeneratedType.cloudArchival.value;
    });
  }

  trackByIndex(index, item) {
    return index;
  }

  getResource(node?, startPage?) {
    this.accessPointControllerService
      .queryBackupClustersUsingPOST({
        akOperationTips: false,
        copyId: this.data.uuid,
        count: 100,
        index: startPage || CommonConsts.PAGE_START,
        path: node ? node.gns_path : ''
      })
      .subscribe(res => {
        each(res.records, (item: any) => {
          item.isLeaf = !item.hasChildren;
          item.children = item.hasChildren ? [] : null;
          item.icon = (() => {
            if (
              node.path === '' &&
              this.data &&
              this.data.rowData &&
              this.data.rowData.environment_os_type ===
                this.dataMapService.getConfig('Os_Type').windows.value
            ) {
              return 'aui-icon-volume';
            } else {
              return item.hasChildren ? 'aui-icon-directory' : 'aui-icon-file';
            }
          })();
        });
        if (isArray(node.children) && !isEmpty(node.children)) {
          node.children = [
            ...reject(node.children, n => {
              return n.isMoreBtn;
            }),
            ...res.records
          ];
        } else {
          node.children.push(...res.records);
        }
        if (res.totalCount > size(node.children)) {
          const moreClickNode = {
            name: `${this.i18n.get('common_more_label')}...`,
            isMoreBtn: true,
            hasChildren: false,
            isLeaf: true,
            children: null,
            parent: node,
            startPage: Math.floor(size(node.children) / 100)
          };
          node.children = [...node.children, moreClickNode];
        }
        if (find(this.treeTableSelection, node)) {
          this.treeTableSelection = [
            ...this.treeTableSelection,
            ...res.records
          ];
        }
        this.treeTableData = [...this.treeTableData];
        this.isViewParentCopy =
          this.isClickedFilesetFile &&
          this.data.generated_by ===
            this.copyDataGeneratedType.cloudArchival.value;
      });
  }

  expandedChange(node) {
    const moreBtn = find(node.children, { isMoreBtn: true });
    if (!!size(node.children) && !moreBtn) {
      return;
    }

    if (DataMap.Resource_Type.fileset.value === this.data.resource_sub_type) {
      this.getResource(node);
    } else if (
      DataMap.Resource_Type.virtualMachine.value === this.data.resource_sub_type
    ) {
      this.getVMwareFiles(node, moreBtn?.startPage);
    }
  }

  tableOptsCallback = data => {
    return [
      {
        id: 'restore',
        label: this.i18n.get('common_restore_label'),
        disabled:
          this.data.status !== DataMap.copydata_validStatus.normal.value,
        onClick: () => {
          this.restoreDisk(data);
        }
      }
    ];
  };

  treeTableOptsCallback = data => {
    return [
      {
        id: 'restore',
        label: this.i18n.get('common_restore_label'),
        disabled:
          this.data.status !== DataMap.copydata_validStatus.normal.value,
        onClick: () => {
          this.restoreFile([data]);
        }
      },
      {
        id: 'download',
        label: this.i18n.get('common_export_label'),
        hidden:
          this.data.resource_sub_type === DataMap.Resource_Type.fileset.value,
        onClick: () => {
          this.downloadFile([data]);
        }
      }
    ];
  };

  enableRecovery() {
    this.isIndexCreating = true;
    this.copiesApiService
      .createCopyIndexV1CopiesCopyIdActionCreateIndexPost({
        copyId: this.data.uuid
      })
      .subscribe({
        next: res => {
          this.isIndexCreating = true;
          this.data.indexed = this.fileIndex.indexing.value;
          setTimeout(() => {
            this.getCopyData();
          }, 1000);
        },
        error: () => {
          this.isIndexCreating = false;
          if (this.data.indexed !== this.fileIndex.deletedFailed.value) {
            this.data.indexed = this.fileIndex.unIndexed.value;
          }
        }
      });
  }

  getCopyData() {
    if (this.timeSub$) {
      this.timeSub$.unsubscribe();
    }

    this.timeSub$ = timer(0, CommonConsts.TIME_INTERVAL)
      .pipe(
        switchMap(index => {
          return this.copiesApiService.queryResourcesV1CopiesGet({
            akLoading: false,
            pageNo: CommonConsts.PAGE_START,
            pageSize: CommonConsts.PAGE_SIZE,
            conditions: JSON.stringify({
              uuid: this.data.uuid,
              resource_id: this.data.resource_id
            })
          });
        }),
        takeUntil(this.destroy$)
      )
      .subscribe(
        res => {
          const item =
            res.items.find(it => it.uuid === this.data.uuid) || ({} as any);
          const isIndexCreating =
            item.indexed === this.fileIndex.indexing.value;
          if (!isIndexCreating) {
            this.data = item;
            this.timeSub$.unsubscribe();
            this.getBasicInfo();
            if (item.indexed === this.fileIndex.indexed.value) {
              this.getTreeTableData();
            }
          }
          this.isIndexCreating = isIndexCreating;
        },
        () => this.timeSub$.unsubscribe()
      );
  }

  restoreDisk(item) {
    this.restoreService.restore({
      childResType: this.data.resource_sub_type,
      copyData: assign(
        {},
        {
          diskRestore: true,
          selection: item,
          ...this.data
        }
      )
    });
  }

  restoreFile(item?) {
    const params: RestoreParams = {
      childResType: this.data.resource_sub_type,
      copyData: {}
    };
    if (
      this.data.resource_sub_type === DataMap.Resource_Type.virtualMachine.value
    ) {
      params.copyData = assign({}, this.data, {
        fineGrainedData: item
          ? this.getVmFilePath(item, true)
          : this.getVmFilePath(this.treeTableSelection),
        fileRestore: true
      });
    } else {
      params.copyData = assign({}, this.data, {
        fineGrainedData: item
          ? this.getPath(item, true)
          : this.getPath(this.treeTableSelection),
        isSingleRestore: this.isSingleRestore
      });
    }
    this.restoreService.restore(params);
  }

  downloadFile(datas) {
    const filterPaths = this.getVmFileDownloadPath(cloneDeep(datas));
    const paths = [];
    each(filterPaths, node => {
      paths.push(
        node.path === '/'
          ? `${node.path}${node.nodeName}`
          : `${node.path}/${node.nodeName}`
      );
    });
    if (!size(paths)) {
      return;
    }
    let memberEsn = '';
    if (this.data.device_esn) {
      memberEsn = this.data.device_esn;
    }
    let tip = null;
    if (
      this.data.resource_sub_type === DataMap.Resource_Type.virtualMachine.value
    ) {
      tip = 'explore_export_file_tip_label';
    }
    const copyId =
      this.activeIndex &&
      this.data.resource_sub_type !==
        DataMap.Resource_Type.ABBackupClient.value &&
      this.data.generated_by === this.copyDataGeneratedType.cloudArchival.value
        ? this.data.parent_copy_uuid
        : this.data.uuid;
    this.downloadFlrFilesComponent.getRequestId(paths, copyId, memberEsn, tip);
  }

  updateModalHeader() {
    this.modal.setProperty({ lvHeader: this.headerTpl });
  }

  afterModalClose() {
    if (
      this.data.resource_sub_type === DataMap.Resource_Type.virtualMachine.value
    ) {
      const extendClose = this.modal.getInstance().lvAfterClose;
      this.modal.getInstance().lvAfterClose = () => {
        extendClose.call(this);
        // 如果浏览了文件，需要关闭索引
        if (!this.hasActiveFileTab) {
          return;
        }
        this.copyControllerService
          .CloseCopyGuestSystem({
            copyId: this.data.uuid
          })
          .subscribe(() => {});
      };
    }
  }
}

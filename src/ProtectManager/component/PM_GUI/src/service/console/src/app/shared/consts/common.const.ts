import { FILE_FORMAT } from './system.const';
//  通用常量
export const CommonConsts = {
  // 自动刷新频率
  TIME_INTERVAL: 10 * 1e3,

  TIME_INTERVAL_FOUR: 4 * 1e3,

  TIME_INTERVAL_FIVE: 5 * 1e3,

  TIME_INTERVAL_TWO: 2 * 1e3,

  TIME_INTERVAL_ALARM_COUNT: 13 * 1e3,

  TIME_INTERVAL_RESOURCE: 59 * 1e3,

  TIME_INTERVAL_SESSION_OUT: 61 * 1e3,

  TIME_INTERVAL_JOB_COUNT: 67 * 1e3,

  MAX_PAGE_SIZE: 200,
  // HCS用户类型
  HCS_USER_TYPE: 'HCS',
  // DME用户类型
  DME_USER_TYPE: 'DME',

  // 服务类型
  serviceProduct: 'HCS',

  // 分页页码
  PAGE_START: 0,

  PAGE_START_EXTRA: 1,

  PAGE_SIZE_SMALL: 10,

  PAGE_SIZE_MAX: 200,

  // 每页数量
  PAGE_SIZE: 20,

  // 表格数据总条数
  PAGE_TOTAL: 0,

  // 排序的方式：asc：升序，desc：降序
  ORDER_TYPE: '',

  // 排序字段名称，缺省为name—可选
  ORDER_BY: '',

  // 每页显示的数据选择项
  PAGE_SIZE_OPTIONS: [20, 50, 100],

  // 每页显示的数据选择项
  SIMPLE_PAGE_SIZE_OPTIONS: [10, 20, 50, 100],

  // 侧边栏距离
  DRAWER_OFFSET: ['0px', '0px', '0px', '0px'],

  // 表格操作列定宽
  TABLE_OPERATION_WIDTH: '144px',

  // 公共规则定义
  REGEX: {
    name: /^[\u4e00-\u9fa5a-zA-Z_]{1}[\u4e00-\u9fa5a-zA-Z_0-9-]{0,63}$/,
    reportName: /^[a-z0-9A-Z\\_]+$/,
    email: /^[a-zA-Z0-9\.\-_\!\#\$\%\&\'\*\+\/\=\?\^\`\{\|\}\~]+@[a-zA-Z0-9\-_]+(\.[a-zA-Z0-9\-_]+)+$/,
    ipv4: /^(25[0-5]|2[0-4]\d|[01]?\d\d?)\.(25[0-5]|2[0-4]\d|[01]?\d\d?)\.(25[0-5]|2[0-4]\d|[01]?\d\d?)\.(25[0-5]|2[0-4]\d|[01]?\d\d?)$/i,
    mask: /^(255|0)\.((128|192|224|240|248|252|254|0)\.0\.0)$|^(255\.255\.(128|192|224|240|248|252|254|0)\.0)$|^(255\.255\.255\.(128|192|224|240|248|252|254|0|255))$/,
    ipv6: /^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|([0-9a-fA-F]{1,4}:){6,6}((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,1}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,4}):((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|:((:[0-9a-fA-F]{1,4}){1,5}):((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|:((:[0-9a-fA-F]{1,4}){1,7}|:)|[Ff][Ee]08:(:[0-9a-fA-F]{1,4}){2,2}%[0-9a-zA-Z]{1,}|(0{1,4}:){6,6}((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|(0{1,4}:){1,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|(0{1,4}:){1,4}:(0{1,4}:)((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|(0{1,4}:){1,3}:(0{1,4}:){1,2}((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|(0{1,4}:){1,2}:(0{1,4}:){1,3}((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|(0{1,4}:):(0{1,4}:){1,4}((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|::(0{1,4}:){1,5}((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|(0{1,4}:){5,5}[Ff]{4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|(0{1,4}:){1,4}:[Ff]{4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|(0{1,4}:){1,3}:(0{1,4}:)[Ff]{4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|(0{1,4}:){1,2}:(0{1,4}:){1,2}[Ff]{4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|(0{1,4}:):(0{1,4}:){1,3}[Ff]{4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|::(0{1,4}:){1,4}[Ff]{4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)|::([Ff]{4}:){0,1}((25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)\.){3}(25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d))$/,
    vMwareName: /^[0-9a-zA-Z_\u4e00-\u9fa5@._\\-]+$/,
    metadataStoragePath: /^\/(\w+\/?)+$/,
    severIp: /^[a-zA-Z0-9][-a-zA-Z0-9]{0,62}(\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+$/,
    severDomain: /^(((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}(:[0-9A-Fa-f]{1,4}){1,2})|(([0-9A-Fa-f]{1,4}:){4}(:[0-9A-Fa-f]{1,4}){1,3})|(([0-9A-Fa-f]{1,4}:){3}(:[0-9A-Fa-f]{1,4}){1,4})|(([0-9A-Fa-f]{1,4}:){2}(:[0-9A-Fa-f]{1,4}){1,5})|(([0-9A-Fa-f]{1,4})?:(:[0-9A-Fa-f]{1,4}){1,6})|(([0-9A-Fa-f]{1,4}:){1,6}:)|(::)|(([0-9A-Fa-f]{1,4}:){6}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)(\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)){3})|(([0-9A-Fa-f]{1,4}:){5}:(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)(\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)){3})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,1}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)(\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)){3})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,2}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)(\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)){3})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)(\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)){3})|(([0-9A-Fa-f]{1,4})?::([0-9A-Fa-f]{1,4}:){0,4}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)(\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)){3})))$/i,
    scriptName: /^.+[\.]{1}(bat|sh){1}$/,
    cloudStorageName: /^[0-9a-zA-Z_\u4e00-\u9fa5@._\-]{3,30}$/,
    volName: /^[a-zA-Z0-9]{3,30}$/,
    componentName: /^[_a-zA-Z]{1}[_a-zA-Z0-9 -]{3,31}$/,
    userName: /^[\u4e00-\u9fa5a-zA-Z_]{1}[\u4e00-\u9fa5a-zA-Z_0-9-]{4,63}$/,
    linuxUserNameBegin: /^[a-zA-Z0-9_]/,
    linuxUserName: /^[a-zA-Z0-9-_]*$/,
    sftpNameBegin: /^[a-zA-Z_]/,
    sftpNameCombination: /^[a-zA-Z_0-9]+$/,
    clusterName: /^[a-zA-Z0-9_\u4e00-\u9fa5]{1}[\u4e00-\u9fa5\.\w-]{3,31}$/,
    storagePoolName: /^[a-zA-Z0-9_\u4e00-\u9fa5]{1}[\u4e00-\u9fa5\.\w-]{3,63}$/,
    description: /^[\u4e00-\u9fa5\w-\.]*$/,
    backupName: /^[a-zA-Z0-9_\u4e00-\u9fa5]{1}[\u4e00-\u9fa5\w-]{3,31}$/,
    nameWithAllowedDots: /^[a-zA-Z0-9_\u4e00-\u9fa5]{1}[\u4e00-\u9fa5\.\w-]*$/,
    contextEngineId: /^[0-9A-F]+$/,
    windowsPath: /^[a-zA-Z]:(\\[\w\u4e00-\u9fa5\s]+)+/,
    linuxPath: /^\/+.*$/,
    unixPath: /^(\/[^\/\\|\;\&\$\<\>\`\'\{\}\(\)\,\[\]\~\*\?\!\s]{1,2048})+$|^\/$/,
    linuxScript: /^.+[\.]{1}(sh){1}$/,
    hdfsScript: /^\/.+[\.]{1}(sh){1}$/,
    windowsScript: /^.+[\.]{1}(bat){1}$/,
    windowsFullPath: /^[a-zA-Z]:(\\)+/,
    nasPath: /^[\\]{2}[^\\]+(\\[^\\]+)+$/,
    nameBegin: /^[\u4e00-\u9fa5a-zA-Z_]/,
    opengaussRestoreName: /^[a-zA-Z_]/,
    nameCombination: /^[\u4e00-\u9fa5a-zA-Z_0-9-]+$/,
    clientRegisterName: /^[a-zA-Z_0-9]+$/,
    dataBaseName: /^[a-zA-Z_0-9]+$/,
    clientRegisterPassword: /^[a-zA-Z0-9~@#_^*%\/.+:;=]+$/,
    domain: /^(?=^.{3,255}$)[a-zA-Z0-9][-a-zA-Z0-9]{0,62}(.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+$/,
    nasshareDomain: /^[a-zA-Z0-9][-_a-zA-Z0-9]{0,62}(\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+$/,
    templateWindowsPath: /^[a-zA-Z]:(\\[^\\\/:*?\"<>|]{1,2048})+$|^[a-zA-Z]:\\$/,
    templatLinuxPath: /^(\/[^\/]{1,2048})+$|^\/$/,
    templateHostLinuxPath: /^(\/[^\/|;&$><`\\!\[\]+]{1,2048})+$|^\/$/,
    nasFileSystemName: /^[a-zA-Z_0-9-.]+$/,
    charset: /^[a-zA-Z_0-9-]+$/,
    linuxNoPathScript: /^[^\/]+[\.]{1}(sh){1}$/,
    windowsNoPathScript: /^[^\\]+[\.]{1}(bat){1}$/,
    communicationComponent: /^[a-zA-Z0-9~!@#$%&*()^]{8,16}$/,
    statefulsetScript: /^\/.+[\.]{1}(sh|py){1}$/,
    nasShareNfsName: /^[a-zA-Z0-9\u4e00-\u9fa5\u3002\uff1f\uff01\uff0c\u3001\uff1b\uff1a\u201c\u201d\u2018\u2019\uff08\uff09\u300a\u300b\u3008\u3009\u3010\u3011\u300e\u300f\u300c\u300d\ufe43\ufe44\u3014\u3015\u2026\u2014\uff5e\ufe4f\uffe5?!\\\"#&%$'()*+-.;<=>@\[\]^_`{|}~\/\u0020]{1,512}$/,
    nasShareCifsName: /^[a-zA-Z0-9\u4e00\u9fa5\u3002\uff1f\uff01\uff0c\u3001\uff1b\uff1a\u201c\u201d\u2018\u2019\uff08\uff09\u300a\u300b\u3008\u3009\u3010\u3011\u300e\u300f\u300c\u300d\ufe43\ufe44\u3014\u3015\u2026\u2014\uff5e\ufe4f\uffe5\!\#\&\%\$\'\(\)\+\-\.\;\=\@\[\]\^\_\`\{\}\~\u0020]{1,512}$/,
    bondingPort: /^[\u4e00-\u9fa5a-zA-Z_0-9-.]+$/,
    label: /^[a-zA-Z0-9](?!.*?[-_.]$)[a-zA-Z0-9-_.]*$/,
    workloadName: /^[\u4e00-\u9fa5a-zA-Z0-9_]{1}[\u4e00-\u9fa5a-zA-Z_0-9-.]{0,49}$/,
    envValue: /^[\u4e00-\u9fa5a-zA-Z0-9_]{1}[\u4e00-\u9fa5a-zA-Z_0-9-.]{0,99}$/,
    urlHttpReg: /http:\/\/[\w\-_]+(\.[\w\-_]+)+([\w\-\.,@?^=%&:/~\+#]*[\w\-\@?^=%&/~\+#])?/,
    urlHttpsReg: /https:\/\/[\w\-_]+(\.[\w\-_]+)+([\w\-\.,@?^=%&:/~\+#]*[\w\-\@?^=%&/~\+#])?/,
    urlReg: /(https|http):\/\/[\w\-_]+(\.[\w\-_]+)+([\w\-\.,@?^=%&:/~\+#]*[\w\-\@?^=%&/~\+#])?/,
    openstackUserName: /^[a-zA-Z_0-9-]+$/,
    cnwareName: /^[\u4e00-\u9fa5a-z0-9A-Z\\_.-]+$/
  }
};

export const quaDrantTable = {
  // 用于X9000端口适配
  A: ['H0', 'H1', 'H2', 'H3', 'H4', 'H5', 'H6'],
  B: ['L0', 'L1', 'L2', 'L3', 'L4', 'L5', 'L6'],
  C: ['H7', 'H8', 'H9', 'H10', 'H11', 'H12', 'H13', 'H14'],
  D: ['L7', 'L8', 'L9', 'L10', 'L11', 'L12', 'L13', 'L14']
};

export const quaDrantTableOther = {
  // 用于X9000端口适配
  A: {
    table: quaDrantTable['A'],
    P0: 'A',
    P1: 'B',
    P2: 'C',
    P3: 'D'
  },
  B: {
    table: quaDrantTable['B'],
    P0: 'B',
    P1: 'C',
    P2: 'D',
    P3: 'A'
  },
  C: {
    table: quaDrantTable['C'],
    P0: 'C',
    P1: 'D',
    P2: 'A',
    P3: 'B'
  },
  D: {
    table: quaDrantTable['D'],
    P0: 'D',
    P1: 'A',
    P2: 'B',
    P3: 'C'
  }
};

export const MultiCluster = {
  isMulti: false,
  esn: '',
  roleType: '',
  isInit: false
};

export const SupportLicense = {
  isFile: true,
  isSan: true,
  isBoth: true,
  isInit: false
};

// 场景
export const Scene = {
  Register: 1,
  Backup: 2,
  Restore: 3
};

// 屏蔽的功能
export const Features = {
  StorageResources: 'StorageResources', // 存储资源
  ClusterType: 'ClusterType', // 集群类型
  SplitTableBackup: 'SplitTableBackup', // 分裂表备份
  LogBackup: 'LogBackup' // 日志备份
};

export const ColorConsts = {
  NORMAL: '#7adfa0', // 绿色
  ABNORMAL: '#F45c5e', // 红色
  RUNNING: '#6CBFFF', // 蓝色，表示运行中或做轻提示
  WARN: '#FDCA5A', // 黄色
  OFFLINE: '#b8becc' // 灰色
};

export const AlarmColorConsts = {
  MAJOR: '#FA8E5A',
  WARNING: '#FDCA5A',
  CRITICAL: '#F45C5E',
  INFO: '#6CBFFF'
};
export const RunningStatusColorConsts = {
  otherStatus: '#6C92FA'
};

export const JobColorConsts = {
  SUCCESSFUL: '#7ADFA0 ',
  RUNNING: '#6CBFFF',
  PENDING: '#B8E0FF',
  ABORTED: '#B8BECC',
  FAILED: '#F45C5E',
  INIT: '#468BC2',
  CANCLELED: '#9EA4B3',
  OTHERS: '#545966'
};

export const HelpUrlCode = {
  host: '0000001839224445',
  database: '0000001918630660',
  bigdata: '0000001948269721',
  virtualization: '0000001918470736',
  container: '0000001918630668',
  cloud: '0000001948269725',
  application: '0000001918470740',
  fileservice: '0000001918630672',
  baremetal: '0000001873679157'
};

// 容量单位
export enum CAPACITY_UNIT {
  BIT = 'BIT',
  BYTE = 'B',
  KB = 'KB',
  MB = 'MB',
  GB = 'GB',
  TB = 'TB',
  PB = 'PB',
  EB = 'EB'
}

export enum CAPACITY_UNIT_EXTRA {
  BIT = 'BIT',
  BYTE = 'B',
  KB = 'KB',
  MB = 'MB',
  GB = 'GB',
  TB = 'TIB',
  PB = 'PB',
  EB = 'EB'
}

// 登录成功状态
export enum LOGIN_RESULT_STATUS {
  SUCCESS = 'success',
  FAILED = 'failed'
}

export const RESET_PSWD_NAVIGATE_STATUS = {
  userId: '',
  userName: '',
  randomCode: ''
};

export const MESSAGE_BOX_ACTION = {
  close: 'close',
  ok: 'ok',
  cancel: 'cancel'
};

export const AllConsts = Object.assign(
  {},
  CommonConsts,
  ColorConsts,
  FILE_FORMAT
);

//不同版本导出日志最大数
export const EXPORT_LOG_MAXMUM = {
  x8000Num: 10,
  x6000Num: 5
};

// 导出接口白名单
export const EXPORT_URL_WHITE_LIST = [
  '^/console/rest/v1/alarms/dump/files/[0-9a-zA-Z]+$',
  '^/console/rest/v1/alarms/action/export$',
  '^/console/rest/v1/events/action/export$',
  '^/console/rest/v1/certs/components/[0-9a-zA-Z]+/request-file/action/export$',
  '^/console/rest/v1/certs/components/[0-9a-zA-Z]+/ca/action/download$',
  '^/console/rest/v1/certs/components/[0-9a-zA-Z]+/crl/action/download$',
  '^/console/rest/v1/jobs/action/export$',
  '^/console/rest/v1/anonymization/report/download$',
  '^/console/rest/v1/sysbackup/images/[0-9a-zA-Z]+/action/download$',
  '^/console/rest/v1/infra/logs/export$',
  '^/console/rest/v1/host-agent/download$',
  '^/console/rest/v1/license/action/export$',
  '^/console/rest/v1/flr/download$',
  '^/console/rest/v1/export-files/[0-9a-zA-Z-]+/action/download',
  '^/console/rest/v1/anti/ransomware/io-detect/report/export$',
  '^/console/rest/v1/certs/components/[0-9a-zA-Z-]+/request-file/action/export-oceancyber$',
  '^/console/rest/v1/anti/ransomware/detect/copy/action/abnormal/download$'
];

// 异步任务接口白名单
export const ASYNC_TASK_URL_WHITE_LIST = [
  {
    url: '^/console/rest/v1/protected-objects/[0-9a-zA-Z-]+/action/backup$',
    method: 'post'
  }, // 备份
  { url: '^/console/rest/v1/restores$', method: 'post' }, // 恢复、即时恢复
  { url: '^/console/rest/v1/live-mount$', method: 'post' }, // 即时挂载
  {
    url: '^/console/rest/v1/storages/[0-9a-zA-Z-]+/action/import$',
    method: 'post'
  }, // 归档归档副本
  { url: '^/console/rest/v1/copies/[0-9a-zA-Z-]+$', method: 'delete' }, // 副本删除
  { url: '^/console/rest/v1/live-mount/[0-9a-zA-Z-]+$', method: 'delete' }, // 即时挂载的卸载
  {
    url: '^/console/rest/v2/copies/[0-9a-zA-Z-]+/action/verify$',
    method: 'post'
  }, // 副本校验
  {
    url: '^/console/rest/v1/live-mount/[0-9a-zA-Z-]+/action/update$',
    method: 'put'
  }, // 即时挂载的更新
  { url: '^/console/rest/v1/anonymization/anonymization-job$', method: 'post' }, // 敏感数据识别、数据脱敏
  {
    url: '^/console/rest/v1/live-mount/[0-9a-zA-Z-]+/action/migrate$',
    method: 'put'
  }, // LiveMount迁移
  { url: '^/console/rest/v1/environments', method: 'post' }, // 扫描
  { url: '^/console/rest/v1/protected-objects/batch$', method: 'post' }, // 资源批量保护
  { url: '^/console/rest/v1/protected-objects$', method: 'put' }, // 资源修改保护
  { url: '^/console/rest/v1/host-agent/register$', method: 'post' }, // 注册客户端
  { url: '^/console/rest/v1/host-agent/action/update$', method: 'post' },
  { url: '^/console/rest/v1/kubernetes$', method: 'post' }, // 注册k8s集群
  { url: '^/console/rest/v2/restore/jobs$', method: 'post' },
  {
    url: '^/console/rest/v2/environments/[0-9a-zA-Z-]+/action/scan$',
    method: 'put'
  },
  {
    url: '^/console/rest/v1/export-files$',
    method: 'post'
  },
  {
    url: '^/console/rest/v1/replication$',
    method: 'post'
  },
  {
    url: '^/console/rest/v1/export-files$',
    method: 'post'
  },
  {
    url:
      '^/console/rest/v1/protected-objects/[0-9a-zA-Z-]+/action/backup-cyber$',
    method: 'post'
  },
  {
    url: '^/console/rest/v1/anti-ransomware/action/detect-cyber$',
    method: 'post'
  },
  {
    url: '^/console/rest/v1/protected-objects-cyber/batch$',
    method: 'post'
  },
  {
    url: '^/console/rest/v1/protected-objects-cyber$',
    method: 'put'
  },
  {
    url: '^/console/rest/v1/live-mount/cyber$',
    method: 'post'
  }
];
export interface WizardStepDataHandler {
  // 步骤校验
  isValid(): boolean;
  // 进入步骤的初始化调用
  initData(data: any): void;
  // 步骤完成时对数据的存储
  saveData(): any;
}

// 角色
export enum ROLE {
  SYS_ADMIN = 'sys_admin',
  ADMIN = 'admin',
  COMMON_USER = 'common_user',
  AUDITOR = 'auditor'
}

// HTTP请求状态
export enum HTTP_STATUS {
  SYSTEM_ERROR = 500,
  NOT_FOUND = 404,
  FORBIDDEN = 403,
  UNAUTHORIZED = 401,
  PARAM_ERROR = 400,
  TIMEOUT = 302,
  NORMAL = 200,
  GATEWAYTIMOUT = 504
}

export enum ErrorCode {
  // 连续三次修改密码错误，用户被强制退出
  ReachedTheMaxFailed = '1677929501',
  // 用户密码未修改，跳转到登录界面
  UserPasswordNotChange = '1677929504',
  // 用户状态异常，跳转到登录界面
  UserStatusAbnormal = '1677929505'
}

export enum CatalogName {
  HostApps = 'Host Apps',
  Virtualization = 'Virtualization',
  Cloud = 'Cloud',
  BigData = 'BigData',
  Copies = 'Copies',
  Storage = 'Storage',
  Vessel = 'Vessel',
  Application = 'Application'
}

export enum Modify_Network_View_Type {
  BackupView = 1,
  ArchiveView
}

export enum Table_Size {
  Default = 'default',
  Small = 'small'
}

export enum Report_Agent_Type {
  built_in = '2',
  built_out = '1'
}

export enum Page_Size_Options {
  Three = 3,
  Ten = 10,
  Twenty = 20,
  Fifty = 50,
  OneHundred = 100
}

export enum LogoutType {
  Manual = 'MANUAL',
  Timeout = 'TIMEOUT'
}

export enum ResourceOperationType {
  deletion,
  protection
}

export enum CopyFeatureEnum {
  INDEX = 0,
  RESTORE = 1,
  INSTANT_RESTORE = 2,
  MOUNT = 3,
  WORM = 4
}

export enum WormStatusEnum {
  UNSET = 1,
  SETTING = 2,
  SET_SUCCESS = 3,
  SET_FAILED = 4
}

export enum GenerationType {
  // # The copy is generated by backup
  BY_BACKUP = 'Backup',
  // # The copy is generated by cloud archive
  BY_CLOUD_ARCHIVE = 'CloudArchive',
  // # The copy is generated by tape archive
  BY_TAPE_ARCHIVE = 'TapeArchive',
  // # The copy is generated by live mount
  BY_LIVE_MOUNTE = 'live_mount',
  // # The copy is generated by replicated
  BY_REPLICATED = 'Replicated',
  // # The copy is generated by Imported
  BY_IMPORTED = 'Imported',
  // # The copy is generated by common interface backup
  BY_COMMON_INTERFACE_BACKUP = 'CommonInterfaceBackup',
  // # dws下载副本，dws归档到磁带的副本需要下载到A8000才能恢复，其他应用不用下载可以直接恢复
  BY_DOWNLOAD = 'Download',
  // # 反向复制
  BY_REVERSE_REPLICATION = 'ReverseReplication',
  // # 级联复制
  BY_CASCADED_REPLICATION = 'CascadedReplication'
}

export const keyWidthCache = [];

export const EMIT_TASK = 'emit_task';

export const SUB_APP_REFRESH_FLAG = {
  emit: true
};

export const cyberEngineMap = {
  operation_target_sla_label: 'explore_intelligent_detection_policy_label',
  '0x206403330002.alarm.desc': 'insight_cyberengine_delete_sla_label',
  '0x206403330002.alarm.desc.detail': 'insight_cyberengine_delete_sla_label',
  '0x206403330003.alarm.desc': 'insight_cyberengine_modify_sla_label',
  '0x206403330003.alarm.desc.detail': 'insight_cyberengine_modify_sla_label',
  '0x206403330001.alarm.desc': 'insight_cyberengine_create_sla_label',
  '0x206403330001.alarm.desc.detail': 'insight_cyberengine_create_sla_label',
  '0x205F02590001.alarm.desc': 'insight_cyberengine_remove_sla_pair_label',
  '0x205F02590001.alarm.desc.detail':
    'insight_cyberengine_remove_sla_pair_label',
  '0x206403330002.alarm.name': 'insight_cyberengine_delete_title_sla_label',
  '0x206403330001.alarm.name': 'insight_cyberengine_create_title_sla_label',
  '0x206403330003.alarm.name': 'insight_cyberengine_modify_title_sla_label',
  common_live_mount_label: 'common_restore_shared_path_label',
  job_log_live_mount_execute_label: 'common_log_live_mount_execute_label',
  job_log_live_mount_request_label: 'common_log_live_mount_request_label',
  nas_plugin_livemount_nfs_mountinfo_label:
    'common_nas_plugin_livemount_nfs_mountinfo_label',
  dme_livemount_complete_label: 'common_dme_livemount_complete_label',
  job_log_live_mount_complete_label: 'common_job_log_live_mount_complete_label',
  common_backup_label: 'common_anti_detection_gen_label',
  job_log_protection_backup_execute_label:
    'common_job_log_protection_execute_label',
  dme_prepare_backup_label: 'common_dme_prepare_backup_label',
  dme_prepare_backup_failed_label: 'common_dme_prepare_backup_failed_label',
  dme_backup_data_complete_label: 'common_dme_backup_data_complete_label',
  dme_create_backup_copy_label: 'common_dme_create_backup_copy_label',
  unlock_running_label: 'common_unlock_running_label',
  lock_running_label: 'common_lock_running_label',
  dme_prepare_livemount_label: 'common_dme_prepare_livemount_label',
  insight_show_expired_copy_label: 'insight_show_expired_snapshot_label',
  insight_job_type_copydata_deletion_label:
    'insight_job_type_snapshot_deletion_label',
  common_copies_label: 'common_snapshot_label',
  dme_delete_snap_task_ok_label: 'insight_dme_delete_snap_task_ok_label',
  dme_delete_snap_task_fail_label: 'insight_dme_delete_snap_task_fail_label',
  job_log_copy_delete_exec_label: 'insight_job_log_copy_delete_exec_label',
  dme_databases_restore_task_prepare_label:
    'insight_dme_databases_restore_task_prepare_label',
  dme_vmware_delete_snap_label: 'insight_dme_vmware_delete_snap_label',
  dme_prepare_delete_snap_label: 'insight_dme_prepare_delete_snap_label',
  dme_archive_delete_info_label: 'insight_dme_archive_delete_info_label',
  dme_archive_delete_prepare_label: 'insight_dme_archive_delete_prepare_label',
  dme_archive_delete_prepare_failed_label:
    'insight_dme_archive_delete_prepare_failed_label',
  dme_archive_delete_metadate_failed_label:
    'insight_dme_archive_delete_metadate_failed_label',
  common_expire_copy_label: 'insight_expire_copy_label',
  dme_prepare_delete_copy_label: 'insight_dme_prepare_delete_copy_label',
  dme_prepare_copy_verify_label: 'insight_dme_prepare_copy_verify_label',
  dme_prepare_copy_verify_failed_label:
    'insight_dme_prepare_copy_verify_failed_label',
  dme_copy_verify_complete_task_label:
    'insight_dme_copy_verify_complete_task_label',
  dme_delete_copy_success_warning_label:
    'insight_dme_delete_copy_success_warning_label',
  operation_target_copycatalog_label: 'operation_target_snapshot_label',
  '0x6403440001.alarm.advice': 'common_active_port_failed_alaram_advice_label',
  '0x6403440002.alarm.advice':
    'common_deactivative_port_failed_alaram_advice_label',
  SLA: 'common_intelligent_detection_policy_label',
  insight_job_copytime_label: 'common_hyperdetect_time_stamp_label',
  common_time_stamp_label: 'common_hyperdetect_time_stamp_label',
  job_log_resource_protect_execute_success_label:
    'job_log_cyberengine_resource_protect_execute_success_label',
  job_log_resource_protect_execute_failed_label:
    'job_log_cyberengine_resource_protect_execute_failed_label',
  common_delete_copy_label: 'protection_snap_delete_label',
  common_copy_data_label: 'common_hyperdetect_copy_data_label',
  job_log_live_mount_delete_clone_copy_label:
    'job_log_live_mount_delete_clone_snapshot_label',
  job_log_live_mount_copy_clone_label: 'job_log_live_mount_snapshot_clone_label'
};

export const distributedMap = {
  explore_detected_immediately_label:
    'explore_detected_immediately_distributed_label'
};

export const dataBackupX3000Map = {
  operation_target_antiransomwarepolicy_label: 'common_worm_policy_label',
  '0x206403440002.alarm.name': 'common_modify_worm_policy_name_label',
  '0x206403440002.alarm.desc': 'common_modify_worm_policy_desc_label',
  '0x206403440002.alarm.desc.detail': 'common_modify_worm_policy_desc_label',
  '0x206403440001.alarm.name': 'common_delete_worm_policy_name_label',
  '0x206403440001.alarm.desc': 'common_delete_worm_policy_desc_label',
  '0x206403440001.alarm.desc.detail': 'common_delete_worm_policy_desc_label',
  '0x206403440003.alarm.name': 'common_create_worm_policy_name_label',
  '0x206403440003.alarm.desc': 'common_create_worm_policy_desc_label',
  '0x206403440003.alarm.desc.detail': 'common_create_worm_policy_desc_label'
};

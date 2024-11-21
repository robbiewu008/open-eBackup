naviData = [
  {
    id: 1,
    parentId: 0,
    name: '简介',
    local: 'helpcenter_000001.html',
    children: [
      {
        id: 8,
        parentId: 1,
        name: '关于本产品',
        local: 'helpcenter_000002.html'
      },
      { id: 9, parentId: 1, name: '快速入门', local: 'helpcenter_000003.html' },
      {
        id: 10,
        parentId: 1,
        name: '个人数据隐私声明',
        local: 'helpcenter_000004.html'
      }
    ]
  },
  { id: 2, parentId: 0, name: '首页', local: 'helpcenter_000005.html' },
  {
    id: 3,
    parentId: 0,
    name: '保护',
    local: 'helpcenter_000006.html',
    children: [
      { id: 11, parentId: 3, name: '总览', local: 'helpcenter_000007.html' },
      {
        id: 12,
        parentId: 3,
        name: '主机',
        local: 'helpcenter_000008.html',
        children: [
          {
            id: 20,
            parentId: 12,
            name: '安装ProtectAgent（自动推送方式，适用于1.6.0及后续版本）',
            local: 'protectagent_install_0017.html'
          },
          {
            id: 21,
            parentId: 12,
            name: '管理ProtectAgent软件包',
            local: 'protectagent_install_0028.html',
            children: [
              {
                id: 23,
                parentId: 21,
                name: '下载ProtectAgent软件包',
                local: 'protectagent_install_0030.html'
              }
            ]
          },
          {
            id: 22,
            parentId: 12,
            name: '管理代理主机',
            local: 'protectagent_install_0031.html',
            children: [
              {
                id: 24,
                parentId: 22,
                name: '查看代理主机信息',
                local: 'protectagent_install_0032.html'
              },
              {
                id: 25,
                parentId: 22,
                name: '管理代理主机',
                local: 'protectagent_install_0033.html'
              }
            ]
          }
        ]
      },
      {
        id: 13,
        parentId: 3,
        name: '数据库',
        local: 'zh-cn_topic_0000001918630660.html',
        children: [
          {
            id: 26,
            parentId: 13,
            name: 'Oracle数据保护',
            local: 'product_documentation_000025.html',
            children: [
              {
                id: 43,
                parentId: 26,
                name: '备份',
                local: 'oracle_gud_0008.html',
                children: [
                  {
                    id: 52,
                    parentId: 43,
                    name: '备份前准备',
                    local: 'oracle_gud_0012.html'
                  },
                  {
                    id: 53,
                    parentId: 43,
                    name: '备份Oracle数据库',
                    local: 'oracle_gud_0013.html',
                    children: [
                      {
                        id: 55,
                        parentId: 53,
                        name: '步骤1：检查并配置数据库环境',
                        local: 'oracle_gud_0015.html',
                        children: [
                          {
                            id: 64,
                            parentId: 55,
                            name: '检查并配置Oracle数据库的Open状态',
                            local: 'oracle_gud_0016.html'
                          },
                          {
                            id: 65,
                            parentId: 55,
                            name: '检查并配置Oracle数据库的归档模式',
                            local: 'oracle_gud_0017.html'
                          },
                          {
                            id: 66,
                            parentId: 55,
                            name: '检查快照控制文件的位置',
                            local: 'oracle_gud_0020.html'
                          },
                          {
                            id: 67,
                            parentId: 55,
                            name: '检查集群数据的存放位置',
                            local: 'oracle_gud_00201.html'
                          }
                        ]
                      },
                      {
                        id: 56,
                        parentId: 53,
                        name:
                          '步骤2：获取存储资源CA证书（适用于存储层快照备份）',
                        local: 'oracle_gud_ca.html'
                      },
                      {
                        id: 57,
                        parentId: 53,
                        name: '步骤3：注册集群',
                        local: 'oracle_gud_0022.html'
                      },
                      {
                        id: 58,
                        parentId: 53,
                        name: '步骤4：注册数据库',
                        local: 'oracle_gud_0023.html'
                      },
                      {
                        id: 59,
                        parentId: 53,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'oracle_gud_0024.html'
                      },
                      {
                        id: 60,
                        parentId: 53,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'oracle_gud_0025.html'
                      },
                      {
                        id: 61,
                        parentId: 53,
                        name: '步骤7：创建备份SLA',
                        local: 'oracle_gud_0026.html'
                      },
                      {
                        id: 62,
                        parentId: 53,
                        name: '步骤8：开启BCT（适用于RMAN备份）',
                        local: 'oracle_gud_0027.html'
                      },
                      {
                        id: 63,
                        parentId: 53,
                        name: '步骤9：执行备份',
                        local: 'oracle_gud_0028.html'
                      }
                    ]
                  },
                  {
                    id: 54,
                    parentId: 43,
                    name: '（可选）同步Trap配置至Oracle主机',
                    local: 'oracle_gud_0031.html'
                  }
                ]
              },
              {
                id: 44,
                parentId: 26,
                name: '复制',
                local: 'oracle_gud_0032.html',
                children: [
                  {
                    id: 68,
                    parentId: 44,
                    name: '复制Oracle数据库副本',
                    local: 'oracle_gud_0034.html',
                    children: [
                      {
                        id: 69,
                        parentId: 68,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'oracle_gud_0036.html'
                      },
                      {
                        id: 70,
                        parentId: 68,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'oracle_gud_0036_1.html'
                      },
                      {
                        id: 71,
                        parentId: 68,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'oracle_gud_0037.html'
                      },
                      {
                        id: 72,
                        parentId: 68,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'oracle_gud_0038.html'
                      },
                      {
                        id: 73,
                        parentId: 68,
                        name: '步骤4：下载并导入证书',
                        local: 'oracle_gud_0039.html'
                      },
                      {
                        id: 74,
                        parentId: 68,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'vmware_gud_0040_0.html'
                      },
                      {
                        id: 75,
                        parentId: 68,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002097717489.html'
                      },
                      {
                        id: 76,
                        parentId: 68,
                        name: '步骤6：添加复制集群',
                        local: 'oracle_gud_0041.html'
                      },
                      {
                        id: 77,
                        parentId: 68,
                        name: '步骤7：创建复制SLA',
                        local: 'oracle_gud_0042.html'
                      },
                      {
                        id: 78,
                        parentId: 68,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'oracle_gud_0043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 45,
                parentId: 26,
                name: '归档',
                local: 'oracle_gud_0044.html',
                children: [
                  {
                    id: 79,
                    parentId: 45,
                    name: '归档Oracle备份副本',
                    local: 'oracle_gud_0047.html',
                    children: [
                      {
                        id: 81,
                        parentId: 79,
                        name: '步骤1：添加归档存储',
                        local: 'oracle_gud_0048.html',
                        children: [
                          {
                            id: 83,
                            parentId: 81,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'oracle_gud_0049.html'
                          },
                          {
                            id: 84,
                            parentId: 81,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'oracle_gud_0050.html'
                          }
                        ]
                      },
                      {
                        id: 82,
                        parentId: 79,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'oracle_gud_0051.html'
                      }
                    ]
                  },
                  {
                    id: 80,
                    parentId: 45,
                    name: '归档Oracle复制副本',
                    local: 'oracle_gud_0052.html',
                    children: [
                      {
                        id: 85,
                        parentId: 80,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'oracle_gud_0053.html'
                      },
                      {
                        id: 86,
                        parentId: 80,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'oracle_gud_0054.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 46,
                parentId: 26,
                name: '恢复',
                local: 'oracle_gud_0055.html',
                children: [
                  {
                    id: 87,
                    parentId: 46,
                    name: '恢复Oracle数据库',
                    local: 'oracle_gud_0058.html'
                  },
                  {
                    id: 88,
                    parentId: 46,
                    name:
                      '恢复Oracle数据库中的单个表或多个表（适用于1.6.0及后续版本）',
                    local: 'oracle_gud_0131.html'
                  },
                  {
                    id: 89,
                    parentId: 46,
                    name:
                      '恢复Oracle数据库中的单个或多个文件（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002028898929.html'
                  }
                ]
              },
              {
                id: 47,
                parentId: 26,
                name: '即时恢复',
                local: 'oracle_gud_0059.html',
                children: [
                  {
                    id: 90,
                    parentId: 47,
                    name: '即时恢复Oracle数据库',
                    local: 'oracle_gud_0062.html'
                  }
                ]
              },
              {
                id: 48,
                parentId: 26,
                name: '全局搜索',
                local: 'oracle_gud_0072.html',
                children: [
                  {
                    id: 91,
                    parentId: 48,
                    name: '全局搜索资源',
                    local: 'oracle_gud_0073.html'
                  },
                  {
                    id: 92,
                    parentId: 48,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002038747309.html'
                  }
                ]
              },
              {
                id: 49,
                parentId: 26,
                name: 'SLA',
                local: 'oracle_gud_0076.html',
                children: [
                  {
                    id: 93,
                    parentId: 49,
                    name: '查看SLA信息',
                    local: 'oracle_gud_0078.html'
                  },
                  {
                    id: 94,
                    parentId: 49,
                    name: '管理SLA',
                    local: 'oracle_gud_0079.html'
                  }
                ]
              },
              {
                id: 50,
                parentId: 26,
                name: '副本',
                local: 'oracle_gud_0080.html',
                children: [
                  {
                    id: 95,
                    parentId: 50,
                    name: '查看Oracle副本信息',
                    local: 'oracle_gud_0081.html'
                  },
                  {
                    id: 96,
                    parentId: 50,
                    name: '管理Oracle副本',
                    local: 'oracle_gud_0082.html'
                  }
                ]
              },
              {
                id: 51,
                parentId: 26,
                name: 'Oracle数据库环境',
                local: 'oracle_gud_0083.html',
                children: [
                  {
                    id: 97,
                    parentId: 51,
                    name: '查看Oracle数据库环境信息',
                    local: 'oracle_gud_0084.html'
                  },
                  {
                    id: 98,
                    parentId: 51,
                    name: '管理数据库',
                    local: 'oracle_gud_0085.html'
                  },
                  {
                    id: 99,
                    parentId: 51,
                    name: '管理数据库集群',
                    local: 'oracle_gud_0086.html'
                  }
                ]
              }
            ]
          },
          {
            id: 27,
            parentId: 13,
            name: 'MySQL/MariaDB/GreatSQL数据保护',
            local: 'zh-cn_topic_0000001826879872.html',
            children: [
              {
                id: 100,
                parentId: 27,
                name: '备份',
                local: 'mysql-0005.html',
                children: [
                  {
                    id: 108,
                    parentId: 100,
                    name: '备份前准备',
                    local: 'mysql-0008.html'
                  },
                  {
                    id: 109,
                    parentId: 100,
                    name: '备份MySQL/MariaDB/GreatSQL数据库',
                    local: 'mysql-0009.html',
                    children: [
                      {
                        id: 110,
                        parentId: 109,
                        name: '步骤1：开启MySQL/MariaDB/GreatSQL数据库权限',
                        local: 'mysql-0010.html'
                      },
                      {
                        id: 111,
                        parentId: 109,
                        name: '步骤2：手动配置软连接',
                        local: 'mysql-0014.html'
                      },
                      {
                        id: 112,
                        parentId: 109,
                        name: '步骤3：手动安装备份工具',
                        local: 'mysql-0011.html',
                        children: [
                          {
                            id: 119,
                            parentId: 112,
                            name: '安装Mariabackup',
                            local: 'mysql-0012.html'
                          },
                          {
                            id: 120,
                            parentId: 112,
                            name: '安装Percona XtraBackup工具依赖软件',
                            local: 'mysql-0013.html'
                          }
                        ]
                      },
                      {
                        id: 113,
                        parentId: 109,
                        name: '步骤4：MySQL/MariaDB/GreatSQL数据库开启日志模式',
                        local: 'mysql-0015.html'
                      },
                      {
                        id: 114,
                        parentId: 109,
                        name: '步骤5：注册MySQL/MariaDB/GreatSQL数据库',
                        local: 'mysql-0016.html'
                      },
                      {
                        id: 115,
                        parentId: 109,
                        name: '步骤6：创建限速策略',
                        local: 'mysql-0017.html'
                      },
                      {
                        id: 116,
                        parentId: 109,
                        name: '步骤7：（可选）开启备份链路加密开关',
                        local: 'mysql-0018.html'
                      },
                      {
                        id: 117,
                        parentId: 109,
                        name: '步骤8：创建备份SLA',
                        local: 'mysql-0019.html'
                      },
                      {
                        id: 118,
                        parentId: 109,
                        name: '步骤9：执行备份',
                        local: 'mysql-0020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 101,
                parentId: 27,
                name: '复制',
                local: 'oracle_gud_000035_12.html',
                children: [
                  {
                    id: 121,
                    parentId: 101,
                    name: '复制MySQL/MariaDB/GreatSQL数据库副本',
                    local: 'mysql-0025.html',
                    children: [
                      {
                        id: 122,
                        parentId: 121,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_13.html'
                      },
                      {
                        id: 123,
                        parentId: 121,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_25.html'
                      },
                      {
                        id: 124,
                        parentId: 121,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'mysql-0028.html'
                      },
                      {
                        id: 125,
                        parentId: 121,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'mysql-0029.html'
                      },
                      {
                        id: 126,
                        parentId: 121,
                        name: '步骤4：下载并导入证书',
                        local: 'mysql-0030.html'
                      },
                      {
                        id: 127,
                        parentId: 121,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'mysql-0031.html'
                      },
                      {
                        id: 128,
                        parentId: 121,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'mysql-00311.html'
                      },
                      {
                        id: 129,
                        parentId: 121,
                        name: '步骤：添加目标集群',
                        local: 'mysql-0032.html'
                      },
                      {
                        id: 130,
                        parentId: 121,
                        name: '步骤7：创建复制SLA',
                        local: 'mysql-0033.html'
                      },
                      {
                        id: 131,
                        parentId: 121,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'mysql-0034.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 102,
                parentId: 27,
                name: '归档',
                local: 'mysql-0035.html',
                children: [
                  {
                    id: 132,
                    parentId: 102,
                    name: '归档MySQL/MariaDB/GreatSQL备份副本',
                    local: 'mysql-0038.html',
                    children: [
                      {
                        id: 134,
                        parentId: 132,
                        name: '步骤1：添加归档存储',
                        local: 'mysql-0039.html',
                        children: [
                          {
                            id: 136,
                            parentId: 134,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'mysql-0040.html'
                          },
                          {
                            id: 137,
                            parentId: 134,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'mysql-0041.html'
                          }
                        ]
                      },
                      {
                        id: 135,
                        parentId: 132,
                        name: '步骤2：创建归档SLA',
                        local: 'mysql-0042.html'
                      }
                    ]
                  },
                  {
                    id: 133,
                    parentId: 102,
                    name: '归档MySQL/MariaDB/GreatSQL复制副本',
                    local: 'mysql-0043.html',
                    children: [
                      {
                        id: 138,
                        parentId: 133,
                        name: '步骤1：创建归档SLA',
                        local: 'mysql-0044.html'
                      },
                      {
                        id: 139,
                        parentId: 133,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'mysql-0045.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 103,
                parentId: 27,
                name: '恢复',
                local: 'mysql-0046.html',
                children: [
                  {
                    id: 140,
                    parentId: 103,
                    name: '恢复MySQL/MariaDB/GreatSQL数据库',
                    local: 'mysql-0049.html'
                  }
                ]
              },
              {
                id: 104,
                parentId: 27,
                name: '全局搜索',
                local: 'mysql-0050.html',
                children: [
                  {
                    id: 141,
                    parentId: 104,
                    name: '全局搜索资源',
                    local: 'mysql-0052.html'
                  },
                  {
                    id: 142,
                    parentId: 104,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'mysql-00522.html'
                  }
                ]
              },
              {
                id: 105,
                parentId: 27,
                name: 'SLA',
                local: 'mysql-0068.html',
                children: [
                  {
                    id: 143,
                    parentId: 105,
                    name: '关于SLA',
                    local: 'mysql-0069.html'
                  },
                  {
                    id: 144,
                    parentId: 105,
                    name: '查看SLA信息',
                    local: 'mysql-0070.html'
                  },
                  {
                    id: 145,
                    parentId: 105,
                    name: '管理SLA',
                    local: 'mysql-0071.html'
                  }
                ]
              },
              {
                id: 106,
                parentId: 27,
                name: '副本',
                local: 'mysql-0072.html',
                children: [
                  {
                    id: 146,
                    parentId: 106,
                    name: '查看MySQL/MariaDB/GreatSQL副本信息',
                    local: 'mysql-0073.html'
                  },
                  {
                    id: 147,
                    parentId: 106,
                    name: '管理MySQL/MariaDB/GreatSQL副本',
                    local: 'mysql-0074.html'
                  }
                ]
              },
              {
                id: 107,
                parentId: 27,
                name: 'MySQL/MariaDB/GreatSQL数据库环境',
                local: 'mysql-0075.html',
                children: [
                  {
                    id: 148,
                    parentId: 107,
                    name: '查看MySQL/MariaDB/GreatSQL数据库环境信息',
                    local: 'mysql-0076.html'
                  },
                  {
                    id: 149,
                    parentId: 107,
                    name: '管理数据库',
                    local: 'mysql-0077.html'
                  },
                  {
                    id: 150,
                    parentId: 107,
                    name: '管理数据库实例',
                    local: 'zh-cn_topic_0000001883230926.html'
                  },
                  {
                    id: 151,
                    parentId: 107,
                    name: '管理数据库集群',
                    local: 'mysql-0078.html'
                  }
                ]
              }
            ]
          },
          {
            id: 28,
            parentId: 13,
            name: 'SQL Server数据保护',
            local: 'zh-cn_topic_0000001826879832.html',
            children: [
              {
                id: 152,
                parentId: 28,
                name: '备份',
                local: 'sql-0008.html',
                children: [
                  {
                    id: 160,
                    parentId: 152,
                    name: '备份前准备',
                    local: 'sql-0011.html'
                  },
                  {
                    id: 161,
                    parentId: 152,
                    name: '备份SQL Server数据库',
                    local: 'sql-0012.html',
                    children: [
                      {
                        id: 162,
                        parentId: 161,
                        name: '步骤1：检查并配置数据库环境',
                        local: 'sql-0013.html'
                      },
                      {
                        id: 163,
                        parentId: 161,
                        name: '步骤2：设置Windows PowerShell权限',
                        local: 'sql-0014.html'
                      },
                      {
                        id: 164,
                        parentId: 161,
                        name: '步骤4：开启sysadmin权限',
                        local: 'sql-0016.html'
                      },
                      {
                        id: 165,
                        parentId: 161,
                        name: '步骤5：设置日志备份恢复模式',
                        local: 'sql-0017.html'
                      },
                      {
                        id: 166,
                        parentId: 161,
                        name: '步骤3：注册SQL Server数据库',
                        local: 'sql-0015.html'
                      },
                      {
                        id: 167,
                        parentId: 161,
                        name: '步骤6：创建限速策略',
                        local: 'sql-0018.html'
                      },
                      {
                        id: 168,
                        parentId: 161,
                        name: '步骤7：（可选）开启备份链路加密开关',
                        local: 'zh-cn_topic_0000002051554157.html'
                      },
                      {
                        id: 169,
                        parentId: 161,
                        name: '步骤8：创建备份SLA',
                        local: 'sql-0020.html'
                      },
                      {
                        id: 170,
                        parentId: 161,
                        name: '步骤9：执行备份',
                        local: 'sql-0021.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 153,
                parentId: 28,
                name: '复制',
                local: 'sql-0024.html',
                children: [
                  {
                    id: 171,
                    parentId: 153,
                    name: '复制SQL Server数据库副本',
                    local: 'sql-0027.html',
                    children: [
                      {
                        id: 172,
                        parentId: 171,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'sql-0029.html'
                      },
                      {
                        id: 173,
                        parentId: 171,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'sql-0030.html'
                      },
                      {
                        id: 174,
                        parentId: 171,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'sql-0031.html'
                      },
                      {
                        id: 175,
                        parentId: 171,
                        name: '步骤3：可选：开启复制链路加密开关',
                        local: 'sql-0032.html'
                      },
                      {
                        id: 176,
                        parentId: 171,
                        name: '步骤4：下载并导入证书',
                        local: 'sql-0033.html'
                      },
                      {
                        id: 177,
                        parentId: 171,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'sql-0034.html'
                      },
                      {
                        id: 178,
                        parentId: 171,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002010400808.html'
                      },
                      {
                        id: 179,
                        parentId: 171,
                        name: '步骤：添加复制集群',
                        local: 'sql-0035.html'
                      },
                      {
                        id: 180,
                        parentId: 171,
                        name: '步骤：创建复制SLA',
                        local: 'sql-0036.html'
                      },
                      {
                        id: 181,
                        parentId: 171,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'sql-0037.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 154,
                parentId: 28,
                name: '归档',
                local: 'sql-0038.html',
                children: [
                  {
                    id: 182,
                    parentId: 154,
                    name: '归档SQL Server备份副本',
                    local: 'sql-0041.html',
                    children: [
                      {
                        id: 184,
                        parentId: 182,
                        name: '步骤1：添加归档存储',
                        local: 'sql-0042.html',
                        children: [
                          {
                            id: 186,
                            parentId: 184,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'sql-0043.html'
                          },
                          {
                            id: 187,
                            parentId: 184,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'sql-0044.html'
                          }
                        ]
                      },
                      {
                        id: 185,
                        parentId: 182,
                        name: '步骤2：创建归档SLA',
                        local: 'sql-0045.html'
                      }
                    ]
                  },
                  {
                    id: 183,
                    parentId: 154,
                    name: '归档SQL Server复制副本',
                    local: 'sql-0046.html',
                    children: [
                      {
                        id: 188,
                        parentId: 183,
                        name: '步骤1：创建归档SLA',
                        local: 'sql-0047.html'
                      },
                      {
                        id: 189,
                        parentId: 183,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'sql-0048.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 155,
                parentId: 28,
                name: '恢复',
                local: 'sql-0049.html',
                children: [
                  {
                    id: 190,
                    parentId: 155,
                    name: '恢复SQL Server数据库',
                    local: 'sql-0052.html'
                  },
                  {
                    id: 191,
                    parentId: 155,
                    name: '恢复SQL Server实例中的单个或多个数据库',
                    local: 'sql-0152.html'
                  }
                ]
              },
              {
                id: 156,
                parentId: 28,
                name: '全局搜索',
                local: 'sql-0053.html',
                children: [
                  {
                    id: 192,
                    parentId: 156,
                    name: '全局搜索资源',
                    local: 'sql-0054.html'
                  },
                  {
                    id: 193,
                    parentId: 156,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'sql-0055.html'
                  }
                ]
              },
              {
                id: 157,
                parentId: 28,
                name: 'SLA',
                local: 'sql-0058.html',
                children: [
                  {
                    id: 194,
                    parentId: 157,
                    name: '查看SLA信息',
                    local: 'sql-0060.html'
                  },
                  {
                    id: 195,
                    parentId: 157,
                    name: '管理SLA',
                    local: 'sql-0061.html'
                  }
                ]
              },
              {
                id: 158,
                parentId: 28,
                name: '副本',
                local: 'sql-0062.html',
                children: [
                  {
                    id: 196,
                    parentId: 158,
                    name: '查看SQL Server副本信息',
                    local: 'sql-0063.html'
                  },
                  {
                    id: 197,
                    parentId: 158,
                    name: '管理SQL Server副本',
                    local: 'sql-0064.html'
                  }
                ]
              },
              {
                id: 159,
                parentId: 28,
                name: 'SQL Server数据库环境',
                local: 'sql-0065.html',
                children: [
                  {
                    id: 198,
                    parentId: 159,
                    name: '查看SQL Server数据库环境信息',
                    local: 'sql-0066.html'
                  },
                  {
                    id: 199,
                    parentId: 159,
                    name: '管理SQL Server',
                    local: 'sql-0067.html'
                  },
                  {
                    id: 200,
                    parentId: 159,
                    name: '管理SQL Server数据库集群',
                    local: 'sql-0068.html'
                  }
                ]
              }
            ]
          },
          {
            id: 29,
            parentId: 13,
            name: 'PostgreSQL数据保护',
            local: 'zh-cn_topic_0000001826879840.html',
            children: [
              {
                id: 201,
                parentId: 29,
                name: '备份',
                local: 'postgresql-0005.html',
                children: [
                  {
                    id: 209,
                    parentId: 201,
                    name: '备份前准备',
                    local: 'postgresql-0008.html'
                  },
                  {
                    id: 210,
                    parentId: 201,
                    name: '备份PostgreSQL',
                    local: 'postgresql-0009.html',
                    children: [
                      {
                        id: 211,
                        parentId: 210,
                        name:
                          '步骤1：检查并开启PostgreSQL数据库安装用户sudo权限',
                        local: 'zh-cn_topic_0000001951390817.html'
                      },
                      {
                        id: 212,
                        parentId: 210,
                        name: '步骤2：开启归档模式',
                        local: 'postgresql-0010_0.html'
                      },
                      {
                        id: 213,
                        parentId: 210,
                        name: '步骤3：注册PostgreSQL单实例下的数据库',
                        local: 'postgresql-0011.html'
                      },
                      {
                        id: 214,
                        parentId: 210,
                        name: '步骤4：注册PostgreSQL集群实例下的数据库',
                        local: 'postgresql-0012.html'
                      },
                      {
                        id: 215,
                        parentId: 210,
                        name: '步骤5：创建限速策略',
                        local: 'postgresql-0013.html'
                      },
                      {
                        id: 216,
                        parentId: 210,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'postgresql-0014.html'
                      },
                      {
                        id: 217,
                        parentId: 210,
                        name: '步骤7：创建备份SLA',
                        local: 'postgresql-0015.html'
                      },
                      {
                        id: 218,
                        parentId: 210,
                        name: '步骤8：执行备份',
                        local: 'postgresql-0016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 202,
                parentId: 29,
                name: '复制',
                local: 'oracle_gud_000035_7.html',
                children: [
                  {
                    id: 219,
                    parentId: 202,
                    name: '复制PostgreSQL数据库副本',
                    local: 'postgresql-0021.html',
                    children: [
                      {
                        id: 220,
                        parentId: 219,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_8.html'
                      },
                      {
                        id: 221,
                        parentId: 219,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_17.html'
                      },
                      {
                        id: 222,
                        parentId: 219,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'postgresql-0024.html'
                      },
                      {
                        id: 223,
                        parentId: 219,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'postgresql-0025.html'
                      },
                      {
                        id: 224,
                        parentId: 219,
                        name: '步骤4：下载并导入证书',
                        local: 'postgresql-0026.html'
                      },
                      {
                        id: 225,
                        parentId: 219,
                        name: '步骤5：创建远端设备管理员',
                        local: 'postgresql-0027.html'
                      },
                      {
                        id: 226,
                        parentId: 219,
                        name: '创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'postgresql-0027_a1.html'
                      },
                      {
                        id: 227,
                        parentId: 219,
                        name: '步骤：添加复制集群',
                        local: 'postgresql-0028.html'
                      },
                      {
                        id: 228,
                        parentId: 219,
                        name: '步骤7：创建复制SLA',
                        local: 'postgresql-0029.html'
                      },
                      {
                        id: 229,
                        parentId: 219,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'postgresql-0030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 203,
                parentId: 29,
                name: '归档',
                local: 'postgresql-0031.html',
                children: [
                  {
                    id: 230,
                    parentId: 203,
                    name: '归档PostgreSQL备份副本',
                    local: 'postgresql-0034.html',
                    children: [
                      {
                        id: 232,
                        parentId: 230,
                        name: '步骤1：添加归档存储',
                        local: 'postgresql-0035.html',
                        children: [
                          {
                            id: 234,
                            parentId: 232,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'postgresql-0036.html'
                          },
                          {
                            id: 235,
                            parentId: 232,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'postgresql-0037.html'
                          }
                        ]
                      },
                      {
                        id: 233,
                        parentId: 230,
                        name: '步骤2：创建归档SLA',
                        local: 'postgresql-0038.html'
                      }
                    ]
                  },
                  {
                    id: 231,
                    parentId: 203,
                    name: '归档PostgreSQL复制副本',
                    local: 'postgresql-0039.html',
                    children: [
                      {
                        id: 236,
                        parentId: 231,
                        name: '步骤1：创建归档SLA',
                        local: 'postgresql-0040.html'
                      },
                      {
                        id: 237,
                        parentId: 231,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'postgresql-0041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 204,
                parentId: 29,
                name: '恢复',
                local: 'postgresql-0042.html',
                children: [
                  {
                    id: 238,
                    parentId: 204,
                    name: '恢复PostgreSQL',
                    local: 'postgresql-0045.html'
                  }
                ]
              },
              {
                id: 205,
                parentId: 29,
                name: '全局搜索',
                local: 'postgresql-0027_a2.html',
                children: [
                  {
                    id: 239,
                    parentId: 205,
                    name: '全局搜索资源',
                    local: 'postgresql-0027_a3.html'
                  },
                  {
                    id: 240,
                    parentId: 205,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'postgresql-0027_a4.html'
                  }
                ]
              },
              {
                id: 206,
                parentId: 29,
                name: 'SLA',
                local: 'postgresql-0050.html',
                children: [
                  {
                    id: 241,
                    parentId: 206,
                    name: '关于SLA',
                    local: 'postgresql-0051.html'
                  },
                  {
                    id: 242,
                    parentId: 206,
                    name: '查看SLA信息',
                    local: 'postgresql-0052.html'
                  },
                  {
                    id: 243,
                    parentId: 206,
                    name: '管理SLA',
                    local: 'postgresql-0053.html'
                  }
                ]
              },
              {
                id: 207,
                parentId: 29,
                name: '副本',
                local: 'postgresql-0054.html',
                children: [
                  {
                    id: 244,
                    parentId: 207,
                    name: '查看PostgreSQL副本信息',
                    local: 'postgresql-0055.html'
                  },
                  {
                    id: 245,
                    parentId: 207,
                    name: '管理PostgreSQL副本',
                    local: 'postgresql-0056.html'
                  }
                ]
              },
              {
                id: 208,
                parentId: 29,
                name: 'PostgreSQL集群环境',
                local: 'postgresql-0057.html',
                children: [
                  {
                    id: 246,
                    parentId: 208,
                    name: '查看PostgreSQL环境信息',
                    local: 'postgresql-0058.html'
                  },
                  {
                    id: 247,
                    parentId: 208,
                    name: '管理PostgreSQL',
                    local: 'postgresql-0059.html'
                  },
                  {
                    id: 248,
                    parentId: 208,
                    name: '管理PostgreSQL数据库集群',
                    local: 'postgresql-0060.html'
                  }
                ]
              }
            ]
          },
          {
            id: 30,
            parentId: 13,
            name: 'DB2数据保护',
            local: 'zh-cn_topic_0000001873759405.html',
            children: [
              {
                id: 249,
                parentId: 30,
                name: '备份',
                local: 'DB2-00003.html',
                children: [
                  {
                    id: 257,
                    parentId: 249,
                    name: '备份前准备',
                    local: 'DB2-00006.html'
                  },
                  {
                    id: 258,
                    parentId: 249,
                    name: '备份DB2数据库/DB2表空间集',
                    local: 'DB2-00007.html',
                    children: [
                      {
                        id: 259,
                        parentId: 258,
                        name: '步骤1：注册DB2数据库',
                        local: 'DB2-00008.html'
                      },
                      {
                        id: 260,
                        parentId: 258,
                        name: '步骤2：创建DB2表空间集',
                        local: 'DB2-00009.html'
                      },
                      {
                        id: 261,
                        parentId: 258,
                        name: '步骤3：创建限速策略',
                        local: 'DB2-00010.html'
                      },
                      {
                        id: 262,
                        parentId: 258,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'DB2-00011.html'
                      },
                      {
                        id: 263,
                        parentId: 258,
                        name: '步骤5：创建备份SLA',
                        local: 'DB2-00012.html'
                      },
                      {
                        id: 264,
                        parentId: 258,
                        name: '步骤6：执行备份',
                        local: 'DB2-00013.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 250,
                parentId: 30,
                name: '复制',
                local: 'DB2-00016.html',
                children: [
                  {
                    id: 265,
                    parentId: 250,
                    name: '复制DB2副本',
                    local: 'DB2-00018.html',
                    children: [
                      {
                        id: 266,
                        parentId: 265,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_14.html'
                      },
                      {
                        id: 267,
                        parentId: 265,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_29.html'
                      },
                      {
                        id: 268,
                        parentId: 265,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'DB2-00021.html'
                      },
                      {
                        id: 269,
                        parentId: 265,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'DB2-00022.html'
                      },
                      {
                        id: 270,
                        parentId: 265,
                        name: '步骤4：下载并导入证书',
                        local: 'DB2-00023.html'
                      },
                      {
                        id: 271,
                        parentId: 265,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'DB2-00024.html'
                      },
                      {
                        id: 272,
                        parentId: 265,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'DB2-00024_a1.html'
                      },
                      {
                        id: 273,
                        parentId: 265,
                        name: '步骤6：添加目标集群',
                        local: 'DB2-00025.html'
                      },
                      {
                        id: 274,
                        parentId: 265,
                        name: '步骤7：创建复制SLA',
                        local: 'DB2-00026.html'
                      },
                      {
                        id: 275,
                        parentId: 265,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'DB2-00027.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 251,
                parentId: 30,
                name: '归档',
                local: 'DB2-00028.html',
                children: [
                  {
                    id: 276,
                    parentId: 251,
                    name: '归档DB2备份副本',
                    local: 'DB2-00031.html',
                    children: [
                      {
                        id: 278,
                        parentId: 276,
                        name: '步骤1：添加归档存储',
                        local: 'DB2-00032.html',
                        children: [
                          {
                            id: 280,
                            parentId: 278,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'DB2-00033.html'
                          },
                          {
                            id: 281,
                            parentId: 278,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'DB2-00034.html'
                          }
                        ]
                      },
                      {
                        id: 279,
                        parentId: 276,
                        name: '步骤2：创建归档SLA',
                        local: 'DB2-00035.html'
                      }
                    ]
                  },
                  {
                    id: 277,
                    parentId: 251,
                    name: '归档DB2复制副本',
                    local: 'DB2-00036.html',
                    children: [
                      {
                        id: 282,
                        parentId: 277,
                        name: '步骤1：创建归档SLA',
                        local: 'DB2-00037.html'
                      },
                      {
                        id: 283,
                        parentId: 277,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'DB2-00038.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 252,
                parentId: 30,
                name: '恢复',
                local: 'DB2-00039.html',
                children: [
                  {
                    id: 284,
                    parentId: 252,
                    name: '恢复DB2数据库/表空间集',
                    local: 'DB2-00042.html'
                  }
                ]
              },
              {
                id: 253,
                parentId: 30,
                name: '全局搜索',
                local: 'DB2-00039_a1.html',
                children: [
                  {
                    id: 285,
                    parentId: 253,
                    name: '全局搜索资源',
                    local: 'DB2-00039_a2.html'
                  },
                  {
                    id: 286,
                    parentId: 253,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'DB2-00039_a3.html'
                  }
                ]
              },
              {
                id: 254,
                parentId: 30,
                name: 'SLA',
                local: 'DB2-00046.html',
                children: [
                  {
                    id: 287,
                    parentId: 254,
                    name: '关于SLA',
                    local: 'DB2-00047.html'
                  },
                  {
                    id: 288,
                    parentId: 254,
                    name: '查看SLA信息',
                    local: 'DB2-00048.html'
                  },
                  {
                    id: 289,
                    parentId: 254,
                    name: '管理SLA',
                    local: 'DB2-00049.html'
                  }
                ]
              },
              {
                id: 255,
                parentId: 30,
                name: '副本',
                local: 'DB2-00050.html',
                children: [
                  {
                    id: 290,
                    parentId: 255,
                    name: '查看DB2副本信息',
                    local: 'DB2-00051.html'
                  },
                  {
                    id: 291,
                    parentId: 255,
                    name: '管理DB2副本',
                    local: 'DB2-00052.html'
                  }
                ]
              },
              {
                id: 256,
                parentId: 30,
                name: 'DB2集群环境',
                local: 'DB2-00053.html',
                children: [
                  {
                    id: 292,
                    parentId: 256,
                    name: '查询DB2信息',
                    local: 'DB2-00054.html'
                  },
                  {
                    id: 293,
                    parentId: 256,
                    name: '管理DB2集群/表空间集',
                    local: 'DB2-00055.html'
                  },
                  {
                    id: 294,
                    parentId: 256,
                    name: '管理DB2数据库/表空间集',
                    local: 'DB2-00056.html'
                  }
                ]
              }
            ]
          },
          {
            id: 31,
            parentId: 13,
            name: 'Informix/GBase 8s数据保护',
            local: 'zh-cn_topic_0000001873759417.html',
            children: [
              {
                id: 295,
                parentId: 31,
                name: '备份',
                local: 'informix-0005.html',
                children: [
                  {
                    id: 303,
                    parentId: 295,
                    name: '备份Informix/GBase 8s',
                    local: 'informix-0008.html',
                    children: [
                      {
                        id: 304,
                        parentId: 303,
                        name: '步骤1：配置XBSA库路径',
                        local: 'zh-cn_topic_0000001792358014.html'
                      },
                      {
                        id: 305,
                        parentId: 303,
                        name: '步骤2：注册Informix/GBase 8s集群',
                        local: 'informix-0009.html'
                      },
                      {
                        id: 306,
                        parentId: 303,
                        name: '步骤3：注册Informix/GBase 8s实例',
                        local: 'informix-0010.html'
                      },
                      {
                        id: 307,
                        parentId: 303,
                        name: '步骤4：创建限速策略',
                        local: 'informix-0011.html'
                      },
                      {
                        id: 308,
                        parentId: 303,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'informix-0012.html'
                      },
                      {
                        id: 309,
                        parentId: 303,
                        name: '步骤6：创建备份SLA',
                        local: 'informix-0013.html'
                      },
                      {
                        id: 310,
                        parentId: 303,
                        name: '步骤7：执行备份',
                        local: 'informix-0014.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 296,
                parentId: 31,
                name: '复制',
                local: 'oracle_gud_000035_2.html',
                children: [
                  {
                    id: 311,
                    parentId: 296,
                    name: '复制Informix/GBase 8s数据库副本',
                    local: 'informix-0019.html',
                    children: [
                      {
                        id: 312,
                        parentId: 311,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_3.html'
                      },
                      {
                        id: 313,
                        parentId: 311,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_8.html'
                      },
                      {
                        id: 314,
                        parentId: 311,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'informix-0022.html'
                      },
                      {
                        id: 315,
                        parentId: 311,
                        name: '步骤3：（可选）：开启复制链路加密开关',
                        local: 'informix-0023.html'
                      },
                      {
                        id: 316,
                        parentId: 311,
                        name: '步骤4：下载并导入证书',
                        local: 'informix-0024.html'
                      },
                      {
                        id: 317,
                        parentId: 311,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'informix-0025.html'
                      },
                      {
                        id: 318,
                        parentId: 311,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002046568669.html'
                      },
                      {
                        id: 319,
                        parentId: 311,
                        name: '步骤：添加复制集群',
                        local: 'informix-0026.html'
                      },
                      {
                        id: 320,
                        parentId: 311,
                        name: '步骤7：创建复制SLA',
                        local: 'informix-0027.html'
                      },
                      {
                        id: 321,
                        parentId: 311,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'informix-0028.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 297,
                parentId: 31,
                name: '归档',
                local: 'informix-0029.html',
                children: [
                  {
                    id: 322,
                    parentId: 297,
                    name: '归档Informix/GBase 8s备份副本',
                    local: 'informix-0032.html',
                    children: [
                      {
                        id: 324,
                        parentId: 322,
                        name: '步骤1：添加归档存储',
                        local: 'informix-0033.html',
                        children: [
                          {
                            id: 326,
                            parentId: 324,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'informix-0034.html'
                          },
                          {
                            id: 327,
                            parentId: 324,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'informix-0035.html'
                          }
                        ]
                      },
                      {
                        id: 325,
                        parentId: 322,
                        name: '步骤2：创建归档SLA',
                        local: 'informix-0036.html'
                      }
                    ]
                  },
                  {
                    id: 323,
                    parentId: 297,
                    name: '归档Informix/GBase 8s复制副本',
                    local: 'informix-0037.html',
                    children: [
                      {
                        id: 328,
                        parentId: 323,
                        name: '步骤1：创建归档SLA',
                        local: 'informix-0038.html'
                      },
                      {
                        id: 329,
                        parentId: 323,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'informix-0039.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 298,
                parentId: 31,
                name: '恢复',
                local: 'informix-0040.html',
                children: [
                  {
                    id: 330,
                    parentId: 298,
                    name: '恢复Informix/GBase 8s',
                    local: 'informix-0043.html'
                  }
                ]
              },
              {
                id: 299,
                parentId: 31,
                name: '全局搜索',
                local: 'informix-0044.html',
                children: [
                  {
                    id: 331,
                    parentId: 299,
                    name: '全局搜索资源',
                    local: 'informix-0045.html'
                  },
                  {
                    id: 332,
                    parentId: 299,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002038759717.html'
                  }
                ]
              },
              {
                id: 300,
                parentId: 31,
                name: 'SLA',
                local: 'informix-0048.html',
                children: [
                  {
                    id: 333,
                    parentId: 300,
                    name: '关于SLA',
                    local: 'informix-0049.html'
                  },
                  {
                    id: 334,
                    parentId: 300,
                    name: '查看SLA信息',
                    local: 'informix-0050.html'
                  },
                  {
                    id: 335,
                    parentId: 300,
                    name: '管理SLA',
                    local: 'informix-0051.html'
                  }
                ]
              },
              {
                id: 301,
                parentId: 31,
                name: '副本',
                local: 'informix-0052.html',
                children: [
                  {
                    id: 336,
                    parentId: 301,
                    name: '查看Informix/GBase 8s副本信息',
                    local: 'informix-0053.html'
                  },
                  {
                    id: 337,
                    parentId: 301,
                    name: '管理Informix/GBase 8s副本',
                    local: 'informix-0054.html'
                  }
                ]
              },
              {
                id: 302,
                parentId: 31,
                name: 'Informix/GBase 8s集群环境',
                local: 'informix-0055.html',
                children: [
                  {
                    id: 338,
                    parentId: 302,
                    name: '查看Informix/GBase 8s环境信息',
                    local: 'informix-0056.html'
                  },
                  {
                    id: 339,
                    parentId: 302,
                    name: '管理Informix/GBase 8s',
                    local: 'informix-0057.html'
                  },
                  {
                    id: 340,
                    parentId: 302,
                    name: '管理Informix/GBase 8s数据库集群',
                    local: 'informix-0058.html'
                  }
                ]
              }
            ]
          },
          {
            id: 32,
            parentId: 13,
            name: 'openGauss数据保护',
            local: 'zh-cn_topic_0000001873679197.html',
            children: [
              {
                id: 341,
                parentId: 32,
                name: '备份',
                local: 'opengauss-0006.html',
                children: [
                  {
                    id: 349,
                    parentId: 341,
                    name: '备份前准备',
                    local: 'opengauss-0009.html'
                  },
                  {
                    id: 350,
                    parentId: 341,
                    name: '备份openGauss/磐维CMDB',
                    local: 'opengauss-0010.html',
                    children: [
                      {
                        id: 351,
                        parentId: 350,
                        name: '步骤1：开启归档模式（仅适用于磐维CMDB）',
                        local: 'postgresql-0010.html'
                      },
                      {
                        id: 352,
                        parentId: 350,
                        name: '步骤2：注册openGauss/磐维CMDB集群',
                        local: 'opengauss-0011.html'
                      },
                      {
                        id: 353,
                        parentId: 350,
                        name: '步骤3：创建限速策略',
                        local: 'opengauss-0012.html'
                      },
                      {
                        id: 354,
                        parentId: 350,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'opengauss-0013.html'
                      },
                      {
                        id: 355,
                        parentId: 350,
                        name: '步骤5：创建备份SLA',
                        local: 'opengauss-0014.html'
                      },
                      {
                        id: 356,
                        parentId: 350,
                        name: '步骤6：执行备份',
                        local: 'opengauss-0015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 342,
                parentId: 32,
                name: '复制',
                local: 'oracle_gud_000035_5.html',
                children: [
                  {
                    id: 357,
                    parentId: 342,
                    name: '复制openGauss/磐维CMDB数据库副本',
                    local: 'opengauss-0020.html',
                    children: [
                      {
                        id: 358,
                        parentId: 357,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_6.html'
                      },
                      {
                        id: 359,
                        parentId: 357,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_13.html'
                      },
                      {
                        id: 360,
                        parentId: 357,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'opengauss-0023.html'
                      },
                      {
                        id: 361,
                        parentId: 357,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'opengauss-0024.html'
                      },
                      {
                        id: 362,
                        parentId: 357,
                        name: '步骤4：下载并导入证书',
                        local: 'opengauss-0025.html'
                      },
                      {
                        id: 363,
                        parentId: 357,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'opengauss-0026.html'
                      },
                      {
                        id: 364,
                        parentId: 357,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'opengauss-0026_a1.html'
                      },
                      {
                        id: 365,
                        parentId: 357,
                        name: '步骤：添加目标集群',
                        local: 'opengauss-0027.html'
                      },
                      {
                        id: 366,
                        parentId: 357,
                        name: '步骤7：创建复制SLA',
                        local: 'opengauss-0028.html'
                      },
                      {
                        id: 367,
                        parentId: 357,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'opengauss-0029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 343,
                parentId: 32,
                name: '归档',
                local: 'opengauss-0030.html',
                children: [
                  {
                    id: 368,
                    parentId: 343,
                    name: '归档openGauss/磐维CMDB备份副本',
                    local: 'opengauss-0033.html',
                    children: [
                      {
                        id: 370,
                        parentId: 368,
                        name: '步骤1：添加归档存储',
                        local: 'opengauss-0034.html',
                        children: [
                          {
                            id: 372,
                            parentId: 370,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'opengauss-0035.html'
                          },
                          {
                            id: 373,
                            parentId: 370,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'opengauss-0036.html'
                          }
                        ]
                      },
                      {
                        id: 371,
                        parentId: 368,
                        name: '步骤2：创建归档SLA',
                        local: 'opengauss-0037.html'
                      }
                    ]
                  },
                  {
                    id: 369,
                    parentId: 343,
                    name: '归档openGauss/磐维CMDB复制副本',
                    local: 'opengauss-0038.html',
                    children: [
                      {
                        id: 374,
                        parentId: 369,
                        name: '步骤1：创建归档SLA',
                        local: 'opengauss-0039.html'
                      },
                      {
                        id: 375,
                        parentId: 369,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'opengauss-0040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 344,
                parentId: 32,
                name: '恢复',
                local: 'opengauss-0041.html',
                children: [
                  {
                    id: 376,
                    parentId: 344,
                    name: '恢复openGauss/磐维CMDB',
                    local: 'opengauss-0044.html'
                  }
                ]
              },
              {
                id: 345,
                parentId: 32,
                name: '全局搜索',
                local: 'opengauss-0026_a2.html',
                children: [
                  {
                    id: 377,
                    parentId: 345,
                    name: '全局搜索资源',
                    local: 'opengauss-0026_a3.html'
                  },
                  {
                    id: 378,
                    parentId: 345,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'opengauss-0026_a4.html'
                  }
                ]
              },
              {
                id: 346,
                parentId: 32,
                name: 'SLA',
                local: 'opengauss-0049.html',
                children: [
                  {
                    id: 379,
                    parentId: 346,
                    name: '关于SLA',
                    local: 'opengauss-0050.html'
                  },
                  {
                    id: 380,
                    parentId: 346,
                    name: '查看SLA信息',
                    local: 'opengauss-0051.html'
                  },
                  {
                    id: 381,
                    parentId: 346,
                    name: '管理SLA',
                    local: 'opengauss-0052.html'
                  }
                ]
              },
              {
                id: 347,
                parentId: 32,
                name: '副本',
                local: 'opengauss-0053.html',
                children: [
                  {
                    id: 382,
                    parentId: 347,
                    name: '查看openGauss/磐维CMDB副本信息',
                    local: 'opengauss-0054.html'
                  },
                  {
                    id: 383,
                    parentId: 347,
                    name: '管理openGauss/磐维CMDB副本',
                    local: 'opengauss-0055.html'
                  }
                ]
              },
              {
                id: 348,
                parentId: 32,
                name: 'openGauss/磐维CMDB数据库环境',
                local: 'opengauss-0056.html',
                children: [
                  {
                    id: 384,
                    parentId: 348,
                    name: '查看openGauss/磐维CMDB信息',
                    local: 'opengauss-0057.html'
                  },
                  {
                    id: 385,
                    parentId: 348,
                    name: '管理openGauss/磐维CMDB',
                    local: 'opengauss-0058.html'
                  },
                  {
                    id: 386,
                    parentId: 348,
                    name: '管理openGauss/磐维CMDB集群',
                    local: 'opengauss-0059.html'
                  }
                ]
              }
            ]
          },
          {
            id: 33,
            parentId: 13,
            name: 'GaussDB T数据保护',
            local: 'zh-cn_topic_0000001827039680.html',
            children: [
              {
                id: 387,
                parentId: 33,
                name: '备份',
                local: 'gaussdbT_00006.html',
                children: [
                  {
                    id: 395,
                    parentId: 387,
                    name: '备份前准备',
                    local: 'gaussdbT_00009.html'
                  },
                  {
                    id: 396,
                    parentId: 387,
                    name: '备份GaussDB T数据库',
                    local: 'gaussdbT_00010.html',
                    children: [
                      {
                        id: 397,
                        parentId: 396,
                        name: '步骤1：检查并配置数据库环境',
                        local: 'gaussdbT_00011.html'
                      },
                      {
                        id: 398,
                        parentId: 396,
                        name: '步骤2：设置Redo日志模式',
                        local: 'gaussdbT_00071.html'
                      },
                      {
                        id: 399,
                        parentId: 396,
                        name: '步骤3：注册GaussDB T数据库',
                        local: 'gaussdbT_00012.html'
                      },
                      {
                        id: 400,
                        parentId: 396,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'gaussdbT_00013.html'
                      },
                      {
                        id: 401,
                        parentId: 396,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'gaussdbT_00014.html'
                      },
                      {
                        id: 402,
                        parentId: 396,
                        name: '步骤6：创建备份SLA',
                        local: 'gaussdbT_00015.html'
                      },
                      {
                        id: 403,
                        parentId: 396,
                        name: '步骤7：执行备份',
                        local: 'gaussdbT_00016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 388,
                parentId: 33,
                name: '复制',
                local: 'gaussdbT_00019.html',
                children: [
                  {
                    id: 404,
                    parentId: 388,
                    name: '复制GaussDB T数据库副本',
                    local: 'gaussdbT_00021.html',
                    children: [
                      {
                        id: 405,
                        parentId: 404,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'gaussdbT_00023.html'
                      },
                      {
                        id: 406,
                        parentId: 404,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_30.html'
                      },
                      {
                        id: 407,
                        parentId: 404,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'gaussdbT_00024.html'
                      },
                      {
                        id: 408,
                        parentId: 404,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'gaussdbT_00025.html'
                      },
                      {
                        id: 409,
                        parentId: 404,
                        name: '步骤4：下载并导入证书',
                        local: 'gaussdbT_00026.html'
                      },
                      {
                        id: 410,
                        parentId: 404,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'gaussdbT_00027.html'
                      },
                      {
                        id: 411,
                        parentId: 404,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'gaussdbT_00027_a1.html'
                      },
                      {
                        id: 412,
                        parentId: 404,
                        name: '步骤6：添加复制集群',
                        local: 'gaussdbT_00028.html'
                      },
                      {
                        id: 413,
                        parentId: 404,
                        name: '步骤：创建复制SLA',
                        local: 'gaussdbT_00029.html'
                      },
                      {
                        id: 414,
                        parentId: 404,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'gaussdbT_00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 389,
                parentId: 33,
                name: '归档',
                local: 'gaussdbT_00031.html',
                children: [
                  {
                    id: 415,
                    parentId: 389,
                    name: '归档GaussDB T备份副本',
                    local: 'gaussdbT_00034.html',
                    children: [
                      {
                        id: 417,
                        parentId: 415,
                        name: '步骤1：添加归档存储',
                        local: 'gaussdbT_00035.html',
                        children: [
                          {
                            id: 419,
                            parentId: 417,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'gaussdbT_00036.html'
                          },
                          {
                            id: 420,
                            parentId: 417,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'gaussdbT_00037.html'
                          }
                        ]
                      },
                      {
                        id: 418,
                        parentId: 415,
                        name: '步骤2：创建归档SLA',
                        local: 'gaussdbT_00038.html'
                      }
                    ]
                  },
                  {
                    id: 416,
                    parentId: 389,
                    name: '归档GaussDB T复制副本',
                    local: 'gaussdbT_00039.html',
                    children: [
                      {
                        id: 421,
                        parentId: 416,
                        name: '步骤1：创建归档SLA',
                        local: 'gaussdbT_00040.html'
                      },
                      {
                        id: 422,
                        parentId: 416,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'gaussdbT_00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 390,
                parentId: 33,
                name: '恢复',
                local: 'gaussdbT_00042.html',
                children: [
                  {
                    id: 423,
                    parentId: 390,
                    name: '恢复GaussDB T数据库',
                    local: 'gaussdbT_00045.html'
                  }
                ]
              },
              {
                id: 391,
                parentId: 33,
                name: '全局搜索',
                local: 'gaussdbT_00042_a1.html',
                children: [
                  {
                    id: 424,
                    parentId: 391,
                    name: '全局搜索资源',
                    local: 'gaussdbT_00042_a2.html'
                  },
                  {
                    id: 425,
                    parentId: 391,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'gaussdbT_00042_a3.html'
                  }
                ]
              },
              {
                id: 392,
                parentId: 33,
                name: 'SLA',
                local: 'gaussdbT_00049.html',
                children: [
                  {
                    id: 426,
                    parentId: 392,
                    name: '关于SLA',
                    local: 'gaussdbT_00050.html'
                  },
                  {
                    id: 427,
                    parentId: 392,
                    name: '查看SLA信息',
                    local: 'gaussdbT_00051.html'
                  },
                  {
                    id: 428,
                    parentId: 392,
                    name: '管理SLA',
                    local: 'gaussdbT_00052.html'
                  }
                ]
              },
              {
                id: 393,
                parentId: 33,
                name: '副本',
                local: 'gaussdbT_00053.html',
                children: [
                  {
                    id: 429,
                    parentId: 393,
                    name: '查看GaussDB T副本信息',
                    local: 'gaussdbT_00054.html'
                  },
                  {
                    id: 430,
                    parentId: 393,
                    name: '管理GaussDB T副本',
                    local: 'gaussdbT_00055.html'
                  }
                ]
              },
              {
                id: 394,
                parentId: 33,
                name: 'GaussDB T数据库环境',
                local: 'gaussdbT_00056.html',
                children: [
                  {
                    id: 431,
                    parentId: 394,
                    name: '查看GaussDB T数据库环境信息',
                    local: 'gaussdbT_00057.html'
                  },
                  {
                    id: 432,
                    parentId: 394,
                    name: '管理数据库',
                    local: 'gaussdbT_00058.html'
                  }
                ]
              }
            ]
          },
          {
            id: 34,
            parentId: 13,
            name: 'TiDB数据保护',
            local: 'zh-cn_topic_0000001873759409.html',
            children: [
              {
                id: 433,
                parentId: 34,
                name: '概述',
                local: 'zh-cn_topic_0000001879213805.html',
                children: [
                  {
                    id: 443,
                    parentId: 433,
                    name: '功能概述',
                    local: 'zh-cn_topic_0000001832454472.html'
                  }
                ]
              },
              {
                id: 434,
                parentId: 34,
                name: '约束与限制',
                local: 'zh-cn_topic_0000001832294664.html'
              },
              {
                id: 435,
                parentId: 34,
                name: '备份',
                local: 'TiDB_00004.html',
                children: [
                  {
                    id: 444,
                    parentId: 435,
                    name: '备份前准备',
                    local: 'TiDB_00007.html'
                  },
                  {
                    id: 445,
                    parentId: 435,
                    name: '备份TiDB备份资源',
                    local: 'TiDB_00008.html',
                    children: [
                      {
                        id: 446,
                        parentId: 445,
                        name: '步骤1：注册TiDB集群',
                        local: 'TiDB_00009.html'
                      },
                      {
                        id: 447,
                        parentId: 445,
                        name: '步骤2：注册TiDB数据库',
                        local: 'TiDB_00010.html'
                      },
                      {
                        id: 448,
                        parentId: 445,
                        name: '步骤3：注册TiDB表集',
                        local: 'TiDB_00011.html'
                      },
                      {
                        id: 449,
                        parentId: 445,
                        name: '步骤4：创建限速策略',
                        local: 'TiDB_00012.html'
                      },
                      {
                        id: 450,
                        parentId: 445,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'TiDB_00013.html'
                      },
                      {
                        id: 451,
                        parentId: 445,
                        name: '步骤6：创建备份SLA',
                        local: 'TiDB_00014.html'
                      },
                      {
                        id: 452,
                        parentId: 445,
                        name: '步骤7：执行备份',
                        local: 'TiDB_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 436,
                parentId: 34,
                name: '复制',
                local: 'TiDB_00018.html',
                children: [
                  {
                    id: 453,
                    parentId: 436,
                    name: '复制TiDB副本',
                    local: 'TiDB_00020.html',
                    children: [
                      {
                        id: 454,
                        parentId: 453,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'TiDB_00022.html'
                      },
                      {
                        id: 455,
                        parentId: 453,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_26.html'
                      },
                      {
                        id: 456,
                        parentId: 453,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'TiDB_00023.html'
                      },
                      {
                        id: 457,
                        parentId: 453,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'TiDB_00024.html'
                      },
                      {
                        id: 458,
                        parentId: 453,
                        name: '步骤4：下载并导入证书',
                        local: 'TiDB_00025.html'
                      },
                      {
                        id: 459,
                        parentId: 453,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'TiDB_00026.html'
                      },
                      {
                        id: 460,
                        parentId: 453,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'TiDB_0002600.html'
                      },
                      {
                        id: 461,
                        parentId: 453,
                        name: '步骤6：添加目标集群',
                        local: 'TiDB_00027.html'
                      },
                      {
                        id: 462,
                        parentId: 453,
                        name: '步骤7：创建复制SLA',
                        local: 'TiDB_00028.html'
                      },
                      {
                        id: 463,
                        parentId: 453,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'TiDB_00029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 437,
                parentId: 34,
                name: '归档',
                local: 'TiDB_00030.html',
                children: [
                  {
                    id: 464,
                    parentId: 437,
                    name: '归档TiDB备份副本',
                    local: 'TiDB_00033.html',
                    children: [
                      {
                        id: 466,
                        parentId: 464,
                        name: '步骤1：添加归档存储',
                        local: 'TiDB_00034.html',
                        children: [
                          {
                            id: 468,
                            parentId: 466,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'TiDB_00035.html'
                          },
                          {
                            id: 469,
                            parentId: 466,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'TiDB_00036.html'
                          }
                        ]
                      },
                      {
                        id: 467,
                        parentId: 464,
                        name: '步骤2：创建归档SLA',
                        local: 'TiDB_00037.html'
                      }
                    ]
                  },
                  {
                    id: 465,
                    parentId: 437,
                    name: '归档TiDB复制副本',
                    local: 'TiDB_00038.html',
                    children: [
                      {
                        id: 470,
                        parentId: 465,
                        name: '步骤1：创建归档SLA',
                        local: 'TiDB_00039.html'
                      },
                      {
                        id: 471,
                        parentId: 465,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'TiDB_00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 438,
                parentId: 34,
                name: '恢复',
                local: 'TiDB_00041.html',
                children: [
                  {
                    id: 472,
                    parentId: 438,
                    name: '恢复TiDB备份资源',
                    local: 'TiDB_00044.html'
                  },
                  {
                    id: 473,
                    parentId: 438,
                    name: '恢复TiDB备份资源中的单个或多个表',
                    local: 'TiDB_00045.html'
                  }
                ]
              },
              {
                id: 439,
                parentId: 34,
                name: '全局搜索',
                local: 'TiDB_00046.html',
                children: [
                  {
                    id: 474,
                    parentId: 439,
                    name: '全局搜索资源',
                    local: 'TiDB_00047.html'
                  },
                  {
                    id: 475,
                    parentId: 439,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'TiDB_000471.html'
                  }
                ]
              },
              {
                id: 440,
                parentId: 34,
                name: 'SLA',
                local: 'TiDB_00050.html',
                children: [
                  {
                    id: 476,
                    parentId: 440,
                    name: '关于SLA',
                    local: 'TiDB_000501.html'
                  },
                  {
                    id: 477,
                    parentId: 440,
                    name: '查看SLA信息',
                    local: 'TiDB_00052.html'
                  },
                  {
                    id: 478,
                    parentId: 440,
                    name: '管理SLA',
                    local: 'TiDB_00053.html'
                  }
                ]
              },
              {
                id: 441,
                parentId: 34,
                name: '副本',
                local: 'TiDB_00054.html',
                children: [
                  {
                    id: 479,
                    parentId: 441,
                    name: '查看TiDB副本信息',
                    local: 'TiDB_00055.html'
                  },
                  {
                    id: 480,
                    parentId: 441,
                    name: '管理TiDB副本',
                    local: 'TiDB_00056.html'
                  }
                ]
              },
              {
                id: 442,
                parentId: 34,
                name: 'TiDB集群环境',
                local: 'TiDB_00057.html',
                children: [
                  {
                    id: 481,
                    parentId: 442,
                    name: '查询TiDB信息',
                    local: 'TiDB_00058.html'
                  },
                  {
                    id: 482,
                    parentId: 442,
                    name: '管理TiDB集群',
                    local: 'TiDB_00059.html'
                  },
                  {
                    id: 483,
                    parentId: 442,
                    name: '管理数据库',
                    local: 'TiDB_00060.html'
                  },
                  {
                    id: 484,
                    parentId: 442,
                    name: '管理表集',
                    local: 'TiDB_00061.html'
                  }
                ]
              }
            ]
          },
          {
            id: 35,
            parentId: 13,
            name: 'OceanBase数据保护',
            local: 'zh-cn_topic_0000001826879852.html',
            children: [
              {
                id: 485,
                parentId: 35,
                name: '备份',
                local: 'oceanbase_00005.html',
                children: [
                  {
                    id: 493,
                    parentId: 485,
                    name: '备份前准备',
                    local: 'oceanbase_00008.html'
                  },
                  {
                    id: 494,
                    parentId: 485,
                    name: '备份OceanBase',
                    local: 'oceanbase_00009.html',
                    children: [
                      {
                        id: 495,
                        parentId: 494,
                        name: '步骤1：检查并开启NFSv4.1服务',
                        local: 'zh-cn_topic_0000001839342213.html'
                      },
                      {
                        id: 496,
                        parentId: 494,
                        name: '步骤2：注册OceanBase集群',
                        local: 'oceanbase_00010.html'
                      },
                      {
                        id: 497,
                        parentId: 494,
                        name: '步骤3：注册OceanBase租户集',
                        local: 'oceanbase_00011.html'
                      },
                      {
                        id: 498,
                        parentId: 494,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'oceanbase_00012.html'
                      },
                      {
                        id: 499,
                        parentId: 494,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'oceanbase_00013.html'
                      },
                      {
                        id: 500,
                        parentId: 494,
                        name: '步骤6：创建备份SLA',
                        local: 'oceanbase_00014.html'
                      },
                      {
                        id: 501,
                        parentId: 494,
                        name: '步骤7：执行备份',
                        local: 'oceanbase_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 486,
                parentId: 35,
                name: '复制',
                local: 'oracle_gud_000035_6.html',
                children: [
                  {
                    id: 502,
                    parentId: 486,
                    name: '复制OceanBase副本',
                    local: 'oceanbase_00020.html',
                    children: [
                      {
                        id: 503,
                        parentId: 502,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_7.html'
                      },
                      {
                        id: 504,
                        parentId: 502,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_15.html'
                      },
                      {
                        id: 505,
                        parentId: 502,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'oceanbase_00023.html'
                      },
                      {
                        id: 506,
                        parentId: 502,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'oceanbase_00024.html'
                      },
                      {
                        id: 507,
                        parentId: 502,
                        name: '步骤4：下载并导入证书',
                        local: 'oceanbase_00025.html'
                      },
                      {
                        id: 508,
                        parentId: 502,
                        name: '步骤5：创建远端设备管理员',
                        local: 'oceanbase_00026.html'
                      },
                      {
                        id: 509,
                        parentId: 502,
                        name: '创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'oceanbase_00026_a1.html'
                      },
                      {
                        id: 510,
                        parentId: 502,
                        name: '步骤6：添加目标集群',
                        local: 'oceanbase_00027.html'
                      },
                      {
                        id: 511,
                        parentId: 502,
                        name: '步骤7：创建复制SLA',
                        local: 'oceanbase_00028.html'
                      },
                      {
                        id: 512,
                        parentId: 502,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'oceanbase_00029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 487,
                parentId: 35,
                name: '归档',
                local: 'oceanbase_00030.html',
                children: [
                  {
                    id: 513,
                    parentId: 487,
                    name: '归档OceanBase备份副本',
                    local: 'oceanbase_00033.html',
                    children: [
                      {
                        id: 515,
                        parentId: 513,
                        name: '步骤1：添加归档存储',
                        local: 'oceanbase_00034.html',
                        children: [
                          {
                            id: 517,
                            parentId: 515,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'oceanbase_00035.html'
                          },
                          {
                            id: 518,
                            parentId: 515,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'oceanbase_00036.html'
                          }
                        ]
                      },
                      {
                        id: 516,
                        parentId: 513,
                        name: '步骤2：创建归档SLA',
                        local: 'oceanbase_00037.html'
                      }
                    ]
                  },
                  {
                    id: 514,
                    parentId: 487,
                    name: '归档OceanBase复制副本',
                    local: 'oceanbase_00038.html',
                    children: [
                      {
                        id: 519,
                        parentId: 514,
                        name: '步骤1：创建归档SLA',
                        local: 'oceanbase_00039.html'
                      },
                      {
                        id: 520,
                        parentId: 514,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'oceanbase_00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 488,
                parentId: 35,
                name: '恢复',
                local: 'oceanbase_00041.html',
                children: [
                  {
                    id: 521,
                    parentId: 488,
                    name: '恢复OceanBase',
                    local: 'oceanbase_00044.html'
                  }
                ]
              },
              {
                id: 489,
                parentId: 35,
                name: '全局搜索',
                local: 'oceanbase_00026_a2.html',
                children: [
                  {
                    id: 522,
                    parentId: 489,
                    name: '全局搜索资源',
                    local: 'oceanbase_00026_a3.html'
                  },
                  {
                    id: 523,
                    parentId: 489,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'oceanbase_00026_a4.html'
                  }
                ]
              },
              {
                id: 490,
                parentId: 35,
                name: 'SLA',
                local: 'oceanbase_00050.html',
                children: [
                  {
                    id: 524,
                    parentId: 490,
                    name: '关于SLA',
                    local: 'oceanbase_00051.html'
                  },
                  {
                    id: 525,
                    parentId: 490,
                    name: '查看SLA信息',
                    local: 'oceanbase_00052.html'
                  },
                  {
                    id: 526,
                    parentId: 490,
                    name: '管理SLA',
                    local: 'oceanbase_00053.html'
                  }
                ]
              },
              {
                id: 491,
                parentId: 35,
                name: '副本',
                local: 'oceanbase_00054.html',
                children: [
                  {
                    id: 527,
                    parentId: 491,
                    name: '查看OceanBase副本信息',
                    local: 'oceanbase_00055.html'
                  },
                  {
                    id: 528,
                    parentId: 491,
                    name: '管理OceanBase副本',
                    local: 'oceanbase_00056.html'
                  }
                ]
              },
              {
                id: 492,
                parentId: 35,
                name: 'OceanBase集群环境',
                local: 'oceanbase_00057.html',
                children: [
                  {
                    id: 529,
                    parentId: 492,
                    name: '查看OceanBase环境信息',
                    local: 'oceanbase_00058.html'
                  },
                  {
                    id: 530,
                    parentId: 492,
                    name: '管理集群',
                    local: 'oceanbase_00059.html'
                  },
                  {
                    id: 531,
                    parentId: 492,
                    name: '管理租户集',
                    local: 'oceanbase_00060.html'
                  }
                ]
              }
            ]
          },
          {
            id: 36,
            parentId: 13,
            name: 'TDSQL数据保护',
            local: 'zh-cn_topic_0000001827039708.html',
            children: [
              {
                id: 532,
                parentId: 36,
                name: '备份',
                local: 'tdsql_gud_006.html',
                children: [
                  {
                    id: 540,
                    parentId: 532,
                    name: '约束与限制',
                    local: 'tdsql_gud_082.html'
                  },
                  {
                    id: 541,
                    parentId: 532,
                    name: '备份前准备',
                    local: 'tdsql_gud_081.html'
                  },
                  {
                    id: 542,
                    parentId: 532,
                    name: '备份TDSQL数据库',
                    local: 'tdsql_gud_009.html',
                    children: [
                      {
                        id: 543,
                        parentId: 542,
                        name:
                          '步骤1：开启TDSQL数据库权限（适用于非分布式实例）',
                        local: 'tdsql_gud_010.html'
                      },
                      {
                        id: 544,
                        parentId: 542,
                        name:
                          '步骤2：开启zkmeta自动备份功能（适用于分布式实例）',
                        local: 'tdsql_gud_080.html'
                      },
                      {
                        id: 545,
                        parentId: 542,
                        name: '步骤3：注册TDSQL数据库',
                        local: 'tdsql_gud_011.html'
                      },
                      {
                        id: 546,
                        parentId: 542,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'tdsql_gud_012.html'
                      },
                      {
                        id: 547,
                        parentId: 542,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'tdsql_gud_013.html'
                      },
                      {
                        id: 548,
                        parentId: 542,
                        name: '步骤6：创建备份SLA',
                        local: 'tdsql_gud_014.html'
                      },
                      {
                        id: 549,
                        parentId: 542,
                        name: '步骤7：执行备份',
                        local: 'tdsql_gud_015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 533,
                parentId: 36,
                name: '复制',
                local: 'tdsql_gud_018.html',
                children: [
                  {
                    id: 550,
                    parentId: 533,
                    name: '复制TDSQL数据库副本',
                    local: 'tdsql_gud_020.html',
                    children: [
                      {
                        id: 551,
                        parentId: 550,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'tdsql_gud_022.html'
                      },
                      {
                        id: 552,
                        parentId: 550,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'tdsql_gud_0026_1.html'
                      },
                      {
                        id: 553,
                        parentId: 550,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'tdsql_gud_023.html'
                      },
                      {
                        id: 554,
                        parentId: 550,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'tdsql_gud_024.html'
                      },
                      {
                        id: 555,
                        parentId: 550,
                        name: '步骤4：下载并导入证书',
                        local: 'tdsql_gud_025.html'
                      },
                      {
                        id: 556,
                        parentId: 550,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'tdsql_gud_026.html'
                      },
                      {
                        id: 557,
                        parentId: 550,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002044460125.html'
                      },
                      {
                        id: 558,
                        parentId: 550,
                        name: '步骤6：添加复制集群',
                        local: 'tdsql_gud_027.html'
                      },
                      {
                        id: 559,
                        parentId: 550,
                        name: '步骤7：创建复制SLA',
                        local: 'tdsql_gud_028.html'
                      },
                      {
                        id: 560,
                        parentId: 550,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'tdsql_gud_029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 534,
                parentId: 36,
                name: '归档',
                local: 'tdsql_gud_030.html',
                children: [
                  {
                    id: 561,
                    parentId: 534,
                    name: '归档TDSQL备份副本',
                    local: 'tdsql_gud_033.html',
                    children: [
                      {
                        id: 563,
                        parentId: 561,
                        name: '步骤1：添加归档存储',
                        local: 'tdsql_gud_034.html',
                        children: [
                          {
                            id: 565,
                            parentId: 563,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'tdsql_gud_035.html'
                          },
                          {
                            id: 566,
                            parentId: 563,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'tdsql_gud_036.html'
                          }
                        ]
                      },
                      {
                        id: 564,
                        parentId: 561,
                        name: '步骤2：创建归档SLA',
                        local: 'tdsql_gud_037.html'
                      }
                    ]
                  },
                  {
                    id: 562,
                    parentId: 534,
                    name: '归档TDSQL复制副本',
                    local: 'tdsql_gud_038.html',
                    children: [
                      {
                        id: 567,
                        parentId: 562,
                        name: '步骤1：创建归档SLA',
                        local: 'tdsql_gud_039.html'
                      },
                      {
                        id: 568,
                        parentId: 562,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'tdsql_gud_040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 535,
                parentId: 36,
                name: '恢复',
                local: 'tdsql_gud_041.html',
                children: [
                  {
                    id: 569,
                    parentId: 535,
                    name: '恢复TDSQL数据库',
                    local: 'tdsql_gud_044.html'
                  }
                ]
              },
              {
                id: 536,
                parentId: 36,
                name: '全局搜索',
                local: 'tdsql_gud_045.html',
                children: [
                  {
                    id: 570,
                    parentId: 536,
                    name: '全局搜索资源',
                    local: 'tdsql_gud_047.html'
                  },
                  {
                    id: 571,
                    parentId: 536,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002002686458.html'
                  }
                ]
              },
              {
                id: 537,
                parentId: 36,
                name: 'SLA',
                local: 'tdsql_gud_059.html',
                children: [
                  {
                    id: 572,
                    parentId: 537,
                    name: '关于SLA',
                    local: 'tdsql_gud_060.html'
                  },
                  {
                    id: 573,
                    parentId: 537,
                    name: '查看SLA信息',
                    local: 'tdsql_gud_061.html'
                  },
                  {
                    id: 574,
                    parentId: 537,
                    name: '管理SLA',
                    local: 'tdsql_gud_062.html'
                  }
                ]
              },
              {
                id: 538,
                parentId: 36,
                name: '副本',
                local: 'tdsql_gud_063.html',
                children: [
                  {
                    id: 575,
                    parentId: 538,
                    name: '查看TDSQL副本信息',
                    local: 'tdsql_gud_064.html'
                  },
                  {
                    id: 576,
                    parentId: 538,
                    name: '管理TDSQL副本',
                    local: 'tdsql_gud_065.html'
                  }
                ]
              },
              {
                id: 539,
                parentId: 36,
                name: 'TDSQL数据库环境',
                local: 'tdsql_gud_066.html',
                children: [
                  {
                    id: 577,
                    parentId: 539,
                    name: '查看TDSQL数据库环境信息',
                    local: 'tdsql_gud_067.html'
                  },
                  {
                    id: 578,
                    parentId: 539,
                    name: '管理数据库集群',
                    local: 'tdsql_gud_069.html'
                  },
                  {
                    id: 579,
                    parentId: 539,
                    name: '管理数据库实例',
                    local: 'tdsql_gud_068.html'
                  }
                ]
              }
            ]
          },
          {
            id: 37,
            parentId: 13,
            name: 'Dameng数据保护',
            local: 'zh-cn_topic_0000001873759369.html',
            children: [
              {
                id: 580,
                parentId: 37,
                name: '备份',
                local: 'dameng-00005.html',
                children: [
                  {
                    id: 588,
                    parentId: 580,
                    name: '备份前准备',
                    local: 'dameng-00008.html'
                  },
                  {
                    id: 589,
                    parentId: 580,
                    name: '备份Dameng',
                    local: 'dameng-00009.html',
                    children: [
                      {
                        id: 590,
                        parentId: 589,
                        name: '步骤1：开启DmAPService服务',
                        local: 'dameng-00010.html'
                      },
                      {
                        id: 591,
                        parentId: 589,
                        name: '步骤2：开启数据库本地归档',
                        local: 'dameng-00011.html'
                      },
                      {
                        id: 592,
                        parentId: 589,
                        name: '步骤3：注册Dameng数据库',
                        local: 'dameng-00012.html'
                      },
                      {
                        id: 593,
                        parentId: 589,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'dameng-00012_1.html'
                      },
                      {
                        id: 594,
                        parentId: 589,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'dameng-00014.html'
                      },
                      {
                        id: 595,
                        parentId: 589,
                        name: '步骤6：创建备份SLA',
                        local: 'dameng-00015.html'
                      },
                      {
                        id: 596,
                        parentId: 589,
                        name: '步骤7：执行备份',
                        local: 'dameng-00016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 581,
                parentId: 37,
                name: '复制',
                local: 'oracle_gud_000035_10.html',
                children: [
                  {
                    id: 597,
                    parentId: 581,
                    name: '复制Dameng数据库副本',
                    local: 'dameng-00021.html',
                    children: [
                      {
                        id: 598,
                        parentId: 597,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_11.html'
                      },
                      {
                        id: 599,
                        parentId: 597,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_22.html'
                      },
                      {
                        id: 600,
                        parentId: 597,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'dameng-00024.html'
                      },
                      {
                        id: 601,
                        parentId: 597,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'dameng-00025.html'
                      },
                      {
                        id: 602,
                        parentId: 597,
                        name: '步骤4：下载并导入证书',
                        local: 'dameng-00026.html'
                      },
                      {
                        id: 603,
                        parentId: 597,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'dameng-00027.html'
                      },
                      {
                        id: 604,
                        parentId: 597,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'dameng-00027_a1.html'
                      },
                      {
                        id: 605,
                        parentId: 597,
                        name: '步骤6：添加复制集群',
                        local: 'dameng-00028.html'
                      },
                      {
                        id: 606,
                        parentId: 597,
                        name: '步骤7：创建复制SLA',
                        local: 'dameng-00029.html'
                      },
                      {
                        id: 607,
                        parentId: 597,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'dameng-00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 582,
                parentId: 37,
                name: '归档',
                local: 'dameng-00031.html',
                children: [
                  {
                    id: 608,
                    parentId: 582,
                    name: '归档Dameng备份副本',
                    local: 'dameng-00034.html',
                    children: [
                      {
                        id: 610,
                        parentId: 608,
                        name: '步骤1：添加归档存储',
                        local: 'dameng-00035.html',
                        children: [
                          {
                            id: 612,
                            parentId: 610,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'dameng-00036.html'
                          },
                          {
                            id: 613,
                            parentId: 610,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'dameng-00037.html'
                          }
                        ]
                      },
                      {
                        id: 611,
                        parentId: 608,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'dameng-00038.html'
                      }
                    ]
                  },
                  {
                    id: 609,
                    parentId: 582,
                    name: '归档Dameng复制副本',
                    local: 'dameng-00039.html',
                    children: [
                      {
                        id: 614,
                        parentId: 609,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'dameng-00040.html'
                      },
                      {
                        id: 615,
                        parentId: 609,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'dameng-00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 583,
                parentId: 37,
                name: '恢复',
                local: 'dameng-00042.html',
                children: [
                  {
                    id: 616,
                    parentId: 583,
                    name: '恢复Dameng',
                    local: 'dameng-00045.html'
                  }
                ]
              },
              {
                id: 584,
                parentId: 37,
                name: '全局搜索',
                local: 'dameng-00027_a2.html',
                children: [
                  {
                    id: 617,
                    parentId: 584,
                    name: '全局搜索资源',
                    local: 'dameng-00027_a3.html'
                  },
                  {
                    id: 618,
                    parentId: 584,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'dameng-00027_a4.html'
                  }
                ]
              },
              {
                id: 585,
                parentId: 37,
                name: 'SLA',
                local: 'dameng-00050.html',
                children: [
                  {
                    id: 619,
                    parentId: 585,
                    name: '关于SLA',
                    local: 'dameng-00051.html'
                  },
                  {
                    id: 620,
                    parentId: 585,
                    name: '查看SLA信息',
                    local: 'dameng-00052.html'
                  },
                  {
                    id: 621,
                    parentId: 585,
                    name: '管理SLA',
                    local: 'dameng-00053.html'
                  }
                ]
              },
              {
                id: 586,
                parentId: 37,
                name: '副本',
                local: 'dameng-00054.html',
                children: [
                  {
                    id: 622,
                    parentId: 586,
                    name: '查看Dameng副本信息',
                    local: 'dameng-00055.html'
                  },
                  {
                    id: 623,
                    parentId: 586,
                    name: '管理Dameng副本',
                    local: 'dameng-00056.html'
                  }
                ]
              },
              {
                id: 587,
                parentId: 37,
                name: 'Dameng环境',
                local: 'dameng-00057.html',
                children: [
                  {
                    id: 624,
                    parentId: 587,
                    name: '查看Dameng环境信息',
                    local: 'dameng-00058.html'
                  },
                  {
                    id: 625,
                    parentId: 587,
                    name: '管理Dameng',
                    local: 'dameng-00059.html'
                  }
                ]
              }
            ]
          },
          {
            id: 38,
            parentId: 13,
            name: 'Kingbase数据保护',
            local: 'zh-cn_topic_0000001827039700.html',
            children: [
              {
                id: 626,
                parentId: 38,
                name: '备份',
                local: 'kingbase-00005.html',
                children: [
                  {
                    id: 634,
                    parentId: 626,
                    name: '备份前准备',
                    local: 'kingbase-00007_a1.html'
                  },
                  {
                    id: 635,
                    parentId: 626,
                    name: '备份Kingbase实例',
                    local: 'kingbase-00008.html',
                    children: [
                      {
                        id: 636,
                        parentId: 635,
                        name: '步骤1：sys_rman初始化配置',
                        local: 'zh-cn_topic_0000002015631765.html'
                      },
                      {
                        id: 637,
                        parentId: 635,
                        name: '步骤2：注册Kingbase单实例下的数据库',
                        local: 'kingbase-00009.html'
                      },
                      {
                        id: 638,
                        parentId: 635,
                        name: '步骤3：注册Kingbase集群实例下的数据库',
                        local: 'kingbase-00010.html'
                      },
                      {
                        id: 639,
                        parentId: 635,
                        name: '步骤4：创建限速策略',
                        local: 'kingbase-00011.html'
                      },
                      {
                        id: 640,
                        parentId: 635,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'kingbase-00012.html'
                      },
                      {
                        id: 641,
                        parentId: 635,
                        name: '步骤：创建备份SLA',
                        local: 'kingbase-00013.html'
                      },
                      {
                        id: 642,
                        parentId: 635,
                        name: '步骤：执行备份',
                        local: 'kingbase-00014.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 627,
                parentId: 38,
                name: '复制',
                local: 'oracle_gud_000035_11.html',
                children: [
                  {
                    id: 643,
                    parentId: 627,
                    name: '复制Kingbase副本',
                    local: 'kingbase-00019.html',
                    children: [
                      {
                        id: 644,
                        parentId: 643,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_12.html'
                      },
                      {
                        id: 645,
                        parentId: 643,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_23.html'
                      },
                      {
                        id: 646,
                        parentId: 643,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'kingbase-00022.html'
                      },
                      {
                        id: 647,
                        parentId: 643,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'kingbase-00023.html'
                      },
                      {
                        id: 648,
                        parentId: 643,
                        name: '步骤4：下载并导入证书',
                        local: 'kingbase-00024.html'
                      },
                      {
                        id: 649,
                        parentId: 643,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'kingbase-00025.html'
                      },
                      {
                        id: 650,
                        parentId: 643,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'kingbase-00025_a1.html'
                      },
                      {
                        id: 651,
                        parentId: 643,
                        name: '步骤：添加目标集群',
                        local: 'kingbase-00026.html'
                      },
                      {
                        id: 652,
                        parentId: 643,
                        name: '步骤7：创建复制SLA',
                        local: 'kingbase-00027.html'
                      },
                      {
                        id: 653,
                        parentId: 643,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'kingbase-00028.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 628,
                parentId: 38,
                name: '归档',
                local: 'kingbase-00029.html',
                children: [
                  {
                    id: 654,
                    parentId: 628,
                    name: '归档Kingbase备份副本',
                    local: 'kingbase-00032.html',
                    children: [
                      {
                        id: 656,
                        parentId: 654,
                        name: '步骤1：添加归档存储',
                        local: 'kingbase-00033.html',
                        children: [
                          {
                            id: 658,
                            parentId: 656,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'kingbase-00034.html'
                          },
                          {
                            id: 659,
                            parentId: 656,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'kingbase-00035.html'
                          }
                        ]
                      },
                      {
                        id: 657,
                        parentId: 654,
                        name: '步骤2：创建归档SLA',
                        local: 'kingbase-00036.html'
                      }
                    ]
                  },
                  {
                    id: 655,
                    parentId: 628,
                    name: '归档Kingbase复制副本',
                    local: 'kingbase-00037.html',
                    children: [
                      {
                        id: 660,
                        parentId: 655,
                        name: '步骤1：创建归档SLA',
                        local: 'kingbase-00038.html'
                      },
                      {
                        id: 661,
                        parentId: 655,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'kingbase-00039.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 629,
                parentId: 38,
                name: '恢复',
                local: 'kingbase-00040.html',
                children: [
                  {
                    id: 662,
                    parentId: 629,
                    name: '恢复Kingbase实例',
                    local: 'kingbase-00043.html'
                  }
                ]
              },
              {
                id: 630,
                parentId: 38,
                name: '全局搜索',
                local: 'kingbase-00025_a2.html',
                children: [
                  {
                    id: 663,
                    parentId: 630,
                    name: '全局搜索资源',
                    local: 'kingbase-00025_a3.html'
                  },
                  {
                    id: 664,
                    parentId: 630,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'kingbase-00025_a4.html'
                  }
                ]
              },
              {
                id: 631,
                parentId: 38,
                name: 'SLA',
                local: 'kingbase-00048.html',
                children: [
                  {
                    id: 665,
                    parentId: 631,
                    name: '关于SLA',
                    local: 'kingbase-00049.html'
                  },
                  {
                    id: 666,
                    parentId: 631,
                    name: '查看SLA信息',
                    local: 'kingbase-00050.html'
                  },
                  {
                    id: 667,
                    parentId: 631,
                    name: '管理SLA',
                    local: 'kingbase-00051.html'
                  }
                ]
              },
              {
                id: 632,
                parentId: 38,
                name: '副本',
                local: 'kingbase-00052.html',
                children: [
                  {
                    id: 668,
                    parentId: 632,
                    name: '查看Kingbase副本信息',
                    local: 'kingbase-00053.html'
                  },
                  {
                    id: 669,
                    parentId: 632,
                    name: '管理Kingbase副本',
                    local: 'kingbase-00054.html'
                  }
                ]
              },
              {
                id: 633,
                parentId: 38,
                name: 'Kingbase集群环境',
                local: 'kingbase-00055.html',
                children: [
                  {
                    id: 670,
                    parentId: 633,
                    name: '查看Kingbase环境信息',
                    local: 'kingbase-00056.html'
                  },
                  {
                    id: 671,
                    parentId: 633,
                    name: '管理Kingbase',
                    local: 'kingbase-00057.html'
                  },
                  {
                    id: 672,
                    parentId: 633,
                    name: '管理Kingbase数据库集群',
                    local: 'kingbase-00058.html'
                  }
                ]
              }
            ]
          },
          {
            id: 39,
            parentId: 13,
            name: 'GoldenDB数据保护',
            local: 'zh-cn_topic_0000001873759373.html',
            children: [
              {
                id: 673,
                parentId: 39,
                name: '备份',
                local: 'goldendb-00005.html',
                children: [
                  {
                    id: 681,
                    parentId: 673,
                    name: '备份前准备',
                    local: 'goldendb-00008.html'
                  },
                  {
                    id: 682,
                    parentId: 673,
                    name: '备份GoldenDB数据库',
                    local: 'goldendb-00009.html',
                    children: [
                      {
                        id: 683,
                        parentId: 682,
                        name: '步骤1：注册GoldenDB集群',
                        local: 'goldendb-00010.html'
                      },
                      {
                        id: 684,
                        parentId: 682,
                        name: '步骤2：创建GoldenDB实例',
                        local: 'goldendb-00011.html'
                      },
                      {
                        id: 685,
                        parentId: 682,
                        name: '步骤3：创建限速策略',
                        local: 'goldendb-00012.html'
                      },
                      {
                        id: 686,
                        parentId: 682,
                        name: '步骤4：（可选）开启备份链路加密开关 ',
                        local: 'goldendb-00013.html'
                      },
                      {
                        id: 687,
                        parentId: 682,
                        name: '步骤5：创建备份SLA',
                        local: 'goldendb-00014.html'
                      },
                      {
                        id: 688,
                        parentId: 682,
                        name: '步骤6：执行备份',
                        local: 'goldendb-00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 674,
                parentId: 39,
                name: '复制',
                local: 'oracle_gud_000035_9.html',
                children: [
                  {
                    id: 689,
                    parentId: 674,
                    name: '复制GoldenDB副本',
                    local: 'goldendb-00020.html',
                    children: [
                      {
                        id: 690,
                        parentId: 689,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_10.html'
                      },
                      {
                        id: 691,
                        parentId: 689,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_19.html'
                      },
                      {
                        id: 692,
                        parentId: 689,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'goldendb-00023.html'
                      },
                      {
                        id: 693,
                        parentId: 689,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'goldendb-00024.html'
                      },
                      {
                        id: 694,
                        parentId: 689,
                        name: '步骤4：下载并导入证书',
                        local: 'goldendb-00025.html'
                      },
                      {
                        id: 695,
                        parentId: 689,
                        name: '步骤5：创建远端设备管理员',
                        local: 'goldendb-00026.html'
                      },
                      {
                        id: 696,
                        parentId: 689,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002010566636.html'
                      },
                      {
                        id: 697,
                        parentId: 689,
                        name: '步骤：添加复制集群',
                        local: 'goldendb-00027.html'
                      },
                      {
                        id: 698,
                        parentId: 689,
                        name: '步骤7：创建复制SLA',
                        local: 'goldendb-00028.html'
                      },
                      {
                        id: 699,
                        parentId: 689,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'goldendb-00029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 675,
                parentId: 39,
                name: '归档',
                local: 'goldendb-00030.html',
                children: [
                  {
                    id: 700,
                    parentId: 675,
                    name: '归档GoldenDB备份副本',
                    local: 'goldendb-00033.html',
                    children: [
                      {
                        id: 702,
                        parentId: 700,
                        name: '步骤1：添加归档存储',
                        local: 'goldendb-00034.html',
                        children: [
                          {
                            id: 704,
                            parentId: 702,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'goldendb-00035.html'
                          },
                          {
                            id: 705,
                            parentId: 702,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'goldendb-00036.html'
                          }
                        ]
                      },
                      {
                        id: 703,
                        parentId: 700,
                        name: '步骤2：创建归档SLA',
                        local: 'goldendb-00037.html'
                      }
                    ]
                  },
                  {
                    id: 701,
                    parentId: 675,
                    name: '归档GoldenDB复制副本',
                    local: 'goldendb-00038.html',
                    children: [
                      {
                        id: 706,
                        parentId: 701,
                        name: '步骤1：创建归档SLA',
                        local: 'goldendb-00039.html'
                      },
                      {
                        id: 707,
                        parentId: 701,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'goldendb-00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 676,
                parentId: 39,
                name: '恢复',
                local: 'goldendb-00041.html',
                children: [
                  {
                    id: 708,
                    parentId: 676,
                    name: '恢复GoldenDB',
                    local: 'goldendb-00044.html'
                  }
                ]
              },
              {
                id: 677,
                parentId: 39,
                name: '全局搜索',
                local: 'goldendb-00045.html',
                children: [
                  {
                    id: 709,
                    parentId: 677,
                    name: '全局搜索资源',
                    local: 'goldendb-00047.html'
                  },
                  {
                    id: 710,
                    parentId: 677,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002002838402.html'
                  }
                ]
              },
              {
                id: 678,
                parentId: 39,
                name: 'SLA',
                local: 'goldendb-00050.html',
                children: [
                  {
                    id: 711,
                    parentId: 678,
                    name: '关于SLA',
                    local: 'goldendb-00051.html'
                  },
                  {
                    id: 712,
                    parentId: 678,
                    name: '查看SLA信息',
                    local: 'goldendb-00052.html'
                  },
                  {
                    id: 713,
                    parentId: 678,
                    name: '管理SLA',
                    local: 'goldendb-00053.html'
                  }
                ]
              },
              {
                id: 679,
                parentId: 39,
                name: '副本',
                local: 'goldendb-00054.html',
                children: [
                  {
                    id: 714,
                    parentId: 679,
                    name: '查看GoldenDB副本信息',
                    local: 'goldendb-00055.html'
                  },
                  {
                    id: 715,
                    parentId: 679,
                    name: '管理GoldenDB副本',
                    local: 'goldendb-00056.html'
                  }
                ]
              },
              {
                id: 680,
                parentId: 39,
                name: 'GoldenDB集群环境',
                local: 'goldendb-00057.html',
                children: [
                  {
                    id: 716,
                    parentId: 680,
                    name: '查看GoldenDB环境信息',
                    local: 'goldendb-00058.html'
                  },
                  {
                    id: 717,
                    parentId: 680,
                    name: '管理实例',
                    local: 'goldendb-00059.html'
                  },
                  {
                    id: 718,
                    parentId: 680,
                    name: '管理集群',
                    local: 'goldendb-00060.html'
                  }
                ]
              }
            ]
          },
          {
            id: 40,
            parentId: 13,
            name: 'GaussDB数据保护',
            local: 'zh-cn_topic_0000001827039692.html',
            children: [
              {
                id: 719,
                parentId: 40,
                name: '备份',
                local: 'TPOPS_GaussDB_00006.html',
                children: [
                  {
                    id: 727,
                    parentId: 719,
                    name: '备份前准备',
                    local: 'TPOPS_GaussDB_00009.html'
                  },
                  {
                    id: 728,
                    parentId: 719,
                    name: '备份GaussDB实例',
                    local: 'TPOPS_GaussDB_00010.html',
                    children: [
                      {
                        id: 729,
                        parentId: 728,
                        name: '步骤1：获取管理面地址和端口',
                        local: 'TPOPS_GaussDB_00014.html'
                      },
                      {
                        id: 730,
                        parentId: 728,
                        name: '步骤2：在TPOPS节点上开启XBSA备份的白名单',
                        local: 'TPOPS_GaussDB_00013.html'
                      },
                      {
                        id: 731,
                        parentId: 728,
                        name: '步骤3：在TPOPS管理界面配置实例的备份默认根路径',
                        local: 'TPOPS_GaussDB_00011.html'
                      },
                      {
                        id: 732,
                        parentId: 728,
                        name: '步骤4：在TPOPS管理界面打开实例监控',
                        local: 'TPOPS_GaussDB_00012.html'
                      },
                      {
                        id: 733,
                        parentId: 728,
                        name: '步骤5：注册GaussDB项目',
                        local: 'TPOPS_GaussDB_00015.html'
                      },
                      {
                        id: 734,
                        parentId: 728,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'TPOPS_GaussDB_00016.html'
                      },
                      {
                        id: 735,
                        parentId: 728,
                        name: '步骤7：创建限速策略',
                        local: 'TPOPS_GaussDB_00017.html'
                      },
                      {
                        id: 736,
                        parentId: 728,
                        name: '步骤8：创建备份SLA',
                        local: 'TPOPS_GaussDB_00018.html'
                      },
                      {
                        id: 737,
                        parentId: 728,
                        name: '步骤9：执行备份',
                        local: 'TPOPS_GaussDB_00019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 720,
                parentId: 40,
                name: '复制',
                local: 'TPOPS_GaussDB_00022.html',
                children: [
                  {
                    id: 738,
                    parentId: 720,
                    name: '复制GaussDB副本',
                    local: 'TPOPS_GaussDB_00024.html',
                    children: [
                      {
                        id: 739,
                        parentId: 738,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'TPOPS_GaussDB_00026.html'
                      },
                      {
                        id: 740,
                        parentId: 738,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_7.html'
                      },
                      {
                        id: 741,
                        parentId: 738,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'TPOPS_GaussDB_00027.html'
                      },
                      {
                        id: 742,
                        parentId: 738,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'TPOPS_GaussDB_00028.html'
                      },
                      {
                        id: 743,
                        parentId: 738,
                        name: '步骤4：下载并导入证书',
                        local: 'TPOPS_GaussDB_00029.html'
                      },
                      {
                        id: 744,
                        parentId: 738,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'TPOPS_GaussDB_00030.html'
                      },
                      {
                        id: 745,
                        parentId: 738,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'TPOPS_GaussDB_000300.html'
                      },
                      {
                        id: 746,
                        parentId: 738,
                        name: '步骤6：添加目标集群',
                        local: 'TPOPS_GaussDB_00031.html'
                      },
                      {
                        id: 747,
                        parentId: 738,
                        name: '步骤7：创建复制SLA',
                        local: 'TPOPS_GaussDB_00032.html'
                      },
                      {
                        id: 748,
                        parentId: 738,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'TPOPS_GaussDB_00033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 721,
                parentId: 40,
                name: '归档',
                local: 'TPOPS_GaussDB_00034.html',
                children: [
                  {
                    id: 749,
                    parentId: 721,
                    name: '归档GaussDB备份副本',
                    local: 'TPOPS_GaussDB_00037.html',
                    children: [
                      {
                        id: 751,
                        parentId: 749,
                        name: '步骤1：添加归档存储',
                        local: 'TPOPS_GaussDB_00038.html',
                        children: [
                          {
                            id: 753,
                            parentId: 751,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'TPOPS_GaussDB_00039.html'
                          },
                          {
                            id: 754,
                            parentId: 751,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'TPOPS_GaussDB_00040.html'
                          }
                        ]
                      },
                      {
                        id: 752,
                        parentId: 749,
                        name: '步骤2：创建归档SLA',
                        local: 'TPOPS_GaussDB_00041.html'
                      }
                    ]
                  },
                  {
                    id: 750,
                    parentId: 721,
                    name: '归档GaussDB复制副本',
                    local: 'TPOPS_GaussDB_00042.html',
                    children: [
                      {
                        id: 755,
                        parentId: 750,
                        name: '步骤1：创建归档SLA',
                        local: 'TPOPS_GaussDB_00043.html'
                      },
                      {
                        id: 756,
                        parentId: 750,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'TPOPS_GaussDB_00044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 722,
                parentId: 40,
                name: '恢复',
                local: 'TPOPS_GaussDB_00045.html',
                children: [
                  {
                    id: 757,
                    parentId: 722,
                    name: '恢复GaussDB实例',
                    local: 'TPOPS_GaussDB_00048.html'
                  }
                ]
              },
              {
                id: 723,
                parentId: 40,
                name: '全局搜索',
                local: 'TPOPS_GaussDB_000451.html',
                children: [
                  {
                    id: 758,
                    parentId: 723,
                    name: '全局搜索资源',
                    local: 'TPOPS_GaussDB_00049.html'
                  },
                  {
                    id: 759,
                    parentId: 723,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'TPOPS_GaussDB_000491.html'
                  }
                ]
              },
              {
                id: 724,
                parentId: 40,
                name: 'SLA',
                local: 'TPOPS_GaussDB_00052.html',
                children: [
                  {
                    id: 760,
                    parentId: 724,
                    name: '关于SLA',
                    local: 'TPOPS_GaussDB_000540.html'
                  },
                  {
                    id: 761,
                    parentId: 724,
                    name: '查看SLA信息',
                    local: 'TPOPS_GaussDB_00054.html'
                  },
                  {
                    id: 762,
                    parentId: 724,
                    name: '管理SLA',
                    local: 'TPOPS_GaussDB_00055.html'
                  }
                ]
              },
              {
                id: 725,
                parentId: 40,
                name: '副本',
                local: 'TPOPS_GaussDB_00056.html',
                children: [
                  {
                    id: 763,
                    parentId: 725,
                    name: '查看GaussDB副本信息',
                    local: 'TPOPS_GaussDB_00057.html'
                  },
                  {
                    id: 764,
                    parentId: 725,
                    name: '管理GaussDB副本',
                    local: 'TPOPS_GaussDB_00058.html'
                  }
                ]
              },
              {
                id: 726,
                parentId: 40,
                name: 'GaussDB',
                local: 'TPOPS_GaussDB_00059.html',
                children: [
                  {
                    id: 765,
                    parentId: 726,
                    name: '查看GaussDB信息',
                    local: 'TPOPS_GaussDB_00060.html'
                  },
                  {
                    id: 766,
                    parentId: 726,
                    name: '管理GaussDB项目',
                    local: 'TPOPS_GaussDB_00061.html'
                  },
                  {
                    id: 767,
                    parentId: 726,
                    name: '管理实例',
                    local: 'TPOPS_GaussDB_00062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 41,
            parentId: 13,
            name: 'GBase 8a数据保护',
            local: 'zh-cn_topic_0000001873759389.html',
            children: [
              {
                id: 768,
                parentId: 41,
                name: '备份',
                local: 'GBase_8a_00006.html',
                children: [
                  {
                    id: 776,
                    parentId: 768,
                    name: '备份前准备',
                    local: 'GBase_8a_00009.html'
                  },
                  {
                    id: 777,
                    parentId: 768,
                    name: '备份GBase 8a数据库',
                    local: 'GBase_8a_00010.html',
                    children: [
                      {
                        id: 778,
                        parentId: 777,
                        name: '步骤1：注册GBase 8a数据库',
                        local: 'GBase_8a_00011.html'
                      },
                      {
                        id: 779,
                        parentId: 777,
                        name: '步骤2：创建限速策略',
                        local: 'GBase_8a_00012.html'
                      },
                      {
                        id: 780,
                        parentId: 777,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'GBase_8a_00013.html'
                      },
                      {
                        id: 781,
                        parentId: 777,
                        name: '步骤4：创建备份SLA',
                        local: 'GBase_8a_00014.html'
                      },
                      {
                        id: 782,
                        parentId: 777,
                        name: '步骤5：执行备份',
                        local: 'GBase_8a_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 769,
                parentId: 41,
                name: '复制',
                local: 'oracle_gud_000035_13.html',
                children: [
                  {
                    id: 783,
                    parentId: 769,
                    name: '复制GBase 8a数据库副本',
                    local: 'GBase_8a_00020.html',
                    children: [
                      {
                        id: 784,
                        parentId: 783,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'GBase_8a_00022.html'
                      },
                      {
                        id: 785,
                        parentId: 783,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_27.html'
                      },
                      {
                        id: 786,
                        parentId: 783,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'GBase_8a_00023.html'
                      },
                      {
                        id: 787,
                        parentId: 783,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'GBase_8a_00024.html'
                      },
                      {
                        id: 788,
                        parentId: 783,
                        name: '步骤4：下载并导入证书',
                        local: 'GBase_8a_00025.html'
                      },
                      {
                        id: 789,
                        parentId: 783,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'GBase_8a_00026.html'
                      },
                      {
                        id: 790,
                        parentId: 783,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'GBase_8a_0002600.html'
                      },
                      {
                        id: 791,
                        parentId: 783,
                        name: '步骤6：添加目标集群',
                        local: 'GBase_8a_00027.html'
                      },
                      {
                        id: 792,
                        parentId: 783,
                        name: '步骤7：创建复制SLA',
                        local: 'GBase_8a_00028.html'
                      },
                      {
                        id: 793,
                        parentId: 783,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'GBase_8a_00029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 770,
                parentId: 41,
                name: '归档',
                local: 'GBase_8a_00030.html',
                children: [
                  {
                    id: 794,
                    parentId: 770,
                    name: '归档GBase 8a备份副本',
                    local: 'GBase_8a_00033.html',
                    children: [
                      {
                        id: 796,
                        parentId: 794,
                        name: '步骤1：添加归档存储',
                        local: 'GBase_8a_00034.html',
                        children: [
                          {
                            id: 798,
                            parentId: 796,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'GBase_8a_00035.html'
                          },
                          {
                            id: 799,
                            parentId: 796,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'GBase_8a_00036.html'
                          }
                        ]
                      },
                      {
                        id: 797,
                        parentId: 794,
                        name: '步骤2：创建归档SLA',
                        local: 'GBase_8a_00037.html'
                      }
                    ]
                  },
                  {
                    id: 795,
                    parentId: 770,
                    name: '归档GBase 8a复制副本',
                    local: 'GBase_8a_00038.html',
                    children: [
                      {
                        id: 800,
                        parentId: 795,
                        name: '步骤1：创建归档SLA',
                        local: 'GBase_8a_00039.html'
                      },
                      {
                        id: 801,
                        parentId: 795,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'GBase_8a_00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 771,
                parentId: 41,
                name: '恢复',
                local: 'GBase_8a_00041.html',
                children: [
                  {
                    id: 802,
                    parentId: 771,
                    name: '恢复GBase 8a数据库',
                    local: 'GBase_8a_00044.html'
                  }
                ]
              },
              {
                id: 772,
                parentId: 41,
                name: '全局搜索',
                local: 'GBase_8a_000411.html',
                children: [
                  {
                    id: 803,
                    parentId: 772,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'GBase_8a_000451.html'
                  }
                ]
              },
              {
                id: 773,
                parentId: 41,
                name: 'SLA',
                local: 'GBase_8a_00048.html',
                children: [
                  {
                    id: 804,
                    parentId: 773,
                    name: '查看SLA信息',
                    local: 'GBase_8a_00050.html'
                  },
                  {
                    id: 805,
                    parentId: 773,
                    name: '管理SLA',
                    local: 'GBase_8a_00051.html'
                  }
                ]
              },
              {
                id: 774,
                parentId: 41,
                name: '副本',
                local: 'GBase_8a_00052.html',
                children: [
                  {
                    id: 806,
                    parentId: 774,
                    name: '查看GBase 8a副本信息',
                    local: 'GBase_8a_00053.html'
                  },
                  {
                    id: 807,
                    parentId: 774,
                    name: '管理GBase 8a副本',
                    local: 'GBase_8a_00054.html'
                  }
                ]
              },
              {
                id: 775,
                parentId: 41,
                name: 'GBase 8a数据库环境',
                local: 'GBase_8a_00055.html',
                children: [
                  {
                    id: 808,
                    parentId: 775,
                    name: '查看GBase 8a数据库环境信息',
                    local: 'GBase_8a_00056.html'
                  },
                  {
                    id: 809,
                    parentId: 775,
                    name: '管理数据库',
                    local: 'GBase_8a_00057.html'
                  }
                ]
              }
            ]
          },
          {
            id: 42,
            parentId: 13,
            name: 'SAP HANA数据保护',
            local: 'zh-cn_topic_0000001873679193.html',
            children: [
              {
                id: 810,
                parentId: 42,
                name: '备份',
                local: 'SAP_HANA_00008.html',
                children: [
                  {
                    id: 820,
                    parentId: 810,
                    name: '备份前准备',
                    local: 'SAP_HANA_00011.html'
                  },
                  {
                    id: 821,
                    parentId: 810,
                    name: '备份SAP HANA数据库（通用数据库入口）',
                    local: 'SAP_HANA_00012.html',
                    children: [
                      {
                        id: 823,
                        parentId: 821,
                        name: '步骤1：注册SAP HANA数据库（File备份方式）',
                        local: 'SAP_HANA_00013.html'
                      },
                      {
                        id: 824,
                        parentId: 821,
                        name: '步骤2：创建限速策略',
                        local: 'SAP_HANA_00014.html'
                      },
                      {
                        id: 825,
                        parentId: 821,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'SAP_HANA_00015.html'
                      },
                      {
                        id: 826,
                        parentId: 821,
                        name: '步骤4：配置日志备份',
                        local: 'SAP_HANA_00016.html'
                      },
                      {
                        id: 827,
                        parentId: 821,
                        name: '步骤5：创建备份SLA',
                        local: 'zh-cn_topic_0000002058629252.html'
                      },
                      {
                        id: 828,
                        parentId: 821,
                        name: '步骤6：执行备份',
                        local: 'SAP_HANA_00024_b2.html'
                      }
                    ]
                  },
                  {
                    id: 822,
                    parentId: 810,
                    name:
                      '备份SAP HANA数据库（SAP HANA应用入口）（仅1.6.0及后续版本）',
                    local: 'SAP_HANA_00019.html',
                    children: [
                      {
                        id: 829,
                        parentId: 822,
                        name: '步骤1：注册SAP HANA数据库（Backint备份方式）',
                        local: 'SAP_HANA_00020.html'
                      },
                      {
                        id: 830,
                        parentId: 822,
                        name: '步骤2：创建限速策略',
                        local: 'SAP_HANA_00022.html'
                      },
                      {
                        id: 831,
                        parentId: 822,
                        name: '步骤4：创建备份SLA',
                        local: 'zh-cn_topic_0000002058628320.html'
                      },
                      {
                        id: 832,
                        parentId: 822,
                        name: '步骤5：执行备份',
                        local: 'SAP_HANA_00025.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 811,
                parentId: 42,
                name: '复制',
                local: 'SAP_HANA_00028.html',
                children: [
                  {
                    id: 833,
                    parentId: 811,
                    name: '复制SAP HANA数据库副本',
                    local: 'SAP_HANA_00031.html',
                    children: [
                      {
                        id: 834,
                        parentId: 833,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'SAP_HANA_00033.html'
                      },
                      {
                        id: 835,
                        parentId: 833,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'SAP_HANA_00034.html'
                      },
                      {
                        id: 836,
                        parentId: 833,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'SAP_HANA_00035.html'
                      },
                      {
                        id: 837,
                        parentId: 833,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'SAP_HANA_00036.html'
                      },
                      {
                        id: 838,
                        parentId: 833,
                        name: '步骤4：下载并导入证书',
                        local: 'SAP_HANA_00037.html'
                      },
                      {
                        id: 839,
                        parentId: 833,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'SAP_HANA_00038.html'
                      },
                      {
                        id: 840,
                        parentId: 833,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'SAP_HANA_00037_a1.html'
                      },
                      {
                        id: 841,
                        parentId: 833,
                        name: '步骤6：添加复制集群',
                        local: 'SAP_HANA_00039.html'
                      },
                      {
                        id: 842,
                        parentId: 833,
                        name: '步骤7：创建复制SLA',
                        local: 'SAP_HANA_00040.html'
                      },
                      {
                        id: 843,
                        parentId: 833,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'SAP_HANA_00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 812,
                parentId: 42,
                name: '归档',
                local: 'SAP_HANA_00042.html',
                children: [
                  {
                    id: 844,
                    parentId: 812,
                    name: '归档SAP HANA备份副本',
                    local: 'SAP_HANA_00045.html',
                    children: [
                      {
                        id: 846,
                        parentId: 844,
                        name: '步骤1：添加归档存储',
                        local: 'SAP_HANA_00046.html',
                        children: [
                          {
                            id: 848,
                            parentId: 846,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'SAP_HANA_00047.html'
                          },
                          {
                            id: 849,
                            parentId: 846,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'SAP_HANA_00048.html'
                          }
                        ]
                      },
                      {
                        id: 847,
                        parentId: 844,
                        name: '步骤2：创建归档SLA',
                        local: 'SAP_HANA_00049.html'
                      }
                    ]
                  },
                  {
                    id: 845,
                    parentId: 812,
                    name: '归档SAP HANA复制副本',
                    local: 'SAP_HANA_00050.html',
                    children: [
                      {
                        id: 850,
                        parentId: 845,
                        name: '步骤1：创建归档SLA',
                        local: 'SAP_HANA_00051.html'
                      },
                      {
                        id: 851,
                        parentId: 845,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'SAP_HANA_00052.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 813,
                parentId: 42,
                name: '恢复',
                local: 'SAP_HANA_00054.html',
                children: [
                  {
                    id: 852,
                    parentId: 813,
                    name: '恢复SAP HANA数据库（通用数据库入口）',
                    local: 'SAP_HANA_00057.html'
                  }
                ]
              },
              {
                id: 814,
                parentId: 42,
                name: '全局搜索',
                local: 'SAP_HANA_00053_a1.html',
                children: [
                  {
                    id: 853,
                    parentId: 814,
                    name: '全局搜索资源',
                    local: 'SAP_HANA_00053_a2.html'
                  },
                  {
                    id: 854,
                    parentId: 814,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'SAP_HANA_00053_a3.html'
                  }
                ]
              },
              {
                id: 815,
                parentId: 42,
                name: 'SLA',
                local: 'SAP_HANA_00063.html',
                children: [
                  {
                    id: 855,
                    parentId: 815,
                    name: '关于SLA',
                    local: 'SAP_HANA_00063_qe.html'
                  },
                  {
                    id: 856,
                    parentId: 815,
                    name: '查看SLA信息',
                    local: 'SAP_HANA_00065.html'
                  },
                  {
                    id: 857,
                    parentId: 815,
                    name: '管理SLA',
                    local: 'SAP_HANA_00066.html'
                  }
                ]
              },
              {
                id: 816,
                parentId: 42,
                name: '副本（通用数据库入口）',
                local: 'SAP_HANA_00067.html',
                children: [
                  {
                    id: 858,
                    parentId: 816,
                    name: '查看SAP HANA副本信息',
                    local: 'SAP_HANA_00068.html'
                  },
                  {
                    id: 859,
                    parentId: 816,
                    name: '管理SAP HANA副本',
                    local: 'SAP_HANA_00069.html'
                  }
                ]
              },
              {
                id: 817,
                parentId: 42,
                name: '副本（SAP HANA应用入口）（仅1.6.0及后续版本）',
                local: 'SAP_HANA_00067_as11.html',
                children: [
                  {
                    id: 860,
                    parentId: 817,
                    name: '查看SAP HANA副本信息',
                    local: 'SAP_HANA_00067_as12.html'
                  },
                  {
                    id: 861,
                    parentId: 817,
                    name: '管理SAP HANA副本',
                    local: 'SAP_HANA_00067_as13.html'
                  }
                ]
              },
              {
                id: 818,
                parentId: 42,
                name: 'SAP HANA数据库环境（通用数据库入口）',
                local: 'SAP_HANA_00070.html',
                children: [
                  {
                    id: 862,
                    parentId: 818,
                    name: '查看SAP HANA数据库环境信息',
                    local: 'SAP_HANA_00071.html'
                  },
                  {
                    id: 863,
                    parentId: 818,
                    name: '管理数据库',
                    local: 'SAP_HANA_00072.html'
                  }
                ]
              },
              {
                id: 819,
                parentId: 42,
                name: 'SAP HANA数据库环境（SAP HANA应用入口）',
                local: 'SAP_HANA_00073.html',
                children: [
                  {
                    id: 864,
                    parentId: 819,
                    name: '查看SAP HANA数据库环境信息',
                    local: 'SAP_HANA_00074.html'
                  },
                  {
                    id: 865,
                    parentId: 819,
                    name: '管理实例',
                    local: 'SAP_HANA_00075.html'
                  },
                  {
                    id: 866,
                    parentId: 819,
                    name: '管理数据库',
                    local: 'SAP_HANA_00076.html'
                  }
                ]
              }
            ]
          }
        ]
      },
      {
        id: 14,
        parentId: 3,
        name: '大数据',
        local: 'zh-cn_topic_0000001948269721.html',
        children: [
          {
            id: 867,
            parentId: 14,
            name: 'ClickHouse数据保护',
            local: 'zh-cn_topic_0000001873759365.html',
            children: [
              {
                id: 875,
                parentId: 867,
                name: '备份',
                local: 'clickhouse-0003.html',
                children: [
                  {
                    id: 884,
                    parentId: 875,
                    name: '备份前准备',
                    local: 'clickhouse-0006.html'
                  },
                  {
                    id: 885,
                    parentId: 875,
                    name: '备份ClickHouse数据库/ClickHouse表集',
                    local: 'clickhouse-0007.html',
                    children: [
                      {
                        id: 886,
                        parentId: 885,
                        name: '步骤1：注册ClickHouse集群',
                        local: 'clickhouse-0008.html'
                      },
                      {
                        id: 887,
                        parentId: 885,
                        name: '步骤2：创建ClickHouse表集',
                        local: 'clickhouse-0009.html'
                      },
                      {
                        id: 888,
                        parentId: 885,
                        name: '步骤3：创建限速策略',
                        local: 'clickhouse-0010.html'
                      },
                      {
                        id: 889,
                        parentId: 885,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'clickhouse-0011.html'
                      },
                      {
                        id: 890,
                        parentId: 885,
                        name: '步骤5：创建备份SLA',
                        local: 'clickhouse-0012.html'
                      },
                      {
                        id: 891,
                        parentId: 885,
                        name: '步骤6：执行备份',
                        local: 'clickhouse-0013.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 876,
                parentId: 867,
                name: '复制',
                local: 'oracle_gud_000035_1.html',
                children: [
                  {
                    id: 892,
                    parentId: 876,
                    name: '复制ClickHouse副本',
                    local: 'clickhouse-0018.html',
                    children: [
                      {
                        id: 893,
                        parentId: 892,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_2.html'
                      },
                      {
                        id: 894,
                        parentId: 892,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_2.html'
                      },
                      {
                        id: 895,
                        parentId: 892,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'clickhouse-0021.html'
                      },
                      {
                        id: 896,
                        parentId: 892,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'clickhouse-0022.html'
                      },
                      {
                        id: 897,
                        parentId: 892,
                        name: '步骤4：下载并导入证书',
                        local: 'clickhouse-0023.html'
                      },
                      {
                        id: 898,
                        parentId: 892,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'clickhouse-0024.html'
                      },
                      {
                        id: 899,
                        parentId: 892,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002010575970.html'
                      },
                      {
                        id: 900,
                        parentId: 892,
                        name: '步骤：添加复制集群',
                        local: 'clickhouse-0025.html'
                      },
                      {
                        id: 901,
                        parentId: 892,
                        name: '步骤7：创建复制SLA',
                        local: 'oracle_gud_000041.html'
                      },
                      {
                        id: 902,
                        parentId: 892,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'clickhouse-0027.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 877,
                parentId: 867,
                name: '归档',
                local: 'clickhouse-0028.html',
                children: [
                  {
                    id: 903,
                    parentId: 877,
                    name: '归档ClickHouse备份副本',
                    local: 'clickhouse-0031.html',
                    children: [
                      {
                        id: 905,
                        parentId: 903,
                        name: '步骤1：添加归档存储',
                        local: 'clickhouse-0032.html',
                        children: [
                          {
                            id: 907,
                            parentId: 905,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'clickhouse-0033.html'
                          },
                          {
                            id: 908,
                            parentId: 905,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'clickhouse-0034.html'
                          }
                        ]
                      },
                      {
                        id: 906,
                        parentId: 903,
                        name: '步骤2：创建归档SLA',
                        local: 'clickhouse-0035.html'
                      }
                    ]
                  },
                  {
                    id: 904,
                    parentId: 877,
                    name: '归档ClickHouse复制副本',
                    local: 'clickhouse-0036.html',
                    children: [
                      {
                        id: 909,
                        parentId: 904,
                        name: '步骤1：创建归档SLA',
                        local: 'clickhouse-0037.html'
                      },
                      {
                        id: 910,
                        parentId: 904,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'clickhouse-0038.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 878,
                parentId: 867,
                name: '恢复',
                local: 'clickhouse-0039.html',
                children: [
                  {
                    id: 911,
                    parentId: 878,
                    name: '恢复ClickHouse数据库/表集',
                    local: 'clickhouse-0042.html'
                  }
                ]
              },
              {
                id: 879,
                parentId: 867,
                name: '全局搜索',
                local: 'zh-cn_topic_0000002038764373.html',
                children: [
                  {
                    id: 912,
                    parentId: 879,
                    name: '全局搜索资源',
                    local: 'clickhouse-0043.html'
                  },
                  {
                    id: 913,
                    parentId: 879,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002002844410.html'
                  }
                ]
              },
              {
                id: 880,
                parentId: 867,
                name: '数据重删压缩',
                local: 'clickhouse-0044.html',
                children: [
                  {
                    id: 914,
                    parentId: 880,
                    name: '关于数据重删压缩',
                    local: 'clickhouse-0045.html'
                  }
                ]
              },
              {
                id: 881,
                parentId: 867,
                name: 'SLA',
                local: 'clickhouse-0046.html',
                children: [
                  {
                    id: 915,
                    parentId: 881,
                    name: '关于SLA',
                    local: 'clickhouse-0047.html'
                  },
                  {
                    id: 916,
                    parentId: 881,
                    name: '查看SLA信息',
                    local: 'clickhouse-0048.html'
                  },
                  {
                    id: 917,
                    parentId: 881,
                    name: '管理SLA',
                    local: 'clickhouse-0049.html'
                  }
                ]
              },
              {
                id: 882,
                parentId: 867,
                name: '副本',
                local: 'clickhouse-0050.html',
                children: [
                  {
                    id: 918,
                    parentId: 882,
                    name: '查看ClickHouse副本信息',
                    local: 'clickhouse-0051.html'
                  },
                  {
                    id: 919,
                    parentId: 882,
                    name: '管理ClickHouse副本',
                    local: 'clickhouse-0052.html'
                  }
                ]
              },
              {
                id: 883,
                parentId: 867,
                name: 'ClickHouse集群环境',
                local: 'clickhouse-0053.html',
                children: [
                  {
                    id: 920,
                    parentId: 883,
                    name: '查询ClickHouse信息',
                    local: 'clickhouse-0054.html'
                  },
                  {
                    id: 921,
                    parentId: 883,
                    name: '管理ClickHouse集群/表集',
                    local: 'clickhouse-0055.html'
                  },
                  {
                    id: 922,
                    parentId: 883,
                    name: '管理ClickHouse数据库/表集保护',
                    local: 'clickhouse-0056.html'
                  }
                ]
              }
            ]
          },
          {
            id: 868,
            parentId: 14,
            name: 'GaussDB(DWS)数据保护',
            local: 'product_documentation_000029.html',
            children: [
              {
                id: 923,
                parentId: 868,
                name: '备份',
                local: 'DWS_00006.html',
                children: [
                  {
                    id: 932,
                    parentId: 923,
                    name: '备份前准备',
                    local: 'DWS_00009.html'
                  },
                  {
                    id: 933,
                    parentId: 923,
                    name: '备份GaussDB(DWS)',
                    local: 'DWS_00010.html',
                    children: [
                      {
                        id: 934,
                        parentId: 933,
                        name: '步骤1：注册GaussDB(DWS)集群',
                        local: 'DWS_00014.html'
                      },
                      {
                        id: 935,
                        parentId: 933,
                        name: '步骤2：创建GaussDB(DWS)Schema集/表集',
                        local: 'DWS_00015.html'
                      },
                      {
                        id: 936,
                        parentId: 933,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'DWS_00016.html'
                      },
                      {
                        id: 937,
                        parentId: 933,
                        name: '步骤4：创建限速策略',
                        local: 'DWS_00017.html'
                      },
                      {
                        id: 938,
                        parentId: 933,
                        name: '步骤5：创建备份SLA',
                        local: 'DWS_00018.html'
                      },
                      {
                        id: 939,
                        parentId: 933,
                        name: '步骤6执行备份',
                        local: 'DWS_00019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 924,
                parentId: 868,
                name: '复制',
                local: 'DWS_00022.html',
                children: [
                  {
                    id: 940,
                    parentId: 924,
                    name: '复制GaussDB(DWS)备份副本',
                    local: 'DWS_00024.html',
                    children: [
                      {
                        id: 941,
                        parentId: 940,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'DWS_00026.html'
                      },
                      {
                        id: 942,
                        parentId: 940,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_14.html'
                      },
                      {
                        id: 943,
                        parentId: 940,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'DWS_00027.html'
                      },
                      {
                        id: 944,
                        parentId: 940,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'DWS_00028.html'
                      },
                      {
                        id: 945,
                        parentId: 940,
                        name: '步骤4：下载并导入证书',
                        local: 'DWS_00029.html'
                      },
                      {
                        id: 946,
                        parentId: 940,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'DWS_00030.html'
                      },
                      {
                        id: 947,
                        parentId: 940,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'DWS_00031_a1.html'
                      },
                      {
                        id: 948,
                        parentId: 940,
                        name: '步骤6：添加复制集群',
                        local: 'DWS_00031.html'
                      },
                      {
                        id: 949,
                        parentId: 940,
                        name: '步骤7：创建复制SLA',
                        local: 'DWS_00031_b1.html'
                      },
                      {
                        id: 950,
                        parentId: 940,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'DWS_00033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 925,
                parentId: 868,
                name: '归档',
                local: 'DWS_00034.html',
                children: [
                  {
                    id: 951,
                    parentId: 925,
                    name: '归档GaussDB(DWS)备份副本',
                    local: 'DWS_00037.html',
                    children: [
                      {
                        id: 953,
                        parentId: 951,
                        name: '步骤1：添加归档存储',
                        local: 'DWS_00038.html',
                        children: [
                          {
                            id: 955,
                            parentId: 953,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'DWS_00039.html'
                          },
                          {
                            id: 956,
                            parentId: 953,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'DWS_00040.html'
                          }
                        ]
                      },
                      {
                        id: 954,
                        parentId: 951,
                        name: '步骤2：创建归档SLA',
                        local: 'DWS_00041.html'
                      }
                    ]
                  },
                  {
                    id: 952,
                    parentId: 925,
                    name: '归档GaussDB(DWS)复制副本',
                    local: 'DWS_00042.html',
                    children: [
                      {
                        id: 957,
                        parentId: 952,
                        name: '步骤1：创建归档SLA',
                        local: 'DWS_00043.html'
                      },
                      {
                        id: 958,
                        parentId: 952,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'DWS_00044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 926,
                parentId: 868,
                name: '恢复',
                local: 'DWS_00045.html',
                children: [
                  {
                    id: 959,
                    parentId: 926,
                    name: '恢复GaussDB(DWS)',
                    local: 'DWS_00048.html'
                  }
                ]
              },
              {
                id: 927,
                parentId: 868,
                name: '全局搜索',
                local: 'DWS_00045_a1.html',
                children: [
                  {
                    id: 960,
                    parentId: 927,
                    name: '全局搜索资源',
                    local: 'DWS_00045_a2.html'
                  },
                  {
                    id: 961,
                    parentId: 927,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'DWS_00045_a3.html'
                  }
                ]
              },
              {
                id: 928,
                parentId: 868,
                name: '数据重删压缩',
                local: 'DWS_00050.html',
                children: [
                  {
                    id: 962,
                    parentId: 928,
                    name: '关于数据重删压缩',
                    local: 'DWS_00051.html'
                  }
                ]
              },
              {
                id: 929,
                parentId: 868,
                name: 'SLA',
                local: 'DWS_00052.html',
                children: [
                  {
                    id: 963,
                    parentId: 929,
                    name: '关于SLA',
                    local: 'DWS_00053.html'
                  },
                  {
                    id: 964,
                    parentId: 929,
                    name: '查看SLA信息',
                    local: 'DWS_00054.html'
                  },
                  {
                    id: 965,
                    parentId: 929,
                    name: '管理SLA',
                    local: 'DWS_00055.html'
                  }
                ]
              },
              {
                id: 930,
                parentId: 868,
                name: '副本',
                local: 'DWS_00056.html',
                children: [
                  {
                    id: 966,
                    parentId: 930,
                    name: '查看GaussDB(DWS)副本信息',
                    local: 'DWS_00057.html'
                  },
                  {
                    id: 967,
                    parentId: 930,
                    name: '管理GaussDB(DWS)副本',
                    local: 'DWS_00058.html'
                  }
                ]
              },
              {
                id: 931,
                parentId: 868,
                name: 'GaussDB(DWS)集群环境',
                local: 'DWS_00059.html',
                children: [
                  {
                    id: 968,
                    parentId: 931,
                    name: '查询GaussDB(DWS)信息',
                    local: 'DWS_00060.html'
                  },
                  {
                    id: 969,
                    parentId: 931,
                    name: '管理GaussDB(DWS)集群',
                    local: 'DWS_00061.html'
                  },
                  {
                    id: 970,
                    parentId: 931,
                    name: '管理GaussDB(DWS)',
                    local: 'DWS_00062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 869,
            parentId: 14,
            name: 'HBase数据保护',
            local: 'product_documentation_000033.html',
            children: [
              {
                id: 971,
                parentId: 869,
                name: '备份',
                local: 'hbase_00007.html',
                children: [
                  {
                    id: 979,
                    parentId: 971,
                    name: '备份前准备',
                    local: 'hbase_00010.html'
                  },
                  {
                    id: 980,
                    parentId: 971,
                    name: '备份HBase备份集',
                    local: 'hbase_00011.html',
                    children: [
                      {
                        id: 981,
                        parentId: 980,
                        name: '步骤1：注册HBase集群',
                        local: 'hbase_00012.html'
                      },
                      {
                        id: 982,
                        parentId: 980,
                        name: '步骤2：创建HBase备份集',
                        local: 'hbase_00013.html'
                      },
                      {
                        id: 983,
                        parentId: 980,
                        name: '步骤3：创建限速策略',
                        local: 'hbase_00014.html'
                      },
                      {
                        id: 984,
                        parentId: 980,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'hbase_00015.html'
                      },
                      {
                        id: 985,
                        parentId: 980,
                        name: '步骤5：创建备份SLA',
                        local: 'hbase_00016.html'
                      },
                      {
                        id: 986,
                        parentId: 980,
                        name: '步骤6：执行备份',
                        local: 'hbase_000017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 972,
                parentId: 869,
                name: '复制',
                local: 'hbase_00020.html',
                children: [
                  {
                    id: 987,
                    parentId: 972,
                    name: '复制HBase副本',
                    local: 'hbase_00023.html',
                    children: [
                      {
                        id: 988,
                        parentId: 987,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'hbase_00025.html'
                      },
                      {
                        id: 989,
                        parentId: 987,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_28.html'
                      },
                      {
                        id: 990,
                        parentId: 987,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'hbase_00026.html'
                      },
                      {
                        id: 991,
                        parentId: 987,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'hbase_00027.html'
                      },
                      {
                        id: 992,
                        parentId: 987,
                        name: '步骤4：下载并导入证书',
                        local: 'hbase_00028.html'
                      },
                      {
                        id: 993,
                        parentId: 987,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'hbase_000029.html'
                      },
                      {
                        id: 994,
                        parentId: 987,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002046619645.html'
                      },
                      {
                        id: 995,
                        parentId: 987,
                        name: '步骤6：添加复制集群',
                        local: 'hbase_00030.html'
                      },
                      {
                        id: 996,
                        parentId: 987,
                        name: '步骤7：创建复制SLA',
                        local: 'hbase_00031.html'
                      },
                      {
                        id: 997,
                        parentId: 987,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'hbase_00032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 973,
                parentId: 869,
                name: '归档',
                local: 'hbase_00033.html',
                children: [
                  {
                    id: 998,
                    parentId: 973,
                    name: '归档HBase备份副本',
                    local: 'hbase_00036.html',
                    children: [
                      {
                        id: 1000,
                        parentId: 998,
                        name: '步骤1：添加归档存储',
                        local: 'hbase_00037.html',
                        children: [
                          {
                            id: 1002,
                            parentId: 1000,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'hbase_00038.html'
                          },
                          {
                            id: 1003,
                            parentId: 1000,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'hbase_00039.html'
                          }
                        ]
                      },
                      {
                        id: 1001,
                        parentId: 998,
                        name: '步骤2：创建归档SLA',
                        local: 'hbase_000040.html'
                      }
                    ]
                  },
                  {
                    id: 999,
                    parentId: 973,
                    name: '归档HBase复制副本',
                    local: 'hbase_00041.html',
                    children: [
                      {
                        id: 1004,
                        parentId: 999,
                        name: '步骤1：创建归档SLA',
                        local: 'hbase_00042.html'
                      },
                      {
                        id: 1005,
                        parentId: 999,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'hbase_00043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 974,
                parentId: 869,
                name: '恢复',
                local: 'hbase_00044.html',
                children: [
                  {
                    id: 1006,
                    parentId: 974,
                    name: '恢复HBase备份集',
                    local: 'hbase_00047.html'
                  },
                  {
                    id: 1007,
                    parentId: 974,
                    name: '恢复HBase备份集中的单个或多个表',
                    local: 'hbase_00048.html'
                  }
                ]
              },
              {
                id: 975,
                parentId: 869,
                name: '全局搜索',
                local: 'hbase_00049.html',
                children: [
                  {
                    id: 1008,
                    parentId: 975,
                    name: '全局搜索资源',
                    local: 'hbase_00050.html'
                  },
                  {
                    id: 1009,
                    parentId: 975,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002038765521.html'
                  }
                ]
              },
              {
                id: 976,
                parentId: 869,
                name: 'SLA',
                local: 'hbase_00053.html',
                children: [
                  {
                    id: 1010,
                    parentId: 976,
                    name: '查看SLA信息',
                    local: 'hbase_00055.html'
                  },
                  {
                    id: 1011,
                    parentId: 976,
                    name: '管理SLA',
                    local: 'hbase_00056.html'
                  }
                ]
              },
              {
                id: 977,
                parentId: 869,
                name: '副本',
                local: 'hbase_00057.html',
                children: [
                  {
                    id: 1012,
                    parentId: 977,
                    name: '查看HBase副本信息',
                    local: 'hbase_00058.html'
                  },
                  {
                    id: 1013,
                    parentId: 977,
                    name: '管理HBase副本',
                    local: 'hbase_00059.html'
                  }
                ]
              },
              {
                id: 978,
                parentId: 869,
                name: 'HBase集群环境',
                local: 'hbase_00060.html',
                children: [
                  {
                    id: 1014,
                    parentId: 978,
                    name: '查询HBase信息',
                    local: 'hbase_00061.html'
                  },
                  {
                    id: 1015,
                    parentId: 978,
                    name: '管理HBase集群',
                    local: 'hbase_00062.html'
                  },
                  {
                    id: 1016,
                    parentId: 978,
                    name: '管理备份集保护',
                    local: 'hbase_00063.html'
                  }
                ]
              }
            ]
          },
          {
            id: 870,
            parentId: 14,
            name: 'Hive数据保护',
            local: 'zh-cn_topic_0000001827039684.html',
            children: [
              {
                id: 1017,
                parentId: 870,
                name: '概述',
                local: 'hive_00002.html',
                children: [
                  {
                    id: 1026,
                    parentId: 1017,
                    name: '功能概览',
                    local: 'hive_00004.html'
                  },
                  {
                    id: 1027,
                    parentId: 1017,
                    name: '约束与限制',
                    local: 'hive_00005.html'
                  }
                ]
              },
              {
                id: 1018,
                parentId: 870,
                name: '备份',
                local: 'hive_00007.html',
                children: [
                  {
                    id: 1028,
                    parentId: 1018,
                    name: '备份前准备',
                    local: 'hive_00010.html'
                  },
                  {
                    id: 1029,
                    parentId: 1018,
                    name: '备份Hive备份集',
                    local: 'hive_00011.html',
                    children: [
                      {
                        id: 1030,
                        parentId: 1029,
                        name: '步骤1：开启数据库表所在目录的快照功能',
                        local: 'hive_00012.html'
                      },
                      {
                        id: 1031,
                        parentId: 1029,
                        name: '步骤2：（可选）生成并获取证书',
                        local: 'hive_00014.html'
                      },
                      {
                        id: 1032,
                        parentId: 1029,
                        name: '步骤3：注册Hive集群',
                        local: 'hive_00015.html'
                      },
                      {
                        id: 1033,
                        parentId: 1029,
                        name: '步骤4：创建Hive备份集',
                        local: 'hive_00016.html'
                      },
                      {
                        id: 1034,
                        parentId: 1029,
                        name: '步骤5：创建限速策略',
                        local: 'hive_00017.html'
                      },
                      {
                        id: 1035,
                        parentId: 1029,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'hive_00018.html'
                      },
                      {
                        id: 1036,
                        parentId: 1029,
                        name: '步骤7：创建备份SLA',
                        local: 'hive_00019.html'
                      },
                      {
                        id: 1037,
                        parentId: 1029,
                        name: '步骤8：执行备份',
                        local: 'hive_00020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1019,
                parentId: 870,
                name: '复制',
                local: 'hive_00023.html',
                children: [
                  {
                    id: 1038,
                    parentId: 1019,
                    name: '复制Hive副本',
                    local: 'hive_00026.html',
                    children: [
                      {
                        id: 1039,
                        parentId: 1038,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'hive_00028.html'
                      },
                      {
                        id: 1040,
                        parentId: 1038,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_6.html'
                      },
                      {
                        id: 1041,
                        parentId: 1038,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'hive_00029.html'
                      },
                      {
                        id: 1042,
                        parentId: 1038,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'hive_00030.html'
                      },
                      {
                        id: 1043,
                        parentId: 1038,
                        name: '步骤4：下载并导入证书',
                        local: 'hive_00031.html'
                      },
                      {
                        id: 1044,
                        parentId: 1038,
                        name: '步骤5：创建远端设备管理员',
                        local: 'hive_00032.html'
                      },
                      {
                        id: 1045,
                        parentId: 1038,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002010579886.html'
                      },
                      {
                        id: 1046,
                        parentId: 1038,
                        name: '步骤6：添加复制集群',
                        local: 'hive_00033.html'
                      },
                      {
                        id: 1047,
                        parentId: 1038,
                        name: '步骤7：创建复制SLA',
                        local: 'hive_00034.html'
                      },
                      {
                        id: 1048,
                        parentId: 1038,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'hive_00035.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1020,
                parentId: 870,
                name: '归档',
                local: 'hive_00036.html',
                children: [
                  {
                    id: 1049,
                    parentId: 1020,
                    name: '归档Hive备份副本',
                    local: 'hive_00039.html',
                    children: [
                      {
                        id: 1051,
                        parentId: 1049,
                        name: '步骤1：添加归档存储',
                        local: 'hive_00040.html',
                        children: [
                          {
                            id: 1053,
                            parentId: 1051,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'hive_00041.html'
                          },
                          {
                            id: 1054,
                            parentId: 1051,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'hive_00042.html'
                          }
                        ]
                      },
                      {
                        id: 1052,
                        parentId: 1049,
                        name: '步骤2：创建归档SLA',
                        local: 'hive_00043.html'
                      }
                    ]
                  },
                  {
                    id: 1050,
                    parentId: 1020,
                    name: '归档Hive复制副本',
                    local: 'hive_00044.html',
                    children: [
                      {
                        id: 1055,
                        parentId: 1050,
                        name: '步骤1：创建归档SLA',
                        local: 'hive_00045.html'
                      },
                      {
                        id: 1056,
                        parentId: 1050,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'hive_00046.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1021,
                parentId: 870,
                name: '恢复',
                local: 'hive_00047.html',
                children: [
                  {
                    id: 1057,
                    parentId: 1021,
                    name: '恢复Hive备份集',
                    local: 'hive_00049.html'
                  },
                  {
                    id: 1058,
                    parentId: 1021,
                    name: '恢复Hive备份集中的单个或多个表',
                    local: 'hive_00050.html'
                  }
                ]
              },
              {
                id: 1022,
                parentId: 870,
                name: '全局搜索',
                local: 'hive_00051.html',
                children: [
                  {
                    id: 1059,
                    parentId: 1022,
                    name: '全局搜索资源',
                    local: 'hive_00052.html'
                  },
                  {
                    id: 1060,
                    parentId: 1022,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002002845722.html'
                  }
                ]
              },
              {
                id: 1023,
                parentId: 870,
                name: 'SLA',
                local: 'hive_00055.html',
                children: [
                  {
                    id: 1061,
                    parentId: 1023,
                    name: '查看SLA信息',
                    local: 'hive_00057.html'
                  },
                  {
                    id: 1062,
                    parentId: 1023,
                    name: '管理SLA',
                    local: 'hive_00058.html'
                  }
                ]
              },
              {
                id: 1024,
                parentId: 870,
                name: '副本',
                local: 'hive_00059.html',
                children: [
                  {
                    id: 1063,
                    parentId: 1024,
                    name: '查询Hive副本信息',
                    local: 'hive_00060.html'
                  },
                  {
                    id: 1064,
                    parentId: 1024,
                    name: '管理Hive副本',
                    local: 'hive_00061.html'
                  }
                ]
              },
              {
                id: 1025,
                parentId: 870,
                name: 'Hive集群环境',
                local: 'hive_00062.html',
                children: [
                  {
                    id: 1065,
                    parentId: 1025,
                    name: '查询Hive信息',
                    local: 'hive_00063.html'
                  },
                  {
                    id: 1066,
                    parentId: 1025,
                    name: '管理Hive集群',
                    local: 'hive_00064.html'
                  },
                  {
                    id: 1067,
                    parentId: 1025,
                    name: '管理备份集',
                    local: 'hive_00065.html'
                  }
                ]
              }
            ]
          },
          {
            id: 871,
            parentId: 14,
            name: 'MongoDB数据保护',
            local: 'zh-cn_topic_0000001873679221.html',
            children: [
              {
                id: 1068,
                parentId: 871,
                name: '备份',
                local: 'mongodb-0005.html',
                children: [
                  {
                    id: 1076,
                    parentId: 1068,
                    name: '备份前准备',
                    local: 'mongodb-0008.html'
                  },
                  {
                    id: 1077,
                    parentId: 1068,
                    name: '备份MongoDB数据库',
                    local: 'mongodb-0009.html',
                    children: [
                      {
                        id: 1078,
                        parentId: 1077,
                        name: '步骤1：注册MongoDB实例',
                        local: 'mongodb-0010.html'
                      },
                      {
                        id: 1079,
                        parentId: 1077,
                        name: '步骤2：（可选）创建限速策略',
                        local: 'mongodb-0011.html'
                      },
                      {
                        id: 1080,
                        parentId: 1077,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'mongodb-0012.html'
                      },
                      {
                        id: 1081,
                        parentId: 1077,
                        name: '步骤4：创建备份SLA',
                        local: 'mongodb-0013.html'
                      },
                      {
                        id: 1082,
                        parentId: 1077,
                        name: '步骤5：执行备份',
                        local: 'mongodb-0014.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1069,
                parentId: 871,
                name: '复制',
                local: 'oracle_gud_000035_0.html',
                children: [
                  {
                    id: 1083,
                    parentId: 1069,
                    name: '复制MongoDB副本',
                    local: 'mongodb-0019.html',
                    children: [
                      {
                        id: 1084,
                        parentId: 1083,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_0.html'
                      },
                      {
                        id: 1085,
                        parentId: 1083,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_0.html'
                      },
                      {
                        id: 1086,
                        parentId: 1083,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'mongodb-0022.html'
                      },
                      {
                        id: 1087,
                        parentId: 1083,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'mongodb-0023.html'
                      },
                      {
                        id: 1088,
                        parentId: 1083,
                        name: '步骤4：下载并导入证书',
                        local: 'mongodb-0024.html'
                      },
                      {
                        id: 1089,
                        parentId: 1083,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'mongodb-0025.html'
                      },
                      {
                        id: 1090,
                        parentId: 1083,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002010413522.html'
                      },
                      {
                        id: 1091,
                        parentId: 1083,
                        name: '步骤：添加复制集群',
                        local: 'mongodb-0026.html'
                      },
                      {
                        id: 1092,
                        parentId: 1083,
                        name: '步骤7：创建复制SLA',
                        local: 'mongodb-0027.html'
                      },
                      {
                        id: 1093,
                        parentId: 1083,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'mongodb-0028.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1070,
                parentId: 871,
                name: '归档',
                local: 'mongodb-0029.html',
                children: [
                  {
                    id: 1094,
                    parentId: 1070,
                    name: '归档MongoDB备份副本',
                    local: 'mongodb-0032.html',
                    children: [
                      {
                        id: 1096,
                        parentId: 1094,
                        name: '步骤1：添加归档存储',
                        local: 'mongodb-0033.html',
                        children: [
                          {
                            id: 1098,
                            parentId: 1096,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'mongodb-0034.html'
                          },
                          {
                            id: 1099,
                            parentId: 1096,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'mongodb-0035.html'
                          }
                        ]
                      },
                      {
                        id: 1097,
                        parentId: 1094,
                        name: '步骤2：创建归档SLA',
                        local: 'mongodb-0036.html'
                      }
                    ]
                  },
                  {
                    id: 1095,
                    parentId: 1070,
                    name: '归档MongoDB复制副本',
                    local: 'mongodb-0037.html',
                    children: [
                      {
                        id: 1100,
                        parentId: 1095,
                        name: '步骤1：创建归档SLA',
                        local: 'mongodb-0038.html'
                      },
                      {
                        id: 1101,
                        parentId: 1095,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'mongodb-0039.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1071,
                parentId: 871,
                name: '恢复',
                local: 'mongodb-0040.html',
                children: [
                  {
                    id: 1102,
                    parentId: 1071,
                    name: '恢复MongoDB',
                    local: 'mongodb-0043.html'
                  }
                ]
              },
              {
                id: 1072,
                parentId: 871,
                name: '全局搜索',
                local: 'mongodb-0044.html',
                children: [
                  {
                    id: 1103,
                    parentId: 1072,
                    name: '全局搜索资源',
                    local: 'mongodb-0046.html'
                  },
                  {
                    id: 1104,
                    parentId: 1072,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002002841954.html'
                  }
                ]
              },
              {
                id: 1073,
                parentId: 871,
                name: 'SLA',
                local: 'zh-cn_topic_0000001954665865.html',
                children: [
                  {
                    id: 1105,
                    parentId: 1073,
                    name: '关于SLA',
                    local: 'vmware_gud_000026.html'
                  },
                  {
                    id: 1106,
                    parentId: 1073,
                    name: '查看SLA信息',
                    local: 'zh-cn_topic_0000001927506528.html'
                  },
                  {
                    id: 1107,
                    parentId: 1073,
                    name: '管理SLA',
                    local: 'zh-cn_topic_0000001927347172.html'
                  }
                ]
              },
              {
                id: 1074,
                parentId: 871,
                name: '副本',
                local: 'mongodb-0053.html',
                children: [
                  {
                    id: 1108,
                    parentId: 1074,
                    name: '查看MongoDB副本信息',
                    local: 'mongodb-0054.html'
                  },
                  {
                    id: 1109,
                    parentId: 1074,
                    name: '管理MongoDB副本',
                    local: 'mongodb-0055.html'
                  }
                ]
              },
              {
                id: 1075,
                parentId: 871,
                name: 'MongoDB环境',
                local: 'mongodb-0056.html',
                children: [
                  {
                    id: 1110,
                    parentId: 1075,
                    name: '查看MongoDB环境信息',
                    local: 'mongodb-0057.html'
                  },
                  {
                    id: 1111,
                    parentId: 1075,
                    name: '管理MongoDB',
                    local: 'mongodb-0058.html'
                  }
                ]
              }
            ]
          },
          {
            id: 872,
            parentId: 14,
            name: 'ElasticSearch数据保护',
            local: 'zh-cn_topic_0000001873759397.html',
            children: [
              {
                id: 1112,
                parentId: 872,
                name: '备份',
                local: 'ES_gud_00007.html',
                children: [
                  {
                    id: 1120,
                    parentId: 1112,
                    name: '备份前准备',
                    local: 'ES_gud_00010.html'
                  },
                  {
                    id: 1121,
                    parentId: 1112,
                    name: '备份Elasticsearch集群',
                    local: 'ES_gud_00011.html',
                    children: [
                      {
                        id: 1122,
                        parentId: 1121,
                        name: '步骤1：（可选）开启安全加密模式',
                        local: 'ES_gud_00012.html'
                      },
                      {
                        id: 1123,
                        parentId: 1121,
                        name: '步骤2：创建并配置挂载目录',
                        local: 'ES_gud_00013.html'
                      },
                      {
                        id: 1124,
                        parentId: 1121,
                        name: '步骤3：注册Elasticsearch集群',
                        local: 'ES_gud_00014.html'
                      },
                      {
                        id: 1125,
                        parentId: 1121,
                        name: '步骤4：创建Elasticsearch备份集',
                        local: 'ES_gud_00015.html'
                      },
                      {
                        id: 1126,
                        parentId: 1121,
                        name: '步骤5：创建限速策略',
                        local: 'ES_gud_00016.html'
                      },
                      {
                        id: 1127,
                        parentId: 1121,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'ES_gud_00017.html'
                      },
                      {
                        id: 1128,
                        parentId: 1121,
                        name: '步骤7：创建备份SLA',
                        local: 'ES_gud_00018.html'
                      },
                      {
                        id: 1129,
                        parentId: 1121,
                        name: '步骤8：执行备份',
                        local: 'ES_gud_00019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1113,
                parentId: 872,
                name: '复制',
                local: 'ES_gud_00022.html',
                children: [
                  {
                    id: 1130,
                    parentId: 1113,
                    name: '复制Elasticsearch副本',
                    local: 'ES_gud_00025.html',
                    children: [
                      {
                        id: 1131,
                        parentId: 1130,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'ES_gud_00027.html'
                      },
                      {
                        id: 1132,
                        parentId: 1130,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_24.html'
                      },
                      {
                        id: 1133,
                        parentId: 1130,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'ES_gud_00028.html'
                      },
                      {
                        id: 1134,
                        parentId: 1130,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'ES_gud_00029.html'
                      },
                      {
                        id: 1135,
                        parentId: 1130,
                        name: '步骤4：下载并导入证书',
                        local: 'ES_gud_00030.html'
                      },
                      {
                        id: 1136,
                        parentId: 1130,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'ES_gud_00031.html'
                      },
                      {
                        id: 1137,
                        parentId: 1130,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002046700657.html'
                      },
                      {
                        id: 1138,
                        parentId: 1130,
                        name: '步骤6：添加复制集群',
                        local: 'ES_gud_00032.html'
                      },
                      {
                        id: 1139,
                        parentId: 1130,
                        name: '步骤7：创建复制SLA',
                        local: 'ES_gud_00033.html'
                      },
                      {
                        id: 1140,
                        parentId: 1130,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'ES_gud_00034.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1114,
                parentId: 872,
                name: '归档',
                local: 'ES_gud_00035.html',
                children: [
                  {
                    id: 1141,
                    parentId: 1114,
                    name: '归档Elasticsearch备份副本',
                    local: 'ES_gud_00038.html',
                    children: [
                      {
                        id: 1143,
                        parentId: 1141,
                        name: '步骤1：添加归档存储',
                        local: 'ES_gud_00039.html',
                        children: [
                          {
                            id: 1145,
                            parentId: 1143,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'ES_gud_00040.html'
                          },
                          {
                            id: 1146,
                            parentId: 1143,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'ES_gud_00041.html'
                          }
                        ]
                      },
                      {
                        id: 1144,
                        parentId: 1141,
                        name: '步骤2：创建归档SLA',
                        local: 'ES_gud_00042.html'
                      }
                    ]
                  },
                  {
                    id: 1142,
                    parentId: 1114,
                    name: '归档Elasticsearch复制副本',
                    local: 'ES_gud_00043.html',
                    children: [
                      {
                        id: 1147,
                        parentId: 1142,
                        name: '步骤1：创建归档SLA',
                        local: 'ES_gud_00044.html'
                      },
                      {
                        id: 1148,
                        parentId: 1142,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'ES_gud_00045.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1115,
                parentId: 872,
                name: '恢复',
                local: 'ES_gud_00046.html',
                children: [
                  {
                    id: 1149,
                    parentId: 1115,
                    name: '恢复Elasticsearch备份集',
                    local: 'ES_gud_00049.html'
                  },
                  {
                    id: 1150,
                    parentId: 1115,
                    name: '恢复Elasticsearch备份集中的单个或多个索引',
                    local: 'ES_gud_00050.html'
                  }
                ]
              },
              {
                id: 1116,
                parentId: 872,
                name: '全局搜索',
                local: 'ES_gud_00047_a1.html',
                children: [
                  {
                    id: 1151,
                    parentId: 1116,
                    name: '全局搜索资源',
                    local: 'ES_gud_00047_a2.html'
                  },
                  {
                    id: 1152,
                    parentId: 1116,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'ES_gud_00047_a3.html'
                  }
                ]
              },
              {
                id: 1117,
                parentId: 872,
                name: 'SLA',
                local: 'ES_gud_00055.html',
                children: [
                  {
                    id: 1153,
                    parentId: 1117,
                    name: '查看SLA信息',
                    local: 'ES_gud_00057.html'
                  },
                  {
                    id: 1154,
                    parentId: 1117,
                    name: '管理SLA',
                    local: 'ES_gud_00058.html'
                  }
                ]
              },
              {
                id: 1118,
                parentId: 872,
                name: '副本',
                local: 'ES_gud_00059.html',
                children: [
                  {
                    id: 1155,
                    parentId: 1118,
                    name: '查询Elasticsearch副本信息',
                    local: 'ES_gud_00060.html'
                  },
                  {
                    id: 1156,
                    parentId: 1118,
                    name: '管理Elasticsearch副本',
                    local: 'ES_gud_00061.html'
                  }
                ]
              },
              {
                id: 1119,
                parentId: 872,
                name: 'Elasticsearch集群环境',
                local: 'ES_gud_00062.html',
                children: [
                  {
                    id: 1157,
                    parentId: 1119,
                    name: '查询Elasticsearch信息',
                    local: 'ES_gud_00063.html'
                  },
                  {
                    id: 1158,
                    parentId: 1119,
                    name: '管理Elasticsearch集群',
                    local: 'ES_gud_00064.html'
                  },
                  {
                    id: 1159,
                    parentId: 1119,
                    name: '管理备份集',
                    local: 'ES_gud_00065.html'
                  }
                ]
              }
            ]
          },
          {
            id: 873,
            parentId: 14,
            name: 'HDFS数据保护',
            local: 'product_documentation_000031.html',
            children: [
              {
                id: 1160,
                parentId: 873,
                name: '备份',
                local: 'hdfs_00007.html',
                children: [
                  {
                    id: 1168,
                    parentId: 1160,
                    name: '备份前准备',
                    local: 'hdfs_00010.html'
                  },
                  {
                    id: 1169,
                    parentId: 1160,
                    name: '备份HDFS文件集',
                    local: 'hdfs_00011.html',
                    children: [
                      {
                        id: 1170,
                        parentId: 1169,
                        name: '步骤1：开启HDFS目录的快照功能',
                        local: 'hdfs_00012.html'
                      },
                      {
                        id: 1171,
                        parentId: 1169,
                        name: '步骤2：检查HDFS ACL的开关状态',
                        local: 'hdfs_00013.html'
                      },
                      {
                        id: 1172,
                        parentId: 1169,
                        name: '步骤3：注册HDFS集群',
                        local: 'hdfs_00014.html'
                      },
                      {
                        id: 1173,
                        parentId: 1169,
                        name: '步骤4：创建HDFS文件集',
                        local: 'hdfs_00015.html'
                      },
                      {
                        id: 1174,
                        parentId: 1169,
                        name: '步骤5：创建限速策略',
                        local: 'hdfs_00016.html'
                      },
                      {
                        id: 1175,
                        parentId: 1169,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'hdfs_00017.html'
                      },
                      {
                        id: 1176,
                        parentId: 1169,
                        name: '步骤7：创建备份SLA',
                        local: 'hdfs_00018.html'
                      },
                      {
                        id: 1177,
                        parentId: 1169,
                        name: '步骤8：执行备份',
                        local: 'hdfs_00019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1161,
                parentId: 873,
                name: '复制',
                local: 'hdfs_00022.html',
                children: [
                  {
                    id: 1178,
                    parentId: 1161,
                    name: '复制HDFS副本',
                    local: 'hdfs_00025.html',
                    children: [
                      {
                        id: 1179,
                        parentId: 1178,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'hdfs_00027.html'
                      },
                      {
                        id: 1180,
                        parentId: 1178,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_1.html'
                      },
                      {
                        id: 1181,
                        parentId: 1178,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'hdfs_00028.html'
                      },
                      {
                        id: 1182,
                        parentId: 1178,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'hdfs_00029.html'
                      },
                      {
                        id: 1183,
                        parentId: 1178,
                        name: '步骤4：下载并导入证书',
                        local: 'hdfs_00030.html'
                      },
                      {
                        id: 1184,
                        parentId: 1178,
                        name: '步骤5：创建远端设备管理员',
                        local: 'hdfs_00031.html'
                      },
                      {
                        id: 1185,
                        parentId: 1178,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002046697729.html'
                      },
                      {
                        id: 1186,
                        parentId: 1178,
                        name: '步骤6：添加复制集群',
                        local: 'hdfs_00032.html'
                      },
                      {
                        id: 1187,
                        parentId: 1178,
                        name: '步骤7：创建复制SLA',
                        local: 'hdfs_00033.html'
                      },
                      {
                        id: 1188,
                        parentId: 1178,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'hdfs_00034.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1162,
                parentId: 873,
                name: '归档',
                local: 'hdfs_00035.html',
                children: [
                  {
                    id: 1189,
                    parentId: 1162,
                    name: '归档HDFS备份副本',
                    local: 'hdfs_00038.html',
                    children: [
                      {
                        id: 1191,
                        parentId: 1189,
                        name: '步骤1：添加归档存储',
                        local: 'hdfs_00039.html',
                        children: [
                          {
                            id: 1193,
                            parentId: 1191,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'hdfs_00040.html'
                          },
                          {
                            id: 1194,
                            parentId: 1191,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'hdfs_00041.html'
                          }
                        ]
                      },
                      {
                        id: 1192,
                        parentId: 1189,
                        name: '步骤2：创建归档SLA',
                        local: 'hdfs_00042.html'
                      }
                    ]
                  },
                  {
                    id: 1190,
                    parentId: 1162,
                    name: '归档HDFS复制副本',
                    local: 'hdfs_00043.html',
                    children: [
                      {
                        id: 1195,
                        parentId: 1190,
                        name: '步骤1：创建归档SLA',
                        local: 'hdfs_00044.html'
                      },
                      {
                        id: 1196,
                        parentId: 1190,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'hdfs_00045.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1163,
                parentId: 873,
                name: '恢复',
                local: 'hdfs_00046.html',
                children: [
                  {
                    id: 1197,
                    parentId: 1163,
                    name: '恢复HDFS文件集',
                    local: 'hdfs_00048.html'
                  }
                ]
              },
              {
                id: 1164,
                parentId: 873,
                name: '全局搜索',
                local: 'hdfs_00050.html',
                children: [
                  {
                    id: 1198,
                    parentId: 1164,
                    name: '全局搜索副本数据',
                    local: 'hdfs_00051.html'
                  },
                  {
                    id: 1199,
                    parentId: 1164,
                    name: '全局搜索资源',
                    local: 'hdfs_00052.html'
                  },
                  {
                    id: 1200,
                    parentId: 1164,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002002686646.html'
                  }
                ]
              },
              {
                id: 1165,
                parentId: 873,
                name: 'SLA',
                local: 'hdfs_00055.html',
                children: [
                  {
                    id: 1201,
                    parentId: 1165,
                    name: '查看SLA信息',
                    local: 'hdfs_00057.html'
                  },
                  {
                    id: 1202,
                    parentId: 1165,
                    name: '管理SLA',
                    local: 'hdfs_00058.html'
                  }
                ]
              },
              {
                id: 1166,
                parentId: 873,
                name: '副本',
                local: 'hdfs_00059.html',
                children: [
                  {
                    id: 1203,
                    parentId: 1166,
                    name: '查询HDFS副本信息',
                    local: 'hdfs_00060.html'
                  },
                  {
                    id: 1204,
                    parentId: 1166,
                    name: '管理HDFS副本',
                    local: 'hdfs_00061.html'
                  }
                ]
              },
              {
                id: 1167,
                parentId: 873,
                name: 'HDFS集群环境',
                local: 'hdfs_00062.html',
                children: [
                  {
                    id: 1205,
                    parentId: 1167,
                    name: '查询HDFS信息',
                    local: 'hdfs_00063.html'
                  },
                  {
                    id: 1206,
                    parentId: 1167,
                    name: '管理HDFS集群',
                    local: 'hdfs_00064.html'
                  },
                  {
                    id: 1207,
                    parentId: 1167,
                    name: '管理HDFS文件集',
                    local: 'hdfs_00065.html'
                  }
                ]
              }
            ]
          },
          {
            id: 874,
            parentId: 14,
            name: 'Redis数据保护',
            local: 'zh-cn_topic_0000001873759393.html',
            children: [
              {
                id: 1208,
                parentId: 874,
                name: '备份',
                local: 'redis-00003.html',
                children: [
                  {
                    id: 1217,
                    parentId: 1208,
                    name: '备份前准备',
                    local: 'redis-00006.html'
                  },
                  {
                    id: 1218,
                    parentId: 1208,
                    name: '备份Redis集群',
                    local: 'redis-00007.html',
                    children: [
                      {
                        id: 1219,
                        parentId: 1218,
                        name: '步骤1：注册Redis集群',
                        local: 'redis-00008.html'
                      },
                      {
                        id: 1220,
                        parentId: 1218,
                        name: '步骤2：创建限速策略',
                        local: 'redis-00009.html'
                      },
                      {
                        id: 1221,
                        parentId: 1218,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'redis-00010.html'
                      },
                      {
                        id: 1222,
                        parentId: 1218,
                        name: '步骤4：创建备份SLA',
                        local: 'redis-00011.html'
                      },
                      {
                        id: 1223,
                        parentId: 1218,
                        name: '步骤5：执行备份',
                        local: 'redis-00012.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1209,
                parentId: 874,
                name: '复制',
                local: 'oracle_gud_000035_3.html',
                children: [
                  {
                    id: 1224,
                    parentId: 1209,
                    name: '复制Redis副本',
                    local: 'redis-00017.html',
                    children: [
                      {
                        id: 1225,
                        parentId: 1224,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_4.html'
                      },
                      {
                        id: 1226,
                        parentId: 1224,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_10.html'
                      },
                      {
                        id: 1227,
                        parentId: 1224,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'redis-00020.html'
                      },
                      {
                        id: 1228,
                        parentId: 1224,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'redis-00021.html'
                      },
                      {
                        id: 1229,
                        parentId: 1224,
                        name: '步骤4：下载并导入证书',
                        local: 'redis-00022.html'
                      },
                      {
                        id: 1230,
                        parentId: 1224,
                        name: '步骤5：创建远端设备管理员',
                        local: 'redis-00023.html'
                      },
                      {
                        id: 1231,
                        parentId: 1224,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002046614601.html'
                      },
                      {
                        id: 1232,
                        parentId: 1224,
                        name: '步骤：添加复制集群',
                        local: 'redis-00024.html'
                      },
                      {
                        id: 1233,
                        parentId: 1224,
                        name: '步骤7：创建复制SLA',
                        local: 'redis-00025.html'
                      },
                      {
                        id: 1234,
                        parentId: 1224,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'redis-00026.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1210,
                parentId: 874,
                name: '归档',
                local: 'redis-00027.html',
                children: [
                  {
                    id: 1235,
                    parentId: 1210,
                    name: '归档Redis备份副本',
                    local: 'redis-00030.html',
                    children: [
                      {
                        id: 1237,
                        parentId: 1235,
                        name: '步骤1：添加归档存储',
                        local: 'redis-00031.html',
                        children: [
                          {
                            id: 1239,
                            parentId: 1237,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'redis-00032.html'
                          },
                          {
                            id: 1240,
                            parentId: 1237,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'redis-00033.html'
                          }
                        ]
                      },
                      {
                        id: 1238,
                        parentId: 1235,
                        name: '步骤2：创建归档SLA',
                        local: 'redis-00034.html'
                      }
                    ]
                  },
                  {
                    id: 1236,
                    parentId: 1210,
                    name: '归档Redis复制副本',
                    local: 'redis-00035.html',
                    children: [
                      {
                        id: 1241,
                        parentId: 1236,
                        name: '步骤1：创建归档SLA',
                        local: 'redis-00036.html'
                      },
                      {
                        id: 1242,
                        parentId: 1236,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'redis-00037.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1211,
                parentId: 874,
                name: '恢复',
                local: 'redis-00038.html',
                children: [
                  {
                    id: 1243,
                    parentId: 1211,
                    name: '恢复Redis集群',
                    local: 'redis-00041.html'
                  }
                ]
              },
              {
                id: 1212,
                parentId: 874,
                name: '全局搜索',
                local: 'zh-cn_topic_0000002038763381.html',
                children: [
                  {
                    id: 1244,
                    parentId: 1212,
                    name: '全局搜索资源',
                    local: 'redis-00042.html'
                  },
                  {
                    id: 1245,
                    parentId: 1212,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002002843406.html'
                  }
                ]
              },
              {
                id: 1213,
                parentId: 874,
                name: '数据重删压缩',
                local: 'redis-00043.html',
                children: [
                  {
                    id: 1246,
                    parentId: 1213,
                    name: '关于数据重删压缩',
                    local: 'redis-00044.html'
                  }
                ]
              },
              {
                id: 1214,
                parentId: 874,
                name: 'SLA',
                local: 'redis-00045.html',
                children: [
                  {
                    id: 1247,
                    parentId: 1214,
                    name: '关于SLA',
                    local: 'redis-00046.html'
                  },
                  {
                    id: 1248,
                    parentId: 1214,
                    name: '查看SLA信息',
                    local: 'redis-00047.html'
                  },
                  {
                    id: 1249,
                    parentId: 1214,
                    name: '管理SLA',
                    local: 'redis-00048.html'
                  }
                ]
              },
              {
                id: 1215,
                parentId: 874,
                name: '副本',
                local: 'redis-00049.html',
                children: [
                  {
                    id: 1250,
                    parentId: 1215,
                    name: '查看Redis副本信息',
                    local: 'redis-00050.html'
                  },
                  {
                    id: 1251,
                    parentId: 1215,
                    name: '管理Redis副本',
                    local: 'redis-00051.html'
                  }
                ]
              },
              {
                id: 1216,
                parentId: 874,
                name: 'Redis集群环境',
                local: 'redis-00052.html',
                children: [
                  {
                    id: 1252,
                    parentId: 1216,
                    name: '查询Redis信息',
                    local: 'redis-00053.html'
                  },
                  {
                    id: 1253,
                    parentId: 1216,
                    name: '管理Redis集群',
                    local: 'redis-00054.html'
                  },
                  {
                    id: 1254,
                    parentId: 1216,
                    name: '管理Redis集群保护',
                    local: 'redis-00055.html'
                  }
                ]
              }
            ]
          }
        ]
      },
      {
        id: 15,
        parentId: 3,
        name: '虚拟化',
        local: 'zh-cn_topic_0000001918470736.html',
        children: [
          {
            id: 1255,
            parentId: 15,
            name: 'VMware数据保护',
            local: 'product_documentation_000027.html',
            children: [
              {
                id: 1260,
                parentId: 1255,
                name: '备份',
                local: 'vmware_gud_0007.html',
                children: [
                  {
                    id: 1269,
                    parentId: 1260,
                    name: '备份前准备',
                    local: 'vmware_gud_0014.html'
                  },
                  {
                    id: 1270,
                    parentId: 1260,
                    name: '备份VMware虚拟机',
                    local: 'vmware_gud_0015.html',
                    children: [
                      {
                        id: 1271,
                        parentId: 1270,
                        name: '步骤1：检查并安装VMware Tools',
                        local: 'vmware_gud_0016.html'
                      },
                      {
                        id: 1272,
                        parentId: 1270,
                        name: '步骤2：检查并开启vmware-vapi-endpoint服务',
                        local: 'vmware_gud_0017.html'
                      },
                      {
                        id: 1273,
                        parentId: 1270,
                        name: '步骤3：配置应用一致性备份脚本',
                        local: 'vmware_gud_0018.html',
                        children: [
                          {
                            id: 1282,
                            parentId: 1273,
                            name: 'DB2数据库',
                            local: 'vmware_gud_0019.html'
                          },
                          {
                            id: 1283,
                            parentId: 1273,
                            name: 'Oracle数据库',
                            local: 'vmware_gud_0020.html'
                          },
                          {
                            id: 1284,
                            parentId: 1273,
                            name: 'Sybase数据库',
                            local: 'vmware_gud_0021.html'
                          },
                          {
                            id: 1285,
                            parentId: 1273,
                            name: 'MySQL数据库',
                            local: 'vmware_gud_0022.html'
                          }
                        ]
                      },
                      {
                        id: 1274,
                        parentId: 1270,
                        name: '步骤4：获取VMware证书',
                        local: 'vmware_gud_0023.html'
                      },
                      {
                        id: 1275,
                        parentId: 1270,
                        name: '步骤5：注册VMware虚拟化环境',
                        local: 'vmware_gud_0024.html'
                      },
                      {
                        id: 1276,
                        parentId: 1270,
                        name:
                          '步骤6：（可选）创建VMware虚拟机组（适用于1.6.0及后续版本）',
                        local: 'vmware_gud_0024_1.html'
                      },
                      {
                        id: 1277,
                        parentId: 1270,
                        name: '步骤7：（可选）创建限速策略',
                        local: 'vmware_gud_0025.html'
                      },
                      {
                        id: 1278,
                        parentId: 1270,
                        name: '步骤8：（可选）开启备份链路加密开关',
                        local: 'vmware_gud_0026.html'
                      },
                      {
                        id: 1279,
                        parentId: 1270,
                        name: '步骤9：（可选）登录iSCSI启动器',
                        local: 'vmwate_gud_0110.html'
                      },
                      {
                        id: 1280,
                        parentId: 1270,
                        name: '步骤10：创建备份SLA',
                        local: 'vmware_gud_0027.html'
                      },
                      {
                        id: 1281,
                        parentId: 1270,
                        name: '步骤11：执行备份',
                        local: 'vmware_gud_0028.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1261,
                parentId: 1255,
                name: '复制',
                local: 'vmware_gud_0031.html',
                children: [
                  {
                    id: 1286,
                    parentId: 1261,
                    name: '复制VMware虚拟机副本',
                    local: 'vmware_gud_0034.html',
                    children: [
                      {
                        id: 1287,
                        parentId: 1286,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'vmware_gud_0036.html'
                      },
                      {
                        id: 1288,
                        parentId: 1286,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_9.html'
                      },
                      {
                        id: 1289,
                        parentId: 1286,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'vmware_gud_0036_1.html'
                      },
                      {
                        id: 1290,
                        parentId: 1286,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'vmware_gud_0038.html'
                      },
                      {
                        id: 1291,
                        parentId: 1286,
                        name: '步骤4：下载并导入证书',
                        local: 'vmware_gud_0039.html'
                      },
                      {
                        id: 1292,
                        parentId: 1286,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'vmware_gud_0040.html'
                      },
                      {
                        id: 1293,
                        parentId: 1286,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002008299624.html'
                      },
                      {
                        id: 1294,
                        parentId: 1286,
                        name: '步骤6：添加复制集群',
                        local: 'vmware_gud_0041.html'
                      },
                      {
                        id: 1295,
                        parentId: 1286,
                        name: '步骤7：创建复制SLA',
                        local: 'vmware_gud_0042_1.html'
                      },
                      {
                        id: 1296,
                        parentId: 1286,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'vmware_gud_0043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1262,
                parentId: 1255,
                name: '归档',
                local: 'vmware_gud_0044.html',
                children: [
                  {
                    id: 1297,
                    parentId: 1262,
                    name: '归档VMware备份副本',
                    local: 'vmware_gud_0047.html',
                    children: [
                      {
                        id: 1299,
                        parentId: 1297,
                        name: '步骤1：添加归档存储',
                        local: 'vmware_gud_0048.html',
                        children: [
                          {
                            id: 1301,
                            parentId: 1299,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'vmware_gud_0049.html'
                          },
                          {
                            id: 1302,
                            parentId: 1299,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'vmware_gud_0050.html'
                          }
                        ]
                      },
                      {
                        id: 1300,
                        parentId: 1297,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'vmware_gud_0051.html'
                      }
                    ]
                  },
                  {
                    id: 1298,
                    parentId: 1262,
                    name: '归档VMware复制副本',
                    local: 'vmware_gud_0052.html',
                    children: [
                      {
                        id: 1303,
                        parentId: 1298,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'vmware_gud_0053.html'
                      },
                      {
                        id: 1304,
                        parentId: 1298,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'vmware_gud_0054.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1263,
                parentId: 1255,
                name: '恢复',
                local: 'vmware_gud_0055.html',
                children: [
                  {
                    id: 1305,
                    parentId: 1263,
                    name: '恢复VMware虚拟机',
                    local: 'vmware_gud_0058.html'
                  },
                  {
                    id: 1306,
                    parentId: 1263,
                    name: '恢复VMware虚拟机磁盘',
                    local: 'vmware_gud_0059.html'
                  }
                ]
              },
              {
                id: 1264,
                parentId: 1255,
                name: '即时恢复',
                local: 'vmware_gud_0061.html',
                children: [
                  {
                    id: 1307,
                    parentId: 1264,
                    name: '即时恢复VMware虚拟机',
                    local: 'vmware_gud_0064.html'
                  }
                ]
              },
              {
                id: 1265,
                parentId: 1255,
                name: '全局搜索',
                local: 'vmware_gud_0074.html',
                children: [
                  {
                    id: 1308,
                    parentId: 1265,
                    name: '关于全局搜索',
                    local: 'vmware_gud_0075.html'
                  },
                  {
                    id: 1309,
                    parentId: 1265,
                    name: '全局搜索副本数据',
                    local: 'vmware_gud_0076.html'
                  },
                  {
                    id: 1310,
                    parentId: 1265,
                    name: '全局搜索资源',
                    local: 'vmware_gud_0077.html'
                  },
                  {
                    id: 1311,
                    parentId: 1265,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002038754917.html'
                  }
                ]
              },
              {
                id: 1266,
                parentId: 1255,
                name: 'SLA',
                local: 'vmware_gud_0080.html',
                children: [
                  {
                    id: 1312,
                    parentId: 1266,
                    name: '查看SLA信息',
                    local: 'vmware_gud_0082.html'
                  },
                  {
                    id: 1313,
                    parentId: 1266,
                    name: '管理SLA',
                    local: 'vmware_gud_0083.html'
                  }
                ]
              },
              {
                id: 1267,
                parentId: 1255,
                name: '副本',
                local: 'vmware_gud_0084.html',
                children: [
                  {
                    id: 1314,
                    parentId: 1267,
                    name: '查看VMware副本信息',
                    local: 'vmware_gud_0085.html'
                  },
                  {
                    id: 1315,
                    parentId: 1267,
                    name: '管理VMware副本',
                    local: 'vmware_gud_0086.html'
                  }
                ]
              },
              {
                id: 1268,
                parentId: 1255,
                name: 'VMware虚拟化环境',
                local: 'vmware_gud_0087.html',
                children: [
                  {
                    id: 1316,
                    parentId: 1268,
                    name: '查看VMware虚拟化环境信息',
                    local: 'vmware_gud_0088.html'
                  },
                  {
                    id: 1317,
                    parentId: 1268,
                    name: '管理VMware注册信息',
                    local: 'vmware_gud_0089.html'
                  },
                  {
                    id: 1318,
                    parentId: 1268,
                    name: '管理集群/主机/虚拟机/虚拟机组',
                    local: 'vmware_gud_0090.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1256,
            parentId: 15,
            name: 'FusionCompute数据保护',
            local: 'zh-cn_topic_0000001873679177.html',
            children: [
              {
                id: 1319,
                parentId: 1256,
                name: '备份',
                local: 'fc_gud_0009.html',
                children: [
                  {
                    id: 1327,
                    parentId: 1319,
                    name: '备份前准备',
                    local: 'fc_gud_0012.html'
                  },
                  {
                    id: 1328,
                    parentId: 1319,
                    name: '备份FusionCompute虚拟机',
                    local: 'fc_gud_0013.html',
                    children: [
                      {
                        id: 1329,
                        parentId: 1328,
                        name: '步骤1：创建FusionCompute对接用户',
                        local: 'fc_gud_0014.html'
                      },
                      {
                        id: 1330,
                        parentId: 1328,
                        name: '步骤2：注册FusionCompute虚拟化环境',
                        local: 'fc_gud_0015.html'
                      },
                      {
                        id: 1331,
                        parentId: 1328,
                        name:
                          '步骤3：（可选）创建FusionCompute虚拟机组（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0024_1.html'
                      },
                      {
                        id: 1332,
                        parentId: 1328,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'fc_gud_0016.html'
                      },
                      {
                        id: 1333,
                        parentId: 1328,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'fc_gud_0017.html'
                      },
                      {
                        id: 1334,
                        parentId: 1328,
                        name: '步骤6：创建备份SLA',
                        local: 'fc_gud_0018.html'
                      },
                      {
                        id: 1335,
                        parentId: 1328,
                        name: '步骤7：执行备份',
                        local: 'fc_gud_0019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1320,
                parentId: 1256,
                name: '复制',
                local: 'fc_gud_0022.html',
                children: [
                  {
                    id: 1336,
                    parentId: 1320,
                    name: '复制FusionCompute虚拟机副本',
                    local: 'fc_gud_0024.html',
                    children: [
                      {
                        id: 1337,
                        parentId: 1336,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_15.html'
                      },
                      {
                        id: 1338,
                        parentId: 1336,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_31.html'
                      },
                      {
                        id: 1339,
                        parentId: 1336,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'fc_gud_0027.html'
                      },
                      {
                        id: 1340,
                        parentId: 1336,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'fc_gud_0028.html'
                      },
                      {
                        id: 1341,
                        parentId: 1336,
                        name: '步骤4：下载并导入证书',
                        local: 'fc_gud_0029.html'
                      },
                      {
                        id: 1342,
                        parentId: 1336,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'fc_gud_0030_0.html'
                      },
                      {
                        id: 1343,
                        parentId: 1336,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002008271350.html'
                      },
                      {
                        id: 1344,
                        parentId: 1336,
                        name: '步骤6：添加复制集群',
                        local: 'fc_gud_0031.html'
                      },
                      {
                        id: 1345,
                        parentId: 1336,
                        name: '步骤7：创建复制SLA',
                        local: 'fc_gud_0032.html'
                      },
                      {
                        id: 1346,
                        parentId: 1336,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'fc_gud_0033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1321,
                parentId: 1256,
                name: '归档',
                local: 'fc_gud_0034.html',
                children: [
                  {
                    id: 1347,
                    parentId: 1321,
                    name: '归档FusionCompute备份副本',
                    local: 'fc_gud_0037.html',
                    children: [
                      {
                        id: 1349,
                        parentId: 1347,
                        name: '步骤1：添加归档存储',
                        local: 'fc_gud_0038.html',
                        children: [
                          {
                            id: 1351,
                            parentId: 1349,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'fc_gud_0039.html'
                          },
                          {
                            id: 1352,
                            parentId: 1349,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'fc_gud_0040.html'
                          }
                        ]
                      },
                      {
                        id: 1350,
                        parentId: 1347,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'fc_gud_0041.html'
                      }
                    ]
                  },
                  {
                    id: 1348,
                    parentId: 1321,
                    name: '归档FusionCompute复制副本',
                    local: 'fc_gud_0042.html',
                    children: [
                      {
                        id: 1353,
                        parentId: 1348,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'fc_gud_0043.html'
                      },
                      {
                        id: 1354,
                        parentId: 1348,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'fc_gud_0044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1322,
                parentId: 1256,
                name: '恢复',
                local: 'fc_gud_0045.html',
                children: [
                  {
                    id: 1355,
                    parentId: 1322,
                    name: '恢复FusionCompute虚拟机',
                    local: 'fc_gud_0048.html'
                  },
                  {
                    id: 1356,
                    parentId: 1322,
                    name: '恢复FusionCompute虚拟机磁盘',
                    local: 'fc_gud_0049.html'
                  }
                ]
              },
              {
                id: 1323,
                parentId: 1256,
                name: '全局搜索',
                local: 'fc_gud_gs1.html',
                children: [
                  {
                    id: 1357,
                    parentId: 1323,
                    name: '关于全局搜索',
                    local: 'zh-cn_topic_0000002038884249.html'
                  },
                  {
                    id: 1358,
                    parentId: 1323,
                    name: '全局搜索副本数据',
                    local: 'fc_gud_gs3.html'
                  },
                  {
                    id: 1359,
                    parentId: 1323,
                    name: '全局搜索资源',
                    local: 'fc_gud_0050.html'
                  },
                  {
                    id: 1360,
                    parentId: 1323,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002002642984.html'
                  }
                ]
              },
              {
                id: 1324,
                parentId: 1256,
                name: 'SLA',
                local: 'fc_gud_0053.html',
                children: [
                  {
                    id: 1361,
                    parentId: 1324,
                    name: '查看SLA信息',
                    local: 'fc_gud_0055.html'
                  },
                  {
                    id: 1362,
                    parentId: 1324,
                    name: '管理SLA',
                    local: 'fc_gud_0056.html'
                  }
                ]
              },
              {
                id: 1325,
                parentId: 1256,
                name: '副本',
                local: 'fc_gud_0057.html',
                children: [
                  {
                    id: 1363,
                    parentId: 1325,
                    name: '查看FusionCompute副本信息',
                    local: 'fc_gud_0058.html'
                  },
                  {
                    id: 1364,
                    parentId: 1325,
                    name: '管理FusionCompute副本',
                    local: 'fc_gud_0059.html'
                  }
                ]
              },
              {
                id: 1326,
                parentId: 1256,
                name: 'FusionCompute虚拟化环境',
                local: 'fc_gud_0060.html',
                children: [
                  {
                    id: 1365,
                    parentId: 1326,
                    name: '查看FusionCompute虚拟化环境信息',
                    local: 'fc_gud_0061.html'
                  },
                  {
                    id: 1366,
                    parentId: 1326,
                    name: '管理FusionCompute注册信息',
                    local: 'fc_gud_0062.html'
                  },
                  {
                    id: 1367,
                    parentId: 1326,
                    name: '管理集群/主机/虚拟机/虚拟机组',
                    local: 'fc_gud_0063.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1257,
            parentId: 15,
            name: 'CNware数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000001873679209.html',
            children: [
              {
                id: 1368,
                parentId: 1257,
                name: '备份',
                local: 'cnware_00007.html',
                children: [
                  {
                    id: 1377,
                    parentId: 1368,
                    name: '备份前准备',
                    local: 'cnware_00010.html'
                  },
                  {
                    id: 1378,
                    parentId: 1368,
                    name: '备份CNware虚拟机',
                    local: 'cnware_00011.html',
                    children: [
                      {
                        id: 1379,
                        parentId: 1378,
                        name: '步骤1：获取CNware证书',
                        local: 'cnware_00012.html'
                      },
                      {
                        id: 1380,
                        parentId: 1378,
                        name: '步骤2：注册CNware虚拟化环境',
                        local: 'cnware_00013.html'
                      },
                      {
                        id: 1381,
                        parentId: 1378,
                        name: '步骤3：（可选）创建CNware虚拟机组',
                        local: 'cnware_00014.html'
                      },
                      {
                        id: 1382,
                        parentId: 1378,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'cnware_00015.html'
                      },
                      {
                        id: 1383,
                        parentId: 1378,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'cnware_00016.html'
                      },
                      {
                        id: 1384,
                        parentId: 1378,
                        name: '步骤6：创建备份SLA',
                        local: 'cnware_00017.html'
                      },
                      {
                        id: 1385,
                        parentId: 1378,
                        name: '步骤7：执行备份',
                        local: 'cnware_00018.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1369,
                parentId: 1257,
                name: '复制',
                local: 'cnware_00021.html',
                children: [
                  {
                    id: 1386,
                    parentId: 1369,
                    name: '复制CNware虚拟机副本',
                    local: 'cnware_00024.html',
                    children: [
                      {
                        id: 1387,
                        parentId: 1386,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'cnware_00026.html'
                      },
                      {
                        id: 1388,
                        parentId: 1386,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_5.html'
                      },
                      {
                        id: 1389,
                        parentId: 1386,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'cnware_00027.html'
                      },
                      {
                        id: 1390,
                        parentId: 1386,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'cnware_00028.html'
                      },
                      {
                        id: 1391,
                        parentId: 1386,
                        name: '步骤4：下载并导入证书',
                        local: 'cnware_00029.html'
                      },
                      {
                        id: 1392,
                        parentId: 1386,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'cnware_00030.html'
                      },
                      {
                        id: 1393,
                        parentId: 1386,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'cnware_000300.html'
                      },
                      {
                        id: 1394,
                        parentId: 1386,
                        name: '步骤6：添加目标集群',
                        local: 'cnware_00031.html'
                      },
                      {
                        id: 1395,
                        parentId: 1386,
                        name: '步骤7：创建复制SLA',
                        local: 'cnware_00032.html'
                      },
                      {
                        id: 1396,
                        parentId: 1386,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'cnware_00033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1370,
                parentId: 1257,
                name: '归档',
                local: 'cnware_00034.html',
                children: [
                  {
                    id: 1397,
                    parentId: 1370,
                    name: '归档CNware备份副本',
                    local: 'cnware_00037.html',
                    children: [
                      {
                        id: 1399,
                        parentId: 1397,
                        name: '步骤1：添加归档存储',
                        local: 'cnware_00038.html',
                        children: [
                          {
                            id: 1401,
                            parentId: 1399,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'cnware_00039.html'
                          },
                          {
                            id: 1402,
                            parentId: 1399,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'cnware_00040.html'
                          }
                        ]
                      },
                      {
                        id: 1400,
                        parentId: 1397,
                        name: '步骤2：创建归档SLA',
                        local: 'cnware_00041.html'
                      }
                    ]
                  },
                  {
                    id: 1398,
                    parentId: 1370,
                    name: '归档CNware复制副本',
                    local: 'cnware_00042.html',
                    children: [
                      {
                        id: 1403,
                        parentId: 1398,
                        name: '步骤1：创建归档SLA',
                        local: 'cnware_00043.html'
                      },
                      {
                        id: 1404,
                        parentId: 1398,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'cnware_00044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1371,
                parentId: 1257,
                name: '恢复',
                local: 'cnware_00045.html',
                children: [
                  {
                    id: 1405,
                    parentId: 1371,
                    name: '恢复CNware虚拟机',
                    local: 'cnware_00048.html'
                  },
                  {
                    id: 1406,
                    parentId: 1371,
                    name: '恢复CNware虚拟机磁盘',
                    local: 'cnware_00049.html'
                  }
                ]
              },
              {
                id: 1372,
                parentId: 1257,
                name: '即时恢复',
                local: 'cnware_00051.html',
                children: [
                  {
                    id: 1407,
                    parentId: 1372,
                    name: '即时恢复CNware虚拟机',
                    local: 'cnware_00054.html'
                  }
                ]
              },
              {
                id: 1373,
                parentId: 1257,
                name: '全局搜索',
                local: 'cnware_00064.html',
                children: [
                  {
                    id: 1408,
                    parentId: 1373,
                    name: '关于全局搜索',
                    local: 'cnware_000641.html'
                  },
                  {
                    id: 1409,
                    parentId: 1373,
                    name: '全局搜索副本数据',
                    local: 'cnware_000642.html'
                  },
                  {
                    id: 1410,
                    parentId: 1373,
                    name: '全局搜索资源',
                    local: 'cnware_00067.html'
                  },
                  {
                    id: 1411,
                    parentId: 1373,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'cnware_000671.html'
                  }
                ]
              },
              {
                id: 1374,
                parentId: 1257,
                name: 'SLA',
                local: 'cnware_00070.html',
                children: [
                  {
                    id: 1412,
                    parentId: 1374,
                    name: '查看SLA信息',
                    local: 'cnware_00072.html'
                  },
                  {
                    id: 1413,
                    parentId: 1374,
                    name: '管理SLA',
                    local: 'cnware_00073.html'
                  }
                ]
              },
              {
                id: 1375,
                parentId: 1257,
                name: '副本',
                local: 'cnware_00074.html',
                children: [
                  {
                    id: 1414,
                    parentId: 1375,
                    name: '查看CNware副本信息',
                    local: 'cnware_00075.html'
                  },
                  {
                    id: 1415,
                    parentId: 1375,
                    name: '管理CNware副本',
                    local: 'cnware_00076.html'
                  }
                ]
              },
              {
                id: 1376,
                parentId: 1257,
                name: 'CNware虚拟化环境',
                local: 'cnware_00077.html',
                children: [
                  {
                    id: 1416,
                    parentId: 1376,
                    name: '查看CNware虚拟化环境信息',
                    local: 'cnware_00078.html'
                  },
                  {
                    id: 1417,
                    parentId: 1376,
                    name: '管理CNware注册信息',
                    local: 'cnware_00079.html'
                  },
                  {
                    id: 1418,
                    parentId: 1376,
                    name: '管理虚拟机/主机/集群',
                    local: 'cnware_00080.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1258,
            parentId: 15,
            name: 'Hyper-V数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002037019572.html',
            children: [
              {
                id: 1419,
                parentId: 1258,
                name: '备份',
                local: 'vmware_gud_0007_0.html',
                children: [
                  {
                    id: 1427,
                    parentId: 1419,
                    name: '备份前准备',
                    local: 'vmware_gud_0014_0.html'
                  },
                  {
                    id: 1428,
                    parentId: 1419,
                    name: '备份Hyper-V虚拟机',
                    local: 'vmware_gud_0015_0.html',
                    children: [
                      {
                        id: 1429,
                        parentId: 1428,
                        name: '步骤1：注册Hyper-V虚拟化环境',
                        local: 'vmware_gud_0024_0.html'
                      },
                      {
                        id: 1430,
                        parentId: 1428,
                        name: '步骤2：（可选）创建限速策略',
                        local: 'vmware_gud_0025_0.html'
                      },
                      {
                        id: 1431,
                        parentId: 1428,
                        name: '步骤3：创建备份SLA',
                        local: 'vmware_gud_0027_0.html'
                      },
                      {
                        id: 1432,
                        parentId: 1428,
                        name: '步骤4：执行备份',
                        local: 'vmware_gud_0028_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1420,
                parentId: 1258,
                name: '复制',
                local: 'vmware_gud_0031_0.html',
                children: [
                  {
                    id: 1433,
                    parentId: 1420,
                    name: '复制Hyper-V虚拟机副本',
                    local: 'vmware_gud_0034_0.html',
                    children: [
                      {
                        id: 1434,
                        parentId: 1433,
                        name: '创建复制网络逻辑端口',
                        local: 'fc_gud_0026_1_20.html'
                      },
                      {
                        id: 1435,
                        parentId: 1433,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'vmware_gud_0036_1_0.html'
                      },
                      {
                        id: 1436,
                        parentId: 1433,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'vmware_gud_0038_0.html'
                      },
                      {
                        id: 1437,
                        parentId: 1433,
                        name: '步骤4：下载并导入证书',
                        local: 'vmware_gud_0039_0.html'
                      },
                      {
                        id: 1438,
                        parentId: 1433,
                        name: '步骤5：创建远端设备管理员',
                        local: 'vmware_gud_0040_1.html'
                      },
                      {
                        id: 1439,
                        parentId: 1433,
                        name: '步骤6：添加复制集群',
                        local: 'vmware_gud_0041_0.html'
                      },
                      {
                        id: 1440,
                        parentId: 1433,
                        name: '步骤7：创建复制SLA',
                        local: 'vmware_gud_0042.html'
                      },
                      {
                        id: 1441,
                        parentId: 1433,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'vmware_gud_0043_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1421,
                parentId: 1258,
                name: '归档',
                local: 'vmware_gud_0044_0.html',
                children: [
                  {
                    id: 1442,
                    parentId: 1421,
                    name: '归档Hyper-V备份副本',
                    local: 'vmware_gud_0047_0.html',
                    children: [
                      {
                        id: 1444,
                        parentId: 1442,
                        name: '步骤1：添加归档存储',
                        local: 'vmware_gud_0048_0.html',
                        children: [
                          {
                            id: 1446,
                            parentId: 1444,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'vmware_gud_0049_0.html'
                          },
                          {
                            id: 1447,
                            parentId: 1444,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'vmware_gud_0050_0.html'
                          }
                        ]
                      },
                      {
                        id: 1445,
                        parentId: 1442,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'vmware_gud_0051_0.html'
                      }
                    ]
                  },
                  {
                    id: 1443,
                    parentId: 1421,
                    name: '归档Hyper-V复制副本',
                    local: 'vmware_gud_0052_0.html',
                    children: [
                      {
                        id: 1448,
                        parentId: 1443,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'vmware_gud_0053_0.html'
                      },
                      {
                        id: 1449,
                        parentId: 1443,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'vmware_gud_0054_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1422,
                parentId: 1258,
                name: '恢复',
                local: 'vmware_gud_0055_0.html',
                children: [
                  {
                    id: 1450,
                    parentId: 1422,
                    name: '恢复Hyper-V虚拟机',
                    local: 'vmware_gud_0058_0.html'
                  },
                  {
                    id: 1451,
                    parentId: 1422,
                    name: '恢复Hyper-V虚拟机磁盘',
                    local: 'vmware_gud_0059_0.html'
                  }
                ]
              },
              {
                id: 1423,
                parentId: 1258,
                name: '全局搜索',
                local: 'vmware_gud_0074_0.html',
                children: [
                  {
                    id: 1452,
                    parentId: 1423,
                    name: '全局搜索副本数据',
                    local: 'vmware_gud_0076_1.html'
                  },
                  {
                    id: 1453,
                    parentId: 1423,
                    name: '全局搜索资源',
                    local: 'vmware_gud_0077_0.html'
                  },
                  {
                    id: 1454,
                    parentId: 1423,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002096124161.html'
                  }
                ]
              },
              {
                id: 1424,
                parentId: 1258,
                name: 'SLA',
                local: 'vmware_gud_0080_0.html',
                children: [
                  {
                    id: 1455,
                    parentId: 1424,
                    name: '查看SLA信息',
                    local: 'vmware_gud_0082_0.html'
                  },
                  {
                    id: 1456,
                    parentId: 1424,
                    name: '管理SLA',
                    local: 'vmware_gud_0083_0.html'
                  }
                ]
              },
              {
                id: 1425,
                parentId: 1258,
                name: '副本',
                local: 'vmware_gud_0084_0.html',
                children: [
                  {
                    id: 1457,
                    parentId: 1425,
                    name: '查看Hyper-V副本信息',
                    local: 'vmware_gud_0085_0.html'
                  },
                  {
                    id: 1458,
                    parentId: 1425,
                    name: '管理Hyper-V副本',
                    local: 'vmware_gud_0086_0.html'
                  }
                ]
              },
              {
                id: 1426,
                parentId: 1258,
                name: 'Hyper-V虚拟化环境',
                local: 'vmware_gud_0087_0.html',
                children: [
                  {
                    id: 1459,
                    parentId: 1426,
                    name: '查看Hyper-V虚拟化环境信息',
                    local: 'vmware_gud_0088_0.html'
                  },
                  {
                    id: 1460,
                    parentId: 1426,
                    name: '管理Hyper-V注册信息',
                    local: 'vmware_gud_0089_0.html'
                  },
                  {
                    id: 1461,
                    parentId: 1426,
                    name: '管理集群/主机/虚拟机/虚拟机组',
                    local: 'vmware_gud_0090_0.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1259,
            parentId: 15,
            name: 'FusionOne Compute数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002085703925.html',
            children: [
              {
                id: 1462,
                parentId: 1259,
                name: '备份',
                local: 'foc_gud_0009.html',
                children: [
                  {
                    id: 1470,
                    parentId: 1462,
                    name: '备份前准备',
                    local: 'foc_gud_0012.html'
                  },
                  {
                    id: 1471,
                    parentId: 1462,
                    name: '备份FusionOne Compute虚拟机',
                    local: 'foc_gud_0013.html',
                    children: [
                      {
                        id: 1472,
                        parentId: 1471,
                        name: '步骤1：创建FusionOne Compute对接用户',
                        local: 'foc_gud_0014.html'
                      },
                      {
                        id: 1473,
                        parentId: 1471,
                        name: '步骤2：注册FusionOne Compute虚拟化环境',
                        local: 'foc_gud_0015.html'
                      },
                      {
                        id: 1474,
                        parentId: 1471,
                        name: '步骤3：（可选）创建FusionOne Compute虚拟机组',
                        local: 'foc_gud_0016.html'
                      },
                      {
                        id: 1475,
                        parentId: 1471,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'foc_gud_0017.html'
                      },
                      {
                        id: 1476,
                        parentId: 1471,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'foc_gud_0018.html'
                      },
                      {
                        id: 1477,
                        parentId: 1471,
                        name: '步骤6：创建备份SLA',
                        local: 'foc_gud_0019.html'
                      },
                      {
                        id: 1478,
                        parentId: 1471,
                        name: '步骤7：执行备份',
                        local: 'foc_gud_0020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1463,
                parentId: 1259,
                name: '复制',
                local: 'foc_gud_0021.html',
                children: [
                  {
                    id: 1479,
                    parentId: 1463,
                    name: '复制FusionOne Compute虚拟机副本',
                    local: 'foc_gud_0024.html',
                    children: [
                      {
                        id: 1480,
                        parentId: 1479,
                        name: '步骤1：创建复制网络逻辑端口',
                        local: 'foc_gud_0026.html'
                      },
                      {
                        id: 1481,
                        parentId: 1479,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'foc_gud_0027.html'
                      },
                      {
                        id: 1482,
                        parentId: 1479,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'foc_gud_0028.html'
                      },
                      {
                        id: 1483,
                        parentId: 1479,
                        name: '步骤4：下载并导入证书',
                        local: 'foc_gud_0029.html'
                      },
                      {
                        id: 1484,
                        parentId: 1479,
                        name: '步骤5：创建远端设备管理员',
                        local: 'foc_gud_0031.html'
                      },
                      {
                        id: 1485,
                        parentId: 1479,
                        name: '步骤6：添加复制集群',
                        local: 'foc_gud_0032.html'
                      },
                      {
                        id: 1486,
                        parentId: 1479,
                        name: '步骤7：创建复制SLA',
                        local: 'foc_gud_0033.html'
                      },
                      {
                        id: 1487,
                        parentId: 1479,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'foc_gud_0034.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1464,
                parentId: 1259,
                name: '归档',
                local: 'foc_gud_0035.html',
                children: [
                  {
                    id: 1488,
                    parentId: 1464,
                    name: '归档FusionOne Compute备份副本',
                    local: 'foc_gud_0038.html',
                    children: [
                      {
                        id: 1490,
                        parentId: 1488,
                        name: '步骤1：添加归档存储',
                        local: 'foc_gud_0039.html',
                        children: [
                          {
                            id: 1492,
                            parentId: 1490,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'foc_gud_0040.html'
                          },
                          {
                            id: 1493,
                            parentId: 1490,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'foc_gud_0041.html'
                          }
                        ]
                      },
                      {
                        id: 1491,
                        parentId: 1488,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'foc_gud_0042.html'
                      }
                    ]
                  },
                  {
                    id: 1489,
                    parentId: 1464,
                    name: '归档FusionOne Compute复制副本',
                    local: 'foc_gud_0043.html',
                    children: [
                      {
                        id: 1494,
                        parentId: 1489,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'foc_gud_0044.html'
                      },
                      {
                        id: 1495,
                        parentId: 1489,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'foc_gud_0045.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1465,
                parentId: 1259,
                name: '恢复',
                local: 'foc_gud_0046.html',
                children: [
                  {
                    id: 1496,
                    parentId: 1465,
                    name: '恢复FusionOne Compute虚拟机',
                    local: 'foc_gud_0049.html'
                  },
                  {
                    id: 1497,
                    parentId: 1465,
                    name: '恢复FusionOne Compute虚拟机磁盘',
                    local: 'foc_gud_0050.html'
                  }
                ]
              },
              {
                id: 1466,
                parentId: 1259,
                name: '全局搜索',
                local: 'foc_gud_0052.html',
                children: [
                  {
                    id: 1498,
                    parentId: 1466,
                    name: '关于全局搜索',
                    local: 'foc_gud_0053.html'
                  },
                  {
                    id: 1499,
                    parentId: 1466,
                    name: '全局搜索副本数据',
                    local: 'foc_gud_0054.html'
                  },
                  {
                    id: 1500,
                    parentId: 1466,
                    name: '全局搜索资源',
                    local: 'foc_gud_0055.html'
                  },
                  {
                    id: 1501,
                    parentId: 1466,
                    name: '全局标签搜索',
                    local: 'foc_gud_0056.html'
                  }
                ]
              },
              {
                id: 1467,
                parentId: 1259,
                name: 'SLA',
                local: 'foc_gud_0059.html',
                children: [
                  {
                    id: 1502,
                    parentId: 1467,
                    name: '查看SLA信息',
                    local: 'foc_gud_0061.html'
                  },
                  {
                    id: 1503,
                    parentId: 1467,
                    name: '管理SLA',
                    local: 'foc_gud_0062.html'
                  }
                ]
              },
              {
                id: 1468,
                parentId: 1259,
                name: '副本',
                local: 'foc_gud_0063.html',
                children: [
                  {
                    id: 1504,
                    parentId: 1468,
                    name: '查看FusionOne Compute副本信息',
                    local: 'foc_gud_0064.html'
                  },
                  {
                    id: 1505,
                    parentId: 1468,
                    name: '管理FusionOne Compute副本',
                    local: 'foc_gud_0065.html'
                  }
                ]
              },
              {
                id: 1469,
                parentId: 1259,
                name: 'FusionOne Compute虚拟化环境',
                local: 'foc_gud_0066.html',
                children: [
                  {
                    id: 1506,
                    parentId: 1469,
                    name: '查看FusionOne Compute虚拟化环境信息',
                    local: 'foc_gud_0067.html'
                  },
                  {
                    id: 1507,
                    parentId: 1469,
                    name: '管理FusionOne Compute注册信息',
                    local: 'foc_gud_0068.html'
                  },
                  {
                    id: 1508,
                    parentId: 1469,
                    name: '管理集群/主机/虚拟机/虚拟机组',
                    local: 'foc_gud_0069.html'
                  }
                ]
              }
            ]
          }
        ]
      },
      {
        id: 16,
        parentId: 3,
        name: '容器',
        local: 'zh-cn_topic_0000001918630668.html',
        children: [
          {
            id: 1509,
            parentId: 16,
            name: 'Kubernetes CSI数据保护',
            local: 'zh-cn_topic_0000001873759377.html',
            children: [
              {
                id: 1511,
                parentId: 1509,
                name: '备份',
                local: 'kubernetes_CSI_00006.html',
                children: [
                  {
                    id: 1520,
                    parentId: 1511,
                    name: '备份前准备（适用于FusionCompute）',
                    local: 'kubernetes_CSI_000091.html',
                    children: [
                      {
                        id: 1525,
                        parentId: 1520,
                        name: '上传Kubernetes安装包至镜像库',
                        local: 'kubernetes_CSI_00069.html'
                      },
                      {
                        id: 1526,
                        parentId: 1520,
                        name: '获取kubeconfig配置文件',
                        local: 'kubernetes_CSI_00065.html'
                      }
                    ]
                  },
                  {
                    id: 1521,
                    parentId: 1511,
                    name: '备份前准备（适用于CCE）',
                    local: 'kubernetes_CSI_000092.html',
                    children: [
                      {
                        id: 1527,
                        parentId: 1521,
                        name: '上传和更新Kubernetes镜像压缩包',
                        local: 'kubernetes_CSI_00102.html'
                      },
                      {
                        id: 1528,
                        parentId: 1521,
                        name: '获取kubeconfig配置文件',
                        local: 'kubernetes_CSI_00078.html'
                      }
                    ]
                  },
                  {
                    id: 1522,
                    parentId: 1511,
                    name: '备份前准备（适用于OpenShift）',
                    local: 'kubernetes_CSI_00078_2.html',
                    children: [
                      {
                        id: 1529,
                        parentId: 1522,
                        name: '上传Kubernetes安装包并获取镜像名和Tag信息',
                        local: 'kubernetes_CSI_00078_3.html'
                      },
                      {
                        id: 1530,
                        parentId: 1522,
                        name: '获取kubeconfig配置文件',
                        local: 'kubernetes_CSI_00078_4.html'
                      },
                      {
                        id: 1531,
                        parentId: 1522,
                        name: '获取Token信息',
                        local: 'kubernetes_CSI_00078_5.html'
                      }
                    ]
                  },
                  {
                    id: 1523,
                    parentId: 1511,
                    name: '备份前准备（适用于原生Kubernetes）',
                    local: 'kubernetes_CSI_00078_6.html',
                    children: [
                      {
                        id: 1532,
                        parentId: 1523,
                        name: '上传Kubernetes安装包至Kubernetes集群',
                        local: 'kubernetes_CSI_00078_7.html'
                      },
                      {
                        id: 1533,
                        parentId: 1523,
                        name: '获取kubeconfig配置文件',
                        local: 'kubernetes_CSI_00078_8.html'
                      }
                    ]
                  },
                  {
                    id: 1524,
                    parentId: 1511,
                    name: '备份命名空间/数据集',
                    local: 'kubernetes_CSI_00010.html',
                    children: [
                      {
                        id: 1534,
                        parentId: 1524,
                        name: '步骤1：（可选）查询Kubernetes集群的节点标签',
                        local: 'kubernetes_CSI_00010_1.html'
                      },
                      {
                        id: 1535,
                        parentId: 1524,
                        name: '步骤2：（可选）生成最小权限Token',
                        local: 'kubernetes_CSI_00077.html'
                      },
                      {
                        id: 1536,
                        parentId: 1524,
                        name: '步骤3：注册集群',
                        local: 'kubernetes_CSI_00011.html'
                      },
                      {
                        id: 1537,
                        parentId: 1524,
                        name: '步骤4：注册数据集',
                        local: 'kubernetes_CSI_00012.html'
                      },
                      {
                        id: 1538,
                        parentId: 1524,
                        name: '步骤5：授权资源',
                        local: 'kubernetes_CSI_00013.html'
                      },
                      {
                        id: 1539,
                        parentId: 1524,
                        name: '步骤6：创建限速策略',
                        local: 'kubernetes_CSI_00014.html'
                      },
                      {
                        id: 1540,
                        parentId: 1524,
                        name: '步骤7：创建备份SLA',
                        local: 'kubernetes_CSI_00015.html'
                      },
                      {
                        id: 1541,
                        parentId: 1524,
                        name: '步骤8：执行备份',
                        local: 'kubernetes_CSI_00016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1512,
                parentId: 1509,
                name: '复制',
                local: 'kubernetes_CSI_00019.html',
                children: [
                  {
                    id: 1542,
                    parentId: 1512,
                    name: '复制Kubernetes CSI副本',
                    local: 'kubernetes_CSI_00022.html',
                    children: [
                      {
                        id: 1543,
                        parentId: 1542,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'kubernetes_CSI_00024.html'
                      },
                      {
                        id: 1544,
                        parentId: 1542,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'kubernetes_CSI_00024_1.html'
                      },
                      {
                        id: 1545,
                        parentId: 1542,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'kubernetes_CSI_00025.html'
                      },
                      {
                        id: 1546,
                        parentId: 1542,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'kubernetes_CSI_00026.html'
                      },
                      {
                        id: 1547,
                        parentId: 1542,
                        name: '步骤4：下载并导入证书',
                        local: 'kubernetes_CSI_00027.html'
                      },
                      {
                        id: 1548,
                        parentId: 1542,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'kubernetes_CSI_00028.html'
                      },
                      {
                        id: 1549,
                        parentId: 1542,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'kubernetes_CSI_00028_a1.html'
                      },
                      {
                        id: 1550,
                        parentId: 1542,
                        name: '步骤6：添加目标集群',
                        local: 'kubernetes_CSI_00029.html'
                      },
                      {
                        id: 1551,
                        parentId: 1542,
                        name: '步骤7：创建复制SLA',
                        local: 'kubernetes_CSI_00030.html'
                      },
                      {
                        id: 1552,
                        parentId: 1542,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'kubernetes_CSI_00031.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1513,
                parentId: 1509,
                name: '归档',
                local: 'kubernetes_CSI_00032.html',
                children: [
                  {
                    id: 1553,
                    parentId: 1513,
                    name: '归档Kubernetes CSI备份副本',
                    local: 'kubernetes_CSI_00035.html',
                    children: [
                      {
                        id: 1555,
                        parentId: 1553,
                        name: '步骤1：添加归档存储',
                        local: 'kubernetes_CSI_00036.html',
                        children: [
                          {
                            id: 1557,
                            parentId: 1555,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'kubernetes_CSI_00037.html'
                          },
                          {
                            id: 1558,
                            parentId: 1555,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'kubernetes_CSI_00038.html'
                          }
                        ]
                      },
                      {
                        id: 1556,
                        parentId: 1553,
                        name: '步骤2：创建归档SLA',
                        local: 'kubernetes_CSI_00039.html'
                      }
                    ]
                  },
                  {
                    id: 1554,
                    parentId: 1513,
                    name: '归档Kubernetes CSI复制副本',
                    local: 'kubernetes_CSI_00040.html',
                    children: [
                      {
                        id: 1559,
                        parentId: 1554,
                        name: '步骤1：创建归档SLA',
                        local: 'kubernetes_CSI_00041.html'
                      },
                      {
                        id: 1560,
                        parentId: 1554,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'kubernetes_CSI_00042.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1514,
                parentId: 1509,
                name: '恢复',
                local: 'kubernetes_CSI_00043.html',
                children: [
                  {
                    id: 1561,
                    parentId: 1514,
                    name: '恢复命名空间/数据集',
                    local: 'kubernetes_CSI_00046.html'
                  },
                  {
                    id: 1562,
                    parentId: 1514,
                    name: '恢复PVC',
                    local: 'kubernetes_CSI_00047.html'
                  }
                ]
              },
              {
                id: 1515,
                parentId: 1509,
                name: '全局搜索',
                local: 'kubernetes_CSI_00043_a1.html',
                children: [
                  {
                    id: 1563,
                    parentId: 1515,
                    name: '全局搜索资源',
                    local: 'kubernetes_CSI_00043_a2.html'
                  },
                  {
                    id: 1564,
                    parentId: 1515,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'kubernetes_CSI_00043_a3.html'
                  }
                ]
              },
              {
                id: 1516,
                parentId: 1509,
                name: 'SLA',
                local: 'kubernetes_CSI_00051.html',
                children: [
                  {
                    id: 1565,
                    parentId: 1516,
                    name: '关于SLA',
                    local: 'kubernetes_CSI_00052.html'
                  },
                  {
                    id: 1566,
                    parentId: 1516,
                    name: '查看SLA信息',
                    local: 'kubernetes_CSI_00053.html'
                  },
                  {
                    id: 1567,
                    parentId: 1516,
                    name: '管理SLA',
                    local: 'kubernetes_CSI_00054.html'
                  }
                ]
              },
              {
                id: 1517,
                parentId: 1509,
                name: '副本',
                local: 'kubernetes_CSI_00055.html',
                children: [
                  {
                    id: 1568,
                    parentId: 1517,
                    name: '查看Kubernetes CSI副本信息',
                    local: 'kubernetes_CSI_00056.html'
                  },
                  {
                    id: 1569,
                    parentId: 1517,
                    name: '管理Kubernetes CSI副本',
                    local: 'kubernetes_CSI_00057.html'
                  }
                ]
              },
              {
                id: 1518,
                parentId: 1509,
                name: '集群/命名空间/数据集',
                local: 'kubernetes_CSI_00058.html',
                children: [
                  {
                    id: 1570,
                    parentId: 1518,
                    name: '查看信息',
                    local: 'kubernetes_CSI_00059.html'
                  },
                  {
                    id: 1571,
                    parentId: 1518,
                    name: '管理集群',
                    local: 'kubernetes_CSI_00060.html'
                  },
                  {
                    id: 1572,
                    parentId: 1518,
                    name: '管理命名空间/数据集',
                    local: 'kubernetes_CSI_00061.html'
                  }
                ]
              },
              {
                id: 1519,
                parentId: 1509,
                name: '常见问题',
                local: 'kubernetes_CSI_00062.html',
                children: [
                  {
                    id: 1573,
                    parentId: 1519,
                    name: 'Token认证时获取证书值（适用于CCE）',
                    local: 'kubernetes_CSI_00079.html'
                  },
                  {
                    id: 1574,
                    parentId: 1519,
                    name: '应用一致性备份的生产环境Pod配置（通用）',
                    local: 'kubernetes_CSI_00066.html'
                  },
                  {
                    id: 1575,
                    parentId: 1519,
                    name: '应用一致性备份的生产环境Pod配置（容器应用为MySQL）',
                    local: 'kubernetes_CSI_00067.html'
                  },
                  {
                    id: 1576,
                    parentId: 1519,
                    name:
                      '应用一致性备份的生产环境Pod配置（容器应用为openGauss）',
                    local: 'kubernetes_CSI_00068.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1510,
            parentId: 16,
            name: 'Kubernetes FlexVolume数据保护',
            local: 'zh-cn_topic_0000001827039668.html',
            children: [
              {
                id: 1577,
                parentId: 1510,
                name: '备份',
                local: 'kubernetes_gud_00007.html',
                children: [
                  {
                    id: 1585,
                    parentId: 1577,
                    name: '备份前准备',
                    local: 'kubernetes_gud_00010.html'
                  },
                  {
                    id: 1586,
                    parentId: 1577,
                    name: '备份命名空间/StatefulSet',
                    local: 'kubernetes_gud_00011.html',
                    children: [
                      {
                        id: 1587,
                        parentId: 1586,
                        name: '步骤1：注册集群',
                        local: 'kubernetes_gud_00012.html'
                      },
                      {
                        id: 1588,
                        parentId: 1586,
                        name: '步骤2：授权资源',
                        local: 'kubernetes_gud_00013.html'
                      },
                      {
                        id: 1589,
                        parentId: 1586,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'kubernetes_gud_00014.html'
                      },
                      {
                        id: 1590,
                        parentId: 1586,
                        name: '步骤4：创建限速策略',
                        local: 'kubernetes_gud_00015.html'
                      },
                      {
                        id: 1591,
                        parentId: 1586,
                        name: '步骤5：创建备份SLA',
                        local: 'kubernetes_gud_00016.html'
                      },
                      {
                        id: 1592,
                        parentId: 1586,
                        name: '步骤6：执行备份',
                        local: 'kubernetes_gud_00017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1578,
                parentId: 1510,
                name: '复制',
                local: 'kubernetes_gud_00020.html',
                children: [
                  {
                    id: 1593,
                    parentId: 1578,
                    name: '复制Kubernetes FlexVolume副本',
                    local: 'kubernetes_gud_00023.html',
                    children: [
                      {
                        id: 1594,
                        parentId: 1593,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'kubernetes_gud_00025.html'
                      },
                      {
                        id: 1595,
                        parentId: 1593,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_16.html'
                      },
                      {
                        id: 1596,
                        parentId: 1593,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'kubernetes_gud_00026.html'
                      },
                      {
                        id: 1597,
                        parentId: 1593,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'kubernetes_gud_00027.html'
                      },
                      {
                        id: 1598,
                        parentId: 1593,
                        name: '步骤4：下载并导入证书',
                        local: 'kubernetes_gud_00028.html'
                      },
                      {
                        id: 1599,
                        parentId: 1593,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'kubernetes_gud_00029.html'
                      },
                      {
                        id: 1600,
                        parentId: 1593,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'kubernetes_gud_00029_a1.html'
                      },
                      {
                        id: 1601,
                        parentId: 1593,
                        name: '步骤6：添加目标集群',
                        local: 'kubernetes_gud_00030.html'
                      },
                      {
                        id: 1602,
                        parentId: 1593,
                        name: '步骤7：创建复制SLA',
                        local: 'kubernetes_gud_00031.html'
                      },
                      {
                        id: 1603,
                        parentId: 1593,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'kubernetes_gud_00032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1579,
                parentId: 1510,
                name: '归档',
                local: 'kubernetes_gud_00033.html',
                children: [
                  {
                    id: 1604,
                    parentId: 1579,
                    name: '归档Kubernetes FlexVolume备份副本',
                    local: 'kubernetes_gud_00036.html',
                    children: [
                      {
                        id: 1606,
                        parentId: 1604,
                        name: '步骤1：添加归档存储',
                        local: 'kubernetes_gud_00037.html',
                        children: [
                          {
                            id: 1608,
                            parentId: 1606,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'kubernetes_gud_00038.html'
                          },
                          {
                            id: 1609,
                            parentId: 1606,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'kubernetes_gud_00039.html'
                          }
                        ]
                      },
                      {
                        id: 1607,
                        parentId: 1604,
                        name: '步骤2：创建归档SLA',
                        local: 'kubernetes_gud_00040.html'
                      }
                    ]
                  },
                  {
                    id: 1605,
                    parentId: 1579,
                    name: '归档Kubernetes FlexVolume复制副本',
                    local: 'kubernetes_gud_00041.html',
                    children: [
                      {
                        id: 1610,
                        parentId: 1605,
                        name: '步骤1：创建归档SLA',
                        local: 'kubernetes_gud_00042.html'
                      },
                      {
                        id: 1611,
                        parentId: 1605,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'kubernetes_gud_00043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1580,
                parentId: 1510,
                name: '恢复',
                local: 'kubernetes_gud_00044.html',
                children: [
                  {
                    id: 1612,
                    parentId: 1580,
                    name: '恢复StatefulSet',
                    local: 'kubernetes_gud_00047.html'
                  }
                ]
              },
              {
                id: 1581,
                parentId: 1510,
                name: '全局搜索',
                local: 'kubernetes_gud_00044_a1.html',
                children: [
                  {
                    id: 1613,
                    parentId: 1581,
                    name: '全局搜索资源',
                    local: 'kubernetes_gud_00044_a2.html'
                  },
                  {
                    id: 1614,
                    parentId: 1581,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'kubernetes_gud_00044_a3.html'
                  }
                ]
              },
              {
                id: 1582,
                parentId: 1510,
                name: 'SLA',
                local: 'kubernetes_gud_00051.html',
                children: [
                  {
                    id: 1615,
                    parentId: 1582,
                    name: '关于SLA',
                    local: 'kubernetes_gud_00052.html'
                  },
                  {
                    id: 1616,
                    parentId: 1582,
                    name: '查看SLA信息',
                    local: 'kubernetes_gud_00053.html'
                  },
                  {
                    id: 1617,
                    parentId: 1582,
                    name: '管理SLA',
                    local: 'kubernetes_gud_00054.html'
                  }
                ]
              },
              {
                id: 1583,
                parentId: 1510,
                name: '副本',
                local: 'kubernetes_gud_00055.html',
                children: [
                  {
                    id: 1618,
                    parentId: 1583,
                    name: '查看Kubernetes FlexVolume副本信息',
                    local: 'kubernetes_gud_00056.html'
                  },
                  {
                    id: 1619,
                    parentId: 1583,
                    name: '管理Kubernetes FlexVolume副本',
                    local: 'kubernetes_gud_00057.html'
                  }
                ]
              },
              {
                id: 1584,
                parentId: 1510,
                name: '集群/命名空间/StatefulSet',
                local: 'kubernetes_gud_00058.html',
                children: [
                  {
                    id: 1620,
                    parentId: 1584,
                    name: '查看信息',
                    local: 'kubernetes_gud_00059.html'
                  },
                  {
                    id: 1621,
                    parentId: 1584,
                    name: '管理集群',
                    local: 'kubernetes_gud_00060.html'
                  },
                  {
                    id: 1622,
                    parentId: 1584,
                    name: '管理命名空间/StatefulSet',
                    local: 'kubernetes_gud_00061.html'
                  }
                ]
              }
            ]
          }
        ]
      },
      {
        id: 17,
        parentId: 3,
        name: '云平台',
        local: 'zh-cn_topic_0000001948269725.html',
        children: [
          {
            id: 1623,
            parentId: 17,
            name: '华为云Stack数据保护',
            local: 'zh-cn_topic_0000001827039672.html',
            children: [
              {
                id: 1627,
                parentId: 1623,
                name: '备份',
                local: 'hcs_gud_0007.html',
                children: [
                  {
                    id: 1635,
                    parentId: 1627,
                    name: '备份云服务器/云硬盘',
                    local: 'hcs_gud_0011.html',
                    children: [
                      {
                        id: 1636,
                        parentId: 1635,
                        name: '步骤1：获取证书',
                        local: 'hcs_gud_0012.html'
                      },
                      {
                        id: 1637,
                        parentId: 1635,
                        name: '步骤2：注册华为云Stack',
                        local: 'hcs_gud_0014.html'
                      },
                      {
                        id: 1638,
                        parentId: 1635,
                        name: '步骤3：添加租户并授权资源',
                        local: 'hcs_gud_0015.html'
                      },
                      {
                        id: 1639,
                        parentId: 1635,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'hcs_gud_0016.html'
                      },
                      {
                        id: 1640,
                        parentId: 1635,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'hcs_gud_0017.html'
                      },
                      {
                        id: 1641,
                        parentId: 1635,
                        name: '步骤6：创建备份SLA',
                        local: 'hcs_gud_0018.html'
                      },
                      {
                        id: 1642,
                        parentId: 1635,
                        name: '步骤7：执行备份',
                        local: 'hcs_gud_0019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1628,
                parentId: 1623,
                name: '复制',
                local: 'hcs_gud_0022.html',
                children: [
                  {
                    id: 1643,
                    parentId: 1628,
                    name: '复制华为云Stack副本',
                    local: 'hcs_gud_0025.html',
                    children: [
                      {
                        id: 1644,
                        parentId: 1643,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'hcs_gud_0027.html'
                      },
                      {
                        id: 1645,
                        parentId: 1643,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'hcs_gud_0027_1.html'
                      },
                      {
                        id: 1646,
                        parentId: 1643,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'hcs_gud_0028.html'
                      },
                      {
                        id: 1647,
                        parentId: 1643,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'hcs_gud_0029.html'
                      },
                      {
                        id: 1648,
                        parentId: 1643,
                        name: '步骤4：下载并导入证书',
                        local: 'hcs_gud_0030.html'
                      },
                      {
                        id: 1649,
                        parentId: 1643,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'hcs_gud_0031.html'
                      },
                      {
                        id: 1650,
                        parentId: 1643,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002044303453.html'
                      },
                      {
                        id: 1651,
                        parentId: 1643,
                        name: '步骤6：添加复制集群',
                        local: 'hcs_gud_0032.html'
                      },
                      {
                        id: 1652,
                        parentId: 1643,
                        name: '步骤7：创建复制SLA',
                        local: 'hcs_gud_0033.html'
                      },
                      {
                        id: 1653,
                        parentId: 1643,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'hcs_gud_0034.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1629,
                parentId: 1623,
                name: '归档',
                local: 'hcs_gud_0035.html',
                children: [
                  {
                    id: 1654,
                    parentId: 1629,
                    name: '归档华为云Stack备份副本',
                    local: 'hcs_gud_0038.html',
                    children: [
                      {
                        id: 1656,
                        parentId: 1654,
                        name: '步骤1：添加归档存储',
                        local: 'hcs_gud_0039.html',
                        children: [
                          {
                            id: 1658,
                            parentId: 1656,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'hcs_gud_0040.html'
                          },
                          {
                            id: 1659,
                            parentId: 1656,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'hcs_gud_0041.html'
                          }
                        ]
                      },
                      {
                        id: 1657,
                        parentId: 1654,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'hcs_gud_0042.html'
                      }
                    ]
                  },
                  {
                    id: 1655,
                    parentId: 1629,
                    name: '归档华为云Stack复制副本',
                    local: 'hcs_gud_0043.html',
                    children: [
                      {
                        id: 1660,
                        parentId: 1655,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'hcs_gud_0044.html'
                      },
                      {
                        id: 1661,
                        parentId: 1655,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'hcs_gud_0045.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1630,
                parentId: 1623,
                name: '恢复',
                local: 'hcs_gud_0046.html',
                children: [
                  {
                    id: 1662,
                    parentId: 1630,
                    name: '恢复云服务器/云硬盘',
                    local: 'hcs_gud_0049.html'
                  },
                  {
                    id: 1663,
                    parentId: 1630,
                    name: '恢复弹性云服务器中的文件',
                    local: 'hcs_gud_re1.html'
                  }
                ]
              },
              {
                id: 1631,
                parentId: 1623,
                name: '全局搜索',
                local: 'hcs_gud_gs1.html',
                children: [
                  {
                    id: 1664,
                    parentId: 1631,
                    name: '关于全局搜索',
                    local: 'zh-cn_topic_0000002002687814.html'
                  },
                  {
                    id: 1665,
                    parentId: 1631,
                    name: '全局搜索副本数据',
                    local: 'hcs_gud_gs2.html'
                  },
                  {
                    id: 1666,
                    parentId: 1631,
                    name: '全局搜索资源',
                    local: 'hcs_gud_gs3.html'
                  },
                  {
                    id: 1667,
                    parentId: 1631,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002038886953.html'
                  }
                ]
              },
              {
                id: 1632,
                parentId: 1623,
                name: 'SLA',
                local: 'hcs_gud_0053.html',
                children: [
                  {
                    id: 1668,
                    parentId: 1632,
                    name: '关于SLA',
                    local: 'hcs_gud_0054.html'
                  },
                  {
                    id: 1669,
                    parentId: 1632,
                    name: '查看SLA信息',
                    local: 'hcs_gud_0055.html'
                  },
                  {
                    id: 1670,
                    parentId: 1632,
                    name: '管理SLA',
                    local: 'hcs_gud_0056.html'
                  }
                ]
              },
              {
                id: 1633,
                parentId: 1623,
                name: '副本',
                local: 'hcs_gud_0057.html',
                children: [
                  {
                    id: 1671,
                    parentId: 1633,
                    name: '查看华为云Stack副本信息',
                    local: 'hcs_gud_0058.html'
                  },
                  {
                    id: 1672,
                    parentId: 1633,
                    name: '管理华为云Stack副本',
                    local: 'hcs_gud_0059.html'
                  }
                ]
              },
              {
                id: 1634,
                parentId: 1623,
                name: '华为云Stack环境',
                local: 'hcs_gud_0060.html',
                children: [
                  {
                    id: 1673,
                    parentId: 1634,
                    name: '查看华为云Stack信息',
                    local: 'hcs_gud_0061.html'
                  },
                  {
                    id: 1674,
                    parentId: 1634,
                    name: '管理华为云Stack注册信息',
                    local: 'hcs_gud_0062.html'
                  },
                  {
                    id: 1675,
                    parentId: 1634,
                    name: '管理租户',
                    local: 'hcs_gud_0063.html'
                  },
                  {
                    id: 1676,
                    parentId: 1634,
                    name: '管理项目/资源集或弹性云服务器',
                    local: 'hcs_gud_0064.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1624,
            parentId: 17,
            name: 'OpenStack数据保护',
            local: 'zh-cn_topic_0000001873679145.html',
            children: [
              {
                id: 1677,
                parentId: 1624,
                name: '备份',
                local: 'Open_Stack_00006_0.html',
                children: [
                  {
                    id: 1685,
                    parentId: 1677,
                    name: '备份前准备',
                    local: 'Open_Stack_00009_0.html'
                  },
                  {
                    id: 1686,
                    parentId: 1677,
                    name: '备份OpenStack云服务器',
                    local: 'Open_Stack_00010_0.html',
                    children: [
                      {
                        id: 1687,
                        parentId: 1686,
                        name: '步骤1：获取KeyStone V3地址',
                        local: 'Open_Stack_00010_1.html'
                      },
                      {
                        id: 1688,
                        parentId: 1686,
                        name: '步骤2：获取证书',
                        local: 'Open_Stack_00011_0.html'
                      },
                      {
                        id: 1689,
                        parentId: 1686,
                        name: '步骤3：创建对接用户',
                        local: 'Open_Stack_00013.html'
                      },
                      {
                        id: 1690,
                        parentId: 1686,
                        name: '步骤4：创建域管理员',
                        local: 'Open_Stack_00014.html'
                      },
                      {
                        id: 1691,
                        parentId: 1686,
                        name: '步骤5：注册OpenStack',
                        local: 'Open_Stack_00015_0.html'
                      },
                      {
                        id: 1692,
                        parentId: 1686,
                        name: '步骤6：添加域',
                        local: 'Open_Stack_00016.html'
                      },
                      {
                        id: 1693,
                        parentId: 1686,
                        name: '步骤7：（可选）创建云服务器组',
                        local: 'zh-cn_topic_0000001930077496.html'
                      },
                      {
                        id: 1694,
                        parentId: 1686,
                        name: '步骤8：创建限速策略',
                        local: 'Open_Stack_00018_0.html'
                      },
                      {
                        id: 1695,
                        parentId: 1686,
                        name: '步骤9：（可选）开启备份链路加密开关',
                        local: 'Open_Stack_00019_0.html'
                      },
                      {
                        id: 1696,
                        parentId: 1686,
                        name: '步骤10：（可选）修改Project的快照配额',
                        local: 'Open_Stack_00020.html'
                      },
                      {
                        id: 1697,
                        parentId: 1686,
                        name: '步骤11：创建备份SLA',
                        local: 'Open_Stack_00021_0.html'
                      },
                      {
                        id: 1698,
                        parentId: 1686,
                        name: '步骤12：执行备份',
                        local: 'Open_Stack_00022_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1678,
                parentId: 1624,
                name: '复制',
                local: 'Open_Stack_00025_0.html',
                children: [
                  {
                    id: 1699,
                    parentId: 1678,
                    name: '复制OpenStack副本',
                    local: 'Open_Stack_00028_0.html',
                    children: [
                      {
                        id: 1700,
                        parentId: 1699,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'Open_Stack_00030_0.html'
                      },
                      {
                        id: 1701,
                        parentId: 1699,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_33.html'
                      },
                      {
                        id: 1702,
                        parentId: 1699,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'Open_Stack_00031_0.html'
                      },
                      {
                        id: 1703,
                        parentId: 1699,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'Open_Stack_00032_0.html'
                      },
                      {
                        id: 1704,
                        parentId: 1699,
                        name: '步骤4：下载并导入证书',
                        local: 'Open_Stack_00033_0.html'
                      },
                      {
                        id: 1705,
                        parentId: 1699,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'Open_Stack_00034_0.html'
                      },
                      {
                        id: 1706,
                        parentId: 1699,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'Open_Stack_000340_0.html'
                      },
                      {
                        id: 1707,
                        parentId: 1699,
                        name: '步骤6：添加目标集群',
                        local: 'Open_Stack_00035_0.html'
                      },
                      {
                        id: 1708,
                        parentId: 1699,
                        name: '步骤7：创建复制SLA',
                        local: 'Open_Stack_00036.html'
                      },
                      {
                        id: 1709,
                        parentId: 1699,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'Open_Stack_00037_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1679,
                parentId: 1624,
                name: '归档',
                local: 'Open_Stack_00038_0.html',
                children: [
                  {
                    id: 1710,
                    parentId: 1679,
                    name: '归档OpenStack备份副本',
                    local: 'Open_Stack_00041_0.html',
                    children: [
                      {
                        id: 1712,
                        parentId: 1710,
                        name: '步骤1：添加归档存储',
                        local: 'Open_Stack_00042_0.html',
                        children: [
                          {
                            id: 1714,
                            parentId: 1712,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'Open_Stack_00043_0.html'
                          },
                          {
                            id: 1715,
                            parentId: 1712,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'Open_Stack_00044_0.html'
                          }
                        ]
                      },
                      {
                        id: 1713,
                        parentId: 1710,
                        name: '步骤2：创建归档SLA',
                        local: 'Open_Stack_00045_0.html'
                      }
                    ]
                  },
                  {
                    id: 1711,
                    parentId: 1679,
                    name: '归档OpenStack复制副本',
                    local: 'Open_Stack_00046_0.html',
                    children: [
                      {
                        id: 1716,
                        parentId: 1711,
                        name: '步骤1：创建归档SLA',
                        local: 'Open_Stack_00047_0.html'
                      },
                      {
                        id: 1717,
                        parentId: 1711,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'Open_Stack_00048_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1680,
                parentId: 1624,
                name: '恢复',
                local: 'Open_Stack_00049_0.html',
                children: [
                  {
                    id: 1718,
                    parentId: 1680,
                    name: '恢复云服务器',
                    local: 'Open_Stack_00052_0.html'
                  },
                  {
                    id: 1719,
                    parentId: 1680,
                    name: '恢复云磁盘',
                    local: 'Open_Stack_00053_0.html'
                  },
                  {
                    id: 1720,
                    parentId: 1680,
                    name: '恢复文件（适用于1.6.0及之后版本）',
                    local: 'Open_Stack_000531.html'
                  }
                ]
              },
              {
                id: 1681,
                parentId: 1624,
                name: '全局搜索',
                local: 'Open_Stack_000532.html',
                children: [
                  {
                    id: 1721,
                    parentId: 1681,
                    name: '全局搜索资源',
                    local: 'Open_Stack_00054_0.html'
                  },
                  {
                    id: 1722,
                    parentId: 1681,
                    name: '全局搜索副本数据',
                    local: 'Open_Stack_000541.html'
                  },
                  {
                    id: 1723,
                    parentId: 1681,
                    name: '全局搜索资源',
                    local: 'Open_Stack_000542.html'
                  },
                  {
                    id: 1724,
                    parentId: 1681,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'Open_Stack_000543.html'
                  }
                ]
              },
              {
                id: 1682,
                parentId: 1624,
                name: 'SLA',
                local: 'Open_Stack_00057_0.html',
                children: [
                  {
                    id: 1725,
                    parentId: 1682,
                    name: '关于SLA',
                    local: 'Open_Stack_00058_0.html'
                  },
                  {
                    id: 1726,
                    parentId: 1682,
                    name: '查看SLA信息',
                    local: 'Open_Stack_00059_0.html'
                  },
                  {
                    id: 1727,
                    parentId: 1682,
                    name: '管理SLA',
                    local: 'Open_Stack_00060_0.html'
                  }
                ]
              },
              {
                id: 1683,
                parentId: 1624,
                name: '副本',
                local: 'Open_Stack_00061_0.html',
                children: [
                  {
                    id: 1728,
                    parentId: 1683,
                    name: '查看OpenStack副本信息',
                    local: 'Open_Stack_00062_0.html'
                  },
                  {
                    id: 1729,
                    parentId: 1683,
                    name: '管理OpenStack副本',
                    local: 'Open_Stack_00063_0.html'
                  }
                ]
              },
              {
                id: 1684,
                parentId: 1624,
                name: 'OpenStack环境信息',
                local: 'Open_Stack_00064_0.html',
                children: [
                  {
                    id: 1730,
                    parentId: 1684,
                    name: '查看OpenStack信息',
                    local: 'Open_Stack_00065_0.html'
                  },
                  {
                    id: 1731,
                    parentId: 1684,
                    name: '管理OpenStack云平台',
                    local: 'Open_Stack_00066_0.html'
                  },
                  {
                    id: 1732,
                    parentId: 1684,
                    name: '管理域',
                    local: 'Open_Stack_00067.html'
                  },
                  {
                    id: 1733,
                    parentId: 1684,
                    name:
                      '管理项目/云服务器或云服务器组（适用于1.6.0及后续版本）',
                    local: 'Open_Stack_00068_0.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1625,
            parentId: 17,
            name: '华为云Stack GaussDB数据保护',
            local: 'zh-cn_topic_0000001826879800.html',
            children: [
              {
                id: 1734,
                parentId: 1625,
                name: '备份',
                local: 'hcs_gaussdb_00006.html',
                children: [
                  {
                    id: 1742,
                    parentId: 1734,
                    name: '备份前准备',
                    local: 'hcs_gaussdb_00009.html'
                  },
                  {
                    id: 1743,
                    parentId: 1734,
                    name: '备份华为云Stack GaussDB实例',
                    local: 'hcs_gaussdb_00010.html',
                    children: [
                      {
                        id: 1744,
                        parentId: 1743,
                        name: '步骤1：注册华为云Stack GaussDB项目',
                        local: 'hcs_gaussdb_00011.html'
                      },
                      {
                        id: 1745,
                        parentId: 1743,
                        name: '步骤2：（可选）开启备份链路加密开关',
                        local: 'hcs_gaussdb_00012.html'
                      },
                      {
                        id: 1746,
                        parentId: 1743,
                        name: '步骤3：创建限速策略',
                        local: 'hcs_gaussdb_00013.html'
                      },
                      {
                        id: 1747,
                        parentId: 1743,
                        name: '步骤4：创建备份SLA',
                        local: 'hcs_gaussdb_00014.html'
                      },
                      {
                        id: 1748,
                        parentId: 1743,
                        name: '步骤5：执行备份',
                        local: 'hcs_gaussdb_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1735,
                parentId: 1625,
                name: '复制',
                local: 'hcs_gaussdb_00018.html',
                children: [
                  {
                    id: 1749,
                    parentId: 1735,
                    name: '复制华为云Stack GaussDB副本',
                    local: 'hcs_gaussdb_00021.html',
                    children: [
                      {
                        id: 1750,
                        parentId: 1749,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'hcs_gaussdb_00023.html'
                      },
                      {
                        id: 1751,
                        parentId: 1749,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_4.html'
                      },
                      {
                        id: 1752,
                        parentId: 1749,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'hcs_gaussdb_00024.html'
                      },
                      {
                        id: 1753,
                        parentId: 1749,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'hcs_gaussdb_00025.html'
                      },
                      {
                        id: 1754,
                        parentId: 1749,
                        name: '步骤4：下载并导入证书',
                        local: 'hcs_gaussdb_00026.html'
                      },
                      {
                        id: 1755,
                        parentId: 1749,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'hcs_gaussdb_00027.html'
                      },
                      {
                        id: 1756,
                        parentId: 1749,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'hcs_gaussdb_0002700.html'
                      },
                      {
                        id: 1757,
                        parentId: 1749,
                        name: '步骤6：添加目标集群',
                        local: 'hcs_gaussdb_00028.html'
                      },
                      {
                        id: 1758,
                        parentId: 1749,
                        name: '步骤7：创建复制SLA',
                        local: 'hcs_gaussdb_00029.html'
                      },
                      {
                        id: 1759,
                        parentId: 1749,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'hcs_gaussdb_00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1736,
                parentId: 1625,
                name: '归档',
                local: 'hcs_gaussdb_00031.html',
                children: [
                  {
                    id: 1760,
                    parentId: 1736,
                    name: '归档华为云Stack GaussDB备份副本',
                    local: 'hcs_gaussdb_00034.html',
                    children: [
                      {
                        id: 1762,
                        parentId: 1760,
                        name: '步骤1：添加归档存储',
                        local: 'hcs_gaussdb_00035.html',
                        children: [
                          {
                            id: 1764,
                            parentId: 1762,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'hcs_gaussdb_00036.html'
                          },
                          {
                            id: 1765,
                            parentId: 1762,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'hcs_gaussdb_00037.html'
                          }
                        ]
                      },
                      {
                        id: 1763,
                        parentId: 1760,
                        name: '步骤2：创建归档SLA',
                        local: 'hcs_gaussdb_00038.html'
                      }
                    ]
                  },
                  {
                    id: 1761,
                    parentId: 1736,
                    name: '归档华为云Stack GaussDB复制副本',
                    local: 'hcs_gaussdb_00039.html',
                    children: [
                      {
                        id: 1766,
                        parentId: 1761,
                        name: '步骤1：创建归档SLA',
                        local: 'hcs_gaussdb_00040.html'
                      },
                      {
                        id: 1767,
                        parentId: 1761,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'hcs_gaussdb_00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1737,
                parentId: 1625,
                name: '恢复',
                local: 'hcs_gaussdb_00042.html',
                children: [
                  {
                    id: 1768,
                    parentId: 1737,
                    name: '恢复华为云Stack GaussDB实例',
                    local: 'hcs_gaussdb_00045.html'
                  }
                ]
              },
              {
                id: 1738,
                parentId: 1625,
                name: '全局搜索',
                local: 'hcs_gaussdb_0004211.html',
                children: [
                  {
                    id: 1769,
                    parentId: 1738,
                    name: '全局搜索资源',
                    local: 'hcs_gaussdb_00046.html'
                  },
                  {
                    id: 1770,
                    parentId: 1738,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'hcs_gaussdb_0004222.html'
                  }
                ]
              },
              {
                id: 1739,
                parentId: 1625,
                name: 'SLA',
                local: 'hcs_gaussdb_00049.html',
                children: [
                  {
                    id: 1771,
                    parentId: 1739,
                    name: '关于SLA',
                    local: 'hcs_gaussdb_000491.html'
                  },
                  {
                    id: 1772,
                    parentId: 1739,
                    name: '查看SLA信息',
                    local: 'hcs_gaussdb_00051.html'
                  },
                  {
                    id: 1773,
                    parentId: 1739,
                    name: '管理SLA',
                    local: 'hcs_gaussdb_00052.html'
                  }
                ]
              },
              {
                id: 1740,
                parentId: 1625,
                name: '副本',
                local: 'hcs_gaussdb_00053.html',
                children: [
                  {
                    id: 1774,
                    parentId: 1740,
                    name: '查看华为云Stack GaussDB副本信息',
                    local: 'hcs_gaussdb_00054.html'
                  },
                  {
                    id: 1775,
                    parentId: 1740,
                    name: '管理华为云Stack GaussDB副本',
                    local: 'hcs_gaussdb_00055.html'
                  }
                ]
              },
              {
                id: 1741,
                parentId: 1625,
                name: '华为云Stack GaussDB',
                local: 'hcs_gaussdb_00056.html',
                children: [
                  {
                    id: 1776,
                    parentId: 1741,
                    name: '查看华为云Stack GaussDB信息',
                    local: 'hcs_gaussdb_00057.html'
                  },
                  {
                    id: 1777,
                    parentId: 1741,
                    name: '管理华为云Stack GaussDB项目',
                    local: 'hcs_gaussdb_00058.html'
                  },
                  {
                    id: 1778,
                    parentId: 1741,
                    name: '管理实例',
                    local: 'hcs_gaussdb_00059.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1626,
            parentId: 17,
            name: '阿里云数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000001826879812.html',
            children: [
              {
                id: 1779,
                parentId: 1626,
                name: '备份',
                local: 'Open_Stack_00006.html',
                children: [
                  {
                    id: 1787,
                    parentId: 1779,
                    name: '备份前准备',
                    local: 'Open_Stack_00009.html'
                  },
                  {
                    id: 1788,
                    parentId: 1779,
                    name: '备份阿里云云服务器',
                    local: 'Open_Stack_00010.html',
                    children: [
                      {
                        id: 1789,
                        parentId: 1788,
                        name: '步骤1：获取云服务器ECS的Endpoint',
                        local: 'zh-cn_topic_0000001888336165.html'
                      },
                      {
                        id: 1790,
                        parentId: 1788,
                        name: '步骤2：获取组织的AK',
                        local: 'zh-cn_topic_0000001841651698.html'
                      },
                      {
                        id: 1791,
                        parentId: 1788,
                        name: '步骤3：获取Region ID',
                        local: 'zh-cn_topic_0000001887970893.html'
                      },
                      {
                        id: 1792,
                        parentId: 1788,
                        name: '步骤4：注册阿里云组织',
                        local: 'Open_Stack_00015.html'
                      },
                      {
                        id: 1793,
                        parentId: 1788,
                        name: '步骤5：创建限速策略',
                        local: 'Open_Stack_00018.html'
                      },
                      {
                        id: 1794,
                        parentId: 1788,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'Open_Stack_00019.html'
                      },
                      {
                        id: 1795,
                        parentId: 1788,
                        name: '步骤7：创建备份SLA',
                        local: 'Open_Stack_00021.html'
                      },
                      {
                        id: 1796,
                        parentId: 1788,
                        name: '步骤8：执行备份',
                        local: 'Open_Stack_00022.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1780,
                parentId: 1626,
                name: '复制',
                local: 'Open_Stack_00025.html',
                children: [
                  {
                    id: 1797,
                    parentId: 1780,
                    name: '复制阿里云副本',
                    local: 'Open_Stack_00028.html',
                    children: [
                      {
                        id: 1798,
                        parentId: 1797,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'Open_Stack_00030.html'
                      },
                      {
                        id: 1799,
                        parentId: 1797,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_12.html'
                      },
                      {
                        id: 1800,
                        parentId: 1797,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'Open_Stack_00031.html'
                      },
                      {
                        id: 1801,
                        parentId: 1797,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'Open_Stack_00032.html'
                      },
                      {
                        id: 1802,
                        parentId: 1797,
                        name: '步骤4：下载并导入证书',
                        local: 'Open_Stack_00033.html'
                      },
                      {
                        id: 1803,
                        parentId: 1797,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'Open_Stack_00034.html'
                      },
                      {
                        id: 1804,
                        parentId: 1797,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'Open_Stack_000340.html'
                      },
                      {
                        id: 1805,
                        parentId: 1797,
                        name: '步骤6：添加目标集群',
                        local: 'Open_Stack_00035.html'
                      },
                      {
                        id: 1806,
                        parentId: 1797,
                        name: '步骤7：创建复制SLA',
                        local: 'Ali_000361.html'
                      },
                      {
                        id: 1807,
                        parentId: 1797,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'Open_Stack_00037.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1781,
                parentId: 1626,
                name: '归档',
                local: 'Open_Stack_00038.html',
                children: [
                  {
                    id: 1808,
                    parentId: 1781,
                    name: '归档阿里云备份副本',
                    local: 'Open_Stack_00041.html',
                    children: [
                      {
                        id: 1810,
                        parentId: 1808,
                        name: '步骤1：添加归档存储',
                        local: 'Open_Stack_00042.html',
                        children: [
                          {
                            id: 1812,
                            parentId: 1810,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'Open_Stack_00043.html'
                          },
                          {
                            id: 1813,
                            parentId: 1810,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'Open_Stack_00044.html'
                          }
                        ]
                      },
                      {
                        id: 1811,
                        parentId: 1808,
                        name: '步骤2：创建归档SLA',
                        local: 'Open_Stack_00045.html'
                      }
                    ]
                  },
                  {
                    id: 1809,
                    parentId: 1781,
                    name: '归档阿里云复制副本',
                    local: 'Open_Stack_00046.html',
                    children: [
                      {
                        id: 1814,
                        parentId: 1809,
                        name: '步骤1：创建归档SLA',
                        local: 'Open_Stack_00047.html'
                      },
                      {
                        id: 1815,
                        parentId: 1809,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'Open_Stack_00048.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1782,
                parentId: 1626,
                name: '恢复',
                local: 'Open_Stack_00049.html',
                children: [
                  {
                    id: 1816,
                    parentId: 1782,
                    name: '恢复云服务器',
                    local: 'Open_Stack_00052.html'
                  },
                  {
                    id: 1817,
                    parentId: 1782,
                    name: '恢复云磁盘',
                    local: 'Open_Stack_00053.html'
                  },
                  {
                    id: 1818,
                    parentId: 1782,
                    name: '恢复文件',
                    local: 'zh-cn_topic_0000001846127238.html'
                  }
                ]
              },
              {
                id: 1783,
                parentId: 1626,
                name: '全局搜索',
                local: 'zh-cn_topic_0000002014804512.html',
                children: [
                  {
                    id: 1819,
                    parentId: 1783,
                    name: '关于全局搜索',
                    local: 'zh-cn_topic_0000002014646280.html'
                  },
                  {
                    id: 1820,
                    parentId: 1783,
                    name: '全局搜索副本数据',
                    local: 'vmware_gud_0076_0.html'
                  },
                  {
                    id: 1821,
                    parentId: 1783,
                    name: '全局搜索资源',
                    local: 'Open_Stack_00054.html'
                  },
                  {
                    id: 1822,
                    parentId: 1783,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002050884109.html'
                  }
                ]
              },
              {
                id: 1784,
                parentId: 1626,
                name: 'SLA',
                local: 'Open_Stack_00057.html',
                children: [
                  {
                    id: 1823,
                    parentId: 1784,
                    name: '关于SLA',
                    local: 'Open_Stack_00058.html'
                  },
                  {
                    id: 1824,
                    parentId: 1784,
                    name: '查看SLA信息',
                    local: 'Open_Stack_00059.html'
                  },
                  {
                    id: 1825,
                    parentId: 1784,
                    name: '管理SLA',
                    local: 'Open_Stack_00060.html'
                  }
                ]
              },
              {
                id: 1785,
                parentId: 1626,
                name: '副本',
                local: 'Open_Stack_00061.html',
                children: [
                  {
                    id: 1826,
                    parentId: 1785,
                    name: '查看阿里云副本信息',
                    local: 'Open_Stack_00062.html'
                  },
                  {
                    id: 1827,
                    parentId: 1785,
                    name: '管理阿里云副本',
                    local: 'Open_Stack_00063.html'
                  }
                ]
              },
              {
                id: 1786,
                parentId: 1626,
                name: '阿里云环境信息',
                local: 'Open_Stack_00064.html',
                children: [
                  {
                    id: 1828,
                    parentId: 1786,
                    name: '查看阿里云资源信息',
                    local: 'Open_Stack_00065.html'
                  },
                  {
                    id: 1829,
                    parentId: 1786,
                    name: '管理阿里云云平台',
                    local: 'Open_Stack_00066.html'
                  },
                  {
                    id: 1830,
                    parentId: 1786,
                    name: '管理可用区/资源集/云服务器',
                    local: 'Open_Stack_00068.html'
                  }
                ]
              }
            ]
          }
        ]
      },
      {
        id: 18,
        parentId: 3,
        name: '应用',
        local: 'zh-cn_topic_0000001918470740.html',
        children: [
          {
            id: 1831,
            parentId: 18,
            name: 'Exchange数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000001826879820.html',
            children: [
              {
                id: 1833,
                parentId: 1831,
                name: '备份',
                local: 'exchange_0012.html',
                children: [
                  {
                    id: 1840,
                    parentId: 1833,
                    name: '备份前准备',
                    local: 'exchange_0017.html',
                    children: [
                      {
                        id: 1843,
                        parentId: 1840,
                        name: '开启Exchange信息存储服务',
                        local: 'exchange_0018.html'
                      },
                      {
                        id: 1844,
                        parentId: 1840,
                        name: '检查Exchange Writer状态',
                        local: 'exchange_0019.html'
                      },
                      {
                        id: 1845,
                        parentId: 1840,
                        name: '检查Exchange数据库状态',
                        local: 'exchange_0020.html'
                      },
                      {
                        id: 1846,
                        parentId: 1840,
                        name: '配置数据库备份与恢复账户',
                        local: 'exchange_0021.html'
                      },
                      {
                        id: 1847,
                        parentId: 1840,
                        name: '配置邮箱备份与恢复账户',
                        local: 'exchange_0022.html'
                      }
                    ]
                  },
                  {
                    id: 1841,
                    parentId: 1833,
                    name: '备份Exchange单机/可用性组或数据库',
                    local: 'exchange_0024.html',
                    children: [
                      {
                        id: 1848,
                        parentId: 1841,
                        name: '步骤1：注册Exchange单机/可用性组',
                        local: 'exchange_0025.html'
                      },
                      {
                        id: 1849,
                        parentId: 1841,
                        name: '步骤2：创建限速策略',
                        local: 'exchange_0026.html'
                      },
                      {
                        id: 1850,
                        parentId: 1841,
                        name: '步骤3：创建备份SLA',
                        local: 'exchange_0027.html'
                      },
                      {
                        id: 1851,
                        parentId: 1841,
                        name: '步骤4：执行备份',
                        local: 'exchange_0028.html'
                      }
                    ]
                  },
                  {
                    id: 1842,
                    parentId: 1833,
                    name: '备份Exchange邮箱',
                    local: 'exchange_0031.html',
                    children: [
                      {
                        id: 1852,
                        parentId: 1842,
                        name: '步骤1：注册Exchange单机/可用性组',
                        local: 'exchange_0032.html'
                      },
                      {
                        id: 1853,
                        parentId: 1842,
                        name: '步骤2：创建限速策略',
                        local: 'exchange_0033.html'
                      },
                      {
                        id: 1854,
                        parentId: 1842,
                        name: '步骤3：创建备份SLA',
                        local: 'exchange_0034.html'
                      },
                      {
                        id: 1855,
                        parentId: 1842,
                        name: '步骤4：执行备份',
                        local: 'exchange_0035.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1834,
                parentId: 1831,
                name: '复制',
                local: 'exchange_0038.html',
                children: [
                  {
                    id: 1856,
                    parentId: 1834,
                    name: '复制Exchange数据库副本',
                    local: 'exchange_0040.html',
                    children: [
                      {
                        id: 1857,
                        parentId: 1856,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_3.html'
                      },
                      {
                        id: 1858,
                        parentId: 1856,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'exchange_00460.html'
                      },
                      {
                        id: 1859,
                        parentId: 1856,
                        name: '步骤：创建复制SLA',
                        local: 'exchange_0048.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1835,
                parentId: 1831,
                name: '恢复',
                local: 'exchange_0061.html',
                children: [
                  {
                    id: 1860,
                    parentId: 1835,
                    name: '恢复单机/可用性组',
                    local: 'exchange_0067.html'
                  },
                  {
                    id: 1861,
                    parentId: 1835,
                    name: '恢复Exchange数据库',
                    local: 'exchange_0068.html'
                  },
                  {
                    id: 1862,
                    parentId: 1835,
                    name: '恢复邮箱',
                    local: 'exchange_0069.html'
                  },
                  {
                    id: 1863,
                    parentId: 1835,
                    name: '邮件级恢复',
                    local: 'exchange_0070.html'
                  },
                  {
                    id: 1864,
                    parentId: 1835,
                    name:
                      '验证恢复后的用户邮箱数据（适用于Microsoft Exchange Server 2010）',
                    local: 'zh-cn_topic_0000001935299986.html'
                  },
                  {
                    id: 1865,
                    parentId: 1835,
                    name:
                      '验证恢复后的用户邮箱数据（适用于Microsoft Exchange Server 2013及后续版本）',
                    local: 'zh-cn_topic_0000001945406733.html'
                  }
                ]
              },
              {
                id: 1836,
                parentId: 1831,
                name: '全局搜索',
                local: 'exchange_00611.html',
                children: [
                  {
                    id: 1866,
                    parentId: 1836,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'exchange_00711.html'
                  }
                ]
              },
              {
                id: 1837,
                parentId: 1831,
                name: 'SLA',
                local: 'exchange_0074.html',
                children: [
                  {
                    id: 1867,
                    parentId: 1837,
                    name: '关于SLA',
                    local: 'exchange_0075.html'
                  },
                  {
                    id: 1868,
                    parentId: 1837,
                    name: '查看SLA信息',
                    local: 'exchange_0076.html'
                  },
                  {
                    id: 1869,
                    parentId: 1837,
                    name: '管理SLA',
                    local: 'exchange_0077.html'
                  }
                ]
              },
              {
                id: 1838,
                parentId: 1831,
                name: '副本',
                local: 'exchange_0078.html',
                children: [
                  {
                    id: 1870,
                    parentId: 1838,
                    name: '查看Exchange副本信息',
                    local: 'exchange_0079.html'
                  },
                  {
                    id: 1871,
                    parentId: 1838,
                    name: '管理Exchange副本',
                    local: 'exchange_0080.html'
                  }
                ]
              },
              {
                id: 1839,
                parentId: 1831,
                name: '管理Exchange',
                local: 'exchange_0081.html',
                children: [
                  {
                    id: 1872,
                    parentId: 1839,
                    name: '查看Exchange环境信息',
                    local: 'exchange_0082.html'
                  },
                  {
                    id: 1873,
                    parentId: 1839,
                    name: '管理Exchange单机或可用性组',
                    local: 'exchange_0083.html'
                  },
                  {
                    id: 1874,
                    parentId: 1839,
                    name: '管理数据库或邮箱',
                    local: 'exchange_0084.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1832,
            parentId: 18,
            name: 'Active Directory数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000001826879868.html',
            children: [
              {
                id: 1875,
                parentId: 1832,
                name: '备份',
                local: 'ActiveDirectory-00006.html',
                children: [
                  {
                    id: 1883,
                    parentId: 1875,
                    name: '备份Active Directory',
                    local: 'ActiveDirectory-00010.html',
                    children: [
                      {
                        id: 1884,
                        parentId: 1883,
                        name: '步骤1：开启Active Directory回收站',
                        local: 'zh-cn_topic_0000002020257438.html'
                      },
                      {
                        id: 1885,
                        parentId: 1883,
                        name: '步骤2：注册Active Directory域控制器',
                        local: 'ActiveDirectory-00013.html'
                      },
                      {
                        id: 1886,
                        parentId: 1883,
                        name: '步骤3：创建限速策略',
                        local: 'ActiveDirectory-00014.html'
                      },
                      {
                        id: 1887,
                        parentId: 1883,
                        name: '步骤4：创建备份SLA',
                        local: 'ActiveDirectory-00016.html'
                      },
                      {
                        id: 1888,
                        parentId: 1883,
                        name: '步骤5：执行备份',
                        local: 'ActiveDirectory-00017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1876,
                parentId: 1832,
                name: '复制',
                local: 'ActiveDirectory-00020.html',
                children: [
                  {
                    id: 1889,
                    parentId: 1876,
                    name: '复制Active Directory副本',
                    local: 'ActiveDirectory-00023.html',
                    children: [
                      {
                        id: 1890,
                        parentId: 1889,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'ActiveDirectory-00025.html'
                      },
                      {
                        id: 1891,
                        parentId: 1889,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_21.html'
                      },
                      {
                        id: 1892,
                        parentId: 1889,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'ActiveDirectory-00026.html'
                      },
                      {
                        id: 1893,
                        parentId: 1889,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'ActiveDirectory-00027.html'
                      },
                      {
                        id: 1894,
                        parentId: 1889,
                        name: '步骤4：下载并导入证书',
                        local: 'ActiveDirectory-00028.html'
                      },
                      {
                        id: 1895,
                        parentId: 1889,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'ActiveDirectory-00029.html'
                      },
                      {
                        id: 1896,
                        parentId: 1889,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'ActiveDirectory-000290.html'
                      },
                      {
                        id: 1897,
                        parentId: 1889,
                        name: '步骤6：添加目标集群',
                        local: 'ActiveDirectory-00030.html'
                      },
                      {
                        id: 1898,
                        parentId: 1889,
                        name: '步骤7：创建复制SLA',
                        local: 'ActiveDirectory-00031.html'
                      },
                      {
                        id: 1899,
                        parentId: 1889,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'ActiveDirectory-00032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1877,
                parentId: 1832,
                name: '归档',
                local: 'ActiveDirectory-00033.html',
                children: [
                  {
                    id: 1900,
                    parentId: 1877,
                    name: '归档Active Directory备份副本',
                    local: 'ActiveDirectory-00036.html',
                    children: [
                      {
                        id: 1902,
                        parentId: 1900,
                        name: '步骤1：添加归档存储',
                        local: 'ActiveDirectory-00037.html',
                        children: [
                          {
                            id: 1904,
                            parentId: 1902,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'ActiveDirectory-00038.html'
                          },
                          {
                            id: 1905,
                            parentId: 1902,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'ActiveDirectory-00039.html'
                          }
                        ]
                      },
                      {
                        id: 1903,
                        parentId: 1900,
                        name: '步骤2：创建归档SLA',
                        local: 'ActiveDirectory-00040.html'
                      }
                    ]
                  },
                  {
                    id: 1901,
                    parentId: 1877,
                    name: '归档Active Directory复制副本',
                    local: 'ActiveDirectory-00041.html',
                    children: [
                      {
                        id: 1906,
                        parentId: 1901,
                        name: '步骤1：创建归档SLA',
                        local: 'ActiveDirectory-00042.html'
                      },
                      {
                        id: 1907,
                        parentId: 1901,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'ActiveDirectory-00043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1878,
                parentId: 1832,
                name: '恢复',
                local: 'ActiveDirectory-00044.html',
                children: [
                  {
                    id: 1908,
                    parentId: 1878,
                    name: '单域控制器场景恢复Active Directory的系统状态',
                    local: 'ActiveDirectory-00047.html'
                  },
                  {
                    id: 1909,
                    parentId: 1878,
                    name: '单域控制器场景恢复Active Directory的对象',
                    local: 'zh-cn_topic_0000001844891849.html'
                  },
                  {
                    id: 1910,
                    parentId: 1878,
                    name: '主备域控制器场景恢复Active Directory的系统状态',
                    local: 'zh-cn_topic_0000002080463105.html'
                  },
                  {
                    id: 1911,
                    parentId: 1878,
                    name: '主备域控制器场景恢复Active Directory的对象',
                    local: 'zh-cn_topic_0000002044462372.html'
                  }
                ]
              },
              {
                id: 1879,
                parentId: 1832,
                name: '全局搜索',
                local: 'ActiveDirectory-00048.html',
                children: [
                  {
                    id: 1912,
                    parentId: 1879,
                    name: '全局搜索资源',
                    local: 'ActiveDirectory-00049.html'
                  },
                  {
                    id: 1913,
                    parentId: 1879,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'ActiveDirectory-000491.html'
                  }
                ]
              },
              {
                id: 1880,
                parentId: 1832,
                name: 'SLA',
                local: 'ActiveDirectory-00052.html',
                children: [
                  {
                    id: 1914,
                    parentId: 1880,
                    name: '关于SLA',
                    local: 'ActiveDirectory-000521.html'
                  },
                  {
                    id: 1915,
                    parentId: 1880,
                    name: '查看SLA信息',
                    local: 'ActiveDirectory-000522.html'
                  },
                  {
                    id: 1916,
                    parentId: 1880,
                    name: '管理SLA',
                    local: 'ActiveDirectory-00055.html'
                  }
                ]
              },
              {
                id: 1881,
                parentId: 1832,
                name: '副本',
                local: 'ActiveDirectory-00056.html',
                children: [
                  {
                    id: 1917,
                    parentId: 1881,
                    name: '查看Active Directory副本信息',
                    local: 'ActiveDirectory-00057.html'
                  },
                  {
                    id: 1918,
                    parentId: 1881,
                    name: '管理Active Directory副本',
                    local: 'ActiveDirectory-00058.html'
                  }
                ]
              },
              {
                id: 1882,
                parentId: 1832,
                name: 'Active Directory域控制器',
                local: 'ActiveDirectory-00059.html',
                children: [
                  {
                    id: 1919,
                    parentId: 1882,
                    name: '查看Active Directory域控制器信息',
                    local: 'ActiveDirectory-00060.html'
                  },
                  {
                    id: 1920,
                    parentId: 1882,
                    name: '管理Active Directory域控制器',
                    local: 'ActiveDirectory-00061.html'
                  }
                ]
              }
            ]
          }
        ]
      },
      {
        id: 19,
        parentId: 3,
        name: '文件系统',
        local: 'zh-cn_topic_0000001918630672.html',
        children: [
          {
            id: 1921,
            parentId: 19,
            name: 'NAS文件数据保护',
            local: 'product_documentation_000035.html',
            children: [
              {
                id: 1927,
                parentId: 1921,
                name: '备份',
                local: 'nas_s_0007.html',
                children: [
                  {
                    id: 1938,
                    parentId: 1927,
                    name: '备份前准备',
                    local: 'nas_s_0010.html'
                  },
                  {
                    id: 1939,
                    parentId: 1927,
                    name: '备份NAS文件系统',
                    local: 'nas_s_0011.html',
                    children: [
                      {
                        id: 1941,
                        parentId: 1939,
                        name: '步骤1：获取存储设备CA证书',
                        local: 'nas_s_0012.html'
                      },
                      {
                        id: 1942,
                        parentId: 1939,
                        name: '步骤2：添加存储设备',
                        local: 'nas_s_0013.html'
                      },
                      {
                        id: 1943,
                        parentId: 1939,
                        name: '步骤3：创建复制网络逻辑端口',
                        local: 'nas_s_0014.html'
                      },
                      {
                        id: 1944,
                        parentId: 1939,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'nas_s_0015.html'
                      },
                      {
                        id: 1945,
                        parentId: 1939,
                        name: '步骤5：创建备份SLA',
                        local: 'nas_s_0016.html'
                      },
                      {
                        id: 1946,
                        parentId: 1939,
                        name: '步骤6：执行备份',
                        local: 'nas_s_0017.html'
                      }
                    ]
                  },
                  {
                    id: 1940,
                    parentId: 1927,
                    name: '备份NAS共享',
                    local: 'nas_s_0020.html',
                    children: [
                      {
                        id: 1947,
                        parentId: 1940,
                        name:
                          '步骤1：（可选）获取存储设备CA证书（适用于OceanStor V5/OceanStor Pacific/NetApp ONTAP）',
                        local: 'nas_s_0021.html'
                      },
                      {
                        id: 1948,
                        parentId: 1940,
                        name:
                          '步骤2：添加存储设备（适用于OceanStor V5/OceanStor Pacific/NetApp ONTAP）',
                        local: 'nas_s_0022.html'
                      },
                      {
                        id: 1949,
                        parentId: 1940,
                        name:
                          '步骤3：配置NAS共享信息（适用于OceanStor V5/OceanStor Pacific/NetApp ONTAP）',
                        local: 'nas_s_0023.html'
                      },
                      {
                        id: 1950,
                        parentId: 1940,
                        name:
                          '步骤4：配置访问权限（适用于OceanStor V5/OceanStor Pacific/NetApp ONTAP）',
                        local: 'nas_s_0025.html'
                      },
                      {
                        id: 1951,
                        parentId: 1940,
                        name:
                          '步骤5：注册NAS共享（适用于除OceanStor V5/OceanStor Pacific/NetApp ONTAP以外的存储设备）',
                        local: 'nas_s_0024.html'
                      },
                      {
                        id: 1952,
                        parentId: 1940,
                        name: '步骤6：（可选）创建限速策略',
                        local: 'nas_s_0026.html'
                      },
                      {
                        id: 1953,
                        parentId: 1940,
                        name: '步骤7：创建备份SLA',
                        local: 'nas_s_0027.html'
                      },
                      {
                        id: 1954,
                        parentId: 1940,
                        name: '步骤8：开启NFSv4.1服务（适用于部分型号）',
                        local: 'nas_s_0028_0.html'
                      },
                      {
                        id: 1955,
                        parentId: 1940,
                        name: '步骤10：执行备份',
                        local: 'nas_s_0029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1928,
                parentId: 1921,
                name: '复制',
                local: 'nas_s_0032.html',
                children: [
                  {
                    id: 1956,
                    parentId: 1928,
                    name: '复制NAS文件系统/NAS共享副本',
                    local: 'nas_s_0035.html',
                    children: [
                      {
                        id: 1957,
                        parentId: 1956,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'nas_s_0037.html'
                      },
                      {
                        id: 1958,
                        parentId: 1956,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'nas_s_0037_1.html'
                      },
                      {
                        id: 1959,
                        parentId: 1956,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'nas_s_0038.html'
                      },
                      {
                        id: 1960,
                        parentId: 1956,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'nas_s_0039.html'
                      },
                      {
                        id: 1961,
                        parentId: 1956,
                        name: '步骤4：下载并导入证书',
                        local: 'nas_s_0040.html'
                      },
                      {
                        id: 1962,
                        parentId: 1956,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'nas_s_0041.html'
                      },
                      {
                        id: 1963,
                        parentId: 1956,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002022805444.html'
                      },
                      {
                        id: 1964,
                        parentId: 1956,
                        name: '步骤6：添加复制集群',
                        local: 'nas_s_0042.html'
                      },
                      {
                        id: 1965,
                        parentId: 1956,
                        name: '步骤7：创建复制SLA',
                        local: 'nas_s_0043.html'
                      },
                      {
                        id: 1966,
                        parentId: 1956,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'nas_s_0044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1929,
                parentId: 1921,
                name: '归档',
                local: 'nas_s_0045.html',
                children: [
                  {
                    id: 1967,
                    parentId: 1929,
                    name: '归档NAS文件系统/NAS共享备份副本',
                    local: 'nas_s_0048.html',
                    children: [
                      {
                        id: 1969,
                        parentId: 1967,
                        name: '步骤1：添加归档存储',
                        local: 'nas_s_0049.html',
                        children: [
                          {
                            id: 1971,
                            parentId: 1969,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'nas_s_0050.html'
                          },
                          {
                            id: 1972,
                            parentId: 1969,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'nas_s_0051.html'
                          }
                        ]
                      },
                      {
                        id: 1970,
                        parentId: 1967,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'nas_s_0052.html'
                      }
                    ]
                  },
                  {
                    id: 1968,
                    parentId: 1929,
                    name: '归档NAS文件系统/NAS共享复制副本',
                    local: 'nas_s_0053.html',
                    children: [
                      {
                        id: 1973,
                        parentId: 1968,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'nas_s_0054.html'
                      },
                      {
                        id: 1974,
                        parentId: 1968,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'nas_s_0055.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1930,
                parentId: 1921,
                name: '恢复',
                local: 'nas_s_0056.html',
                children: [
                  {
                    id: 1975,
                    parentId: 1930,
                    name: '恢复NAS文件系统',
                    local: 'nas_s_0059.html'
                  },
                  {
                    id: 1976,
                    parentId: 1930,
                    name: '恢复NAS文件系统中的文件',
                    local: 'nas_s_0060.html'
                  },
                  {
                    id: 1977,
                    parentId: 1930,
                    name: '恢复NAS共享',
                    local: 'nas_s_0061.html'
                  },
                  {
                    id: 1978,
                    parentId: 1930,
                    name: '恢复NAS共享中的文件',
                    local: 'nas_s_0062.html'
                  }
                ]
              },
              {
                id: 1931,
                parentId: 1921,
                name: '全局搜索',
                local: 'nas_s_0073.html',
                children: [
                  {
                    id: 1979,
                    parentId: 1931,
                    name: '全局搜索副本数据',
                    local: 'nas_s_0074.html'
                  },
                  {
                    id: 1980,
                    parentId: 1931,
                    name: '全局搜索资源',
                    local: 'nas_s_0075.html'
                  },
                  {
                    id: 1981,
                    parentId: 1931,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002022936884.html'
                  }
                ]
              },
              {
                id: 1932,
                parentId: 1921,
                name: 'SLA',
                local: 'nas_s_0078.html',
                children: [
                  {
                    id: 1982,
                    parentId: 1932,
                    name: '关于SLA',
                    local: 'nas_s_0079.html'
                  },
                  {
                    id: 1983,
                    parentId: 1932,
                    name: '查看SLA信息',
                    local: 'nas_s_0080.html'
                  },
                  {
                    id: 1984,
                    parentId: 1932,
                    name: '管理SLA',
                    local: 'nas_s_0081.html'
                  }
                ]
              },
              {
                id: 1933,
                parentId: 1921,
                name: '副本',
                local: 'nas_s_0082.html',
                children: [
                  {
                    id: 1985,
                    parentId: 1933,
                    name: '查看NAS文件系统副本信息',
                    local: 'nas_s_0083.html'
                  },
                  {
                    id: 1986,
                    parentId: 1933,
                    name: '管理NAS文件系统副本',
                    local: 'nas_s_0084.html'
                  },
                  {
                    id: 1987,
                    parentId: 1933,
                    name: '查看NAS共享副本信息',
                    local: 'nas_s_0086.html'
                  },
                  {
                    id: 1988,
                    parentId: 1933,
                    name: '管理NAS共享副本',
                    local: 'nas_s_0087.html'
                  }
                ]
              },
              {
                id: 1934,
                parentId: 1921,
                name: '存储设备信息',
                local: 'nas_s_0088.html',
                children: [
                  {
                    id: 1989,
                    parentId: 1934,
                    name: '查看存储设备信息',
                    local: 'nas_s_0089.html'
                  },
                  {
                    id: 1990,
                    parentId: 1934,
                    name: '管理存储设备信息',
                    local: 'nas_s_0090.html'
                  }
                ]
              },
              {
                id: 1935,
                parentId: 1921,
                name: 'NAS文件系统',
                local: 'nas_s_0091.html',
                children: [
                  {
                    id: 1991,
                    parentId: 1935,
                    name: '查看NAS文件系统',
                    local: 'nas_s_0092.html'
                  },
                  {
                    id: 1992,
                    parentId: 1935,
                    name: '管理NAS文件系统',
                    local: 'nas_s_0093.html'
                  }
                ]
              },
              {
                id: 1936,
                parentId: 1921,
                name: 'NAS共享',
                local: 'nas_s_0094.html',
                children: [
                  {
                    id: 1993,
                    parentId: 1936,
                    name: '查看NAS共享信息',
                    local: 'nas_s_0095.html'
                  },
                  {
                    id: 1994,
                    parentId: 1936,
                    name: '管理NAS共享',
                    local: 'nas_s_0096.html'
                  }
                ]
              },
              {
                id: 1937,
                parentId: 1921,
                name: '常见问题',
                local: 'nas_s_0097.html'
              }
            ]
          },
          {
            id: 1922,
            parentId: 19,
            name: 'NDMP NAS文件系统数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000001952305809.html',
            children: [
              {
                id: 1995,
                parentId: 1922,
                name: '备份',
                local: 'nas_s_0007_0.html',
                children: [
                  {
                    id: 2005,
                    parentId: 1995,
                    name: '备份前准备',
                    local: 'nas_s_0010_0.html'
                  },
                  {
                    id: 2006,
                    parentId: 1995,
                    name: '备份NDMP NAS文件系统',
                    local: 'nas_s_0011_0.html',
                    children: [
                      {
                        id: 2007,
                        parentId: 2006,
                        name: '步骤1：添加存储设备',
                        local: 'nas_s_0013_0.html'
                      },
                      {
                        id: 2008,
                        parentId: 2006,
                        name: '步骤2：（可选）创建限速策略',
                        local: 'nas_s_0015_0.html'
                      },
                      {
                        id: 2009,
                        parentId: 2006,
                        name: '步骤3：创建备份SLA',
                        local: 'nas_s_0016_0.html'
                      },
                      {
                        id: 2010,
                        parentId: 2006,
                        name: '步骤4：执行备份',
                        local: 'nas_s_0017_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1996,
                parentId: 1922,
                name: '复制',
                local: 'nas_s_0032_0.html',
                children: [
                  {
                    id: 2011,
                    parentId: 1996,
                    name: '复制NDMP NAS文件系统',
                    local: 'nas_s_0035_0.html',
                    children: [
                      {
                        id: 2012,
                        parentId: 2011,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'nas_s_0037_1_0.html'
                      },
                      {
                        id: 2013,
                        parentId: 2011,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'nas_s_0038_0.html'
                      },
                      {
                        id: 2014,
                        parentId: 2011,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'nas_s_0039_0.html'
                      },
                      {
                        id: 2015,
                        parentId: 2011,
                        name: '步骤4：下载并导入证书',
                        local: 'nas_s_0040_0.html'
                      },
                      {
                        id: 2016,
                        parentId: 2011,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002046636865.html'
                      },
                      {
                        id: 2017,
                        parentId: 2011,
                        name: '步骤6：添加复制集群',
                        local: 'foc_gud_0032_0.html'
                      },
                      {
                        id: 2018,
                        parentId: 2011,
                        name: '步骤7：创建复制SLA',
                        local: 'nas_s_0043_0.html'
                      },
                      {
                        id: 2019,
                        parentId: 2011,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'nas_s_0044_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1997,
                parentId: 1922,
                name: '归档',
                local: 'nas_s_0045_0.html',
                children: [
                  {
                    id: 2020,
                    parentId: 1997,
                    name: '归档NDMP NAS文件系统副本',
                    local: 'nas_s_0048_0.html',
                    children: [
                      {
                        id: 2022,
                        parentId: 2020,
                        name: '步骤1：添加归档存储',
                        local: 'nas_s_0049_0.html',
                        children: [
                          {
                            id: 2024,
                            parentId: 2022,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'nas_s_0050_0.html'
                          },
                          {
                            id: 2025,
                            parentId: 2022,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'nas_s_0051_0.html'
                          }
                        ]
                      },
                      {
                        id: 2023,
                        parentId: 2020,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'oracle_gud_000031.html'
                      }
                    ]
                  },
                  {
                    id: 2021,
                    parentId: 1997,
                    name: '归档NDMP NAS文件系统复制副本',
                    local: 'nas_s_0053_0.html',
                    children: [
                      {
                        id: 2026,
                        parentId: 2021,
                        name: '步骤1：创建归档SLA',
                        local: 'nas_s_0054_0.html'
                      },
                      {
                        id: 2027,
                        parentId: 2021,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'nas_s_0055_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1998,
                parentId: 1922,
                name: '恢复',
                local: 'nas_s_0056_0.html',
                children: [
                  {
                    id: 2028,
                    parentId: 1998,
                    name: '恢复NDMP NAS文件系统',
                    local: 'nas_s_0059_0.html'
                  },
                  {
                    id: 2029,
                    parentId: 1998,
                    name: '恢复NDMP NAS文件系统中的文件',
                    local: 'nas_s_0060_0.html'
                  }
                ]
              },
              {
                id: 1999,
                parentId: 1922,
                name: '全局搜索',
                local: 'nas_s_0073_0.html',
                children: [
                  {
                    id: 2030,
                    parentId: 1999,
                    name: '全局搜索副本数据',
                    local: 'nas_s_0074_0.html'
                  },
                  {
                    id: 2031,
                    parentId: 1999,
                    name: '全局搜索资源',
                    local: 'nas_s_0075_0.html'
                  },
                  {
                    id: 2032,
                    parentId: 1999,
                    name: '全局标签搜索',
                    local: 'zh-cn_topic_0000002038775953.html'
                  }
                ]
              },
              {
                id: 2000,
                parentId: 1922,
                name: 'SLA',
                local: 'nas_s_0078_0.html',
                children: [
                  {
                    id: 2033,
                    parentId: 2000,
                    name: '关于SLA',
                    local: 'nas_s_0079_0.html'
                  },
                  {
                    id: 2034,
                    parentId: 2000,
                    name: '查看SLA信息',
                    local: 'nas_s_0080_0.html'
                  },
                  {
                    id: 2035,
                    parentId: 2000,
                    name: '管理SLA',
                    local: 'nas_s_0081_0.html'
                  }
                ]
              },
              {
                id: 2001,
                parentId: 1922,
                name: '副本',
                local: 'nas_s_0082_0.html',
                children: [
                  {
                    id: 2036,
                    parentId: 2001,
                    name: '查看NDMP NAS文件系统副本信息',
                    local: 'nas_s_0083_0.html'
                  },
                  {
                    id: 2037,
                    parentId: 2001,
                    name: '管理NDMP NAS文件系统副本',
                    local: 'nas_s_0084_0.html'
                  }
                ]
              },
              {
                id: 2002,
                parentId: 1922,
                name: '存储设备信息',
                local: 'nas_s_0088_0.html',
                children: [
                  {
                    id: 2038,
                    parentId: 2002,
                    name: '查看存储设备信息',
                    local: 'nas_s_0089_0.html'
                  },
                  {
                    id: 2039,
                    parentId: 2002,
                    name: '管理存储设备信息',
                    local: 'nas_s_0090_0.html'
                  }
                ]
              },
              {
                id: 2003,
                parentId: 1922,
                name: 'NDMP NAS文件系统',
                local: 'nas_s_0091_0.html',
                children: [
                  {
                    id: 2040,
                    parentId: 2003,
                    name: '查看NDMP NAS文件系统',
                    local: 'nas_s_0092_0.html'
                  },
                  {
                    id: 2041,
                    parentId: 2003,
                    name: '管理NDMP NAS文件系统',
                    local: 'nas_s_0093_0.html'
                  }
                ]
              },
              {
                id: 2004,
                parentId: 1922,
                name: '常见问题',
                local: 'nas_s_0097_0.html',
                children: [
                  {
                    id: 2042,
                    parentId: 2004,
                    name: '同一个NDMP Server多个并行任务部分失败',
                    local: 'zh-cn_topic_0000002017987564.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1923,
            parentId: 19,
            name: '文件集数据保护',
            local: 'zh-cn_topic_0000001873679157.html',
            children: [
              {
                id: 2043,
                parentId: 1923,
                name: '备份',
                local: 'Files-0005.html',
                children: [
                  {
                    id: 2051,
                    parentId: 2043,
                    name: '挂载对象存储到数据保护代理主机',
                    local: 'object-0006.html'
                  },
                  {
                    id: 2052,
                    parentId: 2043,
                    name: '备份文件集',
                    local: 'Files-0008.html',
                    children: [
                      {
                        id: 2053,
                        parentId: 2052,
                        name: '步骤1：（可选）创建文件集模板',
                        local: 'Files-0009.html'
                      },
                      {
                        id: 2054,
                        parentId: 2052,
                        name: '步骤2：创建文件集',
                        local: 'Files-0010.html'
                      },
                      {
                        id: 2055,
                        parentId: 2052,
                        name: '步骤3：（可选）创建限速策略',
                        local: 'Files-0011.html'
                      },
                      {
                        id: 2056,
                        parentId: 2052,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'Files-0012.html'
                      },
                      {
                        id: 2057,
                        parentId: 2052,
                        name: '步骤5：创建备份SLA',
                        local: 'Files-0013.html'
                      },
                      {
                        id: 2058,
                        parentId: 2052,
                        name: '步骤6：执行备份',
                        local: 'Files-0014.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2044,
                parentId: 1923,
                name: '复制',
                local: 'oracle_gud_000035.html',
                children: [
                  {
                    id: 2059,
                    parentId: 2044,
                    name: '复制文件集副本',
                    local: 'Files-0019.html',
                    children: [
                      {
                        id: 2060,
                        parentId: 2059,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026.html'
                      },
                      {
                        id: 2061,
                        parentId: 2059,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1.html'
                      },
                      {
                        id: 2062,
                        parentId: 2059,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'Files-0022.html'
                      },
                      {
                        id: 2063,
                        parentId: 2059,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'Files-0023.html'
                      },
                      {
                        id: 2064,
                        parentId: 2059,
                        name: '步骤4：下载并导入证书',
                        local: 'Files-0024.html'
                      },
                      {
                        id: 2065,
                        parentId: 2059,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'Files-0025.html'
                      },
                      {
                        id: 2066,
                        parentId: 2059,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002010555368.html'
                      },
                      {
                        id: 2067,
                        parentId: 2059,
                        name: '步骤：添加复制集群',
                        local: 'Files-0026.html'
                      },
                      {
                        id: 2068,
                        parentId: 2059,
                        name: '步骤7：创建复制SLA',
                        local: 'Files-0027.html'
                      },
                      {
                        id: 2069,
                        parentId: 2059,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'Files-0028.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2045,
                parentId: 1923,
                name: '归档',
                local: 'Files-0029.html',
                children: [
                  {
                    id: 2070,
                    parentId: 2045,
                    name: '归档文件集备份副本',
                    local: 'Files-0032.html',
                    children: [
                      {
                        id: 2072,
                        parentId: 2070,
                        name: '步骤1：添加归档存储',
                        local: 'Files-0033.html',
                        children: [
                          {
                            id: 2074,
                            parentId: 2072,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'Files-0034.html'
                          },
                          {
                            id: 2075,
                            parentId: 2072,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'Files-0035.html'
                          }
                        ]
                      },
                      {
                        id: 2073,
                        parentId: 2070,
                        name: '步骤2：创建归档SLA',
                        local: 'Files-0036.html'
                      }
                    ]
                  },
                  {
                    id: 2071,
                    parentId: 2045,
                    name: '归档文件集复制副本',
                    local: 'Files-0037.html',
                    children: [
                      {
                        id: 2076,
                        parentId: 2071,
                        name: '步骤1：创建归档SLA',
                        local: 'Files-0038.html'
                      },
                      {
                        id: 2077,
                        parentId: 2071,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'Files-0039.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2046,
                parentId: 1923,
                name: '恢复',
                local: 'Files-0040.html',
                children: [
                  {
                    id: 2078,
                    parentId: 2046,
                    name: '恢复文件集',
                    local: 'Files-0043.html'
                  },
                  {
                    id: 2079,
                    parentId: 2046,
                    name: '恢复文件集中的单个或多个文件',
                    local: 'Files-0044.html'
                  }
                ]
              },
              {
                id: 2047,
                parentId: 1923,
                name: '全局搜索',
                local: 'Files-0054.html',
                children: [
                  {
                    id: 2080,
                    parentId: 2047,
                    name: '全局搜索副本数据',
                    local: 'Files-0055.html'
                  },
                  {
                    id: 2081,
                    parentId: 2047,
                    name: '全局搜索资源',
                    local: 'Files-0056.html'
                  },
                  {
                    id: 2082,
                    parentId: 2047,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002002699236.html'
                  }
                ]
              },
              {
                id: 2048,
                parentId: 1923,
                name: 'SLA',
                local: 'Files-0059.html',
                children: [
                  {
                    id: 2083,
                    parentId: 2048,
                    name: '查看SLA信息',
                    local: 'Files-0061.html'
                  },
                  {
                    id: 2084,
                    parentId: 2048,
                    name: '管理SLA',
                    local: 'Files-0062.html'
                  }
                ]
              },
              {
                id: 2049,
                parentId: 1923,
                name: '副本',
                local: 'Files-0063.html',
                children: [
                  {
                    id: 2085,
                    parentId: 2049,
                    name: '查看文件集副本信息',
                    local: 'Files-0064.html'
                  },
                  {
                    id: 2086,
                    parentId: 2049,
                    name: '管理文件集副本',
                    local: 'Files-0065.html'
                  }
                ]
              },
              {
                id: 2050,
                parentId: 1923,
                name: '文件集',
                local: 'Files-0066.html',
                children: [
                  {
                    id: 2087,
                    parentId: 2050,
                    name: '查看文件集信息',
                    local: 'Files-0067.html'
                  },
                  {
                    id: 2088,
                    parentId: 2050,
                    name: '管理文件集',
                    local: 'Files-0068.html'
                  },
                  {
                    id: 2089,
                    parentId: 2050,
                    name: '管理文件集模板',
                    local: 'Files-0069.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1924,
            parentId: 19,
            name: '卷数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000001826879836.html',
            children: [
              {
                id: 2090,
                parentId: 1924,
                name: '备份',
                local: 'volume_0007.html',
                children: [
                  {
                    id: 2098,
                    parentId: 2090,
                    name: '备份卷',
                    local: 'volume_0010.html',
                    children: [
                      {
                        id: 2099,
                        parentId: 2098,
                        name: '步骤1：创建卷',
                        local: 'volume_0011.html'
                      },
                      {
                        id: 2100,
                        parentId: 2098,
                        name: '步骤2：创建限速策略',
                        local: 'volume_0012.html'
                      },
                      {
                        id: 2101,
                        parentId: 2098,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'volume_0013.html'
                      },
                      {
                        id: 2102,
                        parentId: 2098,
                        name: '步骤4：创建备份SLA',
                        local: 'volume_0014.html'
                      },
                      {
                        id: 2103,
                        parentId: 2098,
                        name: '步骤5：执行备份',
                        local: 'volume_0015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2091,
                parentId: 1924,
                name: '复制',
                local: 'oracle_gud_000035_14.html',
                children: [
                  {
                    id: 2104,
                    parentId: 2091,
                    name: '复制卷副本',
                    local: 'volume_0021.html',
                    children: [
                      {
                        id: 2105,
                        parentId: 2104,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_16.html'
                      },
                      {
                        id: 2106,
                        parentId: 2104,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_32.html'
                      },
                      {
                        id: 2107,
                        parentId: 2104,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'volume_0024.html'
                      },
                      {
                        id: 2108,
                        parentId: 2104,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'volume_0025.html'
                      },
                      {
                        id: 2109,
                        parentId: 2104,
                        name: '步骤4：下载并导入证书',
                        local: 'volume_0026.html'
                      },
                      {
                        id: 2110,
                        parentId: 2104,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'volume_0027.html'
                      },
                      {
                        id: 2111,
                        parentId: 2104,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002010594166.html'
                      },
                      {
                        id: 2112,
                        parentId: 2104,
                        name: '步骤：添加复制集群',
                        local: 'volume_0028.html'
                      },
                      {
                        id: 2113,
                        parentId: 2104,
                        name: '步骤7：创建复制SLA',
                        local: 'volume_0029.html'
                      },
                      {
                        id: 2114,
                        parentId: 2104,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'volume_0030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2092,
                parentId: 1924,
                name: '归档',
                local: 'volume_0031.html',
                children: [
                  {
                    id: 2115,
                    parentId: 2092,
                    name: '归档卷备份副本',
                    local: 'volume_0034.html',
                    children: [
                      {
                        id: 2117,
                        parentId: 2115,
                        name: '步骤1：添加归档存储',
                        local: 'volume_0035.html',
                        children: [
                          {
                            id: 2119,
                            parentId: 2117,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'volume_0036.html'
                          },
                          {
                            id: 2120,
                            parentId: 2117,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'volume_0037.html'
                          }
                        ]
                      },
                      {
                        id: 2118,
                        parentId: 2115,
                        name: '步骤2：创建归档SLA',
                        local: 'volume_0038.html'
                      }
                    ]
                  },
                  {
                    id: 2116,
                    parentId: 2092,
                    name: '归档卷复制副本',
                    local: 'volume_0039.html',
                    children: [
                      {
                        id: 2121,
                        parentId: 2116,
                        name: '步骤1：创建归档SLA',
                        local: 'volume_0040.html'
                      },
                      {
                        id: 2122,
                        parentId: 2116,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'volume_0041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2093,
                parentId: 1924,
                name: '恢复',
                local: 'volume_0042.html',
                children: [
                  {
                    id: 2123,
                    parentId: 2093,
                    name: '恢复卷',
                    local: 'volume_0045.html'
                  },
                  {
                    id: 2124,
                    parentId: 2093,
                    name: '恢复卷副本中的单个或多个文件',
                    local: 'volume_0046.html'
                  }
                ]
              },
              {
                id: 2094,
                parentId: 1924,
                name: '全局搜索',
                local: 'volume_0056.html',
                children: [
                  {
                    id: 2125,
                    parentId: 2094,
                    name: '全局搜索副本数据',
                    local: 'volume_0057.html'
                  },
                  {
                    id: 2126,
                    parentId: 2094,
                    name: '全局搜索资源',
                    local: 'volume_0058.html'
                  },
                  {
                    id: 2127,
                    parentId: 2094,
                    name: '全局标签搜索',
                    local: 'zh-cn_topic_0000002002855252.html'
                  }
                ]
              },
              {
                id: 2095,
                parentId: 1924,
                name: 'SLA',
                local: 'volume_0061.html',
                children: [
                  {
                    id: 2128,
                    parentId: 2095,
                    name: '关于SLA',
                    local: 'vmware_gud_000026_1.html'
                  },
                  {
                    id: 2129,
                    parentId: 2095,
                    name: '查看SLA信息',
                    local: 'volume_0063.html'
                  },
                  {
                    id: 2130,
                    parentId: 2095,
                    name: '管理SLA',
                    local: 'volume_0064.html'
                  }
                ]
              },
              {
                id: 2096,
                parentId: 1924,
                name: '副本',
                local: 'volume_0065.html',
                children: [
                  {
                    id: 2131,
                    parentId: 2096,
                    name: '查看卷副本信息',
                    local: 'volume_0066.html'
                  },
                  {
                    id: 2132,
                    parentId: 2096,
                    name: '管理卷副本',
                    local: 'volume_0067.html'
                  }
                ]
              },
              {
                id: 2097,
                parentId: 1924,
                name: '卷',
                local: 'volume_0068.html',
                children: [
                  {
                    id: 2133,
                    parentId: 2097,
                    name: '查看卷信息',
                    local: 'volume_0069.html'
                  },
                  {
                    id: 2134,
                    parentId: 2097,
                    name: '管理卷',
                    local: 'volume_0070.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1925,
            parentId: 19,
            name: '对象存储数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000001826879816.html',
            children: [
              {
                id: 2135,
                parentId: 1925,
                name: '备份',
                local: 'object-0003.html',
                children: [
                  {
                    id: 2143,
                    parentId: 2135,
                    name: '备份前准备',
                    local: 'zh-cn_topic_0000001953327945.html',
                    children: [
                      {
                        id: 2145,
                        parentId: 2143,
                        name: '在生产端获取Endpoint',
                        local: 'zh-cn_topic_0000001926224624.html'
                      },
                      {
                        id: 2146,
                        parentId: 2143,
                        name: '在生产端获取AK和SK',
                        local: 'zh-cn_topic_0000001953344001.html'
                      }
                    ]
                  },
                  {
                    id: 2144,
                    parentId: 2135,
                    name: '备份对象存储',
                    local: 'object-0007.html',
                    children: [
                      {
                        id: 2147,
                        parentId: 2144,
                        name: '步骤1：注册对象存储',
                        local: 'object-0008.html'
                      },
                      {
                        id: 2148,
                        parentId: 2144,
                        name: '步骤2：创建对象集合',
                        local: 'object-0009.html'
                      },
                      {
                        id: 2149,
                        parentId: 2144,
                        name: '步骤3：（可选）创建限速策略',
                        local: 'object-0010.html'
                      },
                      {
                        id: 2150,
                        parentId: 2144,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'object-0011.html'
                      },
                      {
                        id: 2151,
                        parentId: 2144,
                        name: '步骤5：创建备份SLA',
                        local: 'object-0012.html'
                      },
                      {
                        id: 2152,
                        parentId: 2144,
                        name: '步骤7：执行备份',
                        local: 'object-0013.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2136,
                parentId: 1925,
                name: '复制',
                local: 'oracle_gud_000035_4.html',
                children: [
                  {
                    id: 2153,
                    parentId: 2136,
                    name: '复制对象存储副本',
                    local: 'object-0018.html',
                    children: [
                      {
                        id: 2154,
                        parentId: 2153,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_5.html'
                      },
                      {
                        id: 2155,
                        parentId: 2153,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_11.html'
                      },
                      {
                        id: 2156,
                        parentId: 2153,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'object-0021.html'
                      },
                      {
                        id: 2157,
                        parentId: 2153,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'object-0022.html'
                      },
                      {
                        id: 2158,
                        parentId: 2153,
                        name: '步骤4：下载并导入证书',
                        local: 'object-0023.html'
                      },
                      {
                        id: 2159,
                        parentId: 2153,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'fc_gud_0030.html'
                      },
                      {
                        id: 2160,
                        parentId: 2153,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002046625105.html'
                      },
                      {
                        id: 2161,
                        parentId: 2153,
                        name: '步骤：添加复制集群',
                        local: 'object-0025.html'
                      },
                      {
                        id: 2162,
                        parentId: 2153,
                        name: '步骤7：创建复制SLA',
                        local: 'object-0026.html'
                      },
                      {
                        id: 2163,
                        parentId: 2153,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'object-0027.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2137,
                parentId: 1925,
                name: '归档',
                local: 'object-0028.html',
                children: [
                  {
                    id: 2164,
                    parentId: 2137,
                    name: '归档对象集合备份副本',
                    local: 'object-0031.html',
                    children: [
                      {
                        id: 2166,
                        parentId: 2164,
                        name: '步骤1：添加归档存储',
                        local: 'object-0032.html',
                        children: [
                          {
                            id: 2168,
                            parentId: 2166,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'object-0033.html'
                          },
                          {
                            id: 2169,
                            parentId: 2166,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'object-0034.html'
                          }
                        ]
                      },
                      {
                        id: 2167,
                        parentId: 2164,
                        name: '步骤2：创建归档SLA',
                        local: 'object-0035.html'
                      }
                    ]
                  },
                  {
                    id: 2165,
                    parentId: 2137,
                    name: '归档对象集合复制副本',
                    local: 'object-0036.html',
                    children: [
                      {
                        id: 2170,
                        parentId: 2165,
                        name: '步骤1：创建归档SLA',
                        local: 'object-0037.html'
                      },
                      {
                        id: 2171,
                        parentId: 2165,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'object-0038.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2138,
                parentId: 1925,
                name: '恢复',
                local: 'object-0039.html',
                children: [
                  {
                    id: 2172,
                    parentId: 2138,
                    name: '恢复对象存储',
                    local: 'object-0042.html'
                  }
                ]
              },
              {
                id: 2139,
                parentId: 1925,
                name: '全局搜索',
                local: 'object-0049.html',
                children: [
                  {
                    id: 2173,
                    parentId: 2139,
                    name: '全局搜索副本数据',
                    local: 'object-0050.html'
                  },
                  {
                    id: 2174,
                    parentId: 2139,
                    name: '全局搜索资源',
                    local: 'object-0051.html'
                  },
                  {
                    id: 2175,
                    parentId: 2139,
                    name: '全局标签搜索',
                    local: 'zh-cn_topic_0000002002688946.html'
                  }
                ]
              },
              {
                id: 2140,
                parentId: 1925,
                name: 'SLA',
                local: 'object-0054.html',
                children: [
                  {
                    id: 2176,
                    parentId: 2140,
                    name: '关于SLA',
                    local: 'object-0055.html'
                  },
                  {
                    id: 2177,
                    parentId: 2140,
                    name: '查看SLA信息',
                    local: 'object-0056.html'
                  },
                  {
                    id: 2178,
                    parentId: 2140,
                    name: '管理SLA',
                    local: 'object-0057.html'
                  }
                ]
              },
              {
                id: 2141,
                parentId: 1925,
                name: '副本',
                local: 'object-0058.html',
                children: [
                  {
                    id: 2179,
                    parentId: 2141,
                    name: '查看对象存储副本信息',
                    local: 'object-0059.html'
                  },
                  {
                    id: 2180,
                    parentId: 2141,
                    name: '管理对象存储副本',
                    local: 'object-0060.html'
                  }
                ]
              },
              {
                id: 2142,
                parentId: 1925,
                name: '对象存储',
                local: 'object-0061.html',
                children: [
                  {
                    id: 2181,
                    parentId: 2142,
                    name: '查看对象存储信息',
                    local: 'object-0062.html'
                  },
                  {
                    id: 2182,
                    parentId: 2142,
                    name: '管理对象集合',
                    local: 'object-0063.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1926,
            parentId: 19,
            name: '通用共享数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000001873759353.html',
            children: [
              {
                id: 2183,
                parentId: 1926,
                name: '备份',
                local: 'commonshares_0007.html',
                children: [
                  {
                    id: 2190,
                    parentId: 2183,
                    name: '备份通用共享资源数据',
                    local: 'commonshares_0010.html',
                    children: [
                      {
                        id: 2191,
                        parentId: 2190,
                        name: '步骤1：创建通用共享',
                        local: 'commonshares_0011.html'
                      },
                      {
                        id: 2192,
                        parentId: 2190,
                        name: '步骤2：创建限速策略',
                        local: 'commonshares_0012.html'
                      },
                      {
                        id: 2193,
                        parentId: 2190,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'commonshares_0013.html'
                      },
                      {
                        id: 2194,
                        parentId: 2190,
                        name: '步骤4：创建备份SLA',
                        local: 'commonshares_0014.html'
                      },
                      {
                        id: 2195,
                        parentId: 2190,
                        name: '步骤5：执行备份',
                        local: 'commonshares_0015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2184,
                parentId: 1926,
                name: '复制',
                local: 'oracle_gud_000035_8.html',
                children: [
                  {
                    id: 2196,
                    parentId: 2184,
                    name: '复制通用共享副本',
                    local: 'commonshares_0021.html',
                    children: [
                      {
                        id: 2197,
                        parentId: 2196,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_9.html'
                      },
                      {
                        id: 2198,
                        parentId: 2196,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_18.html'
                      },
                      {
                        id: 2199,
                        parentId: 2196,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'commonshares_0024.html'
                      },
                      {
                        id: 2200,
                        parentId: 2196,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'commonshares_0025.html'
                      },
                      {
                        id: 2201,
                        parentId: 2196,
                        name: '步骤4：下载并导入证书',
                        local: 'commonshares_0026.html'
                      },
                      {
                        id: 2202,
                        parentId: 2196,
                        name: '步骤5：创建远端设备管理员',
                        local: 'commonshares_0027.html'
                      },
                      {
                        id: 2203,
                        parentId: 2196,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'zh-cn_topic_0000002010591334.html'
                      },
                      {
                        id: 2204,
                        parentId: 2196,
                        name: '步骤：添加复制集群',
                        local: 'commonshares_0028.html'
                      },
                      {
                        id: 2205,
                        parentId: 2196,
                        name: '步骤7：创建复制SLA',
                        local: 'commonshares_0029.html'
                      },
                      {
                        id: 2206,
                        parentId: 2196,
                        name: '步骤8：（可选）创建反向复制/级联复制SLA',
                        local: 'commonshares_0030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2185,
                parentId: 1926,
                name: '归档',
                local: 'commonshares_0031.html',
                children: [
                  {
                    id: 2207,
                    parentId: 2185,
                    name: '归档通用共享资源备份副本',
                    local: 'commonshares_0034.html',
                    children: [
                      {
                        id: 2209,
                        parentId: 2207,
                        name: '步骤1：添加归档存储',
                        local: 'commonshares_0035.html',
                        children: [
                          {
                            id: 2211,
                            parentId: 2209,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'commonshares_0036.html'
                          },
                          {
                            id: 2212,
                            parentId: 2209,
                            name: '创建介质集（归档存储是磁带库）',
                            local: 'commonshares_0037.html'
                          }
                        ]
                      },
                      {
                        id: 2210,
                        parentId: 2207,
                        name: '步骤2：创建归档SLA',
                        local: 'commonshares_0038.html'
                      }
                    ]
                  },
                  {
                    id: 2208,
                    parentId: 2185,
                    name: '归档通用共享资源复制副本',
                    local: 'commonshares_0039.html',
                    children: [
                      {
                        id: 2213,
                        parentId: 2208,
                        name: '步骤1：创建归档SLA',
                        local: 'commonshares_0040.html'
                      },
                      {
                        id: 2214,
                        parentId: 2208,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'commonshares_0041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2186,
                parentId: 1926,
                name: '管理共享信息',
                local: 'commonshares_0046.html',
                children: [
                  {
                    id: 2215,
                    parentId: 2186,
                    name: '配置共享信息',
                    local: 'commonshares_0047.html'
                  },
                  {
                    id: 2216,
                    parentId: 2186,
                    name: '查看共享信息',
                    local: 'commonshares_0048.html'
                  },
                  {
                    id: 2217,
                    parentId: 2186,
                    name: '删除共享信息',
                    local: 'commonshares_0049.html'
                  }
                ]
              },
              {
                id: 2187,
                parentId: 1926,
                name: 'SLA',
                local: 'zh-cn_topic_0000001927513108.html',
                children: [
                  {
                    id: 2218,
                    parentId: 2187,
                    name: '关于SLA',
                    local: 'vmware_gud_000026_0.html'
                  },
                  {
                    id: 2219,
                    parentId: 2187,
                    name: '查看SLA信息',
                    local: 'zh-cn_topic_0000001954672445.html'
                  },
                  {
                    id: 2220,
                    parentId: 2187,
                    name: '管理SLA',
                    local: 'zh-cn_topic_0000001927353788.html'
                  }
                ]
              },
              {
                id: 2188,
                parentId: 1926,
                name: '副本',
                local: 'commonshares_0059.html',
                children: [
                  {
                    id: 2221,
                    parentId: 2188,
                    name: '查看通用共享资源副本信息',
                    local: 'commonshares_0060.html'
                  },
                  {
                    id: 2222,
                    parentId: 2188,
                    name: '管理通用共享资源副本',
                    local: 'commonshares_0061.html'
                  }
                ]
              },
              {
                id: 2189,
                parentId: 1926,
                name: '通用共享',
                local: 'commonshares_0062.html',
                children: [
                  {
                    id: 2223,
                    parentId: 2189,
                    name: '查看通用共享信息',
                    local: 'commonshares_0063.html'
                  },
                  {
                    id: 2224,
                    parentId: 2189,
                    name: '管理通用共享',
                    local: 'commonshares_0064.html'
                  }
                ]
              }
            ]
          }
        ]
      }
    ]
  },
  {
    id: 4,
    parentId: 0,
    name: '数据利用',
    local: 'helpcenter_000068.html',
    children: [
      {
        id: 2225,
        parentId: 4,
        name: '恢复演练',
        local: 'zh-cn_topic_0000001827336292.html',
        children: [
          {
            id: 2229,
            parentId: 2225,
            name: '创建演练计划',
            local: 'Ransomware0011.html'
          },
          {
            id: 2230,
            parentId: 2225,
            name: '管理演练计划',
            local: 'zh-cn_topic_0000001867357857.html'
          },
          {
            id: 2231,
            parentId: 2225,
            name: '总览恢复演练',
            local: 'zh-cn_topic_0000001896690129.html'
          }
        ]
      },
      {
        id: 2226,
        parentId: 4,
        name: '数据脱敏',
        local: 'helpcenter_000092.html',
        children: [
          {
            id: 2232,
            parentId: 2226,
            name: '配置数据脱敏',
            local: 'anonymization_0010.html',
            children: [
              {
                id: 2237,
                parentId: 2232,
                name: '导入并激活License文件',
                local: 'anonymization_0011.html'
              },
              {
                id: 2238,
                parentId: 2232,
                name: '添加脱敏规则',
                local: 'anonymization_0012.html'
              },
              {
                id: 2239,
                parentId: 2232,
                name: '添加识别规则',
                local: 'anonymization_0013.html'
              },
              {
                id: 2240,
                parentId: 2232,
                name: '创建脱敏策略',
                local: 'anonymization_0014.html'
              }
            ]
          },
          {
            id: 2233,
            parentId: 2226,
            name: 'Oracle数据脱敏',
            local: 'anonymization_0015.html'
          },
          {
            id: 2234,
            parentId: 2226,
            name: '管理数据脱敏',
            local: 'anonymization_0016.html',
            children: [
              {
                id: 2241,
                parentId: 2234,
                name: '管理脱敏策略',
                local: 'anonymization_0017.html'
              },
              {
                id: 2242,
                parentId: 2234,
                name: '管理识别规则',
                local: 'anonymization_0018.html'
              },
              {
                id: 2243,
                parentId: 2234,
                name: '管理脱敏规则',
                local: 'anonymization_0019.html'
              }
            ]
          },
          {
            id: 2235,
            parentId: 2226,
            name: '脱敏规则类型说明',
            local: 'anonymization_0020.html'
          },
          {
            id: 2236,
            parentId: 2226,
            name: '配置数据库侦听',
            local: 'anonymization_0021.html'
          }
        ]
      },
      {
        id: 2227,
        parentId: 4,
        name: '防勒索',
        local: 'helpcenter_000094.html',
        children: [
          {
            id: 2244,
            parentId: 2227,
            name: '配置副本防勒索',
            local: 'ransome_0011.html',
            children: [
              {
                id: 2250,
                parentId: 2244,
                name: '创建防勒索\u0026WORM策略',
                local: 'ransome_0012.html'
              }
            ]
          },
          {
            id: 2245,
            parentId: 2227,
            name: '执行副本防勒索（适用于1.5.0版本）',
            local: 'ransome_0013.html',
            children: [
              {
                id: 2251,
                parentId: 2245,
                name: 'VMware副本勒索软件检测',
                local: 'ransome_0014.html'
              },
              {
                id: 2252,
                parentId: 2245,
                name: 'NAS文件系统副本勒索软件检测',
                local: 'ransome_0015.html'
              },
              {
                id: 2253,
                parentId: 2245,
                name: 'NAS共享副本勒索软件检测',
                local: 'ransome_0016.html'
              },
              {
                id: 2254,
                parentId: 2245,
                name: '文件集副本勒索软件检测',
                local: 'ransome_0017.html'
              }
            ]
          },
          {
            id: 2246,
            parentId: 2227,
            name: '执行副本防勒索（适用于1.6.0及后续版本）',
            local: 'ransome16_001.html',
            children: [
              {
                id: 2255,
                parentId: 2246,
                name: 'VMware虚拟机副本勒索软件检测',
                local: 'ransome16_002.html'
              },
              {
                id: 2256,
                parentId: 2246,
                name: 'NAS文件系统副本勒索软件检测',
                local: 'ransome16_003.html'
              },
              {
                id: 2257,
                parentId: 2246,
                name: 'NAS共享副本勒索软件检测',
                local: 'ransome16_004.html'
              },
              {
                id: 2258,
                parentId: 2246,
                name: '文件集副本勒索软件检测',
                local: 'ransome16_005.html'
              },
              {
                id: 2259,
                parentId: 2246,
                name: 'CNware虚拟机副本勒索软件检测',
                local: 'ransome16_006.html'
              },
              {
                id: 2260,
                parentId: 2246,
                name: '华为云Stack副本勒索软件检测',
                local: 'ransome16_007.html'
              },
              {
                id: 2261,
                parentId: 2246,
                name: 'FusionCompute虚拟机副本勒索软件检测',
                local: 'ransome16_008.html'
              },
              {
                id: 2262,
                parentId: 2246,
                name: 'OpenStack云服务器副本勒索软件检测',
                local: 'ransome16_009.html'
              },
              {
                id: 2263,
                parentId: 2246,
                name: 'Hyper-V虚拟机副本勒索软件检测',
                local: 'ransome16_010.html'
              },
              {
                id: 2264,
                parentId: 2246,
                name: 'FusionOne Compute虚拟机副本勒索软件检测',
                local: 'ransome160_012.html'
              }
            ]
          },
          {
            id: 2247,
            parentId: 2227,
            name: '管理副本防勒索',
            local: 'ransome_0018.html',
            children: [
              {
                id: 2265,
                parentId: 2247,
                name: '管理检测模型',
                local: 'ransome_0019.html'
              },
              {
                id: 2266,
                parentId: 2247,
                name: '管理防勒索\u0026WORM策略',
                local: 'ransome_0020.html'
              },
              {
                id: 2267,
                parentId: 2247,
                name: '管理检测模式',
                local: 'ransome_0021.html'
              },
              {
                id: 2268,
                parentId: 2247,
                name: '管理勒索检测副本',
                local: 'ransome_0022.html'
              },
              {
                id: 2269,
                parentId: 2247,
                name: '管理WORM副本',
                local: 'ransome_0023.html'
              },
              {
                id: 2270,
                parentId: 2247,
                name: '管理感染副本操作限制（适用于1.6.0及后续版本）',
                local: 'ransome_dis_001.html',
                children: [
                  {
                    id: 2271,
                    parentId: 2270,
                    name: '新增感染副本操作限制',
                    local: 'ransome_dis_002.html'
                  },
                  {
                    id: 2272,
                    parentId: 2270,
                    name: '浏览感染副本操作限制信息',
                    local: 'ransome_dis_003.html'
                  },
                  {
                    id: 2273,
                    parentId: 2270,
                    name: '修改感染副本操作限制',
                    local: 'ransome_dis_004.html'
                  },
                  {
                    id: 2274,
                    parentId: 2270,
                    name: '删除感染副本操作限制',
                    local: 'ransome_dis_005.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2248,
            parentId: 2227,
            name: '查看资源检测详情（适用于1.5.0版本）',
            local: 'ransome_0024.html',
            children: [
              {
                id: 2275,
                parentId: 2248,
                name: '查看所有资源检测详情',
                local: 'ransome_0025.html'
              },
              {
                id: 2276,
                parentId: 2248,
                name: '查看单个资源类型检测详情',
                local: 'ransome_0026.html'
              }
            ]
          },
          {
            id: 2249,
            parentId: 2227,
            name: '查看资源检测详情（适用于1.6.0及后续版本）',
            local: 'ransome16_011.html',
            children: [
              {
                id: 2277,
                parentId: 2249,
                name: '查看所有资源检测详情',
                local: 'ransome16_012.html'
              },
              {
                id: 2278,
                parentId: 2249,
                name: '查看单个资源类型检测详情',
                local: 'ransome16_013.html'
              }
            ]
          }
        ]
      },
      {
        id: 2228,
        parentId: 4,
        name: 'Air Gap',
        local: 'helpcenter_000096.html',
        children: [
          {
            id: 2279,
            parentId: 2228,
            name: '配置Air Gap',
            local: 'airgap_0011.html',
            children: [
              {
                id: 2282,
                parentId: 2279,
                name: '创建Air Gap策略',
                local: 'airgap_0012.html'
              },
              {
                id: 2283,
                parentId: 2279,
                name: '关联Air Gap策略',
                local: 'airgap_0013.html'
              }
            ]
          },
          {
            id: 2280,
            parentId: 2228,
            name: '管理Air Gap策略',
            local: 'airgap_0014.html',
            children: [
              {
                id: 2284,
                parentId: 2280,
                name: '查看Air Gap策略',
                local: 'airgap_0015.html'
              },
              {
                id: 2285,
                parentId: 2280,
                name: '修改Air Gap策略',
                local: 'airgap_0016.html'
              },
              {
                id: 2286,
                parentId: 2280,
                name: '删除Air Gap策略',
                local: 'airgap_0017.html'
              }
            ]
          },
          {
            id: 2281,
            parentId: 2228,
            name: '管理存储设备',
            local: 'airgap_0018.html',
            children: [
              {
                id: 2287,
                parentId: 2281,
                name: '查看存储设备',
                local: 'airgap_0019.html'
              },
              {
                id: 2288,
                parentId: 2281,
                name: '修改存储设备关联的Air Gap策略',
                local: 'airgap_0020.html'
              },
              {
                id: 2289,
                parentId: 2281,
                name: '移除存储设备关联的Air Gap策略',
                local: 'airgap_0021.html'
              },
              {
                id: 2290,
                parentId: 2281,
                name: '开启存储设备关联的Air Gap策略',
                local: 'airgap_0022.html'
              },
              {
                id: 2291,
                parentId: 2281,
                name: '关闭存储设备关联的Air Gap策略',
                local: 'airgap_0023.html'
              },
              {
                id: 2292,
                parentId: 2281,
                name: '断开存储设备的复制链路（适用于1.6.0及后续版本）',
                local: 'zh-cn_topic_0000002037030445.html'
              }
            ]
          }
        ]
      }
    ]
  },
  {
    id: 5,
    parentId: 0,
    name: '集群高可用',
    local: 'zh-cn_topic_0000001792345234.html',
    children: [
      {
        id: 2293,
        parentId: 5,
        name: '配置集群高可用',
        local: 'HA00010.html',
        children: [
          {
            id: 2296,
            parentId: 2293,
            name: '添加主节点内部通信网络平面',
            local: 'zh-cn_topic_0000001959411965.html',
            children: [
              {
                id: 2301,
                parentId: 2296,
                name: '添加主节点内部通信网络平面（适用于1.5.0版本）',
                local: 'HA00011.html'
              },
              {
                id: 2302,
                parentId: 2296,
                name: '添加主节点内部通信网络（适用于1.6.0及后续版本）',
                local: 'zh-cn_topic_0000001959371749.html'
              }
            ]
          },
          {
            id: 2297,
            parentId: 2293,
            name: '添加成员节点内部通信网络（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000001930311476.html'
          },
          {
            id: 2298,
            parentId: 2293,
            name: '添加成员节点',
            local: 'HA00012.html'
          },
          {
            id: 2299,
            parentId: 2293,
            name: '添加HA成员',
            local: 'HA00013.html'
          },
          {
            id: 2300,
            parentId: 2293,
            name: '（可选）创建备份存储单元组',
            local: 'HA00014.html'
          }
        ]
      },
      { id: 2294, parentId: 5, name: '使用集群高可用', local: 'HA00015.html' },
      {
        id: 2295,
        parentId: 5,
        name: '管理集群高可用',
        local: 'HA00016.html',
        children: [
          {
            id: 2303,
            parentId: 2295,
            name: '管理本地集群节点',
            local: 'HA00017.html',
            children: [
              {
                id: 2310,
                parentId: 2303,
                name: '查看本地集群节点',
                local: 'HA00018.html'
              },
              {
                id: 2311,
                parentId: 2303,
                name: '管理备节点/成员节点',
                local: 'HA00019.html',
                children: [
                  {
                    id: 2313,
                    parentId: 2311,
                    name: '修改备节点/成员节点',
                    local: 'HA00020.html'
                  },
                  {
                    id: 2314,
                    parentId: 2311,
                    name: '删除成员节点',
                    local: 'HA00021.html'
                  }
                ]
              },
              {
                id: 2312,
                parentId: 2303,
                name: '管理HA',
                local: 'HA00022.html',
                children: [
                  {
                    id: 2315,
                    parentId: 2312,
                    name: '修改HA参数',
                    local: 'HA00023.html'
                  },
                  {
                    id: 2316,
                    parentId: 2312,
                    name: '移除HA成员',
                    local: 'HA00024.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2304,
            parentId: 2295,
            name: '管理备份存储单元组',
            local: 'HA00025.html',
            children: [
              {
                id: 2317,
                parentId: 2304,
                name: '查看备份存储单元组',
                local: 'HA00026.html'
              },
              {
                id: 2318,
                parentId: 2304,
                name: '修改备份存储单元组',
                local: 'HA00027.html'
              },
              {
                id: 2319,
                parentId: 2304,
                name: '删除备份存储单元组',
                local: 'HA00028.html'
              }
            ]
          },
          {
            id: 2305,
            parentId: 2295,
            name: '管理备份存储单元（适用于1.5.0版本）',
            local: 'HA00029.html',
            children: [
              {
                id: 2320,
                parentId: 2305,
                name: '查看备份存储单元',
                local: 'HA00030.html'
              },
              {
                id: 2321,
                parentId: 2305,
                name: '创建备份存储单元',
                local: 'HA00031.html'
              },
              {
                id: 2322,
                parentId: 2305,
                name: '修改备份存储单元',
                local: 'HA00032.html'
              },
              {
                id: 2323,
                parentId: 2305,
                name: '删除备份存储单元',
                local: 'HA00033.html'
              },
              {
                id: 2324,
                parentId: 2305,
                name: '备份存储单元升级为成员节点',
                local: 'HA00034.html'
              }
            ]
          },
          {
            id: 2306,
            parentId: 2295,
            name: '管理备份存储设备（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000001882347881.html',
            children: [
              {
                id: 2325,
                parentId: 2306,
                name: '查看备份存储设备',
                local: 'zh-cn_topic_0000001835508820.html'
              },
              {
                id: 2326,
                parentId: 2306,
                name: '创建备份存储设备',
                local: 'zh-cn_topic_0000001835668628.html'
              },
              {
                id: 2327,
                parentId: 2306,
                name: '修改备份存储设备',
                local: 'zh-cn_topic_0000001882188093.html'
              },
              {
                id: 2328,
                parentId: 2306,
                name: '删除备份存储设备',
                local: 'zh-cn_topic_0000001882347885.html'
              },
              {
                id: 2329,
                parentId: 2306,
                name: '备份存储设备升级为成员节点',
                local: 'zh-cn_topic_0000001835508824.html'
              }
            ]
          },
          {
            id: 2307,
            parentId: 2295,
            name: '管理备份存储单元（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000001882402333.html',
            children: [
              {
                id: 2330,
                parentId: 2307,
                name: '查看备份存储单元',
                local: 'zh-cn_topic_0000001835723282.html'
              },
              {
                id: 2331,
                parentId: 2307,
                name: '创建备份存储单元',
                local: 'zh-cn_topic_0000001835563470.html'
              },
              {
                id: 2332,
                parentId: 2307,
                name: '修改备份存储单元',
                local: 'zh-cn_topic_0000001882522541.html'
              },
              {
                id: 2333,
                parentId: 2307,
                name: '删除备份存储单元',
                local: 'zh-cn_topic_0000001882402749.html'
              }
            ]
          },
          {
            id: 2308,
            parentId: 2295,
            name: '管理内部通信网络平面（适用于1.5.0版本）',
            local: 'zh-cn_topic_0000001792502578.html',
            children: [
              {
                id: 2334,
                parentId: 2308,
                name: '修改内部通信网络平面',
                local: 'zh-cn_topic_0000001839221897.html'
              },
              {
                id: 2335,
                parentId: 2308,
                name: '删除内部通信网络平面',
                local: 'zh-cn_topic_0000001839221921.html'
              }
            ]
          },
          {
            id: 2309,
            parentId: 2295,
            name: '管理内部通信网络（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000001932222576.html',
            children: [
              {
                id: 2336,
                parentId: 2309,
                name: '修改内部通信网络',
                local: 'zh-cn_topic_0000001932381960.html'
              },
              {
                id: 2337,
                parentId: 2309,
                name: '删除内部通信网络',
                local: 'zh-cn_topic_0000001959541113.html'
              }
            ]
          }
        ]
      }
    ]
  },
  {
    id: 6,
    parentId: 0,
    name: '监控',
    local: 'admin-00134.html',
    children: [
      {
        id: 2338,
        parentId: 6,
        name: '管理性能统计',
        local: 'admin-00135.html',
        children: [
          {
            id: 2342,
            parentId: 2338,
            name: '性能指标介绍',
            local: 'admin-00136.html'
          },
          {
            id: 2343,
            parentId: 2338,
            name: '配置性能统计开关',
            local: 'admin-00137.html'
          }
        ]
      },
      {
        id: 2339,
        parentId: 6,
        name: '管理告警和事件',
        local: 'admin-00139.html'
      },
      {
        id: 2340,
        parentId: 6,
        name: '管理任务',
        local: 'admin-00140.html',
        children: [
          {
            id: 2344,
            parentId: 2340,
            name: '查看任务进度',
            local: 'admin-00141.html'
          },
          {
            id: 2345,
            parentId: 2340,
            name: '停止任务',
            local: 'admin-00142.html'
          },
          {
            id: 2346,
            parentId: 2340,
            name: '下载任务',
            local: 'admin-00143.html'
          }
        ]
      },
      {
        id: 2341,
        parentId: 6,
        name: '管理报表',
        local: 'admin-00144.html',
        children: [
          {
            id: 2347,
            parentId: 2341,
            name: '用户角色权限',
            local: 'admin-00145.html'
          },
          {
            id: 2348,
            parentId: 2341,
            name: '创建报表',
            local: 'admin-00146.html'
          },
          {
            id: 2349,
            parentId: 2341,
            name: '查看报表',
            local: 'admin-00147.html'
          },
          {
            id: 2350,
            parentId: 2341,
            name: '下载报表',
            local: 'admin-00148.html'
          },
          {
            id: 2351,
            parentId: 2341,
            name: '发送邮件',
            local: 'admin-00149.html'
          },
          {
            id: 2352,
            parentId: 2341,
            name: '删除报表',
            local: 'admin-00150.html'
          }
        ]
      }
    ]
  },
  {
    id: 7,
    parentId: 0,
    name: '系统',
    local: 'helpcenter_000110.html',
    children: [
      {
        id: 2353,
        parentId: 7,
        name: '管理用户（适用于1.5.0版本）',
        local: 'helpcenter_000159.html',
        children: [
          {
            id: 2378,
            parentId: 2353,
            name: '用户角色介绍',
            local: 'helpcenter_000160.html'
          },
          {
            id: 2379,
            parentId: 2353,
            name: '创建用户',
            local: 'helpcenter_000161.html'
          },
          {
            id: 2380,
            parentId: 2353,
            name: '修改用户',
            local: 'helpcenter_000162.html'
          },
          {
            id: 2381,
            parentId: 2353,
            name: '锁定用户',
            local: 'helpcenter_000163.html'
          },
          {
            id: 2382,
            parentId: 2353,
            name: '解锁用户',
            local: 'helpcenter_000164.html'
          },
          {
            id: 2383,
            parentId: 2353,
            name: '移除用户',
            local: 'helpcenter_000165.html'
          },
          {
            id: 2384,
            parentId: 2353,
            name: '重置用户密码',
            local: 'helpcenter_000166.html'
          },
          {
            id: 2385,
            parentId: 2353,
            name: '重置系统管理员密码',
            local: 'helpcenter_000167.html'
          }
        ]
      },
      {
        id: 2354,
        parentId: 7,
        name: '管理RBAC（适用于1.6.0及后续版本）',
        local: 'admin-0055.html',
        children: [
          {
            id: 2386,
            parentId: 2354,
            name: '内置用户角色介绍',
            local: 'admin-0056.html'
          },
          {
            id: 2387,
            parentId: 2354,
            name: '创建角色',
            local: 'zh-cn_topic_0000002059543622.html'
          },
          {
            id: 2388,
            parentId: 2354,
            name: '修改角色',
            local: 'zh-cn_topic_0000002059385278.html'
          },
          {
            id: 2389,
            parentId: 2354,
            name: '克隆角色',
            local: 'zh-cn_topic_0000002095582433.html'
          },
          {
            id: 2390,
            parentId: 2354,
            name: '删除角色',
            local: 'zh-cn_topic_0000002095463905.html'
          },
          {
            id: 2391,
            parentId: 2354,
            name: '创建资源集',
            local: 'zh-cn_topic_0000002059543630.html'
          },
          {
            id: 2392,
            parentId: 2354,
            name: '删除资源集',
            local: 'zh-cn_topic_0000002059385282.html'
          },
          {
            id: 2393,
            parentId: 2354,
            name: '修改资源集',
            local: 'zh-cn_topic_0000002095582437.html'
          },
          {
            id: 2394,
            parentId: 2354,
            name: '创建用户',
            local: 'admin-0057.html'
          },
          {
            id: 2395,
            parentId: 2354,
            name: '修改用户',
            local: 'admin-0058.html'
          },
          {
            id: 2396,
            parentId: 2354,
            name: '锁定用户',
            local: 'admin-0059.html'
          },
          {
            id: 2397,
            parentId: 2354,
            name: '解锁用户',
            local: 'admin-0060.html'
          },
          {
            id: 2398,
            parentId: 2354,
            name: '移除用户',
            local: 'admin-0061.html'
          },
          {
            id: 2399,
            parentId: 2354,
            name: '重置用户密码',
            local: 'admin-0062.html'
          },
          {
            id: 2400,
            parentId: 2354,
            name: '重置系统管理员密码',
            local: 'admin-0063.html'
          },
          {
            id: 2401,
            parentId: 2354,
            name: '找回密码邮箱设置',
            local: 'admin-0064.html'
          }
        ]
      },
      {
        id: 2355,
        parentId: 7,
        name: '管理SAML SSO配置',
        local: 'zh-cn_topic_0000001839224453.html',
        children: [
          {
            id: 2402,
            parentId: 2355,
            name: '创建SAML SSO配置',
            local: 'zh-cn_topic_0000001839144321.html'
          },
          {
            id: 2403,
            parentId: 2355,
            name: '管理SAML SSO 配置',
            local: 'zh-cn_topic_0000001839224341.html',
            children: [
              {
                id: 2405,
                parentId: 2403,
                name: '激活/禁用SAML SSO 配置',
                local: 'zh-cn_topic_0000001839224301.html'
              },
              {
                id: 2406,
                parentId: 2403,
                name: '修改SAML SSO配置',
                local: 'zh-cn_topic_0000001839144461.html'
              },
              {
                id: 2407,
                parentId: 2403,
                name: '删除SAML SSO配置',
                local: 'zh-cn_topic_0000001839224281.html'
              }
            ]
          },
          {
            id: 2404,
            parentId: 2355,
            name: '导出元数据',
            local: 'zh-cn_topic_0000001792345298.html'
          }
        ]
      },
      {
        id: 2356,
        parentId: 7,
        name: '管理配额与功能',
        local: 'zh-cn_topic_0000001792505014.html',
        children: [
          {
            id: 2408,
            parentId: 2356,
            name: '查看配额与功能',
            local: 'zh-cn_topic_0000001839224473.html'
          },
          {
            id: 2409,
            parentId: 2356,
            name: '设置配额',
            local: 'zh-cn_topic_0000001839224433.html'
          },
          {
            id: 2410,
            parentId: 2356,
            name: '设置功能（适用于1.6.RC1及之前版本）',
            local: 'zh-cn_topic_0000001792345246.html'
          }
        ]
      },
      {
        id: 2357,
        parentId: 7,
        name: '管理备份集群',
        local: 'admin-00067.html',
        children: [
          {
            id: 2411,
            parentId: 2357,
            name: '管理本地集群节点',
            local: 'zh-cn_topic_0000001839224397.html',
            children: [
              {
                id: 2413,
                parentId: 2411,
                name: '查看本地集群节点',
                local: 'zh-cn_topic_0000001839144469.html'
              },
              {
                id: 2414,
                parentId: 2411,
                name: '管理备节点/成员节点',
                local: 'zh-cn_topic_0000001792345350.html',
                children: [
                  {
                    id: 2415,
                    parentId: 2414,
                    name: '修改备节点/成员节点',
                    local: 'zh-cn_topic_0000001792345230.html'
                  },
                  {
                    id: 2416,
                    parentId: 2414,
                    name: '删除成员节点',
                    local: 'zh-cn_topic_0000001792345338.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2412,
            parentId: 2357,
            name: '管理多域集群',
            local: 'zh-cn_topic_0000001839224421.html',
            children: [
              {
                id: 2417,
                parentId: 2412,
                name: '查看集群信息',
                local: 'admin-00068.html'
              },
              {
                id: 2418,
                parentId: 2412,
                name: '添加外部集群',
                local: 'admin-00069.html'
              },
              {
                id: 2419,
                parentId: 2412,
                name: '修改外部集群信息',
                local: 'admin-00070.html'
              },
              {
                id: 2420,
                parentId: 2412,
                name: '删除外部集群',
                local: 'admin-00071.html'
              },
              {
                id: 2421,
                parentId: 2412,
                name: '指定外部集群为管理集群',
                local: 'admin-00072.html'
              },
              {
                id: 2422,
                parentId: 2412,
                name: '指定外部集群为被管理集群',
                local: 'admin-00073.html'
              },
              {
                id: 2423,
                parentId: 2412,
                name: '授权',
                local: 'admin-00074.html'
              },
              {
                id: 2424,
                parentId: 2412,
                name: '修改本地集群数据保护管理员的授权',
                local: 'admin-00075.html'
              },
              {
                id: 2425,
                parentId: 2412,
                name: '取消本地集群数据保护管理员的授权',
                local: 'admin-00076.html'
              }
            ]
          }
        ]
      },
      {
        id: 2358,
        parentId: 7,
        name: '管理复制集群',
        local: 'zh-cn_topic_0000001839144457.html',
        children: [
          {
            id: 2426,
            parentId: 2358,
            name: '添加外部集群',
            local: 'zh-cn_topic_0000001839224425.html'
          },
          {
            id: 2427,
            parentId: 2358,
            name: '查看集群信息',
            local: 'zh-cn_topic_0000001839144417.html'
          },
          {
            id: 2428,
            parentId: 2358,
            name: '修改复制集群',
            local: 'zh-cn_topic_0000001839224361.html'
          },
          {
            id: 2429,
            parentId: 2358,
            name: '删除复制集群',
            local: 'zh-cn_topic_0000001839144453.html'
          }
        ]
      },
      {
        id: 2359,
        parentId: 7,
        name: '管理本地存储',
        local: 'admin-00078.html',
        children: [
          {
            id: 2430,
            parentId: 2359,
            name: '查看本地存储信息',
            local: 'admin-00079.html'
          },
          {
            id: 2431,
            parentId: 2359,
            name: '配置本地存储容量告警阈值',
            local: 'admin-00080.html'
          },
          {
            id: 2432,
            parentId: 2359,
            name: '查看本地存储认证信息',
            local: 'admin-00081.html'
          },
          {
            id: 2433,
            parentId: 2359,
            name: '修改本地存储认证信息',
            local: 'admin-00082.html'
          },
          {
            id: 2434,
            parentId: 2359,
            name: '手动回收空间',
            local: 'admin-00087.html'
          }
        ]
      },
      {
        id: 2360,
        parentId: 7,
        name: '管理对象存储',
        local: 'helpcenter_000132.html',
        children: [
          {
            id: 2435,
            parentId: 2360,
            name: '添加归档存储',
            local: 'oracle_gud_000030.html'
          },
          {
            id: 2436,
            parentId: 2360,
            name: '导入归档存储副本',
            local: 'helpcenter_000134.html'
          },
          {
            id: 2437,
            parentId: 2360,
            name: '修改归档存储基本信息',
            local: 'helpcenter_000135.html'
          },
          {
            id: 2438,
            parentId: 2360,
            name: '修改归档存储容量告警阈值',
            local: 'helpcenter_000136.html'
          },
          {
            id: 2439,
            parentId: 2360,
            name: '查看归档存储信息',
            local: 'helpcenter_000137.html'
          },
          {
            id: 2440,
            parentId: 2360,
            name: '删除归档存储',
            local: 'helpcenter_000138.html'
          }
        ]
      },
      {
        id: 2361,
        parentId: 7,
        name: '管理磁带',
        local: 'helpcenter_000139.html',
        children: [
          {
            id: 2441,
            parentId: 2361,
            name: '管理磁带库',
            local: 'helpcenter_000140.html',
            children: [
              {
                id: 2443,
                parentId: 2441,
                name: '扫描磁带库',
                local: 'helpcenter_000141.html'
              },
              {
                id: 2444,
                parentId: 2441,
                name: '管理驱动',
                local: 'helpcenter_000142.html',
                children: [
                  {
                    id: 2446,
                    parentId: 2444,
                    name: '查看驱动',
                    local: 'helpcenter_000143.html'
                  },
                  {
                    id: 2447,
                    parentId: 2444,
                    name: '启用驱动',
                    local: 'helpcenter_000144.html'
                  },
                  {
                    id: 2448,
                    parentId: 2444,
                    name: '禁用驱动',
                    local: 'helpcenter_000145.html'
                  }
                ]
              },
              {
                id: 2445,
                parentId: 2441,
                name: '管理磁带',
                local: 'helpcenter_000146.html',
                children: [
                  {
                    id: 2449,
                    parentId: 2445,
                    name: '查看磁带',
                    local: 'helpcenter_000147.html'
                  },
                  {
                    id: 2450,
                    parentId: 2445,
                    name: '加载磁带',
                    local: 'helpcenter_000148.html'
                  },
                  {
                    id: 2451,
                    parentId: 2445,
                    name: '卸载磁带',
                    local: 'helpcenter_000149.html'
                  },
                  {
                    id: 2452,
                    parentId: 2445,
                    name: '删除磁带',
                    local: 'helpcenter_000150.html'
                  },
                  {
                    id: 2453,
                    parentId: 2445,
                    name: '识别磁带',
                    local: 'helpcenter_000151.html'
                  },
                  {
                    id: 2454,
                    parentId: 2445,
                    name: '标记磁带为空',
                    local: 'helpcenter_000152.html'
                  },
                  {
                    id: 2455,
                    parentId: 2445,
                    name: '擦除磁带',
                    local: 'helpcenter_000153.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2442,
            parentId: 2361,
            name: '管理介质集',
            local: 'helpcenter_000154.html',
            children: [
              {
                id: 2456,
                parentId: 2442,
                name: '创建介质集',
                local: 'helpcenter_000155.html'
              },
              {
                id: 2457,
                parentId: 2442,
                name: '查看介质集',
                local: 'helpcenter_000156.html'
              },
              {
                id: 2458,
                parentId: 2442,
                name: '修改介质集',
                local: 'helpcenter_000157.html'
              },
              {
                id: 2459,
                parentId: 2442,
                name: '删除介质集',
                local: 'helpcenter_000158.html'
              }
            ]
          }
        ]
      },
      {
        id: 2362,
        parentId: 7,
        name: '查看系统信息',
        local: 'informix_gud_00040.html',
        children: [
          {
            id: 2460,
            parentId: 2362,
            name: '查看系统版本信息',
            local: 'informix_gud_00041.html'
          },
          {
            id: 2461,
            parentId: 2362,
            name: '查看设备ESN',
            local: 'informix_gud_00042.html'
          }
        ]
      },
      {
        id: 2363,
        parentId: 7,
        name: '管理安全策略',
        local: 'helpcenter_000168.html'
      },
      {
        id: 2364,
        parentId: 7,
        name: '管理证书',
        local: 'helpcenter_000169.html',
        children: [
          {
            id: 2462,
            parentId: 2364,
            name: '查看证书信息',
            local: 'helpcenter_000170.html'
          },
          {
            id: 2463,
            parentId: 2364,
            name: '添加外部证书',
            local: 'helpcenter_000171.html'
          },
          {
            id: 2464,
            parentId: 2364,
            name: '导入证书',
            local: 'helpcenter_000172.html'
          },
          {
            id: 2465,
            parentId: 2364,
            name: '导出请求文件',
            local: 'helpcenter_000173.html'
          },
          {
            id: 2466,
            parentId: 2364,
            name: '修改证书过期告警',
            local: 'helpcenter_000174.html'
          },
          {
            id: 2467,
            parentId: 2364,
            name: '管理证书吊销列表',
            local: 'helpcenter_000176.html',
            children: [
              {
                id: 2470,
                parentId: 2467,
                name: '导入证书吊销列表',
                local: 'helpcenter_000177.html'
              },
              {
                id: 2471,
                parentId: 2467,
                name: '查看证书吊销列表',
                local: 'helpcenter_000178.html'
              },
              {
                id: 2472,
                parentId: 2467,
                name: '下载证书吊销列表',
                local: 'helpcenter_000179.html'
              },
              {
                id: 2473,
                parentId: 2467,
                name: '删除证书吊销列表',
                local: 'helpcenter_000180.html'
              }
            ]
          },
          {
            id: 2468,
            parentId: 2364,
            name: '下载证书',
            local: 'helpcenter_000181.html'
          },
          {
            id: 2469,
            parentId: 2364,
            name: '删除外部证书',
            local: 'helpcenter_000182.html'
          }
        ]
      },
      {
        id: 2365,
        parentId: 7,
        name: '管理主机受信',
        local: 'helpcenter_000183.html'
      },
      {
        id: 2366,
        parentId: 7,
        name: '管理日志',
        local: 'helpcenter_000184.html'
      },
      {
        id: 2367,
        parentId: 7,
        name: '导出查询',
        local: 'zh-cn_topic_0000001839144381.html'
      },
      {
        id: 2368,
        parentId: 7,
        name: '管理系统数据备份',
        local: 'helpcenter_000185.html',
        children: [
          {
            id: 2474,
            parentId: 2368,
            name: '配置管理数据备份',
            local: 'helpcenter_000186.html'
          },
          {
            id: 2475,
            parentId: 2368,
            name: '导出管理数据备份',
            local: 'helpcenter_000187.html'
          },
          {
            id: 2476,
            parentId: 2368,
            name: '删除管理数据备份',
            local: 'helpcenter_000188.html'
          },
          {
            id: 2477,
            parentId: 2368,
            name: '导入管理数据备份',
            local: 'helpcenter_000189.html'
          },
          {
            id: 2478,
            parentId: 2368,
            name: '恢复管理数据',
            local: 'helpcenter_000190.html'
          }
        ]
      },
      {
        id: 2369,
        parentId: 7,
        name: '管理邮件服务',
        local: 'helpcenter_000191.html'
      },
      {
        id: 2370,
        parentId: 7,
        name: '管理事件转储',
        local: 'helpcenter_000192.html'
      },
      {
        id: 2371,
        parentId: 7,
        name: '管理SNMP Trap通知',
        local: 'helpcenter_000193.html'
      },
      {
        id: 2372,
        parentId: 7,
        name: '管理SFTP服务（适用于1.5.0版本）',
        local: 'helpcenter_000194.html',
        children: [
          {
            id: 2479,
            parentId: 2372,
            name: '开启SFTP服务',
            local: 'helpcenter_000195.html'
          },
          {
            id: 2480,
            parentId: 2372,
            name: '查看SFTP服务',
            local: 'helpcenter_000196.html'
          },
          {
            id: 2481,
            parentId: 2372,
            name: '创建SFTP用户',
            local: 'helpcenter_000197.html'
          },
          {
            id: 2482,
            parentId: 2372,
            name: '修改SFTP用户密码',
            local: 'helpcenter_000198.html'
          },
          {
            id: 2483,
            parentId: 2372,
            name: '删除SFTP用户',
            local: 'helpcenter_000199.html'
          }
        ]
      },
      {
        id: 2373,
        parentId: 7,
        name: '管理SFTP服务（适用于1.6.0及后续版本）',
        local: 'admin-00261.html',
        children: [
          {
            id: 2484,
            parentId: 2373,
            name: '开启SFTP服务',
            local: 'admin-00262.html'
          },
          {
            id: 2485,
            parentId: 2373,
            name: '查看SFTP服务',
            local: 'admin-00263.html'
          },
          {
            id: 2486,
            parentId: 2373,
            name: '创建SFTP用户',
            local: 'admin-00264.html'
          },
          {
            id: 2487,
            parentId: 2373,
            name: '修改SFTP用户密码',
            local: 'admin-00265.html'
          },
          {
            id: 2488,
            parentId: 2373,
            name: '删除SFTP用户',
            local: 'admin-00266.html'
          }
        ]
      },
      {
        id: 2374,
        parentId: 7,
        name: '管理设备时间',
        local: 'helpcenter_000200.html'
      },
      {
        id: 2375,
        parentId: 7,
        name: '配置LDAP服务',
        local: 'zh-cn_topic_0000001839144385.html'
      },
      {
        id: 2376,
        parentId: 7,
        name: '管理Windows ADFS配置（适用于1.6.0及后续版本）',
        local: 'admin-0077.html'
      },
      {
        id: 2377,
        parentId: 7,
        name: '管理备份软件纳管（适用于1.6.0及后续版本）',
        local: 'zh-cn_topic_0000001938830850.html',
        children: [
          {
            id: 2489,
            parentId: 2377,
            name: '添加备份软件纳管',
            local: 'zh-cn_topic_0000001965949181.html'
          },
          {
            id: 2490,
            parentId: 2377,
            name: '修改备份软件纳管',
            local: 'zh-cn_topic_0000001966069409.html'
          },
          {
            id: 2491,
            parentId: 2377,
            name: '删除备份软件纳管',
            local: 'zh-cn_topic_0000001938990182.html'
          }
        ]
      }
    ]
  }
];
topLanguage = 'zh';
topMainPage = 'helpcenter_000001.html';

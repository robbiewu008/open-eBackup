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
        name: '客户端',
        local: 'helpcenter_000008.html',
        children: [
          {
            id: 20,
            parentId: 12,
            name: '安装客户端（自动推送方式，适用于1.6.0及后续版本）',
            local: 'protectagent_install_0017.html'
          },
          {
            id: 21,
            parentId: 12,
            name: '管理客户端软件包',
            local: 'protectagent_install_0028.html',
            children: [
              {
                id: 23,
                parentId: 21,
                name: '下载客户端软件包',
                local: 'protectagent_install_0030.html'
              }
            ]
          },
          {
            id: 22,
            parentId: 12,
            name: '管理客户端',
            local: 'protectagent_install_0031.html',
            children: [
              {
                id: 24,
                parentId: 22,
                name: '查看客户端信息',
                local: 'protectagent_install_0032.html'
              },
              {
                id: 25,
                parentId: 22,
                name: '管理客户端',
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
        local: 'zh-cn_topic_0000002164767482.html',
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
                local: 'oracle_gud_0009.html',
                children: [
                  {
                    id: 53,
                    parentId: 43,
                    name: '备份前准备',
                    local: 'oracle_gud_0012.html'
                  },
                  {
                    id: 54,
                    parentId: 43,
                    name: '备份Oracle数据库',
                    local: 'oracle_gud_0013.html',
                    children: [
                      {
                        id: 56,
                        parentId: 54,
                        name: '步骤1：检查并配置数据库环境',
                        local: 'oracle_gud_0015.html',
                        children: [
                          {
                            id: 65,
                            parentId: 56,
                            name: '检查并配置Oracle数据库的Open状态',
                            local: 'oracle_gud_0016.html'
                          },
                          {
                            id: 66,
                            parentId: 56,
                            name: '检查并配置Oracle数据库的归档模式',
                            local: 'oracle_gud_0017.html'
                          },
                          {
                            id: 67,
                            parentId: 56,
                            name: '检查快照控制文件的位置',
                            local: 'oracle_gud_0020.html'
                          },
                          {
                            id: 68,
                            parentId: 56,
                            name: '检查集群数据的存放位置',
                            local: 'oracle_gud_0021.html'
                          }
                        ]
                      },
                      {
                        id: 57,
                        parentId: 54,
                        name:
                          '步骤2：获取存储资源CA证书（适用于存储层快照备份）',
                        local: 'oracle_gud_0023.html'
                      },
                      {
                        id: 58,
                        parentId: 54,
                        name: '步骤3：注册Oracle数据库',
                        local: 'oracle_gud_0024.html',
                        children: [
                          {
                            id: 69,
                            parentId: 58,
                            name: '注册数据库（适用于集群部署形态）',
                            local: 'oracle_gud_0025.html'
                          },
                          {
                            id: 70,
                            parentId: 58,
                            name: '注册数据库（适用于单机部署形态）',
                            local: 'oracle_gud_0026.html'
                          },
                          {
                            id: 71,
                            parentId: 58,
                            name: '注册数据库（适用于主备部署形态）',
                            local: 'oracle_gud_0027.html'
                          }
                        ]
                      },
                      {
                        id: 59,
                        parentId: 54,
                        name: '步骤4：注册PDB集（适用于1.6.0及后续版本）',
                        local: 'oracle_gud_0028.html'
                      },
                      {
                        id: 60,
                        parentId: 54,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'oracle_gud_0029.html'
                      },
                      {
                        id: 61,
                        parentId: 54,
                        name:
                          '步骤6：（可选）开启备份链路加密开关（适用于部分型号）',
                        local: 'oracle_gud_0030.html'
                      },
                      {
                        id: 62,
                        parentId: 54,
                        name: '步骤7：创建备份SLA',
                        local: 'oracle_gud_0031.html'
                      },
                      {
                        id: 63,
                        parentId: 54,
                        name: '步骤8：开启BCT（适用于RMAN备份）',
                        local: 'oracle_gud_0032.html'
                      },
                      {
                        id: 64,
                        parentId: 54,
                        name: '步骤9：执行备份',
                        local: 'oracle_gud_0033.html'
                      }
                    ]
                  },
                  {
                    id: 55,
                    parentId: 43,
                    name: '（可选）同步Trap配置至Oracle主机',
                    local: 'oracle_gud_0034.html'
                  }
                ]
              },
              {
                id: 44,
                parentId: 26,
                name: '复制',
                local: 'oracle_gud_0035.html',
                children: [
                  {
                    id: 72,
                    parentId: 44,
                    name: '复制Oracle数据库副本',
                    local: 'oracle_gud_0039.html',
                    children: [
                      {
                        id: 73,
                        parentId: 72,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'oracle_gud_0040.html'
                      },
                      {
                        id: 74,
                        parentId: 72,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'oracle_gud_0041.html'
                      },
                      {
                        id: 75,
                        parentId: 72,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'oracle_gud_0042.html'
                      },
                      {
                        id: 76,
                        parentId: 72,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'oracle_gud_0043.html'
                      },
                      {
                        id: 77,
                        parentId: 72,
                        name: '步骤4：下载并导入证书',
                        local: 'oracle_gud_0044.html'
                      },
                      {
                        id: 78,
                        parentId: 72,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'oracle_gud_0045.html'
                      },
                      {
                        id: 79,
                        parentId: 72,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'oracle_gud_0046.html'
                      },
                      {
                        id: 80,
                        parentId: 72,
                        name: '步骤6：添加复制集群',
                        local: 'oracle_gud_0047.html'
                      },
                      {
                        id: 81,
                        parentId: 72,
                        name: '步骤7：创建复制SLA',
                        local: 'oracle_gud_0048.html'
                      },
                      {
                        id: 82,
                        parentId: 72,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'oracle_gud_0049.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 45,
                parentId: 26,
                name: '归档',
                local: 'oracle_gud_0069.html',
                children: [
                  {
                    id: 83,
                    parentId: 45,
                    name: '归档Oracle备份副本',
                    local: 'oracle_gud_0072.html',
                    children: [
                      {
                        id: 85,
                        parentId: 83,
                        name: '步骤1：添加归档存储',
                        local: 'oracle_gud_0073.html',
                        children: [
                          {
                            id: 87,
                            parentId: 85,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'oracle_gud_0074.html'
                          },
                          {
                            id: 88,
                            parentId: 85,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'oracle_gud_0075.html'
                          }
                        ]
                      },
                      {
                        id: 86,
                        parentId: 83,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'oracle_gud_0076.html'
                      }
                    ]
                  },
                  {
                    id: 84,
                    parentId: 45,
                    name: '归档Oracle复制副本',
                    local: 'oracle_gud_0077.html',
                    children: [
                      {
                        id: 89,
                        parentId: 84,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'oracle_gud_0078.html'
                      },
                      {
                        id: 90,
                        parentId: 84,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'oracle_gud_0079.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 46,
                parentId: 26,
                name: '恢复',
                local: 'oracle_gud_0080.html',
                children: [
                  {
                    id: 91,
                    parentId: 46,
                    name: '恢复Oracle数据库',
                    local: 'oracle_gud_0083.html'
                  },
                  {
                    id: 92,
                    parentId: 46,
                    name:
                      '恢复Oracle数据库中的单个表或多个表（适用于1.6.0及后续版本）',
                    local: 'oracle_gud_0084.html'
                  },
                  {
                    id: 93,
                    parentId: 46,
                    name:
                      '恢复Oracle数据库中的单个或多个文件（适用于1.6.0及后续版本）',
                    local: 'oracle_gud_0085.html'
                  },
                  {
                    id: 94,
                    parentId: 46,
                    name:
                      '恢复Oracle数据库中的CDB或多个PDB（适用于1.6.0及后续版本）',
                    local: 'oracle_gud_0086.html'
                  }
                ]
              },
              {
                id: 47,
                parentId: 26,
                name: '即时恢复（适用于部分型号）',
                local: 'oracle_gud_0087.html',
                children: [
                  {
                    id: 95,
                    parentId: 47,
                    name: '即时恢复Oracle数据库',
                    local: 'oracle_gud_0090.html'
                  }
                ]
              },
              {
                id: 48,
                parentId: 26,
                name: '全局搜索',
                local: 'oracle_gud_0100.html',
                children: [
                  {
                    id: 96,
                    parentId: 48,
                    name: '全局搜索资源',
                    local: 'oracle_gud_0101.html'
                  },
                  {
                    id: 97,
                    parentId: 48,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'oracle_gud_0102.html'
                  }
                ]
              },
              {
                id: 49,
                parentId: 26,
                name: 'SLA',
                local: 'oracle_gud_0105.html',
                children: [
                  {
                    id: 98,
                    parentId: 49,
                    name: '查看SLA信息',
                    local: 'oracle_gud_0107.html'
                  },
                  {
                    id: 99,
                    parentId: 49,
                    name: '管理SLA',
                    local: 'oracle_gud_0108.html'
                  }
                ]
              },
              {
                id: 50,
                parentId: 26,
                name: '副本',
                local: 'oracle_gud_0109.html',
                children: [
                  {
                    id: 100,
                    parentId: 50,
                    name: '查看Oracle副本信息',
                    local: 'oracle_gud_0110.html'
                  },
                  {
                    id: 101,
                    parentId: 50,
                    name: '管理Oracle副本',
                    local: 'oracle_gud_0111.html'
                  }
                ]
              },
              {
                id: 51,
                parentId: 26,
                name: 'Oracle数据库环境',
                local: 'oracle_gud_0112.html',
                children: [
                  {
                    id: 102,
                    parentId: 51,
                    name: '查看Oracle数据库环境信息',
                    local: 'oracle_gud_0113.html'
                  },
                  {
                    id: 103,
                    parentId: 51,
                    name: '管理数据库',
                    local: 'oracle_gud_0114.html'
                  },
                  {
                    id: 104,
                    parentId: 51,
                    name: '管理数据库集群',
                    local: 'oracle_gud_0115.html'
                  }
                ]
              },
              {
                id: 52,
                parentId: 26,
                name: '常见问题',
                local: 'oracle_gud_0119.html',
                children: [
                  {
                    id: 105,
                    parentId: 52,
                    name: '登录DeviceManager管理界面',
                    local: 'oracle_gud_0123.html'
                  },
                  {
                    id: 106,
                    parentId: 52,
                    name: '配置Oracle数据库侦听参数',
                    local: 'oracle_gud_0125.html'
                  },
                  {
                    id: 107,
                    parentId: 52,
                    name: '配置Udev（适用于存储层快照备份）',
                    local: 'oracle_gud_0126.html'
                  },
                  {
                    id: 108,
                    parentId: 52,
                    name: '设置Windows PowerShell权限（适用于存储层快照备份）',
                    local: 'oracle_gud_0127.html'
                  },
                  {
                    id: 109,
                    parentId: 52,
                    name: '恢复任务提示存在资源残留（适用于1.6.0及后续版本）',
                    local: 'oracle_gud_0129.html'
                  },
                  {
                    id: 110,
                    parentId: 52,
                    name: '如何禁用Oracle数据库的krb trace',
                    local: 'oracle_gud_0130.html'
                  },
                  {
                    id: 111,
                    parentId: 52,
                    name: '如何检查恢复的目标主机是否为HACS集群中的主机',
                    local: 'oracle_gud_0131.html'
                  },
                  {
                    id: 112,
                    parentId: 52,
                    name:
                      '执行Oracle归档日志备份时，备份任务提示“ORA-19571”错误',
                    local: 'oracle_gud_0132.html'
                  },
                  {
                    id: 113,
                    parentId: 52,
                    name:
                      'Oracle数据保护相关任务超过10分钟未更新任务进度如何处理',
                    local: 'oracle_gud_0134.html'
                  },
                  {
                    id: 114,
                    parentId: 52,
                    name: 'Oracle环境变量注册表被删除后如何处理',
                    local: 'oracle_gud_0135.html'
                  },
                  {
                    id: 115,
                    parentId: 52,
                    name: 'Oracle环境变量配置文件被删除后如何处理',
                    local: 'oracle_gud_0136.html'
                  },
                  {
                    id: 116,
                    parentId: 52,
                    name: '数据库闪回区满导致恢复任务失败',
                    local: 'oracle_gud_0137.html'
                  },
                  {
                    id: 117,
                    parentId: 52,
                    name: '备份任务长时间没有进度',
                    local: 'oracle_gud_0138.html'
                  },
                  {
                    id: 118,
                    parentId: 52,
                    name:
                      'Oracle数据库备份失败，错误详情包含“RMAN-06059: expected archived log not found, loss of archived log compromises recoverability”',
                    local: 'oracle_gud_0140.html'
                  },
                  {
                    id: 119,
                    parentId: 52,
                    name: '恢复Oracle集群，任务状态为“部分成功”时如何处理',
                    local: 'oracle_gud_0141.html'
                  },
                  {
                    id: 120,
                    parentId: 52,
                    name:
                      '恢复较大数据量的Oracle数据库时，恢复任务长时间没有进度',
                    local: 'oracle_gud_0142.html'
                  },
                  {
                    id: 121,
                    parentId: 52,
                    name:
                      '原机恢复/即时恢复任务失败，错误详情包含“ORA-01034: ORACLE not available”',
                    local: 'oracle_gud_0143.html'
                  },
                  {
                    id: 122,
                    parentId: 52,
                    name: '恢复任务提示数据库处于运行状态',
                    local: 'oracle_gud_0144.html'
                  },
                  {
                    id: 123,
                    parentId: 52,
                    name:
                      'Oracle ADG或RAC集群备份，切换数据库认证方式后，数据库运行异常宕机',
                    local: 'oracle_gud_0145.html'
                  },
                  {
                    id: 124,
                    parentId: 52,
                    name:
                      '执行Oracle恢复任务失败，错误详情包含“ORA-19698”错误码',
                    local: 'oracle_gud_0146.html'
                  },
                  {
                    id: 125,
                    parentId: 52,
                    name:
                      'Oracle 12.1集群ASM恢复到Oracle 12.1单机非ASM环境后，备份Oracle 12.1单机非ASM时会卡住',
                    local: 'oracle_gud_0147.html'
                  },
                  {
                    id: 126,
                    parentId: 52,
                    name:
                      '执行Oracle 19C备份任务失败，错误详情包含“ORA-65250”错误码',
                    local: 'oracle_gud_0148.html'
                  },
                  {
                    id: 127,
                    parentId: 52,
                    name: 'Oracle数据库备份失败，错误详情包含“RMAN-06062”',
                    local: 'oracle_gud_0149.html'
                  },
                  {
                    id: 128,
                    parentId: 52,
                    name:
                      '存储层快照全量备份任务失败，错误详情包含“创建LUN快照失败”',
                    local: 'oracle_gud_0150.html'
                  }
                ]
              }
            ]
          },
          {
            id: 27,
            parentId: 13,
            name: 'MySQL/MariaDB/GreatSQL数据保护',
            local: 'zh-cn_topic_0000002200008561.html',
            children: [
              {
                id: 129,
                parentId: 27,
                name: '备份',
                local: 'mysql-0008.html',
                children: [
                  {
                    id: 138,
                    parentId: 129,
                    name: '备份前准备',
                    local: 'mysql-0011.html'
                  },
                  {
                    id: 139,
                    parentId: 129,
                    name: '备份MySQL/MariaDB/GreatSQL数据库',
                    local: 'mysql-0012.html',
                    children: [
                      {
                        id: 140,
                        parentId: 139,
                        name: '步骤1：开启MySQL/MariaDB/GreatSQL数据库权限',
                        local: 'mysql-0013.html'
                      },
                      {
                        id: 141,
                        parentId: 139,
                        name: '步骤2：手动配置软连接',
                        local: 'mysql-0014.html'
                      },
                      {
                        id: 142,
                        parentId: 139,
                        name: '步骤3：手动安装备份工具',
                        local: 'mysql-0015.html',
                        children: [
                          {
                            id: 149,
                            parentId: 142,
                            name: '安装Mariabackup（适用于MariaDB）',
                            local: 'mysql-0016.html'
                          }
                        ]
                      },
                      {
                        id: 143,
                        parentId: 139,
                        name: '步骤4：MySQL/MariaDB/GreatSQL数据库开启日志模式',
                        local: 'mysql-0018.html'
                      },
                      {
                        id: 144,
                        parentId: 139,
                        name: '步骤5：注册MySQL/MariaDB/GreatSQL数据库',
                        local: 'mysql-0019.html'
                      },
                      {
                        id: 145,
                        parentId: 139,
                        name: '步骤6：（可选）创建限速策略',
                        local: 'mysql-0020.html'
                      },
                      {
                        id: 146,
                        parentId: 139,
                        name: '步骤7：（可选）开启备份链路加密开关',
                        local: 'mysql-0021.html'
                      },
                      {
                        id: 147,
                        parentId: 139,
                        name: '步骤8：创建备份SLA',
                        local: 'mysql-0022.html'
                      },
                      {
                        id: 148,
                        parentId: 139,
                        name: '步骤9：执行备份',
                        local: 'mysql-0023.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 130,
                parentId: 27,
                name: '复制',
                local: 'mysql-0026.html',
                children: [
                  {
                    id: 150,
                    parentId: 130,
                    name: '复制MySQL/MariaDB/GreatSQL副本',
                    local: 'mysql-0030.html',
                    children: [
                      {
                        id: 151,
                        parentId: 150,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'mysql-0031.html'
                      },
                      {
                        id: 152,
                        parentId: 150,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'mysql-0032.html'
                      },
                      {
                        id: 153,
                        parentId: 150,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'mysql-0033.html'
                      },
                      {
                        id: 154,
                        parentId: 150,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'mysql-0034.html'
                      },
                      {
                        id: 155,
                        parentId: 150,
                        name: '步骤4：下载并导入证书',
                        local: 'mysql-0035.html'
                      },
                      {
                        id: 156,
                        parentId: 150,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'mysql-0036.html'
                      },
                      {
                        id: 157,
                        parentId: 150,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'mysql-0037.html'
                      },
                      {
                        id: 158,
                        parentId: 150,
                        name: '步骤6：添加复制集群',
                        local: 'mysql-0038.html'
                      },
                      {
                        id: 159,
                        parentId: 150,
                        name: '步骤7：创建复制SLA',
                        local: 'mysql-0039.html'
                      },
                      {
                        id: 160,
                        parentId: 150,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'mysql-0040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 131,
                parentId: 27,
                name: '归档',
                local: 'mysql-0060.html',
                children: [
                  {
                    id: 161,
                    parentId: 131,
                    name: '归档MySQL/MariaDB/GreatSQL备份副本',
                    local: 'mysql-0063.html',
                    children: [
                      {
                        id: 163,
                        parentId: 161,
                        name: '步骤1：添加归档存储',
                        local: 'mysql-0064.html',
                        children: [
                          {
                            id: 165,
                            parentId: 163,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'mysql-0065.html'
                          },
                          {
                            id: 166,
                            parentId: 163,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'mysql-0066.html'
                          }
                        ]
                      },
                      {
                        id: 164,
                        parentId: 161,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'mysql-0067.html'
                      }
                    ]
                  },
                  {
                    id: 162,
                    parentId: 131,
                    name: '归档MySQL/MariaDB/GreatSQL复制副本',
                    local: 'mysql-0068.html',
                    children: [
                      {
                        id: 167,
                        parentId: 162,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'mysql-0069.html'
                      },
                      {
                        id: 168,
                        parentId: 162,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'mysql-0070.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 132,
                parentId: 27,
                name: '恢复',
                local: 'mysql-0071.html',
                children: [
                  {
                    id: 169,
                    parentId: 132,
                    name: '恢复MySQL/MariaDB/GreatSQL数据库',
                    local: 'mysql-0074.html'
                  }
                ]
              },
              {
                id: 133,
                parentId: 27,
                name: '全局搜索',
                local: 'mysql-0088.html',
                children: [
                  {
                    id: 170,
                    parentId: 133,
                    name: '全局搜索资源',
                    local: 'mysql-0089.html'
                  },
                  {
                    id: 171,
                    parentId: 133,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'mysql-0090.html'
                  }
                ]
              },
              {
                id: 134,
                parentId: 27,
                name: 'SLA',
                local: 'mysql-0093.html',
                children: [
                  {
                    id: 172,
                    parentId: 134,
                    name: '关于SLA',
                    local: 'mysql-0094.html'
                  },
                  {
                    id: 173,
                    parentId: 134,
                    name: '查看SLA信息',
                    local: 'mysql-0095.html'
                  },
                  {
                    id: 174,
                    parentId: 134,
                    name: '管理SLA',
                    local: 'mysql-0096.html'
                  }
                ]
              },
              {
                id: 135,
                parentId: 27,
                name: '副本',
                local: 'mysql-0097.html',
                children: [
                  {
                    id: 175,
                    parentId: 135,
                    name: '查看MySQL/MariaDB/GreatSQL副本信息',
                    local: 'mysql-0098.html'
                  },
                  {
                    id: 176,
                    parentId: 135,
                    name: '管理MySQL/MariaDB/GreatSQL副本',
                    local: 'mysql-0099.html'
                  }
                ]
              },
              {
                id: 136,
                parentId: 27,
                name: 'MySQL/MariaDB/GreatSQL数据库环境',
                local: 'mysql-0100.html',
                children: [
                  {
                    id: 177,
                    parentId: 136,
                    name: '查看MySQL/MariaDB/GreatSQL数据库环境信息',
                    local: 'mysql-0101.html'
                  },
                  {
                    id: 178,
                    parentId: 136,
                    name: '管理数据库',
                    local: 'mysql-0102.html'
                  },
                  {
                    id: 179,
                    parentId: 136,
                    name: '管理数据库实例',
                    local: 'mysql-0103.html'
                  },
                  {
                    id: 180,
                    parentId: 136,
                    name: '管理数据库集群',
                    local: 'mysql-0104.html'
                  }
                ]
              },
              {
                id: 137,
                parentId: 27,
                name: '常见问题',
                local: 'mysql-0105.html',
                children: [
                  {
                    id: 181,
                    parentId: 137,
                    name:
                      '无法在MySQL/GreatSQL数据库主机/usr/bin目录下找到mysql、mysqlbinlog、mysqldump和mysqladmin工具',
                    local: 'mysql-0110.html'
                  },
                  {
                    id: 182,
                    parentId: 137,
                    name: 'XtraBackup工具无法运行，备份失败',
                    local: 'mysql-0111.html'
                  },
                  {
                    id: 183,
                    parentId: 137,
                    name:
                      '备份失败，上报“Percona XtraBackup不支持INSTANT ADD/DROP COLUMNS特性”报错',
                    local: 'mysql-0112.html'
                  }
                ]
              }
            ]
          },
          {
            id: 28,
            parentId: 13,
            name: 'SQL Server数据保护',
            local: 'zh-cn_topic_0000002164607798.html',
            children: [
              {
                id: 184,
                parentId: 28,
                name: '备份',
                local: 'sql-0008.html',
                children: [
                  {
                    id: 192,
                    parentId: 184,
                    name: '备份前准备',
                    local: 'sql-0011.html'
                  },
                  {
                    id: 193,
                    parentId: 184,
                    name: '备份SQL Server数据库',
                    local: 'sql-0012.html',
                    children: [
                      {
                        id: 194,
                        parentId: 193,
                        name: '步骤1：检查并配置数据库环境',
                        local: 'sql-0013.html'
                      },
                      {
                        id: 195,
                        parentId: 193,
                        name: '步骤2：设置Windows PowerShell权限',
                        local: 'sql-0014.html'
                      },
                      {
                        id: 196,
                        parentId: 193,
                        name: '步骤3：开启sysadmin权限',
                        local: 'sql-0015.html'
                      },
                      {
                        id: 197,
                        parentId: 193,
                        name: '步骤4：设置日志备份恢复模式',
                        local: 'sql-0016.html'
                      },
                      {
                        id: 198,
                        parentId: 193,
                        name: '步骤5：注册SQL Server数据库',
                        local: 'sql-0017.html'
                      },
                      {
                        id: 199,
                        parentId: 193,
                        name: '步骤6：（可选）创建限速策略',
                        local: 'sql-0018.html'
                      },
                      {
                        id: 200,
                        parentId: 193,
                        name: '步骤7：（可选）开启备份链路加密开关',
                        local: 'sql-0019.html'
                      },
                      {
                        id: 201,
                        parentId: 193,
                        name: '步骤8：创建备份SLA',
                        local: 'sql-0020.html'
                      },
                      {
                        id: 202,
                        parentId: 193,
                        name: '步骤9：执行备份',
                        local: 'sql-0021.html',
                        children: [
                          {
                            id: 203,
                            parentId: 202,
                            name: '执行实例级备份',
                            local: 'sql-0022.html'
                          },
                          {
                            id: 204,
                            parentId: 202,
                            name: '执行可用性组备份',
                            local: 'sql-0023.html'
                          },
                          {
                            id: 205,
                            parentId: 202,
                            name: '执行数据库级备份',
                            local: 'sql-0024.html'
                          }
                        ]
                      }
                    ]
                  }
                ]
              },
              {
                id: 185,
                parentId: 28,
                name: '复制',
                local: 'sql-0025.html',
                children: [
                  {
                    id: 206,
                    parentId: 185,
                    name: '复制SQL Server数据库副本',
                    local: 'sql-0028.html',
                    children: [
                      {
                        id: 207,
                        parentId: 206,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'sql-0029.html'
                      },
                      {
                        id: 208,
                        parentId: 206,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'sql-0030.html'
                      },
                      {
                        id: 209,
                        parentId: 206,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'sql-0031.html'
                      },
                      {
                        id: 210,
                        parentId: 206,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'sql-0032.html'
                      },
                      {
                        id: 211,
                        parentId: 206,
                        name: '步骤4：下载并导入证书',
                        local: 'sql-0033.html'
                      },
                      {
                        id: 212,
                        parentId: 206,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'sql-0034.html'
                      },
                      {
                        id: 213,
                        parentId: 206,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'sql-0035.html'
                      },
                      {
                        id: 214,
                        parentId: 206,
                        name: '步骤6：添加复制集群',
                        local: 'sql-0036.html'
                      },
                      {
                        id: 215,
                        parentId: 206,
                        name: '步骤7：创建复制SLA',
                        local: 'sql-0037.html'
                      },
                      {
                        id: 216,
                        parentId: 206,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'sql-0038.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 186,
                parentId: 28,
                name: '归档',
                local: 'sql-0058.html',
                children: [
                  {
                    id: 217,
                    parentId: 186,
                    name: '归档SQL Server备份副本',
                    local: 'sql-0061.html',
                    children: [
                      {
                        id: 219,
                        parentId: 217,
                        name: '步骤1：添加归档存储',
                        local: 'sql-0062.html',
                        children: [
                          {
                            id: 221,
                            parentId: 219,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'sql-0063.html'
                          },
                          {
                            id: 222,
                            parentId: 219,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'sql-0064.html'
                          }
                        ]
                      },
                      {
                        id: 220,
                        parentId: 217,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'sql-0065.html'
                      }
                    ]
                  },
                  {
                    id: 218,
                    parentId: 186,
                    name: '归档SQL Server复制副本',
                    local: 'sql-0066.html',
                    children: [
                      {
                        id: 223,
                        parentId: 218,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'sql-0067.html'
                      },
                      {
                        id: 224,
                        parentId: 218,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'sql-0068.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 187,
                parentId: 28,
                name: '恢复',
                local: 'sql-0069.html',
                children: [
                  {
                    id: 225,
                    parentId: 187,
                    name: '恢复SQL Server数据库',
                    local: 'sql-0072.html'
                  },
                  {
                    id: 226,
                    parentId: 187,
                    name: '恢复SQL Server实例或可用性组中的单个或多个数据库',
                    local: 'sql-0073.html'
                  }
                ]
              },
              {
                id: 188,
                parentId: 28,
                name: '全局搜索',
                local: 'sql-0074.html',
                children: [
                  {
                    id: 227,
                    parentId: 188,
                    name: '全局搜索资源',
                    local: 'sql-0075.html'
                  },
                  {
                    id: 228,
                    parentId: 188,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'sql-0076.html'
                  }
                ]
              },
              {
                id: 189,
                parentId: 28,
                name: 'SLA',
                local: 'sql-0080.html',
                children: [
                  {
                    id: 229,
                    parentId: 189,
                    name: '查看SLA信息',
                    local: 'sql-0082.html'
                  },
                  {
                    id: 230,
                    parentId: 189,
                    name: '管理SLA',
                    local: 'sql-0083.html'
                  }
                ]
              },
              {
                id: 190,
                parentId: 28,
                name: '副本',
                local: 'sql-0084.html',
                children: [
                  {
                    id: 231,
                    parentId: 190,
                    name: '查看SQL Server副本信息',
                    local: 'sql-0085.html'
                  },
                  {
                    id: 232,
                    parentId: 190,
                    name: '管理SQL Server副本',
                    local: 'sql-0086.html'
                  }
                ]
              },
              {
                id: 191,
                parentId: 28,
                name: 'SQL Server数据库环境',
                local: 'sql-0087.html',
                children: [
                  {
                    id: 233,
                    parentId: 191,
                    name: '查看SQL Server数据库环境信息',
                    local: 'sql-0088.html'
                  },
                  {
                    id: 234,
                    parentId: 191,
                    name: '管理SQL Server',
                    local: 'sql-0089.html'
                  },
                  {
                    id: 235,
                    parentId: 191,
                    name: '管理SQL Server数据库集群',
                    local: 'sql-0090.html'
                  }
                ]
              }
            ]
          },
          {
            id: 29,
            parentId: 13,
            name: 'PostgreSQL数据保护',
            local: 'zh-cn_topic_0000002200094145.html',
            children: [
              {
                id: 236,
                parentId: 29,
                name: '备份',
                local: 'postgresql-0005.html',
                children: [
                  {
                    id: 245,
                    parentId: 236,
                    name: '备份前准备',
                    local: 'postgresql-0008.html'
                  },
                  {
                    id: 246,
                    parentId: 236,
                    name: '备份PostgreSQL',
                    local: 'postgresql-0009.html',
                    children: [
                      {
                        id: 247,
                        parentId: 246,
                        name:
                          '步骤1：检查并开启PostgreSQL数据库安装用户sudo权限',
                        local: 'postgresql-0080.html'
                      },
                      {
                        id: 248,
                        parentId: 246,
                        name: '步骤2：开启归档模式',
                        local: 'postgresql-0010_0.html'
                      },
                      {
                        id: 249,
                        parentId: 246,
                        name: '步骤3：注册PostgreSQL单实例下的数据库',
                        local: 'postgresql-0011.html'
                      },
                      {
                        id: 250,
                        parentId: 246,
                        name: '步骤4：注册PostgreSQL集群实例下的数据库',
                        local: 'postgresql-0012.html'
                      },
                      {
                        id: 251,
                        parentId: 246,
                        name: '（可选）创建限速策略',
                        local: 'postgresql-0013.html'
                      },
                      {
                        id: 252,
                        parentId: 246,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'postgresql-0014.html'
                      },
                      {
                        id: 253,
                        parentId: 246,
                        name: '步骤7：创建备份SLA',
                        local: 'postgresql-0015.html'
                      },
                      {
                        id: 254,
                        parentId: 246,
                        name: '步骤8：执行备份',
                        local: 'postgresql-0016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 237,
                parentId: 29,
                name: '复制',
                local: 'oracle_gud_000035_2.html',
                children: [
                  {
                    id: 255,
                    parentId: 237,
                    name: '复制PostgreSQL数据库副本',
                    local: 'postgresql_9999_1.html',
                    children: [
                      {
                        id: 256,
                        parentId: 255,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'postgresql_9999_2.html'
                      },
                      {
                        id: 257,
                        parentId: 255,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'postgresql_9999_3.html'
                      },
                      {
                        id: 258,
                        parentId: 255,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'postgresql_9999_4.html'
                      },
                      {
                        id: 259,
                        parentId: 255,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'postgresql_9999_5.html'
                      },
                      {
                        id: 260,
                        parentId: 255,
                        name: '步骤4：下载并导入证书',
                        local: 'postgresql_9999_6.html'
                      },
                      {
                        id: 261,
                        parentId: 255,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'postgresql_9999_7.html'
                      },
                      {
                        id: 262,
                        parentId: 255,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'postgresql_9999_8.html'
                      },
                      {
                        id: 263,
                        parentId: 255,
                        name: '步骤6：添加复制集群',
                        local: 'postgresql_9999_9.html'
                      },
                      {
                        id: 264,
                        parentId: 255,
                        name: '步骤7：创建复制SLA',
                        local: 'postgresql_9999_10.html'
                      },
                      {
                        id: 265,
                        parentId: 255,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'postgresql_9999_11.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 238,
                parentId: 29,
                name: '归档',
                local: 'postgresql-0031.html',
                children: [
                  {
                    id: 266,
                    parentId: 238,
                    name: '归档PostgreSQL备份副本',
                    local: 'postgresql-0034.html',
                    children: [
                      {
                        id: 268,
                        parentId: 266,
                        name: '步骤1：添加归档存储',
                        local: 'postgresql-0035.html',
                        children: [
                          {
                            id: 270,
                            parentId: 268,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'postgresql-0036.html'
                          },
                          {
                            id: 271,
                            parentId: 268,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'postgresql-0037.html'
                          }
                        ]
                      },
                      {
                        id: 269,
                        parentId: 266,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'postgresql-0038.html'
                      }
                    ]
                  },
                  {
                    id: 267,
                    parentId: 238,
                    name: '归档PostgreSQL复制副本',
                    local: 'postgresql-0039.html',
                    children: [
                      {
                        id: 272,
                        parentId: 267,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'postgresql-0040.html'
                      },
                      {
                        id: 273,
                        parentId: 267,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'postgresql-0041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 239,
                parentId: 29,
                name: '恢复',
                local: 'postgresql-0042.html',
                children: [
                  {
                    id: 274,
                    parentId: 239,
                    name: '恢复PostgreSQL',
                    local: 'postgresql-0045.html'
                  }
                ]
              },
              {
                id: 240,
                parentId: 29,
                name: '全局搜索',
                local: 'postgresql-0027_a2.html',
                children: [
                  {
                    id: 275,
                    parentId: 240,
                    name: '关于全局搜索',
                    local: 'postgresql-0027_a5.html'
                  },
                  {
                    id: 276,
                    parentId: 240,
                    name: '全局搜索资源',
                    local: 'postgresql-0027_a3.html'
                  },
                  {
                    id: 277,
                    parentId: 240,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'postgresql-0027_a4.html'
                  }
                ]
              },
              {
                id: 241,
                parentId: 29,
                name: 'SLA',
                local: 'postgresql-0050.html',
                children: [
                  {
                    id: 278,
                    parentId: 241,
                    name: '关于SLA',
                    local: 'postgresql-0051.html'
                  },
                  {
                    id: 279,
                    parentId: 241,
                    name: '查看SLA信息',
                    local: 'postgresql-0052.html'
                  },
                  {
                    id: 280,
                    parentId: 241,
                    name: '管理SLA',
                    local: 'postgresql-0053.html'
                  }
                ]
              },
              {
                id: 242,
                parentId: 29,
                name: '副本',
                local: 'postgresql-0054.html',
                children: [
                  {
                    id: 281,
                    parentId: 242,
                    name: '查看PostgreSQL副本信息',
                    local: 'postgresql-0055.html'
                  },
                  {
                    id: 282,
                    parentId: 242,
                    name: '管理PostgreSQL副本',
                    local: 'postgresql-0056.html'
                  }
                ]
              },
              {
                id: 243,
                parentId: 29,
                name: 'PostgreSQL集群环境',
                local: 'postgresql-0057.html',
                children: [
                  {
                    id: 283,
                    parentId: 243,
                    name: '查看PostgreSQL环境信息',
                    local: 'postgresql-0058.html'
                  },
                  {
                    id: 284,
                    parentId: 243,
                    name: '管理PostgreSQL',
                    local: 'postgresql-0059.html'
                  },
                  {
                    id: 285,
                    parentId: 243,
                    name: '管理PostgreSQL数据库集群',
                    local: 'postgresql-0060.html'
                  }
                ]
              },
              {
                id: 244,
                parentId: 29,
                name: '常见问题',
                local: 'postgresql-0061.html',
                children: [
                  {
                    id: 286,
                    parentId: 244,
                    name: '登录DeviceManager管理界面',
                    local: 'postgresql-0077.html'
                  },
                  {
                    id: 287,
                    parentId: 244,
                    name: '手动清理归档日志',
                    local: 'postgresql-00100.html'
                  },
                  {
                    id: 288,
                    parentId: 244,
                    name:
                      '对Patroni集群执行按时间点做日志副本恢复时，恢复任务执行成功但恢复后的数据与指定时间点的数据存在不一致',
                    local: 'zh-cn_topic_0000002199970229.html'
                  }
                ]
              }
            ]
          },
          {
            id: 30,
            parentId: 13,
            name: 'DB2数据保护',
            local: 'zh-cn_topic_0000002164767470.html',
            children: [
              {
                id: 289,
                parentId: 30,
                name: '备份',
                local: 'DB2-0009.html',
                children: [
                  {
                    id: 298,
                    parentId: 289,
                    name: '备份前准备',
                    local: 'DB2-0012.html'
                  },
                  {
                    id: 299,
                    parentId: 289,
                    name: '备份DB2数据库/表空间集',
                    local: 'DB2-0013.html',
                    children: [
                      {
                        id: 300,
                        parentId: 299,
                        name: '步骤1：开启归档模式与增量模式',
                        local: 'DB2-0014.html'
                      },
                      {
                        id: 301,
                        parentId: 299,
                        name: '步骤2：注册DB2数据库',
                        local: 'DB2-0015_a1.html',
                        children: [
                          {
                            id: 307,
                            parentId: 301,
                            name: '注册数据库（适用于单实例部署形态）',
                            local: 'DB2-0015_a2.html'
                          },
                          {
                            id: 308,
                            parentId: 301,
                            name: '注册数据库（适用于集群部署形态）',
                            local: 'DB2-0015_a3.html'
                          },
                          {
                            id: 309,
                            parentId: 301,
                            name: '注册数据库（适用于集群实例部署形态）',
                            local: 'DB2-0015_a4.html'
                          }
                        ]
                      },
                      {
                        id: 302,
                        parentId: 299,
                        name: '步骤3：创建DB2表空间集',
                        local: 'DB2-0016.html'
                      },
                      {
                        id: 303,
                        parentId: 299,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'DB2-0017.html'
                      },
                      {
                        id: 304,
                        parentId: 299,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'DB2-0018.html'
                      },
                      {
                        id: 305,
                        parentId: 299,
                        name: '步骤6：创建备份SLA',
                        local: 'DB2-0019.html'
                      },
                      {
                        id: 306,
                        parentId: 299,
                        name: '步骤7：执行备份',
                        local: 'DB2-0020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 290,
                parentId: 30,
                name: '复制',
                local: 'DB2-0023.html',
                children: [
                  {
                    id: 310,
                    parentId: 290,
                    name: '复制DB2副本',
                    local: 'DB2-0027.html',
                    children: [
                      {
                        id: 311,
                        parentId: 310,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'DB2-0028.html'
                      },
                      {
                        id: 312,
                        parentId: 310,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'DB2-0029.html'
                      },
                      {
                        id: 313,
                        parentId: 310,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'DB2-0030.html'
                      },
                      {
                        id: 314,
                        parentId: 310,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'DB2-0031.html'
                      },
                      {
                        id: 315,
                        parentId: 310,
                        name: '步骤4：下载并导入证书',
                        local: 'DB2-0032.html'
                      },
                      {
                        id: 316,
                        parentId: 310,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'DB2-0033.html'
                      },
                      {
                        id: 317,
                        parentId: 310,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'DB2-0034.html'
                      },
                      {
                        id: 318,
                        parentId: 310,
                        name: '步骤6：添加复制集群',
                        local: 'DB2-0035.html'
                      },
                      {
                        id: 319,
                        parentId: 310,
                        name: '步骤7：创建复制SLA',
                        local: 'DB2-0036.html'
                      },
                      {
                        id: 320,
                        parentId: 310,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'DB2-0037.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 291,
                parentId: 30,
                name: '归档',
                local: 'DB2-0069.html',
                children: [
                  {
                    id: 321,
                    parentId: 291,
                    name: '归档DB2备份副本',
                    local: 'DB2-0072.html',
                    children: [
                      {
                        id: 323,
                        parentId: 321,
                        name: '步骤1：添加归档存储',
                        local: 'DB2-0073.html',
                        children: [
                          {
                            id: 325,
                            parentId: 323,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'DB2-0074.html'
                          },
                          {
                            id: 326,
                            parentId: 323,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'DB2-0075.html'
                          }
                        ]
                      },
                      {
                        id: 324,
                        parentId: 321,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'DB2-0076.html'
                      }
                    ]
                  },
                  {
                    id: 322,
                    parentId: 291,
                    name: '归档DB2复制副本',
                    local: 'DB2-0077.html',
                    children: [
                      {
                        id: 327,
                        parentId: 322,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'DB2-0078.html'
                      },
                      {
                        id: 328,
                        parentId: 322,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'DB2-0079.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 292,
                parentId: 30,
                name: '恢复',
                local: 'DB2-0080.html',
                children: [
                  {
                    id: 329,
                    parentId: 292,
                    name: '恢复DB2数据库/表空间集',
                    local: 'DB2-0083.html'
                  }
                ]
              },
              {
                id: 293,
                parentId: 30,
                name: '全局搜索',
                local: 'DB2-0084.html',
                children: [
                  {
                    id: 330,
                    parentId: 293,
                    name: '全局搜索资源',
                    local: 'DB2-0085.html'
                  },
                  {
                    id: 331,
                    parentId: 293,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'DB2-0086.html'
                  }
                ]
              },
              {
                id: 294,
                parentId: 30,
                name: 'SLA',
                local: 'DB2-0090.html',
                children: [
                  {
                    id: 332,
                    parentId: 294,
                    name: '关于SLA',
                    local: 'DB2-0091.html'
                  },
                  {
                    id: 333,
                    parentId: 294,
                    name: '查看SLA信息',
                    local: 'DB2-0092.html'
                  },
                  {
                    id: 334,
                    parentId: 294,
                    name: '管理SLA',
                    local: 'DB2-0093.html'
                  }
                ]
              },
              {
                id: 295,
                parentId: 30,
                name: '副本',
                local: 'DB2-0094.html',
                children: [
                  {
                    id: 335,
                    parentId: 295,
                    name: '查看DB2副本信息',
                    local: 'DB2-0095.html'
                  },
                  {
                    id: 336,
                    parentId: 295,
                    name: '管理DB2副本',
                    local: 'DB2-0096.html'
                  }
                ]
              },
              {
                id: 296,
                parentId: 30,
                name: 'DB2集群环境',
                local: 'DB2-0097.html',
                children: [
                  {
                    id: 337,
                    parentId: 296,
                    name: '查询DB2信息',
                    local: 'DB2-0098.html'
                  },
                  {
                    id: 338,
                    parentId: 296,
                    name: '管理DB2集群',
                    local: 'DB2-0099.html'
                  },
                  {
                    id: 339,
                    parentId: 296,
                    name: '管理DB2数据库/表空间集/实例',
                    local: 'DB2-0100.html'
                  }
                ]
              },
              {
                id: 297,
                parentId: 30,
                name: '常见问题',
                local: 'DB2-0101.html',
                children: [
                  {
                    id: 340,
                    parentId: 297,
                    name: '登录DeviceManager管理界面',
                    local: 'DB2-0103.html'
                  }
                ]
              }
            ]
          },
          {
            id: 31,
            parentId: 13,
            name: 'Informix/GBase 8s数据保护',
            local: 'zh-cn_topic_0000002200008581.html',
            children: [
              {
                id: 341,
                parentId: 31,
                name: '备份',
                local: 'informix-0008.html',
                children: [
                  {
                    id: 350,
                    parentId: 341,
                    name: '备份Informix/GBase 8s',
                    local: 'informix-0011.html',
                    children: [
                      {
                        id: 351,
                        parentId: 350,
                        name: '步骤1：配置XBSA库路径',
                        local: 'informix-0012.html'
                      },
                      {
                        id: 352,
                        parentId: 350,
                        name: '步骤2：注册Informix/GBase 8s集群',
                        local: 'informix-0013.html'
                      },
                      {
                        id: 353,
                        parentId: 350,
                        name: '步骤3：注册Informix/GBase 8s实例',
                        local: 'informix-0014.html'
                      },
                      {
                        id: 354,
                        parentId: 350,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'informix-0015.html'
                      },
                      {
                        id: 355,
                        parentId: 350,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'informix-0016.html'
                      },
                      {
                        id: 356,
                        parentId: 350,
                        name: '步骤6：创建备份SLA',
                        local: 'informix-0017.html'
                      },
                      {
                        id: 357,
                        parentId: 350,
                        name: '步骤7：执行备份',
                        local: 'informix-0018.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 342,
                parentId: 31,
                name: '复制',
                local: 'informix-0021.html',
                children: [
                  {
                    id: 358,
                    parentId: 342,
                    name: '复制Informix/GBase 8s数据库副本',
                    local: 'informix-0024.html',
                    children: [
                      {
                        id: 359,
                        parentId: 358,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'informix-0025.html'
                      },
                      {
                        id: 360,
                        parentId: 358,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'informix-0026.html'
                      },
                      {
                        id: 361,
                        parentId: 358,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'informix-0027.html'
                      },
                      {
                        id: 362,
                        parentId: 358,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'informix-0028.html'
                      },
                      {
                        id: 363,
                        parentId: 358,
                        name: '步骤4：下载并导入证书',
                        local: 'informix-0029.html'
                      },
                      {
                        id: 364,
                        parentId: 358,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'informix-0030.html'
                      },
                      {
                        id: 365,
                        parentId: 358,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'informix-0031.html'
                      },
                      {
                        id: 366,
                        parentId: 358,
                        name: '步骤6：添加复制集群',
                        local: 'informix-0032.html'
                      },
                      {
                        id: 367,
                        parentId: 358,
                        name: '步骤7：创建复制SLA',
                        local: 'informix-0033.html'
                      },
                      {
                        id: 368,
                        parentId: 358,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'informix-0034.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 343,
                parentId: 31,
                name: '归档',
                local: 'informix-0054.html',
                children: [
                  {
                    id: 369,
                    parentId: 343,
                    name: '归档Informix/GBase 8s备份副本',
                    local: 'informix-0057.html',
                    children: [
                      {
                        id: 371,
                        parentId: 369,
                        name: '步骤1：添加归档存储',
                        local: 'informix-0058.html',
                        children: [
                          {
                            id: 373,
                            parentId: 371,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'informix-0059.html'
                          },
                          {
                            id: 374,
                            parentId: 371,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'informix-0060.html'
                          }
                        ]
                      },
                      {
                        id: 372,
                        parentId: 369,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'informix-0061.html'
                      }
                    ]
                  },
                  {
                    id: 370,
                    parentId: 343,
                    name: '归档Informix/GBase 8s复制副本',
                    local: 'informix-0062.html',
                    children: [
                      {
                        id: 375,
                        parentId: 370,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'informix-0063.html'
                      },
                      {
                        id: 376,
                        parentId: 370,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'informix-0064.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 344,
                parentId: 31,
                name: '恢复',
                local: 'informix-0065.html',
                children: [
                  {
                    id: 377,
                    parentId: 344,
                    name: '恢复Informix/GBase 8s',
                    local: 'informix-0068.html'
                  }
                ]
              },
              {
                id: 345,
                parentId: 31,
                name: '全局搜索',
                local: 'informix-0069.html',
                children: [
                  {
                    id: 378,
                    parentId: 345,
                    name: '全局搜索资源',
                    local: 'informix-0070.html'
                  },
                  {
                    id: 379,
                    parentId: 345,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'informix-0071.html'
                  }
                ]
              },
              {
                id: 346,
                parentId: 31,
                name: 'SLA',
                local: 'informix-0075.html',
                children: [
                  {
                    id: 380,
                    parentId: 346,
                    name: '关于SLA',
                    local: 'informix-0076.html'
                  },
                  {
                    id: 381,
                    parentId: 346,
                    name: '查看SLA信息',
                    local: 'informix-0077.html'
                  },
                  {
                    id: 382,
                    parentId: 346,
                    name: '管理SLA',
                    local: 'informix-0078.html'
                  }
                ]
              },
              {
                id: 347,
                parentId: 31,
                name: '副本',
                local: 'informix-0079.html',
                children: [
                  {
                    id: 383,
                    parentId: 347,
                    name: '查看Informix/GBase 8s副本信息',
                    local: 'informix-0080.html'
                  },
                  {
                    id: 384,
                    parentId: 347,
                    name: '管理Informix/GBase 8s副本',
                    local: 'informix-0081.html'
                  }
                ]
              },
              {
                id: 348,
                parentId: 31,
                name: 'Informix/GBase 8s集群环境',
                local: 'informix-0082.html',
                children: [
                  {
                    id: 385,
                    parentId: 348,
                    name: '查看Informix/GBase 8s环境信息',
                    local: 'informix-0083.html'
                  },
                  {
                    id: 386,
                    parentId: 348,
                    name: '管理Informix/GBase 8s',
                    local: 'informix-0084.html'
                  },
                  {
                    id: 387,
                    parentId: 348,
                    name: '管理Informix/GBase 8s数据库集群',
                    local: 'informix-0085.html'
                  }
                ]
              },
              {
                id: 349,
                parentId: 31,
                name: '常见问题',
                local: 'informix-0086.html',
                children: [
                  {
                    id: 388,
                    parentId: 349,
                    name: '数据库日志空间占满导致业务无法正常进行',
                    local: 'informix-0091.html'
                  },
                  {
                    id: 389,
                    parentId: 349,
                    name: '执行恢复的子任务过程中重启客户端，恢复任务失败',
                    local: 'informix-1092.html'
                  }
                ]
              }
            ]
          },
          {
            id: 32,
            parentId: 13,
            name: 'openGauss/磐维CMDB数据保护',
            local: 'zh-cn_topic_0000002164607782.html',
            children: [
              {
                id: 390,
                parentId: 32,
                name: '备份',
                local: 'opengauss-0006.html',
                children: [
                  {
                    id: 399,
                    parentId: 390,
                    name: '备份前准备',
                    local: 'opengauss-0009.html'
                  },
                  {
                    id: 400,
                    parentId: 390,
                    name: '备份openGauss/磐维CMDB',
                    local: 'opengauss-0010.html',
                    children: [
                      {
                        id: 401,
                        parentId: 400,
                        name: '步骤1：开启归档模式（仅适用于磐维CMDB）',
                        local: 'postgresql-0010.html'
                      },
                      {
                        id: 402,
                        parentId: 400,
                        name: '步骤2：注册openGauss/磐维CMDB集群',
                        local: 'opengauss-0011.html'
                      },
                      {
                        id: 403,
                        parentId: 400,
                        name: '步骤3：（可选）创建限速策略',
                        local: 'opengauss-0012.html'
                      },
                      {
                        id: 404,
                        parentId: 400,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'opengauss-0013.html'
                      },
                      {
                        id: 405,
                        parentId: 400,
                        name: '步骤5：创建备份SLA',
                        local: 'opengauss-0014.html'
                      },
                      {
                        id: 406,
                        parentId: 400,
                        name: '步骤6：执行备份',
                        local: 'opengauss-0015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 391,
                parentId: 32,
                name: '复制',
                local: 'oracle_gud_000035.html',
                children: [
                  {
                    id: 407,
                    parentId: 391,
                    name: '复制openGauss/磐维CMDB数据库副本',
                    local: 'opengauss-0020.html',
                    children: [
                      {
                        id: 408,
                        parentId: 407,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026.html'
                      },
                      {
                        id: 409,
                        parentId: 407,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1.html'
                      },
                      {
                        id: 410,
                        parentId: 407,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'opengauss-0023.html'
                      },
                      {
                        id: 411,
                        parentId: 407,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'opengauss-0024.html'
                      },
                      {
                        id: 412,
                        parentId: 407,
                        name: '步骤4：下载并导入证书',
                        local: 'opengauss-0025.html'
                      },
                      {
                        id: 413,
                        parentId: 407,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'opengauss-0026.html'
                      },
                      {
                        id: 414,
                        parentId: 407,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'opengauss-0026_a1.html'
                      },
                      {
                        id: 415,
                        parentId: 407,
                        name: '步骤6：添加复制集群',
                        local: 'opengauss-0027.html'
                      },
                      {
                        id: 416,
                        parentId: 407,
                        name: '步骤7：创建复制SLA',
                        local: 'opengauss-0028.html'
                      },
                      {
                        id: 417,
                        parentId: 407,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'opengauss-0029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 392,
                parentId: 32,
                name: '归档',
                local: 'opengauss-0030.html',
                children: [
                  {
                    id: 418,
                    parentId: 392,
                    name: '归档openGauss/磐维CMDB备份副本',
                    local: 'opengauss-0033.html',
                    children: [
                      {
                        id: 420,
                        parentId: 418,
                        name: '步骤1：添加归档存储',
                        local: 'opengauss-0034.html',
                        children: [
                          {
                            id: 422,
                            parentId: 420,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'opengauss-0035.html'
                          },
                          {
                            id: 423,
                            parentId: 420,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'opengauss-0036.html'
                          }
                        ]
                      },
                      {
                        id: 421,
                        parentId: 418,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'opengauss-0037.html'
                      }
                    ]
                  },
                  {
                    id: 419,
                    parentId: 392,
                    name: '归档openGauss/磐维CMDB复制副本',
                    local: 'opengauss-0038.html',
                    children: [
                      {
                        id: 424,
                        parentId: 419,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'opengauss-0039.html'
                      },
                      {
                        id: 425,
                        parentId: 419,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'opengauss-0040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 393,
                parentId: 32,
                name: '恢复',
                local: 'opengauss-0041.html',
                children: [
                  {
                    id: 426,
                    parentId: 393,
                    name: '恢复openGauss/磐维CMDB',
                    local: 'opengauss-0044.html'
                  }
                ]
              },
              {
                id: 394,
                parentId: 32,
                name: '全局搜索',
                local: 'opengauss-0026_a2.html',
                children: [
                  {
                    id: 427,
                    parentId: 394,
                    name: '全局搜索资源',
                    local: 'opengauss-0026_a3.html'
                  },
                  {
                    id: 428,
                    parentId: 394,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'opengauss-0026_a4.html'
                  }
                ]
              },
              {
                id: 395,
                parentId: 32,
                name: 'SLA',
                local: 'opengauss-0049.html',
                children: [
                  {
                    id: 429,
                    parentId: 395,
                    name: '关于SLA',
                    local: 'opengauss-0050.html'
                  },
                  {
                    id: 430,
                    parentId: 395,
                    name: '查看SLA信息',
                    local: 'opengauss-0051.html'
                  },
                  {
                    id: 431,
                    parentId: 395,
                    name: '管理SLA',
                    local: 'opengauss-0052.html'
                  }
                ]
              },
              {
                id: 396,
                parentId: 32,
                name: '副本',
                local: 'opengauss-0053.html',
                children: [
                  {
                    id: 432,
                    parentId: 396,
                    name: '查看openGauss/磐维CMDB副本信息',
                    local: 'opengauss-0054.html'
                  },
                  {
                    id: 433,
                    parentId: 396,
                    name: '管理openGauss/磐维CMDB副本',
                    local: 'opengauss-0055.html'
                  }
                ]
              },
              {
                id: 397,
                parentId: 32,
                name: 'openGauss/磐维CMDB数据库环境',
                local: 'opengauss-0056.html',
                children: [
                  {
                    id: 434,
                    parentId: 397,
                    name: '查看openGauss/磐维CMDB数据库环境信息',
                    local: 'opengauss-0057.html'
                  },
                  {
                    id: 435,
                    parentId: 397,
                    name: '管理openGauss/磐维CMDB',
                    local: 'opengauss-0058.html'
                  },
                  {
                    id: 436,
                    parentId: 397,
                    name: '管理openGauss/磐维CMDB集群',
                    local: 'opengauss-0059.html'
                  }
                ]
              },
              {
                id: 398,
                parentId: 32,
                name: '常见问题',
                local: 'opengauss-0060.html',
                children: [
                  {
                    id: 437,
                    parentId: 398,
                    name: '登录DeviceManager管理界面',
                    local: 'opengauss-0077.html'
                  }
                ]
              }
            ]
          },
          {
            id: 33,
            parentId: 13,
            name: 'GaussDB T数据保护',
            local: 'zh-cn_topic_0000002200094105.html',
            children: [
              {
                id: 438,
                parentId: 33,
                name: '备份',
                local: 'gaussdbT_00007.html',
                children: [
                  {
                    id: 447,
                    parentId: 438,
                    name: '备份前准备',
                    local: 'gaussdbT_00010.html'
                  },
                  {
                    id: 448,
                    parentId: 438,
                    name: '备份GaussDB T数据库',
                    local: 'gaussdbT_00011.html',
                    children: [
                      {
                        id: 449,
                        parentId: 448,
                        name: '步骤1：检查并配置数据库环境',
                        local: 'gaussdbT_00012.html'
                      },
                      {
                        id: 450,
                        parentId: 448,
                        name: '步骤2：设置Redo日志模式',
                        local: 'gaussdbT_00013.html'
                      },
                      {
                        id: 451,
                        parentId: 448,
                        name: '步骤3：注册GaussDB T数据库',
                        local: 'gaussdbT_00014.html'
                      },
                      {
                        id: 452,
                        parentId: 448,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'gaussdbT_00015.html'
                      },
                      {
                        id: 453,
                        parentId: 448,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'gaussdbT_00016.html'
                      },
                      {
                        id: 454,
                        parentId: 448,
                        name: '步骤6：创建备份SLA',
                        local: 'gaussdbT_00017.html'
                      },
                      {
                        id: 455,
                        parentId: 448,
                        name: '步骤7：执行备份',
                        local: 'gaussdbT_00018.html',
                        children: [
                          {
                            id: 456,
                            parentId: 455,
                            name: '备份GaussDB T单机',
                            local: 'gaussdbT_00018-1.html'
                          },
                          {
                            id: 457,
                            parentId: 455,
                            name: '备份GaussDB T集群',
                            local: 'gaussdbT_00018-2.html'
                          }
                        ]
                      }
                    ]
                  }
                ]
              },
              {
                id: 439,
                parentId: 33,
                name: '复制',
                local: 'gaussdbT_00021.html',
                children: [
                  {
                    id: 458,
                    parentId: 439,
                    name: '复制GaussDB T数据库副本',
                    local: 'gaussdbT_00025.html',
                    children: [
                      {
                        id: 459,
                        parentId: 458,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'gaussdbT_00026.html'
                      },
                      {
                        id: 460,
                        parentId: 458,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'gaussdbT_00027.html'
                      },
                      {
                        id: 461,
                        parentId: 458,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'gaussdbT_00028.html'
                      },
                      {
                        id: 462,
                        parentId: 458,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'gaussdbT_00029.html'
                      },
                      {
                        id: 463,
                        parentId: 458,
                        name: '步骤4：下载并导入证书',
                        local: 'gaussdbT_00030.html'
                      },
                      {
                        id: 464,
                        parentId: 458,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'gaussdbT_00031.html'
                      },
                      {
                        id: 465,
                        parentId: 458,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'gaussdbT_00032.html'
                      },
                      {
                        id: 466,
                        parentId: 458,
                        name: '步骤6：添加复制集群',
                        local: 'gaussdbT_00033.html'
                      },
                      {
                        id: 467,
                        parentId: 458,
                        name: '步骤7：创建复制SLA',
                        local: 'gaussdbT_00034.html'
                      },
                      {
                        id: 468,
                        parentId: 458,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'gaussdbT_00035.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 440,
                parentId: 33,
                name: '归档',
                local: 'gaussdbT_00055.html',
                children: [
                  {
                    id: 469,
                    parentId: 440,
                    name: '归档GaussDB T备份副本',
                    local: 'gaussdbT_00058.html',
                    children: [
                      {
                        id: 471,
                        parentId: 469,
                        name: '步骤1：添加归档存储',
                        local: 'gaussdbT_00059.html',
                        children: [
                          {
                            id: 473,
                            parentId: 471,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'gaussdbT_00060.html'
                          },
                          {
                            id: 474,
                            parentId: 471,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'gaussdbT_00061.html'
                          }
                        ]
                      },
                      {
                        id: 472,
                        parentId: 469,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'gaussdbT_00062.html'
                      }
                    ]
                  },
                  {
                    id: 470,
                    parentId: 440,
                    name: '归档GaussDB T复制副本',
                    local: 'gaussdbT_00063.html',
                    children: [
                      {
                        id: 475,
                        parentId: 470,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'gaussdbT_00064.html'
                      },
                      {
                        id: 476,
                        parentId: 470,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'gaussdbT_00065.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 441,
                parentId: 33,
                name: '恢复',
                local: 'gaussdbT_00066.html',
                children: [
                  {
                    id: 477,
                    parentId: 441,
                    name: '恢复GaussDB T数据库',
                    local: 'gaussdbT_00069-1.html',
                    children: [
                      {
                        id: 478,
                        parentId: 477,
                        name: '恢复GaussDB T单机',
                        local: 'gaussdbT_00069.html'
                      },
                      {
                        id: 479,
                        parentId: 477,
                        name: '恢复GaussDB T集群',
                        local: 'gaussdbT_00069-2.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 442,
                parentId: 33,
                name: '全局搜索',
                local: 'gaussdbT_00070.html',
                children: [
                  {
                    id: 480,
                    parentId: 442,
                    name: '全局搜索资源',
                    local: 'gaussdbT_00071.html'
                  },
                  {
                    id: 481,
                    parentId: 442,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'gaussdbT_00072.html'
                  }
                ]
              },
              {
                id: 443,
                parentId: 33,
                name: 'SLA',
                local: 'gaussdbT_00075.html',
                children: [
                  {
                    id: 482,
                    parentId: 443,
                    name: '关于SLA',
                    local: 'gaussdbT_00076.html'
                  },
                  {
                    id: 483,
                    parentId: 443,
                    name: '查看SLA信息',
                    local: 'gaussdbT_00077.html'
                  },
                  {
                    id: 484,
                    parentId: 443,
                    name: '管理SLA',
                    local: 'gaussdbT_00078.html'
                  }
                ]
              },
              {
                id: 444,
                parentId: 33,
                name: '副本',
                local: 'gaussdbT_00079.html',
                children: [
                  {
                    id: 485,
                    parentId: 444,
                    name: '查看GaussDB T副本信息',
                    local: 'gaussdbT_00080.html'
                  },
                  {
                    id: 486,
                    parentId: 444,
                    name: '管理GaussDB T副本',
                    local: 'gaussdbT_00081.html'
                  }
                ]
              },
              {
                id: 445,
                parentId: 33,
                name: 'GaussDB T数据库环境',
                local: 'gaussdbT_00082.html',
                children: [
                  {
                    id: 487,
                    parentId: 445,
                    name: '查看GaussDB T数据库环境信息',
                    local: 'gaussdbT_00083.html'
                  },
                  {
                    id: 488,
                    parentId: 445,
                    name: '管理数据库',
                    local: 'gaussdbT_00084.html'
                  }
                ]
              },
              {
                id: 446,
                parentId: 33,
                name: '常见问题',
                local: 'gaussdbT_00085.html',
                children: [
                  {
                    id: 489,
                    parentId: 446,
                    name: '登录DeviceManager管理界面',
                    local: 'gaussdbT_00087.html'
                  },
                  {
                    id: 490,
                    parentId: 446,
                    name: '备份任务子任务阶段出现“内部错误”',
                    local: 'gaussdbT_00090.html'
                  },
                  {
                    id: 491,
                    parentId: 446,
                    name: '恢复任务子任务阶段出现“内部错误”',
                    local: 'gaussdbT_00091.html'
                  },
                  {
                    id: 492,
                    parentId: 446,
                    name:
                      'GaussDB T数据库未运行在ARCHIVELOG模式，导致备份子任务失败',
                    local: 'gaussdbT_00092.html'
                  },
                  {
                    id: 493,
                    parentId: 446,
                    name:
                      '备份副本所在单机和恢复目标单机安装目录不一致，导致GaussDB T单机恢复失败（适用于单机部署形态）',
                    local: 'gaussdbT_00093.html'
                  },
                  {
                    id: 494,
                    parentId: 446,
                    name: '归档日志被自动清理，导致日志备份子任务失败',
                    local: 'gaussdbT_00094.html'
                  }
                ]
              }
            ]
          },
          {
            id: 34,
            parentId: 13,
            name: 'TiDB数据保护',
            local: 'zh-cn_topic_0000002200008525.html',
            children: [
              {
                id: 495,
                parentId: 34,
                name: '概述',
                local: 'zh-cn_topic_0000002200065061.html',
                children: [
                  {
                    id: 505,
                    parentId: 495,
                    name: '功能概述',
                    local: 'zh-cn_topic_0000002164824094.html'
                  }
                ]
              },
              {
                id: 496,
                parentId: 34,
                name: '备份',
                local: 'TiDB_00004.html',
                children: [
                  {
                    id: 506,
                    parentId: 496,
                    name: '备份前准备',
                    local: 'TiDB_00007.html'
                  },
                  {
                    id: 507,
                    parentId: 496,
                    name: '备份TiDB资源',
                    local: 'TiDB_00008.html',
                    children: [
                      {
                        id: 508,
                        parentId: 507,
                        name: '步骤1：注册TiDB集群',
                        local: 'TiDB_00009.html'
                      },
                      {
                        id: 509,
                        parentId: 507,
                        name: '步骤2：注册TiDB数据库',
                        local: 'TiDB_00010.html'
                      },
                      {
                        id: 510,
                        parentId: 507,
                        name: '步骤3：注册TiDB表集',
                        local: 'TiDB_00011.html'
                      },
                      {
                        id: 511,
                        parentId: 507,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'TiDB_00012.html'
                      },
                      {
                        id: 512,
                        parentId: 507,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'TiDB_00013.html'
                      },
                      {
                        id: 513,
                        parentId: 507,
                        name: '步骤6：创建备份SLA',
                        local: 'TiDB_00014.html'
                      },
                      {
                        id: 514,
                        parentId: 507,
                        name: '步骤7：执行备份',
                        local: 'TiDB_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 497,
                parentId: 34,
                name: '复制',
                local: 'TiDB_00018.html',
                children: [
                  {
                    id: 515,
                    parentId: 497,
                    name: '复制TiDB副本',
                    local: 'TiDB_00021.html',
                    children: [
                      {
                        id: 516,
                        parentId: 515,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'TiDB_00022.html'
                      },
                      {
                        id: 517,
                        parentId: 515,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'TiDB_00023.html'
                      },
                      {
                        id: 518,
                        parentId: 515,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'TiDB_00024.html'
                      },
                      {
                        id: 519,
                        parentId: 515,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'TiDB_00025.html'
                      },
                      {
                        id: 520,
                        parentId: 515,
                        name: '步骤4：下载并导入证书',
                        local: 'TiDB_00026.html'
                      },
                      {
                        id: 521,
                        parentId: 515,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'TiDB_00027.html'
                      },
                      {
                        id: 522,
                        parentId: 515,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'TiDB_00028.html'
                      },
                      {
                        id: 523,
                        parentId: 515,
                        name: '步骤6：添加复制集群',
                        local: 'TiDB_00029.html'
                      },
                      {
                        id: 524,
                        parentId: 515,
                        name: '步骤7：创建复制SLA',
                        local: 'TiDB_00030.html'
                      },
                      {
                        id: 525,
                        parentId: 515,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'TiDB_00031.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 498,
                parentId: 34,
                name: '归档',
                local: 'TiDB_00030_0.html',
                children: [
                  {
                    id: 526,
                    parentId: 498,
                    name: '归档TiDB备份副本',
                    local: 'TiDB_00033_0.html',
                    children: [
                      {
                        id: 528,
                        parentId: 526,
                        name: '步骤1：添加归档存储',
                        local: 'TiDB_00034_0.html',
                        children: [
                          {
                            id: 530,
                            parentId: 528,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'TiDB_00035_0.html'
                          },
                          {
                            id: 531,
                            parentId: 528,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'TiDB_00036_0.html'
                          }
                        ]
                      },
                      {
                        id: 529,
                        parentId: 526,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'TiDB_00037_0.html'
                      }
                    ]
                  },
                  {
                    id: 527,
                    parentId: 498,
                    name: '归档TiDB复制副本',
                    local: 'TiDB_00038_0.html',
                    children: [
                      {
                        id: 532,
                        parentId: 527,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'TiDB_00039_0.html'
                      },
                      {
                        id: 533,
                        parentId: 527,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'TiDB_00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 499,
                parentId: 34,
                name: '恢复',
                local: 'TiDB_00041.html',
                children: [
                  {
                    id: 534,
                    parentId: 499,
                    name: '恢复TiDB备份资源',
                    local: 'TiDB_00044.html'
                  },
                  {
                    id: 535,
                    parentId: 499,
                    name: '恢复TiDB备份资源中的单个或多个表',
                    local: 'TiDB_00045.html'
                  }
                ]
              },
              {
                id: 500,
                parentId: 34,
                name: '全局搜索',
                local: 'TiDB_00046.html',
                children: [
                  {
                    id: 536,
                    parentId: 500,
                    name: '全局搜索资源',
                    local: 'TiDB_00047.html'
                  },
                  {
                    id: 537,
                    parentId: 500,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'TiDB_000471_0.html'
                  }
                ]
              },
              {
                id: 501,
                parentId: 34,
                name: 'SLA',
                local: 'TiDB_00050_0.html',
                children: [
                  {
                    id: 538,
                    parentId: 501,
                    name: '关于SLA',
                    local: 'TiDB_000501.html'
                  },
                  {
                    id: 539,
                    parentId: 501,
                    name: '查看SLA信息',
                    local: 'TiDB_00052.html'
                  },
                  {
                    id: 540,
                    parentId: 501,
                    name: '管理SLA',
                    local: 'TiDB_00053.html'
                  }
                ]
              },
              {
                id: 502,
                parentId: 34,
                name: '副本',
                local: 'TiDB_00054.html',
                children: [
                  {
                    id: 541,
                    parentId: 502,
                    name: '查看TiDB副本信息',
                    local: 'TiDB_00055.html'
                  },
                  {
                    id: 542,
                    parentId: 502,
                    name: '管理TiDB副本',
                    local: 'TiDB_00056.html'
                  }
                ]
              },
              {
                id: 503,
                parentId: 34,
                name: 'TiDB集群环境',
                local: 'TiDB_00057.html',
                children: [
                  {
                    id: 543,
                    parentId: 503,
                    name: '查询TiDB信息',
                    local: 'TiDB_00058.html'
                  },
                  {
                    id: 544,
                    parentId: 503,
                    name: '管理TiDB集群',
                    local: 'TiDB_00059.html'
                  },
                  {
                    id: 545,
                    parentId: 503,
                    name: '管理数据库',
                    local: 'TiDB_00060.html'
                  },
                  {
                    id: 546,
                    parentId: 503,
                    name: '管理表集',
                    local: 'TiDB_00061.html'
                  }
                ]
              },
              {
                id: 504,
                parentId: 34,
                name: '常见问题',
                local: 'TiDB_00062.html',
                children: [
                  {
                    id: 547,
                    parentId: 504,
                    name: '在生产环境开启或关闭日志备份开关',
                    local: 'TiDB_00072.html'
                  },
                  {
                    id: 548,
                    parentId: 504,
                    name: 'TiDB集群用户UID不一致',
                    local: 'TiDB_00073.html'
                  },
                  {
                    id: 549,
                    parentId: 504,
                    name: '恢复时报错：new_collation_enabled不匹配',
                    local: 'zh-cn_topic_0000002200065113.html'
                  },
                  {
                    id: 550,
                    parentId: 504,
                    name: '手动连接数据库删除数据库中的表',
                    local: 'zh-cn_topic_0000002188473576.html'
                  }
                ]
              }
            ]
          },
          {
            id: 35,
            parentId: 13,
            name: 'OceanBase数据保护',
            local: 'zh-cn_topic_0000002164607762.html',
            children: [
              {
                id: 551,
                parentId: 35,
                name: '备份',
                local: 'oceanbase_00005.html',
                children: [
                  {
                    id: 560,
                    parentId: 551,
                    name: '备份前准备',
                    local: 'oceanbase_00008.html'
                  },
                  {
                    id: 561,
                    parentId: 551,
                    name: '备份OceanBase',
                    local: 'oceanbase_00009.html',
                    children: [
                      {
                        id: 562,
                        parentId: 561,
                        name: '步骤1：开启NFSv4.1服务',
                        local: 'zh-cn_topic_0000002164645372.html'
                      },
                      {
                        id: 563,
                        parentId: 561,
                        name: '步骤1：开启NFSv4.1服务（适用于部分型号）',
                        local: 'zh-cn_topic_0000002164645420.html'
                      },
                      {
                        id: 564,
                        parentId: 561,
                        name: '步骤2：注册OceanBase集群',
                        local: 'oceanbase_00010.html'
                      },
                      {
                        id: 565,
                        parentId: 561,
                        name: '步骤3：注册OceanBase租户集',
                        local: 'oceanbase_00011.html'
                      },
                      {
                        id: 566,
                        parentId: 561,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'oceanbase_00012.html'
                      },
                      {
                        id: 567,
                        parentId: 561,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'oceanbase_00013.html'
                      },
                      {
                        id: 568,
                        parentId: 561,
                        name: '步骤6：创建备份SLA',
                        local: 'oceanbase_00014.html'
                      },
                      {
                        id: 569,
                        parentId: 561,
                        name: '步骤7：执行备份',
                        local: 'oceanbase_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 552,
                parentId: 35,
                name: '复制',
                local: 'oracle_gud_000035_1.html',
                children: [
                  {
                    id: 570,
                    parentId: 552,
                    name: '复制OceanBase副本',
                    local: 'oceanbase_00020.html',
                    children: [
                      {
                        id: 571,
                        parentId: 570,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'oceanbase_000210.html'
                      },
                      {
                        id: 572,
                        parentId: 570,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_1.html'
                      },
                      {
                        id: 573,
                        parentId: 570,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'oceanbase_00023.html'
                      },
                      {
                        id: 574,
                        parentId: 570,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'oceanbase_00024.html'
                      },
                      {
                        id: 575,
                        parentId: 570,
                        name: '步骤4：下载并导入证书',
                        local: 'oceanbase_00025.html'
                      },
                      {
                        id: 576,
                        parentId: 570,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'oceanbase_00026.html'
                      },
                      {
                        id: 577,
                        parentId: 570,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'oceanbase_00026_a1.html'
                      },
                      {
                        id: 578,
                        parentId: 570,
                        name: '步骤6：添加复制集群',
                        local: 'oceanbase_00027.html'
                      },
                      {
                        id: 579,
                        parentId: 570,
                        name: '步骤7：创建复制SLA',
                        local: 'oceanbase_00028.html'
                      },
                      {
                        id: 580,
                        parentId: 570,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'oceanbase_00029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 553,
                parentId: 35,
                name: '归档',
                local: 'oceanbase_00030.html',
                children: [
                  {
                    id: 581,
                    parentId: 553,
                    name: '归档OceanBase备份副本',
                    local: 'oceanbase_00033.html',
                    children: [
                      {
                        id: 583,
                        parentId: 581,
                        name: '步骤1：添加归档存储',
                        local: 'oceanbase_00034.html',
                        children: [
                          {
                            id: 585,
                            parentId: 583,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'oceanbase_00035.html'
                          },
                          {
                            id: 586,
                            parentId: 583,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'oceanbase_00036.html'
                          }
                        ]
                      },
                      {
                        id: 584,
                        parentId: 581,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'oceanbase_00037.html'
                      }
                    ]
                  },
                  {
                    id: 582,
                    parentId: 553,
                    name: '归档OceanBase复制副本',
                    local: 'oceanbase_00038.html',
                    children: [
                      {
                        id: 587,
                        parentId: 582,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'oceanbase_00039.html'
                      },
                      {
                        id: 588,
                        parentId: 582,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'oceanbase_00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 554,
                parentId: 35,
                name: '恢复',
                local: 'oceanbase_00041.html',
                children: [
                  {
                    id: 589,
                    parentId: 554,
                    name: '恢复OceanBase数据库',
                    local: 'oceanbase_00044.html'
                  },
                  {
                    id: 590,
                    parentId: 554,
                    name: '恢复OceanBase数据库的单个或多个表',
                    local: 'oceanbase_00244.html'
                  }
                ]
              },
              {
                id: 555,
                parentId: 35,
                name: '全局搜索',
                local: 'oceanbase_00026_a2.html',
                children: [
                  {
                    id: 591,
                    parentId: 555,
                    name: '全局搜索资源',
                    local: 'oceanbase_00026_a3.html'
                  },
                  {
                    id: 592,
                    parentId: 555,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'oceanbase_00026_a4.html'
                  }
                ]
              },
              {
                id: 556,
                parentId: 35,
                name: 'SLA',
                local: 'oceanbase_00050.html',
                children: [
                  {
                    id: 593,
                    parentId: 556,
                    name: '关于SLA',
                    local: 'oceanbase_00051.html'
                  },
                  {
                    id: 594,
                    parentId: 556,
                    name: '查看SLA信息',
                    local: 'oceanbase_00052.html'
                  },
                  {
                    id: 595,
                    parentId: 556,
                    name: '管理SLA',
                    local: 'oceanbase_00053.html'
                  }
                ]
              },
              {
                id: 557,
                parentId: 35,
                name: '副本',
                local: 'oceanbase_00054.html',
                children: [
                  {
                    id: 596,
                    parentId: 557,
                    name: '查看OceanBase副本信息',
                    local: 'oceanbase_00055.html'
                  },
                  {
                    id: 597,
                    parentId: 557,
                    name: '管理OceanBase副本',
                    local: 'oceanbase_00056.html'
                  }
                ]
              },
              {
                id: 558,
                parentId: 35,
                name: 'OceanBase集群环境',
                local: 'oceanbase_00057.html',
                children: [
                  {
                    id: 598,
                    parentId: 558,
                    name: '查看OceanBase环境信息',
                    local: 'oceanbase_00058.html'
                  },
                  {
                    id: 599,
                    parentId: 558,
                    name: '管理集群',
                    local: 'oceanbase_00059.html'
                  },
                  {
                    id: 600,
                    parentId: 558,
                    name: '管理租户集',
                    local: 'oceanbase_00060.html'
                  }
                ]
              },
              {
                id: 559,
                parentId: 35,
                name: '常见问题',
                local: 'oceanbase_00061.html',
                children: [
                  {
                    id: 601,
                    parentId: 559,
                    name: '登录DeviceManager管理界面',
                    local: 'oceanbase_00077.html'
                  }
                ]
              }
            ]
          },
          {
            id: 36,
            parentId: 13,
            name: 'TDSQL数据保护',
            local: 'zh-cn_topic_0000002200008577.html',
            children: [
              {
                id: 602,
                parentId: 36,
                name: '备份',
                local: 'tdsql_gud_009.html',
                children: [
                  {
                    id: 611,
                    parentId: 602,
                    name: '备份前准备',
                    local: 'tdsql_gud_012.html'
                  },
                  {
                    id: 612,
                    parentId: 602,
                    name: '备份TDSQL数据库',
                    local: 'tdsql_gud_013.html',
                    children: [
                      {
                        id: 613,
                        parentId: 612,
                        name:
                          '步骤1：开启TDSQL数据库权限（适用于非分布式实例）',
                        local: 'tdsql_gud_014.html'
                      },
                      {
                        id: 614,
                        parentId: 612,
                        name:
                          '步骤2：开启zkmeta自动备份功能（适用于分布式实例）',
                        local: 'tdsql_gud_015.html'
                      },
                      {
                        id: 615,
                        parentId: 612,
                        name: '步骤3：注册TDSQL数据库',
                        local: 'tdsql_gud_016.html'
                      },
                      {
                        id: 616,
                        parentId: 612,
                        name: '步骤4：（可选）修改agent_cfg.xml备份文件',
                        local: 'tdsql_gud_017.html'
                      },
                      {
                        id: 617,
                        parentId: 612,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'tdsql_gud_018.html'
                      },
                      {
                        id: 618,
                        parentId: 612,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'tdsql_gud_019.html'
                      },
                      {
                        id: 619,
                        parentId: 612,
                        name: '步骤7：创建备份SLA',
                        local: 'tdsql_gud_020.html'
                      },
                      {
                        id: 620,
                        parentId: 612,
                        name: '步骤8：执行备份',
                        local: 'tdsql_gud_021.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 603,
                parentId: 36,
                name: '复制',
                local: 'tdsql_gud_022.html'
              },
              {
                id: 604,
                parentId: 36,
                name: '归档',
                local: 'tdsql_gud_046.html',
                children: [
                  {
                    id: 621,
                    parentId: 604,
                    name: '归档TDSQL备份副本',
                    local: 'tdsql_gud_049.html',
                    children: [
                      {
                        id: 623,
                        parentId: 621,
                        name: '步骤1：添加归档存储',
                        local: 'tdsql_gud_050.html',
                        children: [
                          {
                            id: 625,
                            parentId: 623,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'tdsql_gud_051.html'
                          },
                          {
                            id: 626,
                            parentId: 623,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'tdsql_gud_052.html'
                          }
                        ]
                      },
                      {
                        id: 624,
                        parentId: 621,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'tdsql_gud_053.html'
                      }
                    ]
                  },
                  {
                    id: 622,
                    parentId: 604,
                    name: '归档TDSQL复制副本',
                    local: 'tdsql_gud_054.html',
                    children: [
                      {
                        id: 627,
                        parentId: 622,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'tdsql_gud_055.html'
                      },
                      {
                        id: 628,
                        parentId: 622,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'tdsql_gud_056.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 605,
                parentId: 36,
                name: '恢复',
                local: 'tdsql_gud_057.html',
                children: [
                  {
                    id: 629,
                    parentId: 605,
                    name: '恢复TDSQL数据库',
                    local: 'tdsql_gud_060.html'
                  }
                ]
              },
              {
                id: 606,
                parentId: 36,
                name: '全局搜索',
                local: 'tdsql_gud_061.html',
                children: [
                  {
                    id: 630,
                    parentId: 606,
                    name: '全局搜索资源',
                    local: 'tdsql_gud_062.html'
                  },
                  {
                    id: 631,
                    parentId: 606,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'tdsql_gud_063.html'
                  }
                ]
              },
              {
                id: 607,
                parentId: 36,
                name: 'SLA',
                local: 'tdsql_gud_075.html',
                children: [
                  {
                    id: 632,
                    parentId: 607,
                    name: '关于SLA',
                    local: 'tdsql_gud_076.html'
                  },
                  {
                    id: 633,
                    parentId: 607,
                    name: '查看SLA信息',
                    local: 'tdsql_gud_077.html'
                  },
                  {
                    id: 634,
                    parentId: 607,
                    name: '管理SLA',
                    local: 'tdsql_gud_078.html'
                  }
                ]
              },
              {
                id: 608,
                parentId: 36,
                name: '副本',
                local: 'tdsql_gud_079.html',
                children: [
                  {
                    id: 635,
                    parentId: 608,
                    name: '查看TDSQL副本信息',
                    local: 'tdsql_gud_080.html'
                  },
                  {
                    id: 636,
                    parentId: 608,
                    name: '管理TDSQL副本',
                    local: 'tdsql_gud_081.html'
                  }
                ]
              },
              {
                id: 609,
                parentId: 36,
                name: 'TDSQL数据库环境',
                local: 'tdsql_gud_082.html',
                children: [
                  {
                    id: 637,
                    parentId: 609,
                    name: '查看TDSQL数据库环境信息',
                    local: 'tdsql_gud_083.html'
                  },
                  {
                    id: 638,
                    parentId: 609,
                    name: '管理数据库集群',
                    local: 'tdsql_gud_084.html'
                  },
                  {
                    id: 639,
                    parentId: 609,
                    name: '管理数据库实例',
                    local: 'tdsql_gud_085.html'
                  }
                ]
              },
              {
                id: 610,
                parentId: 36,
                name: '常见问题',
                local: 'tdsql_gud_086.html',
                children: [
                  {
                    id: 640,
                    parentId: 610,
                    name: '授权备份存储单元（适用于1.6.0及后续版本）',
                    local: 'tdsql_gud_088.html'
                  },
                  {
                    id: 641,
                    parentId: 610,
                    name: '使用VDC管理员或VDC业务员账号进入云备份控制台',
                    local: 'tdsql_gud_089.html'
                  },
                  {
                    id: 642,
                    parentId: 610,
                    name: '使用VDC管理员或VDC业务员账号进入云备份控制台',
                    local: 'tdsql_gud_090.html'
                  },
                  {
                    id: 643,
                    parentId: 610,
                    name: '登录DeviceManager管理界面',
                    local: 'tdsql_gud_091.html'
                  },
                  {
                    id: 644,
                    parentId: 610,
                    name: '如何配置冷备节点',
                    local: 'tdsql_gud_092.html'
                  },
                  {
                    id: 645,
                    parentId: 610,
                    name: '如何查询TDSQL数据节点是否已安装mysql服务',
                    local: 'tdsql_gud_093.html'
                  },
                  {
                    id: 646,
                    parentId: 610,
                    name: '如何手动停止TDSQL恢复任务',
                    local: 'tdsql_gud_094.html'
                  },
                  {
                    id: 647,
                    parentId: 610,
                    name:
                      'TDSQL MySQL版V5.7.17版本非分布式实例恢复后出现备延迟耗时长',
                    local: 'tdsql_gud_095.html'
                  },
                  {
                    id: 648,
                    parentId: 610,
                    name:
                      '非分布式实例日志备份开启“备份完成后删除归档日志”，生成备份副本失败（适用于1.6.0及后续版本）',
                    local: 'tdsql_gud_096.html'
                  }
                ]
              }
            ]
          },
          {
            id: 37,
            parentId: 13,
            name: 'Dameng数据保护',
            local: 'zh-cn_topic_0000002200008521.html',
            children: [
              {
                id: 649,
                parentId: 37,
                name: '备份',
                local: 'dameng-00005.html',
                children: [
                  {
                    id: 658,
                    parentId: 649,
                    name: '备份前准备',
                    local: 'dameng-00008.html'
                  },
                  {
                    id: 659,
                    parentId: 649,
                    name: '备份Dameng',
                    local: 'dameng-00009.html',
                    children: [
                      {
                        id: 660,
                        parentId: 659,
                        name: '步骤1：开启DmAPService服务',
                        local: 'dameng-00010.html'
                      },
                      {
                        id: 661,
                        parentId: 659,
                        name: '步骤2：开启数据库本地归档',
                        local: 'dameng-00011.html'
                      },
                      {
                        id: 662,
                        parentId: 659,
                        name: '步骤3：注册Dameng数据库',
                        local: 'dameng-00012.html'
                      },
                      {
                        id: 663,
                        parentId: 659,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'dameng-00012_1.html'
                      },
                      {
                        id: 664,
                        parentId: 659,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'dameng-00014.html'
                      },
                      {
                        id: 665,
                        parentId: 659,
                        name: '步骤6：创建备份SLA',
                        local: 'dameng-00015.html'
                      },
                      {
                        id: 666,
                        parentId: 659,
                        name: '步骤7：执行备份',
                        local: 'dameng-00016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 650,
                parentId: 37,
                name: '复制',
                local: 'oracle_gud_000035_0.html',
                children: [
                  {
                    id: 667,
                    parentId: 650,
                    name: '复制Dameng数据库副本',
                    local: 'dameng-00021.html',
                    children: [
                      {
                        id: 668,
                        parentId: 667,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_0.html'
                      },
                      {
                        id: 669,
                        parentId: 667,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_0.html'
                      },
                      {
                        id: 670,
                        parentId: 667,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'dameng-00024.html'
                      },
                      {
                        id: 671,
                        parentId: 667,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'dameng-00025.html'
                      },
                      {
                        id: 672,
                        parentId: 667,
                        name: '步骤4：下载并导入证书',
                        local: 'dameng-00026.html'
                      },
                      {
                        id: 673,
                        parentId: 667,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'dameng-00027.html'
                      },
                      {
                        id: 674,
                        parentId: 667,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'dameng-00027_a1.html'
                      },
                      {
                        id: 675,
                        parentId: 667,
                        name: '步骤6：添加复制集群',
                        local: 'dameng-00028.html'
                      },
                      {
                        id: 676,
                        parentId: 667,
                        name: '步骤7：创建复制SLA',
                        local: 'dameng-00029.html'
                      },
                      {
                        id: 677,
                        parentId: 667,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'dameng-00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 651,
                parentId: 37,
                name: '归档',
                local: 'dameng-00031.html',
                children: [
                  {
                    id: 678,
                    parentId: 651,
                    name: '归档Dameng备份副本',
                    local: 'dameng-00034.html',
                    children: [
                      {
                        id: 680,
                        parentId: 678,
                        name: '步骤1：添加归档存储',
                        local: 'dameng-00035.html',
                        children: [
                          {
                            id: 682,
                            parentId: 680,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'dameng-00036.html'
                          },
                          {
                            id: 683,
                            parentId: 680,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'dameng-00037.html'
                          }
                        ]
                      },
                      {
                        id: 681,
                        parentId: 678,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'dameng-00038.html'
                      }
                    ]
                  },
                  {
                    id: 679,
                    parentId: 651,
                    name: '归档Dameng复制副本',
                    local: 'dameng-00039.html',
                    children: [
                      {
                        id: 684,
                        parentId: 679,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'dameng-00040.html'
                      },
                      {
                        id: 685,
                        parentId: 679,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'dameng-00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 652,
                parentId: 37,
                name: '恢复',
                local: 'dameng-00042.html',
                children: [
                  {
                    id: 686,
                    parentId: 652,
                    name: '恢复Dameng数据库',
                    local: 'dameng-00045.html'
                  },
                  {
                    id: 687,
                    parentId: 652,
                    name: '恢复Dameng数据库中单个或多个表的表空间',
                    local: 'zh-cn_topic_0000002173977160.html'
                  }
                ]
              },
              {
                id: 653,
                parentId: 37,
                name: '全局搜索',
                local: 'dameng-00027_a2.html',
                children: [
                  {
                    id: 688,
                    parentId: 653,
                    name: '全局搜索资源',
                    local: 'dameng-00027_a3.html'
                  },
                  {
                    id: 689,
                    parentId: 653,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'dameng-00027_a4.html'
                  }
                ]
              },
              {
                id: 654,
                parentId: 37,
                name: 'SLA',
                local: 'dameng-00050.html',
                children: [
                  {
                    id: 690,
                    parentId: 654,
                    name: '关于SLA',
                    local: 'dameng-00051.html'
                  },
                  {
                    id: 691,
                    parentId: 654,
                    name: '查看SLA信息',
                    local: 'dameng-00052.html'
                  },
                  {
                    id: 692,
                    parentId: 654,
                    name: '管理SLA',
                    local: 'dameng-00053.html'
                  }
                ]
              },
              {
                id: 655,
                parentId: 37,
                name: '副本',
                local: 'dameng-00054.html',
                children: [
                  {
                    id: 693,
                    parentId: 655,
                    name: '查看Dameng副本信息',
                    local: 'dameng-00055.html'
                  },
                  {
                    id: 694,
                    parentId: 655,
                    name: '管理Dameng副本',
                    local: 'dameng-00056.html'
                  }
                ]
              },
              {
                id: 656,
                parentId: 37,
                name: 'Dameng环境',
                local: 'dameng-00057.html',
                children: [
                  {
                    id: 695,
                    parentId: 656,
                    name: '查看Dameng环境信息',
                    local: 'dameng-00058.html'
                  },
                  {
                    id: 696,
                    parentId: 656,
                    name: '管理Dameng',
                    local: 'dameng-00059.html'
                  }
                ]
              },
              {
                id: 657,
                parentId: 37,
                name: '常见问题',
                local: 'dameng-00060.html',
                children: [
                  {
                    id: 697,
                    parentId: 657,
                    name: '登录DeviceManager管理界面',
                    local: 'zh-cn_topic_0000002164659666.html'
                  },
                  {
                    id: 698,
                    parentId: 657,
                    name: 'Dameng数据库集群恢复后状态异常',
                    local: 'Troubleshooting-0095-a1.html'
                  }
                ]
              }
            ]
          },
          {
            id: 38,
            parentId: 13,
            name: 'Kingbase数据保护',
            local: 'zh-cn_topic_0000002164607806.html',
            children: [
              {
                id: 699,
                parentId: 38,
                name: '备份',
                local: 'kingbase-00008.html',
                children: [
                  {
                    id: 708,
                    parentId: 699,
                    name: '备份前准备',
                    local: 'kingbase-00011.html'
                  },
                  {
                    id: 709,
                    parentId: 699,
                    name: '备份Kingbase实例',
                    local: 'kingbase-00012.html',
                    children: [
                      {
                        id: 710,
                        parentId: 709,
                        name: '步骤1：开启归档模式',
                        local: 'kingbase-00013.html'
                      },
                      {
                        id: 711,
                        parentId: 709,
                        name: '步骤2：sys_rman初始化配置',
                        local: 'kingbase-00014.html'
                      },
                      {
                        id: 712,
                        parentId: 709,
                        name: '步骤3：注册Kingbase单实例下的数据库',
                        local: 'kingbase-00015.html'
                      },
                      {
                        id: 713,
                        parentId: 709,
                        name: '步骤4：注册Kingbase集群实例下的数据库',
                        local: 'kingbase-00016.html'
                      },
                      {
                        id: 714,
                        parentId: 709,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'kingbase-00017.html'
                      },
                      {
                        id: 715,
                        parentId: 709,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'kingbase-00018.html'
                      },
                      {
                        id: 716,
                        parentId: 709,
                        name: '步骤：创建备份SLA',
                        local: 'kingbase-00019.html'
                      },
                      {
                        id: 717,
                        parentId: 709,
                        name: '步骤：执行备份',
                        local: 'kingbase-00020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 700,
                parentId: 38,
                name: '复制',
                local: 'kingbase-00021.html',
                children: [
                  {
                    id: 718,
                    parentId: 700,
                    name: '复制Kingbase副本',
                    local: 'kingbase-00034.html',
                    children: [
                      {
                        id: 719,
                        parentId: 718,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'kingbase-00035.html'
                      },
                      {
                        id: 720,
                        parentId: 718,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'kingbase-00036.html'
                      },
                      {
                        id: 721,
                        parentId: 718,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'kingbase-00037.html'
                      },
                      {
                        id: 722,
                        parentId: 718,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'kingbase-00038.html'
                      },
                      {
                        id: 723,
                        parentId: 718,
                        name: '步骤4：下载并导入证书',
                        local: 'kingbase-00039.html'
                      },
                      {
                        id: 724,
                        parentId: 718,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'kingbase-00040.html'
                      },
                      {
                        id: 725,
                        parentId: 718,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'kingbase-00041.html'
                      },
                      {
                        id: 726,
                        parentId: 718,
                        name: '步骤6：添加复制集群',
                        local: 'kingbase-00042.html'
                      },
                      {
                        id: 727,
                        parentId: 718,
                        name: '步骤7：创建复制SLA',
                        local: 'kingbase-00043.html'
                      },
                      {
                        id: 728,
                        parentId: 718,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'kingbase-00044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 701,
                parentId: 38,
                name: '归档',
                local: 'kingbase-00067.html',
                children: [
                  {
                    id: 729,
                    parentId: 701,
                    name: '归档Kingbase备份副本',
                    local: 'kingbase-00070.html',
                    children: [
                      {
                        id: 731,
                        parentId: 729,
                        name: '步骤1：添加归档存储',
                        local: 'kingbase-00071.html',
                        children: [
                          {
                            id: 733,
                            parentId: 731,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'kingbase-00072.html'
                          },
                          {
                            id: 734,
                            parentId: 731,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'kingbase-00073.html'
                          }
                        ]
                      },
                      {
                        id: 732,
                        parentId: 729,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'kingbase-00074.html'
                      }
                    ]
                  },
                  {
                    id: 730,
                    parentId: 701,
                    name: '归档Kingbase复制副本',
                    local: 'kingbase-00075.html',
                    children: [
                      {
                        id: 735,
                        parentId: 730,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'kingbase-00076.html'
                      },
                      {
                        id: 736,
                        parentId: 730,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'kingbase-00077.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 702,
                parentId: 38,
                name: '恢复',
                local: 'kingbase-00078.html',
                children: [
                  {
                    id: 737,
                    parentId: 702,
                    name: '恢复Kingbase实例',
                    local: 'kingbase-00081.html'
                  }
                ]
              },
              {
                id: 703,
                parentId: 38,
                name: '全局搜索',
                local: 'kingbase-00082.html',
                children: [
                  {
                    id: 738,
                    parentId: 703,
                    name: '全局搜索资源',
                    local: 'kingbase-00083.html'
                  },
                  {
                    id: 739,
                    parentId: 703,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'kingbase-00084.html'
                  }
                ]
              },
              {
                id: 704,
                parentId: 38,
                name: 'SLA',
                local: 'kingbase-00087.html',
                children: [
                  {
                    id: 740,
                    parentId: 704,
                    name: '关于SLA',
                    local: 'kingbase-00088.html'
                  },
                  {
                    id: 741,
                    parentId: 704,
                    name: '查看SLA信息',
                    local: 'kingbase-00089.html'
                  },
                  {
                    id: 742,
                    parentId: 704,
                    name: '管理SLA',
                    local: 'kingbase-00090.html'
                  }
                ]
              },
              {
                id: 705,
                parentId: 38,
                name: '副本',
                local: 'kingbase-00091.html',
                children: [
                  {
                    id: 743,
                    parentId: 705,
                    name: '查看Kingbase副本信息',
                    local: 'kingbase-00092.html'
                  },
                  {
                    id: 744,
                    parentId: 705,
                    name: '管理Kingbase副本',
                    local: 'kingbase-00093.html'
                  }
                ]
              },
              {
                id: 706,
                parentId: 38,
                name: 'Kingbase集群环境',
                local: 'kingbase-00094.html',
                children: [
                  {
                    id: 745,
                    parentId: 706,
                    name: '查看Kingbase环境信息',
                    local: 'kingbase-00095.html'
                  },
                  {
                    id: 746,
                    parentId: 706,
                    name: '管理Kingbase',
                    local: 'kingbase-00096.html'
                  },
                  {
                    id: 747,
                    parentId: 706,
                    name: '管理Kingbase数据库集群',
                    local: 'kingbase-00097.html'
                  }
                ]
              },
              {
                id: 707,
                parentId: 38,
                name: '常见问题',
                local: 'kingbase-00098.html',
                children: [
                  {
                    id: 748,
                    parentId: 707,
                    name: '登录DeviceManager管理界面',
                    local: 'kingbase-00100.html'
                  }
                ]
              }
            ]
          },
          {
            id: 39,
            parentId: 13,
            name: 'GoldenDB数据保护',
            local: 'zh-cn_topic_0000002164767466.html',
            children: [
              {
                id: 749,
                parentId: 39,
                name: '备份',
                local: 'goldendb-00007.html',
                children: [
                  {
                    id: 757,
                    parentId: 749,
                    name: '备份前准备',
                    local: 'goldendb-00010.html'
                  },
                  {
                    id: 758,
                    parentId: 749,
                    name: '备份GoldenDB数据库',
                    local: 'goldendb-00011.html',
                    children: [
                      {
                        id: 759,
                        parentId: 758,
                        name: '步骤1：注册GoldenDB集群',
                        local: 'goldendb-00012.html'
                      },
                      {
                        id: 760,
                        parentId: 758,
                        name: '步骤2：创建GoldenDB实例',
                        local: 'goldendb-00013.html'
                      },
                      {
                        id: 761,
                        parentId: 758,
                        name: '步骤3：（可选）修改dbagent.ini备份文件',
                        local: 'goldendb-02013.html'
                      },
                      {
                        id: 762,
                        parentId: 758,
                        name:
                          '步骤4：（可选）修改agent_cfg.xml备份文件（适用于部分型号）',
                        local: 'goldendb-02014.html'
                      },
                      {
                        id: 763,
                        parentId: 758,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'goldendb-00014.html'
                      },
                      {
                        id: 764,
                        parentId: 758,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'goldendb-00015.html'
                      },
                      {
                        id: 765,
                        parentId: 758,
                        name: '步骤7：创建备份SLA',
                        local: 'goldendb-00016.html'
                      },
                      {
                        id: 766,
                        parentId: 758,
                        name: '步骤8：执行备份',
                        local: 'goldendb-00017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 750,
                parentId: 39,
                name: '复制',
                local: 'goldendb-00020.html',
                children: [
                  {
                    id: 767,
                    parentId: 750,
                    name: '复制GoldenDB数据库副本',
                    local: 'goldendb-00023.html',
                    children: [
                      {
                        id: 768,
                        parentId: 767,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'goldendb-00024.html'
                      },
                      {
                        id: 769,
                        parentId: 767,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'goldendb-00025.html'
                      },
                      {
                        id: 770,
                        parentId: 767,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'goldendb-00026.html'
                      },
                      {
                        id: 771,
                        parentId: 767,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'goldendb-00027.html'
                      },
                      {
                        id: 772,
                        parentId: 767,
                        name: '步骤4：下载并导入证书',
                        local: 'goldendb-00028.html'
                      },
                      {
                        id: 773,
                        parentId: 767,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'goldendb-00029.html'
                      },
                      {
                        id: 774,
                        parentId: 767,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'goldendb-00030.html'
                      },
                      {
                        id: 775,
                        parentId: 767,
                        name: '步骤6：添加复制集群',
                        local: 'goldendb-00031.html'
                      },
                      {
                        id: 776,
                        parentId: 767,
                        name: '步骤7：创建复制SLA',
                        local: 'goldendb-00032.html'
                      },
                      {
                        id: 777,
                        parentId: 767,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'goldendb-00033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 751,
                parentId: 39,
                name: '归档',
                local: 'goldendb-00053.html',
                children: [
                  {
                    id: 778,
                    parentId: 751,
                    name: '归档GoldenDB备份副本',
                    local: 'goldendb-00056.html',
                    children: [
                      {
                        id: 780,
                        parentId: 778,
                        name: '步骤1：添加归档存储',
                        local: 'goldendb-00057.html',
                        children: [
                          {
                            id: 782,
                            parentId: 780,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'goldendb-00058.html'
                          },
                          {
                            id: 783,
                            parentId: 780,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'goldendb-00059.html'
                          }
                        ]
                      },
                      {
                        id: 781,
                        parentId: 778,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'goldendb-00060.html'
                      }
                    ]
                  },
                  {
                    id: 779,
                    parentId: 751,
                    name: '归档GoldenDB复制副本',
                    local: 'goldendb-00061.html',
                    children: [
                      {
                        id: 784,
                        parentId: 779,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'goldendb-00062.html'
                      },
                      {
                        id: 785,
                        parentId: 779,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'goldendb-00063.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 752,
                parentId: 39,
                name: '恢复',
                local: 'goldendb-00064.html',
                children: [
                  {
                    id: 786,
                    parentId: 752,
                    name: '恢复GoldenDB',
                    local: 'goldendb-00067.html'
                  }
                ]
              },
              {
                id: 753,
                parentId: 39,
                name: '全局搜索',
                local: 'goldendb-00068.html',
                children: [
                  {
                    id: 787,
                    parentId: 753,
                    name: '全局搜索资源',
                    local: 'goldendb-00069.html'
                  },
                  {
                    id: 788,
                    parentId: 753,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'goldendb-00070.html'
                  }
                ]
              },
              {
                id: 754,
                parentId: 39,
                name: 'SLA',
                local: 'goldendb-00074.html',
                children: [
                  {
                    id: 789,
                    parentId: 754,
                    name: '关于SLA',
                    local: 'goldendb-00075.html'
                  },
                  {
                    id: 790,
                    parentId: 754,
                    name: '查看SLA信息',
                    local: 'goldendb-00076.html'
                  },
                  {
                    id: 791,
                    parentId: 754,
                    name: '管理SLA',
                    local: 'goldendb-00077.html'
                  }
                ]
              },
              {
                id: 755,
                parentId: 39,
                name: '副本',
                local: 'goldendb-00078.html',
                children: [
                  {
                    id: 792,
                    parentId: 755,
                    name: '查看GoldenDB副本信息',
                    local: 'goldendb-00079.html'
                  },
                  {
                    id: 793,
                    parentId: 755,
                    name: '管理GoldenDB副本',
                    local: 'goldendb-00080.html'
                  }
                ]
              },
              {
                id: 756,
                parentId: 39,
                name: 'GoldenDB集群环境',
                local: 'goldendb-00081.html',
                children: [
                  {
                    id: 794,
                    parentId: 756,
                    name: '查看GoldenDB环境信息',
                    local: 'goldendb-00082.html'
                  },
                  {
                    id: 795,
                    parentId: 756,
                    name: '管理实例',
                    local: 'goldendb-00083.html'
                  },
                  {
                    id: 796,
                    parentId: 756,
                    name: '管理集群',
                    local: 'goldendb-00084.html'
                  }
                ]
              }
            ]
          },
          {
            id: 40,
            parentId: 13,
            name: 'GBase 8a数据保护',
            local: 'zh-cn_topic_0000002200094117.html',
            children: [
              {
                id: 797,
                parentId: 40,
                name: '备份',
                local: 'GBase_8a_00006.html',
                children: [
                  {
                    id: 806,
                    parentId: 797,
                    name: '备份前准备',
                    local: 'GBase_8a_00009.html'
                  },
                  {
                    id: 807,
                    parentId: 797,
                    name: '备份GBase 8a数据库',
                    local: 'GBase_8a_00010.html',
                    children: [
                      {
                        id: 808,
                        parentId: 807,
                        name: '步骤1：注册GBase 8a数据库',
                        local: 'GBase_8a_00011.html'
                      },
                      {
                        id: 809,
                        parentId: 807,
                        name: '步骤2：（可选）创建限速策略',
                        local: 'GBase_8a_00012.html'
                      },
                      {
                        id: 810,
                        parentId: 807,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'GBase_8a_00013.html'
                      },
                      {
                        id: 811,
                        parentId: 807,
                        name: '步骤4：创建备份SLA',
                        local: 'GBase_8a_00014.html'
                      },
                      {
                        id: 812,
                        parentId: 807,
                        name: '步骤5：执行备份',
                        local: 'GBase_8a_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 798,
                parentId: 40,
                name: '复制',
                local: 'oracle_gud_000035_3.html',
                children: [
                  {
                    id: 813,
                    parentId: 798,
                    name: '复制GBase 8a副本',
                    local: 'GBase_8a_0000191.html',
                    children: [
                      {
                        id: 814,
                        parentId: 813,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'GBase_8a_0000192.html'
                      },
                      {
                        id: 815,
                        parentId: 813,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'GBase_8a_0000193.html'
                      },
                      {
                        id: 816,
                        parentId: 813,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'GBase_8a_0000194.html'
                      },
                      {
                        id: 817,
                        parentId: 813,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'GBase_8a_0000195.html'
                      },
                      {
                        id: 818,
                        parentId: 813,
                        name: '步骤4：下载并导入证书',
                        local: 'GBase_8a_0000196.html'
                      },
                      {
                        id: 819,
                        parentId: 813,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'GBase_8a_0000197.html'
                      },
                      {
                        id: 820,
                        parentId: 813,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'GBase_8a_0000198.html'
                      },
                      {
                        id: 821,
                        parentId: 813,
                        name: '步骤6：添加复制集群',
                        local: 'GBase_8a_0000199.html'
                      },
                      {
                        id: 822,
                        parentId: 813,
                        name: '步骤7：创建复制SLA',
                        local: 'GBase_8a_0000200.html'
                      },
                      {
                        id: 823,
                        parentId: 813,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'GBase_8a_0000201.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 799,
                parentId: 40,
                name: '归档',
                local: 'GBase_8a_00030.html',
                children: [
                  {
                    id: 824,
                    parentId: 799,
                    name: '归档GBase 8a备份副本',
                    local: 'GBase_8a_00033.html',
                    children: [
                      {
                        id: 826,
                        parentId: 824,
                        name: '步骤1：添加归档存储',
                        local: 'GBase_8a_00034.html',
                        children: [
                          {
                            id: 828,
                            parentId: 826,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'GBase_8a_00035.html'
                          },
                          {
                            id: 829,
                            parentId: 826,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'GBase_8a_00036.html'
                          }
                        ]
                      },
                      {
                        id: 827,
                        parentId: 824,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'GBase_8a_00037.html'
                      }
                    ]
                  },
                  {
                    id: 825,
                    parentId: 799,
                    name: '归档GBase 8a复制副本',
                    local: 'GBase_8a_00038.html',
                    children: [
                      {
                        id: 830,
                        parentId: 825,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'GBase_8a_00039.html'
                      },
                      {
                        id: 831,
                        parentId: 825,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'GBase_8a_00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 800,
                parentId: 40,
                name: '恢复',
                local: 'GBase_8a_00041.html',
                children: [
                  {
                    id: 832,
                    parentId: 800,
                    name: '恢复GBase 8a数据库',
                    local: 'GBase_8a_00044.html'
                  }
                ]
              },
              {
                id: 801,
                parentId: 40,
                name: '全局搜索',
                local: 'GBase_8a_000411.html',
                children: [
                  {
                    id: 833,
                    parentId: 801,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'GBase_8a_000451.html'
                  }
                ]
              },
              {
                id: 802,
                parentId: 40,
                name: 'SLA',
                local: 'GBase_8a_00048.html',
                children: [
                  {
                    id: 834,
                    parentId: 802,
                    name: '关于SLA',
                    local: 'GBase_8a_00049.html'
                  },
                  {
                    id: 835,
                    parentId: 802,
                    name: '查看SLA信息',
                    local: 'GBase_8a_00050.html'
                  },
                  {
                    id: 836,
                    parentId: 802,
                    name: '管理SLA',
                    local: 'GBase_8a_00051.html'
                  }
                ]
              },
              {
                id: 803,
                parentId: 40,
                name: '副本',
                local: 'GBase_8a_00052.html',
                children: [
                  {
                    id: 837,
                    parentId: 803,
                    name: '查看GBase 8a副本信息',
                    local: 'GBase_8a_00053.html'
                  },
                  {
                    id: 838,
                    parentId: 803,
                    name: '管理GBase 8a副本',
                    local: 'GBase_8a_00054.html'
                  }
                ]
              },
              {
                id: 804,
                parentId: 40,
                name: 'GBase 8a数据库环境',
                local: 'GBase_8a_00055.html',
                children: [
                  {
                    id: 839,
                    parentId: 804,
                    name: '查看GBase 8a数据库环境信息',
                    local: 'GBase_8a_00056.html'
                  },
                  {
                    id: 840,
                    parentId: 804,
                    name: '管理数据库',
                    local: 'GBase_8a_00057.html'
                  }
                ]
              },
              {
                id: 805,
                parentId: 40,
                name: '常见问题',
                local: 'GBase_8a_00058.html',
                children: [
                  {
                    id: 841,
                    parentId: 805,
                    name: '查看GBase服务状态',
                    local: 'GBase_8a_00061.html'
                  },
                  {
                    id: 842,
                    parentId: 805,
                    name: '数据库拓扑结构不一致导致GBase恢复任务失败',
                    local: 'GBase_8a_00161.html'
                  },
                  {
                    id: 843,
                    parentId: 805,
                    name: '本产品升级后无法注册GBase 8a数据库资源',
                    local: 'GBase_8a_00163.html'
                  }
                ]
              }
            ]
          },
          {
            id: 41,
            parentId: 13,
            name: 'SAP HANA数据保护',
            local: 'zh-cn_topic_0000002200008545.html',
            children: [
              {
                id: 844,
                parentId: 41,
                name: '备份',
                local: 'SAP_HANA_0007.html',
                children: [
                  {
                    id: 855,
                    parentId: 844,
                    name: '备份前准备',
                    local: 'SAP_HANA_0010.html'
                  },
                  {
                    id: 856,
                    parentId: 844,
                    name: '备份SAP HANA数据库（通用数据库入口）',
                    local: 'SAP_HANA_0011.html',
                    children: [
                      {
                        id: 858,
                        parentId: 856,
                        name: '步骤1：注册SAP HANA数据库（File备份方式）',
                        local: 'SAP_HANA_0012.html',
                        children: [
                          {
                            id: 864,
                            parentId: 858,
                            name: '注册系统数据库',
                            local: 'SAP_HANA_0012-1.html'
                          },
                          {
                            id: 865,
                            parentId: 858,
                            name: '注册租户数据库',
                            local: 'SAP_HANA_0012-2.html'
                          }
                        ]
                      },
                      {
                        id: 859,
                        parentId: 856,
                        name: '步骤2：（可选）创建限速策略',
                        local: 'SAP_HANA_0013.html'
                      },
                      {
                        id: 860,
                        parentId: 856,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'SAP_HANA_0014.html'
                      },
                      {
                        id: 861,
                        parentId: 856,
                        name: '步骤4：配置日志备份',
                        local: 'SAP_HANA_0015.html'
                      },
                      {
                        id: 862,
                        parentId: 856,
                        name: '步骤5：创建备份SLA',
                        local: 'SAP_HANA_0017.html'
                      },
                      {
                        id: 863,
                        parentId: 856,
                        name: '步骤6：执行备份',
                        local: 'SAP_HANA_0018.html'
                      }
                    ]
                  },
                  {
                    id: 857,
                    parentId: 844,
                    name:
                      '备份SAP HANA数据库（SAP HANA应用入口）（适用于1.6.0及后续版本）',
                    local: 'SAP_HANA_0019.html',
                    children: [
                      {
                        id: 866,
                        parentId: 857,
                        name: '步骤1：注册SAP HANA数据库（Backint备份方式）',
                        local: 'SAP_HANA_0020.html',
                        children: [
                          {
                            id: 870,
                            parentId: 866,
                            name: '注册系统数据库',
                            local: 'SAP_HANA_0020-1.html'
                          },
                          {
                            id: 871,
                            parentId: 866,
                            name: '注册租户数据库',
                            local: 'SAP_HANA_0020-2.html'
                          }
                        ]
                      },
                      {
                        id: 867,
                        parentId: 857,
                        name: '步骤2：（可选）创建限速策略',
                        local: 'SAP_HANA_0021.html'
                      },
                      {
                        id: 868,
                        parentId: 857,
                        name: '步骤4：创建备份SLA',
                        local: 'SAP_HANA_0024.html'
                      },
                      {
                        id: 869,
                        parentId: 857,
                        name: '步骤5：执行备份',
                        local: 'SAP_HANA_0025.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 845,
                parentId: 41,
                name: '复制',
                local: 'SAP_HANA_0026.html',
                children: [
                  {
                    id: 872,
                    parentId: 845,
                    name: '复制SAP HANA数据库副本',
                    local: 'SAP_HANA_0030.html',
                    children: [
                      {
                        id: 873,
                        parentId: 872,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'SAP_HANA_0031.html'
                      },
                      {
                        id: 874,
                        parentId: 872,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'SAP_HANA_0032.html'
                      },
                      {
                        id: 875,
                        parentId: 872,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'SAP_HANA_0033.html'
                      },
                      {
                        id: 876,
                        parentId: 872,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'SAP_HANA_0034.html'
                      },
                      {
                        id: 877,
                        parentId: 872,
                        name: '步骤4：下载并导入证书',
                        local: 'SAP_HANA_0035.html'
                      },
                      {
                        id: 878,
                        parentId: 872,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'SAP_HANA_0036.html'
                      },
                      {
                        id: 879,
                        parentId: 872,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'SAP_HANA_0037.html'
                      },
                      {
                        id: 880,
                        parentId: 872,
                        name: '步骤6：添加复制集群',
                        local: 'SAP_HANA_0038.html'
                      },
                      {
                        id: 881,
                        parentId: 872,
                        name: '步骤7：创建复制SLA',
                        local: 'SAP_HANA_0039.html'
                      },
                      {
                        id: 882,
                        parentId: 872,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'SAP_HANA_0040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 846,
                parentId: 41,
                name: '归档',
                local: 'SAP_HANA_0060.html',
                children: [
                  {
                    id: 883,
                    parentId: 846,
                    name: '归档SAP HANA备份副本',
                    local: 'SAP_HANA_0063.html',
                    children: [
                      {
                        id: 885,
                        parentId: 883,
                        name: '步骤1：添加归档存储',
                        local: 'SAP_HANA_0064.html',
                        children: [
                          {
                            id: 887,
                            parentId: 885,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'SAP_HANA_0065.html'
                          },
                          {
                            id: 888,
                            parentId: 885,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'SAP_HANA_0066.html'
                          }
                        ]
                      },
                      {
                        id: 886,
                        parentId: 883,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'SAP_HANA_0067.html'
                      }
                    ]
                  },
                  {
                    id: 884,
                    parentId: 846,
                    name: '归档SAP HANA复制副本',
                    local: 'SAP_HANA_0068.html',
                    children: [
                      {
                        id: 889,
                        parentId: 884,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'SAP_HANA_0069.html'
                      },
                      {
                        id: 890,
                        parentId: 884,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'SAP_HANA_0070.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 847,
                parentId: 41,
                name: '恢复',
                local: 'SAP_HANA_0072.html',
                children: [
                  {
                    id: 891,
                    parentId: 847,
                    name: '恢复SAP HANA数据库（通用数据库入口）',
                    local: 'SAP_HANA_0075.html'
                  },
                  {
                    id: 892,
                    parentId: 847,
                    name:
                      '恢复SAP HANA数据库（SAP HANA应用入口，适用于1.6.0及后续版本）',
                    local: 'SAP_HANA_0076.html'
                  }
                ]
              },
              {
                id: 848,
                parentId: 41,
                name: '全局搜索',
                local: 'SAP_HANA_0077.html',
                children: [
                  {
                    id: 893,
                    parentId: 848,
                    name: '全局搜索资源',
                    local: 'SAP_HANA_0078.html'
                  },
                  {
                    id: 894,
                    parentId: 848,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'SAP_HANA_0079.html'
                  }
                ]
              },
              {
                id: 849,
                parentId: 41,
                name: 'SLA',
                local: 'SAP_HANA_0083.html',
                children: [
                  {
                    id: 895,
                    parentId: 849,
                    name: '关于SLA',
                    local: 'SAP_HANA_0084.html'
                  },
                  {
                    id: 896,
                    parentId: 849,
                    name: '查看SLA信息',
                    local: 'SAP_HANA_0085.html'
                  },
                  {
                    id: 897,
                    parentId: 849,
                    name: '管理SLA',
                    local: 'SAP_HANA_0086.html'
                  }
                ]
              },
              {
                id: 850,
                parentId: 41,
                name: '副本（通用数据库入口）',
                local: 'SAP_HANA_0087.html',
                children: [
                  {
                    id: 898,
                    parentId: 850,
                    name: '查看SAP HANA副本信息',
                    local: 'SAP_HANA_0088.html'
                  },
                  {
                    id: 899,
                    parentId: 850,
                    name: '管理SAP HANA副本',
                    local: 'SAP_HANA_0089.html'
                  }
                ]
              },
              {
                id: 851,
                parentId: 41,
                name: '副本（SAP HANA应用入口，适用于1.6.0及后续版本）',
                local: 'SAP_HANA_0090.html',
                children: [
                  {
                    id: 900,
                    parentId: 851,
                    name: '查看SAP HANA副本信息',
                    local: 'SAP_HANA_0091.html'
                  },
                  {
                    id: 901,
                    parentId: 851,
                    name: '管理SAP HANA副本',
                    local: 'SAP_HANA_0092.html'
                  }
                ]
              },
              {
                id: 852,
                parentId: 41,
                name: 'SAP HANA数据库环境（通用数据库入口）',
                local: 'SAP_HANA_0093.html',
                children: [
                  {
                    id: 902,
                    parentId: 852,
                    name: '查看SAP HANA数据库环境信息',
                    local: 'SAP_HANA_0094.html'
                  },
                  {
                    id: 903,
                    parentId: 852,
                    name: '管理数据库',
                    local: 'SAP_HANA_0095.html'
                  }
                ]
              },
              {
                id: 853,
                parentId: 41,
                name:
                  'SAP HANA数据库环境（SAP HANA应用入口，适用于1.6.0及后续版本）',
                local: 'SAP_HANA_0096.html',
                children: [
                  {
                    id: 904,
                    parentId: 853,
                    name: '查看SAP HANA数据库环境信息',
                    local: 'SAP_HANA_0097.html'
                  },
                  {
                    id: 905,
                    parentId: 853,
                    name: '管理实例',
                    local: 'SAP_HANA_0098.html'
                  },
                  {
                    id: 906,
                    parentId: 853,
                    name: '管理数据库',
                    local: 'SAP_HANA_0099.html'
                  }
                ]
              },
              {
                id: 854,
                parentId: 41,
                name: '常见问题',
                local: 'SAP_HANA_0100.html',
                children: [
                  {
                    id: 907,
                    parentId: 854,
                    name: '登录DeviceManager管理界面',
                    local: 'SAP_HANA_0102.html'
                  },
                  {
                    id: 908,
                    parentId: 854,
                    name: '恢复SAP HANA数据库时，恢复子任务长时间无进度',
                    local: 'SAP_HANA_0105.html'
                  },
                  {
                    id: 909,
                    parentId: 854,
                    name: '本产品管理界面查看副本数据时，系统提示"操作失败"',
                    local: 'SAP_HANA_0106.html'
                  },
                  {
                    id: 910,
                    parentId: 854,
                    name: 'IO通道数过小导致SAP HANA备份或恢复任务失败',
                    local: 'SAP_HANA_0107.html'
                  },
                  {
                    id: 911,
                    parentId: 854,
                    name: 'SAP HANA由于内存不足，导致备份或恢复任务失败',
                    local: 'SAP_HANA_0108.html'
                  },
                  {
                    id: 912,
                    parentId: 854,
                    name: '生产环境异步IO请求数达到最大值，导致备份失败',
                    local: 'SAP_HANA_0109.html'
                  },
                  {
                    id: 913,
                    parentId: 854,
                    name: 'SAP HANA备份过程中由于存储空间不足，导致备份失败',
                    local: 'SAP_HANA_0110.html'
                  },
                  {
                    id: 914,
                    parentId: 854,
                    name:
                      '将SAP HANA数据库从应用重新注册为通用数据库后，备份失败',
                    local: 'SAP_HANA_0111.html'
                  },
                  {
                    id: 915,
                    parentId: 854,
                    name:
                      '备份SAP HANA数据库时，备份子任务长时间无进度且已备份数据量长时间无变化',
                    local: 'SAP_HANA_0112.html'
                  },
                  {
                    id: 916,
                    parentId: 854,
                    name: '如何为系统数据库和租户数据库创建新的用户',
                    local: 'SAP_HANA_0112-1.html'
                  }
                ]
              }
            ]
          },
          {
            id: 42,
            parentId: 13,
            name: 'AntDB数据保护',
            local: 'zh-cn_topic_0000002174193646.html',
            children: [
              {
                id: 917,
                parentId: 42,
                name: '备份',
                local: 'antdb-0008.html',
                children: [
                  {
                    id: 926,
                    parentId: 917,
                    name: '备份前准备',
                    local: 'antdb-0011.html'
                  },
                  {
                    id: 927,
                    parentId: 917,
                    name: '备份AntDB',
                    local: 'antdb-0012.html',
                    children: [
                      {
                        id: 928,
                        parentId: 927,
                        name: '步骤1：开启归档模式',
                        local: 'antdb-0013.html'
                      },
                      {
                        id: 929,
                        parentId: 927,
                        name: '步骤2：注册AntDB单实例',
                        local: 'antdb-0014.html'
                      },
                      {
                        id: 930,
                        parentId: 927,
                        name: '步骤3：注册AntDB集群实例',
                        local: 'antdb-0015.html'
                      },
                      {
                        id: 931,
                        parentId: 927,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'antdb-0016.html'
                      },
                      {
                        id: 932,
                        parentId: 927,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'antdb-0017.html'
                      },
                      {
                        id: 933,
                        parentId: 927,
                        name: '步骤6：创建备份SLA',
                        local: 'antdb-0018.html'
                      },
                      {
                        id: 934,
                        parentId: 927,
                        name: '步骤7：执行备份',
                        local: 'antdb-0019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 918,
                parentId: 42,
                name: '复制',
                local: 'antdb-0022.html',
                children: [
                  {
                    id: 935,
                    parentId: 918,
                    name: '复制AntDB数据库副本',
                    local: 'antdb-0025.html',
                    children: [
                      {
                        id: 936,
                        parentId: 935,
                        name: '步骤1：创建复制网络逻辑端口',
                        local: 'antdb-0028.html'
                      },
                      {
                        id: 937,
                        parentId: 935,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'antdb-0029.html'
                      },
                      {
                        id: 938,
                        parentId: 935,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'antdb-0030.html'
                      },
                      {
                        id: 939,
                        parentId: 935,
                        name: '步骤4：下载并导入证书',
                        local: 'antdb-0031.html'
                      },
                      {
                        id: 940,
                        parentId: 935,
                        name: '步骤5：创建远端设备管理员',
                        local: 'antdb-0032.html'
                      },
                      {
                        id: 941,
                        parentId: 935,
                        name: '步骤6：添加复制集群',
                        local: 'antdb-0033.html'
                      },
                      {
                        id: 942,
                        parentId: 935,
                        name: '步骤7：创建复制SLA',
                        local: 'antdb-0034.html'
                      },
                      {
                        id: 943,
                        parentId: 935,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'antdb-0035.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 919,
                parentId: 42,
                name: '归档',
                local: 'antdb-0036.html',
                children: [
                  {
                    id: 944,
                    parentId: 919,
                    name: '归档AntDB备份副本',
                    local: 'antdb-0039.html',
                    children: [
                      {
                        id: 946,
                        parentId: 944,
                        name: '步骤1：添加归档存储',
                        local: 'antdb-0040.html',
                        children: [
                          {
                            id: 948,
                            parentId: 946,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'antdb-0041.html'
                          },
                          {
                            id: 949,
                            parentId: 946,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'antdb-0042.html'
                          }
                        ]
                      },
                      {
                        id: 947,
                        parentId: 944,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'antdb-0043.html'
                      }
                    ]
                  },
                  {
                    id: 945,
                    parentId: 919,
                    name: '归档AntDB复制副本',
                    local: 'antdb-0044.html',
                    children: [
                      {
                        id: 950,
                        parentId: 945,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'antdb-0045.html'
                      },
                      {
                        id: 951,
                        parentId: 945,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'antdb-0046.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 920,
                parentId: 42,
                name: '恢复',
                local: 'antdb-0047.html',
                children: [
                  {
                    id: 952,
                    parentId: 920,
                    name: '恢复AntDB',
                    local: 'antdb-0050.html'
                  }
                ]
              },
              {
                id: 921,
                parentId: 42,
                name: '全局搜索',
                local: 'antdb-0051.html',
                children: [
                  {
                    id: 953,
                    parentId: 921,
                    name: '全局搜索资源',
                    local: 'antdb-0053.html'
                  },
                  {
                    id: 954,
                    parentId: 921,
                    name: '全局标签搜索',
                    local: 'antdb-0054.html'
                  }
                ]
              },
              {
                id: 922,
                parentId: 42,
                name: 'SLA',
                local: 'antdb-0059.html',
                children: [
                  {
                    id: 955,
                    parentId: 922,
                    name: '关于SLA',
                    local: 'antdb-0060.html'
                  },
                  {
                    id: 956,
                    parentId: 922,
                    name: '查看SLA信息',
                    local: 'antdb-0061.html'
                  },
                  {
                    id: 957,
                    parentId: 922,
                    name: '管理SLA',
                    local: 'antdb-0062.html'
                  }
                ]
              },
              {
                id: 923,
                parentId: 42,
                name: '副本',
                local: 'antdb-0063.html',
                children: [
                  {
                    id: 958,
                    parentId: 923,
                    name: '查看AntDB副本信息',
                    local: 'antdb-0064.html'
                  },
                  {
                    id: 959,
                    parentId: 923,
                    name: '管理AntDB副本',
                    local: 'antdb-0065.html'
                  }
                ]
              },
              {
                id: 924,
                parentId: 42,
                name: 'AntDB实例环境',
                local: 'antdb-0066.html',
                children: [
                  {
                    id: 960,
                    parentId: 924,
                    name: '查看AntDB环境信息',
                    local: 'antdb-0067.html'
                  },
                  {
                    id: 961,
                    parentId: 924,
                    name: '管理AntDB实例',
                    local: 'antdb-0068.html'
                  }
                ]
              },
              {
                id: 925,
                parentId: 42,
                name: '常见问题',
                local: 'antdb-0069.html',
                children: [
                  {
                    id: 962,
                    parentId: 925,
                    name: '登录DeviceManager管理界面',
                    local: 'antdb-0071.html'
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
        local: 'zh-cn_topic_0000002200094133.html',
        children: [
          {
            id: 963,
            parentId: 14,
            name: 'ClickHouse数据保护',
            local: 'zh-cn_topic_0000002200008533.html',
            children: [
              {
                id: 971,
                parentId: 963,
                name: '备份',
                local: 'clickhouse-0003.html',
                children: [
                  {
                    id: 981,
                    parentId: 971,
                    name: '备份前准备',
                    local: 'clickhouse-0006.html'
                  },
                  {
                    id: 982,
                    parentId: 971,
                    name: '备份ClickHouse数据库/ClickHouse表集',
                    local: 'clickhouse-0007.html',
                    children: [
                      {
                        id: 983,
                        parentId: 982,
                        name: '步骤1：注册ClickHouse集群',
                        local: 'clickhouse-0008.html'
                      },
                      {
                        id: 984,
                        parentId: 982,
                        name: '步骤2：创建ClickHouse表集',
                        local: 'clickhouse-0009.html'
                      },
                      {
                        id: 985,
                        parentId: 982,
                        name: '步骤3：（可选）创建限速策略',
                        local: 'clickhouse-0010.html'
                      },
                      {
                        id: 986,
                        parentId: 982,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'clickhouse-0011.html'
                      },
                      {
                        id: 987,
                        parentId: 982,
                        name: '步骤5：创建备份SLA',
                        local: 'clickhouse-0012.html'
                      },
                      {
                        id: 988,
                        parentId: 982,
                        name: '步骤6：执行备份',
                        local: 'clickhouse-0013.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 972,
                parentId: 963,
                name: '复制',
                local: 'clickhouse-00085.html',
                children: [
                  {
                    id: 989,
                    parentId: 972,
                    name: '复制ClickHouse副本',
                    local: 'clickhouse-0018.html',
                    children: [
                      {
                        id: 990,
                        parentId: 989,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'clickhouse-00087.html'
                      },
                      {
                        id: 991,
                        parentId: 989,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'clickhouse-00088.html'
                      },
                      {
                        id: 992,
                        parentId: 989,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'clickhouse-0021.html'
                      },
                      {
                        id: 993,
                        parentId: 989,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'clickhouse-0022.html'
                      },
                      {
                        id: 994,
                        parentId: 989,
                        name: '步骤4：下载并导入证书',
                        local: 'clickhouse-0023.html'
                      },
                      {
                        id: 995,
                        parentId: 989,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'clickhouse-0024.html'
                      },
                      {
                        id: 996,
                        parentId: 989,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'clickhouse-00073.html'
                      },
                      {
                        id: 997,
                        parentId: 989,
                        name: '步骤6：添加复制集群',
                        local: 'clickhouse-0025.html'
                      },
                      {
                        id: 998,
                        parentId: 989,
                        name: '步骤7：创建复制SLA',
                        local: 'clickhouse-00089.html'
                      },
                      {
                        id: 999,
                        parentId: 989,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'clickhouse-0027.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 973,
                parentId: 963,
                name: '归档',
                local: 'clickhouse-0028.html',
                children: [
                  {
                    id: 1000,
                    parentId: 973,
                    name: '归档ClickHouse备份副本',
                    local: 'clickhouse-0031.html',
                    children: [
                      {
                        id: 1002,
                        parentId: 1000,
                        name: '步骤1：添加归档存储',
                        local: 'clickhouse-0032.html',
                        children: [
                          {
                            id: 1004,
                            parentId: 1002,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'clickhouse-0033.html'
                          },
                          {
                            id: 1005,
                            parentId: 1002,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'clickhouse-0034.html'
                          }
                        ]
                      },
                      {
                        id: 1003,
                        parentId: 1000,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'clickhouse-0035.html'
                      }
                    ]
                  },
                  {
                    id: 1001,
                    parentId: 973,
                    name: '归档ClickHouse复制副本',
                    local: 'clickhouse-0036.html',
                    children: [
                      {
                        id: 1006,
                        parentId: 1001,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'clickhouse-0037.html'
                      },
                      {
                        id: 1007,
                        parentId: 1001,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'clickhouse-0038.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 974,
                parentId: 963,
                name: '恢复',
                local: 'clickhouse-0039.html',
                children: [
                  {
                    id: 1008,
                    parentId: 974,
                    name: '恢复ClickHouse数据库/表集',
                    local: 'clickhouse-0042.html'
                  }
                ]
              },
              {
                id: 975,
                parentId: 963,
                name: '全局搜索',
                local: 'clickhouse-00083.html',
                children: [
                  {
                    id: 1009,
                    parentId: 975,
                    name: '全局搜索资源',
                    local: 'clickhouse-0043.html'
                  },
                  {
                    id: 1010,
                    parentId: 975,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'clickhouse-00084.html'
                  }
                ]
              },
              {
                id: 976,
                parentId: 963,
                name: '数据重删压缩',
                local: 'clickhouse-0044.html',
                children: [
                  {
                    id: 1011,
                    parentId: 976,
                    name: '关于数据重删压缩',
                    local: 'clickhouse-0045.html'
                  }
                ]
              },
              {
                id: 977,
                parentId: 963,
                name: 'SLA',
                local: 'clickhouse-0046.html',
                children: [
                  {
                    id: 1012,
                    parentId: 977,
                    name: '关于SLA',
                    local: 'clickhouse-0047.html'
                  },
                  {
                    id: 1013,
                    parentId: 977,
                    name: '查看SLA信息',
                    local: 'clickhouse-0048.html'
                  },
                  {
                    id: 1014,
                    parentId: 977,
                    name: '管理SLA',
                    local: 'clickhouse-0049.html'
                  }
                ]
              },
              {
                id: 978,
                parentId: 963,
                name: '副本',
                local: 'clickhouse-0050.html',
                children: [
                  {
                    id: 1015,
                    parentId: 978,
                    name: '查看ClickHouse副本信息',
                    local: 'clickhouse-0051.html'
                  },
                  {
                    id: 1016,
                    parentId: 978,
                    name: '管理ClickHouse副本',
                    local: 'clickhouse-0052.html'
                  }
                ]
              },
              {
                id: 979,
                parentId: 963,
                name: 'ClickHouse集群环境',
                local: 'clickhouse-0053.html',
                children: [
                  {
                    id: 1017,
                    parentId: 979,
                    name: '查看ClickHouse环境信息',
                    local: 'clickhouse-0054.html'
                  },
                  {
                    id: 1018,
                    parentId: 979,
                    name: '管理ClickHouse集群/表集',
                    local: 'clickhouse-0055.html'
                  },
                  {
                    id: 1019,
                    parentId: 979,
                    name: '管理ClickHouse数据库/表集保护',
                    local: 'clickhouse-0056.html'
                  }
                ]
              },
              {
                id: 980,
                parentId: 963,
                name: '常见问题',
                local: 'clickhouse-0057.html',
                children: [
                  {
                    id: 1020,
                    parentId: 980,
                    name: '登录DeviceManager管理界面',
                    local: 'clickhouse-00085_0.html'
                  },
                  {
                    id: 1021,
                    parentId: 980,
                    name:
                      'ClickHouse和Redis合并部署场景，偶现ClickHouse节点离线、备份或恢复失败',
                    local: 'clickhouse-0060.html'
                  },
                  {
                    id: 1022,
                    parentId: 980,
                    name: 'MRS生产环境密码过期后，更新认证文件',
                    local: 'clickhouse-0061.html'
                  }
                ]
              }
            ]
          },
          {
            id: 964,
            parentId: 14,
            name: 'GaussDB(DWS)数据保护',
            local: 'product_documentation_000029.html',
            children: [
              {
                id: 1023,
                parentId: 964,
                name: '备份',
                local: 'DWS_00016.html',
                children: [
                  {
                    id: 1032,
                    parentId: 1023,
                    name: '备份前准备',
                    local: 'DWS_00019.html'
                  },
                  {
                    id: 1033,
                    parentId: 1023,
                    name: '备份GaussDB(DWS)',
                    local: 'DWS_00020.html',
                    children: [
                      {
                        id: 1034,
                        parentId: 1033,
                        name: '步骤1：注册GaussDB(DWS)集群',
                        local: 'DWS_00024.html'
                      },
                      {
                        id: 1035,
                        parentId: 1033,
                        name: '步骤2：创建GaussDB(DWS)Schema集/表集',
                        local: 'DWS_00025.html'
                      },
                      {
                        id: 1036,
                        parentId: 1033,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'DWS_00026.html'
                      },
                      {
                        id: 1037,
                        parentId: 1033,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'DWS_00027.html'
                      },
                      {
                        id: 1038,
                        parentId: 1033,
                        name: '步骤5：创建备份SLA',
                        local: 'DWS_00028.html'
                      },
                      {
                        id: 1039,
                        parentId: 1033,
                        name: '步骤6：执行备份',
                        local: 'DWS_00029.html',
                        children: [
                          {
                            id: 1040,
                            parentId: 1039,
                            name: '执行集群备份',
                            local: 'DWS_00030.html'
                          },
                          {
                            id: 1041,
                            parentId: 1039,
                            name: '执行schema集备份',
                            local: 'DWS_00031.html'
                          },
                          {
                            id: 1042,
                            parentId: 1039,
                            name: '执行表集备份',
                            local: 'DWS_00032.html'
                          }
                        ]
                      }
                    ]
                  }
                ]
              },
              {
                id: 1024,
                parentId: 964,
                name: '复制',
                local: 'DWS_00033.html',
                children: [
                  {
                    id: 1043,
                    parentId: 1024,
                    name: '复制GaussDB(DWS)备份副本',
                    local: 'DWS_00037.html',
                    children: [
                      {
                        id: 1044,
                        parentId: 1043,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'DWS_00038.html'
                      },
                      {
                        id: 1045,
                        parentId: 1043,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'DWS_00039.html'
                      },
                      {
                        id: 1046,
                        parentId: 1043,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'DWS_00040.html'
                      },
                      {
                        id: 1047,
                        parentId: 1043,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'DWS_00041.html'
                      },
                      {
                        id: 1048,
                        parentId: 1043,
                        name: '步骤4：下载并导入证书',
                        local: 'DWS_00042.html'
                      },
                      {
                        id: 1049,
                        parentId: 1043,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'DWS_00043.html'
                      },
                      {
                        id: 1050,
                        parentId: 1043,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'DWS_00044.html'
                      },
                      {
                        id: 1051,
                        parentId: 1043,
                        name: '步骤6：添加复制集群',
                        local: 'DWS_00045.html'
                      },
                      {
                        id: 1052,
                        parentId: 1043,
                        name: '步骤7：创建复制SLA',
                        local: 'DWS_00046.html'
                      },
                      {
                        id: 1053,
                        parentId: 1043,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'DWS_00047.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1025,
                parentId: 964,
                name: '归档',
                local: 'DWS_00067.html',
                children: [
                  {
                    id: 1054,
                    parentId: 1025,
                    name: '归档GaussDB(DWS)备份副本',
                    local: 'DWS_00070.html',
                    children: [
                      {
                        id: 1056,
                        parentId: 1054,
                        name: '步骤1：添加归档存储',
                        local: 'DWS_00071.html',
                        children: [
                          {
                            id: 1058,
                            parentId: 1056,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'DWS_00072.html'
                          },
                          {
                            id: 1059,
                            parentId: 1056,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'DWS_00073.html'
                          }
                        ]
                      },
                      {
                        id: 1057,
                        parentId: 1054,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'DWS_00074.html'
                      }
                    ]
                  },
                  {
                    id: 1055,
                    parentId: 1025,
                    name: '归档GaussDB(DWS)复制副本',
                    local: 'DWS_00075.html',
                    children: [
                      {
                        id: 1060,
                        parentId: 1055,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'DWS_00076.html'
                      },
                      {
                        id: 1061,
                        parentId: 1055,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'DWS_00077.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1026,
                parentId: 964,
                name: '恢复',
                local: 'DWS_00078.html',
                children: [
                  {
                    id: 1062,
                    parentId: 1026,
                    name: '恢复前准备',
                    local: 'DWS_00080.html'
                  },
                  {
                    id: 1063,
                    parentId: 1026,
                    name: '恢复GaussDB(DWS)',
                    local: 'DWS_00082.html',
                    children: [
                      {
                        id: 1064,
                        parentId: 1063,
                        name: '恢复集群',
                        local: 'DWS_00085.html'
                      },
                      {
                        id: 1065,
                        parentId: 1063,
                        name: '恢复单个或多个表',
                        local: 'DWS_00086.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1027,
                parentId: 964,
                name: '全局搜索',
                local: 'DWS_00087.html',
                children: [
                  {
                    id: 1066,
                    parentId: 1027,
                    name: '全局搜索资源',
                    local: 'DWS_00088.html'
                  },
                  {
                    id: 1067,
                    parentId: 1027,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'DWS_00089.html'
                  }
                ]
              },
              {
                id: 1028,
                parentId: 964,
                name: 'SLA',
                local: 'DWS_00092.html',
                children: [
                  {
                    id: 1068,
                    parentId: 1028,
                    name: '关于SLA',
                    local: 'DWS_00093.html'
                  },
                  {
                    id: 1069,
                    parentId: 1028,
                    name: '查看SLA信息',
                    local: 'DWS_00094.html'
                  },
                  {
                    id: 1070,
                    parentId: 1028,
                    name: '管理SLA',
                    local: 'DWS_00095.html'
                  }
                ]
              },
              {
                id: 1029,
                parentId: 964,
                name: '副本',
                local: 'DWS_00096.html',
                children: [
                  {
                    id: 1071,
                    parentId: 1029,
                    name: '查看GaussDB(DWS)副本信息',
                    local: 'DWS_00097.html'
                  },
                  {
                    id: 1072,
                    parentId: 1029,
                    name: '管理GaussDB(DWS)副本',
                    local: 'DWS_00098.html'
                  }
                ]
              },
              {
                id: 1030,
                parentId: 964,
                name: 'GaussDB(DWS)集群环境',
                local: 'DWS_00099.html',
                children: [
                  {
                    id: 1073,
                    parentId: 1030,
                    name: '查询GaussDB(DWS)信息',
                    local: 'DWS_00100.html'
                  },
                  {
                    id: 1074,
                    parentId: 1030,
                    name: '管理GaussDB(DWS)集群',
                    local: 'DWS_00101.html'
                  },
                  {
                    id: 1075,
                    parentId: 1030,
                    name: '管理GaussDB(DWS)',
                    local: 'DWS_00102.html'
                  }
                ]
              },
              {
                id: 1031,
                parentId: 964,
                name: '常见问题',
                local: 'DWS_00109.html',
                children: [
                  {
                    id: 1076,
                    parentId: 1031,
                    name: '登录DeviceManager管理界面',
                    local: 'DWS_00111.html'
                  },
                  {
                    id: 1077,
                    parentId: 1031,
                    name: 'GaussDB(DWS)执行Roach备份失败',
                    local: 'DWS_00114.html'
                  },
                  {
                    id: 1078,
                    parentId: 1031,
                    name:
                      '集群副本恢复到新集群，因临时目录“/tmp/omm_mppdb”不存在，恢复失败',
                    local: 'DWS_00115.html'
                  },
                  {
                    id: 1079,
                    parentId: 1031,
                    name:
                      '集群副本恢复到新集群，因下发恢复命令的节点解析结果出错，恢复任务失败',
                    local: 'DWS_00116.html'
                  },
                  {
                    id: 1080,
                    parentId: 1031,
                    name: '配置客户端与本产品的全量映射关系',
                    local: 'DWS_00117.html'
                  },
                  {
                    id: 1081,
                    parentId: 1031,
                    name:
                      '对于GaussDB(DWS) 8.0.0版本，集群副本恢复到新集群，因下发恢复命令的节点拷贝cluster_static_config文件出错，恢复任务失败',
                    local: 'DWS_00119.html'
                  },
                  {
                    id: 1082,
                    parentId: 1031,
                    name:
                      '对于GaussDB(DWS) 8.1.1版本，执行恢复任务时，因网络通信异常，恢复任务失败',
                    local: 'DWS_00120.html'
                  },
                  {
                    id: 1083,
                    parentId: 1031,
                    name:
                      '对于GaussDB(DWS) 8.1.3版本，执行备份任务时，因GaussDB(DWS)软件处理异常，备份任务失败',
                    local: 'DWS_00121.html'
                  },
                  {
                    id: 1084,
                    parentId: 1031,
                    name: 'GDS运行在GaussDB(DWS)沙箱部署场景的注意事项',
                    local: 'DWS_00122.html',
                    children: [
                      {
                        id: 1091,
                        parentId: 1084,
                        name: 'GDS运行在GaussDB(DWS)节点上的注意事项',
                        local: 'DWS_00123.html'
                      },
                      {
                        id: 1092,
                        parentId: 1084,
                        name: 'GDS运行在其他节点的注意事项',
                        local: 'DWS_00124.html'
                      }
                    ]
                  },
                  {
                    id: 1085,
                    parentId: 1031,
                    name: '修改GaussDB(DWS)配置文件',
                    local: 'DWS_00125.html'
                  },
                  {
                    id: 1086,
                    parentId: 1031,
                    name: 'GaussDB(DWS)备份或恢复失败时，进行网络连通性检查',
                    local: 'DWS_00126.html'
                  },
                  {
                    id: 1087,
                    parentId: 1031,
                    name:
                      '客户端独立部署到生产主机场景、如何在生产主机上添加环境变量（适用于1.5.0SPC13及后续版本）',
                    local: 'DWS_00128.html'
                  },
                  {
                    id: 1088,
                    parentId: 1031,
                    name:
                      'GaussDB(DWS)全量备份失败后进行增量备份，增量备份同样无法成功',
                    local: 'DWS_00129.html'
                  },
                  {
                    id: 1089,
                    parentId: 1031,
                    name:
                      'GaussDB(DWS)副本过期或副本删除任务部分成功，导致已部署客户端的所有CN和DN节点数据残留。',
                    local: 'DWS_00129-1.html'
                  },
                  {
                    id: 1090,
                    parentId: 1031,
                    name: '如何创建DataTurbo逻辑端口',
                    local: 'DWS_00129-2.html'
                  }
                ]
              }
            ]
          },
          {
            id: 965,
            parentId: 14,
            name: 'HBase数据保护',
            local: 'product_documentation_000033.html',
            children: [
              {
                id: 1093,
                parentId: 965,
                name: '备份',
                local: 'hbase_00007.html',
                children: [
                  {
                    id: 1102,
                    parentId: 1093,
                    name: '备份前准备',
                    local: 'hbase_00010.html'
                  },
                  {
                    id: 1103,
                    parentId: 1093,
                    name: '备份HBase备份集',
                    local: 'hbase_00011.html',
                    children: [
                      {
                        id: 1104,
                        parentId: 1103,
                        name: '步骤1：注册HBase集群',
                        local: 'hbase_00012.html'
                      },
                      {
                        id: 1105,
                        parentId: 1103,
                        name: '步骤2：创建HBase备份集',
                        local: 'hbase_00013.html'
                      },
                      {
                        id: 1106,
                        parentId: 1103,
                        name: '步骤3：（可选）创建限速策略',
                        local: 'hbase_00014.html'
                      },
                      {
                        id: 1107,
                        parentId: 1103,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'hbase_00015.html'
                      },
                      {
                        id: 1108,
                        parentId: 1103,
                        name: '步骤5：创建备份SLA',
                        local: 'hbase_00016.html'
                      },
                      {
                        id: 1109,
                        parentId: 1103,
                        name: '步骤6：执行备份',
                        local: 'hbase_000017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1094,
                parentId: 965,
                name: '复制',
                local: 'hbase_00020.html',
                children: [
                  {
                    id: 1110,
                    parentId: 1094,
                    name: '复制HBase副本',
                    local: 'hbase_00023.html',
                    children: [
                      {
                        id: 1111,
                        parentId: 1110,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'hbase_00025.html'
                      },
                      {
                        id: 1112,
                        parentId: 1110,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'hbase_00097.html'
                      },
                      {
                        id: 1113,
                        parentId: 1110,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'hbase_00026.html'
                      },
                      {
                        id: 1114,
                        parentId: 1110,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'hbase_00027.html'
                      },
                      {
                        id: 1115,
                        parentId: 1110,
                        name: '步骤4：下载并导入证书',
                        local: 'hbase_00028.html'
                      },
                      {
                        id: 1116,
                        parentId: 1110,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'hbase_000029.html'
                      },
                      {
                        id: 1117,
                        parentId: 1110,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'hbase_00077.html'
                      },
                      {
                        id: 1118,
                        parentId: 1110,
                        name: '步骤6：添加复制集群',
                        local: 'hbase_00030.html'
                      },
                      {
                        id: 1119,
                        parentId: 1110,
                        name: '步骤7：创建复制SLA',
                        local: 'hbase_00031.html'
                      },
                      {
                        id: 1120,
                        parentId: 1110,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'hbase_00032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1095,
                parentId: 965,
                name: '归档',
                local: 'hbase_00033.html',
                children: [
                  {
                    id: 1121,
                    parentId: 1095,
                    name: '归档HBase备份副本',
                    local: 'hbase_00036.html',
                    children: [
                      {
                        id: 1123,
                        parentId: 1121,
                        name: '添加归档存储',
                        local: 'hbase_00037.html',
                        children: [
                          {
                            id: 1125,
                            parentId: 1123,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'hbase_00038.html'
                          },
                          {
                            id: 1126,
                            parentId: 1123,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'hbase_00039.html'
                          }
                        ]
                      },
                      {
                        id: 1124,
                        parentId: 1121,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'hbase_000040.html'
                      }
                    ]
                  },
                  {
                    id: 1122,
                    parentId: 1095,
                    name: '归档HBase复制副本',
                    local: 'hbase_00041.html',
                    children: [
                      {
                        id: 1127,
                        parentId: 1122,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'hbase_00042.html'
                      },
                      {
                        id: 1128,
                        parentId: 1122,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'hbase_00043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1096,
                parentId: 965,
                name: '恢复',
                local: 'hbase_00044.html',
                children: [
                  {
                    id: 1129,
                    parentId: 1096,
                    name: '恢复HBase备份集',
                    local: 'hbase_00047.html'
                  },
                  {
                    id: 1130,
                    parentId: 1096,
                    name: '恢复HBase备份集中的单个或多个表',
                    local: 'hbase_00048.html'
                  }
                ]
              },
              {
                id: 1097,
                parentId: 965,
                name: '全局搜索',
                local: 'hbase_00049.html',
                children: [
                  {
                    id: 1131,
                    parentId: 1097,
                    name: '全局搜索资源',
                    local: 'hbase_00050.html'
                  },
                  {
                    id: 1132,
                    parentId: 1097,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'hbase_00097_0.html'
                  }
                ]
              },
              {
                id: 1098,
                parentId: 965,
                name: 'SLA',
                local: 'hbase_00053.html',
                children: [
                  {
                    id: 1133,
                    parentId: 1098,
                    name: '关于SLA',
                    local: 'hbase_00054.html'
                  },
                  {
                    id: 1134,
                    parentId: 1098,
                    name: '查看SLA信息',
                    local: 'hbase_00055.html'
                  },
                  {
                    id: 1135,
                    parentId: 1098,
                    name: '管理SLA',
                    local: 'hbase_00056.html'
                  }
                ]
              },
              {
                id: 1099,
                parentId: 965,
                name: '副本',
                local: 'hbase_00057.html',
                children: [
                  {
                    id: 1136,
                    parentId: 1099,
                    name: '查看HBase副本信息',
                    local: 'hbase_00058.html'
                  },
                  {
                    id: 1137,
                    parentId: 1099,
                    name: '管理HBase副本',
                    local: 'hbase_00059.html'
                  }
                ]
              },
              {
                id: 1100,
                parentId: 965,
                name: 'HBase集群环境',
                local: 'hbase_00060.html',
                children: [
                  {
                    id: 1138,
                    parentId: 1100,
                    name: '查看HBase环境信息',
                    local: 'hbase_00061.html'
                  },
                  {
                    id: 1139,
                    parentId: 1100,
                    name: '管理HBase集群',
                    local: 'hbase_00062.html'
                  },
                  {
                    id: 1140,
                    parentId: 1100,
                    name: '管理备份集保护',
                    local: 'hbase_00063.html'
                  }
                ]
              },
              {
                id: 1101,
                parentId: 965,
                name: '常见问题',
                local: 'hbase_00064.html',
                children: [
                  {
                    id: 1141,
                    parentId: 1101,
                    name: '登录DeviceManager管理界面',
                    local: 'hbase_00072_aq2.html'
                  },
                  {
                    id: 1142,
                    parentId: 1101,
                    name:
                      'MRS生产环境Kerberos认证的主体密码过期后，更新认证文件',
                    local: 'hbase_00067.html'
                  },
                  {
                    id: 1143,
                    parentId: 1101,
                    name:
                      '备份MRS平台Hbase应用中Graphbase服务的元数据表后，恢复失败',
                    local: 'hbase_00068.html'
                  },
                  {
                    id: 1144,
                    parentId: 1101,
                    name: '不同大数据平台配置hbase.master.logcleaner.ttl参数',
                    local: 'hbase_00069.html'
                  }
                ]
              }
            ]
          },
          {
            id: 966,
            parentId: 14,
            name: 'Hive数据保护',
            local: 'zh-cn_topic_0000002200094101.html',
            children: [
              {
                id: 1145,
                parentId: 966,
                name: '备份',
                local: 'hive_00007.html',
                children: [
                  {
                    id: 1154,
                    parentId: 1145,
                    name: '备份前准备',
                    local: 'hive_00010.html'
                  },
                  {
                    id: 1155,
                    parentId: 1145,
                    name: '备份Hive备份集',
                    local: 'hive_00011.html',
                    children: [
                      {
                        id: 1156,
                        parentId: 1155,
                        name: '步骤1：开启数据库表所在目录的快照功能',
                        local: 'hive_00012.html'
                      },
                      {
                        id: 1157,
                        parentId: 1155,
                        name: '步骤2：（可选）生成并获取证书',
                        local: 'hive_00014.html'
                      },
                      {
                        id: 1158,
                        parentId: 1155,
                        name: '步骤3：注册Hive集群',
                        local: 'hive_00015.html'
                      },
                      {
                        id: 1159,
                        parentId: 1155,
                        name: '步骤4：创建Hive备份集',
                        local: 'hive_00016.html'
                      },
                      {
                        id: 1160,
                        parentId: 1155,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'hive_00017.html'
                      },
                      {
                        id: 1161,
                        parentId: 1155,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'hive_00018.html'
                      },
                      {
                        id: 1162,
                        parentId: 1155,
                        name: '步骤7：创建备份SLA',
                        local: 'hive_00019.html'
                      },
                      {
                        id: 1163,
                        parentId: 1155,
                        name: '步骤8：执行备份',
                        local: 'hive_00020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1146,
                parentId: 966,
                name: '复制',
                local: 'hive_00023.html',
                children: [
                  {
                    id: 1164,
                    parentId: 1146,
                    name: '复制Hive副本',
                    local: 'hive_00026.html',
                    children: [
                      {
                        id: 1165,
                        parentId: 1164,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'hive_00028.html'
                      },
                      {
                        id: 1166,
                        parentId: 1164,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'hive_00099.html'
                      },
                      {
                        id: 1167,
                        parentId: 1164,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'hive_00029.html'
                      },
                      {
                        id: 1168,
                        parentId: 1164,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'hive_00030.html'
                      },
                      {
                        id: 1169,
                        parentId: 1164,
                        name: '步骤4：下载并导入证书',
                        local: 'hive_00031.html'
                      },
                      {
                        id: 1170,
                        parentId: 1164,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'hive_00032.html'
                      },
                      {
                        id: 1171,
                        parentId: 1164,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'hive_00079.html'
                      },
                      {
                        id: 1172,
                        parentId: 1164,
                        name: '步骤6：添加复制集群',
                        local: 'hive_00033.html'
                      },
                      {
                        id: 1173,
                        parentId: 1164,
                        name: '步骤7：创建复制SLA',
                        local: 'hive_00034.html'
                      },
                      {
                        id: 1174,
                        parentId: 1164,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'hive_00035.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1147,
                parentId: 966,
                name: '归档',
                local: 'hive_00036.html',
                children: [
                  {
                    id: 1175,
                    parentId: 1147,
                    name: '归档Hive备份副本',
                    local: 'hive_00039.html',
                    children: [
                      {
                        id: 1177,
                        parentId: 1175,
                        name: '步骤1：添加归档存储',
                        local: 'hive_00040.html',
                        children: [
                          {
                            id: 1179,
                            parentId: 1177,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'hive_00041.html'
                          },
                          {
                            id: 1180,
                            parentId: 1177,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'hive_00042.html'
                          }
                        ]
                      },
                      {
                        id: 1178,
                        parentId: 1175,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'hive_00043.html'
                      }
                    ]
                  },
                  {
                    id: 1176,
                    parentId: 1147,
                    name: '归档Hive复制副本',
                    local: 'hive_00044.html',
                    children: [
                      {
                        id: 1181,
                        parentId: 1176,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'hive_00045.html'
                      },
                      {
                        id: 1182,
                        parentId: 1176,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'hive_00046.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1148,
                parentId: 966,
                name: '恢复',
                local: 'hive_00047.html',
                children: [
                  {
                    id: 1183,
                    parentId: 1148,
                    name: '恢复Hive备份集',
                    local: 'hive_00049.html'
                  },
                  {
                    id: 1184,
                    parentId: 1148,
                    name: '恢复Hive备份集中的单个或多个表',
                    local: 'hive_00050.html'
                  }
                ]
              },
              {
                id: 1149,
                parentId: 966,
                name: '全局搜索',
                local: 'hive_00051.html',
                children: [
                  {
                    id: 1185,
                    parentId: 1149,
                    name: '全局搜索资源',
                    local: 'hive_00052.html'
                  },
                  {
                    id: 1186,
                    parentId: 1149,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'hive_00099_0.html'
                  }
                ]
              },
              {
                id: 1150,
                parentId: 966,
                name: 'SLA',
                local: 'hive_00055.html',
                children: [
                  {
                    id: 1187,
                    parentId: 1150,
                    name: '关于SLA',
                    local: 'hive_00056.html'
                  },
                  {
                    id: 1188,
                    parentId: 1150,
                    name: '查看SLA信息',
                    local: 'hive_00057.html'
                  },
                  {
                    id: 1189,
                    parentId: 1150,
                    name: '管理SLA',
                    local: 'hive_00058.html'
                  }
                ]
              },
              {
                id: 1151,
                parentId: 966,
                name: '副本',
                local: 'hive_00059.html',
                children: [
                  {
                    id: 1190,
                    parentId: 1151,
                    name: '查看Hive副本信息',
                    local: 'hive_00060.html'
                  },
                  {
                    id: 1191,
                    parentId: 1151,
                    name: '管理Hive副本',
                    local: 'hive_00061.html'
                  }
                ]
              },
              {
                id: 1152,
                parentId: 966,
                name: 'Hive集群环境',
                local: 'hive_00062.html',
                children: [
                  {
                    id: 1192,
                    parentId: 1152,
                    name: '查看Hive环境信息',
                    local: 'hive_00063.html'
                  },
                  {
                    id: 1193,
                    parentId: 1152,
                    name: '管理Hive集群',
                    local: 'hive_00064.html'
                  },
                  {
                    id: 1194,
                    parentId: 1152,
                    name: '管理备份集',
                    local: 'hive_00065.html'
                  }
                ]
              },
              {
                id: 1153,
                parentId: 966,
                name: '常见问题',
                local: 'hive_00066.html',
                children: [
                  {
                    id: 1195,
                    parentId: 1153,
                    name: '登录DeviceManager管理界面',
                    local: 'hive_00070_as.html'
                  },
                  {
                    id: 1196,
                    parentId: 1153,
                    name:
                      'MRS生产环境kerberos认证的主体密码过期后，更新认证文件',
                    local: 'hive_00070.html'
                  },
                  {
                    id: 1197,
                    parentId: 1153,
                    name: '操作用户权限不足导致备份或恢复任务失败',
                    local: 'hive_00071.html'
                  },
                  {
                    id: 1198,
                    parentId: 1153,
                    name: '退出集群安全模式',
                    local: 'Hive_00075.html'
                  }
                ]
              }
            ]
          },
          {
            id: 967,
            parentId: 14,
            name: 'MongoDB数据保护',
            local: 'zh-cn_topic_0000002164767502.html',
            children: [
              {
                id: 1199,
                parentId: 967,
                name: '备份',
                local: 'mongodb-0007.html',
                children: [
                  {
                    id: 1207,
                    parentId: 1199,
                    name: '备份前准备',
                    local: 'mongodb-0010.html'
                  },
                  {
                    id: 1208,
                    parentId: 1199,
                    name: '备份MongoDB数据库',
                    local: 'mongodb-0011.html',
                    children: [
                      {
                        id: 1209,
                        parentId: 1208,
                        name: '步骤1：注册MongoDB实例',
                        local: 'mongodb-0012.html'
                      },
                      {
                        id: 1210,
                        parentId: 1208,
                        name: '步骤2：（可选）创建限速策略',
                        local: 'mongodb-0013.html'
                      },
                      {
                        id: 1211,
                        parentId: 1208,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'mongodb-0014.html'
                      },
                      {
                        id: 1212,
                        parentId: 1208,
                        name: '步骤4：创建备份SLA',
                        local: 'mongodb-0015.html'
                      },
                      {
                        id: 1213,
                        parentId: 1208,
                        name: '步骤5：执行备份',
                        local: 'mongodb-0016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1200,
                parentId: 967,
                name: '复制',
                local: 'mongodb-0017.html',
                children: [
                  {
                    id: 1214,
                    parentId: 1200,
                    name: '复制MongoDB数据库副本',
                    local: 'mongodb-0020.html',
                    children: [
                      {
                        id: 1215,
                        parentId: 1214,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'mongodb-0021.html'
                      },
                      {
                        id: 1216,
                        parentId: 1214,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'mongodb-0022.html'
                      },
                      {
                        id: 1217,
                        parentId: 1214,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'mongodb-0023.html'
                      },
                      {
                        id: 1218,
                        parentId: 1214,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'mongodb-0024.html'
                      },
                      {
                        id: 1219,
                        parentId: 1214,
                        name: '步骤4：下载并导入证书',
                        local: 'mongodb-0025.html'
                      },
                      {
                        id: 1220,
                        parentId: 1214,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'mongodb-0026.html'
                      },
                      {
                        id: 1221,
                        parentId: 1214,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'mongodb-0027.html'
                      },
                      {
                        id: 1222,
                        parentId: 1214,
                        name: '步骤6：添加复制集群',
                        local: 'mongodb-0028.html'
                      },
                      {
                        id: 1223,
                        parentId: 1214,
                        name: '步骤7：创建复制SLA',
                        local: 'mongodb-0029.html'
                      },
                      {
                        id: 1224,
                        parentId: 1214,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'mongodb-0030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1201,
                parentId: 967,
                name: '归档',
                local: 'mongodb-0050.html',
                children: [
                  {
                    id: 1225,
                    parentId: 1201,
                    name: '归档MongoDB备份副本',
                    local: 'mongodb-0053.html',
                    children: [
                      {
                        id: 1227,
                        parentId: 1225,
                        name: '步骤1：添加归档存储',
                        local: 'mongodb-0054.html',
                        children: [
                          {
                            id: 1229,
                            parentId: 1227,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'mongodb-0055.html'
                          },
                          {
                            id: 1230,
                            parentId: 1227,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'mongodb-0056.html'
                          }
                        ]
                      },
                      {
                        id: 1228,
                        parentId: 1225,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'mongodb-0057.html'
                      }
                    ]
                  },
                  {
                    id: 1226,
                    parentId: 1201,
                    name: '归档MongoDB复制副本',
                    local: 'mongodb-0058.html',
                    children: [
                      {
                        id: 1231,
                        parentId: 1226,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'mongodb-0059.html'
                      },
                      {
                        id: 1232,
                        parentId: 1226,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'mongodb-0060.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1202,
                parentId: 967,
                name: '恢复',
                local: 'mongodb-0061.html',
                children: [
                  {
                    id: 1233,
                    parentId: 1202,
                    name: '恢复MongoDB',
                    local: 'mongodb-0064.html'
                  }
                ]
              },
              {
                id: 1203,
                parentId: 967,
                name: '全局搜索',
                local: 'mongodb-0065.html',
                children: [
                  {
                    id: 1234,
                    parentId: 1203,
                    name: '全局搜索资源',
                    local: 'mongodb-0066.html'
                  },
                  {
                    id: 1235,
                    parentId: 1203,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'mongodb-0067.html'
                  }
                ]
              },
              {
                id: 1204,
                parentId: 967,
                name: 'SLA',
                local: 'mongodb-0071.html',
                children: [
                  {
                    id: 1236,
                    parentId: 1204,
                    name: '关于SLA',
                    local: 'mongodb-0072.html'
                  },
                  {
                    id: 1237,
                    parentId: 1204,
                    name: '查看SLA信息',
                    local: 'mongodb-0073.html'
                  },
                  {
                    id: 1238,
                    parentId: 1204,
                    name: '管理SLA',
                    local: 'mongodb-0074.html'
                  }
                ]
              },
              {
                id: 1205,
                parentId: 967,
                name: '副本',
                local: 'mongodb-0075.html',
                children: [
                  {
                    id: 1239,
                    parentId: 1205,
                    name: '查看MongoDB副本信息',
                    local: 'mongodb-0076.html'
                  },
                  {
                    id: 1240,
                    parentId: 1205,
                    name: '管理MongoDB副本',
                    local: 'mongodb-0077.html'
                  }
                ]
              },
              {
                id: 1206,
                parentId: 967,
                name: 'MongoDB环境',
                local: 'mongodb-0078.html',
                children: [
                  {
                    id: 1241,
                    parentId: 1206,
                    name: '查看MongoDB环境信息',
                    local: 'mongodb-0079.html'
                  },
                  {
                    id: 1242,
                    parentId: 1206,
                    name: '管理MongoDB',
                    local: 'mongodb-0080.html'
                  }
                ]
              }
            ]
          },
          {
            id: 968,
            parentId: 14,
            name: 'Elasticsearch数据保护',
            local: 'zh-cn_topic_0000002200008549.html',
            children: [
              {
                id: 1243,
                parentId: 968,
                name: '备份',
                local: 'ES_gud_00007.html',
                children: [
                  {
                    id: 1252,
                    parentId: 1243,
                    name: '备份前准备',
                    local: 'ES_gud_00010.html'
                  },
                  {
                    id: 1253,
                    parentId: 1243,
                    name: '备份Elasticsearch集群',
                    local: 'ES_gud_00011.html',
                    children: [
                      {
                        id: 1254,
                        parentId: 1253,
                        name: '步骤1：（可选）开启安全加密模式',
                        local: 'ES_gud_00012.html'
                      },
                      {
                        id: 1255,
                        parentId: 1253,
                        name: '步骤2：创建并配置挂载目录',
                        local: 'ES_gud_00013.html'
                      },
                      {
                        id: 1256,
                        parentId: 1253,
                        name: '步骤3：注册Elasticsearch集群',
                        local: 'ES_gud_00014.html'
                      },
                      {
                        id: 1257,
                        parentId: 1253,
                        name: '步骤4：创建Elasticsearch备份集',
                        local: 'ES_gud_00015.html'
                      },
                      {
                        id: 1258,
                        parentId: 1253,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'ES_gud_00016.html'
                      },
                      {
                        id: 1259,
                        parentId: 1253,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'ES_gud_00017.html'
                      },
                      {
                        id: 1260,
                        parentId: 1253,
                        name: '步骤7：创建备份SLA',
                        local: 'ES_gud_00018.html'
                      },
                      {
                        id: 1261,
                        parentId: 1253,
                        name: '步骤8：执行备份',
                        local: 'ES_gud_00019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1244,
                parentId: 968,
                name: '复制',
                local: 'ES_gud_00022.html',
                children: [
                  {
                    id: 1262,
                    parentId: 1244,
                    name: '复制Elasticsearch副本',
                    local: 'ES_gud_00025.html',
                    children: [
                      {
                        id: 1263,
                        parentId: 1262,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'ES_gud_00027.html'
                      },
                      {
                        id: 1264,
                        parentId: 1262,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_4.html'
                      },
                      {
                        id: 1265,
                        parentId: 1262,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'ES_gud_00028.html'
                      },
                      {
                        id: 1266,
                        parentId: 1262,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'ES_gud_00029.html'
                      },
                      {
                        id: 1267,
                        parentId: 1262,
                        name: '步骤4：下载并导入证书',
                        local: 'ES_gud_00030.html'
                      },
                      {
                        id: 1268,
                        parentId: 1262,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'ES_gud_00031.html'
                      },
                      {
                        id: 1269,
                        parentId: 1262,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'Es_gud_00088.html'
                      },
                      {
                        id: 1270,
                        parentId: 1262,
                        name: '步骤6：添加复制集群',
                        local: 'ES_gud_00032.html'
                      },
                      {
                        id: 1271,
                        parentId: 1262,
                        name: '步骤7：创建复制SLA',
                        local: 'ES_gud_00033.html'
                      },
                      {
                        id: 1272,
                        parentId: 1262,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'ES_gud_00034.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1245,
                parentId: 968,
                name: '归档',
                local: 'ES_gud_00035.html',
                children: [
                  {
                    id: 1273,
                    parentId: 1245,
                    name: '归档Elasticsearch备份副本',
                    local: 'ES_gud_00038.html',
                    children: [
                      {
                        id: 1275,
                        parentId: 1273,
                        name: '步骤1：添加归档存储',
                        local: 'ES_gud_00039.html',
                        children: [
                          {
                            id: 1277,
                            parentId: 1275,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'ES_gud_00040.html'
                          },
                          {
                            id: 1278,
                            parentId: 1275,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'ES_gud_00041.html'
                          }
                        ]
                      },
                      {
                        id: 1276,
                        parentId: 1273,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'ES_gud_00042.html'
                      }
                    ]
                  },
                  {
                    id: 1274,
                    parentId: 1245,
                    name: '归档Elasticsearch复制副本',
                    local: 'ES_gud_00043.html',
                    children: [
                      {
                        id: 1279,
                        parentId: 1274,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'ES_gud_00044.html'
                      },
                      {
                        id: 1280,
                        parentId: 1274,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'ES_gud_00045.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1246,
                parentId: 968,
                name: '恢复',
                local: 'ES_gud_00046.html',
                children: [
                  {
                    id: 1281,
                    parentId: 1246,
                    name: '恢复Elasticsearch备份集',
                    local: 'ES_gud_00049.html'
                  },
                  {
                    id: 1282,
                    parentId: 1246,
                    name: '恢复Elasticsearch备份集中的单个或多个索引',
                    local: 'ES_gud_00050.html'
                  }
                ]
              },
              {
                id: 1247,
                parentId: 968,
                name: '全局搜索',
                local: 'ES_gud_00047_a1.html',
                children: [
                  {
                    id: 1283,
                    parentId: 1247,
                    name: '全局搜索资源',
                    local: 'ES_gud_00047_a2.html'
                  },
                  {
                    id: 1284,
                    parentId: 1247,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'ES_gud_00047_a3.html'
                  }
                ]
              },
              {
                id: 1248,
                parentId: 968,
                name: 'SLA',
                local: 'ES_gud_00055.html',
                children: [
                  {
                    id: 1285,
                    parentId: 1248,
                    name: '关于SLA',
                    local: 'ES_gud_00056.html'
                  },
                  {
                    id: 1286,
                    parentId: 1248,
                    name: '查看SLA信息',
                    local: 'ES_gud_00057.html'
                  },
                  {
                    id: 1287,
                    parentId: 1248,
                    name: '管理SLA',
                    local: 'ES_gud_00058.html'
                  }
                ]
              },
              {
                id: 1249,
                parentId: 968,
                name: '副本',
                local: 'ES_gud_00059.html',
                children: [
                  {
                    id: 1288,
                    parentId: 1249,
                    name: '查看Elasticsearch副本信息',
                    local: 'ES_gud_00060.html'
                  },
                  {
                    id: 1289,
                    parentId: 1249,
                    name: '管理Elasticsearch副本',
                    local: 'ES_gud_00061.html'
                  }
                ]
              },
              {
                id: 1250,
                parentId: 968,
                name: 'Elasticsearch集群环境',
                local: 'ES_gud_00062.html',
                children: [
                  {
                    id: 1290,
                    parentId: 1250,
                    name: '查看Elasticsearch环境信息',
                    local: 'ES_gud_00063.html'
                  },
                  {
                    id: 1291,
                    parentId: 1250,
                    name: '管理Elasticsearch集群',
                    local: 'ES_gud_00064.html'
                  },
                  {
                    id: 1292,
                    parentId: 1250,
                    name: '管理备份集',
                    local: 'ES_gud_00065.html'
                  }
                ]
              },
              {
                id: 1251,
                parentId: 968,
                name: '常见问题',
                local: 'ES_gud_00066.html',
                children: [
                  {
                    id: 1293,
                    parentId: 1251,
                    name: '登录DeviceManager管理界面',
                    local: 'ES_gud_00074_a123.html'
                  },
                  {
                    id: 1294,
                    parentId: 1251,
                    name:
                      'MRS生产环境kerberos认证的主体密码过期后，更新认证文件',
                    local: 'ES_gud_00069.html'
                  },
                  {
                    id: 1295,
                    parentId: 1251,
                    name: '索引被删除导致备份失败，如何跳过被删除索引的备份',
                    local: 'ES_gud_00070.html'
                  },
                  {
                    id: 1296,
                    parentId: 1251,
                    name:
                      '执行备份/恢复操作时，由于配置文件plugin_attribute_1.1.0.json中配置项取值错误，操作失败',
                    local: 'ES_gud_00071.html'
                  }
                ]
              }
            ]
          },
          {
            id: 969,
            parentId: 14,
            name: 'HDFS数据保护',
            local: 'product_documentation_000031.html',
            children: [
              {
                id: 1297,
                parentId: 969,
                name: '备份',
                local: 'hdfs_00007.html',
                children: [
                  {
                    id: 1306,
                    parentId: 1297,
                    name: '备份前准备',
                    local: 'hdfs_00010.html'
                  },
                  {
                    id: 1307,
                    parentId: 1297,
                    name: '备份HDFS文件集',
                    local: 'hdfs_00011.html',
                    children: [
                      {
                        id: 1308,
                        parentId: 1307,
                        name: '步骤1：开启HDFS目录的快照功能',
                        local: 'hdfs_00012.html'
                      },
                      {
                        id: 1309,
                        parentId: 1307,
                        name: '步骤2：检查HDFS ACL的开关状态',
                        local: 'hdfs_00013.html'
                      },
                      {
                        id: 1310,
                        parentId: 1307,
                        name: '步骤3：注册HDFS集群',
                        local: 'hdfs_00014.html'
                      },
                      {
                        id: 1311,
                        parentId: 1307,
                        name: '步骤4：创建HDFS文件集',
                        local: 'hdfs_00015.html'
                      },
                      {
                        id: 1312,
                        parentId: 1307,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'hdfs_00016.html'
                      },
                      {
                        id: 1313,
                        parentId: 1307,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'hdfs_00017.html'
                      },
                      {
                        id: 1314,
                        parentId: 1307,
                        name: '步骤7：创建备份SLA',
                        local: 'hdfs_00018.html'
                      },
                      {
                        id: 1315,
                        parentId: 1307,
                        name: '步骤8：执行备份',
                        local: 'hdfs_00019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1298,
                parentId: 969,
                name: '复制',
                local: 'hdfs_00022.html',
                children: [
                  {
                    id: 1316,
                    parentId: 1298,
                    name: '复制HDFS副本',
                    local: 'hdfs_00025.html',
                    children: [
                      {
                        id: 1317,
                        parentId: 1316,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'hdfs_00027.html'
                      },
                      {
                        id: 1318,
                        parentId: 1316,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'hdfs_00089.html'
                      },
                      {
                        id: 1319,
                        parentId: 1316,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'hdfs_00028.html'
                      },
                      {
                        id: 1320,
                        parentId: 1316,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'hdfs_00029.html'
                      },
                      {
                        id: 1321,
                        parentId: 1316,
                        name: '步骤4：下载并导入证书',
                        local: 'hdfs_00030.html'
                      },
                      {
                        id: 1322,
                        parentId: 1316,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'hdfs_00031.html'
                      },
                      {
                        id: 1323,
                        parentId: 1316,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'hdfs_00079.html'
                      },
                      {
                        id: 1324,
                        parentId: 1316,
                        name: '步骤6：添加复制集群',
                        local: 'hdfs_00032.html'
                      },
                      {
                        id: 1325,
                        parentId: 1316,
                        name: '步骤7：创建复制SLA',
                        local: 'hdfs_00033.html'
                      },
                      {
                        id: 1326,
                        parentId: 1316,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'hdfs_00034.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1299,
                parentId: 969,
                name: '归档',
                local: 'hdfs_00035.html',
                children: [
                  {
                    id: 1327,
                    parentId: 1299,
                    name: '归档HDFS备份副本',
                    local: 'hdfs_00038.html',
                    children: [
                      {
                        id: 1329,
                        parentId: 1327,
                        name: '步骤1：添加归档存储',
                        local: 'hdfs_00039.html',
                        children: [
                          {
                            id: 1331,
                            parentId: 1329,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'hdfs_00040.html'
                          },
                          {
                            id: 1332,
                            parentId: 1329,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'hdfs_00041.html'
                          }
                        ]
                      },
                      {
                        id: 1330,
                        parentId: 1327,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'hdfs_00042.html'
                      }
                    ]
                  },
                  {
                    id: 1328,
                    parentId: 1299,
                    name: '归档HDFS复制副本',
                    local: 'hdfs_00043.html',
                    children: [
                      {
                        id: 1333,
                        parentId: 1328,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'hdfs_00044.html'
                      },
                      {
                        id: 1334,
                        parentId: 1328,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'hdfs_00045.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1300,
                parentId: 969,
                name: '恢复',
                local: 'hdfs_00046.html',
                children: [
                  {
                    id: 1335,
                    parentId: 1300,
                    name: '恢复HDFS文件集',
                    local: 'hdfs_00048.html'
                  }
                ]
              },
              {
                id: 1301,
                parentId: 969,
                name: '全局搜索',
                local: 'hdfs_00050.html',
                children: [
                  {
                    id: 1336,
                    parentId: 1301,
                    name: '全局搜索副本数据',
                    local: 'hdfs_00051.html'
                  },
                  {
                    id: 1337,
                    parentId: 1301,
                    name: '全局搜索资源',
                    local: 'hdfs_00052.html'
                  },
                  {
                    id: 1338,
                    parentId: 1301,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'hdfs_00089_0.html'
                  }
                ]
              },
              {
                id: 1302,
                parentId: 969,
                name: 'SLA',
                local: 'hdfs_00055.html',
                children: [
                  {
                    id: 1339,
                    parentId: 1302,
                    name: '关于SLA',
                    local: 'hdfs_00056.html'
                  },
                  {
                    id: 1340,
                    parentId: 1302,
                    name: '查看SLA信息',
                    local: 'hdfs_00057.html'
                  },
                  {
                    id: 1341,
                    parentId: 1302,
                    name: '管理SLA',
                    local: 'hdfs_00058.html'
                  }
                ]
              },
              {
                id: 1303,
                parentId: 969,
                name: '副本',
                local: 'hdfs_00059.html',
                children: [
                  {
                    id: 1342,
                    parentId: 1303,
                    name: '查看HDFS副本信息',
                    local: 'hdfs_00060.html'
                  },
                  {
                    id: 1343,
                    parentId: 1303,
                    name: '管理HDFS副本',
                    local: 'hdfs_00061.html'
                  }
                ]
              },
              {
                id: 1304,
                parentId: 969,
                name: 'HDFS集群环境',
                local: 'hdfs_00062.html',
                children: [
                  {
                    id: 1344,
                    parentId: 1304,
                    name: '查看HDFS环境信息',
                    local: 'hdfs_00063.html'
                  },
                  {
                    id: 1345,
                    parentId: 1304,
                    name: '管理HDFS集群',
                    local: 'hdfs_00064.html'
                  },
                  {
                    id: 1346,
                    parentId: 1304,
                    name: '管理HDFS文件集',
                    local: 'hdfs_00065.html'
                  }
                ]
              },
              {
                id: 1305,
                parentId: 969,
                name: '常见问题',
                local: 'hdfs_00066.html',
                children: [
                  {
                    id: 1347,
                    parentId: 1305,
                    name: '登录DeviceManager管理界面',
                    local: 'hdfs_00071_as.html'
                  },
                  {
                    id: 1348,
                    parentId: 1305,
                    name: 'MRS生产环境密码过期后，更新认证文件',
                    local: 'hdfs_00069.html'
                  },
                  {
                    id: 1349,
                    parentId: 1305,
                    name: 'HDFS执行索引任务失败',
                    local: 'hdfs_00070.html'
                  },
                  {
                    id: 1350,
                    parentId: 1305,
                    name:
                      '执行备份/恢复操作时，由于配置文件plugin_attribute_1.1.0.json中配置项取值错误，操作失败',
                    local: 'hdfs_00071.html'
                  },
                  {
                    id: 1351,
                    parentId: 1305,
                    name: '管理快照',
                    local: 'hdfs_00091.html'
                  }
                ]
              }
            ]
          },
          {
            id: 970,
            parentId: 14,
            name: 'Redis数据保护',
            local: 'zh-cn_topic_0000002200094113.html',
            children: [
              {
                id: 1352,
                parentId: 970,
                name: '备份',
                local: 'redis-00003.html',
                children: [
                  {
                    id: 1362,
                    parentId: 1352,
                    name: '备份前准备',
                    local: 'redis-00006.html'
                  },
                  {
                    id: 1363,
                    parentId: 1352,
                    name: '备份Redis集群',
                    local: 'redis-00007.html',
                    children: [
                      {
                        id: 1364,
                        parentId: 1363,
                        name: '步骤1：注册Redis集群',
                        local: 'redis-00008.html'
                      },
                      {
                        id: 1365,
                        parentId: 1363,
                        name: '步骤2：（可选）创建限速策略',
                        local: 'redis-00009.html'
                      },
                      {
                        id: 1366,
                        parentId: 1363,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'redis-00010.html'
                      },
                      {
                        id: 1367,
                        parentId: 1363,
                        name: '步骤4：创建备份SLA',
                        local: 'redis-00011.html'
                      },
                      {
                        id: 1368,
                        parentId: 1363,
                        name: '步骤5：执行备份',
                        local: 'redis-00012.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1353,
                parentId: 970,
                name: '复制',
                local: 'redis-00085.html',
                children: [
                  {
                    id: 1369,
                    parentId: 1353,
                    name: '复制Redis副本',
                    local: 'redis-00017.html',
                    children: [
                      {
                        id: 1370,
                        parentId: 1369,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'redis-00087.html'
                      },
                      {
                        id: 1371,
                        parentId: 1369,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'redis-00088.html'
                      },
                      {
                        id: 1372,
                        parentId: 1369,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'redis-00020.html'
                      },
                      {
                        id: 1373,
                        parentId: 1369,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'redis-00021.html'
                      },
                      {
                        id: 1374,
                        parentId: 1369,
                        name: '步骤4：下载并导入证书',
                        local: 'redis-00022.html'
                      },
                      {
                        id: 1375,
                        parentId: 1369,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'redis-00023.html'
                      },
                      {
                        id: 1376,
                        parentId: 1369,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'redis-00072.html'
                      },
                      {
                        id: 1377,
                        parentId: 1369,
                        name: '步骤6：添加复制集群',
                        local: 'redis-00024.html'
                      },
                      {
                        id: 1378,
                        parentId: 1369,
                        name: '步骤7：创建复制SLA',
                        local: 'redis-00025.html'
                      },
                      {
                        id: 1379,
                        parentId: 1369,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'redis-00026.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1354,
                parentId: 970,
                name: '归档',
                local: 'redis-00027.html',
                children: [
                  {
                    id: 1380,
                    parentId: 1354,
                    name: '归档Redis备份副本',
                    local: 'redis-00030.html',
                    children: [
                      {
                        id: 1382,
                        parentId: 1380,
                        name: '步骤1：添加归档存储',
                        local: 'redis-00031.html',
                        children: [
                          {
                            id: 1384,
                            parentId: 1382,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'redis-00032.html'
                          },
                          {
                            id: 1385,
                            parentId: 1382,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'redis-00033.html'
                          }
                        ]
                      },
                      {
                        id: 1383,
                        parentId: 1380,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'redis-00034.html'
                      }
                    ]
                  },
                  {
                    id: 1381,
                    parentId: 1354,
                    name: '归档Redis复制副本',
                    local: 'redis-00035.html',
                    children: [
                      {
                        id: 1386,
                        parentId: 1381,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'redis-00036.html'
                      },
                      {
                        id: 1387,
                        parentId: 1381,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'redis-00037.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1355,
                parentId: 970,
                name: '恢复',
                local: 'redis-00038.html',
                children: [
                  {
                    id: 1388,
                    parentId: 1355,
                    name: '恢复Redis集群',
                    local: 'redis-00041.html'
                  }
                ]
              },
              {
                id: 1356,
                parentId: 970,
                name: '全局搜索',
                local: 'redis-00082.html',
                children: [
                  {
                    id: 1389,
                    parentId: 1356,
                    name: '全局搜索资源',
                    local: 'redis-00042.html'
                  },
                  {
                    id: 1390,
                    parentId: 1356,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'redis-00083.html'
                  }
                ]
              },
              {
                id: 1357,
                parentId: 970,
                name: '数据重删压缩',
                local: 'redis-00043.html',
                children: [
                  {
                    id: 1391,
                    parentId: 1357,
                    name: '关于数据重删压缩',
                    local: 'redis-00044.html'
                  }
                ]
              },
              {
                id: 1358,
                parentId: 970,
                name: 'SLA',
                local: 'redis-00045.html',
                children: [
                  {
                    id: 1392,
                    parentId: 1358,
                    name: '关于SLA',
                    local: 'redis-00046.html'
                  },
                  {
                    id: 1393,
                    parentId: 1358,
                    name: '查看SLA信息',
                    local: 'redis-00047.html'
                  },
                  {
                    id: 1394,
                    parentId: 1358,
                    name: '管理SLA',
                    local: 'redis-00048.html'
                  }
                ]
              },
              {
                id: 1359,
                parentId: 970,
                name: '副本',
                local: 'redis-00049.html',
                children: [
                  {
                    id: 1395,
                    parentId: 1359,
                    name: '查看Redis副本信息',
                    local: 'redis-00050.html'
                  },
                  {
                    id: 1396,
                    parentId: 1359,
                    name: '管理Redis副本',
                    local: 'redis-00051.html'
                  }
                ]
              },
              {
                id: 1360,
                parentId: 970,
                name: 'Redis集群环境',
                local: 'redis-00052.html',
                children: [
                  {
                    id: 1397,
                    parentId: 1360,
                    name: '查看Redis环境信息',
                    local: 'redis-00053.html'
                  },
                  {
                    id: 1398,
                    parentId: 1360,
                    name: '管理Redis集群',
                    local: 'redis-00054.html'
                  },
                  {
                    id: 1399,
                    parentId: 1360,
                    name: '管理Redis集群保护',
                    local: 'redis-00055.html'
                  }
                ]
              },
              {
                id: 1361,
                parentId: 970,
                name: '常见问题',
                local: 'redis-00056.html',
                children: [
                  {
                    id: 1400,
                    parentId: 1361,
                    name: '登录DeviceManager管理界面',
                    local: 'redis-00084.html'
                  },
                  {
                    id: 1401,
                    parentId: 1361,
                    name:
                      'ClickHouse和Redis合并部署场景，偶现Redis节点离线、备份或恢复失败',
                    local: 'redis-00059.html'
                  },
                  {
                    id: 1402,
                    parentId: 1361,
                    name: 'MRS生产环境密码过期后，更新认证文件',
                    local: 'redis-00060.html'
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
        local: 'zh-cn_topic_0000002200094081.html',
        children: [
          {
            id: 1403,
            parentId: 15,
            name: 'VMware数据保护',
            local: 'product_documentation_000027.html',
            children: [
              {
                id: 1409,
                parentId: 1403,
                name: '备份',
                local: 'vmware_gud_0009.html',
                children: [
                  {
                    id: 1420,
                    parentId: 1409,
                    name: '备份前准备',
                    local: 'vmware_gud_0017.html'
                  },
                  {
                    id: 1421,
                    parentId: 1409,
                    name: '备份VMware虚拟机',
                    local: 'vmware_gud_0018.html',
                    children: [
                      {
                        id: 1422,
                        parentId: 1421,
                        name: '步骤1：检查并安装VMware Tools',
                        local: 'vmware_gud_0019.html'
                      },
                      {
                        id: 1423,
                        parentId: 1421,
                        name: '步骤2：检查并开启vmware-vapi-endpoint服务',
                        local: 'vmware_gud_0020.html'
                      },
                      {
                        id: 1424,
                        parentId: 1421,
                        name: '步骤3：配置应用一致性备份脚本',
                        local: 'vmware_gud_0021.html',
                        children: [
                          {
                            id: 1433,
                            parentId: 1424,
                            name: 'DB2数据库',
                            local: 'vmware_gud_0022.html'
                          },
                          {
                            id: 1434,
                            parentId: 1424,
                            name: 'Oracle数据库',
                            local: 'vmware_gud_0023.html'
                          },
                          {
                            id: 1435,
                            parentId: 1424,
                            name: 'Sybase数据库',
                            local: 'vmware_gud_0024.html'
                          },
                          {
                            id: 1436,
                            parentId: 1424,
                            name: 'MySQL数据库',
                            local: 'vmware_gud_0025.html'
                          }
                        ]
                      },
                      {
                        id: 1425,
                        parentId: 1421,
                        name: '步骤4：获取VMware证书',
                        local: 'vmware_gud_0026.html'
                      },
                      {
                        id: 1426,
                        parentId: 1421,
                        name: '步骤5：注册VMware虚拟化环境',
                        local: 'vmware_gud_0027.html'
                      },
                      {
                        id: 1427,
                        parentId: 1421,
                        name:
                          '步骤6：（可选）创建VMware虚拟机组（适用于1.6.0及后续版本）',
                        local: 'vmware_gud_0028.html'
                      },
                      {
                        id: 1428,
                        parentId: 1421,
                        name: '步骤7：（可选）创建限速策略',
                        local: 'vmware_gud_0029.html'
                      },
                      {
                        id: 1429,
                        parentId: 1421,
                        name: '步骤8：（可选）开启备份链路加密开关',
                        local: 'vmware_gud_0030.html'
                      },
                      {
                        id: 1430,
                        parentId: 1421,
                        name: '步骤9：登录iSCSI启动器',
                        local: 'vmware_gud_0031.html'
                      },
                      {
                        id: 1431,
                        parentId: 1421,
                        name: '步骤10：创建备份SLA',
                        local: 'vmware_gud_0032_1.html'
                      },
                      {
                        id: 1432,
                        parentId: 1421,
                        name: '步骤11：执行备份',
                        local: 'vmware_gud_0033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1410,
                parentId: 1403,
                name: '复制',
                local: 'vmware_gud_0034.html',
                children: [
                  {
                    id: 1437,
                    parentId: 1410,
                    name: '复制VMware虚拟机副本',
                    local: 'vmware_gud_0037.html',
                    children: [
                      {
                        id: 1438,
                        parentId: 1437,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'vmware_gud_0038.html'
                      },
                      {
                        id: 1439,
                        parentId: 1437,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'vmware_gud_0039.html'
                      },
                      {
                        id: 1440,
                        parentId: 1437,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'vmware_gud_0040.html'
                      },
                      {
                        id: 1441,
                        parentId: 1437,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'vmware_gud_0041.html'
                      },
                      {
                        id: 1442,
                        parentId: 1437,
                        name: '步骤4：下载并导入证书',
                        local: 'vmware_gud_0042.html'
                      },
                      {
                        id: 1443,
                        parentId: 1437,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'vmware_gud_0043.html'
                      },
                      {
                        id: 1444,
                        parentId: 1437,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'vmware_gud_0044.html'
                      },
                      {
                        id: 1445,
                        parentId: 1437,
                        name: '步骤6：添加复制集群',
                        local: 'vmware_gud_0045.html'
                      },
                      {
                        id: 1446,
                        parentId: 1437,
                        name: '步骤7：创建复制SLA',
                        local: 'vmware_gud_0046.html'
                      },
                      {
                        id: 1447,
                        parentId: 1437,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'vmware_gud_0047.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1411,
                parentId: 1403,
                name: '归档',
                local: 'vmware_gud_0067.html',
                children: [
                  {
                    id: 1448,
                    parentId: 1411,
                    name: '归档VMware备份副本',
                    local: 'vmware_gud_0070.html',
                    children: [
                      {
                        id: 1450,
                        parentId: 1448,
                        name: '步骤1：添加归档存储',
                        local: 'vmware_gud_0071.html',
                        children: [
                          {
                            id: 1452,
                            parentId: 1450,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'vmware_gud_0072.html'
                          },
                          {
                            id: 1453,
                            parentId: 1450,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'vmware_gud_0073.html'
                          }
                        ]
                      },
                      {
                        id: 1451,
                        parentId: 1448,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'vmware_gud_0074.html'
                      }
                    ]
                  },
                  {
                    id: 1449,
                    parentId: 1411,
                    name: '归档VMware复制副本',
                    local: 'vmware_gud_0075.html',
                    children: [
                      {
                        id: 1454,
                        parentId: 1449,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'vmware_gud_0076.html'
                      },
                      {
                        id: 1455,
                        parentId: 1449,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'vmware_gud_0077.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1412,
                parentId: 1403,
                name: '恢复',
                local: 'vmware_gud_0078.html',
                children: [
                  {
                    id: 1456,
                    parentId: 1412,
                    name: '恢复VMware虚拟机',
                    local: 'vmware_gud_0081.html'
                  },
                  {
                    id: 1457,
                    parentId: 1412,
                    name: '恢复VMware虚拟机磁盘',
                    local: 'vmware_gud_0082.html'
                  },
                  {
                    id: 1458,
                    parentId: 1412,
                    name: '恢复VMware虚拟机中的文件',
                    local: 'vmware_gud_0083.html'
                  }
                ]
              },
              {
                id: 1413,
                parentId: 1403,
                name: '即时恢复',
                local: 'vmware_gud_0084.html',
                children: [
                  {
                    id: 1459,
                    parentId: 1413,
                    name: '即时恢复VMware虚拟机',
                    local: 'vmware_gud_0087.html'
                  }
                ]
              },
              {
                id: 1414,
                parentId: 1403,
                name: '即时挂载',
                local: 'vmware_gud_0088.html',
                children: [
                  {
                    id: 1460,
                    parentId: 1414,
                    name: '即时挂载VMware虚拟机',
                    local: 'vmware_gud_0091.html'
                  },
                  {
                    id: 1461,
                    parentId: 1414,
                    name: '管理VMware即时挂载',
                    local: 'vmware_gud_0092.html',
                    children: [
                      {
                        id: 1462,
                        parentId: 1461,
                        name: '查看VMware即时挂载信息',
                        local: 'vmware_gud_0093.html'
                      },
                      {
                        id: 1463,
                        parentId: 1461,
                        name: '管理即时挂载',
                        local: 'vmware_gud_0094.html'
                      },
                      {
                        id: 1464,
                        parentId: 1461,
                        name: '创建挂载更新策略',
                        local: 'vmware_gud_0095.html'
                      },
                      {
                        id: 1465,
                        parentId: 1461,
                        name: '管理挂载更新策略',
                        local: 'vmware_gud_0096.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1415,
                parentId: 1403,
                name: '全局搜索',
                local: 'vmware_gud_0097.html',
                children: [
                  {
                    id: 1466,
                    parentId: 1415,
                    name: '关于全局搜索',
                    local: 'vmware_gud_0098.html'
                  },
                  {
                    id: 1467,
                    parentId: 1415,
                    name: '全局搜索副本数据',
                    local: 'vmware_gud_0099.html'
                  },
                  {
                    id: 1468,
                    parentId: 1415,
                    name: '全局搜索资源',
                    local: 'vmware_gud_0100.html'
                  },
                  {
                    id: 1469,
                    parentId: 1415,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'vmware_gud_0101.html'
                  }
                ]
              },
              {
                id: 1416,
                parentId: 1403,
                name: 'SLA',
                local: 'vmware_gud_0104.html',
                children: [
                  {
                    id: 1470,
                    parentId: 1416,
                    name: '关于SLA',
                    local: 'vmware_gud_0105.html'
                  },
                  {
                    id: 1471,
                    parentId: 1416,
                    name: '查看SLA信息',
                    local: 'vmware_gud_0106.html'
                  },
                  {
                    id: 1472,
                    parentId: 1416,
                    name: '管理SLA',
                    local: 'vmware_gud_0107.html'
                  }
                ]
              },
              {
                id: 1417,
                parentId: 1403,
                name: '副本',
                local: 'vmware_gud_0108.html',
                children: [
                  {
                    id: 1473,
                    parentId: 1417,
                    name: '查看VMware副本信息',
                    local: 'vmware_gud_0109.html'
                  },
                  {
                    id: 1474,
                    parentId: 1417,
                    name: '管理VMware副本',
                    local: 'vmware_gud_0110.html'
                  }
                ]
              },
              {
                id: 1418,
                parentId: 1403,
                name: 'VMware虚拟化环境',
                local: 'vmware_gud_0111.html',
                children: [
                  {
                    id: 1475,
                    parentId: 1418,
                    name: '查看VMware虚拟化环境信息',
                    local: 'vmware_gud_0112.html'
                  },
                  {
                    id: 1476,
                    parentId: 1418,
                    name: '管理VMware注册信息',
                    local: 'vmware_gud_0113.html'
                  },
                  {
                    id: 1477,
                    parentId: 1418,
                    name: '管理集群/主机/虚拟机/虚拟机组',
                    local: 'vmware_gud_0114.html'
                  }
                ]
              },
              {
                id: 1419,
                parentId: 1403,
                name: '常见问题',
                local: 'vmware_gud_0115.html',
                children: [
                  {
                    id: 1478,
                    parentId: 1419,
                    name: '登录DeviceManager管理界面',
                    local: 'vmware_gud_0117.html'
                  },
                  {
                    id: 1479,
                    parentId: 1419,
                    name: 'VMware虚拟机备份恢复传输模式',
                    local: 'vmware_gud_0121.html'
                  },
                  {
                    id: 1480,
                    parentId: 1419,
                    name: '注册VMware虚拟化环境时，注册用户的最小权限要求',
                    local: 'vmware_gud_0122.html'
                  },
                  {
                    id: 1481,
                    parentId: 1419,
                    name: 'VMware虚拟机磁盘文件被锁定后如何处理',
                    local: 'vmware_gud_0123.html'
                  },
                  {
                    id: 1482,
                    parentId: 1419,
                    name: '如何修改ESXi主机NFS卷的挂载数量',
                    local: 'vmware_gud_0124.html'
                  },
                  {
                    id: 1483,
                    parentId: 1419,
                    name: '如何启用VMware虚拟机的网络适配器',
                    local: 'vmware_gud_0125.html'
                  },
                  {
                    id: 1484,
                    parentId: 1419,
                    name: '如何在vCenter上添加用户权限',
                    local: 'vmware_gud_0126.html'
                  },
                  {
                    id: 1485,
                    parentId: 1419,
                    name: '受保护的虚拟机名称存在特殊字符导致备份失败',
                    local: 'vmware_gud_0128.html'
                  },
                  {
                    id: 1486,
                    parentId: 1419,
                    name: '恢复虚拟机后CD-ROM设备不可访问',
                    local: 'vmware_gud_0129.html'
                  },
                  {
                    id: 1487,
                    parentId: 1419,
                    name:
                      '恢复VMware虚拟机时，任务详情中提示“恢复nvram文件失败”如何处理',
                    local: 'vmware_gud_0130.html'
                  },
                  {
                    id: 1488,
                    parentId: 1419,
                    name: '恢复虚拟机至原位置新机，恢复成功但虚拟机启动异常',
                    local: 'vmware_gud_0131.html'
                  },
                  {
                    id: 1489,
                    parentId: 1419,
                    name: '生产存储扩容后，备份任务失败如何处理',
                    local: 'vmware_gud_0132.html'
                  },
                  {
                    id: 1490,
                    parentId: 1419,
                    name: '恢复虚拟机时，恢复成功但虚拟机开机失败',
                    local: 'vmware_gud_0133.html'
                  },
                  {
                    id: 1491,
                    parentId: 1419,
                    name: '恢复虚拟机或磁盘时，恢复失败',
                    local: 'vmware_gud_0134.html'
                  },
                  {
                    id: 1492,
                    parentId: 1419,
                    name:
                      'ESXi主机的域名无法被客户端所在主机识别，备份恢复失败',
                    local: 'vmware_gud_0135.html'
                  },
                  {
                    id: 1493,
                    parentId: 1419,
                    name: '覆盖原虚拟机进行恢复时，恢复成功但虚拟机开机失败',
                    local: 'vmware_gud_0136.html'
                  },
                  {
                    id: 1494,
                    parentId: 1419,
                    name: '创建虚拟机快照时，出现快照无法创建的报错',
                    local: 'vmware_gud_0137.html'
                  },
                  {
                    id: 1495,
                    parentId: 1419,
                    name: '管理口故障恢复后，VMware资源链路未恢复正常',
                    local: 'vmware_gud_0138.html'
                  },
                  {
                    id: 1496,
                    parentId: 1419,
                    name: '使用NBD/NBDSSL传输模式备份恢复大容量虚拟机硬盘失败',
                    local: 'vmware_gud_0139.html'
                  },
                  {
                    id: 1497,
                    parentId: 1419,
                    name:
                      '生产存储硬盘域故障，备份任务卡住（适用于1.6.0及后续版本）',
                    local: 'vmware_gud_0140.html'
                  },
                  {
                    id: 1498,
                    parentId: 1419,
                    name:
                      '“准备NAS文件系统”到“开始备份虚拟机”备份任务间隔时间过长',
                    local: 'vmware_gud_0141.html'
                  },
                  {
                    id: 1499,
                    parentId: 1419,
                    name: '备份VMware虚拟机时VDDK打开磁盘失败',
                    local: 'vmware_gud_0142.html'
                  },
                  {
                    id: 1500,
                    parentId: 1419,
                    name: '包含RDM盘的备份或恢复任务失败',
                    local: 'vmware_gud_0143.html'
                  },
                  {
                    id: 1501,
                    parentId: 1419,
                    name:
                      'vCenter Server上虚拟机出现“虚拟机需要整合”状态的提示',
                    local: 'vmware_gud_0144.html'
                  },
                  {
                    id: 1502,
                    parentId: 1419,
                    name:
                      '执行备份恢复任务时，任务详情停滞在“任务等待数据迁移调度器处理”',
                    local: 'vmware_gud_0145.html'
                  },
                  {
                    id: 1503,
                    parentId: 1419,
                    name:
                      '备份虚拟机时，虚拟机所在ESXi主机配置了NFC backup属性的VMKernel网卡，导致备份任务失败',
                    local: 'vmware_gud_0146.html'
                  },
                  {
                    id: 1504,
                    parentId: 1419,
                    name: 'VMware系统盘恢复到新虚拟机后目标虚拟机启动失败',
                    local: 'vmware_gud_0147.html'
                  },
                  {
                    id: 1505,
                    parentId: 1419,
                    name: '虚拟机磁盘覆盖恢复后业务数据异常',
                    local: 'vmware_gud_0148.html'
                  },
                  {
                    id: 1506,
                    parentId: 1419,
                    name: '使用Hot-Add传输模式备份虚拟机时，备份任务一直重试',
                    local: 'vmware_gud_0149.html'
                  },
                  {
                    id: 1507,
                    parentId: 1419,
                    name:
                      '执行备份任务失败，任务详情显示“通知客户端xx完成磁盘xx备份任务失败”或者“通知客户端xx进行磁盘xx备份失败”',
                    local: 'vmware_gud_0166.html'
                  },
                  {
                    id: 1508,
                    parentId: 1419,
                    name: 'VMware恢复任务失败，任务详情显示“创建虚拟机失败”',
                    local: 'vmware_gud_0167.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1404,
            parentId: 15,
            name: 'FusionCompute数据保护',
            local: 'zh-cn_topic_0000002164607770.html',
            children: [
              {
                id: 1509,
                parentId: 1404,
                name: '备份',
                local: 'fc_gud_0009.html',
                children: [
                  {
                    id: 1518,
                    parentId: 1509,
                    name: '备份前准备',
                    local: 'fc_gud_0012.html'
                  },
                  {
                    id: 1519,
                    parentId: 1509,
                    name: '备份FusionCompute虚拟机',
                    local: 'fc_gud_0013.html',
                    children: [
                      {
                        id: 1520,
                        parentId: 1519,
                        name: '步骤1：创建FusionCompute对接用户',
                        local: 'fc_gud_0014.html'
                      },
                      {
                        id: 1521,
                        parentId: 1519,
                        name: '步骤2：检查FusionCompute接口访问隔离策略',
                        local: 'fc_gud_0014_0.html'
                      },
                      {
                        id: 1522,
                        parentId: 1519,
                        name: '步骤3：注册FusionCompute虚拟化环境',
                        local: 'fc_gud_0015.html'
                      },
                      {
                        id: 1523,
                        parentId: 1519,
                        name:
                          '步骤4：（可选）创建FusionCompute虚拟机组（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0024_1.html'
                      },
                      {
                        id: 1524,
                        parentId: 1519,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'fc_gud_0016.html'
                      },
                      {
                        id: 1525,
                        parentId: 1519,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'fc_gud_0017.html'
                      },
                      {
                        id: 1526,
                        parentId: 1519,
                        name: '步骤7：创建备份SLA',
                        local: 'fc_gud_0018.html'
                      },
                      {
                        id: 1527,
                        parentId: 1519,
                        name: '步骤8：执行备份',
                        local: 'fc_gud_0019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1510,
                parentId: 1404,
                name: '复制',
                local: 'fc_gud_0022.html',
                children: [
                  {
                    id: 1528,
                    parentId: 1510,
                    name: '复制FusionCompute虚拟机副本',
                    local: 'fc_gud_0024.html',
                    children: [
                      {
                        id: 1529,
                        parentId: 1528,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'fc_gud_0026_2.html'
                      },
                      {
                        id: 1530,
                        parentId: 1528,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0026_1_3.html'
                      },
                      {
                        id: 1531,
                        parentId: 1528,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'fc_gud_0027.html'
                      },
                      {
                        id: 1532,
                        parentId: 1528,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'fc_gud_0028.html'
                      },
                      {
                        id: 1533,
                        parentId: 1528,
                        name: '步骤4：下载并导入证书',
                        local: 'fc_gud_0029.html'
                      },
                      {
                        id: 1534,
                        parentId: 1528,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'fc_gud_0030.html'
                      },
                      {
                        id: 1535,
                        parentId: 1528,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'fc_gud_0030_1.html'
                      },
                      {
                        id: 1536,
                        parentId: 1528,
                        name: '步骤6：添加复制集群',
                        local: 'fc_gud_0031.html'
                      },
                      {
                        id: 1537,
                        parentId: 1528,
                        name: '步骤7：创建复制SLA',
                        local: 'fc_gud_0032.html'
                      },
                      {
                        id: 1538,
                        parentId: 1528,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'fc_gud_0033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1511,
                parentId: 1404,
                name: '归档',
                local: 'fc_gud_0034.html',
                children: [
                  {
                    id: 1539,
                    parentId: 1511,
                    name: '归档FusionCompute备份副本',
                    local: 'fc_gud_0037.html',
                    children: [
                      {
                        id: 1541,
                        parentId: 1539,
                        name: '步骤1：添加归档存储',
                        local: 'fc_gud_0038.html',
                        children: [
                          {
                            id: 1543,
                            parentId: 1541,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'fc_gud_0039.html'
                          },
                          {
                            id: 1544,
                            parentId: 1541,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'fc_gud_0040.html'
                          }
                        ]
                      },
                      {
                        id: 1542,
                        parentId: 1539,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'fc_gud_0041.html'
                      }
                    ]
                  },
                  {
                    id: 1540,
                    parentId: 1511,
                    name: '归档FusionCompute复制副本',
                    local: 'fc_gud_0042.html',
                    children: [
                      {
                        id: 1545,
                        parentId: 1540,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'fc_gud_0043.html'
                      },
                      {
                        id: 1546,
                        parentId: 1540,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'fc_gud_0044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1512,
                parentId: 1404,
                name: '恢复',
                local: 'fc_gud_0045.html',
                children: [
                  {
                    id: 1547,
                    parentId: 1512,
                    name: '恢复FusionCompute虚拟机',
                    local: 'fc_gud_0048.html'
                  },
                  {
                    id: 1548,
                    parentId: 1512,
                    name: '恢复FusionCompute虚拟机磁盘',
                    local: 'fc_gud_0049.html'
                  },
                  {
                    id: 1549,
                    parentId: 1512,
                    name: '恢复FusionCompute虚拟机中的文件',
                    local: 'fc_gud_re1.html'
                  }
                ]
              },
              {
                id: 1513,
                parentId: 1404,
                name: '全局搜索',
                local: 'fc_gud_gs1.html',
                children: [
                  {
                    id: 1550,
                    parentId: 1513,
                    name: '关于全局搜索',
                    local: 'fc_gud_gs2_0.html'
                  },
                  {
                    id: 1551,
                    parentId: 1513,
                    name: '全局搜索副本数据',
                    local: 'fc_gud_gs3.html'
                  },
                  {
                    id: 1552,
                    parentId: 1513,
                    name: '全局搜索资源',
                    local: 'fc_gud_0050.html'
                  },
                  {
                    id: 1553,
                    parentId: 1513,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'fc_gud_0050_1.html'
                  }
                ]
              },
              {
                id: 1514,
                parentId: 1404,
                name: 'SLA',
                local: 'fc_gud_0053.html',
                children: [
                  {
                    id: 1554,
                    parentId: 1514,
                    name: '关于SLA',
                    local: 'fc_gud_0054.html'
                  },
                  {
                    id: 1555,
                    parentId: 1514,
                    name: '查看SLA信息',
                    local: 'fc_gud_0055.html'
                  },
                  {
                    id: 1556,
                    parentId: 1514,
                    name: '管理SLA',
                    local: 'fc_gud_0056.html'
                  }
                ]
              },
              {
                id: 1515,
                parentId: 1404,
                name: '副本',
                local: 'fc_gud_0057.html',
                children: [
                  {
                    id: 1557,
                    parentId: 1515,
                    name: '查看FusionCompute副本信息',
                    local: 'fc_gud_0058.html'
                  },
                  {
                    id: 1558,
                    parentId: 1515,
                    name: '管理FusionCompute副本',
                    local: 'fc_gud_0059.html'
                  }
                ]
              },
              {
                id: 1516,
                parentId: 1404,
                name: 'FusionCompute虚拟化环境',
                local: 'fc_gud_0060.html',
                children: [
                  {
                    id: 1559,
                    parentId: 1516,
                    name: '查看FusionCompute虚拟化环境信息',
                    local: 'fc_gud_0061.html'
                  },
                  {
                    id: 1560,
                    parentId: 1516,
                    name: '管理FusionCompute注册信息',
                    local: 'fc_gud_0062.html'
                  },
                  {
                    id: 1561,
                    parentId: 1516,
                    name: '管理集群/主机/虚拟机/虚拟机组',
                    local: 'fc_gud_0063.html'
                  }
                ]
              },
              {
                id: 1517,
                parentId: 1404,
                name: '常见问题',
                local: 'fc_gud_0064.html',
                children: [
                  {
                    id: 1562,
                    parentId: 1517,
                    name: '登录DeviceManager管理界面',
                    local: 'fc_gud_dm.html'
                  },
                  {
                    id: 1563,
                    parentId: 1517,
                    name: '备份恢复传输模式（适用于FusionCompute）',
                    local: 'protectagent_install_0072.html'
                  },
                  {
                    id: 1564,
                    parentId: 1517,
                    name: '配置FusionCompute角色权限',
                    local: 'fc_gud_0066.html'
                  },
                  {
                    id: 1565,
                    parentId: 1517,
                    name: '如何获取存储设备CA证书',
                    local: 'fc_gud_0066_2.html'
                  },
                  {
                    id: 1566,
                    parentId: 1517,
                    name:
                      '配置备份网络平面（适用于FusionCompute 8.7.0及后续版本）',
                    local: 'fc_gud_0079.html'
                  },
                  {
                    id: 1567,
                    parentId: 1517,
                    name:
                      '整机恢复到新位置失败，错误详情包含"ErrorCode: 10300005"',
                    local: 'fc_gud_0067.html'
                  },
                  {
                    id: 1568,
                    parentId: 1517,
                    name: '备份任务失败，错误详情提示客户端执行挂载失败',
                    local: 'fc_gud_0075.html'
                  },
                  {
                    id: 1569,
                    parentId: 1517,
                    name:
                      '备份恢复任务失败，任务一定时间内进度未更新或错误详情包含“执行业务子任务失败”',
                    local: 'fc_gud_0076.html'
                  },
                  {
                    id: 1570,
                    parentId: 1517,
                    name: '本产品出现业务中断或FusionCompute注册失败',
                    local: 'fc_gud_0077.html'
                  },
                  {
                    id: 1571,
                    parentId: 1517,
                    name:
                      '对FusionCompute生产环境执行资源扫描任务失败或结果延迟',
                    local: 'fc_gud_0078.html'
                  },
                  {
                    id: 1572,
                    parentId: 1517,
                    name: 'FusionCompute节点故障导致备份恢复任务失败',
                    local: 'fc_gud_0078_1.html'
                  },
                  {
                    id: 1573,
                    parentId: 1517,
                    name: '备份任务失败，错误详情包含"ErrorCode: 00000000"',
                    local: 'fc_gud_0078_2.html'
                  },
                  {
                    id: 1574,
                    parentId: 1517,
                    name: '数据盘恢复到新位置后虚拟机磁盘数量变多',
                    local: 'fc_gud_0078_3.html'
                  },
                  {
                    id: 1575,
                    parentId: 1517,
                    name: '虚拟机磁盘覆盖恢复后业务数据异常',
                    local: 'fc_gud_0078_4.html'
                  },
                  {
                    id: 1576,
                    parentId: 1517,
                    name:
                      'FusionCompute系统盘恢复到新虚拟机后目标虚拟机启动失败',
                    local: 'fc_gud_0147.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1405,
            parentId: 15,
            name: 'CNware数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002164607758.html',
            children: [
              {
                id: 1577,
                parentId: 1405,
                name: '备份',
                local: 'cnware_00008.html',
                children: [
                  {
                    id: 1587,
                    parentId: 1577,
                    name: '备份前准备',
                    local: 'cnware_00011.html'
                  },
                  {
                    id: 1588,
                    parentId: 1577,
                    name: '备份CNware虚拟机',
                    local: 'cnware_00012.html',
                    children: [
                      {
                        id: 1589,
                        parentId: 1588,
                        name: '步骤1：申请备份账号',
                        local: 'cnware_00013.html'
                      },
                      {
                        id: 1590,
                        parentId: 1588,
                        name: '步骤2：获取CNware证书',
                        local: 'cnware_00014.html'
                      },
                      {
                        id: 1591,
                        parentId: 1588,
                        name: '步骤3：注册CNware虚拟化环境',
                        local: 'cnware_00015.html'
                      },
                      {
                        id: 1592,
                        parentId: 1588,
                        name: '步骤4：（可选）创建CNware虚拟机组',
                        local: 'cnware_00016.html'
                      },
                      {
                        id: 1593,
                        parentId: 1588,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'cnware_00017.html'
                      },
                      {
                        id: 1594,
                        parentId: 1588,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'cnware_00018.html'
                      },
                      {
                        id: 1595,
                        parentId: 1588,
                        name: '步骤7：创建备份SLA',
                        local: 'cnware_00019.html'
                      },
                      {
                        id: 1596,
                        parentId: 1588,
                        name: '步骤8：执行备份',
                        local: 'cnware_00020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1578,
                parentId: 1405,
                name: '复制',
                local: 'cnware_00023.html',
                children: [
                  {
                    id: 1597,
                    parentId: 1578,
                    name: '复制CNware虚拟机副本',
                    local: 'cnware_00027.html',
                    children: [
                      {
                        id: 1598,
                        parentId: 1597,
                        name: '步骤1：创建复制网络逻辑端口',
                        local: 'cnware_00028.html'
                      },
                      {
                        id: 1599,
                        parentId: 1597,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'cnware_00029.html'
                      },
                      {
                        id: 1600,
                        parentId: 1597,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'cnware_00030.html'
                      },
                      {
                        id: 1601,
                        parentId: 1597,
                        name: '步骤4：下载并导入证书',
                        local: 'cnware_00031.html'
                      },
                      {
                        id: 1602,
                        parentId: 1597,
                        name: '步骤5：创建远端设备管理员',
                        local: 'cnware_00032.html'
                      },
                      {
                        id: 1603,
                        parentId: 1597,
                        name: '步骤6：添加复制集群',
                        local: 'cnware_00033.html'
                      },
                      {
                        id: 1604,
                        parentId: 1597,
                        name: '步骤7：创建复制SLA',
                        local: 'cnware_00034.html'
                      },
                      {
                        id: 1605,
                        parentId: 1597,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'cnware_00035.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1579,
                parentId: 1405,
                name: '归档',
                local: 'cnware_00055.html',
                children: [
                  {
                    id: 1606,
                    parentId: 1579,
                    name: '归档CNware备份副本',
                    local: 'cnware_00058.html',
                    children: [
                      {
                        id: 1608,
                        parentId: 1606,
                        name: '步骤1：添加归档存储',
                        local: 'cnware_00059.html',
                        children: [
                          {
                            id: 1610,
                            parentId: 1608,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'cnware_00060.html'
                          },
                          {
                            id: 1611,
                            parentId: 1608,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'cnware_00061.html'
                          }
                        ]
                      },
                      {
                        id: 1609,
                        parentId: 1606,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'cnware_00062.html'
                      }
                    ]
                  },
                  {
                    id: 1607,
                    parentId: 1579,
                    name: '归档CNware复制副本',
                    local: 'cnware_00063.html',
                    children: [
                      {
                        id: 1612,
                        parentId: 1607,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'cnware_00064.html'
                      },
                      {
                        id: 1613,
                        parentId: 1607,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'cnware_00065.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1580,
                parentId: 1405,
                name: '恢复',
                local: 'cnware_00066.html',
                children: [
                  {
                    id: 1614,
                    parentId: 1580,
                    name: '恢复CNware虚拟机',
                    local: 'cnware_00069.html'
                  },
                  {
                    id: 1615,
                    parentId: 1580,
                    name: '恢复CNware虚拟机磁盘',
                    local: 'cnware_00070.html'
                  }
                ]
              },
              {
                id: 1581,
                parentId: 1405,
                name: '即时恢复',
                local: 'cnware_00072.html',
                children: [
                  {
                    id: 1616,
                    parentId: 1581,
                    name: '即时恢复CNware虚拟机',
                    local: 'cnware_00075.html'
                  }
                ]
              },
              {
                id: 1582,
                parentId: 1405,
                name: '全局搜索',
                local: 'cnware_00085.html',
                children: [
                  {
                    id: 1617,
                    parentId: 1582,
                    name: '关于全局搜索',
                    local: 'cnware_00086.html'
                  },
                  {
                    id: 1618,
                    parentId: 1582,
                    name: '全局搜索副本数据',
                    local: 'cnware_00087.html'
                  },
                  {
                    id: 1619,
                    parentId: 1582,
                    name: '全局搜索资源',
                    local: 'cnware_00088.html'
                  },
                  {
                    id: 1620,
                    parentId: 1582,
                    name: '全局标签搜索',
                    local: 'cnware_00089.html'
                  }
                ]
              },
              {
                id: 1583,
                parentId: 1405,
                name: 'SLA',
                local: 'cnware_00092.html',
                children: [
                  {
                    id: 1621,
                    parentId: 1583,
                    name: '关于SLA',
                    local: 'cnware_00093.html'
                  },
                  {
                    id: 1622,
                    parentId: 1583,
                    name: '查看SLA信息',
                    local: 'cnware_00094.html'
                  },
                  {
                    id: 1623,
                    parentId: 1583,
                    name: '管理SLA',
                    local: 'cnware_00095.html'
                  }
                ]
              },
              {
                id: 1584,
                parentId: 1405,
                name: '副本',
                local: 'cnware_00096.html',
                children: [
                  {
                    id: 1624,
                    parentId: 1584,
                    name: '查看CNware副本信息',
                    local: 'cnware_00097.html'
                  },
                  {
                    id: 1625,
                    parentId: 1584,
                    name: '管理CNware副本',
                    local: 'cnware_00098.html'
                  }
                ]
              },
              {
                id: 1585,
                parentId: 1405,
                name: 'CNware虚拟化环境',
                local: 'cnware_00099.html',
                children: [
                  {
                    id: 1626,
                    parentId: 1585,
                    name: '查看CNware虚拟化环境信息',
                    local: 'cnware_00100.html'
                  },
                  {
                    id: 1627,
                    parentId: 1585,
                    name: '管理CNware注册信息',
                    local: 'cnware_00101.html'
                  },
                  {
                    id: 1628,
                    parentId: 1585,
                    name: '管理虚拟机/主机/集群',
                    local: 'cnware_00102.html'
                  }
                ]
              },
              {
                id: 1586,
                parentId: 1405,
                name: '常见问题',
                local: 'cnware_00103.html',
                children: [
                  {
                    id: 1629,
                    parentId: 1586,
                    name: '注册CNware虚拟化环境时，注册用户的最小权限要求',
                    local: 'cnware_00108.html'
                  },
                  {
                    id: 1630,
                    parentId: 1586,
                    name: '如何在CNware云平台添加用户权限',
                    local: 'cnware_00109.html'
                  },
                  {
                    id: 1631,
                    parentId: 1586,
                    name: '恢复任务失败，界面提示新增磁盘失败',
                    local: 'cnware_00110.html'
                  },
                  {
                    id: 1632,
                    parentId: 1586,
                    name:
                      '即时挂载/即时恢复任务失败，界面提示存储设备扫描存储资源失败',
                    local: 'cnware_00111.html'
                  },
                  {
                    id: 1633,
                    parentId: 1586,
                    name: '备份任务失败/资源链路离线',
                    local: 'cnware_00112.html'
                  },
                  {
                    id: 1634,
                    parentId: 1586,
                    name: '资源扫描或备份恢复任务失败，无明显错误提示',
                    local: 'cnware_00113.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1406,
            parentId: 15,
            name: 'Hyper-V数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002164607774.html',
            children: [
              {
                id: 1635,
                parentId: 1406,
                name: '备份',
                local: 'hyper_gud_0009.html',
                children: [
                  {
                    id: 1644,
                    parentId: 1635,
                    name: '备份前准备',
                    local: 'hyper_gud_0013.html'
                  },
                  {
                    id: 1645,
                    parentId: 1635,
                    name: '备份Hyper-V虚拟机',
                    local: 'hyper_gud_0014.html',
                    children: [
                      {
                        id: 1646,
                        parentId: 1645,
                        name: '步骤1：注册Hyper-V虚拟化环境',
                        local: 'hyper_gud_0015.html'
                      },
                      {
                        id: 1647,
                        parentId: 1645,
                        name: '步骤2：（可选）创建Hyper-V虚拟机组',
                        local: 'hyper_gud_0016.html'
                      },
                      {
                        id: 1648,
                        parentId: 1645,
                        name: '步骤3：（可选）创建限速策略',
                        local: 'hyper_gud_0017.html'
                      },
                      {
                        id: 1649,
                        parentId: 1645,
                        name: '步骤4：创建备份SLA',
                        local: 'hyper_gud_0018.html'
                      },
                      {
                        id: 1650,
                        parentId: 1645,
                        name: '步骤5：执行备份',
                        local: 'hyper_gud_0019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1636,
                parentId: 1406,
                name: '复制',
                local: 'hyper_gud_0020.html',
                children: [
                  {
                    id: 1651,
                    parentId: 1636,
                    name: '复制Hyper-V虚拟机副本',
                    local: 'hyper_gud_0024.html',
                    children: [
                      {
                        id: 1652,
                        parentId: 1651,
                        name: '步骤1：创建复制网络逻辑端口',
                        local: 'hyper_gud_0025.html'
                      },
                      {
                        id: 1653,
                        parentId: 1651,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'hyper_gud_0026.html'
                      },
                      {
                        id: 1654,
                        parentId: 1651,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'hyper_gud_0027.html'
                      },
                      {
                        id: 1655,
                        parentId: 1651,
                        name: '步骤4：下载并导入证书',
                        local: 'hyper_gud_0028.html'
                      },
                      {
                        id: 1656,
                        parentId: 1651,
                        name: '步骤5：创建远端设备管理员',
                        local: 'hyper_gud_0029.html'
                      },
                      {
                        id: 1657,
                        parentId: 1651,
                        name: '步骤6：添加复制集群',
                        local: 'hyper_gud_0030.html'
                      },
                      {
                        id: 1658,
                        parentId: 1651,
                        name: '步骤7：创建复制SLA',
                        local: 'hyper_gud_0031.html'
                      },
                      {
                        id: 1659,
                        parentId: 1651,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'hyper_gud_0032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1637,
                parentId: 1406,
                name: '归档',
                local: 'hyper_gud_0052.html',
                children: [
                  {
                    id: 1660,
                    parentId: 1637,
                    name: '归档Hyper-V备份副本',
                    local: 'hyper_gud_0055.html',
                    children: [
                      {
                        id: 1662,
                        parentId: 1660,
                        name: '步骤1：添加归档存储',
                        local: 'hyper_gud_0056.html',
                        children: [
                          {
                            id: 1664,
                            parentId: 1662,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'hyper_gud_0057.html'
                          },
                          {
                            id: 1665,
                            parentId: 1662,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'hyper_gud_0058.html'
                          }
                        ]
                      },
                      {
                        id: 1663,
                        parentId: 1660,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'hyper_gud_0059.html'
                      }
                    ]
                  },
                  {
                    id: 1661,
                    parentId: 1637,
                    name: '归档Hyper-V复制副本',
                    local: 'hyper_gud_0060.html',
                    children: [
                      {
                        id: 1666,
                        parentId: 1661,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'hyper_gud_0061.html'
                      },
                      {
                        id: 1667,
                        parentId: 1661,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'hyper_gud_0062.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1638,
                parentId: 1406,
                name: '恢复',
                local: 'hyper_gud_0063.html',
                children: [
                  {
                    id: 1668,
                    parentId: 1638,
                    name: '恢复Hyper-V虚拟机',
                    local: 'hyper_gud_0066.html'
                  },
                  {
                    id: 1669,
                    parentId: 1638,
                    name: '恢复Hyper-V虚拟机磁盘',
                    local: 'hyper_gud_0067.html'
                  },
                  {
                    id: 1670,
                    parentId: 1638,
                    name: '恢复Hyper-V虚拟机中的文件',
                    local: 'hyper_gud_0068.html'
                  }
                ]
              },
              {
                id: 1639,
                parentId: 1406,
                name: '全局搜索',
                local: 'hyper_gud_0069.html',
                children: [
                  {
                    id: 1671,
                    parentId: 1639,
                    name: '全局搜索副本数据',
                    local: 'hyper_gud_0070.html'
                  },
                  {
                    id: 1672,
                    parentId: 1639,
                    name: '全局搜索资源',
                    local: 'hyper_gud_0071.html'
                  },
                  {
                    id: 1673,
                    parentId: 1639,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'hyper_gud_0072.html'
                  }
                ]
              },
              {
                id: 1640,
                parentId: 1406,
                name: 'SLA',
                local: 'hyper_gud_0075.html',
                children: [
                  {
                    id: 1674,
                    parentId: 1640,
                    name: '关于SLA',
                    local: 'hyper_gud_0076.html'
                  },
                  {
                    id: 1675,
                    parentId: 1640,
                    name: '查看SLA信息',
                    local: 'hyper_gud_0077.html'
                  },
                  {
                    id: 1676,
                    parentId: 1640,
                    name: '管理SLA',
                    local: 'hyper_gud_0078.html'
                  }
                ]
              },
              {
                id: 1641,
                parentId: 1406,
                name: '副本',
                local: 'hyper_gud_0079.html',
                children: [
                  {
                    id: 1677,
                    parentId: 1641,
                    name: '查看Hyper-V副本信息',
                    local: 'hyper_gud_0080.html'
                  },
                  {
                    id: 1678,
                    parentId: 1641,
                    name: '管理Hyper-V副本',
                    local: 'hyper_gud_0081.html'
                  }
                ]
              },
              {
                id: 1642,
                parentId: 1406,
                name: 'Hyper-V虚拟化环境',
                local: 'hyper_gud_0082.html',
                children: [
                  {
                    id: 1679,
                    parentId: 1642,
                    name: '查看Hyper-V虚拟化环境信息',
                    local: 'hyper_gud_0083.html'
                  },
                  {
                    id: 1680,
                    parentId: 1642,
                    name: '管理Hyper-V注册信息',
                    local: 'hyper_gud_0084.html'
                  },
                  {
                    id: 1681,
                    parentId: 1642,
                    name: '管理集群/主机/虚拟机/虚拟机组',
                    local: 'hyper_gud_0085.html'
                  }
                ]
              },
              {
                id: 1643,
                parentId: 1406,
                name: '常见问题',
                local: 'hyper_gud_0086.html',
                children: [
                  {
                    id: 1682,
                    parentId: 1643,
                    name: '登录DeviceManager管理界面',
                    local: 'hyper_gud_0090.html'
                  },
                  {
                    id: 1683,
                    parentId: 1643,
                    name: '虚拟机磁盘覆盖恢复后业务数据异常',
                    local: 'hyper_gud_0092.html'
                  },
                  {
                    id: 1684,
                    parentId: 1643,
                    name:
                      '执行Hyper-V备份任务失败，错误详情包含“Read file failed, error: 22”',
                    local: 'hyper_gud_0093.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1407,
            parentId: 15,
            name: 'FusionOne Compute数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002200094125.html',
            children: [
              {
                id: 1685,
                parentId: 1407,
                name: '备份',
                local: 'foc_gud_0009.html',
                children: [
                  {
                    id: 1694,
                    parentId: 1685,
                    name: '备份前准备',
                    local: 'foc_gud_0012.html'
                  },
                  {
                    id: 1695,
                    parentId: 1685,
                    name: '备份FusionOne Compute虚拟机',
                    local: 'foc_gud_0013.html',
                    children: [
                      {
                        id: 1696,
                        parentId: 1695,
                        name: '步骤1：创建FusionOne Compute对接用户',
                        local: 'foc_gud_0014.html'
                      },
                      {
                        id: 1697,
                        parentId: 1695,
                        name: '步骤2：注册FusionOne Compute虚拟化环境',
                        local: 'foc_gud_0015.html'
                      },
                      {
                        id: 1698,
                        parentId: 1695,
                        name: '步骤3：（可选）创建FusionOne Compute虚拟机组',
                        local: 'foc_gud_0016.html'
                      },
                      {
                        id: 1699,
                        parentId: 1695,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'foc_gud_0017.html'
                      },
                      {
                        id: 1700,
                        parentId: 1695,
                        name: '步骤5：（可选）开启备份链路加密开关',
                        local: 'foc_gud_0018.html'
                      },
                      {
                        id: 1701,
                        parentId: 1695,
                        name: '步骤6：创建备份SLA',
                        local: 'foc_gud_0019.html'
                      },
                      {
                        id: 1702,
                        parentId: 1695,
                        name: '步骤7：执行备份',
                        local: 'foc_gud_0020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1686,
                parentId: 1407,
                name: '复制',
                local: 'foc_gud_0021.html',
                children: [
                  {
                    id: 1703,
                    parentId: 1686,
                    name: '复制FusionOne Compute虚拟机副本',
                    local: 'foc_gud_0024.html',
                    children: [
                      {
                        id: 1704,
                        parentId: 1703,
                        name: '步骤1：创建复制网络逻辑端口',
                        local: 'foc_gud_0026.html'
                      },
                      {
                        id: 1705,
                        parentId: 1703,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'foc_gud_0027.html'
                      },
                      {
                        id: 1706,
                        parentId: 1703,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'foc_gud_0028.html'
                      },
                      {
                        id: 1707,
                        parentId: 1703,
                        name: '步骤4：下载并导入证书',
                        local: 'foc_gud_0029.html'
                      },
                      {
                        id: 1708,
                        parentId: 1703,
                        name: '步骤5：创建远端设备管理员',
                        local: 'foc_gud_0031.html'
                      },
                      {
                        id: 1709,
                        parentId: 1703,
                        name: '步骤6：添加复制集群',
                        local: 'foc_gud_0032.html'
                      },
                      {
                        id: 1710,
                        parentId: 1703,
                        name: '步骤7：创建复制SLA',
                        local: 'foc_gud_0033.html'
                      },
                      {
                        id: 1711,
                        parentId: 1703,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'foc_gud_0034.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1687,
                parentId: 1407,
                name: '归档',
                local: 'foc_gud_0035.html',
                children: [
                  {
                    id: 1712,
                    parentId: 1687,
                    name: '归档FusionOne Compute备份副本',
                    local: 'foc_gud_0038.html',
                    children: [
                      {
                        id: 1714,
                        parentId: 1712,
                        name: '步骤1：添加归档存储',
                        local: 'foc_gud_0039.html',
                        children: [
                          {
                            id: 1716,
                            parentId: 1714,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'foc_gud_0040.html'
                          },
                          {
                            id: 1717,
                            parentId: 1714,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'foc_gud_0041.html'
                          }
                        ]
                      },
                      {
                        id: 1715,
                        parentId: 1712,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'foc_gud_0042.html'
                      }
                    ]
                  },
                  {
                    id: 1713,
                    parentId: 1687,
                    name: '归档FusionOne Compute复制副本',
                    local: 'foc_gud_0043.html',
                    children: [
                      {
                        id: 1718,
                        parentId: 1713,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'foc_gud_0044.html'
                      },
                      {
                        id: 1719,
                        parentId: 1713,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'foc_gud_0045.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1688,
                parentId: 1407,
                name: '恢复',
                local: 'foc_gud_0046.html',
                children: [
                  {
                    id: 1720,
                    parentId: 1688,
                    name: '恢复FusionOne Compute虚拟机',
                    local: 'foc_gud_0049.html'
                  },
                  {
                    id: 1721,
                    parentId: 1688,
                    name: '恢复FusionOne Compute虚拟机磁盘',
                    local: 'foc_gud_0050.html'
                  },
                  {
                    id: 1722,
                    parentId: 1688,
                    name: '恢复FusionOne Compute虚拟机中的文件',
                    local: 'foc_gud_0051.html'
                  }
                ]
              },
              {
                id: 1689,
                parentId: 1407,
                name: '全局搜索',
                local: 'foc_gud_0052.html',
                children: [
                  {
                    id: 1723,
                    parentId: 1689,
                    name: '关于全局搜索',
                    local: 'foc_gud_0053.html'
                  },
                  {
                    id: 1724,
                    parentId: 1689,
                    name: '全局搜索副本数据',
                    local: 'foc_gud_0054.html'
                  },
                  {
                    id: 1725,
                    parentId: 1689,
                    name: '全局搜索资源',
                    local: 'foc_gud_0055.html'
                  },
                  {
                    id: 1726,
                    parentId: 1689,
                    name: '全局标签搜索',
                    local: 'foc_gud_0056.html'
                  }
                ]
              },
              {
                id: 1690,
                parentId: 1407,
                name: 'SLA',
                local: 'foc_gud_0059.html',
                children: [
                  {
                    id: 1727,
                    parentId: 1690,
                    name: '关于SLA',
                    local: 'foc_gud_0060.html'
                  },
                  {
                    id: 1728,
                    parentId: 1690,
                    name: '查看SLA信息',
                    local: 'foc_gud_0061.html'
                  },
                  {
                    id: 1729,
                    parentId: 1690,
                    name: '管理SLA',
                    local: 'foc_gud_0062.html'
                  }
                ]
              },
              {
                id: 1691,
                parentId: 1407,
                name: '副本',
                local: 'foc_gud_0063.html',
                children: [
                  {
                    id: 1730,
                    parentId: 1691,
                    name: '查看FusionOne Compute副本信息',
                    local: 'foc_gud_0064.html'
                  },
                  {
                    id: 1731,
                    parentId: 1691,
                    name: '管理FusionOne Compute副本',
                    local: 'foc_gud_0065.html'
                  }
                ]
              },
              {
                id: 1692,
                parentId: 1407,
                name: 'FusionOne Compute虚拟化环境',
                local: 'foc_gud_0066.html',
                children: [
                  {
                    id: 1732,
                    parentId: 1692,
                    name: '查看FusionOne Compute虚拟化环境信息',
                    local: 'foc_gud_0067.html'
                  },
                  {
                    id: 1733,
                    parentId: 1692,
                    name: '管理FusionOne Compute注册信息',
                    local: 'foc_gud_0068.html'
                  },
                  {
                    id: 1734,
                    parentId: 1692,
                    name: '管理集群/主机/虚拟机/虚拟机组',
                    local: 'foc_gud_0069.html'
                  }
                ]
              },
              {
                id: 1693,
                parentId: 1407,
                name: '常见问题',
                local: 'foc_gud_0070.html',
                children: [
                  {
                    id: 1735,
                    parentId: 1693,
                    name: '登录DeviceManager管理界面',
                    local: 'foc_gud_0073.html'
                  },
                  {
                    id: 1736,
                    parentId: 1693,
                    name: '备份恢复传输模式（适用于FusionOne Compute）',
                    local: 'foc_gud_0073_3.html'
                  },
                  {
                    id: 1737,
                    parentId: 1693,
                    name: '配置FusionOne Compute角色权限',
                    local: 'foc_gud_0074.html'
                  },
                  {
                    id: 1738,
                    parentId: 1693,
                    name:
                      '整机恢复到新位置失败，错误详情包含"ErrorCode: 10300005"',
                    local: 'foc_gud_0075.html'
                  },
                  {
                    id: 1739,
                    parentId: 1693,
                    name: '备份任务失败，错误详情提示客户端执行挂载失败',
                    local: 'foc_gud_0076.html'
                  },
                  {
                    id: 1740,
                    parentId: 1693,
                    name:
                      '对FusionOne Compute生产环境执行资源扫描任务失败或结果延迟',
                    local: 'foc_gud_0078.html'
                  },
                  {
                    id: 1741,
                    parentId: 1693,
                    name: 'FusionOne Compute节点故障导致备份恢复任务失败',
                    local: 'foc_gud_0079.html'
                  },
                  {
                    id: 1742,
                    parentId: 1693,
                    name: '虚拟机磁盘覆盖恢复后业务数据异常',
                    local: 'foc_gud_0078_4.html'
                  },
                  {
                    id: 1743,
                    parentId: 1693,
                    name:
                      'FusionOne Compute系统盘恢复到新虚拟机后目标虚拟机启动失败',
                    local: 'foc_gud_0147.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1408,
            parentId: 15,
            name: 'Nutanix数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002164767518.html',
            children: [
              {
                id: 1744,
                parentId: 1408,
                name: '概述',
                local: 'Nutanix_0003.html',
                children: [
                  {
                    id: 1754,
                    parentId: 1744,
                    name: '简介',
                    local: 'Nutanix_0004.html'
                  },
                  {
                    id: 1755,
                    parentId: 1744,
                    name: '功能概览',
                    local: 'Nutanix_0005.html'
                  },
                  {
                    id: 1756,
                    parentId: 1744,
                    name: '约束与限制',
                    local: 'Nutanix_0006.html'
                  }
                ]
              },
              {
                id: 1745,
                parentId: 1408,
                name: '备份',
                local: 'Nutanix_0008.html',
                children: [
                  {
                    id: 1757,
                    parentId: 1745,
                    name: '备份前准备',
                    local: 'Nutanix_0011.html'
                  },
                  {
                    id: 1758,
                    parentId: 1745,
                    name: '备份Nutanix虚拟化环境',
                    local: 'Nutanix_0012.html',
                    children: [
                      {
                        id: 1759,
                        parentId: 1758,
                        name:
                          '步骤1：在Nutanix虚拟化平台注册用户并添加用户权限',
                        local: 'Nutanix_0013.html'
                      },
                      {
                        id: 1760,
                        parentId: 1758,
                        name: '步骤2：获取Nutanix证书',
                        local: 'Nutanix_0014.html'
                      },
                      {
                        id: 1761,
                        parentId: 1758,
                        name: '步骤3：注册Nutanix虚拟化环境',
                        local: 'Nutanix_0015.html'
                      },
                      {
                        id: 1762,
                        parentId: 1758,
                        name: '步骤4：（可选）创建Nutanix虚拟机组',
                        local: 'Nutanix_0016.html'
                      },
                      {
                        id: 1763,
                        parentId: 1758,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'Nutanix_0017.html'
                      },
                      {
                        id: 1764,
                        parentId: 1758,
                        name: '步骤6：（可选）开启备份链路加密开关',
                        local: 'Nutanix_0018.html'
                      },
                      {
                        id: 1765,
                        parentId: 1758,
                        name: '步骤7：创建备份SLA',
                        local: 'Nutanix_0019_a.html'
                      },
                      {
                        id: 1766,
                        parentId: 1758,
                        name: '步骤8：执行备份',
                        local: 'Nutanix_0020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1746,
                parentId: 1408,
                name: '复制',
                local: 'Nutanix_0021.html',
                children: [
                  {
                    id: 1767,
                    parentId: 1746,
                    name: '复制Nutanix副本',
                    local: 'Nutanix_0025.html',
                    children: [
                      {
                        id: 1768,
                        parentId: 1767,
                        name: '步骤1：创建复制网络逻辑端口',
                        local: 'Nutanix_0026.html'
                      },
                      {
                        id: 1769,
                        parentId: 1767,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'Nutanix_0027.html'
                      },
                      {
                        id: 1770,
                        parentId: 1767,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'Nutanix_0028.html'
                      },
                      {
                        id: 1771,
                        parentId: 1767,
                        name: '步骤4：下载并导入证书',
                        local: 'Nutanix_0029.html'
                      },
                      {
                        id: 1772,
                        parentId: 1767,
                        name: '步骤5：创建远端设备管理员',
                        local: 'Nutanix_0030.html'
                      },
                      {
                        id: 1773,
                        parentId: 1767,
                        name: '步骤6：添加复制集群',
                        local: 'Nutanix_0031.html'
                      },
                      {
                        id: 1774,
                        parentId: 1767,
                        name: '步骤7：创建复制SLA',
                        local: 'Nutanix_0031_a.html'
                      },
                      {
                        id: 1775,
                        parentId: 1767,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'Nutanix_0033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1747,
                parentId: 1408,
                name: '归档',
                local: 'Nutanix_0064.html',
                children: [
                  {
                    id: 1776,
                    parentId: 1747,
                    name: '归档Nutanix备份副本',
                    local: 'Nutanix_0067.html',
                    children: [
                      {
                        id: 1778,
                        parentId: 1776,
                        name: '步骤1：添加归档存储',
                        local: 'Nutanix_0068.html',
                        children: [
                          {
                            id: 1780,
                            parentId: 1778,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'Nutanix_0069.html'
                          },
                          {
                            id: 1781,
                            parentId: 1778,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'Nutanix_0070.html'
                          }
                        ]
                      },
                      {
                        id: 1779,
                        parentId: 1776,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'Nutanix_0071.html'
                      }
                    ]
                  },
                  {
                    id: 1777,
                    parentId: 1747,
                    name: '归档Nutanix复制副本',
                    local: 'Nutanix_0072.html',
                    children: [
                      {
                        id: 1782,
                        parentId: 1777,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'Nutanix_0073.html'
                      },
                      {
                        id: 1783,
                        parentId: 1777,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'Nutanix_0074.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1748,
                parentId: 1408,
                name: '恢复',
                local: 'Nutanix_0075.html',
                children: [
                  {
                    id: 1784,
                    parentId: 1748,
                    name: '恢复Nutanix虚拟机',
                    local: 'Nutanix_0078.html'
                  }
                ]
              },
              {
                id: 1749,
                parentId: 1408,
                name: '全局搜索',
                local: 'Nutanix_0080.html',
                children: [
                  {
                    id: 1785,
                    parentId: 1749,
                    name: '关于全局搜索',
                    local: 'Nutanix_0081.html'
                  },
                  {
                    id: 1786,
                    parentId: 1749,
                    name: '全局搜索副本数据',
                    local: 'Nutanix_0082.html'
                  },
                  {
                    id: 1787,
                    parentId: 1749,
                    name: '全局搜索资源',
                    local: 'Nutanix_0083.html'
                  },
                  {
                    id: 1788,
                    parentId: 1749,
                    name: '全局标签搜索',
                    local: 'Nutanix_0084.html'
                  }
                ]
              },
              {
                id: 1750,
                parentId: 1408,
                name: 'SLA',
                local: 'Nutanix_0087.html',
                children: [
                  {
                    id: 1789,
                    parentId: 1750,
                    name: '关于SLA',
                    local: 'Nutanix_0088.html'
                  },
                  {
                    id: 1790,
                    parentId: 1750,
                    name: '查看SLA信息',
                    local: 'Nutanix_0089.html'
                  },
                  {
                    id: 1791,
                    parentId: 1750,
                    name: '管理SLA',
                    local: 'Nutanix_0090.html'
                  }
                ]
              },
              {
                id: 1751,
                parentId: 1408,
                name: '副本',
                local: 'Nutanix_0091.html',
                children: [
                  {
                    id: 1792,
                    parentId: 1751,
                    name: '查看Nutanix副本信息',
                    local: 'Nutanix_0092.html'
                  },
                  {
                    id: 1793,
                    parentId: 1751,
                    name: '管理Nutanix副本',
                    local: 'Nutanix_0093.html'
                  }
                ]
              },
              {
                id: 1752,
                parentId: 1408,
                name: 'Nutanix虚拟化环境',
                local: 'Nutanix_0094.html',
                children: [
                  {
                    id: 1794,
                    parentId: 1752,
                    name: '查看Nutanix虚拟化环境信息',
                    local: 'Nutanix_0095.html'
                  },
                  {
                    id: 1795,
                    parentId: 1752,
                    name: '管理Nutanix注册信息',
                    local: 'Nutanix_0096.html'
                  },
                  {
                    id: 1796,
                    parentId: 1752,
                    name: '管理集群/主机/虚拟机/虚拟机组',
                    local: 'Nutanix_0097.html'
                  }
                ]
              },
              {
                id: 1753,
                parentId: 1408,
                name: '常见问题',
                local: 'Nutanix_0098.html',
                children: [
                  {
                    id: 1797,
                    parentId: 1753,
                    name: '登录DeviceManager管理界面',
                    local: 'Nutanix_0100.html'
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
        local: 'zh-cn_topic_0000002164607766.html',
        children: [
          {
            id: 1798,
            parentId: 16,
            name: 'Kubernetes CSI数据保护',
            local: 'zh-cn_topic_0000002164767522.html',
            children: [
              {
                id: 1799,
                parentId: 1798,
                name: '备份',
                local: 'kubernetes_CSI_00008.html',
                children: [
                  {
                    id: 1808,
                    parentId: 1799,
                    name: '备份前准备（适用于FusionCompute）',
                    local: 'kubernetes_CSI_00011.html',
                    children: [
                      {
                        id: 1813,
                        parentId: 1808,
                        name: '上传Kubernetes镜像压缩包至镜像库',
                        local: 'kubernetes_CSI_00012.html'
                      },
                      {
                        id: 1814,
                        parentId: 1808,
                        name: '获取kubeconfig配置文件',
                        local: 'kubernetes_CSI_00013.html'
                      }
                    ]
                  },
                  {
                    id: 1809,
                    parentId: 1799,
                    name: '备份前准备（适用于CCE）',
                    local: 'kubernetes_CSI_00014.html',
                    children: [
                      {
                        id: 1815,
                        parentId: 1809,
                        name: '上传和更新Kubernetes镜像压缩包',
                        local: 'kubernetes_CSI_00015.html'
                      },
                      {
                        id: 1816,
                        parentId: 1809,
                        name: '获取kubeconfig配置文件',
                        local: 'kubernetes_CSI_00016.html'
                      }
                    ]
                  },
                  {
                    id: 1810,
                    parentId: 1799,
                    name: '备份前准备（适用于OpenShift）',
                    local: 'kubernetes_CSI_00017.html',
                    children: [
                      {
                        id: 1817,
                        parentId: 1810,
                        name: '上传Kubernetes镜像压缩包并获取镜像名和Tag信息',
                        local: 'kubernetes_CSI_00018.html'
                      },
                      {
                        id: 1818,
                        parentId: 1810,
                        name: '获取kubeconfig配置文件',
                        local: 'kubernetes_CSI_00019.html'
                      },
                      {
                        id: 1819,
                        parentId: 1810,
                        name: '获取Token信息',
                        local: 'kubernetes_CSI_00020.html'
                      }
                    ]
                  },
                  {
                    id: 1811,
                    parentId: 1799,
                    name: '备份前准备（适用于原生Kubernetes）',
                    local: 'kubernetes_CSI_00021.html',
                    children: [
                      {
                        id: 1820,
                        parentId: 1811,
                        name: '上传Kubernetes镜像压缩包至Kubernetes集群',
                        local: 'kubernetes_CSI_00022.html'
                      },
                      {
                        id: 1821,
                        parentId: 1811,
                        name: '获取kubeconfig配置文件',
                        local: 'kubernetes_CSI_00023.html'
                      }
                    ]
                  },
                  {
                    id: 1812,
                    parentId: 1799,
                    name: '备份命名空间/数据集',
                    local: 'kubernetes_CSI_00024.html',
                    children: [
                      {
                        id: 1822,
                        parentId: 1812,
                        name: '步骤1：（可选）查询Kubernetes集群的节点标签',
                        local: 'kubernetes_CSI_00025.html'
                      },
                      {
                        id: 1823,
                        parentId: 1812,
                        name: '步骤2：（可选）生成最小权限Token',
                        local: 'kubernetes_CSI_00026.html'
                      },
                      {
                        id: 1824,
                        parentId: 1812,
                        name: '步骤3：注册集群',
                        local: 'kubernetes_CSI_00027.html'
                      },
                      {
                        id: 1825,
                        parentId: 1812,
                        name: '步骤4：注册数据集',
                        local: 'kubernetes_CSI_00028.html'
                      },
                      {
                        id: 1826,
                        parentId: 1812,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'kubernetes_CSI_00030.html'
                      },
                      {
                        id: 1827,
                        parentId: 1812,
                        name: '步骤6：创建备份SLA',
                        local: 'kubernetes_CSI_00031.html'
                      },
                      {
                        id: 1828,
                        parentId: 1812,
                        name: '步骤7：执行备份',
                        local: 'kubernetes_CSI_00032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1800,
                parentId: 1798,
                name: '复制',
                local: 'kubernetes_CSI_00035.html',
                children: [
                  {
                    id: 1829,
                    parentId: 1800,
                    name: '复制Kubernetes CSI副本',
                    local: 'kubernetes_CSI_00039.html',
                    children: [
                      {
                        id: 1830,
                        parentId: 1829,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'kubernetes_CSI_00040.html'
                      },
                      {
                        id: 1831,
                        parentId: 1829,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'kubernetes_CSI_00041.html'
                      },
                      {
                        id: 1832,
                        parentId: 1829,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'kubernetes_CSI_00042.html'
                      },
                      {
                        id: 1833,
                        parentId: 1829,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'kubernetes_CSI_00043.html'
                      },
                      {
                        id: 1834,
                        parentId: 1829,
                        name: '步骤4：下载并导入证书',
                        local: 'kubernetes_CSI_00044.html'
                      },
                      {
                        id: 1835,
                        parentId: 1829,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'kubernetes_CSI_00045.html'
                      },
                      {
                        id: 1836,
                        parentId: 1829,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'kubernetes_CSI_00046.html'
                      },
                      {
                        id: 1837,
                        parentId: 1829,
                        name: '步骤6：添加复制集群',
                        local: 'kubernetes_CSI_00047.html'
                      },
                      {
                        id: 1838,
                        parentId: 1829,
                        name: '步骤7：创建复制SLA',
                        local: 'kubernetes_CSI_00048.html'
                      },
                      {
                        id: 1839,
                        parentId: 1829,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'kubernetes_CSI_00049.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1801,
                parentId: 1798,
                name: '归档',
                local: 'kubernetes_CSI_00069.html',
                children: [
                  {
                    id: 1840,
                    parentId: 1801,
                    name: '归档Kubernetes CSI备份副本',
                    local: 'kubernetes_CSI_00072.html',
                    children: [
                      {
                        id: 1842,
                        parentId: 1840,
                        name: '步骤1：添加归档存储',
                        local: 'kubernetes_CSI_00073.html',
                        children: [
                          {
                            id: 1844,
                            parentId: 1842,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'kubernetes_CSI_00074.html'
                          },
                          {
                            id: 1845,
                            parentId: 1842,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'kubernetes_CSI_00075.html'
                          }
                        ]
                      },
                      {
                        id: 1843,
                        parentId: 1840,
                        name: '步骤12：创建备份副本归档SLA',
                        local: 'kubernetes_CSI_00076.html'
                      }
                    ]
                  },
                  {
                    id: 1841,
                    parentId: 1801,
                    name: '归档Kubernetes CSI复制副本',
                    local: 'kubernetes_CSI_00077.html',
                    children: [
                      {
                        id: 1846,
                        parentId: 1841,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'kubernetes_CSI_00078.html'
                      },
                      {
                        id: 1847,
                        parentId: 1841,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'kubernetes_CSI_00079.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1802,
                parentId: 1798,
                name: '恢复',
                local: 'kubernetes_CSI_00080.html',
                children: [
                  {
                    id: 1848,
                    parentId: 1802,
                    name: '恢复命名空间/数据集',
                    local: 'kubernetes_CSI_00083.html'
                  },
                  {
                    id: 1849,
                    parentId: 1802,
                    name: '恢复PVC',
                    local: 'kubernetes_CSI_00084.html'
                  }
                ]
              },
              {
                id: 1803,
                parentId: 1798,
                name: '全局搜索',
                local: 'kubernetes_CSI_00085.html',
                children: [
                  {
                    id: 1850,
                    parentId: 1803,
                    name: '全局搜索资源',
                    local: 'kubernetes_CSI_00086.html'
                  },
                  {
                    id: 1851,
                    parentId: 1803,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'kubernetes_CSI_00087.html'
                  }
                ]
              },
              {
                id: 1804,
                parentId: 1798,
                name: 'SLA',
                local: 'kubernetes_CSI_00091.html',
                children: [
                  {
                    id: 1852,
                    parentId: 1804,
                    name: '关于SLA',
                    local: 'kubernetes_CSI_00092.html'
                  },
                  {
                    id: 1853,
                    parentId: 1804,
                    name: '查看SLA信息',
                    local: 'kubernetes_CSI_00093.html'
                  },
                  {
                    id: 1854,
                    parentId: 1804,
                    name: '管理SLA',
                    local: 'kubernetes_CSI_00094.html'
                  }
                ]
              },
              {
                id: 1805,
                parentId: 1798,
                name: '副本',
                local: 'kubernetes_CSI_00095.html',
                children: [
                  {
                    id: 1855,
                    parentId: 1805,
                    name: '查看Kubernetes CSI副本信息',
                    local: 'kubernetes_CSI_00096.html'
                  },
                  {
                    id: 1856,
                    parentId: 1805,
                    name: '管理Kubernetes CSI副本',
                    local: 'kubernetes_CSI_00097.html'
                  }
                ]
              },
              {
                id: 1806,
                parentId: 1798,
                name: '集群/命名空间/数据集',
                local: 'kubernetes_CSI_00098.html',
                children: [
                  {
                    id: 1857,
                    parentId: 1806,
                    name: '查看信息',
                    local: 'kubernetes_CSI_00099.html'
                  },
                  {
                    id: 1858,
                    parentId: 1806,
                    name: '管理集群',
                    local: 'kubernetes_CSI_00100.html'
                  },
                  {
                    id: 1859,
                    parentId: 1806,
                    name: '管理命名空间/数据集',
                    local: 'kubernetes_CSI_00101.html'
                  }
                ]
              },
              {
                id: 1807,
                parentId: 1798,
                name: '常见问题',
                local: 'kubernetes_CSI_00102.html',
                children: [
                  {
                    id: 1860,
                    parentId: 1807,
                    name: '登录DeviceManager管理界面',
                    local: 'kubernetes_CSI_00104.html'
                  },
                  {
                    id: 1861,
                    parentId: 1807,
                    name: 'Token认证时获取证书值（适用于CCE）',
                    local: 'kubernetes_CSI_00107.html'
                  },
                  {
                    id: 1862,
                    parentId: 1807,
                    name: '应用一致性备份的生产环境Pod配置（通用）',
                    local: 'kubernetes_CSI_00108.html'
                  },
                  {
                    id: 1863,
                    parentId: 1807,
                    name: '应用一致性备份的生产环境Pod配置（容器应用为MySQL）',
                    local: 'kubernetes_CSI_00109.html'
                  },
                  {
                    id: 1864,
                    parentId: 1807,
                    name:
                      '应用一致性备份的生产环境Pod配置（容器应用为openGauss）',
                    local: 'kubernetes_CSI_00110.html'
                  },
                  {
                    id: 1865,
                    parentId: 1807,
                    name:
                      '应用一致性备份的生产环境Pod配置（容器应用为PostgreSQL）',
                    local: 'kubernetes_CSI_00110_a1.html'
                  },
                  {
                    id: 1866,
                    parentId: 1807,
                    name:
                      '应用一致性备份的生产环境Pod配置（容器应用为MairaDB）',
                    local: 'kubernetes_CSI_00110_a2.html'
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
        local: 'zh-cn_topic_0000002164607810.html',
        children: [
          {
            id: 1867,
            parentId: 17,
            name: '华为云Stack数据保护',
            local: 'zh-cn_topic_0000002200094089.html',
            children: [
              {
                id: 1870,
                parentId: 1867,
                name: '备份',
                local: 'HCS_Stack_gud_0008.html',
                children: [
                  {
                    id: 1878,
                    parentId: 1870,
                    name: '备份云硬盘',
                    local: 'HCS_Stack_gud_0012.html',
                    children: [
                      {
                        id: 1879,
                        parentId: 1878,
                        name: '步骤1：（可选）获取证书',
                        local: 'HCS_Stack_gud_0013.html'
                      },
                      {
                        id: 1880,
                        parentId: 1878,
                        name: '步骤2：注册华为云Stack',
                        local: 'HCS_Stack_gud_0015.html'
                      },
                      {
                        id: 1881,
                        parentId: 1878,
                        name: '步骤3：添加租户并授权资源',
                        local: 'HCS_Stack_gud_0016.html'
                      },
                      {
                        id: 1882,
                        parentId: 1878,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'HCS_Stack_gud_0017.html'
                      },
                      {
                        id: 1883,
                        parentId: 1878,
                        name: '步骤5：（可选）创建限速策略',
                        local: 'HCS_Stack_gud_0018.html'
                      },
                      {
                        id: 1884,
                        parentId: 1878,
                        name: '步骤6：创建备份SLA',
                        local: 'HCS_Stack_gud_0019.html'
                      },
                      {
                        id: 1885,
                        parentId: 1878,
                        name: '步骤7：执行备份',
                        local: 'HCS_Stack_gud_0020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1871,
                parentId: 1867,
                name: '复制',
                local: 'HCS_Stack_gud_0021.html',
                children: [
                  {
                    id: 1886,
                    parentId: 1871,
                    name: '复制华为云Stack副本',
                    local: 'HCS_Stack_gud_0024.html',
                    children: [
                      {
                        id: 1887,
                        parentId: 1886,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'HCS_Stack_gud_0025.html'
                      },
                      {
                        id: 1888,
                        parentId: 1886,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'HCS_Stack_gud_0026.html'
                      },
                      {
                        id: 1889,
                        parentId: 1886,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'HCS_Stack_gud_0027.html'
                      },
                      {
                        id: 1890,
                        parentId: 1886,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'HCS_Stack_gud_0028.html'
                      },
                      {
                        id: 1891,
                        parentId: 1886,
                        name: '步骤4：下载并导入证书',
                        local: 'HCS_Stack_gud_0029.html'
                      },
                      {
                        id: 1892,
                        parentId: 1886,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'HCS_Stack_gud_0030.html'
                      },
                      {
                        id: 1893,
                        parentId: 1886,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'HCS_Stack_gud_0031.html'
                      },
                      {
                        id: 1894,
                        parentId: 1886,
                        name: '步骤6：添加复制集群',
                        local: 'HCS_Stack_gud_0032.html'
                      },
                      {
                        id: 1895,
                        parentId: 1886,
                        name: '步骤7：创建复制SLA',
                        local: 'HCS_Stack_gud_0033.html'
                      },
                      {
                        id: 1896,
                        parentId: 1886,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'HCS_Stack_gud_0034.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1872,
                parentId: 1867,
                name: '归档',
                local: 'HCS_Stack_gud_0054.html',
                children: [
                  {
                    id: 1897,
                    parentId: 1872,
                    name: '归档华为云Stack备份副本',
                    local: 'HCS_Stack_gud_0057.html',
                    children: [
                      {
                        id: 1899,
                        parentId: 1897,
                        name: '步骤1：添加归档存储',
                        local: 'HCS_Stack_gud_0058.html',
                        children: [
                          {
                            id: 1901,
                            parentId: 1899,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'HCS_Stack_gud_0059.html'
                          },
                          {
                            id: 1902,
                            parentId: 1899,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'HCS_Stack_gud_0060.html'
                          }
                        ]
                      },
                      {
                        id: 1900,
                        parentId: 1897,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'HCS_Stack_gud_0061.html'
                      }
                    ]
                  },
                  {
                    id: 1898,
                    parentId: 1872,
                    name: '归档华为云Stack复制副本',
                    local: 'HCS_Stack_gud_0062.html',
                    children: [
                      {
                        id: 1903,
                        parentId: 1898,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'HCS_Stack_gud_0063.html'
                      },
                      {
                        id: 1904,
                        parentId: 1898,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'HCS_Stack_gud_0064.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1873,
                parentId: 1867,
                name: '恢复',
                local: 'HCS_Stack_gud_0065.html',
                children: [
                  {
                    id: 1905,
                    parentId: 1873,
                    name: '恢复云硬盘',
                    local: 'HCS_Stack_gud_0068.html'
                  },
                  {
                    id: 1906,
                    parentId: 1873,
                    name: '恢复云硬盘中的文件',
                    local: 'HCS_Stack_gud_0069.html'
                  }
                ]
              },
              {
                id: 1874,
                parentId: 1867,
                name: '全局搜索',
                local: 'HCS_Stack_gud_0070.html',
                children: [
                  {
                    id: 1907,
                    parentId: 1874,
                    name: '关于全局搜索',
                    local: 'HCS_Stack_gud_0071.html'
                  },
                  {
                    id: 1908,
                    parentId: 1874,
                    name: '全局搜索副本数据',
                    local: 'HCS_Stack_gud_0072.html'
                  },
                  {
                    id: 1909,
                    parentId: 1874,
                    name: '全局搜索资源',
                    local: 'HCS_Stack_gud_0073.html'
                  },
                  {
                    id: 1910,
                    parentId: 1874,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'HCS_Stack_gud_0074.html'
                  }
                ]
              },
              {
                id: 1875,
                parentId: 1867,
                name: 'SLA',
                local: 'HCS_Stack_gud_0077.html',
                children: [
                  {
                    id: 1911,
                    parentId: 1875,
                    name: '关于SLA',
                    local: 'HCS_Stack_gud_0078.html'
                  },
                  {
                    id: 1912,
                    parentId: 1875,
                    name: '查看SLA信息',
                    local: 'HCS_Stack_gud_0079.html'
                  },
                  {
                    id: 1913,
                    parentId: 1875,
                    name: '管理SLA',
                    local: 'HCS_Stack_gud_0080.html'
                  }
                ]
              },
              {
                id: 1876,
                parentId: 1867,
                name: '副本',
                local: 'HCS_Stack_gud_0081.html',
                children: [
                  {
                    id: 1914,
                    parentId: 1876,
                    name: '查看华为云Stack副本信息',
                    local: 'HCS_Stack_gud_0082.html'
                  },
                  {
                    id: 1915,
                    parentId: 1876,
                    name: '管理华为云Stack副本',
                    local: 'HCS_Stack_gud_0083.html'
                  }
                ]
              },
              {
                id: 1877,
                parentId: 1867,
                name: '华为云Stack环境',
                local: 'HCS_Stack_gud_0084.html',
                children: [
                  {
                    id: 1916,
                    parentId: 1877,
                    name: '查看华为云Stack信息',
                    local: 'HCS_Stack_gud_0085.html'
                  },
                  {
                    id: 1917,
                    parentId: 1877,
                    name: '管理华为云Stack注册信息',
                    local: 'HCS_Stack_gud_0086.html'
                  },
                  {
                    id: 1918,
                    parentId: 1877,
                    name: '管理租户',
                    local: 'HCS_Stack_gud_0087.html'
                  },
                  {
                    id: 1919,
                    parentId: 1877,
                    name: '管理项目/资源集或弹性云服务器',
                    local: 'HCS_Stack_gud_0088.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1868,
            parentId: 17,
            name: '华为云Stack GaussDB数据保护',
            local: 'zh-cn_topic_0000002164767526.html',
            children: [
              {
                id: 1920,
                parentId: 1868,
                name: '备份',
                local: 'hcs_gaussdb_00006.html',
                children: [
                  {
                    id: 1929,
                    parentId: 1920,
                    name: '备份前准备',
                    local: 'hcs_gaussdb_00009.html'
                  },
                  {
                    id: 1930,
                    parentId: 1920,
                    name: '备份华为云Stack GaussDB实例',
                    local: 'hcs_gaussdb_00010.html',
                    children: [
                      {
                        id: 1931,
                        parentId: 1930,
                        name: '步骤1：注册华为云Stack GaussDB项目',
                        local: 'hcs_gaussdb_00011.html'
                      },
                      {
                        id: 1932,
                        parentId: 1930,
                        name: '步骤2：（可选）开启备份链路加密开关',
                        local: 'hcs_gaussdb_00012.html'
                      },
                      {
                        id: 1933,
                        parentId: 1930,
                        name: '步骤3：（可选）创建限速策略',
                        local: 'hcs_gaussdb_00013.html'
                      },
                      {
                        id: 1934,
                        parentId: 1930,
                        name: '步骤4：创建备份SLA',
                        local: 'hcs_gaussdb_00014.html'
                      },
                      {
                        id: 1935,
                        parentId: 1930,
                        name: '步骤5：执行备份',
                        local: 'hcs_gaussdb_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1921,
                parentId: 1868,
                name: '复制',
                local: 'hcs_gaussdb_00018.html',
                children: [
                  {
                    id: 1936,
                    parentId: 1921,
                    name: '复制华为云Stack GaussDB副本',
                    local: 'hcs_gaussdb_000184.html',
                    children: [
                      {
                        id: 1937,
                        parentId: 1936,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'hcs_gaussdb_000185.html'
                      },
                      {
                        id: 1938,
                        parentId: 1936,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'hcs_gaussdb_000186.html'
                      },
                      {
                        id: 1939,
                        parentId: 1936,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'hcs_gaussdb_000187.html'
                      },
                      {
                        id: 1940,
                        parentId: 1936,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'hcs_gaussdb_000188.html'
                      },
                      {
                        id: 1941,
                        parentId: 1936,
                        name: '步骤4：下载并导入证书',
                        local: 'hcs_gaussdb_000189.html'
                      },
                      {
                        id: 1942,
                        parentId: 1936,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'hcs_gaussdb_000190.html'
                      },
                      {
                        id: 1943,
                        parentId: 1936,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'hcs_gaussdb_000191.html'
                      },
                      {
                        id: 1944,
                        parentId: 1936,
                        name: '步骤6：添加复制集群',
                        local: 'hcs_gaussdb_000192.html'
                      },
                      {
                        id: 1945,
                        parentId: 1936,
                        name: '步骤7：创建复制SLA',
                        local: 'hcs_gaussdb_000193.html'
                      },
                      {
                        id: 1946,
                        parentId: 1936,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'hcs_gaussdb_000194.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1922,
                parentId: 1868,
                name: '归档',
                local: 'hcs_gaussdb_00031.html',
                children: [
                  {
                    id: 1947,
                    parentId: 1922,
                    name: '归档华为云Stack GaussDB备份副本',
                    local: 'hcs_gaussdb_00034.html',
                    children: [
                      {
                        id: 1949,
                        parentId: 1947,
                        name: '步骤1：添加归档存储',
                        local: 'hcs_gaussdb_00035.html',
                        children: [
                          {
                            id: 1951,
                            parentId: 1949,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'hcs_gaussdb_00036.html'
                          },
                          {
                            id: 1952,
                            parentId: 1949,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'hcs_gaussdb_00037.html'
                          }
                        ]
                      },
                      {
                        id: 1950,
                        parentId: 1947,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'hcs_gaussdb_00038.html'
                      }
                    ]
                  },
                  {
                    id: 1948,
                    parentId: 1922,
                    name: '归档华为云Stack GaussDB复制副本',
                    local: 'hcs_gaussdb_00039.html',
                    children: [
                      {
                        id: 1953,
                        parentId: 1948,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'hcs_gaussdb_00040.html'
                      },
                      {
                        id: 1954,
                        parentId: 1948,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'hcs_gaussdb_00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1923,
                parentId: 1868,
                name: '恢复',
                local: 'hcs_gaussdb_00042.html',
                children: [
                  {
                    id: 1955,
                    parentId: 1923,
                    name: '恢复华为云Stack GaussDB实例',
                    local: 'hcs_gaussdb_00045.html'
                  }
                ]
              },
              {
                id: 1924,
                parentId: 1868,
                name: '全局搜索',
                local: 'hcs_gaussdb_0004211.html',
                children: [
                  {
                    id: 1956,
                    parentId: 1924,
                    name: '全局搜索资源',
                    local: 'hcs_gaussdb_00046.html'
                  },
                  {
                    id: 1957,
                    parentId: 1924,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'hcs_gaussdb_0004222.html'
                  }
                ]
              },
              {
                id: 1925,
                parentId: 1868,
                name: 'SLA',
                local: 'hcs_gaussdb_00049.html',
                children: [
                  {
                    id: 1958,
                    parentId: 1925,
                    name: '关于SLA',
                    local: 'hcs_gaussdb_000491.html'
                  },
                  {
                    id: 1959,
                    parentId: 1925,
                    name: '查看SLA信息',
                    local: 'hcs_gaussdb_00051.html'
                  },
                  {
                    id: 1960,
                    parentId: 1925,
                    name: '管理SLA',
                    local: 'hcs_gaussdb_00052.html'
                  }
                ]
              },
              {
                id: 1926,
                parentId: 1868,
                name: '副本',
                local: 'hcs_gaussdb_00053.html',
                children: [
                  {
                    id: 1961,
                    parentId: 1926,
                    name: '查看华为云Stack GaussDB副本信息',
                    local: 'hcs_gaussdb_00054.html'
                  },
                  {
                    id: 1962,
                    parentId: 1926,
                    name: '管理华为云Stack GaussDB副本',
                    local: 'hcs_gaussdb_00055.html'
                  }
                ]
              },
              {
                id: 1927,
                parentId: 1868,
                name: '华为云Stack GaussDB',
                local: 'hcs_gaussdb_00056.html',
                children: [
                  {
                    id: 1963,
                    parentId: 1927,
                    name: '查看华为云Stack GaussDB信息',
                    local: 'hcs_gaussdb_00057.html'
                  },
                  {
                    id: 1964,
                    parentId: 1927,
                    name: '管理华为云Stack GaussDB项目',
                    local: 'hcs_gaussdb_00058.html'
                  },
                  {
                    id: 1965,
                    parentId: 1927,
                    name: '管理实例',
                    local: 'hcs_gaussdb_00059.html'
                  }
                ]
              },
              {
                id: 1928,
                parentId: 1868,
                name: '常见问题',
                local: 'hcs_gaussdb_00060.html',
                children: [
                  {
                    id: 1966,
                    parentId: 1928,
                    name: '单节点部署架构不支持日志备份',
                    local: 'zh-cn_topic_0000002200124361.html'
                  },
                  {
                    id: 1967,
                    parentId: 1928,
                    name: '查看GaussDB实例版本',
                    local: 'zh-cn_topic_0000002223487201.html'
                  },
                  {
                    id: 1968,
                    parentId: 1928,
                    name: '可恢复时间段内日志不连续',
                    local: 'zh-cn_topic_0000002189332534.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1869,
            parentId: 17,
            name: '阿里云数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002200008529.html',
            children: [
              {
                id: 1969,
                parentId: 1869,
                name: '备份',
                local: 'acloud_00008.html',
                children: [
                  {
                    id: 1978,
                    parentId: 1969,
                    name: '备份前准备',
                    local: 'acloud_00011.html'
                  },
                  {
                    id: 1979,
                    parentId: 1969,
                    name: '备份阿里云云服务器',
                    local: 'acloud_00012.html',
                    children: [
                      {
                        id: 1980,
                        parentId: 1979,
                        name: '步骤1：获取AccessKey ID',
                        local: 'acloud_00015.html'
                      },
                      {
                        id: 1981,
                        parentId: 1979,
                        name: '步骤2：注册阿里云组织',
                        local: 'acloud_00017.html'
                      },
                      {
                        id: 1982,
                        parentId: 1979,
                        name: '步骤3：（可选）创建限速策略',
                        local: 'acloud_00018.html'
                      },
                      {
                        id: 1983,
                        parentId: 1979,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'acloud_00019.html'
                      },
                      {
                        id: 1984,
                        parentId: 1979,
                        name: '步骤5：创建备份SLA',
                        local: 'acloud_00020.html'
                      },
                      {
                        id: 1985,
                        parentId: 1979,
                        name: '步骤6：执行备份',
                        local: 'acloud_00021.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1970,
                parentId: 1869,
                name: '复制',
                local: 'acloud_00024.html',
                children: [
                  {
                    id: 1986,
                    parentId: 1970,
                    name: '复制阿里云副本',
                    local: 'acloud_00028.html',
                    children: [
                      {
                        id: 1987,
                        parentId: 1986,
                        name: '步骤1：创建复制网络逻辑端口',
                        local: 'acloud_00029.html'
                      },
                      {
                        id: 1988,
                        parentId: 1986,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'acloud_00030.html'
                      },
                      {
                        id: 1989,
                        parentId: 1986,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'acloud_00031.html'
                      },
                      {
                        id: 1990,
                        parentId: 1986,
                        name: '步骤4：下载并导入证书',
                        local: 'acloud_00032.html'
                      },
                      {
                        id: 1991,
                        parentId: 1986,
                        name: '步骤5：创建远端设备管理员',
                        local: 'acloud_00033.html'
                      },
                      {
                        id: 1992,
                        parentId: 1986,
                        name: '步骤6：添加复制集群',
                        local: 'acloud_00034.html'
                      },
                      {
                        id: 1993,
                        parentId: 1986,
                        name: '步骤7：创建复制SLA',
                        local: 'acloud_00035.html'
                      },
                      {
                        id: 1994,
                        parentId: 1986,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'acloud_00036.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1971,
                parentId: 1869,
                name: '归档',
                local: 'acloud_00056.html',
                children: [
                  {
                    id: 1995,
                    parentId: 1971,
                    name: '归档阿里云备份副本',
                    local: 'acloud_00059.html',
                    children: [
                      {
                        id: 1997,
                        parentId: 1995,
                        name: '步骤1：添加归档存储',
                        local: 'acloud_00060.html',
                        children: [
                          {
                            id: 1999,
                            parentId: 1997,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'acloud_00061.html'
                          },
                          {
                            id: 2000,
                            parentId: 1997,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'acloud_00062.html'
                          }
                        ]
                      },
                      {
                        id: 1998,
                        parentId: 1995,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'acloud_00063.html'
                      }
                    ]
                  },
                  {
                    id: 1996,
                    parentId: 1971,
                    name: '归档阿里云复制副本',
                    local: 'acloud_00064.html',
                    children: [
                      {
                        id: 2001,
                        parentId: 1996,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'acloud_00065.html'
                      },
                      {
                        id: 2002,
                        parentId: 1996,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'acloud_00066.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1972,
                parentId: 1869,
                name: '恢复',
                local: 'acloud_00067.html',
                children: [
                  {
                    id: 2003,
                    parentId: 1972,
                    name: '恢复云服务器',
                    local: 'acloud_00070.html'
                  },
                  {
                    id: 2004,
                    parentId: 1972,
                    name: '恢复云磁盘',
                    local: 'acloud_00071.html'
                  }
                ]
              },
              {
                id: 1973,
                parentId: 1869,
                name: '全局搜索',
                local: 'acloud_00073.html',
                children: [
                  {
                    id: 2005,
                    parentId: 1973,
                    name: '关于全局搜索',
                    local: 'acloud_00074.html'
                  },
                  {
                    id: 2006,
                    parentId: 1973,
                    name: '全局搜索副本数据',
                    local: 'acloud_00075.html'
                  },
                  {
                    id: 2007,
                    parentId: 1973,
                    name: '全局搜索资源',
                    local: 'acloud_00076.html'
                  },
                  {
                    id: 2008,
                    parentId: 1973,
                    name: '全局标签搜索',
                    local: 'acloud_00077.html'
                  }
                ]
              },
              {
                id: 1974,
                parentId: 1869,
                name: 'SLA',
                local: 'acloud_00080.html',
                children: [
                  {
                    id: 2009,
                    parentId: 1974,
                    name: '关于SLA',
                    local: 'acloud_00081.html'
                  },
                  {
                    id: 2010,
                    parentId: 1974,
                    name: '查看SLA信息',
                    local: 'acloud_00082.html'
                  },
                  {
                    id: 2011,
                    parentId: 1974,
                    name: '管理SLA',
                    local: 'acloud_00083.html'
                  }
                ]
              },
              {
                id: 1975,
                parentId: 1869,
                name: '副本',
                local: 'acloud_00084.html',
                children: [
                  {
                    id: 2012,
                    parentId: 1975,
                    name: '查看阿里云副本信息',
                    local: 'acloud_00085.html'
                  },
                  {
                    id: 2013,
                    parentId: 1975,
                    name: '管理阿里云副本',
                    local: 'acloud_00086.html'
                  }
                ]
              },
              {
                id: 1976,
                parentId: 1869,
                name: '阿里云环境信息',
                local: 'acloud_00087.html',
                children: [
                  {
                    id: 2014,
                    parentId: 1976,
                    name: '查看阿里云资源信息',
                    local: 'acloud_00088.html'
                  },
                  {
                    id: 2015,
                    parentId: 1976,
                    name: '管理阿里云平台',
                    local: 'acloud_00089.html'
                  },
                  {
                    id: 2016,
                    parentId: 1976,
                    name: '管理可用区/资源集/云服务器',
                    local: 'acloud_00090.html'
                  }
                ]
              },
              {
                id: 1977,
                parentId: 1869,
                name: '常见问题',
                local: 'acloud_00091.html'
              }
            ]
          }
        ]
      },
      {
        id: 18,
        parentId: 3,
        name: '应用',
        local: 'zh-cn_topic_0000002200094085.html',
        children: [
          {
            id: 2017,
            parentId: 18,
            name: 'Exchange数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002164767486.html',
            children: [
              {
                id: 2019,
                parentId: 2017,
                name: '备份',
                local: 'exchange_0010.html',
                children: [
                  {
                    id: 2028,
                    parentId: 2019,
                    name: '备份前准备',
                    local: 'exchange_0015.html',
                    children: [
                      {
                        id: 2031,
                        parentId: 2028,
                        name: '开启Exchange信息存储服务',
                        local: 'exchange_0016.html'
                      },
                      {
                        id: 2032,
                        parentId: 2028,
                        name: '检查Exchange Writer状态',
                        local: 'exchange_0017.html'
                      },
                      {
                        id: 2033,
                        parentId: 2028,
                        name: '检查Exchange数据库状态',
                        local: 'exchange_0018.html'
                      },
                      {
                        id: 2034,
                        parentId: 2028,
                        name: '配置数据库备份与恢复账户',
                        local: 'exchange_0019.html'
                      },
                      {
                        id: 2035,
                        parentId: 2028,
                        name: '配置邮箱备份与恢复账户',
                        local: 'exchange_0020.html'
                      }
                    ]
                  },
                  {
                    id: 2029,
                    parentId: 2019,
                    name: '备份Exchange单机/可用性组或数据库',
                    local: 'exchange_0021.html',
                    children: [
                      {
                        id: 2036,
                        parentId: 2029,
                        name: '步骤1：注册Exchange单机/可用性组',
                        local: 'exchange_0022.html'
                      },
                      {
                        id: 2037,
                        parentId: 2029,
                        name: '步骤2：（可选）创建限速策略',
                        local: 'exchange_0023.html'
                      },
                      {
                        id: 2038,
                        parentId: 2029,
                        name: '步骤3：创建备份SLA',
                        local: 'exchange_0024.html'
                      },
                      {
                        id: 2039,
                        parentId: 2029,
                        name: '步骤4：执行备份',
                        local: 'exchange_0025.html'
                      }
                    ]
                  },
                  {
                    id: 2030,
                    parentId: 2019,
                    name: '备份Exchange邮箱',
                    local: 'exchange_0028.html',
                    children: [
                      {
                        id: 2040,
                        parentId: 2030,
                        name: '步骤1：注册Exchange单机/可用性组',
                        local: 'exchange_0029.html'
                      },
                      {
                        id: 2041,
                        parentId: 2030,
                        name: '步骤2：（可选）创建限速策略',
                        local: 'exchange_0030.html'
                      },
                      {
                        id: 2042,
                        parentId: 2030,
                        name: '步骤3：创建备份SLA',
                        local: 'exchange_0031.html'
                      },
                      {
                        id: 2043,
                        parentId: 2030,
                        name: '步骤4：执行备份',
                        local: 'exchange_0032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2020,
                parentId: 2017,
                name: '复制',
                local: 'exchange_0035.html',
                children: [
                  {
                    id: 2044,
                    parentId: 2020,
                    name: '复制Exchange副本',
                    local: 'exchange_0038.html',
                    children: [
                      {
                        id: 2045,
                        parentId: 2044,
                        name: '步骤1：创建复制网络逻辑端口',
                        local: 'exchange_0039.html'
                      },
                      {
                        id: 2046,
                        parentId: 2044,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'exchange_0040.html'
                      },
                      {
                        id: 2047,
                        parentId: 2044,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'exchange_0041.html'
                      },
                      {
                        id: 2048,
                        parentId: 2044,
                        name: '步骤4：下载并导入证书',
                        local: 'exchange_0042.html'
                      },
                      {
                        id: 2049,
                        parentId: 2044,
                        name: '步骤5：创建远端设备管理员',
                        local: 'exchange_0043.html'
                      },
                      {
                        id: 2050,
                        parentId: 2044,
                        name: '步骤6：添加复制集群',
                        local: 'exchange_0044.html'
                      },
                      {
                        id: 2051,
                        parentId: 2044,
                        name: '步骤7：创建复制SLA',
                        local: 'exchange_0045.html'
                      },
                      {
                        id: 2052,
                        parentId: 2044,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'exchange_0046.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2021,
                parentId: 2017,
                name: '归档',
                local: 'exchange_0066.html',
                children: [
                  {
                    id: 2053,
                    parentId: 2021,
                    name: '归档Exchange备份副本',
                    local: 'exchange_0069.html',
                    children: [
                      {
                        id: 2055,
                        parentId: 2053,
                        name: '步骤1：添加归档存储',
                        local: 'exchange_0070.html',
                        children: [
                          {
                            id: 2057,
                            parentId: 2055,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'exchange_0071.html'
                          },
                          {
                            id: 2058,
                            parentId: 2055,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'exchange_0072.html'
                          }
                        ]
                      },
                      {
                        id: 2056,
                        parentId: 2053,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'exchange_0073.html'
                      }
                    ]
                  },
                  {
                    id: 2054,
                    parentId: 2021,
                    name: '归档Exchange复制副本',
                    local: 'exchange_0074.html',
                    children: [
                      {
                        id: 2059,
                        parentId: 2054,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'exchange_0075.html'
                      },
                      {
                        id: 2060,
                        parentId: 2054,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'exchange_0076.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2022,
                parentId: 2017,
                name: '恢复',
                local: 'exchange_0077.html',
                children: [
                  {
                    id: 2061,
                    parentId: 2022,
                    name: '恢复单机/可用性组',
                    local: 'exchange_0083.html'
                  },
                  {
                    id: 2062,
                    parentId: 2022,
                    name: '恢复Exchange数据库',
                    local: 'exchange_0084.html'
                  },
                  {
                    id: 2063,
                    parentId: 2022,
                    name: '恢复邮箱',
                    local: 'exchange_0085.html'
                  },
                  {
                    id: 2064,
                    parentId: 2022,
                    name: '邮件级恢复',
                    local: 'exchange_0086.html'
                  },
                  {
                    id: 2065,
                    parentId: 2022,
                    name:
                      '验证恢复后的用户邮箱数据（适用于Microsoft Exchange Server 2010）',
                    local: 'exchange_0087.html'
                  },
                  {
                    id: 2066,
                    parentId: 2022,
                    name:
                      '验证恢复后的用户邮箱数据（适用于Microsoft Exchange Server 2013及后续版本）',
                    local: 'exchange_0088.html'
                  }
                ]
              },
              {
                id: 2023,
                parentId: 2017,
                name: '全局搜索',
                local: 'exchange_0089.html',
                children: [
                  {
                    id: 2067,
                    parentId: 2023,
                    name: '全局搜索资源',
                    local: 'exchange_0090.html'
                  },
                  {
                    id: 2068,
                    parentId: 2023,
                    name: '全局标签搜索',
                    local: 'exchange_0091.html'
                  }
                ]
              },
              {
                id: 2024,
                parentId: 2017,
                name: 'SLA',
                local: 'exchange_0094.html',
                children: [
                  {
                    id: 2069,
                    parentId: 2024,
                    name: '关于SLA',
                    local: 'exchange_0095.html'
                  },
                  {
                    id: 2070,
                    parentId: 2024,
                    name: '查看SLA信息',
                    local: 'exchange_0096.html'
                  },
                  {
                    id: 2071,
                    parentId: 2024,
                    name: '管理SLA',
                    local: 'exchange_0097.html'
                  }
                ]
              },
              {
                id: 2025,
                parentId: 2017,
                name: '副本',
                local: 'exchange_0098.html',
                children: [
                  {
                    id: 2072,
                    parentId: 2025,
                    name: '查看Exchange副本信息',
                    local: 'exchange_0099.html'
                  },
                  {
                    id: 2073,
                    parentId: 2025,
                    name: '管理Exchange副本',
                    local: 'exchange_0100.html'
                  }
                ]
              },
              {
                id: 2026,
                parentId: 2017,
                name: '管理Exchange',
                local: 'exchange_0101.html',
                children: [
                  {
                    id: 2074,
                    parentId: 2026,
                    name: '查看Exchange环境信息',
                    local: 'exchange_0102.html'
                  },
                  {
                    id: 2075,
                    parentId: 2026,
                    name: '管理Exchange单机或可用性组',
                    local: 'exchange_0103.html'
                  },
                  {
                    id: 2076,
                    parentId: 2026,
                    name: '管理数据库或邮箱',
                    local: 'exchange_0104.html'
                  }
                ]
              },
              {
                id: 2027,
                parentId: 2017,
                name: '常见问题',
                local: 'exchange_0105.html'
              }
            ]
          },
          {
            id: 2018,
            parentId: 18,
            name: 'Active Directory数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002200094097.html',
            children: [
              {
                id: 2077,
                parentId: 2018,
                name: '备份',
                local: 'ActiveDirectory-00008.html',
                children: [
                  {
                    id: 2086,
                    parentId: 2077,
                    name: '备份Active Directory',
                    local: 'ActiveDirectory-00011.html',
                    children: [
                      {
                        id: 2087,
                        parentId: 2086,
                        name: '步骤1：开启Active Directory回收站',
                        local: 'ActiveDirectory-00012.html'
                      },
                      {
                        id: 2088,
                        parentId: 2086,
                        name: '步骤2：注册Active Directory域控制器',
                        local: 'ActiveDirectory-00013.html'
                      },
                      {
                        id: 2089,
                        parentId: 2086,
                        name: '步骤3：（可选）创建限速策略',
                        local: 'ActiveDirectory-00014.html'
                      },
                      {
                        id: 2090,
                        parentId: 2086,
                        name: '步骤4：创建备份SLA',
                        local: 'ActiveDirectory-00015.html'
                      },
                      {
                        id: 2091,
                        parentId: 2086,
                        name: '步骤5：执行备份',
                        local: 'ActiveDirectory-00016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2078,
                parentId: 2018,
                name: '复制',
                local: 'ActiveDirectory-00019.html',
                children: [
                  {
                    id: 2092,
                    parentId: 2078,
                    name: '复制Active Directory副本',
                    local: 'ActiveDirectory-00023.html',
                    children: [
                      {
                        id: 2093,
                        parentId: 2092,
                        name: '步骤1：创建复制网络逻辑端口',
                        local: 'ActiveDirectory-00024.html'
                      },
                      {
                        id: 2094,
                        parentId: 2092,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'ActiveDirectory-00025.html'
                      },
                      {
                        id: 2095,
                        parentId: 2092,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'ActiveDirectory-00026.html'
                      },
                      {
                        id: 2096,
                        parentId: 2092,
                        name: '步骤4：下载并导入证书',
                        local: 'ActiveDirectory-00027.html'
                      },
                      {
                        id: 2097,
                        parentId: 2092,
                        name: '步骤5：创建远端设备管理员',
                        local: 'ActiveDirectory-00028.html'
                      },
                      {
                        id: 2098,
                        parentId: 2092,
                        name: '步骤6：添加复制集群',
                        local: 'ActiveDirectory-00029.html'
                      },
                      {
                        id: 2099,
                        parentId: 2092,
                        name: '步骤7：创建复制SLA',
                        local: 'ActiveDirectory-00030.html'
                      },
                      {
                        id: 2100,
                        parentId: 2092,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'ActiveDirectory-00031.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2079,
                parentId: 2018,
                name: '归档',
                local: 'ActiveDirectory-00051.html',
                children: [
                  {
                    id: 2101,
                    parentId: 2079,
                    name: '归档Active Directory备份副本',
                    local: 'ActiveDirectory-00054.html',
                    children: [
                      {
                        id: 2102,
                        parentId: 2101,
                        name: '步骤1：添加归档存储',
                        local: 'ActiveDirectory-00055.html',
                        children: [
                          {
                            id: 2104,
                            parentId: 2102,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'ActiveDirectory-00056.html'
                          },
                          {
                            id: 2105,
                            parentId: 2102,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'ActiveDirectory-00057.html'
                          }
                        ]
                      },
                      {
                        id: 2103,
                        parentId: 2101,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'ActiveDirectory-00058.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2080,
                parentId: 2018,
                name: '恢复',
                local: 'ActiveDirectory-00062.html',
                children: [
                  {
                    id: 2106,
                    parentId: 2080,
                    name: '单域控制器场景恢复Active Directory的系统状态',
                    local: 'ActiveDirectory-00065.html'
                  },
                  {
                    id: 2107,
                    parentId: 2080,
                    name: '单域控制器场景恢复Active Directory的对象',
                    local: 'ActiveDirectory-00066.html'
                  },
                  {
                    id: 2108,
                    parentId: 2080,
                    name: '主备域控制器场景恢复Active Directory的系统状态',
                    local: 'ActiveDirectory-00067.html'
                  },
                  {
                    id: 2109,
                    parentId: 2080,
                    name: '主备域控制器场景恢复Active Directory的对象',
                    local: 'ActiveDirectory-00068.html'
                  }
                ]
              },
              {
                id: 2081,
                parentId: 2018,
                name: '全局搜索',
                local: 'ActiveDirectory-00069.html',
                children: [
                  {
                    id: 2110,
                    parentId: 2081,
                    name: '全局搜索资源',
                    local: 'ActiveDirectory-00070.html'
                  },
                  {
                    id: 2111,
                    parentId: 2081,
                    name: '全局标签搜索',
                    local: 'ActiveDirectory-00071.html'
                  }
                ]
              },
              {
                id: 2082,
                parentId: 2018,
                name: 'SLA',
                local: 'ActiveDirectory-00074.html',
                children: [
                  {
                    id: 2112,
                    parentId: 2082,
                    name: '关于SLA',
                    local: 'ActiveDirectory-00075.html'
                  },
                  {
                    id: 2113,
                    parentId: 2082,
                    name: '查看SLA信息',
                    local: 'ActiveDirectory-00076.html'
                  },
                  {
                    id: 2114,
                    parentId: 2082,
                    name: '管理SLA',
                    local: 'ActiveDirectory-00077.html'
                  }
                ]
              },
              {
                id: 2083,
                parentId: 2018,
                name: '副本',
                local: 'ActiveDirectory-00078.html',
                children: [
                  {
                    id: 2115,
                    parentId: 2083,
                    name: '查看Active Directory副本信息',
                    local: 'ActiveDirectory-00079.html'
                  },
                  {
                    id: 2116,
                    parentId: 2083,
                    name: '管理Active Directory副本',
                    local: 'ActiveDirectory-00080.html'
                  }
                ]
              },
              {
                id: 2084,
                parentId: 2018,
                name: 'Active Directory域控制器',
                local: 'ActiveDirectory-00081.html',
                children: [
                  {
                    id: 2117,
                    parentId: 2084,
                    name: '查看Active Directory域控制器信息',
                    local: 'ActiveDirectory-00082.html'
                  },
                  {
                    id: 2118,
                    parentId: 2084,
                    name: '管理Active Directory域控制器',
                    local: 'ActiveDirectory-00083.html'
                  }
                ]
              },
              {
                id: 2085,
                parentId: 2018,
                name: '常见问题',
                local: 'ActiveDirectory-00084.html',
                children: [
                  {
                    id: 2119,
                    parentId: 2085,
                    name: '设置Windows服务器进入目录服务修复模式（GUI）',
                    local: 'ActiveDirectory-00089.html'
                  },
                  {
                    id: 2120,
                    parentId: 2085,
                    name: '设置Windows服务器进入目录服务修复模式（命令行）',
                    local: 'ActiveDirectory-00090.html'
                  },
                  {
                    id: 2121,
                    parentId: 2085,
                    name: '设置Windows服务器退出目录服务修复模式',
                    local: 'ActiveDirectory-00091.html'
                  },
                  {
                    id: 2122,
                    parentId: 2085,
                    name: '安装Windows Server Backup',
                    local: 'ActiveDirectory-00092.html'
                  },
                  {
                    id: 2123,
                    parentId: 2085,
                    name:
                      'Active Directory和Exchange单机合并部署场景下恢复用户邮箱数据',
                    local: 'ActiveDirectory-00093.html'
                  },
                  {
                    id: 2124,
                    parentId: 2085,
                    name:
                      'Active Directory对象级恢复computer后，域账号登录失败',
                    local: 'ActiveDirectory-00094.html'
                  },
                  {
                    id: 2125,
                    parentId: 2085,
                    name: 'Active Directory执行副本恢复后启动DataTurbo服务失败',
                    local: 'ActiveDirectory-00095.html'
                  },
                  {
                    id: 2126,
                    parentId: 2085,
                    name:
                      'Active Directory回收站未开启场景下，对象属性恢复失败',
                    local: 'ActiveDirectory-00096.html'
                  },
                  {
                    id: 2127,
                    parentId: 2085,
                    name:
                      'Active Directory进行对象级恢复时报错：该对象的DistinguishedName已存在',
                    local: 'ActiveDirectory-00097.html'
                  },
                  {
                    id: 2128,
                    parentId: 2085,
                    name:
                      '找不到系统编写器导致备份任务失败，错误详情包含“8078001D”错误码',
                    local: 'ActiveDirectory-00099.html'
                  },
                  {
                    id: 2129,
                    parentId: 2085,
                    name: '主域控制器在系统状态恢复后无法创建新用户',
                    local: 'ActiveDirectory-00100.html'
                  },
                  {
                    id: 2130,
                    parentId: 2085,
                    name:
                      '主备域控制器场景恢复Active Directory的系统状态时，遇到同名对象（删除后重新创建的同名对象），新建的同名对象名称会加上乱码',
                    local: 'zh-cn_topic_0000002230518277.html'
                  },
                  {
                    id: 2131,
                    parentId: 2085,
                    name: '查询当前域的域控制器个数',
                    local: 'zh-cn_topic_0000002240475737.html'
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
        local: 'zh-cn_topic_0000002164767498.html',
        children: [
          {
            id: 2132,
            parentId: 19,
            name: 'NAS文件数据保护',
            local: 'product_documentation_000035.html',
            children: [
              {
                id: 2138,
                parentId: 2132,
                name: '备份',
                local: 'nas_s_0007.html',
                children: [
                  {
                    id: 2150,
                    parentId: 2138,
                    name: '备份前准备',
                    local: 'nas_s_0010.html'
                  },
                  {
                    id: 2151,
                    parentId: 2138,
                    name: '备份NAS文件系统',
                    local: 'nas_s_0011.html',
                    children: [
                      {
                        id: 2153,
                        parentId: 2151,
                        name: '步骤1：（可选）获取存储设备CA证书',
                        local: 'nas_s_0012.html'
                      },
                      {
                        id: 2154,
                        parentId: 2151,
                        name: '步骤2：添加存储设备',
                        local: 'nas_s_0013.html'
                      },
                      {
                        id: 2155,
                        parentId: 2151,
                        name: '步骤3：创建复制网络逻辑端口',
                        local: 'nas_s_0014.html'
                      },
                      {
                        id: 2156,
                        parentId: 2151,
                        name: '步骤3：创建复制网络逻辑端口',
                        local: 'fc_gud_0026_1_2.html'
                      },
                      {
                        id: 2157,
                        parentId: 2151,
                        name: '步骤4：（可选）创建限速策略',
                        local: 'nas_s_0015.html'
                      },
                      {
                        id: 2158,
                        parentId: 2151,
                        name: '步骤5：创建备份SLA',
                        local: 'nas_s_0016.html'
                      },
                      {
                        id: 2159,
                        parentId: 2151,
                        name: '步骤6：执行备份',
                        local: 'nas_s_0017.html'
                      }
                    ]
                  },
                  {
                    id: 2152,
                    parentId: 2138,
                    name: '备份NAS共享',
                    local: 'nas_s_0020.html',
                    children: [
                      {
                        id: 2160,
                        parentId: 2152,
                        name:
                          '步骤1：（可选）获取存储设备CA证书（适用于OceanStor V5/OceanStor Pacific/NetApp ONTAP）',
                        local: 'nas_s_0021.html'
                      },
                      {
                        id: 2161,
                        parentId: 2152,
                        name:
                          '步骤2：（可选）添加存储设备（适用于OceanStor V5/OceanStor Pacific/NetApp ONTAP）',
                        local: 'nas_s_0022.html'
                      },
                      {
                        id: 2162,
                        parentId: 2152,
                        name:
                          '（可选）步骤3：配置NAS共享信息（适用于OceanStor V5/OceanStor Pacific/NetApp ONTAP）',
                        local: 'nas_s_0023.html'
                      },
                      {
                        id: 2163,
                        parentId: 2152,
                        name:
                          '步骤4：配置访问权限（适用于OceanStor V5/OceanStor Pacific/NetApp ONTAP）',
                        local: 'nas_s_0025.html'
                      },
                      {
                        id: 2164,
                        parentId: 2152,
                        name: '（可选）步骤5：注册NAS共享',
                        local: 'nas_s_0024.html'
                      },
                      {
                        id: 2165,
                        parentId: 2152,
                        name: '步骤6：（可选）创建限速策略',
                        local: 'nas_s_0026.html'
                      },
                      {
                        id: 2166,
                        parentId: 2152,
                        name: '步骤7：创建备份SLA',
                        local: 'nas_s_0027.html'
                      },
                      {
                        id: 2167,
                        parentId: 2152,
                        name: '步骤8：开启NFSv4.1服务',
                        local: 'nas_s_0028.html'
                      },
                      {
                        id: 2168,
                        parentId: 2152,
                        name: '步骤9：执行备份',
                        local: 'nas_s_0029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2139,
                parentId: 2132,
                name: '复制',
                local: 'nas_s_0032.html',
                children: [
                  {
                    id: 2169,
                    parentId: 2139,
                    name: '复制NAS文件系统/NAS共享副本',
                    local: 'nas_s_9999_1.html',
                    children: [
                      {
                        id: 2170,
                        parentId: 2169,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'nas_s_9999_2.html'
                      },
                      {
                        id: 2171,
                        parentId: 2169,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'nas_s_9999_3.html'
                      },
                      {
                        id: 2172,
                        parentId: 2169,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'nas_s_9999_4.html'
                      },
                      {
                        id: 2173,
                        parentId: 2169,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'nas_s_9999_5.html'
                      },
                      {
                        id: 2174,
                        parentId: 2169,
                        name: '步骤4：下载并导入证书',
                        local: 'nas_s_9999_6.html'
                      },
                      {
                        id: 2175,
                        parentId: 2169,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'nas_s_9999_7.html'
                      },
                      {
                        id: 2176,
                        parentId: 2169,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'nas_s_9999_8.html'
                      },
                      {
                        id: 2177,
                        parentId: 2169,
                        name: '步骤6：添加复制集群',
                        local: 'nas_s_9999_9.html'
                      },
                      {
                        id: 2178,
                        parentId: 2169,
                        name: '步骤7：创建复制SLA',
                        local: 'nas_s_9999_10.html'
                      },
                      {
                        id: 2179,
                        parentId: 2169,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'nas_s_9999_11.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2140,
                parentId: 2132,
                name: '归档',
                local: 'nas_s_0045.html',
                children: [
                  {
                    id: 2180,
                    parentId: 2140,
                    name: '归档NAS文件系统/NAS共享备份副本',
                    local: 'nas_s_0048.html',
                    children: [
                      {
                        id: 2182,
                        parentId: 2180,
                        name: '步骤1：添加归档存储',
                        local: 'nas_s_0049.html',
                        children: [
                          {
                            id: 2184,
                            parentId: 2182,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'nas_s_0050.html'
                          },
                          {
                            id: 2185,
                            parentId: 2182,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'nas_s_0051.html'
                          }
                        ]
                      },
                      {
                        id: 2183,
                        parentId: 2180,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'nas_s_0052.html'
                      }
                    ]
                  },
                  {
                    id: 2181,
                    parentId: 2140,
                    name: '归档NAS文件系统/NAS共享复制副本',
                    local: 'nas_s_0053.html',
                    children: [
                      {
                        id: 2186,
                        parentId: 2181,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'nas_s_0054.html'
                      },
                      {
                        id: 2187,
                        parentId: 2181,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'nas_s_0055.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2141,
                parentId: 2132,
                name: '恢复',
                local: 'nas_s_0056.html',
                children: [
                  {
                    id: 2188,
                    parentId: 2141,
                    name: '恢复NAS文件系统',
                    local: 'nas_s_0059.html'
                  },
                  {
                    id: 2189,
                    parentId: 2141,
                    name: '恢复NAS文件系统中的文件',
                    local: 'nas_s_0060.html'
                  },
                  {
                    id: 2190,
                    parentId: 2141,
                    name: '恢复NAS共享',
                    local: 'nas_s_0061.html'
                  },
                  {
                    id: 2191,
                    parentId: 2141,
                    name: '恢复NAS共享中的文件',
                    local: 'nas_s_0062.html'
                  }
                ]
              },
              {
                id: 2142,
                parentId: 2132,
                name: '即时挂载',
                local: 'nas_s_0063.html',
                children: [
                  {
                    id: 2192,
                    parentId: 2142,
                    name: '关于即时挂载',
                    local: 'nas_s_0064.html'
                  },
                  {
                    id: 2193,
                    parentId: 2142,
                    name: '约束与限制',
                    local: 'nas_s_0065.html'
                  },
                  {
                    id: 2194,
                    parentId: 2142,
                    name: '即时挂载NAS文件系统',
                    local: 'nas_s_0066.html'
                  },
                  {
                    id: 2195,
                    parentId: 2142,
                    name: '即时挂载NAS共享',
                    local: 'nas_s_0067.html'
                  },
                  {
                    id: 2196,
                    parentId: 2142,
                    name: '手动挂载NAS文件系统或NAS共享',
                    local: 'nas_s_0108.html'
                  },
                  {
                    id: 2197,
                    parentId: 2142,
                    name: '管理即时挂载',
                    local: 'nas_s_0068.html',
                    children: [
                      {
                        id: 2198,
                        parentId: 2197,
                        name: '查看NAS文件系统即时挂载信息',
                        local: 'nas_s_0069.html'
                      },
                      {
                        id: 2199,
                        parentId: 2197,
                        name: '查看NAS共享即时挂载信息',
                        local: 'nas_s_0070.html'
                      },
                      {
                        id: 2200,
                        parentId: 2197,
                        name: '管理NAS文件系统即时挂载',
                        local: 'nas_s_0071.html'
                      },
                      {
                        id: 2201,
                        parentId: 2197,
                        name: '管理NAS共享即时挂载',
                        local: 'nas_s_0072.html'
                      },
                      {
                        id: 2202,
                        parentId: 2197,
                        name: '创建NAS文件系统挂载更新策略',
                        local: 'nas_s_0199.html'
                      },
                      {
                        id: 2203,
                        parentId: 2197,
                        name: '创建NAS共享挂载更新策略',
                        local: 'nas_s_0198.html'
                      },
                      {
                        id: 2204,
                        parentId: 2197,
                        name: '管理NAS文件系统挂载更新策略',
                        local: 'nas_s_0197.html'
                      },
                      {
                        id: 2205,
                        parentId: 2197,
                        name: '管理NAS共享挂载更新策略',
                        local: 'nas_s_0196.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2143,
                parentId: 2132,
                name: '全局搜索',
                local: 'nas_s_0073.html',
                children: [
                  {
                    id: 2206,
                    parentId: 2143,
                    name: '关于全局搜索',
                    local: 'fc_gud_gs2.html'
                  },
                  {
                    id: 2207,
                    parentId: 2143,
                    name: '全局搜索副本数据',
                    local: 'nas_s_0074.html'
                  },
                  {
                    id: 2208,
                    parentId: 2143,
                    name: '全局搜索资源',
                    local: 'nas_s_0075.html'
                  },
                  {
                    id: 2209,
                    parentId: 2143,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'zh-cn_topic_0000002200131629.html'
                  }
                ]
              },
              {
                id: 2144,
                parentId: 2132,
                name: 'SLA',
                local: 'nas_s_0078.html',
                children: [
                  {
                    id: 2210,
                    parentId: 2144,
                    name: '关于SLA',
                    local: 'nas_s_0079.html'
                  },
                  {
                    id: 2211,
                    parentId: 2144,
                    name: '查看SLA信息',
                    local: 'nas_s_0080.html'
                  },
                  {
                    id: 2212,
                    parentId: 2144,
                    name: '管理SLA',
                    local: 'nas_s_0081.html'
                  }
                ]
              },
              {
                id: 2145,
                parentId: 2132,
                name: '副本',
                local: 'nas_s_0082.html',
                children: [
                  {
                    id: 2213,
                    parentId: 2145,
                    name: '查看NAS文件系统副本信息',
                    local: 'nas_s_0083.html'
                  },
                  {
                    id: 2214,
                    parentId: 2145,
                    name: '管理NAS文件系统副本',
                    local: 'nas_s_0084.html'
                  },
                  {
                    id: 2215,
                    parentId: 2145,
                    name: '查看NAS共享副本信息',
                    local: 'nas_s_0086.html'
                  },
                  {
                    id: 2216,
                    parentId: 2145,
                    name: '管理NAS共享副本',
                    local: 'nas_s_0087.html'
                  }
                ]
              },
              {
                id: 2146,
                parentId: 2132,
                name: '存储设备信息',
                local: 'nas_s_0088.html',
                children: [
                  {
                    id: 2217,
                    parentId: 2146,
                    name: '查看存储设备信息',
                    local: 'nas_s_0089.html'
                  },
                  {
                    id: 2218,
                    parentId: 2146,
                    name: '管理存储设备信息',
                    local: 'nas_s_0090.html'
                  }
                ]
              },
              {
                id: 2147,
                parentId: 2132,
                name: 'NAS文件系统',
                local: 'nas_s_0091.html',
                children: [
                  {
                    id: 2219,
                    parentId: 2147,
                    name: '查看NAS文件系统',
                    local: 'nas_s_0092.html'
                  },
                  {
                    id: 2220,
                    parentId: 2147,
                    name: '管理NAS文件系统',
                    local: 'nas_s_0093.html'
                  }
                ]
              },
              {
                id: 2148,
                parentId: 2132,
                name: 'NAS共享',
                local: 'nas_s_0094.html',
                children: [
                  {
                    id: 2221,
                    parentId: 2148,
                    name: '查看NAS共享信息',
                    local: 'nas_s_0095.html'
                  },
                  {
                    id: 2222,
                    parentId: 2148,
                    name: '管理NAS共享',
                    local: 'nas_s_0096.html'
                  }
                ]
              },
              {
                id: 2149,
                parentId: 2132,
                name: '常见问题',
                local: 'nas_s_0097.html',
                children: [
                  {
                    id: 2223,
                    parentId: 2149,
                    name: '登录DeviceManager管理界面',
                    local: 'nas_s_dm.html'
                  },
                  {
                    id: 2224,
                    parentId: 2149,
                    name: '配置DNS服务',
                    local: 'nas_s_0099.html'
                  },
                  {
                    id: 2225,
                    parentId: 2149,
                    name: '创建文件系统',
                    local: 'nas_s_0100.html'
                  },
                  {
                    id: 2226,
                    parentId: 2149,
                    name:
                      'NAS共享或文件集聚合副本进行文件级恢复，副本目录展开失败',
                    local: 'nas_s_0100_1.html'
                  },
                  {
                    id: 2227,
                    parentId: 2149,
                    name: '修改目标端重删设置',
                    local: 'zh-cn_topic_0000002164804928.html'
                  },
                  {
                    id: 2228,
                    parentId: 2149,
                    name: '查看用户信息',
                    local: 'zh-cn_topic_0000002164804940.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2133,
            parentId: 19,
            name: 'NDMP NAS文件系统数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002164767454.html',
            children: [
              {
                id: 2229,
                parentId: 2133,
                name: '备份',
                local: 'ndmp_0007.html',
                children: [
                  {
                    id: 2239,
                    parentId: 2229,
                    name: '备份前准备',
                    local: 'ndmp_0010.html'
                  },
                  {
                    id: 2240,
                    parentId: 2229,
                    name: '备份NDMP NAS文件系统',
                    local: 'ndmp_0011.html',
                    children: [
                      {
                        id: 2241,
                        parentId: 2240,
                        name: '步骤1：添加存储设备',
                        local: 'ndmp_0014.html'
                      },
                      {
                        id: 2242,
                        parentId: 2240,
                        name: '步骤2：（可选）创建文件目录',
                        local: 'ndmp_0015.html'
                      },
                      {
                        id: 2243,
                        parentId: 2240,
                        name: '步骤3：（可选）创建限速策略',
                        local: 'ndmp_0017.html'
                      },
                      {
                        id: 2244,
                        parentId: 2240,
                        name: '步骤4：创建备份SLA',
                        local: 'ndmp_0018.html'
                      },
                      {
                        id: 2245,
                        parentId: 2240,
                        name: '步骤5：执行备份',
                        local: 'ndmp_0019.html',
                        children: [
                          {
                            id: 2246,
                            parentId: 2245,
                            name: '备份文件系统',
                            local: 'ndmp_0019-1.html'
                          },
                          {
                            id: 2247,
                            parentId: 2245,
                            name: '备份文件目录',
                            local: 'ndmp_0019-2.html'
                          }
                        ]
                      }
                    ]
                  }
                ]
              },
              {
                id: 2230,
                parentId: 2133,
                name: '复制',
                local: 'ndmp_0022.html',
                children: [
                  {
                    id: 2248,
                    parentId: 2230,
                    name: '复制NDMP NAS文件系统',
                    local: 'ndmp_0026.html',
                    children: [
                      {
                        id: 2249,
                        parentId: 2248,
                        name: '步骤1：创建复制网络逻辑端口',
                        local: 'ndmp_0027.html'
                      },
                      {
                        id: 2250,
                        parentId: 2248,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'ndmp_0028.html'
                      },
                      {
                        id: 2251,
                        parentId: 2248,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'ndmp_0029.html'
                      },
                      {
                        id: 2252,
                        parentId: 2248,
                        name: '步骤4：下载并导入证书',
                        local: 'ndmp_0030.html'
                      },
                      {
                        id: 2253,
                        parentId: 2248,
                        name: '步骤5：创建远端设备管理员',
                        local: 'ndmp_0031.html'
                      },
                      {
                        id: 2254,
                        parentId: 2248,
                        name: '步骤6：添加复制集群',
                        local: 'ndmp_0032.html'
                      },
                      {
                        id: 2255,
                        parentId: 2248,
                        name: '步骤7：创建复制SLA',
                        local: 'ndmp_0033.html'
                      },
                      {
                        id: 2256,
                        parentId: 2248,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'ndmp_0034.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2231,
                parentId: 2133,
                name: '归档',
                local: 'ndmp_0054.html',
                children: [
                  {
                    id: 2257,
                    parentId: 2231,
                    name: '归档NDMP NAS文件系统副本',
                    local: 'ndmp_0057.html',
                    children: [
                      {
                        id: 2259,
                        parentId: 2257,
                        name: '步骤1：添加归档存储',
                        local: 'ndmp_0058.html',
                        children: [
                          {
                            id: 2261,
                            parentId: 2259,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'ndmp_0059.html'
                          },
                          {
                            id: 2262,
                            parentId: 2259,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'ndmp_0060.html'
                          }
                        ]
                      },
                      {
                        id: 2260,
                        parentId: 2257,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'ndmp_0061.html'
                      }
                    ]
                  },
                  {
                    id: 2258,
                    parentId: 2231,
                    name: '归档NDMP NAS文件系统复制副本',
                    local: 'ndmp_0062.html',
                    children: [
                      {
                        id: 2263,
                        parentId: 2258,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'ndmp_0063.html'
                      },
                      {
                        id: 2264,
                        parentId: 2258,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'ndmp_0064.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2232,
                parentId: 2133,
                name: '恢复',
                local: 'ndmp_0065.html',
                children: [
                  {
                    id: 2265,
                    parentId: 2232,
                    name: '执行恢复',
                    local: 'ndmp_0068-0.html',
                    children: [
                      {
                        id: 2266,
                        parentId: 2265,
                        name: '恢复NDMP NAS文件系统/文件目录',
                        local: 'ndmp_0068.html'
                      },
                      {
                        id: 2267,
                        parentId: 2265,
                        name: '恢复NDMP NAS文件系统/文件目录中的单个或多个文件',
                        local: 'ndmp_0069.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2233,
                parentId: 2133,
                name: '全局搜索',
                local: 'ndmp_0083.html',
                children: [
                  {
                    id: 2268,
                    parentId: 2233,
                    name: '全局搜索副本数据',
                    local: 'ndmp_0084.html'
                  },
                  {
                    id: 2269,
                    parentId: 2233,
                    name: '全局搜索资源',
                    local: 'ndmp_0085.html'
                  },
                  {
                    id: 2270,
                    parentId: 2233,
                    name: '全局标签搜索',
                    local: 'ndmp_0086.html'
                  }
                ]
              },
              {
                id: 2234,
                parentId: 2133,
                name: 'SLA',
                local: 'ndmp_0089.html',
                children: [
                  {
                    id: 2271,
                    parentId: 2234,
                    name: '关于SLA',
                    local: 'ndmp_0090.html'
                  },
                  {
                    id: 2272,
                    parentId: 2234,
                    name: '查看SLA信息',
                    local: 'ndmp_0091.html'
                  },
                  {
                    id: 2273,
                    parentId: 2234,
                    name: '管理SLA',
                    local: 'ndmp_0092.html'
                  }
                ]
              },
              {
                id: 2235,
                parentId: 2133,
                name: '副本',
                local: 'ndmp_0093.html',
                children: [
                  {
                    id: 2274,
                    parentId: 2235,
                    name: '查看NDMP NAS文件系统副本信息',
                    local: 'ndmp_0094.html'
                  },
                  {
                    id: 2275,
                    parentId: 2235,
                    name: '管理NDMP NAS文件系统副本',
                    local: 'ndmp_0095.html'
                  }
                ]
              },
              {
                id: 2236,
                parentId: 2133,
                name: '存储设备信息',
                local: 'ndmp_0097.html',
                children: [
                  {
                    id: 2276,
                    parentId: 2236,
                    name: '查看存储设备信息',
                    local: 'ndmp_0098.html'
                  },
                  {
                    id: 2277,
                    parentId: 2236,
                    name: '管理存储设备信息',
                    local: 'ndmp_0099.html'
                  }
                ]
              },
              {
                id: 2237,
                parentId: 2133,
                name: 'NDMP NAS文件系统',
                local: 'ndmp_0100.html',
                children: [
                  {
                    id: 2278,
                    parentId: 2237,
                    name: '查看NDMP NAS文件系统',
                    local: 'ndmp_0101.html'
                  },
                  {
                    id: 2279,
                    parentId: 2237,
                    name: '管理NDMP NAS文件系统',
                    local: 'ndmp_0102.html'
                  }
                ]
              },
              {
                id: 2238,
                parentId: 2133,
                name: '常见问题',
                local: 'ndmp_0106.html',
                children: [
                  {
                    id: 2280,
                    parentId: 2238,
                    name: '登录DeviceManager管理界面',
                    local: 'ndmp_0110.html'
                  },
                  {
                    id: 2281,
                    parentId: 2238,
                    name: '配置DNS服务',
                    local: 'ndmp_0111.html'
                  },
                  {
                    id: 2282,
                    parentId: 2238,
                    name: '同一个NDMP Server多个并行任务部分失败',
                    local: 'ndmp_0114.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2134,
            parentId: 19,
            name: '文件集数据保护',
            local: 'zh-cn_topic_0000002164607814.html',
            children: [
              {
                id: 2283,
                parentId: 2134,
                name: '备份',
                local: 'Files-0007.html',
                children: [
                  {
                    id: 2292,
                    parentId: 2283,
                    name: '备份文件集',
                    local: 'Files-0010.html',
                    children: [
                      {
                        id: 2293,
                        parentId: 2292,
                        name: '步骤1：（可选）创建文件集模板',
                        local: 'Files-0011.html'
                      },
                      {
                        id: 2294,
                        parentId: 2292,
                        name: '步骤2：创建文件集',
                        local: 'Files-0012.html'
                      },
                      {
                        id: 2295,
                        parentId: 2292,
                        name: '步骤3：（可选）创建限速策略',
                        local: 'Files-0013.html'
                      },
                      {
                        id: 2296,
                        parentId: 2292,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'Files-0014.html'
                      },
                      {
                        id: 2297,
                        parentId: 2292,
                        name: '步骤5：创建备份SLA',
                        local: 'Files-0015.html'
                      },
                      {
                        id: 2298,
                        parentId: 2292,
                        name: '步骤6：执行文件集备份',
                        local: 'Files-0016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2284,
                parentId: 2134,
                name: '复制',
                local: 'Files-0017.html',
                children: [
                  {
                    id: 2299,
                    parentId: 2284,
                    name: '复制文件集副本',
                    local: 'Files-0021.html',
                    children: [
                      {
                        id: 2300,
                        parentId: 2299,
                        name: '步骤1：创建复制网络逻辑端口（适用于1.5.0版本）',
                        local: 'Files-0022.html'
                      },
                      {
                        id: 2301,
                        parentId: 2299,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'Files-0023.html'
                      },
                      {
                        id: 2302,
                        parentId: 2299,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'Files-0024.html'
                      },
                      {
                        id: 2303,
                        parentId: 2299,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'Files-0025.html'
                      },
                      {
                        id: 2304,
                        parentId: 2299,
                        name: '步骤4：下载并导入证书',
                        local: 'Files-0026.html'
                      },
                      {
                        id: 2305,
                        parentId: 2299,
                        name: '步骤5：创建远端设备管理员（适用于1.5.0版本）',
                        local: 'Files-0027.html'
                      },
                      {
                        id: 2306,
                        parentId: 2299,
                        name:
                          '步骤5：创建远端设备管理员（适用于1.6.0及后续版本）',
                        local: 'Files-0028.html'
                      },
                      {
                        id: 2307,
                        parentId: 2299,
                        name: '步骤6：添加复制集群',
                        local: 'Files-0029.html'
                      },
                      {
                        id: 2308,
                        parentId: 2299,
                        name: '步骤7：创建复制SLA',
                        local: 'Files-0030.html'
                      },
                      {
                        id: 2309,
                        parentId: 2299,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'Files-0031.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2285,
                parentId: 2134,
                name: '归档',
                local: 'Files-0051.html',
                children: [
                  {
                    id: 2310,
                    parentId: 2285,
                    name: '归档文件集备份副本',
                    local: 'Files-0054.html',
                    children: [
                      {
                        id: 2312,
                        parentId: 2310,
                        name: '步骤1：添加归档存储',
                        local: 'Files-0055.html',
                        children: [
                          {
                            id: 2314,
                            parentId: 2312,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'Files-0056.html'
                          },
                          {
                            id: 2315,
                            parentId: 2312,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'Files-0057.html'
                          }
                        ]
                      },
                      {
                        id: 2313,
                        parentId: 2310,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'Files-0058.html'
                      }
                    ]
                  },
                  {
                    id: 2311,
                    parentId: 2285,
                    name: '归档文件集复制副本',
                    local: 'Files-0059.html',
                    children: [
                      {
                        id: 2316,
                        parentId: 2311,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'Files-0060.html'
                      },
                      {
                        id: 2317,
                        parentId: 2311,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'Files-0061.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2286,
                parentId: 2134,
                name: '恢复',
                local: 'Files-0062.html',
                children: [
                  {
                    id: 2318,
                    parentId: 2286,
                    name: '执行恢复',
                    local: 'Files-0065.html',
                    children: [
                      {
                        id: 2319,
                        parentId: 2318,
                        name: '恢复文件集',
                        local: 'Files-0066.html'
                      },
                      {
                        id: 2320,
                        parentId: 2318,
                        name: '恢复文件集中的单个或多个文件',
                        local: 'Files-0067.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2287,
                parentId: 2134,
                name: '全局搜索',
                local: 'Files-0078.html',
                children: [
                  {
                    id: 2321,
                    parentId: 2287,
                    name: '全局搜索副本数据',
                    local: 'Files-0079.html'
                  },
                  {
                    id: 2322,
                    parentId: 2287,
                    name: '全局搜索资源',
                    local: 'Files-0080.html'
                  },
                  {
                    id: 2323,
                    parentId: 2287,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'Files-0081.html'
                  }
                ]
              },
              {
                id: 2288,
                parentId: 2134,
                name: 'SLA',
                local: 'Files-0084.html',
                children: [
                  {
                    id: 2324,
                    parentId: 2288,
                    name: '查看SLA信息',
                    local: 'Files-0086.html'
                  },
                  {
                    id: 2325,
                    parentId: 2288,
                    name: '管理SLA',
                    local: 'Files-0087.html'
                  }
                ]
              },
              {
                id: 2289,
                parentId: 2134,
                name: '副本',
                local: 'Files-0088.html',
                children: [
                  {
                    id: 2326,
                    parentId: 2289,
                    name: '查看文件集副本信息',
                    local: 'Files-0089.html'
                  },
                  {
                    id: 2327,
                    parentId: 2289,
                    name: '管理文件集副本',
                    local: 'Files-0090.html'
                  }
                ]
              },
              {
                id: 2290,
                parentId: 2134,
                name: '文件集',
                local: 'Files-0091.html',
                children: [
                  {
                    id: 2328,
                    parentId: 2290,
                    name: '查看文件集信息',
                    local: 'Files-0092.html'
                  },
                  {
                    id: 2329,
                    parentId: 2290,
                    name: '管理文件集',
                    local: 'Files-0093.html'
                  },
                  {
                    id: 2330,
                    parentId: 2290,
                    name: '管理文件集模板',
                    local: 'Files-0094.html'
                  }
                ]
              },
              {
                id: 2291,
                parentId: 2134,
                name: '常见问题',
                local: 'Files-0095.html',
                children: [
                  {
                    id: 2331,
                    parentId: 2291,
                    name: '登录DeviceManager管理界面',
                    local: 'Files-0097.html'
                  },
                  {
                    id: 2332,
                    parentId: 2291,
                    name:
                      'NAS共享或文件集聚合副本进行文件级恢复，副本目录展开失败',
                    local: 'Files-0100.html'
                  },
                  {
                    id: 2333,
                    parentId: 2291,
                    name:
                      '开启一致性备份后，逻辑卷空间不足无法创建快照，备份任务转为非一致性备份（Linux OS）',
                    local: 'Files-0101.html'
                  },
                  {
                    id: 2334,
                    parentId: 2291,
                    name: '应用临时快照文件删除失败',
                    local: 'Files-0102.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2135,
            parentId: 19,
            name: '卷数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002164767490.html',
            children: [
              {
                id: 2335,
                parentId: 2135,
                name: '备份',
                local: 'volume_0008.html',
                children: [
                  {
                    id: 2344,
                    parentId: 2335,
                    name: '备份卷',
                    local: 'volume_0011.html',
                    children: [
                      {
                        id: 2345,
                        parentId: 2344,
                        name: '步骤1：创建卷',
                        local: 'volume_0012.html'
                      },
                      {
                        id: 2346,
                        parentId: 2344,
                        name: '步骤2：（可选）创建限速策略',
                        local: 'volume_0013.html'
                      },
                      {
                        id: 2347,
                        parentId: 2344,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'volume_0014.html'
                      },
                      {
                        id: 2348,
                        parentId: 2344,
                        name: '步骤4：创建备份SLA',
                        local: 'volume_0015.html'
                      },
                      {
                        id: 2349,
                        parentId: 2344,
                        name: '步骤5：执行备份',
                        local: 'volume_0016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2336,
                parentId: 2135,
                name: '复制',
                local: 'volume_0017.html',
                children: [
                  {
                    id: 2350,
                    parentId: 2336,
                    name: '复制卷副本',
                    local: 'volume_0021.html',
                    children: [
                      {
                        id: 2351,
                        parentId: 2350,
                        name:
                          '步骤1：创建复制网络逻辑端口（适用于1.6.0及后续版本）',
                        local: 'volume_0022.html'
                      },
                      {
                        id: 2352,
                        parentId: 2350,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'volume_0023.html'
                      },
                      {
                        id: 2353,
                        parentId: 2350,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'volume_0024.html'
                      },
                      {
                        id: 2354,
                        parentId: 2350,
                        name: '步骤4：下载并导入证书',
                        local: 'volume_0025.html'
                      },
                      {
                        id: 2355,
                        parentId: 2350,
                        name: '步骤5：创建远端设备管理员',
                        local: 'volume_0026.html'
                      },
                      {
                        id: 2356,
                        parentId: 2350,
                        name: '步骤6：添加复制集群',
                        local: 'volume_0027.html'
                      },
                      {
                        id: 2357,
                        parentId: 2350,
                        name: '步骤7：创建复制SLA',
                        local: 'volume_0028.html'
                      },
                      {
                        id: 2358,
                        parentId: 2350,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'volume_0029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2337,
                parentId: 2135,
                name: '归档',
                local: 'volume_0039.html',
                children: [
                  {
                    id: 2359,
                    parentId: 2337,
                    name: '归档卷备份副本',
                    local: 'volume_0042.html',
                    children: [
                      {
                        id: 2361,
                        parentId: 2359,
                        name: '步骤1：添加归档存储',
                        local: 'volume_0043.html',
                        children: [
                          {
                            id: 2363,
                            parentId: 2361,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'volume_0044.html'
                          },
                          {
                            id: 2364,
                            parentId: 2361,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'volume_0045.html'
                          }
                        ]
                      },
                      {
                        id: 2362,
                        parentId: 2359,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'volume_0046.html'
                      }
                    ]
                  },
                  {
                    id: 2360,
                    parentId: 2337,
                    name: '归档卷复制副本',
                    local: 'volume_0047.html',
                    children: [
                      {
                        id: 2365,
                        parentId: 2360,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'volume_0048.html'
                      },
                      {
                        id: 2366,
                        parentId: 2360,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'volume_0049.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2338,
                parentId: 2135,
                name: '恢复',
                local: 'volume_0050.html',
                children: [
                  {
                    id: 2367,
                    parentId: 2338,
                    name: '恢复卷',
                    local: 'volume_0053.html'
                  },
                  {
                    id: 2368,
                    parentId: 2338,
                    name: '恢复卷副本中的单个或多个文件',
                    local: 'volume_0054.html'
                  }
                ]
              },
              {
                id: 2339,
                parentId: 2135,
                name: '全局搜索',
                local: 'volume_0064.html',
                children: [
                  {
                    id: 2369,
                    parentId: 2339,
                    name: '关于全局搜索',
                    local: 'volume_0065.html'
                  },
                  {
                    id: 2370,
                    parentId: 2339,
                    name: '全局搜索副本数据',
                    local: 'volume_0066.html'
                  },
                  {
                    id: 2371,
                    parentId: 2339,
                    name: '全局搜索资源',
                    local: 'volume_0067.html'
                  },
                  {
                    id: 2372,
                    parentId: 2339,
                    name: '全局标签搜索',
                    local: 'volume_0068.html'
                  }
                ]
              },
              {
                id: 2340,
                parentId: 2135,
                name: 'SLA',
                local: 'volume_0071.html',
                children: [
                  {
                    id: 2373,
                    parentId: 2340,
                    name: '关于SLA',
                    local: 'volume_0072.html'
                  },
                  {
                    id: 2374,
                    parentId: 2340,
                    name: '查看SLA信息',
                    local: 'volume_0073.html'
                  },
                  {
                    id: 2375,
                    parentId: 2340,
                    name: '管理SLA',
                    local: 'volume_0074.html'
                  }
                ]
              },
              {
                id: 2341,
                parentId: 2135,
                name: '副本',
                local: 'volume_0075.html',
                children: [
                  {
                    id: 2376,
                    parentId: 2341,
                    name: '查看卷副本信息',
                    local: 'volume_0076.html'
                  },
                  {
                    id: 2377,
                    parentId: 2341,
                    name: '管理卷副本',
                    local: 'volume_0077.html'
                  }
                ]
              },
              {
                id: 2342,
                parentId: 2135,
                name: '卷',
                local: 'volume_0078.html',
                children: [
                  {
                    id: 2378,
                    parentId: 2342,
                    name: '查看卷信息',
                    local: 'volume_0079.html'
                  },
                  {
                    id: 2379,
                    parentId: 2342,
                    name: '管理卷',
                    local: 'volume_0080.html'
                  }
                ]
              },
              {
                id: 2343,
                parentId: 2135,
                name: '常见问题',
                local: 'volume_0081.html',
                children: [
                  {
                    id: 2380,
                    parentId: 2343,
                    name: '登录DeviceManager管理界面',
                    local: 'volume_0083.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2136,
            parentId: 19,
            name: '对象存储数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002200008513.html',
            children: [
              {
                id: 2381,
                parentId: 2136,
                name: '备份',
                local: 'object-0007.html',
                children: [
                  {
                    id: 2390,
                    parentId: 2381,
                    name: '备份前准备',
                    local: 'object-0010.html',
                    children: [
                      {
                        id: 2392,
                        parentId: 2390,
                        name: '在生产端获取Endpoint',
                        local: 'object-0011.html'
                      },
                      {
                        id: 2393,
                        parentId: 2390,
                        name: '在生产端获取AK和SK',
                        local: 'object-0012.html'
                      }
                    ]
                  },
                  {
                    id: 2391,
                    parentId: 2381,
                    name: '备份对象存储',
                    local: 'object-0013.html',
                    children: [
                      {
                        id: 2394,
                        parentId: 2391,
                        name: '步骤1：注册对象存储',
                        local: 'object-0014.html'
                      },
                      {
                        id: 2395,
                        parentId: 2391,
                        name: '步骤2：创建对象集合',
                        local: 'object-0015.html'
                      },
                      {
                        id: 2396,
                        parentId: 2391,
                        name: '步骤3：（可选）创建限速策略',
                        local: 'object-0016.html'
                      },
                      {
                        id: 2397,
                        parentId: 2391,
                        name: '步骤4：（可选）开启备份链路加密开关',
                        local: 'object-0017.html'
                      },
                      {
                        id: 2398,
                        parentId: 2391,
                        name: '步骤5：创建备份SLA',
                        local: 'object-0018.html'
                      },
                      {
                        id: 2399,
                        parentId: 2391,
                        name: '步骤6：开启NFSv4.1服务',
                        local: 'object-0019.html'
                      },
                      {
                        id: 2400,
                        parentId: 2391,
                        name: '步骤7：执行备份',
                        local: 'object-0021.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2382,
                parentId: 2136,
                name: '复制',
                local: 'object-0024.html',
                children: [
                  {
                    id: 2401,
                    parentId: 2382,
                    name: '复制对象存储副本',
                    local: 'object-0028.html',
                    children: [
                      {
                        id: 2402,
                        parentId: 2401,
                        name: '步骤1：创建复制网络逻辑端口',
                        local: 'object-0029.html'
                      },
                      {
                        id: 2403,
                        parentId: 2401,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'object-0030.html'
                      },
                      {
                        id: 2404,
                        parentId: 2401,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'object-0031.html'
                      },
                      {
                        id: 2405,
                        parentId: 2401,
                        name: '步骤4：下载并导入证书',
                        local: 'object-0032.html'
                      },
                      {
                        id: 2406,
                        parentId: 2401,
                        name: '步骤5：创建远端设备管理员',
                        local: 'object-0033.html'
                      },
                      {
                        id: 2407,
                        parentId: 2401,
                        name: '步骤6：添加复制集群',
                        local: 'object-0034.html'
                      },
                      {
                        id: 2408,
                        parentId: 2401,
                        name: '步骤7：创建复制SLA',
                        local: 'object-0035.html'
                      },
                      {
                        id: 2409,
                        parentId: 2401,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'object-0036.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2383,
                parentId: 2136,
                name: '归档',
                local: 'object-0056.html',
                children: [
                  {
                    id: 2410,
                    parentId: 2383,
                    name: '归档对象集合备份副本',
                    local: 'object-0059.html',
                    children: [
                      {
                        id: 2412,
                        parentId: 2410,
                        name: '步骤1：添加归档存储',
                        local: 'object-0060.html',
                        children: [
                          {
                            id: 2414,
                            parentId: 2412,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'object-0061.html'
                          },
                          {
                            id: 2415,
                            parentId: 2412,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'object-0062.html'
                          }
                        ]
                      },
                      {
                        id: 2413,
                        parentId: 2410,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'object-0063.html'
                      }
                    ]
                  },
                  {
                    id: 2411,
                    parentId: 2383,
                    name: '归档对象集合复制副本',
                    local: 'object-0064.html',
                    children: [
                      {
                        id: 2416,
                        parentId: 2411,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'object-0065.html'
                      },
                      {
                        id: 2417,
                        parentId: 2411,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'object-0066.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2384,
                parentId: 2136,
                name: '恢复',
                local: 'object-0067.html',
                children: [
                  {
                    id: 2418,
                    parentId: 2384,
                    name: '恢复对象存储',
                    local: 'object-0070-1.html',
                    children: [
                      {
                        id: 2419,
                        parentId: 2418,
                        name: '恢复对象存储的桶',
                        local: 'object-0070.html'
                      },
                      {
                        id: 2420,
                        parentId: 2418,
                        name: '恢复对象存储桶中的单个或多个对象',
                        local: 'object-0070-2.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2385,
                parentId: 2136,
                name: '全局搜索',
                local: 'object-0071.html',
                children: [
                  {
                    id: 2421,
                    parentId: 2385,
                    name: '全局搜索副本数据',
                    local: 'object-0072.html'
                  },
                  {
                    id: 2422,
                    parentId: 2385,
                    name: '全局搜索资源',
                    local: 'object-0073.html'
                  },
                  {
                    id: 2423,
                    parentId: 2385,
                    name: '全局标签搜索',
                    local: 'object-0074.html'
                  }
                ]
              },
              {
                id: 2386,
                parentId: 2136,
                name: 'SLA',
                local: 'object-0077.html',
                children: [
                  {
                    id: 2424,
                    parentId: 2386,
                    name: '关于SLA',
                    local: 'object-0078.html'
                  },
                  {
                    id: 2425,
                    parentId: 2386,
                    name: '查看SLA信息',
                    local: 'object-0079.html'
                  },
                  {
                    id: 2426,
                    parentId: 2386,
                    name: '管理SLA',
                    local: 'object-0080.html'
                  }
                ]
              },
              {
                id: 2387,
                parentId: 2136,
                name: '副本',
                local: 'object-0081.html',
                children: [
                  {
                    id: 2427,
                    parentId: 2387,
                    name: '查看对象存储副本信息',
                    local: 'object-0082.html'
                  },
                  {
                    id: 2428,
                    parentId: 2387,
                    name: '管理对象存储副本',
                    local: 'object-0083.html'
                  }
                ]
              },
              {
                id: 2388,
                parentId: 2136,
                name: '对象存储',
                local: 'object-0084.html',
                children: [
                  {
                    id: 2429,
                    parentId: 2388,
                    name: '查看对象存储信息',
                    local: 'object-0085.html'
                  },
                  {
                    id: 2430,
                    parentId: 2388,
                    name: '管理对象集合',
                    local: 'object-0086.html'
                  }
                ]
              },
              {
                id: 2389,
                parentId: 2136,
                name: '常见问题',
                local: 'object-0087.html',
                children: [
                  {
                    id: 2431,
                    parentId: 2389,
                    name: '登录DeviceManager管理界面',
                    local: 'object-0089.html'
                  },
                  {
                    id: 2432,
                    parentId: 2389,
                    name: '对象存储备份或恢复任务部分成功查看失败文件列表方法',
                    local: 'object-0092.html'
                  },
                  {
                    id: 2433,
                    parentId: 2389,
                    name: '配置DNS服务',
                    local: 'object-0093.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2137,
            parentId: 19,
            name: '通用共享数据保护（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002164607786.html',
            children: [
              {
                id: 2434,
                parentId: 2137,
                name: '备份',
                local: 'commonshares_0008.html',
                children: [
                  {
                    id: 2443,
                    parentId: 2434,
                    name: '备份通用共享资源数据',
                    local: 'commonshares_0011.html',
                    children: [
                      {
                        id: 2444,
                        parentId: 2443,
                        name: '步骤1：创建通用共享',
                        local: 'commonshares_0012.html'
                      },
                      {
                        id: 2445,
                        parentId: 2443,
                        name: '步骤2：（可选）创建限速策略',
                        local: 'commonshares_0013.html'
                      },
                      {
                        id: 2446,
                        parentId: 2443,
                        name: '步骤3：（可选）开启备份链路加密开关',
                        local: 'commonshares_0014.html'
                      },
                      {
                        id: 2447,
                        parentId: 2443,
                        name: '步骤4：创建备份SLA',
                        local: 'commonshares_0015.html'
                      },
                      {
                        id: 2448,
                        parentId: 2443,
                        name: '步骤5：执行备份',
                        local: 'commonshares_0016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2435,
                parentId: 2137,
                name: '复制',
                local: 'commonshares_0019.html',
                children: [
                  {
                    id: 2449,
                    parentId: 2435,
                    name: '复制通用共享副本',
                    local: 'commonshares_0022.html',
                    children: [
                      {
                        id: 2450,
                        parentId: 2449,
                        name: '步骤1：创建复制网络逻辑端口',
                        local: 'commonshares_0024.html'
                      },
                      {
                        id: 2451,
                        parentId: 2449,
                        name: '步骤2：（可选）创建IPsec策略',
                        local: 'commonshares_0025.html'
                      },
                      {
                        id: 2452,
                        parentId: 2449,
                        name: '步骤3：（可选）开启复制链路加密开关',
                        local: 'commonshares_0026.html'
                      },
                      {
                        id: 2453,
                        parentId: 2449,
                        name: '步骤4：下载并导入证书',
                        local: 'commonshares_0027.html'
                      },
                      {
                        id: 2454,
                        parentId: 2449,
                        name: '步骤5：创建远端设备管理员',
                        local: 'commonshares_0028.html'
                      },
                      {
                        id: 2455,
                        parentId: 2449,
                        name: '步骤6：添加复制集群',
                        local: 'commonshares_0029.html'
                      },
                      {
                        id: 2456,
                        parentId: 2449,
                        name: '步骤7：创建复制SLA',
                        local: 'commonshares_0030.html'
                      },
                      {
                        id: 2457,
                        parentId: 2449,
                        name: '步骤8：创建反向复制/级联复制SLA',
                        local: 'commonshares_0031.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2436,
                parentId: 2137,
                name: '归档',
                local: 'commonshares_0032.html',
                children: [
                  {
                    id: 2458,
                    parentId: 2436,
                    name: '归档通用共享资源备份副本',
                    local: 'commonshares_0035.html',
                    children: [
                      {
                        id: 2460,
                        parentId: 2458,
                        name: '步骤1：添加归档存储',
                        local: 'commonshares_0036.html',
                        children: [
                          {
                            id: 2462,
                            parentId: 2460,
                            name: '添加对象存储（归档存储是对象存储）',
                            local: 'commonshares_0037.html'
                          },
                          {
                            id: 2463,
                            parentId: 2460,
                            name:
                              '创建介质集（归档存储是磁带库）（适用于部分型号）',
                            local: 'commonshares_0038.html'
                          }
                        ]
                      },
                      {
                        id: 2461,
                        parentId: 2458,
                        name: '步骤2：创建备份副本归档SLA',
                        local: 'commonshares_0039.html'
                      }
                    ]
                  },
                  {
                    id: 2459,
                    parentId: 2436,
                    name: '归档通用共享资源复制副本',
                    local: 'commonshares_0040.html',
                    children: [
                      {
                        id: 2464,
                        parentId: 2459,
                        name: '步骤1：创建复制副本归档SLA',
                        local: 'commonshares_0041.html'
                      },
                      {
                        id: 2465,
                        parentId: 2459,
                        name: '步骤2：创建复制副本周期性归档',
                        local: 'commonshares_0042.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2437,
                parentId: 2137,
                name: '管理共享信息',
                local: 'commonshares_0043.html',
                children: [
                  {
                    id: 2466,
                    parentId: 2437,
                    name: '配置共享信息',
                    local: 'commonshares_0044.html'
                  },
                  {
                    id: 2467,
                    parentId: 2437,
                    name: '查看共享信息',
                    local: 'commonshares_0045.html'
                  },
                  {
                    id: 2468,
                    parentId: 2437,
                    name: '删除共享信息',
                    local: 'commonshares_0046.html'
                  }
                ]
              },
              {
                id: 2438,
                parentId: 2137,
                name: '全局搜索',
                local: 'commonshares_0047.html',
                children: [
                  {
                    id: 2469,
                    parentId: 2438,
                    name: '全局搜索资源',
                    local: 'commonshares_0048.html'
                  },
                  {
                    id: 2470,
                    parentId: 2438,
                    name: '全局标签搜索',
                    local: 'commonshares_0049.html'
                  }
                ]
              },
              {
                id: 2439,
                parentId: 2137,
                name: 'SLA',
                local: 'commonshares_0050.html',
                children: [
                  {
                    id: 2471,
                    parentId: 2439,
                    name: '关于SLA',
                    local: 'commonshares_0051.html'
                  },
                  {
                    id: 2472,
                    parentId: 2439,
                    name: '查看SLA信息',
                    local: 'commonshares_0052.html'
                  },
                  {
                    id: 2473,
                    parentId: 2439,
                    name: '管理SLA',
                    local: 'commonshares_0053.html'
                  }
                ]
              },
              {
                id: 2440,
                parentId: 2137,
                name: '副本',
                local: 'commonshares_0054.html',
                children: [
                  {
                    id: 2474,
                    parentId: 2440,
                    name: '查看通用共享资源副本信息',
                    local: 'commonshares_0055.html'
                  },
                  {
                    id: 2475,
                    parentId: 2440,
                    name: '管理通用共享资源副本',
                    local: 'commonshares_0056.html'
                  }
                ]
              },
              {
                id: 2441,
                parentId: 2137,
                name: '通用共享',
                local: 'commonshares_0057.html',
                children: [
                  {
                    id: 2476,
                    parentId: 2441,
                    name: '查看通用共享信息',
                    local: 'commonshares_0058.html'
                  },
                  {
                    id: 2477,
                    parentId: 2441,
                    name: '管理通用共享',
                    local: 'commonshares_0059.html'
                  }
                ]
              },
              {
                id: 2442,
                parentId: 2137,
                name: '常见问题',
                local: 'commonshares_0060.html',
                children: [
                  {
                    id: 2478,
                    parentId: 2442,
                    name: '登录DeviceManager管理界面',
                    local: 'commonshares_0062.html'
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
        id: 2479,
        parentId: 4,
        name: '恢复演练',
        local: 'zh-cn_topic_0000002200029753.html',
        children: [
          {
            id: 2483,
            parentId: 2479,
            name: '创建演练计划',
            local: 'Ransomware0011.html'
          },
          {
            id: 2484,
            parentId: 2479,
            name: '管理演练计划',
            local: 'zh-cn_topic_0000002164604958.html'
          },
          {
            id: 2485,
            parentId: 2479,
            name: '总览恢复演练',
            local: 'zh-cn_topic_0000002199971281.html'
          },
          {
            id: 2486,
            parentId: 2479,
            name: '常见问题',
            local: 'Ransomware0027.html',
            children: [
              {
                id: 2487,
                parentId: 2486,
                name: '不同操作系统脚本格式要求',
                local: 'zh-cn_topic_0000002164604938.html'
              }
            ]
          }
        ]
      },
      {
        id: 2480,
        parentId: 4,
        name: '数据脱敏',
        local: 'helpcenter_000092.html',
        children: [
          {
            id: 2488,
            parentId: 2480,
            name: '配置数据脱敏',
            local: 'anonymization_0010.html',
            children: [
              {
                id: 2494,
                parentId: 2488,
                name: '导入并激活License文件',
                local: 'anonymization_0011.html'
              },
              {
                id: 2495,
                parentId: 2488,
                name: '添加脱敏规则',
                local: 'anonymization_0012.html'
              },
              {
                id: 2496,
                parentId: 2488,
                name: '添加识别规则',
                local: 'anonymization_0013.html'
              },
              {
                id: 2497,
                parentId: 2488,
                name: '创建脱敏策略',
                local: 'anonymization_0014.html'
              }
            ]
          },
          {
            id: 2489,
            parentId: 2480,
            name: 'Oracle数据脱敏',
            local: 'anonymization_0015.html'
          },
          {
            id: 2490,
            parentId: 2480,
            name: '管理数据脱敏',
            local: 'anonymization_0016.html',
            children: [
              {
                id: 2498,
                parentId: 2490,
                name: '管理脱敏策略',
                local: 'anonymization_0017.html'
              },
              {
                id: 2499,
                parentId: 2490,
                name: '管理识别规则',
                local: 'anonymization_0018.html'
              },
              {
                id: 2500,
                parentId: 2490,
                name: '管理脱敏规则',
                local: 'anonymization_0019.html'
              }
            ]
          },
          {
            id: 2491,
            parentId: 2480,
            name: '常见问题',
            local: 'zh-cn_topic_0000002232113305.html'
          },
          {
            id: 2492,
            parentId: 2480,
            name: '脱敏规则类型说明',
            local: 'anonymization_0020.html'
          },
          {
            id: 2493,
            parentId: 2480,
            name: '配置数据库侦听',
            local: 'anonymization_0021.html'
          }
        ]
      },
      {
        id: 2481,
        parentId: 4,
        name: '防勒索',
        local: 'helpcenter_000094.html',
        children: [
          {
            id: 2501,
            parentId: 2481,
            name: '配置副本防勒索',
            local: 'ransome_0011.html',
            children: [
              {
                id: 2507,
                parentId: 2501,
                name: '创建防勒索\u0026WORM策略',
                local: 'ransome_0012.html'
              }
            ]
          },
          {
            id: 2502,
            parentId: 2481,
            name: '执行副本防勒索（适用于1.5.0版本）',
            local: 'ransome_0013.html',
            children: [
              {
                id: 2508,
                parentId: 2502,
                name: 'VMware副本勒索软件检测',
                local: 'ransome_0014.html'
              },
              {
                id: 2509,
                parentId: 2502,
                name: 'NAS文件系统副本勒索软件检测',
                local: 'ransome_0015.html'
              },
              {
                id: 2510,
                parentId: 2502,
                name: 'NAS共享副本勒索软件检测',
                local: 'ransome_0016.html'
              },
              {
                id: 2511,
                parentId: 2502,
                name: '文件集副本勒索软件检测',
                local: 'ransome_0017.html'
              }
            ]
          },
          {
            id: 2503,
            parentId: 2481,
            name: '执行副本防勒索（适用于1.6.0及后续版本）',
            local: 'ransome16_001.html',
            children: [
              {
                id: 2512,
                parentId: 2503,
                name: 'VMware虚拟机副本勒索软件检测',
                local: 'ransome16_002.html'
              },
              {
                id: 2513,
                parentId: 2503,
                name: 'NAS文件系统副本勒索软件检测',
                local: 'ransome16_003.html'
              },
              {
                id: 2514,
                parentId: 2503,
                name: 'NAS共享副本勒索软件检测',
                local: 'ransome16_004.html'
              },
              {
                id: 2515,
                parentId: 2503,
                name: '文件集副本勒索软件检测',
                local: 'ransome16_005.html'
              },
              {
                id: 2516,
                parentId: 2503,
                name: 'CNware虚拟机副本勒索软件检测',
                local: 'ransome16_006.html'
              },
              {
                id: 2517,
                parentId: 2503,
                name: '华为云Stack副本勒索软件检测',
                local: 'ransome16_007.html'
              },
              {
                id: 2518,
                parentId: 2503,
                name: 'FusionCompute虚拟机副本勒索软件检测',
                local: 'ransome16_008.html'
              },
              {
                id: 2519,
                parentId: 2503,
                name: 'Hyper-V虚拟机副本勒索软件检测',
                local: 'ransome16_010.html'
              },
              {
                id: 2520,
                parentId: 2503,
                name: 'FusionOne Compute虚拟机副本勒索软件检测',
                local: 'ransome160_012.html'
              }
            ]
          },
          {
            id: 2504,
            parentId: 2481,
            name: '管理副本防勒索',
            local: 'ransome_0018.html',
            children: [
              {
                id: 2521,
                parentId: 2504,
                name: '管理检测模型',
                local: 'ransome_0019.html'
              },
              {
                id: 2522,
                parentId: 2504,
                name: '管理防勒索\u0026WORM策略',
                local: 'ransome_0020.html'
              },
              {
                id: 2523,
                parentId: 2504,
                name: '管理检测模式',
                local: 'ransome_0021.html'
              },
              {
                id: 2524,
                parentId: 2504,
                name: '管理勒索检测副本',
                local: 'ransome_0022.html'
              },
              {
                id: 2525,
                parentId: 2504,
                name: '管理WORM副本',
                local: 'ransome_0023.html'
              },
              {
                id: 2526,
                parentId: 2504,
                name: '管理感染副本操作限制（适用于1.6.0及后续版本）',
                local: 'ransome_dis_001.html',
                children: [
                  {
                    id: 2527,
                    parentId: 2526,
                    name: '新增感染副本操作限制',
                    local: 'ransome_dis_002.html'
                  },
                  {
                    id: 2528,
                    parentId: 2526,
                    name: '浏览感染副本操作限制信息',
                    local: 'ransome_dis_003.html'
                  },
                  {
                    id: 2529,
                    parentId: 2526,
                    name: '修改感染副本操作限制',
                    local: 'ransome_dis_004.html'
                  },
                  {
                    id: 2530,
                    parentId: 2526,
                    name: '删除感染副本操作限制',
                    local: 'ransome_dis_005.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2505,
            parentId: 2481,
            name: '查看资源检测详情（适用于1.5.0版本）',
            local: 'ransome_0024.html',
            children: [
              {
                id: 2531,
                parentId: 2505,
                name: '查看所有资源检测详情',
                local: 'ransome_0025.html'
              },
              {
                id: 2532,
                parentId: 2505,
                name: '查看单个资源类型检测详情',
                local: 'ransome_0026.html'
              }
            ]
          },
          {
            id: 2506,
            parentId: 2481,
            name: '查看资源检测详情（适用于1.6.0及后续版本）',
            local: 'ransome16_011.html',
            children: [
              {
                id: 2533,
                parentId: 2506,
                name: '查看所有资源检测详情',
                local: 'ransome16_012.html'
              },
              {
                id: 2534,
                parentId: 2506,
                name: '查看单个资源类型检测详情',
                local: 'ransome16_013.html'
              }
            ]
          }
        ]
      },
      {
        id: 2482,
        parentId: 4,
        name: 'Air Gap',
        local: 'helpcenter_000096.html',
        children: [
          {
            id: 2535,
            parentId: 2482,
            name: '配置Air Gap',
            local: 'airgap_0011.html',
            children: [
              {
                id: 2539,
                parentId: 2535,
                name: '创建Air Gap策略',
                local: 'airgap_0012.html'
              },
              {
                id: 2540,
                parentId: 2535,
                name: '关联Air Gap策略',
                local: 'airgap_0013.html'
              }
            ]
          },
          {
            id: 2536,
            parentId: 2482,
            name: '管理Air Gap策略',
            local: 'airgap_0014.html',
            children: [
              {
                id: 2541,
                parentId: 2536,
                name: '查看Air Gap策略',
                local: 'airgap_0015.html'
              },
              {
                id: 2542,
                parentId: 2536,
                name: '修改Air Gap策略',
                local: 'airgap_0016.html'
              },
              {
                id: 2543,
                parentId: 2536,
                name: '删除Air Gap策略',
                local: 'airgap_0017.html'
              }
            ]
          },
          {
            id: 2537,
            parentId: 2482,
            name: '管理存储设备',
            local: 'airgap_0018.html',
            children: [
              {
                id: 2544,
                parentId: 2537,
                name: '查看存储设备',
                local: 'airgap_0019.html'
              },
              {
                id: 2545,
                parentId: 2537,
                name: '修改存储设备关联的Air Gap策略',
                local: 'airgap_0020.html'
              },
              {
                id: 2546,
                parentId: 2537,
                name: '移除存储设备关联的Air Gap策略',
                local: 'airgap_0021.html'
              },
              {
                id: 2547,
                parentId: 2537,
                name: '开启存储设备关联的Air Gap策略',
                local: 'airgap_0022.html'
              },
              {
                id: 2548,
                parentId: 2537,
                name: '关闭存储设备关联的Air Gap策略',
                local: 'airgap_0023.html'
              },
              {
                id: 2549,
                parentId: 2537,
                name: '断开存储设备的复制链路（适用于1.6.0及后续版本）',
                local: 'airgap_00231.html'
              }
            ]
          },
          {
            id: 2538,
            parentId: 2482,
            name: '常见问题',
            local: 'airgap_0024.html'
          }
        ]
      }
    ]
  },
  {
    id: 5,
    parentId: 0,
    name: '集群高可用',
    local: 'zh-cn_topic_0000002200115409.html',
    children: [
      {
        id: 2550,
        parentId: 5,
        name: '配置集群高可用',
        local: 'HA00010.html',
        children: [
          {
            id: 2554,
            parentId: 2550,
            name: '添加主节点内部通信网络平面',
            local: 'zh-cn_topic_0000002164632012.html',
            children: [
              {
                id: 2559,
                parentId: 2554,
                name: '添加主节点内部通信网络平面（适用于1.5.0版本）',
                local: 'HA00011.html'
              },
              {
                id: 2560,
                parentId: 2554,
                name: '添加主节点内部通信网络（适用于1.6.0及后续版本）',
                local: 'zh-cn_topic_0000002200118337.html'
              }
            ]
          },
          {
            id: 2555,
            parentId: 2550,
            name: '添加成员节点内部通信网络（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002164791784.html'
          },
          {
            id: 2556,
            parentId: 2550,
            name: '添加成员节点',
            local: 'HA00012.html'
          },
          {
            id: 2557,
            parentId: 2550,
            name: '添加HA成员',
            local: 'HA00013.html'
          },
          {
            id: 2558,
            parentId: 2550,
            name: '（可选）创建备份存储单元组',
            local: 'HA00014.html'
          }
        ]
      },
      { id: 2551, parentId: 5, name: '使用集群高可用', local: 'HA00015.html' },
      {
        id: 2552,
        parentId: 5,
        name: '管理集群高可用',
        local: 'HA00016.html',
        children: [
          {
            id: 2561,
            parentId: 2552,
            name: '管理本地集群节点',
            local: 'HA00017.html',
            children: [
              {
                id: 2568,
                parentId: 2561,
                name: '查看本地集群节点',
                local: 'HA00018.html'
              },
              {
                id: 2569,
                parentId: 2561,
                name: '管理备节点/成员节点',
                local: 'HA00019.html',
                children: [
                  {
                    id: 2571,
                    parentId: 2569,
                    name: '修改备节点/成员节点',
                    local: 'HA00020.html'
                  },
                  {
                    id: 2572,
                    parentId: 2569,
                    name: '删除成员节点',
                    local: 'HA00021.html'
                  }
                ]
              },
              {
                id: 2570,
                parentId: 2561,
                name: '管理HA',
                local: 'HA00022.html',
                children: [
                  {
                    id: 2573,
                    parentId: 2570,
                    name: '修改HA参数',
                    local: 'HA00023.html'
                  },
                  {
                    id: 2574,
                    parentId: 2570,
                    name: '移除HA成员',
                    local: 'HA00024.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2562,
            parentId: 2552,
            name: '管理备份存储单元组',
            local: 'HA00025.html',
            children: [
              {
                id: 2575,
                parentId: 2562,
                name: '查看备份存储单元组',
                local: 'HA00026.html'
              },
              {
                id: 2576,
                parentId: 2562,
                name: '修改备份存储单元组',
                local: 'HA00027.html'
              },
              {
                id: 2577,
                parentId: 2562,
                name: '删除备份存储单元组',
                local: 'HA00028.html'
              }
            ]
          },
          {
            id: 2563,
            parentId: 2552,
            name: '管理备份存储单元（适用于1.5.0版本）',
            local: 'HA00029.html',
            children: [
              {
                id: 2578,
                parentId: 2563,
                name: '查看备份存储单元',
                local: 'HA00030.html'
              },
              {
                id: 2579,
                parentId: 2563,
                name: '创建备份存储单元',
                local: 'HA00031.html'
              },
              {
                id: 2580,
                parentId: 2563,
                name: '修改备份存储单元',
                local: 'HA00032.html'
              },
              {
                id: 2581,
                parentId: 2563,
                name: '删除备份存储单元',
                local: 'HA00033.html'
              },
              {
                id: 2582,
                parentId: 2563,
                name: '备份存储单元升级为成员节点',
                local: 'HA00034.html'
              }
            ]
          },
          {
            id: 2564,
            parentId: 2552,
            name: '管理备份存储设备（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002164791716.html',
            children: [
              {
                id: 2583,
                parentId: 2564,
                name: '查看备份存储设备',
                local: 'zh-cn_topic_0000002164631992.html'
              },
              {
                id: 2584,
                parentId: 2564,
                name: '创建备份存储设备',
                local: 'zh-cn_topic_0000002200032757.html'
              },
              {
                id: 2585,
                parentId: 2564,
                name: '修改备份存储设备',
                local: 'zh-cn_topic_0000002200118341.html'
              },
              {
                id: 2586,
                parentId: 2564,
                name: '删除备份存储设备',
                local: 'zh-cn_topic_0000002200118365.html'
              },
              {
                id: 2587,
                parentId: 2564,
                name: '备份存储设备升级为成员节点',
                local: 'zh-cn_topic_0000002200032737.html'
              }
            ]
          },
          {
            id: 2565,
            parentId: 2552,
            name: '管理备份存储单元（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002200032761.html',
            children: [
              {
                id: 2588,
                parentId: 2565,
                name: '查看备份存储单元',
                local: 'zh-cn_topic_0000002200032765.html'
              },
              {
                id: 2589,
                parentId: 2565,
                name: '创建备份存储单元',
                local: 'zh-cn_topic_0000002200118325.html'
              },
              {
                id: 2590,
                parentId: 2565,
                name: '修改备份存储单元',
                local: 'zh-cn_topic_0000002200118349.html'
              },
              {
                id: 2591,
                parentId: 2565,
                name: '删除备份存储单元',
                local: 'zh-cn_topic_0000002200032781.html'
              }
            ]
          },
          {
            id: 2566,
            parentId: 2552,
            name: '管理内部通信网络平面（适用于1.5.0版本）',
            local: 'zh-cn_topic_0000002164632048.html',
            children: [
              {
                id: 2592,
                parentId: 2566,
                name: '修改内部通信网络平面',
                local: 'zh-cn_topic_0000002164632044.html'
              },
              {
                id: 2593,
                parentId: 2566,
                name: '删除内部通信网络平面',
                local: 'zh-cn_topic_0000002164791760.html'
              }
            ]
          },
          {
            id: 2567,
            parentId: 2552,
            name: '管理内部通信网络（适用于1.6.0及后续版本）',
            local: 'zh-cn_topic_0000002200118333.html',
            children: [
              {
                id: 2594,
                parentId: 2567,
                name: '修改内部通信网络',
                local: 'zh-cn_topic_0000002164791744.html'
              },
              {
                id: 2595,
                parentId: 2567,
                name: '删除内部通信网络',
                local: 'zh-cn_topic_0000002200032745.html'
              }
            ]
          }
        ]
      },
      { id: 2553, parentId: 5, name: '常见问题', local: 'HA00035.html' }
    ]
  },
  {
    id: 6,
    parentId: 0,
    name: '监控',
    local: 'admin-00134.html',
    children: [
      {
        id: 2596,
        parentId: 6,
        name: '管理性能统计',
        local: 'admin-00135.html',
        children: [
          {
            id: 2600,
            parentId: 2596,
            name: '性能指标介绍',
            local: 'admin-00136.html'
          },
          {
            id: 2601,
            parentId: 2596,
            name: '配置性能统计开关',
            local: 'admin-00137.html'
          }
        ]
      },
      {
        id: 2597,
        parentId: 6,
        name: '管理告警和事件',
        local: 'admin-00139.html'
      },
      {
        id: 2598,
        parentId: 6,
        name: '管理任务',
        local: 'admin-00140.html',
        children: [
          {
            id: 2602,
            parentId: 2598,
            name: '查看任务进度',
            local: 'admin-00141.html'
          },
          {
            id: 2603,
            parentId: 2598,
            name: '停止任务',
            local: 'admin-00142.html'
          },
          {
            id: 2604,
            parentId: 2598,
            name: '下载任务',
            local: 'admin-00143.html'
          }
        ]
      },
      {
        id: 2599,
        parentId: 6,
        name: '管理报表',
        local: 'admin-00144.html',
        children: [
          {
            id: 2605,
            parentId: 2599,
            name: '用户角色权限',
            local: 'admin-00145.html'
          },
          {
            id: 2606,
            parentId: 2599,
            name: '创建报表',
            local: 'admin-00146.html'
          },
          {
            id: 2607,
            parentId: 2599,
            name: '查看报表',
            local: 'admin-00147.html'
          },
          {
            id: 2608,
            parentId: 2599,
            name: '下载报表',
            local: 'admin-00148.html'
          },
          {
            id: 2609,
            parentId: 2599,
            name: '发送邮件',
            local: 'admin-00149.html'
          },
          {
            id: 2610,
            parentId: 2599,
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
        id: 2611,
        parentId: 7,
        name: '管理用户（适用于1.5.0版本）',
        local: 'helpcenter_000159.html',
        children: [
          {
            id: 2636,
            parentId: 2611,
            name: '用户角色介绍',
            local: 'helpcenter_000160.html'
          },
          {
            id: 2637,
            parentId: 2611,
            name: '创建用户',
            local: 'helpcenter_000161.html'
          },
          {
            id: 2638,
            parentId: 2611,
            name: '修改用户',
            local: 'helpcenter_000162.html'
          },
          {
            id: 2639,
            parentId: 2611,
            name: '锁定用户',
            local: 'helpcenter_000163.html'
          },
          {
            id: 2640,
            parentId: 2611,
            name: '解锁用户',
            local: 'helpcenter_000164.html'
          },
          {
            id: 2641,
            parentId: 2611,
            name: '删除用户',
            local: 'helpcenter_000165.html'
          },
          {
            id: 2642,
            parentId: 2611,
            name: '重置用户密码',
            local: 'helpcenter_000166.html'
          },
          {
            id: 2643,
            parentId: 2611,
            name: '重置系统管理员密码',
            local: 'helpcenter_000167.html'
          }
        ]
      },
      {
        id: 2612,
        parentId: 7,
        name: '管理RBAC（适用于1.6.0及后续版本）',
        local: 'admin-0055.html',
        children: [
          {
            id: 2644,
            parentId: 2612,
            name: '内置用户角色介绍',
            local: 'admin-0056.html'
          },
          {
            id: 2645,
            parentId: 2612,
            name: '创建角色',
            local: 'zh-cn_topic_0000002164788864.html'
          },
          {
            id: 2646,
            parentId: 2612,
            name: '修改角色',
            local: 'zh-cn_topic_0000002164629136.html'
          },
          {
            id: 2647,
            parentId: 2612,
            name: '克隆角色',
            local: 'zh-cn_topic_0000002200115361.html'
          },
          {
            id: 2648,
            parentId: 2612,
            name: '删除角色',
            local: 'zh-cn_topic_0000002164629024.html'
          },
          {
            id: 2649,
            parentId: 2612,
            name: '创建资源集',
            local: 'zh-cn_topic_0000002164629172.html'
          },
          {
            id: 2650,
            parentId: 2612,
            name: '删除资源集',
            local: 'zh-cn_topic_0000002200029905.html'
          },
          {
            id: 2651,
            parentId: 2612,
            name: '修改资源集',
            local: 'zh-cn_topic_0000002200115373.html'
          },
          {
            id: 2652,
            parentId: 2612,
            name: '创建用户',
            local: 'admin-0057.html'
          },
          {
            id: 2653,
            parentId: 2612,
            name: '修改用户',
            local: 'admin-0058.html'
          },
          {
            id: 2654,
            parentId: 2612,
            name: '锁定用户',
            local: 'admin-0059.html'
          },
          {
            id: 2655,
            parentId: 2612,
            name: '解锁用户',
            local: 'admin-0060.html'
          },
          {
            id: 2656,
            parentId: 2612,
            name: '删除用户',
            local: 'admin-0061.html'
          },
          {
            id: 2657,
            parentId: 2612,
            name: '重置用户密码',
            local: 'admin-0062.html'
          },
          {
            id: 2658,
            parentId: 2612,
            name: '重置系统管理员密码',
            local: 'admin-0063.html'
          },
          {
            id: 2659,
            parentId: 2612,
            name: '找回密码邮箱设置',
            local: 'admin-0064.html'
          }
        ]
      },
      {
        id: 2613,
        parentId: 7,
        name: '管理SAML SSO配置',
        local: 'zh-cn_topic_0000002164629040.html',
        children: [
          {
            id: 2660,
            parentId: 2613,
            name: '创建SAML SSO配置',
            local: 'zh-cn_topic_0000002164629160.html'
          },
          {
            id: 2661,
            parentId: 2613,
            name: '管理SAML SSO 配置',
            local: 'zh-cn_topic_0000002164788780.html',
            children: [
              {
                id: 2663,
                parentId: 2661,
                name: '激活/禁用SAML SSO 配置',
                local: 'zh-cn_topic_0000002200029877.html'
              },
              {
                id: 2664,
                parentId: 2661,
                name: '修改SAML SSO配置',
                local: 'zh-cn_topic_0000002164629200.html'
              },
              {
                id: 2665,
                parentId: 2661,
                name: '删除SAML SSO配置',
                local: 'zh-cn_topic_0000002200115341.html'
              }
            ]
          },
          {
            id: 2662,
            parentId: 2613,
            name: '导出元数据',
            local: 'zh-cn_topic_0000002200029953.html'
          }
        ]
      },
      {
        id: 2614,
        parentId: 7,
        name: '管理配额与功能',
        local: 'zh-cn_topic_0000002164788888.html',
        children: [
          {
            id: 2666,
            parentId: 2614,
            name: '查看配额与功能',
            local: 'zh-cn_topic_0000002164629072.html'
          },
          {
            id: 2667,
            parentId: 2614,
            name: '设置配额',
            local: 'zh-cn_topic_0000002164629148.html'
          },
          {
            id: 2668,
            parentId: 2614,
            name: '设置功能（适用于1.5.0版本）',
            local: 'zh-cn_topic_0000002164788784.html'
          }
        ]
      },
      {
        id: 2615,
        parentId: 7,
        name: '管理备份集群（用于部分型号）',
        local: 'admin-00067.html',
        children: [
          {
            id: 2669,
            parentId: 2615,
            name: '管理本地集群节点',
            local: 'zh-cn_topic_0000002200115329.html',
            children: [
              {
                id: 2671,
                parentId: 2669,
                name: '查看本地集群节点',
                local: 'zh-cn_topic_0000002200115517.html'
              },
              {
                id: 2672,
                parentId: 2669,
                name: '管理备节点/成员节点',
                local: 'zh-cn_topic_0000002164788760.html',
                children: [
                  {
                    id: 2673,
                    parentId: 2672,
                    name: '修改备节点/成员节点',
                    local: 'zh-cn_topic_0000002200115357.html'
                  },
                  {
                    id: 2674,
                    parentId: 2672,
                    name: '删除成员节点',
                    local: 'zh-cn_topic_0000002164629060.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2670,
            parentId: 2615,
            name: '管理多域集群',
            local: 'zh-cn_topic_0000002164788848.html',
            children: [
              {
                id: 2675,
                parentId: 2670,
                name: '查看集群信息',
                local: 'admin-00068.html'
              },
              {
                id: 2676,
                parentId: 2670,
                name: '添加外部集群',
                local: 'admin-00069.html'
              },
              {
                id: 2677,
                parentId: 2670,
                name: '修改外部集群信息',
                local: 'admin-00070.html'
              },
              {
                id: 2678,
                parentId: 2670,
                name: '删除外部集群',
                local: 'admin-00071.html'
              },
              {
                id: 2679,
                parentId: 2670,
                name: '指定外部集群为管理集群',
                local: 'admin-00072.html'
              },
              {
                id: 2680,
                parentId: 2670,
                name: '指定外部集群为被管理集群',
                local: 'admin-00073.html'
              },
              {
                id: 2681,
                parentId: 2670,
                name: '授权',
                local: 'admin-00074.html'
              },
              {
                id: 2682,
                parentId: 2670,
                name: '修改本地集群数据保护管理员的授权',
                local: 'admin-00075.html'
              },
              {
                id: 2683,
                parentId: 2670,
                name: '取消本地集群数据保护管理员的授权',
                local: 'admin-00076.html'
              }
            ]
          }
        ]
      },
      {
        id: 2616,
        parentId: 7,
        name: '管理复制集群',
        local: 'zh-cn_topic_0000002164788900.html',
        children: [
          {
            id: 2684,
            parentId: 2616,
            name: '添加外部集群',
            local: 'zh-cn_topic_0000002164629128.html'
          },
          {
            id: 2685,
            parentId: 2616,
            name: '查看集群信息',
            local: 'zh-cn_topic_0000002164788928.html'
          },
          {
            id: 2686,
            parentId: 2616,
            name: '修改复制集群',
            local: 'zh-cn_topic_0000002164629096.html'
          },
          {
            id: 2687,
            parentId: 2616,
            name: '删除复制集群',
            local: 'zh-cn_topic_0000002164629184.html'
          }
        ]
      },
      {
        id: 2617,
        parentId: 7,
        name: '管理本地存储',
        local: 'admin-00078.html',
        children: [
          {
            id: 2688,
            parentId: 2617,
            name: '查看本地存储信息',
            local: 'admin-00079.html'
          },
          {
            id: 2689,
            parentId: 2617,
            name: '配置本地存储容量告警阈值',
            local: 'admin-00080.html'
          },
          {
            id: 2690,
            parentId: 2617,
            name: '查看本地存储认证信息',
            local: 'admin-00081.html'
          },
          {
            id: 2691,
            parentId: 2617,
            name: '修改本地存储认证信息',
            local: 'admin-00082.html'
          },
          {
            id: 2692,
            parentId: 2617,
            name: '手动回收空间',
            local: 'admin-00087.html'
          }
        ]
      },
      {
        id: 2618,
        parentId: 7,
        name: '管理对象存储',
        local: 'helpcenter_000132.html',
        children: [
          {
            id: 2693,
            parentId: 2618,
            name: '添加归档存储',
            local: 'oracle_gud_000030.html'
          },
          {
            id: 2694,
            parentId: 2618,
            name: '导入归档存储副本',
            local: 'helpcenter_000134.html'
          },
          {
            id: 2695,
            parentId: 2618,
            name: '修改归档存储基本信息',
            local: 'helpcenter_000135.html'
          },
          {
            id: 2696,
            parentId: 2618,
            name: '修改归档存储容量告警阈值',
            local: 'helpcenter_000136.html'
          },
          {
            id: 2697,
            parentId: 2618,
            name: '查看归档存储信息',
            local: 'helpcenter_000137.html'
          },
          {
            id: 2698,
            parentId: 2618,
            name: '删除归档存储',
            local: 'helpcenter_000138.html'
          }
        ]
      },
      {
        id: 2619,
        parentId: 7,
        name: '管理磁带（适用于部分型号）',
        local: 'helpcenter_000139.html',
        children: [
          {
            id: 2699,
            parentId: 2619,
            name: '管理磁带库',
            local: 'helpcenter_000140.html',
            children: [
              {
                id: 2701,
                parentId: 2699,
                name: '扫描磁带库',
                local: 'helpcenter_000141.html'
              },
              {
                id: 2702,
                parentId: 2699,
                name: '管理驱动',
                local: 'helpcenter_000142.html',
                children: [
                  {
                    id: 2704,
                    parentId: 2702,
                    name: '查看驱动',
                    local: 'helpcenter_000143.html'
                  },
                  {
                    id: 2705,
                    parentId: 2702,
                    name: '启用驱动',
                    local: 'helpcenter_000144.html'
                  },
                  {
                    id: 2706,
                    parentId: 2702,
                    name: '禁用驱动',
                    local: 'helpcenter_000145.html'
                  }
                ]
              },
              {
                id: 2703,
                parentId: 2699,
                name: '管理磁带',
                local: 'helpcenter_000146.html',
                children: [
                  {
                    id: 2707,
                    parentId: 2703,
                    name: '查看磁带',
                    local: 'helpcenter_000147.html'
                  },
                  {
                    id: 2708,
                    parentId: 2703,
                    name: '加载磁带',
                    local: 'helpcenter_000148.html'
                  },
                  {
                    id: 2709,
                    parentId: 2703,
                    name: '卸载磁带',
                    local: 'helpcenter_000149.html'
                  },
                  {
                    id: 2710,
                    parentId: 2703,
                    name: '删除磁带',
                    local: 'helpcenter_000150.html'
                  },
                  {
                    id: 2711,
                    parentId: 2703,
                    name: '识别磁带',
                    local: 'helpcenter_000151.html'
                  },
                  {
                    id: 2712,
                    parentId: 2703,
                    name: '标记磁带为空',
                    local: 'helpcenter_000152.html'
                  },
                  {
                    id: 2713,
                    parentId: 2703,
                    name: '擦除磁带',
                    local: 'helpcenter_000153.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2700,
            parentId: 2619,
            name: '管理介质集',
            local: 'helpcenter_000154.html',
            children: [
              {
                id: 2714,
                parentId: 2700,
                name: '创建介质集',
                local: 'helpcenter_000155.html'
              },
              {
                id: 2715,
                parentId: 2700,
                name: '查看介质集',
                local: 'helpcenter_000156.html'
              },
              {
                id: 2716,
                parentId: 2700,
                name: '修改介质集',
                local: 'helpcenter_000157.html'
              },
              {
                id: 2717,
                parentId: 2700,
                name: '删除介质集',
                local: 'helpcenter_000158.html'
              }
            ]
          }
        ]
      },
      {
        id: 2620,
        parentId: 7,
        name: '查看系统信息（适用于部分型号）',
        local: 'informix_gud_00040.html',
        children: [
          {
            id: 2718,
            parentId: 2620,
            name: '查看系统版本信息',
            local: 'informix_gud_00041.html'
          },
          {
            id: 2719,
            parentId: 2620,
            name: '查看设备ESN',
            local: 'informix_gud_00042.html'
          }
        ]
      },
      {
        id: 2621,
        parentId: 7,
        name: '管理安全策略',
        local: 'helpcenter_000168.html'
      },
      {
        id: 2622,
        parentId: 7,
        name: '管理证书',
        local: 'helpcenter_000169.html',
        children: [
          {
            id: 2720,
            parentId: 2622,
            name: '查看证书信息',
            local: 'helpcenter_000170.html'
          },
          {
            id: 2721,
            parentId: 2622,
            name: '添加外部证书',
            local: 'helpcenter_000171.html'
          },
          {
            id: 2722,
            parentId: 2622,
            name: '导入证书',
            local: 'helpcenter_000172.html'
          },
          {
            id: 2723,
            parentId: 2622,
            name: '导出请求文件',
            local: 'helpcenter_000173.html'
          },
          {
            id: 2724,
            parentId: 2622,
            name: '修改证书过期告警',
            local: 'helpcenter_000174.html'
          },
          {
            id: 2725,
            parentId: 2622,
            name: '管理证书吊销列表',
            local: 'helpcenter_000176.html',
            children: [
              {
                id: 2728,
                parentId: 2725,
                name: '导入证书吊销列表',
                local: 'helpcenter_000177.html'
              },
              {
                id: 2729,
                parentId: 2725,
                name: '查看证书吊销列表',
                local: 'helpcenter_000178.html'
              },
              {
                id: 2730,
                parentId: 2725,
                name: '下载证书吊销列表',
                local: 'helpcenter_000179.html'
              },
              {
                id: 2731,
                parentId: 2725,
                name: '删除证书吊销列表',
                local: 'helpcenter_000180.html'
              }
            ]
          },
          {
            id: 2726,
            parentId: 2622,
            name: '下载证书',
            local: 'helpcenter_000181.html'
          },
          {
            id: 2727,
            parentId: 2622,
            name: '删除外部证书',
            local: 'helpcenter_000182.html'
          }
        ]
      },
      {
        id: 2623,
        parentId: 7,
        name: '管理主机受信',
        local: 'helpcenter_000183.html'
      },
      {
        id: 2624,
        parentId: 7,
        name: '管理日志',
        local: 'helpcenter_000184.html'
      },
      {
        id: 2625,
        parentId: 7,
        name: '导出查询',
        local: 'zh-cn_topic_0000002200029737.html'
      },
      {
        id: 2626,
        parentId: 7,
        name: '管理系统数据备份',
        local: 'helpcenter_000185.html',
        children: [
          {
            id: 2732,
            parentId: 2626,
            name: '配置管理数据备份',
            local: 'helpcenter_000186.html'
          },
          {
            id: 2733,
            parentId: 2626,
            name: '导出管理数据备份',
            local: 'helpcenter_000187.html'
          },
          {
            id: 2734,
            parentId: 2626,
            name: '删除管理数据备份',
            local: 'helpcenter_000188.html'
          },
          {
            id: 2735,
            parentId: 2626,
            name: '导入管理数据备份',
            local: 'helpcenter_000189.html'
          },
          {
            id: 2736,
            parentId: 2626,
            name: '恢复管理数据',
            local: 'helpcenter_000190.html'
          }
        ]
      },
      {
        id: 2627,
        parentId: 7,
        name: '管理邮件服务',
        local: 'helpcenter_000191.html'
      },
      {
        id: 2628,
        parentId: 7,
        name: '管理事件转储（适用于部分型号）',
        local: 'helpcenter_000192.html'
      },
      {
        id: 2629,
        parentId: 7,
        name: '在本产品管理界面配置SNMP Trap通知',
        local: 'helpcenter_000193.html'
      },
      {
        id: 2630,
        parentId: 7,
        name: '管理SFTP服务（适用于1.5.0版本）（适用于部分型号）',
        local: 'helpcenter_000194.html',
        children: [
          {
            id: 2737,
            parentId: 2630,
            name: '开启SFTP服务',
            local: 'helpcenter_000195.html'
          },
          {
            id: 2738,
            parentId: 2630,
            name: '查看SFTP服务',
            local: 'helpcenter_000196.html'
          },
          {
            id: 2739,
            parentId: 2630,
            name: '创建SFTP用户',
            local: 'helpcenter_000197.html'
          },
          {
            id: 2740,
            parentId: 2630,
            name: '修改SFTP用户密码',
            local: 'helpcenter_000198.html'
          },
          {
            id: 2741,
            parentId: 2630,
            name: '删除SFTP用户',
            local: 'helpcenter_000199.html'
          }
        ]
      },
      {
        id: 2631,
        parentId: 7,
        name: '管理SFTP服务（适用于1.6.0及后续版本）（适用于部分型号）',
        local: 'admin-00261.html',
        children: [
          {
            id: 2742,
            parentId: 2631,
            name: '开启SFTP服务',
            local: 'admin-00262.html'
          },
          {
            id: 2743,
            parentId: 2631,
            name: '查看SFTP服务',
            local: 'admin-00263.html'
          },
          {
            id: 2744,
            parentId: 2631,
            name: '创建SFTP用户',
            local: 'admin-00264.html'
          },
          {
            id: 2745,
            parentId: 2631,
            name: '修改SFTP用户密码',
            local: 'admin-00265.html'
          },
          {
            id: 2746,
            parentId: 2631,
            name: '删除SFTP用户',
            local: 'admin-00266.html'
          }
        ]
      },
      {
        id: 2632,
        parentId: 7,
        name: '管理设备时间（适用于部分型号）',
        local: 'helpcenter_000200.html'
      },
      {
        id: 2633,
        parentId: 7,
        name: '配置LDAP服务',
        local: 'zh-cn_topic_0000002200029741.html'
      },
      {
        id: 2634,
        parentId: 7,
        name: '管理Windows ADFS配置（适用于1.6.0及后续版本）',
        local: 'admin-0077.html'
      },
      {
        id: 2635,
        parentId: 7,
        name: '管理备份软件纳管（适用于1.6.0及后续版本）',
        local: 'zh-cn_topic_0000002200115393.html',
        children: [
          {
            id: 2747,
            parentId: 2635,
            name: '添加备份软件纳管',
            local: 'zh-cn_topic_0000002164628984.html'
          },
          {
            id: 2748,
            parentId: 2635,
            name: '修改备份软件纳管',
            local: 'zh-cn_topic_0000002164788876.html'
          },
          {
            id: 2749,
            parentId: 2635,
            name: '删除备份软件纳管',
            local: 'zh-cn_topic_0000002164788700.html'
          }
        ]
      }
    ]
  }
];
topLanguage = 'zh';
topMainPage = 'helpcenter_000001.html';

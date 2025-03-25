naviData = [
  {
    id: 1,
    parentId: 0,
    name: 'Overview',
    local: 'helpcenter000001.html',
    children: [
      {
        id: 8,
        parentId: 1,
        name: 'About the OceanProtect',
        local: 'helpcenter000002.html'
      },
      {
        id: 9,
        parentId: 1,
        name: 'Quick Start',
        local: 'helpcenter000003.html'
      },
      {
        id: 10,
        parentId: 1,
        name: 'Personal Data Privacy Statement',
        local: 'helpcenter000004.html'
      }
    ]
  },
  { id: 2, parentId: 0, name: 'Home Page', local: 'helpcenter000005.html' },
  {
    id: 3,
    parentId: 0,
    name: 'Protection',
    local: 'helpcenter000006.html',
    children: [
      { id: 11, parentId: 3, name: 'Overview', local: 'helpcenter000007.html' },
      {
        id: 12,
        parentId: 3,
        name: 'Hosts',
        local: 'helpcenter000008.html',
        children: [
          {
            id: 20,
            parentId: 12,
            name:
              'Installing ProtectAgent (Automatic Push Mode, Applicable to 1.6.0 and Later Versions)',
            local: 'protectagent_install_0017.html'
          },
          {
            id: 21,
            parentId: 12,
            name: 'Managing the ProtectAgent Software Package',
            local: 'protectagent_install_0028.html',
            children: [
              {
                id: 23,
                parentId: 21,
                name: 'Uploading the ProtectAgent Software Package',
                local: 'protectagent_install_0029.html'
              },
              {
                id: 24,
                parentId: 21,
                name: 'Downloading the ProtectAgent Software Package',
                local: 'protectagent_install_0030.html'
              }
            ]
          },
          {
            id: 22,
            parentId: 12,
            name: 'Managing a ProtectAgent Host',
            local: 'protectagent_install_0031.html',
            children: [
              {
                id: 25,
                parentId: 22,
                name: 'Viewing Agent Host Information',
                local: 'protectagent_install_0032.html'
              },
              {
                id: 26,
                parentId: 22,
                name: 'Managing an Agent Host',
                local: 'protectagent_install_0033.html'
              }
            ]
          }
        ]
      },
      {
        id: 13,
        parentId: 3,
        name: 'Databases',
        local: 'en-us_topic_0000001918630660.html',
        children: [
          {
            id: 27,
            parentId: 13,
            name: 'Oracle Data Protection',
            local: 'product_documentation_000025.html',
            children: [
              {
                id: 44,
                parentId: 27,
                name: 'Backup',
                local: 'oracle_gud_0008.html',
                children: [
                  {
                    id: 53,
                    parentId: 44,
                    name: 'Preparations for Backup',
                    local: 'oracle_gud_0012.html'
                  },
                  {
                    id: 54,
                    parentId: 44,
                    name: 'Backing Up an Oracle Database',
                    local: 'oracle_gud_0013.html',
                    children: [
                      {
                        id: 56,
                        parentId: 54,
                        name:
                          'Step 1: Checking and Configuring the Database Environment',
                        local: 'oracle_gud_0015.html',
                        children: [
                          {
                            id: 65,
                            parentId: 56,
                            name:
                              'Checking and Configuring the Open State of the Oracle Database',
                            local: 'oracle_gud_0016.html'
                          },
                          {
                            id: 66,
                            parentId: 56,
                            name:
                              'Checking and Configuring the Archive Mode of the Oracle Database',
                            local: 'oracle_gud_0017.html'
                          },
                          {
                            id: 67,
                            parentId: 56,
                            name: 'Checking the Snapshot Control File Location',
                            local: 'oracle_gud_0020.html'
                          },
                          {
                            id: 68,
                            parentId: 56,
                            name:
                              'Checking the Storage Location of Cluster Data',
                            local: 'oracle_gud_00201.html'
                          }
                        ]
                      },
                      {
                        id: 57,
                        parentId: 54,
                        name:
                          'Step 2: Obtaining the CA Certificate of the Storage Resource (Applicable to Snapshot-based Backup at the Storage Layer)',
                        local: 'oracle_gud_ca.html'
                      },
                      {
                        id: 58,
                        parentId: 54,
                        name: 'Step 3: Registering a Cluster',
                        local: 'oracle_gud_0022.html'
                      },
                      {
                        id: 59,
                        parentId: 54,
                        name: 'Step 4: Registering a Database',
                        local: 'oracle_gud_0023.html'
                      },
                      {
                        id: 60,
                        parentId: 54,
                        name:
                          'Step 5: (Optional) Creating a Rate Limiting Policy',
                        local: 'oracle_gud_0024.html'
                      },
                      {
                        id: 61,
                        parentId: 54,
                        name:
                          'Step 6: (Optional) Enabling Backup Link Encryption',
                        local: 'oracle_gud_0025.html'
                      },
                      {
                        id: 62,
                        parentId: 54,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'oracle_gud_0026.html'
                      },
                      {
                        id: 63,
                        parentId: 54,
                        name:
                          'Step 8: Enabling BCT (Applicable to RMAN-based Backup)',
                        local: 'oracle_gud_0027.html'
                      },
                      {
                        id: 64,
                        parentId: 54,
                        name: 'Step 9: Performing Backup',
                        local: 'oracle_gud_0028.html'
                      }
                    ]
                  },
                  {
                    id: 55,
                    parentId: 44,
                    name:
                      '(Optional) Synchronizing Trap Configurations to an Oracle Host',
                    local: 'oracle_gud_0031.html'
                  }
                ]
              },
              {
                id: 45,
                parentId: 27,
                name: 'Replication',
                local: 'oracle_gud_0032.html',
                children: [
                  {
                    id: 69,
                    parentId: 45,
                    name: 'Replicating an Oracle Database Copy',
                    local: 'oracle_gud_0034.html',
                    children: [
                      {
                        id: 70,
                        parentId: 69,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'oracle_gud_0036.html'
                      },
                      {
                        id: 71,
                        parentId: 69,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'oracle_gud_0036_1.html'
                      },
                      {
                        id: 72,
                        parentId: 69,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'oracle_gud_0037.html'
                      },
                      {
                        id: 73,
                        parentId: 69,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'oracle_gud_0038.html'
                      },
                      {
                        id: 74,
                        parentId: 69,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'oracle_gud_0039.html'
                      },
                      {
                        id: 75,
                        parentId: 69,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'vmware_gud_0040.html'
                      },
                      {
                        id: 76,
                        parentId: 69,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002097717489.html'
                      },
                      {
                        id: 77,
                        parentId: 69,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'oracle_gud_0041.html'
                      },
                      {
                        id: 78,
                        parentId: 69,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'oracle_gud_0042.html'
                      },
                      {
                        id: 79,
                        parentId: 69,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'oracle_gud_0043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 46,
                parentId: 27,
                name: 'Archiving',
                local: 'oracle_gud_0044.html',
                children: [
                  {
                    id: 80,
                    parentId: 46,
                    name: 'Archiving Oracle Backup Copies',
                    local: 'oracle_gud_0047.html',
                    children: [
                      {
                        id: 82,
                        parentId: 80,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'oracle_gud_0048.html',
                        children: [
                          {
                            id: 84,
                            parentId: 82,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'oracle_gud_0049.html'
                          },
                          {
                            id: 85,
                            parentId: 82,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'oracle_gud_0050.html'
                          }
                        ]
                      },
                      {
                        id: 83,
                        parentId: 80,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'oracle_gud_0051.html'
                      }
                    ]
                  },
                  {
                    id: 81,
                    parentId: 46,
                    name: 'Archiving Oracle Replication Copies',
                    local: 'oracle_gud_0052.html',
                    children: [
                      {
                        id: 86,
                        parentId: 81,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'oracle_gud_0053.html'
                      },
                      {
                        id: 87,
                        parentId: 81,
                        name:
                          'Step 2: Creating Periodic Archives of Replication Copies',
                        local: 'oracle_gud_0054.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 47,
                parentId: 27,
                name: 'Restoration',
                local: 'oracle_gud_0055.html',
                children: [
                  {
                    id: 88,
                    parentId: 47,
                    name: 'Restoring an Oracle Database',
                    local: 'oracle_gud_0058.html'
                  },
                  {
                    id: 89,
                    parentId: 47,
                    name:
                      'Restoring a Single Table or Multiple Tables in an Oracle Database (Applicable to 1.6.0 and Later Versions)',
                    local: 'oracle_gud_0131.html'
                  },
                  {
                    id: 90,
                    parentId: 47,
                    name:
                      'Restoring a Single File or Multiple Files in an Oracle Database (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002028898929.html'
                  }
                ]
              },
              {
                id: 48,
                parentId: 27,
                name: 'Instant Recovery',
                local: 'oracle_gud_0059.html',
                children: [
                  {
                    id: 91,
                    parentId: 48,
                    name: 'Restoring an Oracle Database Instantly',
                    local: 'oracle_gud_0062.html'
                  }
                ]
              },
              {
                id: 49,
                parentId: 27,
                name: 'Global Search',
                local: 'oracle_gud_0072.html',
                children: [
                  {
                    id: 92,
                    parentId: 49,
                    name: 'Global Search for Resources',
                    local: 'oracle_gud_0073.html'
                  },
                  {
                    id: 93,
                    parentId: 49,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002038747309.html'
                  }
                ]
              },
              {
                id: 50,
                parentId: 27,
                name: 'SLA ',
                local: 'oracle_gud_0076.html',
                children: [
                  {
                    id: 94,
                    parentId: 50,
                    name: 'Viewing SLA Information',
                    local: 'oracle_gud_0078.html'
                  },
                  {
                    id: 95,
                    parentId: 50,
                    name: 'Managing SLAs',
                    local: 'oracle_gud_0079.html'
                  }
                ]
              },
              {
                id: 51,
                parentId: 27,
                name: 'Copies',
                local: 'oracle_gud_0080.html',
                children: [
                  {
                    id: 96,
                    parentId: 51,
                    name: 'Viewing Oracle Copy Information',
                    local: 'oracle_gud_0081.html'
                  },
                  {
                    id: 97,
                    parentId: 51,
                    name: 'Managing Oracle Copies',
                    local: 'oracle_gud_0082.html'
                  }
                ]
              },
              {
                id: 52,
                parentId: 27,
                name: 'Oracle Database Environment',
                local: 'oracle_gud_0083.html',
                children: [
                  {
                    id: 98,
                    parentId: 52,
                    name: 'Viewing the Oracle Database Environment',
                    local: 'oracle_gud_0084.html'
                  },
                  {
                    id: 99,
                    parentId: 52,
                    name: 'Managing Databases',
                    local: 'oracle_gud_0085.html'
                  },
                  {
                    id: 100,
                    parentId: 52,
                    name: 'Managing Database Clusters',
                    local: 'oracle_gud_0086.html'
                  }
                ]
              }
            ]
          },
          {
            id: 28,
            parentId: 13,
            name: 'MySQL/MariaDB/GreatSQL Data Protection',
            local: 'en-us_topic_0000001826879872.html',
            children: [
              {
                id: 101,
                parentId: 28,
                name: 'Backup',
                local: 'mysql-0007.html',
                children: [
                  {
                    id: 109,
                    parentId: 101,
                    name: 'Preparations for Backup',
                    local: 'mysql-0010.html'
                  },
                  {
                    id: 110,
                    parentId: 101,
                    name: 'Backing Up the MySQL/MariaDB/GreatSQL Database',
                    local: 'mysql-0011.html',
                    children: [
                      {
                        id: 111,
                        parentId: 110,
                        name:
                          'Step 1: Enabling MySQL/MariaDB/GreatSQL Database Permissions',
                        local: 'mysql-0012.html'
                      },
                      {
                        id: 112,
                        parentId: 110,
                        name: 'Step 2: Manually Configuring Soft Links',
                        local: 'mysql-0016.html'
                      },
                      {
                        id: 113,
                        parentId: 110,
                        name: 'Step 3: Manually Installing the Backup Tool',
                        local: 'mysql-0013.html',
                        children: [
                          {
                            id: 120,
                            parentId: 113,
                            name: 'Installing Mariabackup',
                            local: 'mysql-0014.html'
                          },
                          {
                            id: 121,
                            parentId: 113,
                            name:
                              'Installing the Software on Which the Percona XtraBackup Tool Depends',
                            local: 'mysql-0015.html'
                          }
                        ]
                      },
                      {
                        id: 114,
                        parentId: 110,
                        name:
                          'Step 4: Enabling Log Mode for the MySQL/MariaDB/GreatSQL Database',
                        local: 'mysql-0017.html'
                      },
                      {
                        id: 115,
                        parentId: 110,
                        name:
                          'Step 5: Registering a MySQL/MariaDB/GreatSQL Database',
                        local: 'mysql-0018.html'
                      },
                      {
                        id: 116,
                        parentId: 110,
                        name: 'Step 6: Creating a Rate Limiting Policy',
                        local: 'mysql-0019.html'
                      },
                      {
                        id: 117,
                        parentId: 110,
                        name:
                          'Step 7: (Optional) Enabling Backup Link Encryption',
                        local: 'mysql-0020.html'
                      },
                      {
                        id: 118,
                        parentId: 110,
                        name: 'Step 8: Creating a Backup SLA',
                        local: 'mysql-0021.html'
                      },
                      {
                        id: 119,
                        parentId: 110,
                        name: 'Step 9: Performing Backup',
                        local: 'mysql-0022.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 102,
                parentId: 28,
                name: 'Replication',
                local: 'mysql-00241.html',
                children: [
                  {
                    id: 122,
                    parentId: 102,
                    name: 'Replicating MySQL/MariaDB/GreatSQL Database Copies',
                    local: 'mysql-0028.html',
                    children: [
                      {
                        id: 123,
                        parentId: 122,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'mysql-00291.html'
                      },
                      {
                        id: 124,
                        parentId: 122,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'mysql-00292.html'
                      },
                      {
                        id: 125,
                        parentId: 122,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'mysql-0031.html'
                      },
                      {
                        id: 126,
                        parentId: 122,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'mysql-0032.html'
                      },
                      {
                        id: 127,
                        parentId: 122,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'mysql-0033.html'
                      },
                      {
                        id: 128,
                        parentId: 122,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'mysql-0034.html'
                      },
                      {
                        id: 129,
                        parentId: 122,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'mysql-00311.html'
                      },
                      {
                        id: 130,
                        parentId: 122,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'mysql-0035.html'
                      },
                      {
                        id: 131,
                        parentId: 122,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'mysql-0036.html'
                      },
                      {
                        id: 132,
                        parentId: 122,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'mysql-0037.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 103,
                parentId: 28,
                name: 'Archiving',
                local: 'mysql-0038.html',
                children: [
                  {
                    id: 133,
                    parentId: 103,
                    name: 'Archiving MySQL/MariaDB/GreatSQL Backup Copies',
                    local: 'mysql-0041.html',
                    children: [
                      {
                        id: 135,
                        parentId: 133,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'mysql-0042.html',
                        children: [
                          {
                            id: 137,
                            parentId: 135,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'mysql-0043.html'
                          },
                          {
                            id: 138,
                            parentId: 135,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'mysql-0044.html'
                          }
                        ]
                      },
                      {
                        id: 136,
                        parentId: 133,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'mysql-0045.html'
                      }
                    ]
                  },
                  {
                    id: 134,
                    parentId: 103,
                    name: 'Archiving MySQL/MariaDB/GreatSQL Replication Copies',
                    local: 'mysql-0046.html',
                    children: [
                      {
                        id: 139,
                        parentId: 134,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'mysql-0047.html'
                      },
                      {
                        id: 140,
                        parentId: 134,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'mysql-0048.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 104,
                parentId: 28,
                name: 'Restoration',
                local: 'mysql-0049.html',
                children: [
                  {
                    id: 141,
                    parentId: 104,
                    name: 'Restoring MySQL/MariaDB/GreatSQL Databases',
                    local: 'mysql-0052.html'
                  }
                ]
              },
              {
                id: 105,
                parentId: 28,
                name: 'Global Search',
                local: 'mysql-0053.html',
                children: [
                  {
                    id: 142,
                    parentId: 105,
                    name: 'Global Search for Resources',
                    local: 'mysql-0054.html'
                  },
                  {
                    id: 143,
                    parentId: 105,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'mysql-00522.html'
                  }
                ]
              },
              {
                id: 106,
                parentId: 28,
                name: 'SLA ',
                local: 'mysql-0070.html',
                children: [
                  {
                    id: 144,
                    parentId: 106,
                    name: 'About SLA',
                    local: 'mysql-0071.html'
                  },
                  {
                    id: 145,
                    parentId: 106,
                    name: 'Viewing SLA Information',
                    local: 'mysql-0072.html'
                  },
                  {
                    id: 146,
                    parentId: 106,
                    name: 'Managing SLAs',
                    local: 'mysql-0073.html'
                  }
                ]
              },
              {
                id: 107,
                parentId: 28,
                name: 'Copies',
                local: 'mysql-0074.html',
                children: [
                  {
                    id: 147,
                    parentId: 107,
                    name: 'Viewing MySQL/MariaDB/GreatSQL Copy Information',
                    local: 'mysql-0075.html'
                  },
                  {
                    id: 148,
                    parentId: 107,
                    name: 'Managing MySQL/MariaDB/GreatSQL Copies',
                    local: 'mysql-0076.html'
                  }
                ]
              },
              {
                id: 108,
                parentId: 28,
                name: 'MySQL/MariaDB/GreatSQL Database Environment',
                local: 'mysql-0077.html',
                children: [
                  {
                    id: 149,
                    parentId: 108,
                    name:
                      'Viewing MySQL/MariaDB/GreatSQL Database Environment Information',
                    local: 'mysql-0078.html'
                  },
                  {
                    id: 150,
                    parentId: 108,
                    name: 'Managing Databases',
                    local: 'mysql-0079.html'
                  },
                  {
                    id: 151,
                    parentId: 108,
                    name: 'Managing Database Instances',
                    local: 'mysql-00790.html'
                  },
                  {
                    id: 152,
                    parentId: 108,
                    name: 'Managing Database Clusters',
                    local: 'mysql-0080.html'
                  }
                ]
              }
            ]
          },
          {
            id: 29,
            parentId: 13,
            name: 'SQL Server Data Protection',
            local: 'en-us_topic_0000001826879832.html',
            children: [
              {
                id: 153,
                parentId: 29,
                name: 'Backup',
                local: 'sql-0005.html',
                children: [
                  {
                    id: 161,
                    parentId: 153,
                    name: 'Preparations for Backup',
                    local: 'sql-0008.html'
                  },
                  {
                    id: 162,
                    parentId: 153,
                    name: 'Backing Up a SQL Server Database',
                    local: 'sql-0009.html',
                    children: [
                      {
                        id: 163,
                        parentId: 162,
                        name:
                          'Step 1: Checking and Configuring the Database Environment',
                        local: 'sql-0010.html'
                      },
                      {
                        id: 164,
                        parentId: 162,
                        name: 'Step 2: Setting Windows PowerShell Permissions',
                        local: 'sql-0011.html'
                      },
                      {
                        id: 165,
                        parentId: 162,
                        name: 'Step 4: Enabling the sysadmin Permission',
                        local: 'sql-0013.html'
                      },
                      {
                        id: 166,
                        parentId: 162,
                        name:
                          'Step 5: Setting the Recovery Model for Log Backup',
                        local: 'sql-0014.html'
                      },
                      {
                        id: 167,
                        parentId: 162,
                        name: 'Step 3: Registering the SQL Server Database',
                        local: 'sql-0012.html'
                      },
                      {
                        id: 168,
                        parentId: 162,
                        name: 'Step 6: Creating a Rate Limiting Policy',
                        local: 'sql-0015.html'
                      },
                      {
                        id: 169,
                        parentId: 162,
                        name:
                          'Step 7: (Optional) Enabling Backup Link Encryption',
                        local: 'en-us_topic_0000002051554157.html'
                      },
                      {
                        id: 170,
                        parentId: 162,
                        name: 'Step 8: Creating a Backup SLA',
                        local: 'sql-0017.html'
                      },
                      {
                        id: 171,
                        parentId: 162,
                        name: 'Step 9: Performing Backup',
                        local: 'sql-0018.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 154,
                parentId: 29,
                name: 'Replication',
                local: 'oracle_gud_000035_9.html',
                children: [
                  {
                    id: 172,
                    parentId: 154,
                    name: 'Replicating SQL Server Database Copies',
                    local: 'sql-0023.html',
                    children: [
                      {
                        id: 173,
                        parentId: 172,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'fc_gud_0026_7.html'
                      },
                      {
                        id: 174,
                        parentId: 172,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_11.html'
                      },
                      {
                        id: 175,
                        parentId: 172,
                        name: 'Step 2: (Optional) Creating an IPsec Policy',
                        local: 'sql-0026.html'
                      },
                      {
                        id: 176,
                        parentId: 172,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'sql-0027.html'
                      },
                      {
                        id: 177,
                        parentId: 172,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'sql-0028.html'
                      },
                      {
                        id: 178,
                        parentId: 172,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'sql-0029.html'
                      },
                      {
                        id: 179,
                        parentId: 172,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002010400808.html'
                      },
                      {
                        id: 180,
                        parentId: 172,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'sql-0030.html'
                      },
                      {
                        id: 181,
                        parentId: 172,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'sql-0031.html'
                      },
                      {
                        id: 182,
                        parentId: 172,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'sql-0032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 155,
                parentId: 29,
                name: 'Archiving',
                local: 'sql-0033.html',
                children: [
                  {
                    id: 183,
                    parentId: 155,
                    name: 'Archiving SQL Server Backup Copies',
                    local: 'sql-0036.html',
                    children: [
                      {
                        id: 185,
                        parentId: 183,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'sql-0037.html',
                        children: [
                          {
                            id: 187,
                            parentId: 185,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'sql-0038.html'
                          },
                          {
                            id: 188,
                            parentId: 185,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'sql-0039.html'
                          }
                        ]
                      },
                      {
                        id: 186,
                        parentId: 183,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'sql-0040.html'
                      }
                    ]
                  },
                  {
                    id: 184,
                    parentId: 155,
                    name: 'Archiving SQL Server Replication Copies',
                    local: 'sql-0041.html',
                    children: [
                      {
                        id: 189,
                        parentId: 184,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'sql-0042.html'
                      },
                      {
                        id: 190,
                        parentId: 184,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'sql-0043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 156,
                parentId: 29,
                name: 'Restoration',
                local: 'sql-0044.html',
                children: [
                  {
                    id: 191,
                    parentId: 156,
                    name: 'Restoring an SQL Server Database',
                    local: 'sql-0047.html'
                  },
                  {
                    id: 192,
                    parentId: 156,
                    name:
                      'Restoring One or More Databases in an SQL Server Instance',
                    local: 'sql-0152.html'
                  }
                ]
              },
              {
                id: 157,
                parentId: 29,
                name: 'Global Search',
                local: 'sql-0048.html',
                children: [
                  {
                    id: 193,
                    parentId: 157,
                    name: 'About Global Search',
                    local: 'en-us_topic_0000002092410796.html'
                  },
                  {
                    id: 194,
                    parentId: 157,
                    name: 'Global Search for Copies',
                    local: 'vmware_gud_0076_3.html'
                  },
                  {
                    id: 195,
                    parentId: 157,
                    name: 'Global Search for Resources',
                    local: 'sql-0049.html'
                  },
                  {
                    id: 196,
                    parentId: 157,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002678126.html'
                  }
                ]
              },
              {
                id: 158,
                parentId: 29,
                name: 'SLA ',
                local: 'sql-0052.html',
                children: [
                  {
                    id: 197,
                    parentId: 158,
                    name: 'Viewing SLA Information',
                    local: 'sql-0054.html'
                  },
                  {
                    id: 198,
                    parentId: 158,
                    name: 'Managing SLAs',
                    local: 'sql-0055.html'
                  }
                ]
              },
              {
                id: 159,
                parentId: 29,
                name: 'Copies',
                local: 'sql-0056.html',
                children: [
                  {
                    id: 199,
                    parentId: 159,
                    name: 'Viewing SQL Server Copy Information',
                    local: 'sql-0057.html'
                  },
                  {
                    id: 200,
                    parentId: 159,
                    name: 'Managing SQL Server Database Copies',
                    local: 'sql-0058.html'
                  }
                ]
              },
              {
                id: 160,
                parentId: 29,
                name: 'SQL Server Database Environment',
                local: 'sql-0059.html',
                children: [
                  {
                    id: 201,
                    parentId: 160,
                    name:
                      'Viewing Environment Information of the SQL Server Database',
                    local: 'sql-0060.html'
                  },
                  {
                    id: 202,
                    parentId: 160,
                    name: 'Managing SQL Server',
                    local: 'sql-0061.html'
                  },
                  {
                    id: 203,
                    parentId: 160,
                    name: 'Managing SQL Server Database Clusters',
                    local: 'sql-0062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 30,
            parentId: 13,
            name: 'PostgreSQL Data Protection',
            local: 'en-us_topic_0000001826879840.html',
            children: [
              {
                id: 204,
                parentId: 30,
                name: 'Backup',
                local: 'postgresql-0006_a1.html',
                children: [
                  {
                    id: 212,
                    parentId: 204,
                    name: 'Preparations for Backup',
                    local: 'postgresql-0009_a1.html'
                  },
                  {
                    id: 213,
                    parentId: 204,
                    name: 'Backing Up PostgreSQL',
                    local: 'postgresql-0010-2.html',
                    children: [
                      {
                        id: 214,
                        parentId: 213,
                        name:
                          'Step 1: Checking and Enabling the sudo Permission of the PostgreSQL Database Installation User',
                        local: 'en-us_topic_0000001951390817.html'
                      },
                      {
                        id: 215,
                        parentId: 213,
                        name: 'Step 2: Enabling the Archive Mode',
                        local: 'postgresql-0010_0.html'
                      },
                      {
                        id: 216,
                        parentId: 213,
                        name:
                          'Step 3: Registering the Database of a Single PostgreSQL Instance',
                        local: 'postgresql-0011.html'
                      },
                      {
                        id: 217,
                        parentId: 213,
                        name:
                          'Step 4: Registering the Database of a PostgreSQL Cluster Instance',
                        local: 'postgresql-0012.html'
                      },
                      {
                        id: 218,
                        parentId: 213,
                        name: 'Step 5: Creating a Rate Limiting Policy',
                        local: 'postgresql-0013.html'
                      },
                      {
                        id: 219,
                        parentId: 213,
                        name:
                          'Step 6: (Optional) Enabling Backup Link Encryption',
                        local: 'postgresql-0014.html'
                      },
                      {
                        id: 220,
                        parentId: 213,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'postgresql-0015_a1.html'
                      },
                      {
                        id: 221,
                        parentId: 213,
                        name: 'Step 8: Performing Backup',
                        local: 'postgresql-0016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 205,
                parentId: 30,
                name: 'Replication',
                local: 'oracle_gud_000035_11.html',
                children: [
                  {
                    id: 222,
                    parentId: 205,
                    name: 'Replicating PostgreSQL Database Copies',
                    local: 'postgresql-0021.html',
                    children: [
                      {
                        id: 223,
                        parentId: 222,
                        name: 'Planning the Replication Network',
                        local: 'postgresql-0025_a1.html'
                      },
                      {
                        id: 224,
                        parentId: 222,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'postgresql-0071_a1.html'
                      },
                      {
                        id: 225,
                        parentId: 222,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'postgresql-0072_a2.html'
                      },
                      {
                        id: 226,
                        parentId: 222,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'postgresql-0024.html'
                      },
                      {
                        id: 227,
                        parentId: 222,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'postgresql-0025.html'
                      },
                      {
                        id: 228,
                        parentId: 222,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'postgresql-0026.html'
                      },
                      {
                        id: 229,
                        parentId: 222,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'postgresql-0027.html'
                      },
                      {
                        id: 230,
                        parentId: 222,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'postgresql-0027_a1.html'
                      },
                      {
                        id: 231,
                        parentId: 222,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'postgresql-0028.html'
                      },
                      {
                        id: 232,
                        parentId: 222,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'postgresql-0033_a1.html'
                      },
                      {
                        id: 233,
                        parentId: 222,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'postgresql-0030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 206,
                parentId: 30,
                name: 'Archiving',
                local: 'postgresql-0031.html',
                children: [
                  {
                    id: 234,
                    parentId: 206,
                    name: 'Archiving PostgreSQL Backup Copies',
                    local: 'postgresql-0034.html',
                    children: [
                      {
                        id: 236,
                        parentId: 234,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'postgresql-0035.html',
                        children: [
                          {
                            id: 238,
                            parentId: 236,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'postgresql-0036.html'
                          },
                          {
                            id: 239,
                            parentId: 236,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'postgresql-0037.html'
                          }
                        ]
                      },
                      {
                        id: 237,
                        parentId: 234,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'postgresql-0038.html'
                      }
                    ]
                  },
                  {
                    id: 235,
                    parentId: 206,
                    name: 'Archiving PostgreSQL Replication Copies',
                    local: 'postgresql-0043_a1.html',
                    children: [
                      {
                        id: 240,
                        parentId: 235,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'postgresql-0044_a1.html'
                      },
                      {
                        id: 241,
                        parentId: 235,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'postgresql-0045_a1.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 207,
                parentId: 30,
                name: 'Restoration',
                local: 'postgresql-0042.html',
                children: [
                  {
                    id: 242,
                    parentId: 207,
                    name: 'Restoring PostgreSQL',
                    local: 'postgresql-0045.html'
                  }
                ]
              },
              {
                id: 208,
                parentId: 30,
                name: 'Global Search',
                local: 'postgresql-0027_a2.html',
                children: [
                  {
                    id: 243,
                    parentId: 208,
                    name: 'About Global Search',
                    local: 'fc_gud_gs2_0.html'
                  },
                  {
                    id: 244,
                    parentId: 208,
                    name: 'Global Search for Resources',
                    local: 'postgresql-0027_a3.html'
                  },
                  {
                    id: 245,
                    parentId: 208,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'postgresql-0027_a4.html'
                  }
                ]
              },
              {
                id: 209,
                parentId: 30,
                name: 'SLA ',
                local: 'postgresql-0050.html',
                children: [
                  {
                    id: 246,
                    parentId: 209,
                    name: 'About SLA',
                    local: 'postgresql-0055_a1.html'
                  },
                  {
                    id: 247,
                    parentId: 209,
                    name: 'Viewing SLA Information',
                    local: 'postgresql-0056_1q.html'
                  },
                  {
                    id: 248,
                    parentId: 209,
                    name: 'Managing SLAs',
                    local: 'postgresql-0053.html'
                  }
                ]
              },
              {
                id: 210,
                parentId: 30,
                name: 'Copies',
                local: 'postgresql-0058_a1.html',
                children: [
                  {
                    id: 249,
                    parentId: 210,
                    name: 'Viewing PostgreSQL Copy Information',
                    local: 'postgresql-0055.html'
                  },
                  {
                    id: 250,
                    parentId: 210,
                    name: 'Managing PostgreSQL Copies',
                    local: 'postgresql-0056.html'
                  }
                ]
              },
              {
                id: 211,
                parentId: 30,
                name: 'PostgreSQL Cluster Environment',
                local: 'postgresql-0057.html',
                children: [
                  {
                    id: 251,
                    parentId: 211,
                    name: 'Checking the PostgreSQL Environment',
                    local: 'postgresql-0058.html'
                  },
                  {
                    id: 252,
                    parentId: 211,
                    name: 'Managing PostgreSQL',
                    local: 'postgresql-0059.html'
                  },
                  {
                    id: 253,
                    parentId: 211,
                    name: 'Managing PostgreSQL Database Clusters',
                    local: 'postgresql-0060.html'
                  }
                ]
              }
            ]
          },
          {
            id: 31,
            parentId: 13,
            name: 'DB2 Data Protection',
            local: 'en-us_topic_0000001873759405.html',
            children: [
              {
                id: 254,
                parentId: 31,
                name: 'Backup',
                local: 'DB2-00007.html',
                children: [
                  {
                    id: 262,
                    parentId: 254,
                    name: 'Preparations for Backup',
                    local: 'DB2-00010.html'
                  },
                  {
                    id: 263,
                    parentId: 254,
                    name: 'Backing Up DB2 Databases or Tablespace Sets',
                    local: 'DB2-00011.html',
                    children: [
                      {
                        id: 264,
                        parentId: 263,
                        name: 'Step 1: Registering the DB2 Database',
                        local: 'DB2-00012.html'
                      },
                      {
                        id: 265,
                        parentId: 263,
                        name: 'Step 2: Creating a DB2 Tablespace Set',
                        local: 'DB2-00013.html'
                      },
                      {
                        id: 266,
                        parentId: 263,
                        name: 'Step 3: Creating a Rate Limiting Policy',
                        local: 'DB2-00014.html'
                      },
                      {
                        id: 267,
                        parentId: 263,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'DB2-00011_a1.html'
                      },
                      {
                        id: 268,
                        parentId: 263,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'DB2-00016.html'
                      },
                      {
                        id: 269,
                        parentId: 263,
                        name: 'Step 6: Performing Backup',
                        local: 'DB2-00017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 255,
                parentId: 31,
                name: 'Replication',
                local: 'oracle_gud_000035_13.html',
                children: [
                  {
                    id: 270,
                    parentId: 255,
                    name: 'DB2 Copy Replication',
                    local: 'DB2-00023.html',
                    children: [
                      {
                        id: 271,
                        parentId: 270,
                        name: 'Planning the Replication Network',
                        local: 'DB2-00024.html'
                      },
                      {
                        id: 272,
                        parentId: 270,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'fc_gud_0026_10.html'
                      },
                      {
                        id: 273,
                        parentId: 270,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_17.html'
                      },
                      {
                        id: 274,
                        parentId: 270,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'DB2-00026.html'
                      },
                      {
                        id: 275,
                        parentId: 270,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'DB2-00027.html'
                      },
                      {
                        id: 276,
                        parentId: 270,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'DB2-00028.html'
                      },
                      {
                        id: 277,
                        parentId: 270,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'DB2-00029.html'
                      },
                      {
                        id: 278,
                        parentId: 270,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'DB2-00024_a1.html'
                      },
                      {
                        id: 279,
                        parentId: 270,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'DB2-00030.html'
                      },
                      {
                        id: 280,
                        parentId: 270,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'DB2-00031.html'
                      },
                      {
                        id: 281,
                        parentId: 270,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'DB2-00032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 256,
                parentId: 31,
                name: 'Archiving',
                local: 'DB2-00033.html',
                children: [
                  {
                    id: 282,
                    parentId: 256,
                    name: 'Archiving DB2 Backup Copies',
                    local: 'DB2-00036.html',
                    children: [
                      {
                        id: 284,
                        parentId: 282,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'DB2-00037.html',
                        children: [
                          {
                            id: 286,
                            parentId: 284,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'DB2-00038.html'
                          },
                          {
                            id: 287,
                            parentId: 284,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'DB2-00039.html'
                          }
                        ]
                      },
                      {
                        id: 285,
                        parentId: 282,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'DB2-00040.html'
                      }
                    ]
                  },
                  {
                    id: 283,
                    parentId: 256,
                    name: 'Archiving DB2 Replication Copies',
                    local: 'DB2-00041.html',
                    children: [
                      {
                        id: 288,
                        parentId: 283,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'DB2-00042.html'
                      },
                      {
                        id: 289,
                        parentId: 283,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'DB2-00043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 257,
                parentId: 31,
                name: 'Restoration',
                local: 'DB2-00044.html',
                children: [
                  {
                    id: 290,
                    parentId: 257,
                    name: 'Restoring DB2 Databases or Tablespace Sets',
                    local: 'DB2-00047.html'
                  }
                ]
              },
              {
                id: 258,
                parentId: 31,
                name: 'Global Search',
                local: 'DB2-00039_a1.html',
                children: [
                  {
                    id: 291,
                    parentId: 258,
                    name: 'Global Search for Resources',
                    local: 'DB2-00039_a2.html'
                  },
                  {
                    id: 292,
                    parentId: 258,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'DB2-00039_a3.html'
                  }
                ]
              },
              {
                id: 259,
                parentId: 31,
                name: 'SLA ',
                local: 'DB2-00051.html',
                children: [
                  {
                    id: 293,
                    parentId: 259,
                    name: 'About SLA',
                    local: 'DB2-00052.html'
                  },
                  {
                    id: 294,
                    parentId: 259,
                    name: 'Viewing SLA Information',
                    local: 'DB2-00053.html'
                  },
                  {
                    id: 295,
                    parentId: 259,
                    name: 'Managing SLAs',
                    local: 'DB2-00054.html'
                  }
                ]
              },
              {
                id: 260,
                parentId: 31,
                name: 'Copies',
                local: 'DB2-00055.html',
                children: [
                  {
                    id: 296,
                    parentId: 260,
                    name: 'Viewing DB2 Copy Information',
                    local: 'DB2-00056.html'
                  },
                  {
                    id: 297,
                    parentId: 260,
                    name: 'Managing DB2 Copies',
                    local: 'DB2-00057.html'
                  }
                ]
              },
              {
                id: 261,
                parentId: 31,
                name: 'DB2 Cluster Environment',
                local: 'DB2-00058.html',
                children: [
                  {
                    id: 298,
                    parentId: 261,
                    name: 'Querying DB2 Information',
                    local: 'DB2-00059.html'
                  },
                  {
                    id: 299,
                    parentId: 261,
                    name: 'Managing DB2 Clusters or Tablespace Sets',
                    local: 'DB2-00060.html'
                  },
                  {
                    id: 300,
                    parentId: 261,
                    name: 'Managing DB2 Databases or Tablespace Sets',
                    local: 'DB2-00061.html'
                  }
                ]
              }
            ]
          },
          {
            id: 32,
            parentId: 13,
            name: 'Informix/GBase 8s Data Protection',
            local: 'en-us_topic_0000001873759417.html',
            children: [
              {
                id: 301,
                parentId: 32,
                name: 'Backup',
                local: 'informix-0007.html',
                children: [
                  {
                    id: 309,
                    parentId: 301,
                    name: 'Backing Up Informix/GBase 8s',
                    local: 'informix-0010.html',
                    children: [
                      {
                        id: 310,
                        parentId: 309,
                        name: 'Step 1: Configuring the XBSA Library Path',
                        local: 'informix-0011.html'
                      },
                      {
                        id: 311,
                        parentId: 309,
                        name:
                          'Step 2: Registering an Informix/GBase 8s Cluster',
                        local: 'informix-0012.html'
                      },
                      {
                        id: 312,
                        parentId: 309,
                        name:
                          'Step 3: Registering an Informix/GBase 8s Instance',
                        local: 'informix-0013.html'
                      },
                      {
                        id: 313,
                        parentId: 309,
                        name: 'Step 4: Creating a Rate Limiting Policy',
                        local: 'informix-0014.html'
                      },
                      {
                        id: 314,
                        parentId: 309,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'informix-0015.html'
                      },
                      {
                        id: 315,
                        parentId: 309,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'informix-0016.html'
                      },
                      {
                        id: 316,
                        parentId: 309,
                        name: 'Step 7: Performing Backup',
                        local: 'informix-0017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 302,
                parentId: 32,
                name: 'Replication',
                local: 'oracle_gud_000035.html',
                children: [
                  {
                    id: 317,
                    parentId: 302,
                    name: 'Replicating Informix/GBase 8s Database Copies',
                    local: 'informix-0023.html',
                    children: [
                      {
                        id: 318,
                        parentId: 317,
                        name: 'Planning the Replication Network',
                        local: 'informix-0024.html'
                      },
                      {
                        id: 319,
                        parentId: 317,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'fc_gud_0026.html'
                      },
                      {
                        id: 320,
                        parentId: 317,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1.html'
                      },
                      {
                        id: 321,
                        parentId: 317,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'informix-0026.html'
                      },
                      {
                        id: 322,
                        parentId: 317,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'informix-0027.html'
                      },
                      {
                        id: 323,
                        parentId: 317,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'informix-0028.html'
                      },
                      {
                        id: 324,
                        parentId: 317,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'informix-0029.html'
                      },
                      {
                        id: 325,
                        parentId: 317,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002046568669.html'
                      },
                      {
                        id: 326,
                        parentId: 317,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'informix-0030.html'
                      },
                      {
                        id: 327,
                        parentId: 317,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'informix-0031.html'
                      },
                      {
                        id: 328,
                        parentId: 317,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'informix-0032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 303,
                parentId: 32,
                name: 'Archiving',
                local: 'informix-0033.html',
                children: [
                  {
                    id: 329,
                    parentId: 303,
                    name: 'Archiving Informix/GBase 8s Backup Copies',
                    local: 'informix-0036.html',
                    children: [
                      {
                        id: 331,
                        parentId: 329,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'informix-0037.html',
                        children: [
                          {
                            id: 333,
                            parentId: 331,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'informix-0038.html'
                          },
                          {
                            id: 334,
                            parentId: 331,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'informix-0039.html'
                          }
                        ]
                      },
                      {
                        id: 332,
                        parentId: 329,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'informix-0040.html'
                      }
                    ]
                  },
                  {
                    id: 330,
                    parentId: 303,
                    name: 'Archiving Informix/GBase 8s Replication Copies',
                    local: 'informix-0041.html',
                    children: [
                      {
                        id: 335,
                        parentId: 330,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'informix-0042.html'
                      },
                      {
                        id: 336,
                        parentId: 330,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'informix-0043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 304,
                parentId: 32,
                name: 'Restoration',
                local: 'informix-0044.html',
                children: [
                  {
                    id: 337,
                    parentId: 304,
                    name: 'Restoring Informix/GBase 8s',
                    local: 'informix-0047.html'
                  }
                ]
              },
              {
                id: 305,
                parentId: 32,
                name: 'Global Search',
                local: 'informix-0048.html',
                children: [
                  {
                    id: 338,
                    parentId: 305,
                    name: 'About Global Search',
                    local: 'en-us_topic_0000002093405854.html'
                  },
                  {
                    id: 339,
                    parentId: 305,
                    name: 'Global Search for Copies',
                    local: 'vmware_gud_0076.html'
                  },
                  {
                    id: 340,
                    parentId: 305,
                    name: 'Global Search for Resources',
                    local: 'informix-0049.html'
                  },
                  {
                    id: 341,
                    parentId: 305,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002038759717.html'
                  }
                ]
              },
              {
                id: 306,
                parentId: 32,
                name: 'SLA ',
                local: 'informix-0052.html',
                children: [
                  {
                    id: 342,
                    parentId: 306,
                    name: 'About SLA ',
                    local: 'informix-0053.html'
                  },
                  {
                    id: 343,
                    parentId: 306,
                    name: 'Viewing SLA Information',
                    local: 'informix-0054.html'
                  },
                  {
                    id: 344,
                    parentId: 306,
                    name: 'Managing SLAs',
                    local: 'informix-0055.html'
                  }
                ]
              },
              {
                id: 307,
                parentId: 32,
                name: 'Copies',
                local: 'informix-0056.html',
                children: [
                  {
                    id: 345,
                    parentId: 307,
                    name: 'Viewing Informix/GBase 8s Copy Information',
                    local: 'informix-0057.html'
                  },
                  {
                    id: 346,
                    parentId: 307,
                    name: 'Managing Informix/GBase 8s Copies',
                    local: 'informix-0058.html'
                  }
                ]
              },
              {
                id: 308,
                parentId: 32,
                name: 'Informix/GBase 8s Cluster Environment',
                local: 'informix-0059.html',
                children: [
                  {
                    id: 347,
                    parentId: 308,
                    name:
                      'Viewing the Informix/GBase 8s Environment Information',
                    local: 'informix-0060.html'
                  },
                  {
                    id: 348,
                    parentId: 308,
                    name: 'Managing Informix/GBase 8s',
                    local: 'informix-0061.html'
                  },
                  {
                    id: 349,
                    parentId: 308,
                    name: 'Managing Informix/GBase 8s Database Clusters',
                    local: 'informix-0062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 33,
            parentId: 13,
            name: 'openGauss Data Protection',
            local: 'en-us_topic_0000001873679197.html',
            children: [
              {
                id: 350,
                parentId: 33,
                name: 'Backup',
                local: 'opengauss-0107.html',
                children: [
                  {
                    id: 358,
                    parentId: 350,
                    name: 'Preparations for Backup',
                    local: 'opengauss-0110.html'
                  },
                  {
                    id: 359,
                    parentId: 350,
                    name: 'Backing Up openGauss',
                    local: 'opengauss-0111.html',
                    children: [
                      {
                        id: 360,
                        parentId: 359,
                        name: 'Step 2: Registering an openGauss Cluster',
                        local: 'opengauss-0112.html'
                      },
                      {
                        id: 361,
                        parentId: 359,
                        name: 'Step 3: Creating a Rate Limiting Policy',
                        local: 'opengauss-0113.html'
                      },
                      {
                        id: 362,
                        parentId: 359,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'opengauss-0114.html'
                      },
                      {
                        id: 363,
                        parentId: 359,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'opengauss-0115.html'
                      },
                      {
                        id: 364,
                        parentId: 359,
                        name: 'Step 6: Performing Backup',
                        local: 'opengauss-0116.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 351,
                parentId: 33,
                name: 'Replication',
                local: 'opengauss-0119.html',
                children: [
                  {
                    id: 365,
                    parentId: 351,
                    name: 'Replicating openGauss Database Copies',
                    local: 'opengauss-0122.html',
                    children: [
                      {
                        id: 366,
                        parentId: 365,
                        name: 'Planning the Replication Network',
                        local: 'opengauss-0123.html'
                      },
                      {
                        id: 367,
                        parentId: 365,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'opengauss-0124.html'
                      },
                      {
                        id: 368,
                        parentId: 365,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'opengauss-0125.html'
                      },
                      {
                        id: 369,
                        parentId: 365,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'opengauss-0126.html'
                      },
                      {
                        id: 370,
                        parentId: 365,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'opengauss-0127.html'
                      },
                      {
                        id: 371,
                        parentId: 365,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'opengauss-0128.html'
                      },
                      {
                        id: 372,
                        parentId: 365,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'opengauss-0129.html'
                      },
                      {
                        id: 373,
                        parentId: 365,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'opengauss-0026_a1.html'
                      },
                      {
                        id: 374,
                        parentId: 365,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'opengauss-0130.html'
                      },
                      {
                        id: 375,
                        parentId: 365,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'opengauss-0131.html'
                      },
                      {
                        id: 376,
                        parentId: 365,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA ',
                        local: 'opengauss-0132.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 352,
                parentId: 33,
                name: 'Archiving',
                local: 'opengauss-0133.html',
                children: [
                  {
                    id: 377,
                    parentId: 352,
                    name: 'Archiving openGauss Backup Copies',
                    local: 'opengauss-0136.html',
                    children: [
                      {
                        id: 379,
                        parentId: 377,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'opengauss-0137.html',
                        children: [
                          {
                            id: 381,
                            parentId: 379,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'opengauss-0138.html'
                          },
                          {
                            id: 382,
                            parentId: 379,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'opengauss-0139.html'
                          }
                        ]
                      },
                      {
                        id: 380,
                        parentId: 377,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'opengauss-0140.html'
                      }
                    ]
                  },
                  {
                    id: 378,
                    parentId: 352,
                    name: 'Archiving openGauss Replication Copies',
                    local: 'opengauss-0141.html',
                    children: [
                      {
                        id: 383,
                        parentId: 378,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'opengauss-0142.html'
                      },
                      {
                        id: 384,
                        parentId: 378,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'opengauss-0143.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 353,
                parentId: 33,
                name: 'Restoration',
                local: 'opengauss-0144.html',
                children: [
                  {
                    id: 385,
                    parentId: 353,
                    name: 'Restoring openGauss',
                    local: 'opengauss-0147.html'
                  }
                ]
              },
              {
                id: 354,
                parentId: 33,
                name: 'Global Search',
                local: 'opengauss-0026_a2.html',
                children: [
                  {
                    id: 386,
                    parentId: 354,
                    name: 'Global Search for Resources',
                    local: 'opengauss-0026_a3.html'
                  },
                  {
                    id: 387,
                    parentId: 354,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'opengauss-0026_a4.html'
                  }
                ]
              },
              {
                id: 355,
                parentId: 33,
                name: 'SLA ',
                local: 'opengauss-0152.html',
                children: [
                  {
                    id: 388,
                    parentId: 355,
                    name: 'About SLA',
                    local: 'opengauss-0153.html'
                  },
                  {
                    id: 389,
                    parentId: 355,
                    name: 'Viewing SLA Information',
                    local: 'opengauss-0154.html'
                  },
                  {
                    id: 390,
                    parentId: 355,
                    name: 'Managing SLAs',
                    local: 'opengauss-0155.html'
                  }
                ]
              },
              {
                id: 356,
                parentId: 33,
                name: 'Copies',
                local: 'opgs_0060.html',
                children: [
                  {
                    id: 391,
                    parentId: 356,
                    name: 'Viewing openGauss Copy Information',
                    local: 'opengauss-0157.html'
                  },
                  {
                    id: 392,
                    parentId: 356,
                    name: 'Managing openGauss Copies',
                    local: 'opgs_0062.html'
                  }
                ]
              },
              {
                id: 357,
                parentId: 33,
                name: 'openGauss Database Environments',
                local: 'opengauss-0159.html',
                children: [
                  {
                    id: 393,
                    parentId: 357,
                    name: 'Viewing openGauss Information',
                    local: 'opengauss-0160.html'
                  },
                  {
                    id: 394,
                    parentId: 357,
                    name: 'Managing openGauss',
                    local: 'opengauss-0161.html'
                  },
                  {
                    id: 395,
                    parentId: 357,
                    name: 'Managing openGauss Clusters',
                    local: 'opengauss-0162.html'
                  }
                ]
              }
            ]
          },
          {
            id: 34,
            parentId: 13,
            name: 'GaussDB T Data Protection',
            local: 'en-us_topic_0000001827039680.html',
            children: [
              {
                id: 396,
                parentId: 34,
                name: 'Backup',
                local: 'gaussdbT_00006.html',
                children: [
                  {
                    id: 404,
                    parentId: 396,
                    name: 'Preparations for Backup',
                    local: 'gaussdbT_00009.html'
                  },
                  {
                    id: 405,
                    parentId: 396,
                    name: 'Backing Up a GaussDB T Database',
                    local: 'gaussdbT_00010.html',
                    children: [
                      {
                        id: 406,
                        parentId: 405,
                        name:
                          'Step 1: Checking and Configuring the Database Environment',
                        local: 'gaussdbT_00011.html'
                      },
                      {
                        id: 407,
                        parentId: 405,
                        name: 'Step 2: Setting the Redo Log Mode',
                        local: 'gaussdbT_00071.html'
                      },
                      {
                        id: 408,
                        parentId: 405,
                        name: 'Step 3: Registering the GaussDB T Database',
                        local: 'gaussdbT_00012.html'
                      },
                      {
                        id: 409,
                        parentId: 405,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'gaussdbT_00013.html'
                      },
                      {
                        id: 410,
                        parentId: 405,
                        name:
                          'Step 5: (Optional) Creating a Rate Limiting Policy',
                        local: 'gaussdbT_00014.html'
                      },
                      {
                        id: 411,
                        parentId: 405,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'gaussdbT_00015.html'
                      },
                      {
                        id: 412,
                        parentId: 405,
                        name: 'Step 7: Performing Backup',
                        local: 'gaussdbT_00016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 397,
                parentId: 34,
                name: 'Replication',
                local: 'gaussdbT_00019.html',
                children: [
                  {
                    id: 413,
                    parentId: 397,
                    name: 'Replicating a GaussDB T Database Copy',
                    local: 'gaussdbT_00021.html',
                    children: [
                      {
                        id: 414,
                        parentId: 413,
                        name: 'Planning the Replication Network',
                        local: 'gaussdbT_00022.html'
                      },
                      {
                        id: 415,
                        parentId: 413,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'gaussdbT_00023.html'
                      },
                      {
                        id: 416,
                        parentId: 413,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_10.html'
                      },
                      {
                        id: 417,
                        parentId: 413,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'gaussdbT_00024.html'
                      },
                      {
                        id: 418,
                        parentId: 413,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'gaussdbT_00025.html'
                      },
                      {
                        id: 419,
                        parentId: 413,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'gaussdbT_00026.html'
                      },
                      {
                        id: 420,
                        parentId: 413,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'gaussdbT_00027.html'
                      },
                      {
                        id: 421,
                        parentId: 413,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'gaussdbT_00027_a1.html'
                      },
                      {
                        id: 422,
                        parentId: 413,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'gaussdbT_00028.html'
                      },
                      {
                        id: 423,
                        parentId: 413,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'gaussdbT_00029.html'
                      },
                      {
                        id: 424,
                        parentId: 413,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'gaussdbT_00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 398,
                parentId: 34,
                name: 'Archiving',
                local: 'gaussdbT_00031.html',
                children: [
                  {
                    id: 425,
                    parentId: 398,
                    name: 'Archiving GaussDB T Backup Copies',
                    local: 'gaussdbT_00034.html',
                    children: [
                      {
                        id: 427,
                        parentId: 425,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'gaussdbT_00035.html',
                        children: [
                          {
                            id: 429,
                            parentId: 427,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'gaussdbT_00036.html'
                          },
                          {
                            id: 430,
                            parentId: 427,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'gaussdbT_00037.html'
                          }
                        ]
                      },
                      {
                        id: 428,
                        parentId: 425,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'gaussdbT_00038.html'
                      }
                    ]
                  },
                  {
                    id: 426,
                    parentId: 398,
                    name: 'Archiving GaussDB T Replication Copies ',
                    local: 'gaussdbT_00039.html',
                    children: [
                      {
                        id: 431,
                        parentId: 426,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'gaussdbT_00040.html'
                      },
                      {
                        id: 432,
                        parentId: 426,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'gaussdbT_00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 399,
                parentId: 34,
                name: 'Restoration',
                local: 'gaussdbT_00042.html',
                children: [
                  {
                    id: 433,
                    parentId: 399,
                    name: 'Restoring the GaussDB T Database',
                    local: 'gaussdbT_00045.html'
                  }
                ]
              },
              {
                id: 400,
                parentId: 34,
                name: 'Global Search',
                local: 'gaussdbT_00042_a1.html',
                children: [
                  {
                    id: 434,
                    parentId: 400,
                    name: 'Global Search for Resources',
                    local: 'gaussdbT_00042_a2.html'
                  },
                  {
                    id: 435,
                    parentId: 400,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'gaussdbT_00042_a3.html'
                  }
                ]
              },
              {
                id: 401,
                parentId: 34,
                name: 'SLA ',
                local: 'gaussdbT_00049.html',
                children: [
                  {
                    id: 436,
                    parentId: 401,
                    name: 'About SLA',
                    local: 'gaussdbT_00050.html'
                  },
                  {
                    id: 437,
                    parentId: 401,
                    name: 'Viewing SLA Information',
                    local: 'gaussdbT_00051.html'
                  },
                  {
                    id: 438,
                    parentId: 401,
                    name: 'Managing SLAs',
                    local: 'gaussdbT_00052.html'
                  }
                ]
              },
              {
                id: 402,
                parentId: 34,
                name: 'Copies',
                local: 'gaussdbT_00053.html',
                children: [
                  {
                    id: 439,
                    parentId: 402,
                    name: 'Viewing GaussDB T Database Copy Information',
                    local: 'gaussdbT_00054.html'
                  },
                  {
                    id: 440,
                    parentId: 402,
                    name: 'Managing GaussDB T Database Copies',
                    local: 'gaussdbT_00055.html'
                  }
                ]
              },
              {
                id: 403,
                parentId: 34,
                name: 'GaussDB T Database Environment',
                local: 'gaussdbT_00056.html',
                children: [
                  {
                    id: 441,
                    parentId: 403,
                    name:
                      'Viewing the GaussDB T Database Environment Information',
                    local: 'gaussdbT_00057.html'
                  },
                  {
                    id: 442,
                    parentId: 403,
                    name: 'Managing Databases',
                    local: 'gaussdbT_00058.html'
                  }
                ]
              }
            ]
          },
          {
            id: 35,
            parentId: 13,
            name: 'TiDB Data Protection',
            local: 'en-us_topic_0000001873759409.html',
            children: [
              {
                id: 443,
                parentId: 35,
                name: 'Overview',
                local: 'TIDB_00071.html',
                children: [
                  {
                    id: 453,
                    parentId: 443,
                    name: 'Function Overview',
                    local: 'TIDB_00072.html'
                  }
                ]
              },
              {
                id: 444,
                parentId: 35,
                name: 'Constraints',
                local: 'TIDB_0007333.html'
              },
              {
                id: 445,
                parentId: 35,
                name: 'Backup',
                local: 'TiDB_00004.html',
                children: [
                  {
                    id: 454,
                    parentId: 445,
                    name: 'Preparing for the Backup',
                    local: 'TiDB_00007.html'
                  },
                  {
                    id: 455,
                    parentId: 445,
                    name: 'Backing Up TiDB Backup Resources',
                    local: 'TiDB_00008.html',
                    children: [
                      {
                        id: 456,
                        parentId: 455,
                        name: 'Step 1: Registering a TiDB Cluster',
                        local: 'TiDB_00009.html'
                      },
                      {
                        id: 457,
                        parentId: 455,
                        name: 'Step 2: Registering a TiDB Database',
                        local: 'TiDB_00010.html'
                      },
                      {
                        id: 458,
                        parentId: 455,
                        name: 'Step 3: Registering a TiDB Table Set',
                        local: 'TiDB_00011.html'
                      },
                      {
                        id: 459,
                        parentId: 455,
                        name: 'Step 4: Creating a Rate Limiting Policy',
                        local: 'TiDB_00012.html'
                      },
                      {
                        id: 460,
                        parentId: 455,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'TiDB_00013.html'
                      },
                      {
                        id: 461,
                        parentId: 455,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'TiDB_00014.html'
                      },
                      {
                        id: 462,
                        parentId: 455,
                        name: 'Step 7: Performing Backup',
                        local: 'TiDB_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 446,
                parentId: 35,
                name: 'Replication',
                local: 'TiDB_00018.html',
                children: [
                  {
                    id: 463,
                    parentId: 446,
                    name: 'Replicating TiDB Copies',
                    local: 'TiDB_00020.html',
                    children: [
                      {
                        id: 464,
                        parentId: 463,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'TiDB_00022.html'
                      },
                      {
                        id: 465,
                        parentId: 463,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'TiDB_000221.html'
                      },
                      {
                        id: 466,
                        parentId: 463,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'TiDB_00023.html'
                      },
                      {
                        id: 467,
                        parentId: 463,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'TiDB_00024.html'
                      },
                      {
                        id: 468,
                        parentId: 463,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'TiDB_00025.html'
                      },
                      {
                        id: 469,
                        parentId: 463,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'TiDB_00026.html'
                      },
                      {
                        id: 470,
                        parentId: 463,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'TiDB_0002600.html'
                      },
                      {
                        id: 471,
                        parentId: 463,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'TiDB_00027.html'
                      },
                      {
                        id: 472,
                        parentId: 463,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'TiDB_00028.html'
                      },
                      {
                        id: 473,
                        parentId: 463,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'TiDB_00029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 447,
                parentId: 35,
                name: 'Archiving',
                local: 'TiDB_00030.html',
                children: [
                  {
                    id: 474,
                    parentId: 447,
                    name: 'Archiving TiDB Backup Copies',
                    local: 'TiDB_00033.html',
                    children: [
                      {
                        id: 476,
                        parentId: 474,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'TiDB_00034.html',
                        children: [
                          {
                            id: 478,
                            parentId: 476,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'TiDB_00035.html'
                          },
                          {
                            id: 479,
                            parentId: 476,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'TiDB_00036.html'
                          }
                        ]
                      },
                      {
                        id: 477,
                        parentId: 474,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'TiDB_00037.html'
                      }
                    ]
                  },
                  {
                    id: 475,
                    parentId: 447,
                    name: 'Archiving TiDB Replication Copies',
                    local: 'TiDB_00038.html',
                    children: [
                      {
                        id: 480,
                        parentId: 475,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'TiDB_00039.html'
                      },
                      {
                        id: 481,
                        parentId: 475,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'TiDB_00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 448,
                parentId: 35,
                name: 'Restoration',
                local: 'TiDB_00041.html',
                children: [
                  {
                    id: 482,
                    parentId: 448,
                    name: 'Restoring TiDB Backup Resources',
                    local: 'TiDB_00044.html'
                  },
                  {
                    id: 483,
                    parentId: 448,
                    name:
                      'Restoring One or More Tables in TiDB Backup Resources',
                    local: 'TiDB_00045.html'
                  }
                ]
              },
              {
                id: 449,
                parentId: 35,
                name: 'Global Search',
                local: 'TiDB_00046.html',
                children: [
                  {
                    id: 484,
                    parentId: 449,
                    name: 'Global Search for Resources',
                    local: 'TiDB_00047.html'
                  },
                  {
                    id: 485,
                    parentId: 449,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'TiDB_000471.html'
                  }
                ]
              },
              {
                id: 450,
                parentId: 35,
                name: 'SLA',
                local: 'TiDB_00050.html',
                children: [
                  {
                    id: 486,
                    parentId: 450,
                    name: 'About SLA',
                    local: 'TiDB_000501.html'
                  },
                  {
                    id: 487,
                    parentId: 450,
                    name: 'Viewing SLA Information',
                    local: 'TiDB_00052.html'
                  },
                  {
                    id: 488,
                    parentId: 450,
                    name: 'Managing SLAs',
                    local: 'TiDB_00053.html'
                  }
                ]
              },
              {
                id: 451,
                parentId: 35,
                name: 'Copies',
                local: 'TiDB_00054.html',
                children: [
                  {
                    id: 489,
                    parentId: 451,
                    name: 'Viewing TiDB Copy Information',
                    local: 'TiDB_00055.html'
                  },
                  {
                    id: 490,
                    parentId: 451,
                    name: 'Managing TiDB Copies',
                    local: 'TiDB_00056.html'
                  }
                ]
              },
              {
                id: 452,
                parentId: 35,
                name: 'TiDB Cluster Environment',
                local: 'TiDB_00057.html',
                children: [
                  {
                    id: 491,
                    parentId: 452,
                    name: 'Querying TiDB Information',
                    local: 'TiDB_00058.html'
                  },
                  {
                    id: 492,
                    parentId: 452,
                    name: 'Managing TiDB Clusters',
                    local: 'TiDB_00059.html'
                  },
                  {
                    id: 493,
                    parentId: 452,
                    name: 'Managing Databases',
                    local: 'TiDB_00060.html'
                  },
                  {
                    id: 494,
                    parentId: 452,
                    name: 'Managing Table Sets',
                    local: 'TiDB_00061.html'
                  }
                ]
              }
            ]
          },
          {
            id: 36,
            parentId: 13,
            name: 'OceanBase Data Protection',
            local: 'en-us_topic_0000001826879852.html',
            children: [
              {
                id: 495,
                parentId: 36,
                name: 'Backup',
                local: 'oceanbase_00005.html',
                children: [
                  {
                    id: 503,
                    parentId: 495,
                    name: 'Preparations for Backup',
                    local: 'oceanbase_00008.html'
                  },
                  {
                    id: 504,
                    parentId: 495,
                    name: 'Backing Up OceanBase',
                    local: 'oceanbase_00009.html',
                    children: [
                      {
                        id: 505,
                        parentId: 504,
                        name:
                          'Step 1: Checking and Enabling the NFSv4.1 Service',
                        local: 'en-us_topic_0000001839342213.html'
                      },
                      {
                        id: 506,
                        parentId: 504,
                        name: 'Step 2: Registering an OceanBase Cluster',
                        local: 'oceanbase_00010.html'
                      },
                      {
                        id: 507,
                        parentId: 504,
                        name: 'Step 3: Registering an OceanBase Tenant Set',
                        local: 'oceanbase_00011.html'
                      },
                      {
                        id: 508,
                        parentId: 504,
                        name:
                          'Step 4: (Optional) Creating a Rate Limiting Policy',
                        local: 'oceanbase_00012.html'
                      },
                      {
                        id: 509,
                        parentId: 504,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'oceanbase_00013.html'
                      },
                      {
                        id: 510,
                        parentId: 504,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'oceanbase_00014.html'
                      },
                      {
                        id: 511,
                        parentId: 504,
                        name: 'Step 7: Performing Backup',
                        local: 'oceanbase_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 496,
                parentId: 36,
                name: 'Replication',
                local: 'oracle_gud_000035_3.html',
                children: [
                  {
                    id: 512,
                    parentId: 496,
                    name: 'Replicating OceanBase Copies',
                    local: 'oceanbase_00020.html',
                    children: [
                      {
                        id: 513,
                        parentId: 512,
                        name: 'Planning the Replication Network',
                        local: 'oceanbase_00021.html'
                      },
                      {
                        id: 514,
                        parentId: 512,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'oceanbase_000210.html'
                      },
                      {
                        id: 515,
                        parentId: 512,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_5.html'
                      },
                      {
                        id: 516,
                        parentId: 512,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'oceanbase_00023.html'
                      },
                      {
                        id: 517,
                        parentId: 512,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'oceanbase_00024.html'
                      },
                      {
                        id: 518,
                        parentId: 512,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'oceanbase_00025.html'
                      },
                      {
                        id: 519,
                        parentId: 512,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'oceanbase_00026.html'
                      },
                      {
                        id: 520,
                        parentId: 512,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'oceanbase_00026_a1.html'
                      },
                      {
                        id: 521,
                        parentId: 512,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'oceanbase_00027.html'
                      },
                      {
                        id: 522,
                        parentId: 512,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'oceanbase_00028.html'
                      },
                      {
                        id: 523,
                        parentId: 512,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'oceanbase_00029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 497,
                parentId: 36,
                name: 'Archiving',
                local: 'oceanbase_00030.html',
                children: [
                  {
                    id: 524,
                    parentId: 497,
                    name: 'Archiving OceanBase Backup Copies',
                    local: 'oceanbase_00033.html',
                    children: [
                      {
                        id: 526,
                        parentId: 524,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'oceanbase_00034.html',
                        children: [
                          {
                            id: 528,
                            parentId: 526,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'oceanbase_00035.html'
                          },
                          {
                            id: 529,
                            parentId: 526,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'oceanbase_00036.html'
                          }
                        ]
                      },
                      {
                        id: 527,
                        parentId: 524,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'oceanbase_00037.html'
                      }
                    ]
                  },
                  {
                    id: 525,
                    parentId: 497,
                    name: 'Archiving OceanBase Replication Copies',
                    local: 'oceanbase_00038.html',
                    children: [
                      {
                        id: 530,
                        parentId: 525,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'oceanbase_00039.html'
                      },
                      {
                        id: 531,
                        parentId: 525,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'oceanbase_00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 498,
                parentId: 36,
                name: 'Restoration',
                local: 'oceanbase_00041.html',
                children: [
                  {
                    id: 532,
                    parentId: 498,
                    name: 'Restoring OceanBase',
                    local: 'oceanbase_00044.html'
                  }
                ]
              },
              {
                id: 499,
                parentId: 36,
                name: 'Global Search',
                local: 'oceanbase_00026_a2.html',
                children: [
                  {
                    id: 533,
                    parentId: 499,
                    name: 'Global Search for Resources',
                    local: 'oceanbase_00026_a3.html'
                  },
                  {
                    id: 534,
                    parentId: 499,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'oceanbase_00026_a4.html'
                  }
                ]
              },
              {
                id: 500,
                parentId: 36,
                name: 'SLA ',
                local: 'oceanbase_00050.html',
                children: [
                  {
                    id: 535,
                    parentId: 500,
                    name: 'About SLA',
                    local: 'oceanbase_00051.html'
                  },
                  {
                    id: 536,
                    parentId: 500,
                    name: 'Viewing SLA Information',
                    local: 'oceanbase_00052.html'
                  },
                  {
                    id: 537,
                    parentId: 500,
                    name: 'Managing SLAs',
                    local: 'oceanbase_00053.html'
                  }
                ]
              },
              {
                id: 501,
                parentId: 36,
                name: 'Copies',
                local: 'oceanbase_00054.html',
                children: [
                  {
                    id: 538,
                    parentId: 501,
                    name: 'Viewing OceanBase Copy Information',
                    local: 'oceanbase_00055.html'
                  },
                  {
                    id: 539,
                    parentId: 501,
                    name: 'Managing OceanBase Copies',
                    local: 'oceanbase_00056.html'
                  }
                ]
              },
              {
                id: 502,
                parentId: 36,
                name: 'OceanBase Cluster Environment',
                local: 'oceanbase_00057.html',
                children: [
                  {
                    id: 540,
                    parentId: 502,
                    name: 'Viewing the OceanBase Environment',
                    local: 'oceanbase_00058.html'
                  },
                  {
                    id: 541,
                    parentId: 502,
                    name: 'Managing Clusters',
                    local: 'oceanbase_00059.html'
                  },
                  {
                    id: 542,
                    parentId: 502,
                    name: 'Managing Tenant Sets',
                    local: 'oceanbase_00060.html'
                  }
                ]
              }
            ]
          },
          {
            id: 37,
            parentId: 13,
            name: 'TDSQL Data Protection',
            local: 'en-us_topic_0000001827039708.html',
            children: [
              {
                id: 543,
                parentId: 37,
                name: 'Backup',
                local: 'tdsql_gud_006.html',
                children: [
                  {
                    id: 551,
                    parentId: 543,
                    name: 'Constraints',
                    local: 'tdsql_gud_082.html'
                  },
                  {
                    id: 552,
                    parentId: 543,
                    name: 'Preparations for Backup',
                    local: 'tdsql_gud_081.html'
                  },
                  {
                    id: 553,
                    parentId: 543,
                    name: 'Backing Up a TDSQL Database',
                    local: 'tdsql_gud_009.html',
                    children: [
                      {
                        id: 554,
                        parentId: 553,
                        name:
                          'Step 1: Enabling the TDSQL Database Permission (Applicable to Non-distributed Instances)',
                        local: 'tdsql_gud_010.html'
                      },
                      {
                        id: 555,
                        parentId: 553,
                        name:
                          'Step 2: Enabling the Automatic zkmeta Backup Function (Applicable to Distributed Instances)',
                        local: 'tdsql_gud_080.html'
                      },
                      {
                        id: 556,
                        parentId: 553,
                        name: 'Step 3: Registering a TDSQL Database',
                        local: 'tdsql_gud_011.html'
                      },
                      {
                        id: 557,
                        parentId: 553,
                        name:
                          'Step 4: (Optional) Creating a Rate Limiting Policy',
                        local: 'tdsql_gud_012.html'
                      },
                      {
                        id: 558,
                        parentId: 553,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'tdsql_gud_013.html'
                      },
                      {
                        id: 559,
                        parentId: 553,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'tdsql_gud_014.html'
                      },
                      {
                        id: 560,
                        parentId: 553,
                        name: 'Step 7: Performing Backup',
                        local: 'tdsql_gud_015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 544,
                parentId: 37,
                name: 'Replication',
                local: 'tdsql_gud_018.html',
                children: [
                  {
                    id: 561,
                    parentId: 544,
                    name: 'Constraints',
                    local: 'tdsql_gud_019.html'
                  },
                  {
                    id: 562,
                    parentId: 544,
                    name: 'Replicating TDSQL Database Copies',
                    local: 'tdsql_gud_020.html',
                    children: [
                      {
                        id: 563,
                        parentId: 562,
                        name: 'Planning the Replication Network',
                        local: 'tdsql_gud_021.html'
                      },
                      {
                        id: 564,
                        parentId: 562,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'tdsql_gud_022.html'
                      },
                      {
                        id: 565,
                        parentId: 562,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'tdsql_gud_0026_1.html'
                      },
                      {
                        id: 566,
                        parentId: 562,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'tdsql_gud_023.html'
                      },
                      {
                        id: 567,
                        parentId: 562,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'tdsql_gud_024.html'
                      },
                      {
                        id: 568,
                        parentId: 562,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'tdsql_gud_025.html'
                      },
                      {
                        id: 569,
                        parentId: 562,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to Version 1.5.0)',
                        local: 'tdsql_gud_026.html'
                      },
                      {
                        id: 570,
                        parentId: 562,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002044460125.html'
                      },
                      {
                        id: 571,
                        parentId: 562,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'tdsql_gud_027.html'
                      },
                      {
                        id: 572,
                        parentId: 562,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'tdsql_gud_028.html'
                      },
                      {
                        id: 573,
                        parentId: 562,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'tdsql_gud_029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 545,
                parentId: 37,
                name: 'Archiving',
                local: 'tdsql_gud_030.html',
                children: [
                  {
                    id: 574,
                    parentId: 545,
                    name: 'Archiving TDSQL Backup Copies',
                    local: 'tdsql_gud_033.html',
                    children: [
                      {
                        id: 576,
                        parentId: 574,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'tdsql_gud_034.html',
                        children: [
                          {
                            id: 578,
                            parentId: 576,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'tdsql_gud_035.html'
                          },
                          {
                            id: 579,
                            parentId: 576,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'tdsql_gud_036.html'
                          }
                        ]
                      },
                      {
                        id: 577,
                        parentId: 574,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'tdsql_gud_037.html'
                      }
                    ]
                  },
                  {
                    id: 575,
                    parentId: 545,
                    name: 'Archiving TDSQL Replication Copies ',
                    local: 'tdsql_gud_038.html',
                    children: [
                      {
                        id: 580,
                        parentId: 575,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'tdsql_gud_039.html'
                      },
                      {
                        id: 581,
                        parentId: 575,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'tdsql_gud_040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 546,
                parentId: 37,
                name: 'Restoration',
                local: 'tdsql_gud_041.html',
                children: [
                  {
                    id: 582,
                    parentId: 546,
                    name: 'Restoring a TDSQL Database',
                    local: 'tdsql_gud_044.html'
                  }
                ]
              },
              {
                id: 547,
                parentId: 37,
                name: 'Global Search',
                local: 'tdsql_gud_045.html',
                children: [
                  {
                    id: 583,
                    parentId: 547,
                    name: 'Global Search for Resources',
                    local: 'tdsql_gud_047.html'
                  },
                  {
                    id: 584,
                    parentId: 547,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002686458.html'
                  }
                ]
              },
              {
                id: 548,
                parentId: 37,
                name: 'SLA ',
                local: 'tdsql_gud_059.html',
                children: [
                  {
                    id: 585,
                    parentId: 548,
                    name: 'About SLA ',
                    local: 'tdsql_gud_060.html'
                  },
                  {
                    id: 586,
                    parentId: 548,
                    name: 'Viewing SLA Information',
                    local: 'tdsql_gud_061.html'
                  },
                  {
                    id: 587,
                    parentId: 548,
                    name: 'Managing SLAs',
                    local: 'tdsql_gud_062.html'
                  }
                ]
              },
              {
                id: 549,
                parentId: 37,
                name: 'Copies',
                local: 'tdsql_gud_063.html',
                children: [
                  {
                    id: 588,
                    parentId: 549,
                    name: 'Viewing TDSQL Copy Information',
                    local: 'tdsql_gud_064.html'
                  },
                  {
                    id: 589,
                    parentId: 549,
                    name: 'Managing TDSQL Copies',
                    local: 'tdsql_gud_065.html'
                  }
                ]
              },
              {
                id: 550,
                parentId: 37,
                name: 'TDSQL Database Environment',
                local: 'tdsql_gud_066.html',
                children: [
                  {
                    id: 590,
                    parentId: 550,
                    name: 'Viewing TDSQL Database Environment Information',
                    local: 'tdsql_gud_067.html'
                  },
                  {
                    id: 591,
                    parentId: 550,
                    name: 'Managing Database Clusters',
                    local: 'tdsql_gud_069.html'
                  },
                  {
                    id: 592,
                    parentId: 550,
                    name: 'Managing Database Instances',
                    local: 'tdsql_gud_068.html'
                  }
                ]
              }
            ]
          },
          {
            id: 38,
            parentId: 13,
            name: 'Dameng Data Protection',
            local: 'en-us_topic_0000001873759369.html',
            children: [
              {
                id: 593,
                parentId: 38,
                name: 'Backup',
                local: 'dameng-00007.html',
                children: [
                  {
                    id: 601,
                    parentId: 593,
                    name: 'Preparations for Backup',
                    local: 'en-us_topic_0000002012732625.html'
                  },
                  {
                    id: 602,
                    parentId: 593,
                    name: 'Backing Up Dameng',
                    local: 'dameng-00011.html',
                    children: [
                      {
                        id: 603,
                        parentId: 602,
                        name: 'Step 1: Enabling the DmAPService',
                        local: 'dameng-00012.html'
                      },
                      {
                        id: 604,
                        parentId: 602,
                        name:
                          'Step 2: Enabling Local Archiving for the Database',
                        local: 'dameng-00013.html'
                      },
                      {
                        id: 605,
                        parentId: 602,
                        name: 'Step 3: Registering the Dameng Database',
                        local: 'dameng-00014.html'
                      },
                      {
                        id: 606,
                        parentId: 602,
                        name:
                          'Step 4: (Optional) Creating a Rate Limiting Policy',
                        local: 'en-us_topic_0000001976337238.html'
                      },
                      {
                        id: 607,
                        parentId: 602,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'dameng-00016.html'
                      },
                      {
                        id: 608,
                        parentId: 602,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'dameng-00017.html'
                      },
                      {
                        id: 609,
                        parentId: 602,
                        name: 'Step 7: Performing Backup',
                        local: 'dameng-00018.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 594,
                parentId: 38,
                name: 'Replication',
                local: 'oracle_gud_000035_1.html',
                children: [
                  {
                    id: 610,
                    parentId: 594,
                    name: 'Replicating Dameng Database Copies',
                    local: 'dameng-00024.html',
                    children: [
                      {
                        id: 611,
                        parentId: 610,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'fc_gud_0026_2.html'
                      },
                      {
                        id: 612,
                        parentId: 610,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_2.html'
                      },
                      {
                        id: 613,
                        parentId: 610,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'dameng-00027.html'
                      },
                      {
                        id: 614,
                        parentId: 610,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'dameng-00028.html'
                      },
                      {
                        id: 615,
                        parentId: 610,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'dameng-00029.html'
                      },
                      {
                        id: 616,
                        parentId: 610,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'dameng-00030.html'
                      },
                      {
                        id: 617,
                        parentId: 610,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'dameng-00027_a1.html'
                      },
                      {
                        id: 618,
                        parentId: 610,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'dameng-00031.html'
                      },
                      {
                        id: 619,
                        parentId: 610,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'dameng-00032.html'
                      },
                      {
                        id: 620,
                        parentId: 610,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'dameng-00033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 595,
                parentId: 38,
                name: 'Archiving',
                local: 'dameng-00034.html',
                children: [
                  {
                    id: 621,
                    parentId: 595,
                    name: 'Archiving Dameng Backup Copies',
                    local: 'dameng-00037.html',
                    children: [
                      {
                        id: 623,
                        parentId: 621,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'dameng-00038.html',
                        children: [
                          {
                            id: 625,
                            parentId: 623,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'dameng-00039.html'
                          },
                          {
                            id: 626,
                            parentId: 623,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'dameng-00040.html'
                          }
                        ]
                      },
                      {
                        id: 624,
                        parentId: 621,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'dameng-00041.html'
                      }
                    ]
                  },
                  {
                    id: 622,
                    parentId: 595,
                    name: 'Archiving Dameng Replication Copies',
                    local: 'dameng-00042.html',
                    children: [
                      {
                        id: 627,
                        parentId: 622,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'dameng-00043.html'
                      },
                      {
                        id: 628,
                        parentId: 622,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'dameng-00044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 596,
                parentId: 38,
                name: 'Restoration',
                local: 'dameng-00045.html',
                children: [
                  {
                    id: 629,
                    parentId: 596,
                    name: 'Restoring Dameng',
                    local: 'dameng-00048.html'
                  }
                ]
              },
              {
                id: 597,
                parentId: 38,
                name: 'Global Search',
                local: 'dameng-00027_a2.html',
                children: [
                  {
                    id: 630,
                    parentId: 597,
                    name: 'Global Search for Resources',
                    local: 'dameng-00027_a3.html'
                  },
                  {
                    id: 631,
                    parentId: 597,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'dameng-00027_a4.html'
                  }
                ]
              },
              {
                id: 598,
                parentId: 38,
                name: 'SLA ',
                local: 'dameng-00053.html',
                children: [
                  {
                    id: 632,
                    parentId: 598,
                    name: 'About SLA',
                    local: 'dameng-00054.html'
                  },
                  {
                    id: 633,
                    parentId: 598,
                    name: 'Viewing SLA Information',
                    local: 'dameng-00055.html'
                  },
                  {
                    id: 634,
                    parentId: 598,
                    name: 'Managing SLAs',
                    local: 'dameng-00056.html'
                  }
                ]
              },
              {
                id: 599,
                parentId: 38,
                name: 'Copies',
                local: 'dameng-00057.html',
                children: [
                  {
                    id: 635,
                    parentId: 599,
                    name: 'Viewing Dameng Copy Information',
                    local: 'dameng-00058.html'
                  },
                  {
                    id: 636,
                    parentId: 599,
                    name: 'Managing Dameng Copies',
                    local: 'dameng-00059.html'
                  }
                ]
              },
              {
                id: 600,
                parentId: 38,
                name: 'Dameng Environment',
                local: 'dameng-00060.html',
                children: [
                  {
                    id: 637,
                    parentId: 600,
                    name: 'Viewing Dameng Environment Information',
                    local: 'dameng-00061.html'
                  },
                  {
                    id: 638,
                    parentId: 600,
                    name: 'Managing Dameng Databases',
                    local: 'dameng-00062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 39,
            parentId: 13,
            name: 'Kingbase Data Protection',
            local: 'en-us_topic_0000001827039700.html',
            children: [
              {
                id: 639,
                parentId: 39,
                name: 'Backup',
                local: 'kingbase-00005.html',
                children: [
                  {
                    id: 647,
                    parentId: 639,
                    name: 'Preparing for the Backup',
                    local: 'dameng-00008_0.html'
                  },
                  {
                    id: 648,
                    parentId: 639,
                    name: 'Backing Up a Kingbase Instance',
                    local: 'kingbase-00008.html',
                    children: [
                      {
                        id: 649,
                        parentId: 648,
                        name: 'Step 1: Initializing sys_rman',
                        local: 'en-us_topic_0000002015631765.html'
                      },
                      {
                        id: 650,
                        parentId: 648,
                        name:
                          'Step 2: Registering the Database of a Single Kingbase Instance',
                        local: 'kingbase-00009.html'
                      },
                      {
                        id: 651,
                        parentId: 648,
                        name:
                          'Step 3: Registering the Database of a Kingbase Cluster Instance',
                        local: 'kingbase-00010.html'
                      },
                      {
                        id: 652,
                        parentId: 648,
                        name: 'Step 4: Creating a Rate Limiting Policy',
                        local: 'kingbase-00011.html'
                      },
                      {
                        id: 653,
                        parentId: 648,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'kingbase-00012.html'
                      },
                      {
                        id: 654,
                        parentId: 648,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'kingbase-00013.html'
                      },
                      {
                        id: 655,
                        parentId: 648,
                        name: 'Step 7: Executing Backup',
                        local: 'kingbase-00014.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 640,
                parentId: 39,
                name: 'Replication',
                local: 'oracle_gud_000035_2.html',
                children: [
                  {
                    id: 656,
                    parentId: 640,
                    name: 'Replicating Kingbase Copies',
                    local: 'kingbase-00020.html',
                    children: [
                      {
                        id: 657,
                        parentId: 656,
                        name: 'Planning the Replication Network',
                        local: 'kingbase-00021.html'
                      },
                      {
                        id: 658,
                        parentId: 656,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'fc_gud_0026_3.html'
                      },
                      {
                        id: 659,
                        parentId: 656,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_4.html'
                      },
                      {
                        id: 660,
                        parentId: 656,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'kingbase-00023.html'
                      },
                      {
                        id: 661,
                        parentId: 656,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'kingbase-00024.html'
                      },
                      {
                        id: 662,
                        parentId: 656,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'kingbase-00025.html'
                      },
                      {
                        id: 663,
                        parentId: 656,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'kingbase-00026.html'
                      },
                      {
                        id: 664,
                        parentId: 656,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'kingbase-00025_a1.html'
                      },
                      {
                        id: 665,
                        parentId: 656,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'kingbase-00027.html'
                      },
                      {
                        id: 666,
                        parentId: 656,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'kingbase-00028.html'
                      },
                      {
                        id: 667,
                        parentId: 656,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'kingbase-00029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 641,
                parentId: 39,
                name: 'Archiving',
                local: 'kingbase-00030.html',
                children: [
                  {
                    id: 668,
                    parentId: 641,
                    name: 'Archiving Kingbase Backup Copies',
                    local: 'kingbase-00033.html',
                    children: [
                      {
                        id: 670,
                        parentId: 668,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'kingbase-00034.html',
                        children: [
                          {
                            id: 672,
                            parentId: 670,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'kingbase-00035.html'
                          },
                          {
                            id: 673,
                            parentId: 670,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'kingbase-00036.html'
                          }
                        ]
                      },
                      {
                        id: 671,
                        parentId: 668,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'kingbase-00037.html'
                      }
                    ]
                  },
                  {
                    id: 669,
                    parentId: 641,
                    name: 'Archiving Kingbase Replication Copies',
                    local: 'kingbase-00038.html',
                    children: [
                      {
                        id: 674,
                        parentId: 669,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'kingbase-00039.html'
                      },
                      {
                        id: 675,
                        parentId: 669,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'kingbase-00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 642,
                parentId: 39,
                name: 'Restoration',
                local: 'kingbase-00041.html',
                children: [
                  {
                    id: 676,
                    parentId: 642,
                    name: 'Restoring a Kingbase Instance',
                    local: 'kingbase-00044.html'
                  }
                ]
              },
              {
                id: 643,
                parentId: 39,
                name: 'Global Search',
                local: 'kingbase-00025_a2.html',
                children: [
                  {
                    id: 677,
                    parentId: 643,
                    name: 'Global Search for Resources',
                    local: 'kingbase-00025_a3.html'
                  },
                  {
                    id: 678,
                    parentId: 643,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'kingbase-00025_a4.html'
                  }
                ]
              },
              {
                id: 644,
                parentId: 39,
                name: 'SLA ',
                local: 'kingbase-00049.html',
                children: [
                  {
                    id: 679,
                    parentId: 644,
                    name: 'About SLA',
                    local: 'kingbase-00049a1.html'
                  },
                  {
                    id: 680,
                    parentId: 644,
                    name: 'Viewing SLA Information',
                    local: 'kingbase-00051.html'
                  },
                  {
                    id: 681,
                    parentId: 644,
                    name: 'Managing SLAs',
                    local: 'kingbase-00052.html'
                  }
                ]
              },
              {
                id: 645,
                parentId: 39,
                name: 'Copies',
                local: 'kingbase-00053.html',
                children: [
                  {
                    id: 682,
                    parentId: 645,
                    name: 'Viewing Kingbase Copy Information',
                    local: 'kingbase-00054.html'
                  },
                  {
                    id: 683,
                    parentId: 645,
                    name: 'Managing Kingbase Copies',
                    local: 'kingbase-00055.html'
                  }
                ]
              },
              {
                id: 646,
                parentId: 39,
                name: 'Kingbase Cluster Environment',
                local: 'kingbase-00056.html',
                children: [
                  {
                    id: 684,
                    parentId: 646,
                    name: 'Checking the Kingbase Environment',
                    local: 'kingbase-00057.html'
                  },
                  {
                    id: 685,
                    parentId: 646,
                    name: 'Managing Kingbase',
                    local: 'kingbase-00058.html'
                  },
                  {
                    id: 686,
                    parentId: 646,
                    name: 'Managing Kingbase Database Clusters',
                    local: 'kingbase-00059.html'
                  }
                ]
              }
            ]
          },
          {
            id: 40,
            parentId: 13,
            name: 'GoldenDB Data Protection',
            local: 'en-us_topic_0000001873759373.html',
            children: [
              {
                id: 687,
                parentId: 40,
                name: 'Backup',
                local: 'goldendb-00005.html',
                children: [
                  {
                    id: 695,
                    parentId: 687,
                    name: 'Preparations for Backup',
                    local: 'goldendb-00008.html'
                  },
                  {
                    id: 696,
                    parentId: 687,
                    name: 'Backing Up a GoldenDB Database',
                    local: 'goldendb-00009.html',
                    children: [
                      {
                        id: 697,
                        parentId: 696,
                        name: 'Step 1: Registering a GoldenDB Cluster',
                        local: 'goldendb-00010.html'
                      },
                      {
                        id: 698,
                        parentId: 696,
                        name: 'Step 2: Creating a GoldenDB Instance',
                        local: 'goldendb-00011.html'
                      },
                      {
                        id: 699,
                        parentId: 696,
                        name: 'Step 3: Creating a Rate Limiting Policy ',
                        local: 'goldendb-00012.html'
                      },
                      {
                        id: 700,
                        parentId: 696,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'goldendb-00013.html'
                      },
                      {
                        id: 701,
                        parentId: 696,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'goldendb-00014.html'
                      },
                      {
                        id: 702,
                        parentId: 696,
                        name: 'Step 6: Performing Backup',
                        local: 'goldendb-00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 688,
                parentId: 40,
                name: 'Replication',
                local: 'oracle_gud_000035_0.html',
                children: [
                  {
                    id: 703,
                    parentId: 688,
                    name: 'Replicating GoldenDB Copies',
                    local: 'goldendb-00021.html',
                    children: [
                      {
                        id: 704,
                        parentId: 703,
                        name: 'Planning the Replication Network',
                        local: 'goldendb-00022.html'
                      },
                      {
                        id: 705,
                        parentId: 703,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'fc_gud_0026_0.html'
                      },
                      {
                        id: 706,
                        parentId: 703,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_0.html'
                      },
                      {
                        id: 707,
                        parentId: 703,
                        name: 'Step 2: (Optional) Creating an IPsec Policy',
                        local: 'goldendb-00024.html'
                      },
                      {
                        id: 708,
                        parentId: 703,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'goldendb-00025.html'
                      },
                      {
                        id: 709,
                        parentId: 703,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'goldendb-00026.html'
                      },
                      {
                        id: 710,
                        parentId: 703,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'goldendb-00027.html'
                      },
                      {
                        id: 711,
                        parentId: 703,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'goldendb-00031.html'
                      },
                      {
                        id: 712,
                        parentId: 703,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'goldendb-00028.html'
                      },
                      {
                        id: 713,
                        parentId: 703,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'goldendb-00029.html'
                      },
                      {
                        id: 714,
                        parentId: 703,
                        name:
                          'Step 8: (Optional) Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'goldendb-00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 689,
                parentId: 40,
                name: 'Archiving',
                local: 'goldendb-00031_0.html',
                children: [
                  {
                    id: 715,
                    parentId: 689,
                    name: 'Archiving GoldenDB Backup Copies',
                    local: 'goldendb-00034.html',
                    children: [
                      {
                        id: 717,
                        parentId: 715,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'goldendb-00035.html',
                        children: [
                          {
                            id: 719,
                            parentId: 717,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'goldendb-00036.html'
                          },
                          {
                            id: 720,
                            parentId: 717,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'goldendb-00037.html'
                          }
                        ]
                      },
                      {
                        id: 718,
                        parentId: 715,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'goldendb-00038.html'
                      }
                    ]
                  },
                  {
                    id: 716,
                    parentId: 689,
                    name: 'Archiving GoldenDB Replication Copies ',
                    local: 'goldendb-00039.html',
                    children: [
                      {
                        id: 721,
                        parentId: 716,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'goldendb-00040.html'
                      },
                      {
                        id: 722,
                        parentId: 716,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'goldendb-00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 690,
                parentId: 40,
                name: 'Restoration',
                local: 'goldendb-00042.html',
                children: [
                  {
                    id: 723,
                    parentId: 690,
                    name: 'Restoring GoldenDB',
                    local: 'goldendb-00045.html'
                  }
                ]
              },
              {
                id: 691,
                parentId: 40,
                name: 'Global Search',
                local: 'goldendb-00046.html',
                children: [
                  {
                    id: 724,
                    parentId: 691,
                    name: 'About Global Search',
                    local: 'goldendb-00051.html'
                  },
                  {
                    id: 725,
                    parentId: 691,
                    name: 'Global Search for Copies',
                    local: 'goldendb-00052.html'
                  },
                  {
                    id: 726,
                    parentId: 691,
                    name: 'Global Search for Resources',
                    local: 'goldendb-00048.html'
                  },
                  {
                    id: 727,
                    parentId: 691,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002838402.html'
                  }
                ]
              },
              {
                id: 692,
                parentId: 40,
                name: 'SLA',
                local: 'goldendb-00057.html',
                children: [
                  {
                    id: 728,
                    parentId: 692,
                    name: 'About SLA',
                    local: 'goldendb-00058.html'
                  },
                  {
                    id: 729,
                    parentId: 692,
                    name: 'Viewing SLA Information',
                    local: 'goldendb-00059.html'
                  },
                  {
                    id: 730,
                    parentId: 692,
                    name: 'Managing SLAs',
                    local: 'goldendb-00060.html'
                  }
                ]
              },
              {
                id: 693,
                parentId: 40,
                name: 'Copies',
                local: 'goldendb-00055.html',
                children: [
                  {
                    id: 731,
                    parentId: 693,
                    name: 'Viewing GoldenDB Copy Information',
                    local: 'goldendb-00056_0.html'
                  },
                  {
                    id: 732,
                    parentId: 693,
                    name: 'Managing GoldenDB Copies',
                    local: 'goldendb-00057_0.html'
                  }
                ]
              },
              {
                id: 694,
                parentId: 40,
                name: 'GoldenDB Cluster Environment',
                local: 'goldendb-00058_0.html',
                children: [
                  {
                    id: 733,
                    parentId: 694,
                    name: 'Viewing the GoldenDB Environment',
                    local: 'goldendb-00059_0.html'
                  },
                  {
                    id: 734,
                    parentId: 694,
                    name: 'Managing Instances',
                    local: 'goldendb-00060_0.html'
                  },
                  {
                    id: 735,
                    parentId: 694,
                    name: 'Management Clusters',
                    local: 'goldendb-00061.html'
                  }
                ]
              }
            ]
          },
          {
            id: 41,
            parentId: 13,
            name: 'GaussDB Data Protection',
            local: 'en-us_topic_0000001827039692.html',
            children: [
              {
                id: 736,
                parentId: 41,
                name: 'Backup',
                local: 'TPOPS_GaussDB_00006.html',
                children: [
                  {
                    id: 744,
                    parentId: 736,
                    name: 'Preparing for the Backup',
                    local: 'TPOPS_GaussDB_00009.html'
                  },
                  {
                    id: 745,
                    parentId: 736,
                    name: 'Backing Up GaussDB Instances',
                    local: 'TPOPS_GaussDB_00010.html',
                    children: [
                      {
                        id: 746,
                        parentId: 745,
                        name:
                          'Step 1: Obtaining the Address and Port of the Management Plane',
                        local: 'TPOPS_GaussDB_00014.html'
                      },
                      {
                        id: 747,
                        parentId: 745,
                        name:
                          'Step 2: Enabling the XBSA Backup Whitelist on the TPOPS Node',
                        local: 'TPOPS_GaussDB_00013.html'
                      },
                      {
                        id: 748,
                        parentId: 745,
                        name: 'Step 3: Registering the GaussDB Project',
                        local: 'TPOPS_GaussDB_00015.html'
                      },
                      {
                        id: 749,
                        parentId: 745,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'TPOPS_GaussDB_00016.html'
                      },
                      {
                        id: 750,
                        parentId: 745,
                        name: 'Step 5: Creating a Rate Limiting Policy',
                        local: 'TPOPS_GaussDB_00017.html'
                      },
                      {
                        id: 751,
                        parentId: 745,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'TPOPS_GaussDB_00018.html'
                      },
                      {
                        id: 752,
                        parentId: 745,
                        name: 'Step 7: Performing Backup',
                        local: 'TPOPS_GaussDB_00019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 737,
                parentId: 41,
                name: 'Replication',
                local: 'TPOPS_GaussDB_00022.html',
                children: [
                  {
                    id: 753,
                    parentId: 737,
                    name: 'Replicating GaussDB Copies',
                    local: 'TPOPS_GaussDB_00024.html',
                    children: [
                      {
                        id: 754,
                        parentId: 753,
                        name: 'Planning the Replication Network',
                        local: 'TPOPS_GaussDB_00025.html'
                      },
                      {
                        id: 755,
                        parentId: 753,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'TPOPS_GaussDB_00026.html'
                      },
                      {
                        id: 756,
                        parentId: 753,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'TPOPS_GaussDB_000260.html'
                      },
                      {
                        id: 757,
                        parentId: 753,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'TPOPS_GaussDB_00027.html'
                      },
                      {
                        id: 758,
                        parentId: 753,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'TPOPS_GaussDB_00028.html'
                      },
                      {
                        id: 759,
                        parentId: 753,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'TPOPS_GaussDB_00029.html'
                      },
                      {
                        id: 760,
                        parentId: 753,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'TPOPS_GaussDB_00030.html'
                      },
                      {
                        id: 761,
                        parentId: 753,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'TPOPS_GaussDB_000300.html'
                      },
                      {
                        id: 762,
                        parentId: 753,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'TPOPS_GaussDB_00031.html'
                      },
                      {
                        id: 763,
                        parentId: 753,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'TPOPS_GaussDB_00032.html'
                      },
                      {
                        id: 764,
                        parentId: 753,
                        name:
                          'Step 8: (Optional) Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'TPOPS_GaussDB_00033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 738,
                parentId: 41,
                name: 'Archiving',
                local: 'TPOPS_GaussDB_00034.html',
                children: [
                  {
                    id: 765,
                    parentId: 738,
                    name: 'Archiving GaussDB Backup Copies',
                    local: 'TPOPS_GaussDB_00037.html',
                    children: [
                      {
                        id: 767,
                        parentId: 765,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'TPOPS_GaussDB_00038.html',
                        children: [
                          {
                            id: 769,
                            parentId: 767,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'TPOPS_GaussDB_00039.html'
                          },
                          {
                            id: 770,
                            parentId: 767,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'TPOPS_GaussDB_00040.html'
                          }
                        ]
                      },
                      {
                        id: 768,
                        parentId: 765,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'TPOPS_GaussDB_00041.html'
                      }
                    ]
                  },
                  {
                    id: 766,
                    parentId: 738,
                    name: 'Archiving GaussDB Replication Copies',
                    local: 'TPOPS_GaussDB_00042.html',
                    children: [
                      {
                        id: 771,
                        parentId: 766,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'TPOPS_GaussDB_00043.html'
                      },
                      {
                        id: 772,
                        parentId: 766,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'TPOPS_GaussDB_00044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 739,
                parentId: 41,
                name: 'Restoration',
                local: 'TPOPS_GaussDB_00045.html',
                children: [
                  {
                    id: 773,
                    parentId: 739,
                    name: 'Restoring GaussDB Instances',
                    local: 'TPOPS_GaussDB_00048.html'
                  }
                ]
              },
              {
                id: 740,
                parentId: 41,
                name: 'Global Search',
                local: 'TPOPS_GaussDB_000451.html',
                children: [
                  {
                    id: 774,
                    parentId: 740,
                    name: 'Global Search for Resources',
                    local: 'TPOPS_GaussDB_00049.html'
                  },
                  {
                    id: 775,
                    parentId: 740,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'TPOPS_GaussDB_000492.html'
                  }
                ]
              },
              {
                id: 741,
                parentId: 41,
                name: 'SLA ',
                local: 'TPOPS_GaussDB_00052.html',
                children: [
                  {
                    id: 776,
                    parentId: 741,
                    name: 'About SLA ',
                    local: 'TPOPS_GaussDB_000540.html'
                  },
                  {
                    id: 777,
                    parentId: 741,
                    name: 'Viewing SLA Information',
                    local: 'TPOPS_GaussDB_00054.html'
                  },
                  {
                    id: 778,
                    parentId: 741,
                    name: 'Managing SLAs',
                    local: 'TPOPS_GaussDB_00055.html'
                  }
                ]
              },
              {
                id: 742,
                parentId: 41,
                name: 'Copies',
                local: 'TPOPS_GaussDB_00056.html',
                children: [
                  {
                    id: 779,
                    parentId: 742,
                    name: 'Viewing GaussDB Copy Information',
                    local: 'TPOPS_GaussDB_00057.html'
                  },
                  {
                    id: 780,
                    parentId: 742,
                    name: 'Managing GaussDB Copies',
                    local: 'TPOPS_GaussDB_00058.html'
                  }
                ]
              },
              {
                id: 743,
                parentId: 41,
                name: 'GaussDB',
                local: 'TPOPS_GaussDB_00059.html',
                children: [
                  {
                    id: 781,
                    parentId: 743,
                    name: 'Viewing GaussDB Information',
                    local: 'TPOPS_GaussDB_00060.html'
                  },
                  {
                    id: 782,
                    parentId: 743,
                    name: 'Managing GaussDB Projects',
                    local: 'TPOPS_GaussDB_00061.html'
                  },
                  {
                    id: 783,
                    parentId: 743,
                    name: 'Managing Instances',
                    local: 'TPOPS_GaussDB_00062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 42,
            parentId: 13,
            name: 'GBase 8a Data Protection',
            local: 'en-us_topic_0000001873759389.html',
            children: [
              {
                id: 784,
                parentId: 42,
                name: 'Backup',
                local: 'GBase_8a_00007.html',
                children: [
                  {
                    id: 792,
                    parentId: 784,
                    name: 'Preparations for Backup',
                    local: 'GBase_8a_00010.html'
                  },
                  {
                    id: 793,
                    parentId: 784,
                    name: 'Backing Up a GBase 8a Database',
                    local: 'GBase_8a_00011.html',
                    children: [
                      {
                        id: 794,
                        parentId: 793,
                        name: 'Step 1 Registering the GBase 8a Database',
                        local: 'GBase_8a_00012.html'
                      },
                      {
                        id: 795,
                        parentId: 793,
                        name: 'Step 2 Creating a Rate Limiting Policy',
                        local: 'GBase_8a_00013.html'
                      },
                      {
                        id: 796,
                        parentId: 793,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'GBase_8a_00014.html'
                      },
                      {
                        id: 797,
                        parentId: 793,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'GBase_8a_00015.html'
                      },
                      {
                        id: 798,
                        parentId: 793,
                        name: 'Step 5: Performing Backup',
                        local: 'GBase_8a_00016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 785,
                parentId: 42,
                name: 'Replication',
                local: 'oracle_gud_000035_8.html',
                children: [
                  {
                    id: 799,
                    parentId: 785,
                    name: 'Replicating a GBase 8a Database Copy',
                    local: 'GBase_8a_00021.html',
                    children: [
                      {
                        id: 800,
                        parentId: 799,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'GBase_8a_00023.html'
                      },
                      {
                        id: 801,
                        parentId: 799,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'GBase_8a_000231.html'
                      },
                      {
                        id: 802,
                        parentId: 799,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'GBase_8a_00024.html'
                      },
                      {
                        id: 803,
                        parentId: 799,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'GBase_8a_00025.html'
                      },
                      {
                        id: 804,
                        parentId: 799,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'GBase_8a_00026.html'
                      },
                      {
                        id: 805,
                        parentId: 799,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'GBase_8a_00027.html'
                      },
                      {
                        id: 806,
                        parentId: 799,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'GBase_8a_0002600.html'
                      },
                      {
                        id: 807,
                        parentId: 799,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'GBase_8a_00028.html'
                      },
                      {
                        id: 808,
                        parentId: 799,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'GBase_8a_00029.html'
                      },
                      {
                        id: 809,
                        parentId: 799,
                        name:
                          '(Optional) Step 8: Creating a Bidirectional Replication or Cascading Replication SLA',
                        local: 'GBase_8a_00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 786,
                parentId: 42,
                name: 'Archiving',
                local: 'GBase_8a_00031.html',
                children: [
                  {
                    id: 810,
                    parentId: 786,
                    name: 'Archiving GBase 8a Backup Copies',
                    local: 'GBase_8a_00034.html',
                    children: [
                      {
                        id: 812,
                        parentId: 810,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'GBase_8a_00035.html',
                        children: [
                          {
                            id: 814,
                            parentId: 812,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'GBase_8a_00036.html'
                          },
                          {
                            id: 815,
                            parentId: 812,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'GBase_8a_00037.html'
                          }
                        ]
                      },
                      {
                        id: 813,
                        parentId: 810,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'GBase_8a_00038.html'
                      }
                    ]
                  },
                  {
                    id: 811,
                    parentId: 786,
                    name: 'Archiving GBase 8a Replication Copies',
                    local: 'GBase_8a_00039.html',
                    children: [
                      {
                        id: 816,
                        parentId: 811,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'GBase_8a_00040.html'
                      },
                      {
                        id: 817,
                        parentId: 811,
                        name:
                          'Step 2 Creating a Periodic Archive Policy for Replication Copies',
                        local: 'GBase_8a_00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 787,
                parentId: 42,
                name: 'Restoration',
                local: 'GBase_8a_00042.html',
                children: [
                  {
                    id: 818,
                    parentId: 787,
                    name: 'Restoring a GBase 8a Database',
                    local: 'GBase_8a_00045.html'
                  }
                ]
              },
              {
                id: 788,
                parentId: 42,
                name: 'Global Search',
                local: 'GBase_8a_000411.html',
                children: [
                  {
                    id: 819,
                    parentId: 788,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'GBase_8a_000451.html'
                  }
                ]
              },
              {
                id: 789,
                parentId: 42,
                name: 'SLA ',
                local: 'GBase_8a_00049.html',
                children: [
                  {
                    id: 820,
                    parentId: 789,
                    name: 'Viewing SLA Information',
                    local: 'GBase_8a_00051.html'
                  },
                  {
                    id: 821,
                    parentId: 789,
                    name: 'Managing SLAs',
                    local: 'GBase_8a_00052.html'
                  }
                ]
              },
              {
                id: 790,
                parentId: 42,
                name: 'Copies',
                local: 'GBase_8a_00053.html',
                children: [
                  {
                    id: 822,
                    parentId: 790,
                    name: 'Viewing GBase 8a Copy Details',
                    local: 'GBase_8a_00054.html'
                  },
                  {
                    id: 823,
                    parentId: 790,
                    name: 'Managing GBase 8a Copies',
                    local: 'GBase_8a_00055.html'
                  }
                ]
              },
              {
                id: 791,
                parentId: 42,
                name: 'GBase 8a Database Environment',
                local: 'GBase_8a_00056.html',
                children: [
                  {
                    id: 824,
                    parentId: 791,
                    name:
                      'Viewing the GBase 8a Database Environment Information',
                    local: 'GBase_8a_00057.html'
                  },
                  {
                    id: 825,
                    parentId: 791,
                    name: 'Managing Databases',
                    local: 'GBase_8a_00058.html'
                  }
                ]
              }
            ]
          },
          {
            id: 43,
            parentId: 13,
            name: 'SAP HANA Data Protection',
            local: 'en-us_topic_0000001873679193.html',
            children: [
              {
                id: 826,
                parentId: 43,
                name: 'Backup',
                local: 'SAP_HANA_0008.html',
                children: [
                  {
                    id: 836,
                    parentId: 826,
                    name: 'Preparations for Backup',
                    local: 'SAP_HANA_0011.html'
                  },
                  {
                    id: 837,
                    parentId: 826,
                    name:
                      'Backing Up the SAP HANA Database (General Databases Path)',
                    local: 'SAP_HANA_0012.html',
                    children: [
                      {
                        id: 839,
                        parentId: 837,
                        name:
                          'Step 1: Registering the SAP HANA Database (File Backup Mode)',
                        local: 'SAP_HANA_0013.html'
                      },
                      {
                        id: 840,
                        parentId: 837,
                        name: 'Step 2 Creating a Rate Limiting Policy',
                        local: 'SAP_HANA_0014.html'
                      },
                      {
                        id: 841,
                        parentId: 837,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'SAP_HANA_0015.html'
                      },
                      {
                        id: 842,
                        parentId: 837,
                        name: 'Step 4: Configuring Log Backup.',
                        local: 'SAP_HANA_0016.html'
                      },
                      {
                        id: 843,
                        parentId: 837,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'en-us_topic_0000002058629252.html'
                      },
                      {
                        id: 844,
                        parentId: 837,
                        name: 'Step 6: Performing Backup',
                        local: 'SAP_HANA_00024_b2.html'
                      }
                    ]
                  },
                  {
                    id: 838,
                    parentId: 826,
                    name:
                      'Backing Up the SAP HANA Database (SAP HANA from the Applications Path) (Only for 1.6.0 and Later Versions)',
                    local: 'SAP_HANA_0019.html',
                    children: [
                      {
                        id: 845,
                        parentId: 838,
                        name:
                          'Step 1: Registering the SAP HANA Database (Backint Backup Mode)',
                        local: 'SAP_HANA_0020.html'
                      },
                      {
                        id: 846,
                        parentId: 838,
                        name: 'Step 2 Creating a Rate Limiting Policy',
                        local: 'SAP_HANA_0022.html'
                      },
                      {
                        id: 847,
                        parentId: 838,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'SAP_HANA_0023.html'
                      },
                      {
                        id: 848,
                        parentId: 838,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'en-us_topic_0000002058628320.html'
                      },
                      {
                        id: 849,
                        parentId: 838,
                        name: 'Step 5: Performing Backup',
                        local: 'SAP_HANA_0025.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 827,
                parentId: 43,
                name: 'Replication',
                local: 'SAP_HANA_0028.html',
                children: [
                  {
                    id: 850,
                    parentId: 827,
                    name: 'Replicating an SAP HANA Database Copy',
                    local: 'SAP_HANA_0031.html',
                    children: [
                      {
                        id: 851,
                        parentId: 850,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'SAP_HANA_0033.html'
                      },
                      {
                        id: 852,
                        parentId: 850,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'SAP_HANA_0034.html'
                      },
                      {
                        id: 853,
                        parentId: 850,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'SAP_HANA_0035.html'
                      },
                      {
                        id: 854,
                        parentId: 850,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'SAP_HANA_0036.html'
                      },
                      {
                        id: 855,
                        parentId: 850,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'SAP_HANA_0037.html'
                      },
                      {
                        id: 856,
                        parentId: 850,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'SAP_HANA_0038.html'
                      },
                      {
                        id: 857,
                        parentId: 850,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'SAP_HANA_00037_a1.html'
                      },
                      {
                        id: 858,
                        parentId: 850,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'SAP_HANA_0039.html'
                      },
                      {
                        id: 859,
                        parentId: 850,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'SAP_HANA_0040.html'
                      },
                      {
                        id: 860,
                        parentId: 850,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'SAP_HANA_0041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 828,
                parentId: 43,
                name: 'Archiving',
                local: 'SAP_HANA_0042.html',
                children: [
                  {
                    id: 861,
                    parentId: 828,
                    name: 'Archiving SAP HANA Backup Copies',
                    local: 'SAP_HANA_0045.html',
                    children: [
                      {
                        id: 863,
                        parentId: 861,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'SAP_HANA_0046.html',
                        children: [
                          {
                            id: 865,
                            parentId: 863,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'SAP_HANA_0047.html'
                          },
                          {
                            id: 866,
                            parentId: 863,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'SAP_HANA_0048.html'
                          }
                        ]
                      },
                      {
                        id: 864,
                        parentId: 861,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'SAP_HANA_0049.html'
                      }
                    ]
                  },
                  {
                    id: 862,
                    parentId: 828,
                    name: 'Archiving SAP HANA Replication Copies',
                    local: 'SAP_HANA_0050.html',
                    children: [
                      {
                        id: 867,
                        parentId: 862,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'SAP_HANA_0051.html'
                      },
                      {
                        id: 868,
                        parentId: 862,
                        name:
                          'Step 2 Creating a Periodic Archive Policy for Replication Copies',
                        local: 'SAP_HANA_0052.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 829,
                parentId: 43,
                name: 'Restoration',
                local: 'SAP_HANA_0054.html',
                children: [
                  {
                    id: 869,
                    parentId: 829,
                    name:
                      'Restoring an SAP HANA Database (General Database Path)',
                    local: 'SAP_HANA_0057.html'
                  }
                ]
              },
              {
                id: 830,
                parentId: 43,
                name: 'Global Search',
                local: 'SAP_HANA_00053_a1.html',
                children: [
                  {
                    id: 870,
                    parentId: 830,
                    name: 'Global Search for Resources',
                    local: 'SAP_HANA_00053_a2.html'
                  },
                  {
                    id: 871,
                    parentId: 830,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'SAP_HANA_00053_a3.html'
                  }
                ]
              },
              {
                id: 831,
                parentId: 43,
                name: 'SLA ',
                local: 'SAP_HANA_0063.html',
                children: [
                  {
                    id: 872,
                    parentId: 831,
                    name: 'About SLA',
                    local: 'SAP_HANA_00063_qe.html'
                  },
                  {
                    id: 873,
                    parentId: 831,
                    name: 'Viewing SLA Information',
                    local: 'SAP_HANA_0065.html'
                  },
                  {
                    id: 874,
                    parentId: 831,
                    name: 'Managing SLAs',
                    local: 'SAP_HANA_0066.html'
                  }
                ]
              },
              {
                id: 832,
                parentId: 43,
                name: 'Copies (General Databases Path)',
                local: 'SAP_HANA_0067.html',
                children: [
                  {
                    id: 875,
                    parentId: 832,
                    name: 'Viewing SAP HANA Copy Details',
                    local: 'SAP_HANA_0068.html'
                  },
                  {
                    id: 876,
                    parentId: 832,
                    name: 'Managing SAP HANA Database Copies',
                    local: 'SAP_HANA_0069.html'
                  }
                ]
              },
              {
                id: 833,
                parentId: 43,
                name:
                  'Copies (SAP HANA Accessed from the Applications Path) (Only for 1.6.0 and Later Versions)',
                local: 'SAP_HANA_00067_as11.html',
                children: [
                  {
                    id: 877,
                    parentId: 833,
                    name: 'Viewing SAP HANA Copy Details',
                    local: 'SAP_HANA_00067_as12.html'
                  },
                  {
                    id: 878,
                    parentId: 833,
                    name: 'Managing SAP HANA Database Copies',
                    local: 'SAP_HANA_00067_as13.html'
                  }
                ]
              },
              {
                id: 834,
                parentId: 43,
                name: 'SAP HANA Database Environment (General Database Path)',
                local: 'SAP_HANA_0070.html',
                children: [
                  {
                    id: 879,
                    parentId: 834,
                    name:
                      'Viewing the SAP HANA Database Environment Information',
                    local: 'SAP_HANA_0071.html'
                  },
                  {
                    id: 880,
                    parentId: 834,
                    name: 'Managing Databases',
                    local: 'SAP_HANA_0072.html'
                  }
                ]
              },
              {
                id: 835,
                parentId: 43,
                name:
                  'SAP HANA Database Environment (SAP HANA Accessed from the Applications Path)',
                local: 'SAP_HANA_0073.html',
                children: [
                  {
                    id: 881,
                    parentId: 835,
                    name:
                      'Viewing the SAP HANA Database Environment Information',
                    local: 'SAP_HANA_0074.html'
                  },
                  {
                    id: 882,
                    parentId: 835,
                    name: 'Managing Instances',
                    local: 'SAP_HANA_0075.html'
                  },
                  {
                    id: 883,
                    parentId: 835,
                    name: 'Managing Databases',
                    local: 'SAP_HANA_0076.html'
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
        name: 'Big Data',
        local: 'en-us_topic_0000001948269721.html',
        children: [
          {
            id: 884,
            parentId: 14,
            name: 'ClickHouse Data Protection',
            local: 'en-us_topic_0000001873759365.html',
            children: [
              {
                id: 892,
                parentId: 884,
                name: 'Backup',
                local: 'clickhouse-0003.html',
                children: [
                  {
                    id: 901,
                    parentId: 892,
                    name: 'Preparing for the Backup',
                    local: 'clickhouse-0006.html'
                  },
                  {
                    id: 902,
                    parentId: 892,
                    name: 'Backing Up the ClickHouse Database and Table Set',
                    local: 'clickhouse-0007.html',
                    children: [
                      {
                        id: 903,
                        parentId: 902,
                        name: 'Step 1: Registering a ClickHouse Cluster',
                        local: 'clickhouse-0008.html'
                      },
                      {
                        id: 904,
                        parentId: 902,
                        name: 'Step 2: Creating a ClickHouse Table Set',
                        local: 'clickhouse-0009.html'
                      },
                      {
                        id: 905,
                        parentId: 902,
                        name: 'Step 3: Creating a Rate Limiting Policy',
                        local: 'clickhouse-0010.html'
                      },
                      {
                        id: 906,
                        parentId: 902,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'clickhouse-0011.html'
                      },
                      {
                        id: 907,
                        parentId: 902,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'clickhouse-0012.html'
                      },
                      {
                        id: 908,
                        parentId: 902,
                        name: 'Step 6: Performing Backup',
                        local: 'clickhouse-0013.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 893,
                parentId: 884,
                name: 'Replication',
                local: 'oracle_gud_000035_7.html',
                children: [
                  {
                    id: 909,
                    parentId: 893,
                    name: 'ClickHouse Replication Copy',
                    local: 'clickhouse-0019.html',
                    children: [
                      {
                        id: 910,
                        parentId: 909,
                        name: 'Planning the Replication Network',
                        local: 'clickhouse-0020.html'
                      },
                      {
                        id: 911,
                        parentId: 909,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'fc_gud_0026_6.html'
                      },
                      {
                        id: 912,
                        parentId: 909,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_9.html'
                      },
                      {
                        id: 913,
                        parentId: 909,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'clickhouse-0022.html'
                      },
                      {
                        id: 914,
                        parentId: 909,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'clickhouse-0023.html'
                      },
                      {
                        id: 915,
                        parentId: 909,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'clickhouse-0024.html'
                      },
                      {
                        id: 916,
                        parentId: 909,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'clickhouse-0025.html'
                      },
                      {
                        id: 917,
                        parentId: 909,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002010575970.html'
                      },
                      {
                        id: 918,
                        parentId: 909,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'clickhouse-0026.html'
                      },
                      {
                        id: 919,
                        parentId: 909,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'oracle_gud_000041.html'
                      },
                      {
                        id: 920,
                        parentId: 909,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication/Cascading Replication SLA',
                        local: 'clickhouse-0028.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 894,
                parentId: 884,
                name: 'Archiving',
                local: 'clickhouse-0029.html',
                children: [
                  {
                    id: 921,
                    parentId: 894,
                    name: 'Archiving ClickHouse Backup Copies',
                    local: 'clickhouse-0032.html',
                    children: [
                      {
                        id: 923,
                        parentId: 921,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'clickhouse-0033.html',
                        children: [
                          {
                            id: 925,
                            parentId: 923,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'clickhouse-0034.html'
                          },
                          {
                            id: 926,
                            parentId: 923,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'clickhouse-0035.html'
                          }
                        ]
                      },
                      {
                        id: 924,
                        parentId: 921,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'clickhouse-0036.html'
                      }
                    ]
                  },
                  {
                    id: 922,
                    parentId: 894,
                    name: 'Archiving ClickHouse Replication Copies',
                    local: 'clickhouse-0037.html',
                    children: [
                      {
                        id: 927,
                        parentId: 922,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'clickhouse-0038.html'
                      },
                      {
                        id: 928,
                        parentId: 922,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'clickhouse-0039.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 895,
                parentId: 884,
                name: 'Restoration',
                local: 'clickhouse-0040.html',
                children: [
                  {
                    id: 929,
                    parentId: 895,
                    name: 'Restoring a ClickHouse Database or Table Set',
                    local: 'clickhouse-0043.html'
                  }
                ]
              },
              {
                id: 896,
                parentId: 884,
                name: 'Global Search',
                local: 'en-us_topic_0000002038764373.html',
                children: [
                  {
                    id: 930,
                    parentId: 896,
                    name: 'Global Search for Resources',
                    local: 'clickhouse-0044.html'
                  },
                  {
                    id: 931,
                    parentId: 896,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002844410.html'
                  }
                ]
              },
              {
                id: 897,
                parentId: 884,
                name:
                  'Data Deduplication and Compression (Applicable to OceanProtect X Series Backup Appliances and OceanProtect E1000)',
                local: 'clickhouse-0045.html',
                children: [
                  {
                    id: 932,
                    parentId: 897,
                    name: 'About Data Deduplication and Compression',
                    local: 'clickhouse-0046.html'
                  }
                ]
              },
              {
                id: 898,
                parentId: 884,
                name: 'SLA',
                local: 'clickhouse-0047.html',
                children: [
                  {
                    id: 933,
                    parentId: 898,
                    name: 'About SLA',
                    local: 'clickhouse-0048.html'
                  },
                  {
                    id: 934,
                    parentId: 898,
                    name: 'Viewing SLA Information',
                    local: 'clickhouse-0049.html'
                  },
                  {
                    id: 935,
                    parentId: 898,
                    name: 'Managing SLAs',
                    local: 'clickhouse-0050.html'
                  }
                ]
              },
              {
                id: 899,
                parentId: 884,
                name: 'Copies',
                local: 'clickhouse-0051.html',
                children: [
                  {
                    id: 936,
                    parentId: 899,
                    name: 'Viewing ClickHouse Copy Information',
                    local: 'clickhouse-0052.html'
                  },
                  {
                    id: 937,
                    parentId: 899,
                    name: 'Managing ClickHouse Copies',
                    local: 'clickhouse-0053.html'
                  }
                ]
              },
              {
                id: 900,
                parentId: 884,
                name: 'ClickHouse Cluster Environment',
                local: 'clickhouse-0054.html',
                children: [
                  {
                    id: 938,
                    parentId: 900,
                    name: 'Querying ClickHouse Information',
                    local: 'clickhouse-0055.html'
                  },
                  {
                    id: 939,
                    parentId: 900,
                    name: 'Managing ClickHouse Clusters/Table Sets',
                    local: 'clickhouse-0056.html'
                  },
                  {
                    id: 940,
                    parentId: 900,
                    name: 'Managing ClickHouse Database/Table Set Protection',
                    local: 'clickhouse-0057.html'
                  }
                ]
              }
            ]
          },
          {
            id: 885,
            parentId: 14,
            name: 'GaussDB (DWS) Data Protection',
            local: 'product_documentation_000029.html',
            children: [
              {
                id: 941,
                parentId: 885,
                name: 'Backup',
                local: 'DWS_00006.html',
                children: [
                  {
                    id: 950,
                    parentId: 941,
                    name: 'Preparing for the Backup',
                    local: 'DWS_00009.html'
                  },
                  {
                    id: 951,
                    parentId: 941,
                    name: 'Backing Up GaussDB (DWS)',
                    local: 'DWS_00010.html',
                    children: [
                      {
                        id: 952,
                        parentId: 951,
                        name: 'Step 1: Registering a GaussDB (DWS) Cluster',
                        local: 'DWS_00014.html'
                      },
                      {
                        id: 953,
                        parentId: 951,
                        name:
                          'Step 2: Creating a GaussDB (DWS) Schema Set/Table Set',
                        local: 'DWS_00015.html'
                      },
                      {
                        id: 954,
                        parentId: 951,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'DWS_00016.html'
                      },
                      {
                        id: 955,
                        parentId: 951,
                        name: 'Step 4: Creating a Rate Limiting Policy',
                        local: 'DWS_00017.html'
                      },
                      {
                        id: 956,
                        parentId: 951,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'DWS_00018.html'
                      },
                      {
                        id: 957,
                        parentId: 951,
                        name: 'Step 6: Performing Backup',
                        local: 'DWS_00019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 942,
                parentId: 885,
                name: 'Replication',
                local: 'DWS_00022.html',
                children: [
                  {
                    id: 958,
                    parentId: 942,
                    name: 'Replicating a GaussDB (DWS) Backup Copies',
                    local: 'DWS_00024.html',
                    children: [
                      {
                        id: 959,
                        parentId: 958,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'DWS_00026.html'
                      },
                      {
                        id: 960,
                        parentId: 958,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_15.html'
                      },
                      {
                        id: 961,
                        parentId: 958,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'DWS_00027.html'
                      },
                      {
                        id: 962,
                        parentId: 958,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'DWS_00028.html'
                      },
                      {
                        id: 963,
                        parentId: 958,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'DWS_00029.html'
                      },
                      {
                        id: 964,
                        parentId: 958,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'DWS_00030.html'
                      },
                      {
                        id: 965,
                        parentId: 958,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'DWS_00031_a1.html'
                      },
                      {
                        id: 966,
                        parentId: 958,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'DWS_00031.html'
                      },
                      {
                        id: 967,
                        parentId: 958,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'DWS_00031_b1.html'
                      },
                      {
                        id: 968,
                        parentId: 958,
                        name:
                          'Step 8: (Optional) Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'DWS_00033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 943,
                parentId: 885,
                name: 'Archiving',
                local: 'DWS_00034.html',
                children: [
                  {
                    id: 969,
                    parentId: 943,
                    name: 'Archiving GaussDB (DWS) Backup Copies',
                    local: 'DWS_00037.html',
                    children: [
                      {
                        id: 971,
                        parentId: 969,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'DWS_00038.html',
                        children: [
                          {
                            id: 973,
                            parentId: 971,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'DWS_00039.html'
                          },
                          {
                            id: 974,
                            parentId: 971,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'DWS_00040.html'
                          }
                        ]
                      },
                      {
                        id: 972,
                        parentId: 969,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'DWS_00041.html'
                      }
                    ]
                  },
                  {
                    id: 970,
                    parentId: 943,
                    name: 'Archiving GaussDB (DWS) Replication Copies',
                    local: 'DWS_00042.html',
                    children: [
                      {
                        id: 975,
                        parentId: 970,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'DWS_00043.html'
                      },
                      {
                        id: 976,
                        parentId: 970,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'DWS_00044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 944,
                parentId: 885,
                name: 'Restoration',
                local: 'DWS_00045.html',
                children: [
                  {
                    id: 977,
                    parentId: 944,
                    name: 'Restoring the GaussDB (DWS)',
                    local: 'DWS_00048.html'
                  }
                ]
              },
              {
                id: 945,
                parentId: 885,
                name: 'Global Search',
                local: 'DWS_00045_a1.html',
                children: [
                  {
                    id: 978,
                    parentId: 945,
                    name: 'Global Search for Resources',
                    local: 'DWS_00045_a2.html'
                  },
                  {
                    id: 979,
                    parentId: 945,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'DWS_00045_a3.html'
                  }
                ]
              },
              {
                id: 946,
                parentId: 885,
                name:
                  'Data Deduplication and Compression (Applicable to OceanProtect X Series Backup Appliances and OceanProtect E1000)',
                local: 'vmware_gud_000088_0.html',
                children: [
                  {
                    id: 980,
                    parentId: 946,
                    name: 'About Data Deduplication and Compression',
                    local: 'vmware_gud_000089_0.html'
                  }
                ]
              },
              {
                id: 947,
                parentId: 885,
                name: 'SLA',
                local: 'DWS_00052.html',
                children: [
                  {
                    id: 981,
                    parentId: 947,
                    name: 'About SLA',
                    local: 'DWS_00053.html'
                  },
                  {
                    id: 982,
                    parentId: 947,
                    name: 'Viewing SLA Information',
                    local: 'DWS_00054.html'
                  },
                  {
                    id: 983,
                    parentId: 947,
                    name: 'Managing SLAs',
                    local: 'DWS_00055.html'
                  }
                ]
              },
              {
                id: 948,
                parentId: 885,
                name: 'Copies',
                local: 'DWS_00056.html',
                children: [
                  {
                    id: 984,
                    parentId: 948,
                    name: 'Viewing GaussDB (DWS) Copy Information',
                    local: 'DWS_00057.html'
                  },
                  {
                    id: 985,
                    parentId: 948,
                    name: 'Managing GaussDB (DWS) Copies',
                    local: 'DWS_00058.html'
                  }
                ]
              },
              {
                id: 949,
                parentId: 885,
                name: 'GaussDB (DWS) Cluster Environment',
                local: 'DWS_00059.html',
                children: [
                  {
                    id: 986,
                    parentId: 949,
                    name: 'Querying GaussDB (DWS) Information',
                    local: 'DWS_00060.html'
                  },
                  {
                    id: 987,
                    parentId: 949,
                    name: 'Managing the GaussDB (DWS) Cluster',
                    local: 'DWS_00061.html'
                  },
                  {
                    id: 988,
                    parentId: 949,
                    name: 'Managing GaussDB (DWS)',
                    local: 'DWS_00062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 886,
            parentId: 14,
            name: 'HBase Data Protection',
            local: 'product_documentation_000033.html',
            children: [
              {
                id: 989,
                parentId: 886,
                name: 'Backup',
                local: 'hbase_00007.html',
                children: [
                  {
                    id: 997,
                    parentId: 989,
                    name: 'Preparing for the Backup',
                    local: 'hbase_00010.html'
                  },
                  {
                    id: 998,
                    parentId: 989,
                    name: 'Backing Up HBase Backup Sets',
                    local: 'hbase_00011.html',
                    children: [
                      {
                        id: 999,
                        parentId: 998,
                        name: 'Step 1: Registering an HBase Cluster',
                        local: 'hbase_00012.html'
                      },
                      {
                        id: 1000,
                        parentId: 998,
                        name: 'Step 2: Creating an HBase Backup Set',
                        local: 'hbase_00013.html'
                      },
                      {
                        id: 1001,
                        parentId: 998,
                        name: 'Step 3: Creating a Rate Limiting Policy',
                        local: 'hbase_00014.html'
                      },
                      {
                        id: 1002,
                        parentId: 998,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'hbase_00015.html'
                      },
                      {
                        id: 1003,
                        parentId: 998,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'hbase_00016.html'
                      },
                      {
                        id: 1004,
                        parentId: 998,
                        name: 'Step 6: Performing Backup',
                        local: 'hbase_00017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 990,
                parentId: 886,
                name: 'Replication',
                local: 'hbase_00020.html',
                children: [
                  {
                    id: 1005,
                    parentId: 990,
                    name: 'Replicating HBase Copies',
                    local: 'hbase_00023.html',
                    children: [
                      {
                        id: 1006,
                        parentId: 1005,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'hbase_00025.html'
                      },
                      {
                        id: 1007,
                        parentId: 1005,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'hbase_00026_a1.html'
                      },
                      {
                        id: 1008,
                        parentId: 1005,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'hbase_00026.html'
                      },
                      {
                        id: 1009,
                        parentId: 1005,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'hbase_00028_a1.html'
                      },
                      {
                        id: 1010,
                        parentId: 1005,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'hbase_00028.html'
                      },
                      {
                        id: 1011,
                        parentId: 1005,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'hbase_00029.html'
                      },
                      {
                        id: 1012,
                        parentId: 1005,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002046619645.html'
                      },
                      {
                        id: 1013,
                        parentId: 1005,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'hbase_00030.html'
                      },
                      {
                        id: 1014,
                        parentId: 1005,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'hbase_00031.html'
                      },
                      {
                        id: 1015,
                        parentId: 1005,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'hbase_00032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 991,
                parentId: 886,
                name: 'Archiving',
                local: 'hbase_00033.html',
                children: [
                  {
                    id: 1016,
                    parentId: 991,
                    name: 'Archiving HBase Backup Copies',
                    local: 'hbase_00036.html',
                    children: [
                      {
                        id: 1018,
                        parentId: 1016,
                        name: 'Adding an Archive Storage',
                        local: 'hbase_00037.html',
                        children: [
                          {
                            id: 1020,
                            parentId: 1018,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'hbase_00038.html'
                          },
                          {
                            id: 1021,
                            parentId: 1018,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'hbase_00039.html'
                          }
                        ]
                      },
                      {
                        id: 1019,
                        parentId: 1016,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'hbase_00040.html'
                      }
                    ]
                  },
                  {
                    id: 1017,
                    parentId: 991,
                    name: 'Archiving HBase Replication Copies',
                    local: 'hbase_00041.html',
                    children: [
                      {
                        id: 1022,
                        parentId: 1017,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'hbase_00042.html'
                      },
                      {
                        id: 1023,
                        parentId: 1017,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'hbase_00043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 992,
                parentId: 886,
                name: 'Restoration',
                local: 'hbase_00044.html',
                children: [
                  {
                    id: 1024,
                    parentId: 992,
                    name: 'Restoring HBase Backup Sets',
                    local: 'hbase_00048_a1.html'
                  },
                  {
                    id: 1025,
                    parentId: 992,
                    name:
                      'Restoring One or Multiple Tables in an HBase Backup Set',
                    local: 'hbase_00048.html'
                  }
                ]
              },
              {
                id: 993,
                parentId: 886,
                name: 'Global Search',
                local: 'hbase_00049.html',
                children: [
                  {
                    id: 1026,
                    parentId: 993,
                    name: 'Global Search for Resources',
                    local: 'hbase_00050.html'
                  },
                  {
                    id: 1027,
                    parentId: 993,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002038765521.html'
                  }
                ]
              },
              {
                id: 994,
                parentId: 886,
                name: 'SLA',
                local: 'hbase_00053.html',
                children: [
                  {
                    id: 1028,
                    parentId: 994,
                    name: 'Viewing SLA Information',
                    local: 'hbase_00055.html'
                  },
                  {
                    id: 1029,
                    parentId: 994,
                    name: 'Managing SLAs',
                    local: 'hbase_00056.html'
                  }
                ]
              },
              {
                id: 995,
                parentId: 886,
                name: 'Copies',
                local: 'hbase_00057.html',
                children: [
                  {
                    id: 1030,
                    parentId: 995,
                    name: 'Viewing HBase Copy Information',
                    local: 'hbase_00060_a1.html'
                  },
                  {
                    id: 1031,
                    parentId: 995,
                    name: 'Managing HBase Copies',
                    local: 'hbase_00059.html'
                  }
                ]
              },
              {
                id: 996,
                parentId: 886,
                name: 'HBase Cluster Environment',
                local: 'hbase_00060.html',
                children: [
                  {
                    id: 1032,
                    parentId: 996,
                    name: 'Querying HBase Information',
                    local: 'hbase_00061.html'
                  },
                  {
                    id: 1033,
                    parentId: 996,
                    name: 'Managing HBase Clusters',
                    local: 'hbase_00064_a1.html'
                  },
                  {
                    id: 1034,
                    parentId: 996,
                    name: 'Managing Backup Set Protection',
                    local: 'hbase_00065_a1.html'
                  }
                ]
              }
            ]
          },
          {
            id: 887,
            parentId: 14,
            name: 'Hive Data Protection',
            local: 'en-us_topic_0000001827039684.html',
            children: [
              {
                id: 1035,
                parentId: 887,
                name: 'Overview',
                local: 'hive_00003.html',
                children: [
                  {
                    id: 1044,
                    parentId: 1035,
                    name: 'Function Overview',
                    local: 'hive_00005.html'
                  },
                  {
                    id: 1045,
                    parentId: 1035,
                    name: 'Constraints',
                    local: 'hive_00006.html'
                  }
                ]
              },
              {
                id: 1036,
                parentId: 887,
                name: 'Backup',
                local: 'hive_00008.html',
                children: [
                  {
                    id: 1046,
                    parentId: 1036,
                    name: 'Preparing for the Backup',
                    local: 'hive_00011.html'
                  },
                  {
                    id: 1047,
                    parentId: 1036,
                    name: 'Backing Up Hive Backup Sets',
                    local: 'hive_00012.html',
                    children: [
                      {
                        id: 1048,
                        parentId: 1047,
                        name:
                          'Step 1: Enabling the Snapshot Function for the Directory Where the Database Table Resides',
                        local: 'hive_00013.html'
                      },
                      {
                        id: 1049,
                        parentId: 1047,
                        name:
                          'Step 2: (Optional) Generating and Obtaining a Certificate',
                        local: 'hive_00015.html'
                      },
                      {
                        id: 1050,
                        parentId: 1047,
                        name: 'Step 3: Registering a Hive Cluster',
                        local: 'hive_00016.html'
                      },
                      {
                        id: 1051,
                        parentId: 1047,
                        name: 'Step 4: Creating a Hive Backup Set',
                        local: 'hive_00017.html'
                      },
                      {
                        id: 1052,
                        parentId: 1047,
                        name: 'Step 5: Creating a Rate Limiting Policy',
                        local: 'hive_00018.html'
                      },
                      {
                        id: 1053,
                        parentId: 1047,
                        name:
                          'Step 6: (Optional) Enabling Backup Link Encryption',
                        local: 'hive_00018_a1.html'
                      },
                      {
                        id: 1054,
                        parentId: 1047,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'hive_00020.html'
                      },
                      {
                        id: 1055,
                        parentId: 1047,
                        name: 'Step 8: Performing Backup',
                        local: 'hive_00021.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1037,
                parentId: 887,
                name: 'Replication',
                local: 'hive_00024.html',
                children: [
                  {
                    id: 1056,
                    parentId: 1037,
                    name: 'Replicating Hive Copies',
                    local: 'hive_00027.html',
                    children: [
                      {
                        id: 1057,
                        parentId: 1056,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'hive_00029.html'
                      },
                      {
                        id: 1058,
                        parentId: 1056,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'hive_00030.html'
                      },
                      {
                        id: 1059,
                        parentId: 1056,
                        name: '(Optional) Step 2: Creating an IPsec Policy ',
                        local: 'hive_00031.html'
                      },
                      {
                        id: 1060,
                        parentId: 1056,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'hive_00032.html'
                      },
                      {
                        id: 1061,
                        parentId: 1056,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'hive_00033.html'
                      },
                      {
                        id: 1062,
                        parentId: 1056,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'hive_00034.html'
                      },
                      {
                        id: 1063,
                        parentId: 1056,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002010579886.html'
                      },
                      {
                        id: 1064,
                        parentId: 1056,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'hive_00035.html'
                      },
                      {
                        id: 1065,
                        parentId: 1056,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'hive_00036.html'
                      },
                      {
                        id: 1066,
                        parentId: 1056,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'hive_00037.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1038,
                parentId: 887,
                name: 'Archiving',
                local: 'hive_00038.html',
                children: [
                  {
                    id: 1067,
                    parentId: 1038,
                    name: 'Archiving Hive Backup Copies',
                    local: 'hive_00041.html',
                    children: [
                      {
                        id: 1069,
                        parentId: 1067,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'hive_00042.html',
                        children: [
                          {
                            id: 1071,
                            parentId: 1069,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'hive_00043.html'
                          },
                          {
                            id: 1072,
                            parentId: 1069,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'hive_00044.html'
                          }
                        ]
                      },
                      {
                        id: 1070,
                        parentId: 1067,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'hive_00045.html'
                      }
                    ]
                  },
                  {
                    id: 1068,
                    parentId: 1038,
                    name: 'Archiving Hive Replication Copies',
                    local: 'hive_00046.html',
                    children: [
                      {
                        id: 1073,
                        parentId: 1068,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'hive_00047.html'
                      },
                      {
                        id: 1074,
                        parentId: 1068,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'hive_00048.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1039,
                parentId: 887,
                name: 'Restoration',
                local: 'hive_00049.html',
                children: [
                  {
                    id: 1075,
                    parentId: 1039,
                    name: 'Restoring Hive Backup Sets',
                    local: 'hive_00051.html'
                  },
                  {
                    id: 1076,
                    parentId: 1039,
                    name:
                      'Restoring One or Multiple Tables in a Hive Backup Set',
                    local: 'hive_00052.html'
                  }
                ]
              },
              {
                id: 1040,
                parentId: 887,
                name: 'Global Search',
                local: 'hive_00053.html',
                children: [
                  {
                    id: 1077,
                    parentId: 1040,
                    name: 'Global Search for Resources',
                    local: 'hive_00054.html'
                  },
                  {
                    id: 1078,
                    parentId: 1040,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002845722.html'
                  }
                ]
              },
              {
                id: 1041,
                parentId: 887,
                name: 'SLA',
                local: 'hive_00057.html',
                children: [
                  {
                    id: 1079,
                    parentId: 1041,
                    name: 'Viewing SLA Information',
                    local: 'hive_00059.html'
                  },
                  {
                    id: 1080,
                    parentId: 1041,
                    name: 'Managing SLAs',
                    local: 'hive_00060.html'
                  }
                ]
              },
              {
                id: 1042,
                parentId: 887,
                name: 'Copies',
                local: 'hive_00061.html',
                children: [
                  {
                    id: 1081,
                    parentId: 1042,
                    name: 'Viewing Hive Copy Information',
                    local: 'hive_00062.html'
                  },
                  {
                    id: 1082,
                    parentId: 1042,
                    name: 'Managing Hive Copies',
                    local: 'hive_00063.html'
                  }
                ]
              },
              {
                id: 1043,
                parentId: 887,
                name: 'Hive Cluster Environment',
                local: 'hive_00064.html',
                children: [
                  {
                    id: 1083,
                    parentId: 1043,
                    name: 'Querying Hive Information',
                    local: 'hive_00065.html'
                  },
                  {
                    id: 1084,
                    parentId: 1043,
                    name: 'Managing Hive Clusters',
                    local: 'hive_00066.html'
                  },
                  {
                    id: 1085,
                    parentId: 1043,
                    name: 'Managing Backup Sets',
                    local: 'hive_00067.html'
                  }
                ]
              }
            ]
          },
          {
            id: 888,
            parentId: 14,
            name: 'MongoDB Data Protection',
            local: 'en-us_topic_0000001873679221.html',
            children: [
              {
                id: 1086,
                parentId: 888,
                name: 'Backup',
                local: 'mongodb-0007.html',
                children: [
                  {
                    id: 1094,
                    parentId: 1086,
                    name: 'Preparations for Backup',
                    local: 'mongodb-0010.html'
                  },
                  {
                    id: 1095,
                    parentId: 1086,
                    name: 'Backing Up MongoDB Databases',
                    local: 'mongodb-0011.html',
                    children: [
                      {
                        id: 1096,
                        parentId: 1095,
                        name: 'Step 1: Registering a MongoDB Instance',
                        local: 'mongodb-0012.html'
                      },
                      {
                        id: 1097,
                        parentId: 1095,
                        name:
                          'Step 2: (Optional) Creating a Rate Limiting Policy',
                        local: 'mongodb-0013.html'
                      },
                      {
                        id: 1098,
                        parentId: 1095,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'mongodb-0014.html'
                      },
                      {
                        id: 1099,
                        parentId: 1095,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'mongodb-0015.html'
                      },
                      {
                        id: 1100,
                        parentId: 1095,
                        name: 'Step 5: Performing Backup',
                        local: 'mongodb-0016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1087,
                parentId: 888,
                name: 'Replication',
                local: 'oracle_gud_000035_6.html',
                children: [
                  {
                    id: 1101,
                    parentId: 1087,
                    name: 'Replicating MongoDB Copies',
                    local: 'mongodb-0022.html',
                    children: [
                      {
                        id: 1102,
                        parentId: 1101,
                        name: 'Planning the Replication Network',
                        local: 'mongodb-0023.html'
                      },
                      {
                        id: 1103,
                        parentId: 1101,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'fc_gud_0026_5.html'
                      },
                      {
                        id: 1104,
                        parentId: 1101,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_8.html'
                      },
                      {
                        id: 1105,
                        parentId: 1101,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'mongodb-0025.html'
                      },
                      {
                        id: 1106,
                        parentId: 1101,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'mongodb-0026.html'
                      },
                      {
                        id: 1107,
                        parentId: 1101,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'mongodb-0027.html'
                      },
                      {
                        id: 1108,
                        parentId: 1101,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'mongodb-0028.html'
                      },
                      {
                        id: 1109,
                        parentId: 1101,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'mongodb-0028_0.html'
                      },
                      {
                        id: 1110,
                        parentId: 1101,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'mongodb-0029.html'
                      },
                      {
                        id: 1111,
                        parentId: 1101,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'mongodb-0030.html'
                      },
                      {
                        id: 1112,
                        parentId: 1101,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'mongodb-0031.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1088,
                parentId: 888,
                name: 'Archiving',
                local: 'mongodb-0032.html',
                children: [
                  {
                    id: 1113,
                    parentId: 1088,
                    name: 'Archiving MongoDB Backup Copies',
                    local: 'mongodb-0035.html',
                    children: [
                      {
                        id: 1115,
                        parentId: 1113,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'mongodb-0036.html',
                        children: [
                          {
                            id: 1117,
                            parentId: 1115,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'mongodb-0037.html'
                          },
                          {
                            id: 1118,
                            parentId: 1115,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'mongodb-0038.html'
                          }
                        ]
                      },
                      {
                        id: 1116,
                        parentId: 1113,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'mongodb-0039.html'
                      }
                    ]
                  },
                  {
                    id: 1114,
                    parentId: 1088,
                    name: 'Archiving MongoDB Replication Copies',
                    local: 'mongodb-0040.html',
                    children: [
                      {
                        id: 1119,
                        parentId: 1114,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'mongodb-0041.html'
                      },
                      {
                        id: 1120,
                        parentId: 1114,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'mongodb-0042.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1089,
                parentId: 888,
                name: 'Restoration',
                local: 'mongodb-0043.html',
                children: [
                  {
                    id: 1121,
                    parentId: 1089,
                    name: 'Restoring MongoDB',
                    local: 'mongodb-0046.html'
                  }
                ]
              },
              {
                id: 1090,
                parentId: 888,
                name: 'Global Search',
                local: 'mongodb-0047.html',
                children: [
                  {
                    id: 1122,
                    parentId: 1090,
                    name: 'About Global Search',
                    local: 'en-us_topic_0000002093900072.html'
                  },
                  {
                    id: 1123,
                    parentId: 1090,
                    name: 'Global Search for Copies',
                    local: 'vmware_gud_0076_1.html'
                  },
                  {
                    id: 1124,
                    parentId: 1090,
                    name: 'Global Search for Resources',
                    local: 'mongodb-0048.html'
                  },
                  {
                    id: 1125,
                    parentId: 1090,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002841954.html'
                  }
                ]
              },
              {
                id: 1091,
                parentId: 888,
                name: 'SLA ',
                local: 'en-us_topic_0000001954665865.html',
                children: [
                  {
                    id: 1126,
                    parentId: 1091,
                    name: 'About SLA',
                    local: 'vmware_gud_000026_0.html'
                  },
                  {
                    id: 1127,
                    parentId: 1091,
                    name: 'Viewing SLA Information',
                    local: 'en-us_topic_0000001927506528.html'
                  },
                  {
                    id: 1128,
                    parentId: 1091,
                    name: 'Managing SLAs',
                    local: 'en-us_topic_0000001927347172.html'
                  }
                ]
              },
              {
                id: 1092,
                parentId: 888,
                name: 'Copies',
                local: 'mongodb-0055.html',
                children: [
                  {
                    id: 1129,
                    parentId: 1092,
                    name: 'Viewing MongoDB Copy Information',
                    local: 'mongodb-0056.html'
                  },
                  {
                    id: 1130,
                    parentId: 1092,
                    name: 'Managing MongoDB Copies',
                    local: 'mongodb-0057.html'
                  }
                ]
              },
              {
                id: 1093,
                parentId: 888,
                name: 'MongoDB Environment',
                local: 'mongodb-0058.html',
                children: [
                  {
                    id: 1131,
                    parentId: 1093,
                    name: 'Viewing the MongoDB Environment',
                    local: 'mongodb-0059.html'
                  },
                  {
                    id: 1132,
                    parentId: 1093,
                    name: 'Managing MongoDB',
                    local: 'mongodb-0060.html'
                  }
                ]
              }
            ]
          },
          {
            id: 889,
            parentId: 14,
            name: 'Elasticsearch Data Protection',
            local: 'en-us_topic_0000001873759397.html',
            children: [
              {
                id: 1133,
                parentId: 889,
                name: 'Backup',
                local: 'ES_gud_00008.html',
                children: [
                  {
                    id: 1141,
                    parentId: 1133,
                    name: 'Preparing for the Backup',
                    local: 'ES_gud_00011.html'
                  },
                  {
                    id: 1142,
                    parentId: 1133,
                    name: 'Backing Up an Elasticsearch Cluster',
                    local: 'ES_gud_00012.html',
                    children: [
                      {
                        id: 1143,
                        parentId: 1142,
                        name:
                          'Step 1: (Optional) Enabling the Security Encryption Mode',
                        local: 'ES_gud_00013.html'
                      },
                      {
                        id: 1144,
                        parentId: 1142,
                        name:
                          'Step 2: Creating and Configuring a Mount Directory',
                        local: 'ES_gud_00014.html'
                      },
                      {
                        id: 1145,
                        parentId: 1142,
                        name: 'Step 3: Registering an Elasticsearch Cluster',
                        local: 'ES_gud_00015.html'
                      },
                      {
                        id: 1146,
                        parentId: 1142,
                        name: 'Step 4: Creating an Elasticsearch Backup Set',
                        local: 'ES_gud_00016.html'
                      },
                      {
                        id: 1147,
                        parentId: 1142,
                        name: 'Step 5: Creating a Rate Limiting Policy',
                        local: 'ES_gud_00017.html'
                      },
                      {
                        id: 1148,
                        parentId: 1142,
                        name:
                          'Step 6: (Optional) Enabling Backup Link Encryption',
                        local: 'ES_gud_00018.html'
                      },
                      {
                        id: 1149,
                        parentId: 1142,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'ES_gud_00019.html'
                      },
                      {
                        id: 1150,
                        parentId: 1142,
                        name: 'Step 8: Performing Backup',
                        local: 'ES_gud_00020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1134,
                parentId: 889,
                name: 'Replication',
                local: 'ES_gud_00023.html',
                children: [
                  {
                    id: 1151,
                    parentId: 1134,
                    name: 'Replicating an Elasticsearch Copy',
                    local: 'ES_gud_00026.html',
                    children: [
                      {
                        id: 1152,
                        parentId: 1151,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'ES_gud_00028.html'
                      },
                      {
                        id: 1153,
                        parentId: 1151,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'ES_gud_00029.html'
                      },
                      {
                        id: 1154,
                        parentId: 1151,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'ES_gud_00030.html'
                      },
                      {
                        id: 1155,
                        parentId: 1151,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'ES_gud_00029_1a.html'
                      },
                      {
                        id: 1156,
                        parentId: 1151,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'ES_gud_00032.html'
                      },
                      {
                        id: 1157,
                        parentId: 1151,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'ES_gud_00033.html'
                      },
                      {
                        id: 1158,
                        parentId: 1151,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002046700657.html'
                      },
                      {
                        id: 1159,
                        parentId: 1151,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'ES_gud_00034.html'
                      },
                      {
                        id: 1160,
                        parentId: 1151,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'ES_gud_00035.html'
                      },
                      {
                        id: 1161,
                        parentId: 1151,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'ES_gud_00036.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1135,
                parentId: 889,
                name: 'Archiving',
                local: 'ES_gud_00037.html',
                children: [
                  {
                    id: 1162,
                    parentId: 1135,
                    name: 'Archiving Elasticsearch Backup Copies',
                    local: 'ES_gud_00040.html',
                    children: [
                      {
                        id: 1164,
                        parentId: 1162,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'ES_gud_00041.html',
                        children: [
                          {
                            id: 1166,
                            parentId: 1164,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'ES_gud_00042.html'
                          },
                          {
                            id: 1167,
                            parentId: 1164,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'ES_gud_00043.html'
                          }
                        ]
                      },
                      {
                        id: 1165,
                        parentId: 1162,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'ES_gud_00044.html'
                      }
                    ]
                  },
                  {
                    id: 1163,
                    parentId: 1135,
                    name: 'Archiving Elasticsearch Replication Copies',
                    local: 'ES_gud_00045.html',
                    children: [
                      {
                        id: 1168,
                        parentId: 1163,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'ES_gud_00046.html'
                      },
                      {
                        id: 1169,
                        parentId: 1163,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'ES_gud_00047.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1136,
                parentId: 889,
                name: 'Restoration',
                local: 'ES_gud_00048.html',
                children: [
                  {
                    id: 1170,
                    parentId: 1136,
                    name: 'Restoring an Elasticsearch Backup Set',
                    local: 'ES_gud_00051.html'
                  },
                  {
                    id: 1171,
                    parentId: 1136,
                    name:
                      'Restoring One or Multiple Indexes in an Elasticsearch Backup Set',
                    local: 'ES_gud_00052.html'
                  }
                ]
              },
              {
                id: 1137,
                parentId: 889,
                name: 'Global Search',
                local: 'en-us_topic_0000002014774340.html',
                children: [
                  {
                    id: 1172,
                    parentId: 1137,
                    name: 'Global Search for Resources',
                    local: 'oracle_gud_000045.html'
                  },
                  {
                    id: 1173,
                    parentId: 1137,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002014774344.html'
                  }
                ]
              },
              {
                id: 1138,
                parentId: 889,
                name: 'SLA',
                local: 'ES_gud_00057.html',
                children: [
                  {
                    id: 1174,
                    parentId: 1138,
                    name: 'Viewing SLA Information',
                    local: 'ES_gud_00059.html'
                  },
                  {
                    id: 1175,
                    parentId: 1138,
                    name: 'Managing SLAs',
                    local: 'ES_gud_00060.html'
                  }
                ]
              },
              {
                id: 1139,
                parentId: 889,
                name: 'Copies',
                local: 'ES_gud_00061.html',
                children: [
                  {
                    id: 1176,
                    parentId: 1139,
                    name: 'Viewing Elasticsearch Copy Information',
                    local: 'ES_gud_00062.html'
                  },
                  {
                    id: 1177,
                    parentId: 1139,
                    name: 'Managing Elasticsearch Copies',
                    local: 'ES_gud_00063.html'
                  }
                ]
              },
              {
                id: 1140,
                parentId: 889,
                name: 'Elasticsearch Cluster Environment',
                local: 'ES_gud_00063_a1.html',
                children: [
                  {
                    id: 1178,
                    parentId: 1140,
                    name: 'Querying Elasticsearch Information',
                    local: 'ES_gud_00065.html'
                  },
                  {
                    id: 1179,
                    parentId: 1140,
                    name: 'Managing Elasticsearch Clusters',
                    local: 'ES_gud_00066.html'
                  },
                  {
                    id: 1180,
                    parentId: 1140,
                    name: 'Managing Backup Sets',
                    local: 'ES_gud_00067_a1.html'
                  }
                ]
              }
            ]
          },
          {
            id: 890,
            parentId: 14,
            name: 'HDFS Data Protection',
            local: 'product_documentation_000031.html',
            children: [
              {
                id: 1181,
                parentId: 890,
                name: 'Backup',
                local: 'hdfs_00008.html',
                children: [
                  {
                    id: 1189,
                    parentId: 1181,
                    name: 'Preparing for the Backup',
                    local: 'hdfs_00011.html'
                  },
                  {
                    id: 1190,
                    parentId: 1181,
                    name: 'Backing Up HDFS Filesets',
                    local: 'hdfs_00012.html',
                    children: [
                      {
                        id: 1191,
                        parentId: 1190,
                        name:
                          'Step 1: Enabling the Snapshot Function for the HDFS Directory',
                        local: 'hdfs_00013.html'
                      },
                      {
                        id: 1192,
                        parentId: 1190,
                        name: 'Step 2: Checking the HDFS ACL Switch Status',
                        local: 'hdfs_00014.html'
                      },
                      {
                        id: 1193,
                        parentId: 1190,
                        name: 'Step 3: Registering an HDFS Cluster',
                        local: 'hdfs_00015.html'
                      },
                      {
                        id: 1194,
                        parentId: 1190,
                        name: 'Step 4: Creating an HDFS Fileset',
                        local: 'hdfs_00016.html'
                      },
                      {
                        id: 1195,
                        parentId: 1190,
                        name: 'Step 5: Creating a Rate Limiting Policy',
                        local: 'hdfs_00017.html'
                      },
                      {
                        id: 1196,
                        parentId: 1190,
                        name:
                          'Step 6: (Optional) Enabling Backup Link Encryption',
                        local: 'hdfs_00018.html'
                      },
                      {
                        id: 1197,
                        parentId: 1190,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'hdfs_00019.html'
                      },
                      {
                        id: 1198,
                        parentId: 1190,
                        name: 'Step 8: Performing Backup',
                        local: 'hdfs_00020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1182,
                parentId: 890,
                name: 'Replication',
                local: 'hdfs_00023.html',
                children: [
                  {
                    id: 1199,
                    parentId: 1182,
                    name: 'Replicating HDFS Copies',
                    local: 'hdfs_00026.html',
                    children: [
                      {
                        id: 1200,
                        parentId: 1199,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'hdfs_00028.html'
                      },
                      {
                        id: 1201,
                        parentId: 1199,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'hdfs_00029.html'
                      },
                      {
                        id: 1202,
                        parentId: 1199,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'hdfs_00030.html'
                      },
                      {
                        id: 1203,
                        parentId: 1199,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'hdfs_00031.html'
                      },
                      {
                        id: 1204,
                        parentId: 1199,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'hdfs_00032.html'
                      },
                      {
                        id: 1205,
                        parentId: 1199,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'hdfs_00033.html'
                      },
                      {
                        id: 1206,
                        parentId: 1199,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002046697729.html'
                      },
                      {
                        id: 1207,
                        parentId: 1199,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'hdfs_00034.html'
                      },
                      {
                        id: 1208,
                        parentId: 1199,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'hdfs_00035.html'
                      },
                      {
                        id: 1209,
                        parentId: 1199,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'hdfs_00036.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1183,
                parentId: 890,
                name: 'Archiving',
                local: 'hdfs_00037.html',
                children: [
                  {
                    id: 1210,
                    parentId: 1183,
                    name: 'Archiving HDFS Backup Copies',
                    local: 'hdfs_00040.html',
                    children: [
                      {
                        id: 1212,
                        parentId: 1210,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'hdfs_00041.html',
                        children: [
                          {
                            id: 1214,
                            parentId: 1212,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'hdfs_00042.html'
                          },
                          {
                            id: 1215,
                            parentId: 1212,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'hdfs_00043.html'
                          }
                        ]
                      },
                      {
                        id: 1213,
                        parentId: 1210,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'hdfs_00044.html'
                      }
                    ]
                  },
                  {
                    id: 1211,
                    parentId: 1183,
                    name: 'Archiving HDFS Replication Copies',
                    local: 'hdfs_00045.html',
                    children: [
                      {
                        id: 1216,
                        parentId: 1211,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'hdfs_00046.html'
                      },
                      {
                        id: 1217,
                        parentId: 1211,
                        name:
                          'Step 2: Creating Periodic Archiving for Replication Copies',
                        local: 'hdfs_00047.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1184,
                parentId: 890,
                name: 'Restoration',
                local: 'hdfs_00047_1a.html',
                children: [
                  {
                    id: 1218,
                    parentId: 1184,
                    name: 'Restoring HDFS Filesets',
                    local: 'hdfs_00050.html'
                  },
                  {
                    id: 1219,
                    parentId: 1184,
                    name: 'Restoring One or Multiple Files in an HDFS Fileset',
                    local: 'hdfs_00051.html'
                  }
                ]
              },
              {
                id: 1185,
                parentId: 890,
                name: 'Global Search',
                local: 'hdfs_00052.html',
                children: [
                  {
                    id: 1220,
                    parentId: 1185,
                    name: 'Global Search for Copies',
                    local: 'hdfs_00053.html'
                  },
                  {
                    id: 1221,
                    parentId: 1185,
                    name: 'Global Search for Resources',
                    local: 'hdfs_00054.html'
                  },
                  {
                    id: 1222,
                    parentId: 1185,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002686646.html'
                  }
                ]
              },
              {
                id: 1186,
                parentId: 890,
                name: 'SLA',
                local: 'hdfs_00057.html',
                children: [
                  {
                    id: 1223,
                    parentId: 1186,
                    name: 'Viewing SLA Information',
                    local: 'hdfs_00059.html'
                  },
                  {
                    id: 1224,
                    parentId: 1186,
                    name: 'Managing SLAs',
                    local: 'hdfs_00060.html'
                  }
                ]
              },
              {
                id: 1187,
                parentId: 890,
                name: 'Copies',
                local: 'hdfs_00060_1a.html',
                children: [
                  {
                    id: 1225,
                    parentId: 1187,
                    name: 'Viewing HDFS Copy Information',
                    local: 'hdfs_00062.html'
                  },
                  {
                    id: 1226,
                    parentId: 1187,
                    name: 'Managing HDFS Copies',
                    local: 'hdfs_00063.html'
                  }
                ]
              },
              {
                id: 1188,
                parentId: 890,
                name: 'HDFS Cluster Environment',
                local: 'hdfs_00064.html',
                children: [
                  {
                    id: 1227,
                    parentId: 1188,
                    name: 'Querying HDFS Information',
                    local: 'hdfs_00065.html'
                  },
                  {
                    id: 1228,
                    parentId: 1188,
                    name: 'Managing HDFS Clusters',
                    local: 'hdfs_00066.html'
                  },
                  {
                    id: 1229,
                    parentId: 1188,
                    name: 'Managing HDFS Filesets',
                    local: 'hdfs_00067.html'
                  }
                ]
              }
            ]
          },
          {
            id: 891,
            parentId: 14,
            name: 'Redis Data Protection',
            local: 'en-us_topic_0000001873759393.html',
            children: [
              {
                id: 1230,
                parentId: 891,
                name: 'Backup',
                local: 'redis-00003.html',
                children: [
                  {
                    id: 1239,
                    parentId: 1230,
                    name: 'Preparing for the Backup',
                    local: 'redis-00006.html'
                  },
                  {
                    id: 1240,
                    parentId: 1230,
                    name: 'Backing Up a Redis Cluster',
                    local: 'redis-00007.html',
                    children: [
                      {
                        id: 1241,
                        parentId: 1240,
                        name: 'Step 1: Registering a Redis Cluster',
                        local: 'redis-00008.html'
                      },
                      {
                        id: 1242,
                        parentId: 1240,
                        name: 'Step 2 Creating a Rate Limiting Policy',
                        local: 'redis-00009.html'
                      },
                      {
                        id: 1243,
                        parentId: 1240,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'redis-00010.html'
                      },
                      {
                        id: 1244,
                        parentId: 1240,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'redis-00011.html'
                      },
                      {
                        id: 1245,
                        parentId: 1240,
                        name: 'Step 5: Performing Backup',
                        local: 'redis-00012.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1231,
                parentId: 891,
                name: 'Replication',
                local: 'oracle_gud_000035_5.html',
                children: [
                  {
                    id: 1246,
                    parentId: 1231,
                    name: 'Replicating Redis Copies',
                    local: 'redis-00018.html',
                    children: [
                      {
                        id: 1247,
                        parentId: 1246,
                        name: 'Planning the Replication Network',
                        local: 'redis-00019.html'
                      },
                      {
                        id: 1248,
                        parentId: 1246,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'fc_gud_0026_4.html'
                      },
                      {
                        id: 1249,
                        parentId: 1246,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_7.html'
                      },
                      {
                        id: 1250,
                        parentId: 1246,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'redis-00021.html'
                      },
                      {
                        id: 1251,
                        parentId: 1246,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'redis-00022.html'
                      },
                      {
                        id: 1252,
                        parentId: 1246,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'redis-00023.html'
                      },
                      {
                        id: 1253,
                        parentId: 1246,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'redis-00024.html'
                      },
                      {
                        id: 1254,
                        parentId: 1246,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002046614601.html'
                      },
                      {
                        id: 1255,
                        parentId: 1246,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'redis-00025.html'
                      },
                      {
                        id: 1256,
                        parentId: 1246,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'redis-00026.html'
                      },
                      {
                        id: 1257,
                        parentId: 1246,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'redis-00027.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1232,
                parentId: 891,
                name: 'Archiving',
                local: 'redis-00028.html',
                children: [
                  {
                    id: 1258,
                    parentId: 1232,
                    name: 'Archiving Redis Backup Copies',
                    local: 'redis-00031.html',
                    children: [
                      {
                        id: 1260,
                        parentId: 1258,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'redis-00032.html',
                        children: [
                          {
                            id: 1262,
                            parentId: 1260,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'redis-00033.html'
                          },
                          {
                            id: 1263,
                            parentId: 1260,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'redis-00034.html'
                          }
                        ]
                      },
                      {
                        id: 1261,
                        parentId: 1258,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'redis-00035.html'
                      }
                    ]
                  },
                  {
                    id: 1259,
                    parentId: 1232,
                    name: 'Archiving Redis Replication Copies',
                    local: 'redis-00036.html',
                    children: [
                      {
                        id: 1264,
                        parentId: 1259,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'redis-00037.html'
                      },
                      {
                        id: 1265,
                        parentId: 1259,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'redis-00038.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1233,
                parentId: 891,
                name: 'Restoration',
                local: 'redis-00039.html',
                children: [
                  {
                    id: 1266,
                    parentId: 1233,
                    name: 'Restoring a Redis Cluster',
                    local: 'redis-00042.html'
                  }
                ]
              },
              {
                id: 1234,
                parentId: 891,
                name: 'Global Search',
                local: 'en-us_topic_0000002038763381.html',
                children: [
                  {
                    id: 1267,
                    parentId: 1234,
                    name: 'Global Search for Resources',
                    local: 'redis-00043.html'
                  },
                  {
                    id: 1268,
                    parentId: 1234,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002843406.html'
                  }
                ]
              },
              {
                id: 1235,
                parentId: 891,
                name:
                  'Data Deduplication and Compression (Applicable to OceanProtect X Series Backup Appliances and OceanProtect E1000)',
                local: 'redis-00044.html',
                children: [
                  {
                    id: 1269,
                    parentId: 1235,
                    name: 'About Data Deduplication and Compression',
                    local: 'redis-00045.html'
                  }
                ]
              },
              {
                id: 1236,
                parentId: 891,
                name: 'SLA',
                local: 'redis-00046.html',
                children: [
                  {
                    id: 1270,
                    parentId: 1236,
                    name: 'About SLA',
                    local: 'redis-00047.html'
                  },
                  {
                    id: 1271,
                    parentId: 1236,
                    name: 'Viewing SLA Information',
                    local: 'redis-00048.html'
                  },
                  {
                    id: 1272,
                    parentId: 1236,
                    name: 'Managing SLAs',
                    local: 'redis-00049.html'
                  }
                ]
              },
              {
                id: 1237,
                parentId: 891,
                name: 'Copies',
                local: 'redis-00050.html',
                children: [
                  {
                    id: 1273,
                    parentId: 1237,
                    name: 'Viewing Redis Copy Information',
                    local: 'redis-00051.html'
                  },
                  {
                    id: 1274,
                    parentId: 1237,
                    name: 'Managing Redis Copies',
                    local: 'redis-00052.html'
                  }
                ]
              },
              {
                id: 1238,
                parentId: 891,
                name: 'Redis Cluster Environments',
                local: 'redis-00053.html',
                children: [
                  {
                    id: 1275,
                    parentId: 1238,
                    name: 'Querying Redis Information',
                    local: 'redis-00054.html'
                  },
                  {
                    id: 1276,
                    parentId: 1238,
                    name: 'Managing Redis Clusters',
                    local: 'redis-00055.html'
                  },
                  {
                    id: 1277,
                    parentId: 1238,
                    name: 'Managing Redis Cluster Protection',
                    local: 'redis-00056.html'
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
        name: 'Virtualization',
        local: 'en-us_topic_0000001918470736.html',
        children: [
          {
            id: 1278,
            parentId: 15,
            name: 'VMware Data Protection',
            local: 'product_documentation_000027.html',
            children: [
              {
                id: 1283,
                parentId: 1278,
                name: 'Backup',
                local: 'vmware_gud_0007_0.html',
                children: [
                  {
                    id: 1292,
                    parentId: 1283,
                    name: 'Preparing for the Backup',
                    local: 'vmware_gud_0014_0.html'
                  },
                  {
                    id: 1293,
                    parentId: 1283,
                    name: 'Backing Up a VMware VM',
                    local: 'vmware_gud_0015_0.html',
                    children: [
                      {
                        id: 1294,
                        parentId: 1293,
                        name: 'Step 1: Checking and Installing VMware Tools',
                        local: 'vmware_gud_0016.html'
                      },
                      {
                        id: 1295,
                        parentId: 1293,
                        name:
                          'Step 2: Checking and Enabling the vmware-vapi-endpoint Service',
                        local: 'vmware_gud_0017.html'
                      },
                      {
                        id: 1296,
                        parentId: 1293,
                        name:
                          'Step 3: Configuring the Scripts for Application-Consistent Backup',
                        local: 'vmware_gud_0018.html',
                        children: [
                          {
                            id: 1305,
                            parentId: 1296,
                            name: 'DB2 Database',
                            local: 'vmware_gud_0019.html'
                          },
                          {
                            id: 1306,
                            parentId: 1296,
                            name: 'Oracle Database',
                            local: 'vmware_gud_0020.html'
                          },
                          {
                            id: 1307,
                            parentId: 1296,
                            name: 'Sybase Database',
                            local: 'vmware_gud_0021.html'
                          },
                          {
                            id: 1308,
                            parentId: 1296,
                            name: 'MySQL Database',
                            local: 'vmware_gud_0022.html'
                          }
                        ]
                      },
                      {
                        id: 1297,
                        parentId: 1293,
                        name: 'Step 4: Obtaining the VMware Certificate',
                        local: 'vmware_gud_0023.html'
                      },
                      {
                        id: 1298,
                        parentId: 1293,
                        name:
                          'Step 5: Registering the VMware Virtualization Environment',
                        local: 'vmware_gud_0024_0.html'
                      },
                      {
                        id: 1299,
                        parentId: 1293,
                        name:
                          'Step 6: (Optional) Creating a VMware VM Group (Applicable to 1.6.0 and Later Versions)',
                        local: 'vmware_gud_0024_1.html'
                      },
                      {
                        id: 1300,
                        parentId: 1293,
                        name:
                          'Step 7: (Optional) Creating a Rate Limiting Policy',
                        local: 'vmware_gud_0025_0.html'
                      },
                      {
                        id: 1301,
                        parentId: 1293,
                        name:
                          'Step 8: (Optional) Enabling Backup Link Encryption',
                        local: 'vmware_gud_0026.html'
                      },
                      {
                        id: 1302,
                        parentId: 1293,
                        name: 'Step 9: Logging In to the iSCSI Initiator',
                        local: 'vmware_gud_iscsi.html'
                      },
                      {
                        id: 1303,
                        parentId: 1293,
                        name: 'Step 10: Creating a Backup SLA',
                        local: 'vmware_gud_0027_0.html'
                      },
                      {
                        id: 1304,
                        parentId: 1293,
                        name: 'Step 11: Performing Backup',
                        local: 'vmware_gud_0028_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1284,
                parentId: 1278,
                name: 'Replication',
                local: 'vmware_gud_0031_0.html',
                children: [
                  {
                    id: 1309,
                    parentId: 1284,
                    name: 'Replicating a VMware VM Copy',
                    local: 'vmware_gud_0034_0.html',
                    children: [
                      {
                        id: 1310,
                        parentId: 1309,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'vmware_gud_0036.html'
                      },
                      {
                        id: 1311,
                        parentId: 1309,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'vmware_gud_0036_1.html'
                      },
                      {
                        id: 1312,
                        parentId: 1309,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'vmware_gud_0037_0.html'
                      },
                      {
                        id: 1313,
                        parentId: 1309,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption (Applicable to OceanProtect X Series Backup Appliances and OceanProtect E1000)',
                        local: 'vmware_gud_0038_0.html'
                      },
                      {
                        id: 1314,
                        parentId: 1309,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'vmware_gud_0039_0.html'
                      },
                      {
                        id: 1315,
                        parentId: 1309,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'vmware_gud_0040_1.html'
                      },
                      {
                        id: 1316,
                        parentId: 1309,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'vmware_gud_0040_1_0.html'
                      },
                      {
                        id: 1317,
                        parentId: 1309,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'vmware_gud_0041_0.html'
                      },
                      {
                        id: 1318,
                        parentId: 1309,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'vmware_gud_0042_0.html'
                      },
                      {
                        id: 1319,
                        parentId: 1309,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'vmware_gud_0043_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1285,
                parentId: 1278,
                name: 'Archiving',
                local: 'vmware_gud_0044_0.html',
                children: [
                  {
                    id: 1320,
                    parentId: 1285,
                    name: 'Archiving VMware Backup Copies',
                    local: 'vmware_gud_0047_0.html',
                    children: [
                      {
                        id: 1322,
                        parentId: 1320,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'vmware_gud_0048_0.html',
                        children: [
                          {
                            id: 1324,
                            parentId: 1322,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'vmware_gud_0049_0.html'
                          },
                          {
                            id: 1325,
                            parentId: 1322,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'vmware_gud_0050_0.html'
                          }
                        ]
                      },
                      {
                        id: 1323,
                        parentId: 1320,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'vmware_gud_0051_0.html'
                      }
                    ]
                  },
                  {
                    id: 1321,
                    parentId: 1285,
                    name: 'Archiving VMware Replication Copies',
                    local: 'vmware_gud_0052_0.html',
                    children: [
                      {
                        id: 1326,
                        parentId: 1321,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'vmware_gud_0053_0.html'
                      },
                      {
                        id: 1327,
                        parentId: 1321,
                        name:
                          'Step 2: Creating Periodic Archives of Replication Copies',
                        local: 'vmware_gud_0054_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1286,
                parentId: 1278,
                name: 'Restoration',
                local: 'vmware_gud_0055_0.html',
                children: [
                  {
                    id: 1328,
                    parentId: 1286,
                    name: 'Restoring VMware VMs',
                    local: 'vmware_gud_0058_0.html'
                  },
                  {
                    id: 1329,
                    parentId: 1286,
                    name: 'Restoring VMware VM Disks',
                    local: 'vmware_gud_0059_0.html'
                  },
                  {
                    id: 1330,
                    parentId: 1286,
                    name: 'Restoring Files in a VMware VM',
                    local: 'vmware_gud_0060_0.html'
                  }
                ]
              },
              {
                id: 1287,
                parentId: 1278,
                name:
                  'Instant Recovery (Applicable Only to OceanProtect X Series Backup Appliances)',
                local: 'vmware_gud_0061.html',
                children: [
                  {
                    id: 1331,
                    parentId: 1287,
                    name: 'Instant Recovery of VMware VMs',
                    local: 'vmware_gud_0064.html'
                  }
                ]
              },
              {
                id: 1288,
                parentId: 1278,
                name: 'Global Search',
                local: 'vmware_gud_0074_0.html',
                children: [
                  {
                    id: 1332,
                    parentId: 1288,
                    name: 'About Global Search',
                    local: 'vmware_gud_0075.html'
                  },
                  {
                    id: 1333,
                    parentId: 1288,
                    name: 'Global Search for Copies',
                    local: 'vmware_gud_0076_5.html'
                  },
                  {
                    id: 1334,
                    parentId: 1288,
                    name: 'Global Search for Resources',
                    local: 'vmware_gud_0077_0.html'
                  },
                  {
                    id: 1335,
                    parentId: 1288,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'vmware_gud_0077_1.html'
                  }
                ]
              },
              {
                id: 1289,
                parentId: 1278,
                name: 'SLA ',
                local: 'vmware_gud_0080_0.html',
                children: [
                  {
                    id: 1336,
                    parentId: 1289,
                    name: 'Viewing SLA Information',
                    local: 'vmware_gud_0082_0.html'
                  },
                  {
                    id: 1337,
                    parentId: 1289,
                    name: 'Managing SLAs',
                    local: 'vmware_gud_0083_0.html'
                  }
                ]
              },
              {
                id: 1290,
                parentId: 1278,
                name: 'Copies',
                local: 'vmware_gud_0084_0.html',
                children: [
                  {
                    id: 1338,
                    parentId: 1290,
                    name: 'Viewing VMware Copy Data',
                    local: 'vmware_gud_0085_0.html'
                  },
                  {
                    id: 1339,
                    parentId: 1290,
                    name: 'Managing VMware Copies',
                    local: 'vmware_gud_0086_0.html'
                  }
                ]
              },
              {
                id: 1291,
                parentId: 1278,
                name: 'VMware Virtualization Environment',
                local: 'vmware_gud_0087_0.html',
                children: [
                  {
                    id: 1340,
                    parentId: 1291,
                    name:
                      'Viewing Information About the VMware Virtualization Environment',
                    local: 'vmware_gud_0088_0.html'
                  },
                  {
                    id: 1341,
                    parentId: 1291,
                    name: 'Managing VMware Registration Information',
                    local: 'vmware_gud_0089_0.html'
                  },
                  {
                    id: 1342,
                    parentId: 1291,
                    name: 'Managing Clusters/Hosts/VMs/VM Groups',
                    local: 'vmware_gud_0090_0.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1279,
            parentId: 15,
            name: 'FusionCompute Data Protection',
            local: 'en-us_topic_0000001873679177.html',
            children: [
              {
                id: 1343,
                parentId: 1279,
                name: 'Backup',
                local: 'fc_gud_0009_0.html',
                children: [
                  {
                    id: 1351,
                    parentId: 1343,
                    name: 'Preparations for Backup',
                    local: 'fc_gud_0012_0.html'
                  },
                  {
                    id: 1352,
                    parentId: 1343,
                    name: 'Backing Up a FusionCompute VM',
                    local: 'fc_gud_0013_0.html',
                    children: [
                      {
                        id: 1353,
                        parentId: 1352,
                        name:
                          'Step 1: Create a FusionCompute Interconnection User',
                        local: 'fc_gud_0014_0.html'
                      },
                      {
                        id: 1354,
                        parentId: 1352,
                        name:
                          'Step 2: Registering the FusionCompute Virtualization Environment',
                        local: 'fc_gud_0015_0.html'
                      },
                      {
                        id: 1355,
                        parentId: 1352,
                        name:
                          'Step 3: (Optional) Creating a FusionCompute VM Group (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0024_1_0.html'
                      },
                      {
                        id: 1356,
                        parentId: 1352,
                        name:
                          'Step 4: (Optional) Creating a Rate Limiting Policy',
                        local: 'fc_gud_0016_0.html'
                      },
                      {
                        id: 1357,
                        parentId: 1352,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'fc_gud_0017_0.html'
                      },
                      {
                        id: 1358,
                        parentId: 1352,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'fc_gud_0018_0.html'
                      },
                      {
                        id: 1359,
                        parentId: 1352,
                        name: 'Step 7: Performing Backup',
                        local: 'fc_gud_0019_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1344,
                parentId: 1279,
                name: 'Replication',
                local: 'fc_gud_0022_0.html',
                children: [
                  {
                    id: 1360,
                    parentId: 1344,
                    name: 'Replicating a FusionCompute VM Copy',
                    local: 'fc_gud_0024_0.html',
                    children: [
                      {
                        id: 1361,
                        parentId: 1360,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'fc_gud_0026_9.html'
                      },
                      {
                        id: 1362,
                        parentId: 1360,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_13.html'
                      },
                      {
                        id: 1363,
                        parentId: 1360,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'fc_gud_0027_0.html'
                      },
                      {
                        id: 1364,
                        parentId: 1360,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'fc_gud_0028_0.html'
                      },
                      {
                        id: 1365,
                        parentId: 1360,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'fc_gud_0029_0.html'
                      },
                      {
                        id: 1366,
                        parentId: 1360,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'fc_gud_0030.html'
                      },
                      {
                        id: 1367,
                        parentId: 1360,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002008271350.html'
                      },
                      {
                        id: 1368,
                        parentId: 1360,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'fc_gud_0031_1.html'
                      },
                      {
                        id: 1369,
                        parentId: 1360,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'fc_gud_0032_0.html'
                      },
                      {
                        id: 1370,
                        parentId: 1360,
                        name:
                          '(Optional) Step 8: Creating a Bidirectional Replication or Cascading Replication SLA',
                        local: 'fc_gud_0033_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1345,
                parentId: 1279,
                name: 'Archiving',
                local: 'fc_gud_0034_0.html',
                children: [
                  {
                    id: 1371,
                    parentId: 1345,
                    name: 'Archiving FusionCompute Backup Copies',
                    local: 'fc_gud_0037_0.html',
                    children: [
                      {
                        id: 1373,
                        parentId: 1371,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'fc_gud_0038_0.html',
                        children: [
                          {
                            id: 1375,
                            parentId: 1373,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'fc_gud_0039_0.html'
                          },
                          {
                            id: 1376,
                            parentId: 1373,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'fc_gud_0040_0.html'
                          }
                        ]
                      },
                      {
                        id: 1374,
                        parentId: 1371,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'fc_gud_0041_0.html'
                      }
                    ]
                  },
                  {
                    id: 1372,
                    parentId: 1345,
                    name: 'Archiving FusionCompute Replication Copies',
                    local: 'fc_gud_0042_0.html',
                    children: [
                      {
                        id: 1377,
                        parentId: 1372,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'fc_gud_0043_0.html'
                      },
                      {
                        id: 1378,
                        parentId: 1372,
                        name:
                          'Step 2: Creating Periodic Archives of Replication Copies',
                        local: 'fc_gud_0044_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1346,
                parentId: 1279,
                name: 'Restoration',
                local: 'fc_gud_0045_0.html',
                children: [
                  {
                    id: 1379,
                    parentId: 1346,
                    name: 'Restoring FusionCompute VMs',
                    local: 'fc_gud_0048_0.html'
                  },
                  {
                    id: 1380,
                    parentId: 1346,
                    name: 'Restoring FusionCompute VM Disks',
                    local: 'fc_gud_0049_0.html'
                  },
                  {
                    id: 1381,
                    parentId: 1346,
                    name: 'Restoring Files on a FusionCompute VM',
                    local: 'fc_gud_re1_0.html'
                  }
                ]
              },
              {
                id: 1347,
                parentId: 1279,
                name: 'Global Search',
                local: 'fc_gud_gs1_0.html',
                children: [
                  {
                    id: 1382,
                    parentId: 1347,
                    name: 'About Global Search',
                    local: 'en-us_topic_0000002038884249.html'
                  },
                  {
                    id: 1383,
                    parentId: 1347,
                    name: 'Global Search for Copies',
                    local: 'fc_gud_gs3_0.html'
                  },
                  {
                    id: 1384,
                    parentId: 1347,
                    name: 'Global Search for Resources',
                    local: 'fc_gud_0050_0.html'
                  },
                  {
                    id: 1385,
                    parentId: 1347,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002642984.html'
                  }
                ]
              },
              {
                id: 1348,
                parentId: 1279,
                name: 'SLA ',
                local: 'fc_gud_0053_0.html',
                children: [
                  {
                    id: 1386,
                    parentId: 1348,
                    name: 'Viewing SLA Information',
                    local: 'fc_gud_0055_0.html'
                  },
                  {
                    id: 1387,
                    parentId: 1348,
                    name: 'Managing SLAs',
                    local: 'fc_gud_0056_0.html'
                  }
                ]
              },
              {
                id: 1349,
                parentId: 1279,
                name: 'Copies',
                local: 'fc_gud_0057_0.html',
                children: [
                  {
                    id: 1388,
                    parentId: 1349,
                    name: 'Viewing FusionCompute Copy Data',
                    local: 'fc_gud_0058_0.html'
                  },
                  {
                    id: 1389,
                    parentId: 1349,
                    name: 'Managing FusionCompute Copies',
                    local: 'fc_gud_0059_0.html'
                  }
                ]
              },
              {
                id: 1350,
                parentId: 1279,
                name: 'FusionCompute Virtualization Environment',
                local: 'fc_gud_0060_0.html',
                children: [
                  {
                    id: 1390,
                    parentId: 1350,
                    name:
                      'Querying Information About the FusionCompute Virtualization Environment',
                    local: 'fc_gud_0061_0.html'
                  },
                  {
                    id: 1391,
                    parentId: 1350,
                    name: 'Managing FusionCompute Registration Information',
                    local: 'fc_gud_0062_0.html'
                  },
                  {
                    id: 1392,
                    parentId: 1350,
                    name: 'Managing Clusters/Hosts/VMs/VM Groups',
                    local: 'fc_gud_0063_0.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1280,
            parentId: 15,
            name:
              'CNware Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001873679209.html',
            children: [
              {
                id: 1393,
                parentId: 1280,
                name: 'Backup',
                local: 'CNware_00007.html',
                children: [
                  {
                    id: 1402,
                    parentId: 1393,
                    name: 'Preparing for the Backup',
                    local: 'CNware_00015.html'
                  },
                  {
                    id: 1403,
                    parentId: 1393,
                    name: 'Backing Up CNware VMs',
                    local: 'CNware_00016.html',
                    children: [
                      {
                        id: 1404,
                        parentId: 1403,
                        name: 'Step 1: Obtaining the CNware Certificate',
                        local: 'CNware_00024.html'
                      },
                      {
                        id: 1405,
                        parentId: 1403,
                        name:
                          'Step 2: Registering the CNware Virtualization Environment',
                        local: 'CNware_00025.html'
                      },
                      {
                        id: 1406,
                        parentId: 1403,
                        name: 'Step 3: (Optional) Creating a CNware VM Group',
                        local: 'CNware_00014.html'
                      },
                      {
                        id: 1407,
                        parentId: 1403,
                        name:
                          'Step 4: (Optional) Creating a Rate Limiting Policy',
                        local: 'CNware_00026.html'
                      },
                      {
                        id: 1408,
                        parentId: 1403,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'CNware_00027.html'
                      },
                      {
                        id: 1409,
                        parentId: 1403,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'CNware_00029.html'
                      },
                      {
                        id: 1410,
                        parentId: 1403,
                        name: 'Step 7: Performing Backup',
                        local: 'CNware_00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1394,
                parentId: 1280,
                name: 'Replication',
                local: 'CNware_00033.html',
                children: [
                  {
                    id: 1411,
                    parentId: 1394,
                    name: 'Replicating a CNware VM Copy',
                    local: 'CNware_00036.html',
                    children: [
                      {
                        id: 1412,
                        parentId: 1411,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'CNware_000251.html'
                      },
                      {
                        id: 1413,
                        parentId: 1411,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances)',
                        local: 'CNware_000261.html'
                      },
                      {
                        id: 1414,
                        parentId: 1411,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'CNware_00039.html'
                      },
                      {
                        id: 1415,
                        parentId: 1411,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'CNware_00040.html'
                      },
                      {
                        id: 1416,
                        parentId: 1411,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'CNware_00041.html'
                      },
                      {
                        id: 1417,
                        parentId: 1411,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'CNware_000300.html'
                      },
                      {
                        id: 1418,
                        parentId: 1411,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'CNware_00043.html'
                      },
                      {
                        id: 1419,
                        parentId: 1411,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'CNware_00044.html'
                      },
                      {
                        id: 1420,
                        parentId: 1411,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'CNware_00045.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1395,
                parentId: 1280,
                name: 'Archiving',
                local: 'CNware_00046.html',
                children: [
                  {
                    id: 1421,
                    parentId: 1395,
                    name: 'Archiving CNware Backup Copies',
                    local: 'CNware_00049.html',
                    children: [
                      {
                        id: 1423,
                        parentId: 1421,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'CNware_00050.html',
                        children: [
                          {
                            id: 1425,
                            parentId: 1423,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'CNware_00051.html'
                          },
                          {
                            id: 1426,
                            parentId: 1423,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'CNware_00052.html'
                          }
                        ]
                      },
                      {
                        id: 1424,
                        parentId: 1421,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'CNware_00053.html'
                      }
                    ]
                  },
                  {
                    id: 1422,
                    parentId: 1395,
                    name: 'Archiving CNware Replication Copies',
                    local: 'CNware_00054.html',
                    children: [
                      {
                        id: 1427,
                        parentId: 1422,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'CNware_00055.html'
                      },
                      {
                        id: 1428,
                        parentId: 1422,
                        name:
                          'Step 2 Creating a Periodic Archive Policy for Replication Copies',
                        local: 'CNware_00056.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1396,
                parentId: 1280,
                name: 'Restoration',
                local: 'CNware_00057.html',
                children: [
                  {
                    id: 1429,
                    parentId: 1396,
                    name: 'Restoring CNware VMs',
                    local: 'CNware_00060.html'
                  },
                  {
                    id: 1430,
                    parentId: 1396,
                    name: 'Restoring CNware VM Disks',
                    local: 'CNware_00061.html'
                  },
                  {
                    id: 1431,
                    parentId: 1396,
                    name: 'Restoring Files on a CNware VM',
                    local: 'CNware_00062.html'
                  }
                ]
              },
              {
                id: 1397,
                parentId: 1280,
                name: 'Instant Recovery',
                local: 'CNware_00063.html',
                children: [
                  {
                    id: 1432,
                    parentId: 1397,
                    name: 'Instant Recovery of CNware VMs',
                    local: 'CNware_00066.html'
                  }
                ]
              },
              {
                id: 1398,
                parentId: 1280,
                name: 'Global Search',
                local: 'CNware_00076.html',
                children: [
                  {
                    id: 1433,
                    parentId: 1398,
                    name: 'About Global Search',
                    local: 'CNware_000641.html'
                  },
                  {
                    id: 1434,
                    parentId: 1398,
                    name: 'Global Search for Copies',
                    local: 'CNware_000642.html'
                  },
                  {
                    id: 1435,
                    parentId: 1398,
                    name: 'Global Search for Resources',
                    local: 'CNware_00079.html'
                  },
                  {
                    id: 1436,
                    parentId: 1398,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'CNware_000671.html'
                  }
                ]
              },
              {
                id: 1399,
                parentId: 1280,
                name: 'SLA ',
                local: 'CNware_00082.html',
                children: [
                  {
                    id: 1437,
                    parentId: 1399,
                    name: 'Viewing SLA Information',
                    local: 'CNware_00084.html'
                  },
                  {
                    id: 1438,
                    parentId: 1399,
                    name: 'Managing SLAs',
                    local: 'CNware_00085.html'
                  }
                ]
              },
              {
                id: 1400,
                parentId: 1280,
                name: 'Copies',
                local: 'CNware_00086.html',
                children: [
                  {
                    id: 1439,
                    parentId: 1400,
                    name: 'Viewing CNware Copy Data',
                    local: 'CNware_00087.html'
                  },
                  {
                    id: 1440,
                    parentId: 1400,
                    name: 'Managing CNware Copies',
                    local: 'CNware_00088.html'
                  }
                ]
              },
              {
                id: 1401,
                parentId: 1280,
                name: 'CNware Virtualization Environment',
                local: 'CNware_00089.html',
                children: [
                  {
                    id: 1441,
                    parentId: 1401,
                    name:
                      'Querying Information About the CNware Virtualization Environment',
                    local: 'CNware_00090.html'
                  },
                  {
                    id: 1442,
                    parentId: 1401,
                    name: 'Managing CNware Registration Information',
                    local: 'CNware_00091.html'
                  },
                  {
                    id: 1443,
                    parentId: 1401,
                    name: 'Management VMs/Hosts/Clusters',
                    local: 'CNware_00092.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1281,
            parentId: 15,
            name:
              'Hyper-V Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000002037019572.html',
            children: [
              {
                id: 1444,
                parentId: 1281,
                name: 'Backup',
                local: 'vmware_gud_0007.html',
                children: [
                  {
                    id: 1452,
                    parentId: 1444,
                    name: 'Preparing for the Backup',
                    local: 'vmware_gud_0014.html'
                  },
                  {
                    id: 1453,
                    parentId: 1444,
                    name: 'Backing Up Hyper-V VMs',
                    local: 'vmware_gud_0015.html',
                    children: [
                      {
                        id: 1454,
                        parentId: 1453,
                        name:
                          'Step 1: Registering a Hyper-V Virtualization Environment',
                        local: 'vmware_gud_0024.html'
                      },
                      {
                        id: 1455,
                        parentId: 1453,
                        name:
                          'Step 2: (Optional) Creating a Rate Limiting Policy',
                        local: 'vmware_gud_0025.html'
                      },
                      {
                        id: 1456,
                        parentId: 1453,
                        name: 'Step 3: Creating a Backup SLA',
                        local: 'vmware_gud_0027.html'
                      },
                      {
                        id: 1457,
                        parentId: 1453,
                        name: 'Step 4: Performing Backup',
                        local: 'vmware_gud_0028.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1445,
                parentId: 1281,
                name: 'Replication',
                local: 'vmware_gud_0031.html',
                children: [
                  {
                    id: 1458,
                    parentId: 1445,
                    name: 'Replicating Hyper-V VM Copies',
                    local: 'vmware_gud_0034.html',
                    children: [
                      {
                        id: 1459,
                        parentId: 1458,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect E1000 (OceanProtect Backup Storage))',
                        local: 'oracle_gud_000037.html'
                      },
                      {
                        id: 1460,
                        parentId: 1458,
                        name:
                          'Step1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances)',
                        local: 'en-us_topic_0000002128093497.html'
                      },
                      {
                        id: 1461,
                        parentId: 1458,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'vmware_gud_0037.html'
                      },
                      {
                        id: 1462,
                        parentId: 1458,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'vmware_gud_0038.html'
                      },
                      {
                        id: 1463,
                        parentId: 1458,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'vmware_gud_0039.html'
                      },
                      {
                        id: 1464,
                        parentId: 1458,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'vmware_gud_0040_0.html'
                      },
                      {
                        id: 1465,
                        parentId: 1458,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'vmware_gud_0041.html'
                      },
                      {
                        id: 1466,
                        parentId: 1458,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'vmware_gud_0042.html'
                      },
                      {
                        id: 1467,
                        parentId: 1458,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'vmware_gud_0043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1446,
                parentId: 1281,
                name: 'Archiving',
                local: 'vmware_gud_0044.html',
                children: [
                  {
                    id: 1468,
                    parentId: 1446,
                    name: 'Archiving Hyper-V Backup Copies',
                    local: 'vmware_gud_0047.html',
                    children: [
                      {
                        id: 1470,
                        parentId: 1468,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'vmware_gud_0048.html',
                        children: [
                          {
                            id: 1472,
                            parentId: 1470,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'vmware_gud_0049.html'
                          },
                          {
                            id: 1473,
                            parentId: 1470,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'vmware_gud_0050.html'
                          }
                        ]
                      },
                      {
                        id: 1471,
                        parentId: 1468,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'vmware_gud_0051.html'
                      }
                    ]
                  },
                  {
                    id: 1469,
                    parentId: 1446,
                    name: 'Archiving Hyper-V Replication Copies',
                    local: 'vmware_gud_0052.html',
                    children: [
                      {
                        id: 1474,
                        parentId: 1469,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'vmware_gud_0053.html'
                      },
                      {
                        id: 1475,
                        parentId: 1469,
                        name:
                          'Step 2: Creating Periodic Archives of Replication Copies',
                        local: 'vmware_gud_0054.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1447,
                parentId: 1281,
                name: 'Restoration',
                local: 'vmware_gud_0055.html',
                children: [
                  {
                    id: 1476,
                    parentId: 1447,
                    name: 'Restoring Hyper-V VMs',
                    local: 'vmware_gud_0058.html'
                  },
                  {
                    id: 1477,
                    parentId: 1447,
                    name: 'Restoring Hyper-V VM Disks',
                    local: 'vmware_gud_0059.html'
                  },
                  {
                    id: 1478,
                    parentId: 1447,
                    name: 'Restoring Files on Hyper-V VMs',
                    local: 'vmware_gud_0060.html'
                  }
                ]
              },
              {
                id: 1448,
                parentId: 1281,
                name: 'Global Search',
                local: 'vmware_gud_0074.html',
                children: [
                  {
                    id: 1479,
                    parentId: 1448,
                    name: 'Global Search for Copies',
                    local: 'vmware_gud_0076_0.html'
                  },
                  {
                    id: 1480,
                    parentId: 1448,
                    name: 'Global Search for Resources',
                    local: 'vmware_gud_0077.html'
                  },
                  {
                    id: 1481,
                    parentId: 1448,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002096124161.html'
                  }
                ]
              },
              {
                id: 1449,
                parentId: 1281,
                name: 'SLA ',
                local: 'vmware_gud_0080.html',
                children: [
                  {
                    id: 1482,
                    parentId: 1449,
                    name: 'Viewing SLA Information',
                    local: 'vmware_gud_0082.html'
                  },
                  {
                    id: 1483,
                    parentId: 1449,
                    name: 'Managing SLAs',
                    local: 'vmware_gud_0083.html'
                  }
                ]
              },
              {
                id: 1450,
                parentId: 1281,
                name: 'Copies',
                local: 'vmware_gud_0084.html',
                children: [
                  {
                    id: 1484,
                    parentId: 1450,
                    name: 'Viewing Hyper-V Copy Information',
                    local: 'vmware_gud_0085.html'
                  },
                  {
                    id: 1485,
                    parentId: 1450,
                    name: 'Managing Hyper-V Copies',
                    local: 'vmware_gud_0086.html'
                  }
                ]
              },
              {
                id: 1451,
                parentId: 1281,
                name: 'Hyper-V Virtualization Environment',
                local: 'vmware_gud_0087.html',
                children: [
                  {
                    id: 1486,
                    parentId: 1451,
                    name:
                      'Viewing Information About a Hyper-V Virtualization Environment',
                    local: 'vmware_gud_0088.html'
                  },
                  {
                    id: 1487,
                    parentId: 1451,
                    name: 'Managing Hyper-V Registration Information',
                    local: 'vmware_gud_0089.html'
                  },
                  {
                    id: 1488,
                    parentId: 1451,
                    name: 'Managing Clusters/Hosts/VMs/VM Groups',
                    local: 'vmware_gud_0090.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1282,
            parentId: 15,
            name:
              'FusionOne Compute Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000002085703925.html',
            children: [
              {
                id: 1489,
                parentId: 1282,
                name: 'Backup',
                local: 'fc_gud_0009.html',
                children: [
                  {
                    id: 1497,
                    parentId: 1489,
                    name: 'Preparations for Backup',
                    local: 'fc_gud_0012.html'
                  },
                  {
                    id: 1498,
                    parentId: 1489,
                    name: 'Backing Up a FusionOne Compute VM',
                    local: 'fc_gud_0013.html',
                    children: [
                      {
                        id: 1499,
                        parentId: 1498,
                        name:
                          'Step 1: Create a FusionOne Compute Interconnection User',
                        local: 'fc_gud_0014.html'
                      },
                      {
                        id: 1500,
                        parentId: 1498,
                        name:
                          'Step 2: Registering the FusionOne Compute Virtualization Environment',
                        local: 'fc_gud_0015.html'
                      },
                      {
                        id: 1501,
                        parentId: 1498,
                        name:
                          'Step 3: (Optional) Creating a FusionOne Compute VM Group',
                        local: 'fc_gud_0024_1.html'
                      },
                      {
                        id: 1502,
                        parentId: 1498,
                        name:
                          'Step 4: (Optional) Creating a Rate Limiting Policy',
                        local: 'fc_gud_0016.html'
                      },
                      {
                        id: 1503,
                        parentId: 1498,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'fc_gud_0017.html'
                      },
                      {
                        id: 1504,
                        parentId: 1498,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'fc_gud_0018.html'
                      },
                      {
                        id: 1505,
                        parentId: 1498,
                        name: 'Step 7: Performing Backup',
                        local: 'fc_gud_0019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1490,
                parentId: 1282,
                name: 'Replication',
                local: 'fc_gud_0022.html',
                children: [
                  {
                    id: 1506,
                    parentId: 1490,
                    name: 'Replicating a FusionOne Compute VM Copy',
                    local: 'fc_gud_0024.html',
                    children: [
                      {
                        id: 1507,
                        parentId: 1506,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'foc_gud_0025_1.html'
                      },
                      {
                        id: 1508,
                        parentId: 1506,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances)',
                        local: 'fc_gud_0026_1_1.html'
                      },
                      {
                        id: 1509,
                        parentId: 1506,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'fc_gud_0027.html'
                      },
                      {
                        id: 1510,
                        parentId: 1506,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'fc_gud_0028.html'
                      },
                      {
                        id: 1511,
                        parentId: 1506,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'fc_gud_0029.html'
                      },
                      {
                        id: 1512,
                        parentId: 1506,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'foc_gud_0031.html'
                      },
                      {
                        id: 1513,
                        parentId: 1506,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'fc_gud_0031.html'
                      },
                      {
                        id: 1514,
                        parentId: 1506,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'fc_gud_0032.html'
                      },
                      {
                        id: 1515,
                        parentId: 1506,
                        name:
                          '(Optional) Step 8: Creating a Bidirectional Replication or Cascading Replication SLA',
                        local: 'fc_gud_0033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1491,
                parentId: 1282,
                name: 'Archiving',
                local: 'fc_gud_0034.html',
                children: [
                  {
                    id: 1516,
                    parentId: 1491,
                    name: 'Archiving FusionOne Compute Backup Copies',
                    local: 'fc_gud_0037.html',
                    children: [
                      {
                        id: 1518,
                        parentId: 1516,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'fc_gud_0038.html',
                        children: [
                          {
                            id: 1520,
                            parentId: 1518,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'fc_gud_0039.html'
                          },
                          {
                            id: 1521,
                            parentId: 1518,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'fc_gud_0040.html'
                          }
                        ]
                      },
                      {
                        id: 1519,
                        parentId: 1516,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'fc_gud_0041.html'
                      }
                    ]
                  },
                  {
                    id: 1517,
                    parentId: 1491,
                    name: 'Archiving FusionOne Compute Replication Copies',
                    local: 'fc_gud_0042.html',
                    children: [
                      {
                        id: 1522,
                        parentId: 1517,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'fc_gud_0043.html'
                      },
                      {
                        id: 1523,
                        parentId: 1517,
                        name:
                          'Step 2: Creating Periodic Archives of Replication Copies',
                        local: 'fc_gud_0044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1492,
                parentId: 1282,
                name: 'Restoration',
                local: 'fc_gud_0045.html',
                children: [
                  {
                    id: 1524,
                    parentId: 1492,
                    name: 'Restoring FusionOne Compute VMs',
                    local: 'fc_gud_0048.html'
                  },
                  {
                    id: 1525,
                    parentId: 1492,
                    name: 'Restoring FusionOne Compute VM Disks',
                    local: 'fc_gud_0049.html'
                  },
                  {
                    id: 1526,
                    parentId: 1492,
                    name: 'Restoring Files on a FusionOne Compute VM',
                    local: 'fc_gud_re1.html'
                  }
                ]
              },
              {
                id: 1493,
                parentId: 1282,
                name: 'Global Search',
                local: 'fc_gud_gs1.html',
                children: [
                  {
                    id: 1527,
                    parentId: 1493,
                    name: 'About Global Search',
                    local: 'en-us_topic_0000002038884797.html'
                  },
                  {
                    id: 1528,
                    parentId: 1493,
                    name: 'Global Search for Copies',
                    local: 'fc_gud_gs3.html'
                  },
                  {
                    id: 1529,
                    parentId: 1493,
                    name: 'Global Search for Resources',
                    local: 'fc_gud_0050.html'
                  },
                  {
                    id: 1530,
                    parentId: 1493,
                    name: 'Tag-based Global Search',
                    local: 'en-us_topic_0000002002488232.html'
                  }
                ]
              },
              {
                id: 1494,
                parentId: 1282,
                name: 'SLA ',
                local: 'fc_gud_0053.html',
                children: [
                  {
                    id: 1531,
                    parentId: 1494,
                    name: 'Viewing SLA Information',
                    local: 'fc_gud_0055.html'
                  },
                  {
                    id: 1532,
                    parentId: 1494,
                    name: 'Managing SLAs',
                    local: 'fc_gud_0056.html'
                  }
                ]
              },
              {
                id: 1495,
                parentId: 1282,
                name: 'Copies',
                local: 'fc_gud_0057.html',
                children: [
                  {
                    id: 1533,
                    parentId: 1495,
                    name: 'Viewing FusionOne Compute Copy Data',
                    local: 'fc_gud_0058.html'
                  },
                  {
                    id: 1534,
                    parentId: 1495,
                    name: 'Managing FusionOne Compute Copies',
                    local: 'fc_gud_0059.html'
                  }
                ]
              },
              {
                id: 1496,
                parentId: 1282,
                name: 'FusionOne Compute Virtualization Environment',
                local: 'fc_gud_0060.html',
                children: [
                  {
                    id: 1535,
                    parentId: 1496,
                    name:
                      'Querying Information About the FusionOne Compute Virtualization Environment',
                    local: 'fc_gud_0061.html'
                  },
                  {
                    id: 1536,
                    parentId: 1496,
                    name: 'Managing FusionOne Compute Registration Information',
                    local: 'fc_gud_0062.html'
                  },
                  {
                    id: 1537,
                    parentId: 1496,
                    name: 'Managing Clusters/Hosts/VMs/VM Groups',
                    local: 'fc_gud_0063.html'
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
        name: 'Containers',
        local: 'en-us_topic_0000001918630668.html',
        children: [
          {
            id: 1538,
            parentId: 16,
            name: 'Kubernetes CSI Data Protection',
            local: 'en-us_topic_0000001873759377.html',
            children: [
              {
                id: 1540,
                parentId: 1538,
                name: 'Backup',
                local: 'kubernetes_CSI_00006.html',
                children: [
                  {
                    id: 1549,
                    parentId: 1540,
                    name: 'Preparing for Backup (Applicable to FusionCompute)',
                    local: 'kubernetes_CSI_000091.html',
                    children: [
                      {
                        id: 1554,
                        parentId: 1549,
                        name:
                          'Uploading the Kubernetes Installation Package to the Image Repository',
                        local: 'kubernetes_CSI_00069.html'
                      },
                      {
                        id: 1555,
                        parentId: 1549,
                        name: 'Obtaining the kubeconfig Configuration File',
                        local: 'kubernetes_CSI_00065.html'
                      }
                    ]
                  },
                  {
                    id: 1550,
                    parentId: 1540,
                    name: 'Preparing for Backup (Applicable to CCE)',
                    local: 'kubernetes_CSI_000092.html',
                    children: [
                      {
                        id: 1556,
                        parentId: 1550,
                        name:
                          'Uploading and Updating the Kubernetes Installation Package',
                        local: 'kubernetes_CSI_00102.html'
                      },
                      {
                        id: 1557,
                        parentId: 1550,
                        name: 'Obtaining the kubeconfig Configuration File',
                        local: 'kubernetes_CSI_00078.html'
                      }
                    ]
                  },
                  {
                    id: 1551,
                    parentId: 1540,
                    name: 'Preparing for Backup (Applicable to OpenShift)',
                    local: 'kubernetes_CSI_00078_2.html',
                    children: [
                      {
                        id: 1558,
                        parentId: 1551,
                        name:
                          'Uploading the Kubernetes Installation Package and Obtaining the Image Name and Tag Information',
                        local: 'kubernetes_CSI_00078_3.html'
                      },
                      {
                        id: 1559,
                        parentId: 1551,
                        name: 'Obtaining the kubeconfig Configuration File',
                        local: 'kubernetes_CSI_00078_4.html'
                      },
                      {
                        id: 1560,
                        parentId: 1551,
                        name: 'Obtaining the Token Information',
                        local: 'kubernetes_CSI_00078_5.html'
                      }
                    ]
                  },
                  {
                    id: 1552,
                    parentId: 1540,
                    name:
                      'Preparing for Backup (Applicable to Native Kubernetes)',
                    local: 'kubernetes_CSI_00078_6.html',
                    children: [
                      {
                        id: 1561,
                        parentId: 1552,
                        name:
                          'Uploading the Kubernetes Installation Package to the Kubernetes Cluster',
                        local: 'kubernetes_CSI_00078_7.html'
                      },
                      {
                        id: 1562,
                        parentId: 1552,
                        name: 'Obtaining the kubeconfig Configuration File',
                        local: 'kubernetes_CSI_00078_8.html'
                      }
                    ]
                  },
                  {
                    id: 1553,
                    parentId: 1540,
                    name: 'Backing Up Namespaces or Datasets',
                    local: 'kubernetes_CSI_00010.html',
                    children: [
                      {
                        id: 1563,
                        parentId: 1553,
                        name:
                          'Step 1: (Optional) Querying Node Labels of the Kubernetes Cluster',
                        local: 'kubernetes_CSI_00010_1.html'
                      },
                      {
                        id: 1564,
                        parentId: 1553,
                        name:
                          'Step 2: (Optional) Generating a Token with the Minimum Permissions',
                        local: 'kubernetes_CSI_00077.html'
                      },
                      {
                        id: 1565,
                        parentId: 1553,
                        name: 'Step 3: Registering a Cluster',
                        local: 'kubernetes_CSI_00011.html'
                      },
                      {
                        id: 1566,
                        parentId: 1553,
                        name: 'Step 4: Registering a Dataset',
                        local: 'kubernetes_CSI_00012.html'
                      },
                      {
                        id: 1567,
                        parentId: 1553,
                        name: 'Step 5: Authorizing a Resource',
                        local: 'kubernetes_CSI_00013.html'
                      },
                      {
                        id: 1568,
                        parentId: 1553,
                        name: 'Step 6: Creating a Rate Limiting Policy',
                        local: 'kubernetes_CSI_00014.html'
                      },
                      {
                        id: 1569,
                        parentId: 1553,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'kubernetes_CSI_00015.html'
                      },
                      {
                        id: 1570,
                        parentId: 1553,
                        name: 'Step 8: Performing Backup',
                        local: 'kubernetes_CSI_00016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1541,
                parentId: 1538,
                name: 'Replication',
                local: 'kubernetes_CSI_00019.html',
                children: [
                  {
                    id: 1571,
                    parentId: 1541,
                    name: 'Replicating Kubernetes CSI Copies',
                    local: 'kubernetes_CSI_00022.html',
                    children: [
                      {
                        id: 1572,
                        parentId: 1571,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'kubernetes_CSI_00024.html'
                      },
                      {
                        id: 1573,
                        parentId: 1571,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'kubernetes_CSI_00024_1.html'
                      },
                      {
                        id: 1574,
                        parentId: 1571,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'kubernetes_CSI_00025.html'
                      },
                      {
                        id: 1575,
                        parentId: 1571,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'kubernetes_CSI_00026.html'
                      },
                      {
                        id: 1576,
                        parentId: 1571,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'kubernetes_CSI_00027.html'
                      },
                      {
                        id: 1577,
                        parentId: 1571,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'kubernetes_CSI_00028.html'
                      },
                      {
                        id: 1578,
                        parentId: 1571,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'kubernetes_CSI_00028_a1.html'
                      },
                      {
                        id: 1579,
                        parentId: 1571,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'kubernetes_CSI_00029.html'
                      },
                      {
                        id: 1580,
                        parentId: 1571,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'kubernetes_CSI_00030.html'
                      },
                      {
                        id: 1581,
                        parentId: 1571,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'kubernetes_CSI_00031.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1542,
                parentId: 1538,
                name: 'Archiving',
                local: 'kubernetes_CSI_00032.html',
                children: [
                  {
                    id: 1582,
                    parentId: 1542,
                    name: 'Archiving Kubernetes CSI Backup Copies',
                    local: 'kubernetes_CSI_00035.html',
                    children: [
                      {
                        id: 1584,
                        parentId: 1582,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'kubernetes_CSI_00036.html',
                        children: [
                          {
                            id: 1586,
                            parentId: 1584,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'kubernetes_CSI_00037.html'
                          },
                          {
                            id: 1587,
                            parentId: 1584,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'kubernetes_CSI_00038.html'
                          }
                        ]
                      },
                      {
                        id: 1585,
                        parentId: 1582,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'kubernetes_CSI_00039.html'
                      }
                    ]
                  },
                  {
                    id: 1583,
                    parentId: 1542,
                    name: 'Archiving Kubernetes CSI Replication Copies',
                    local: 'kubernetes_CSI_00040.html',
                    children: [
                      {
                        id: 1588,
                        parentId: 1583,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'kubernetes_CSI_00041.html'
                      },
                      {
                        id: 1589,
                        parentId: 1583,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'kubernetes_CSI_00042.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1543,
                parentId: 1538,
                name: 'Restoration',
                local: 'kubernetes_CSI_00043.html',
                children: [
                  {
                    id: 1590,
                    parentId: 1543,
                    name: 'Restoring Namespaces or Datasets',
                    local: 'kubernetes_CSI_00046.html'
                  },
                  {
                    id: 1591,
                    parentId: 1543,
                    name: 'Restoring PVCs',
                    local: 'kubernetes_CSI_00047.html'
                  }
                ]
              },
              {
                id: 1544,
                parentId: 1538,
                name: 'Global Search',
                local: 'kubernetes_CSI_00043_a1.html',
                children: [
                  {
                    id: 1592,
                    parentId: 1544,
                    name: 'Global Search for Resources',
                    local: 'kubernetes_CSI_00043_a2.html'
                  },
                  {
                    id: 1593,
                    parentId: 1544,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'kubernetes_CSI_00043_a3.html'
                  }
                ]
              },
              {
                id: 1545,
                parentId: 1538,
                name: 'SLA ',
                local: 'kubernetes_CSI_00051.html',
                children: [
                  {
                    id: 1594,
                    parentId: 1545,
                    name: 'About SLA',
                    local: 'kubernetes_CSI_00052.html'
                  },
                  {
                    id: 1595,
                    parentId: 1545,
                    name: 'Viewing SLA Information',
                    local: 'kubernetes_CSI_00053.html'
                  },
                  {
                    id: 1596,
                    parentId: 1545,
                    name: 'Managing SLAs',
                    local: 'kubernetes_CSI_00054.html'
                  }
                ]
              },
              {
                id: 1546,
                parentId: 1538,
                name: 'Copies',
                local: 'kubernetes_CSI_00055.html',
                children: [
                  {
                    id: 1597,
                    parentId: 1546,
                    name: 'Viewing Kubernetes CSI Copy Information',
                    local: 'kubernetes_CSI_00056.html'
                  },
                  {
                    id: 1598,
                    parentId: 1546,
                    name: 'Managing Kubernetes CSI Copies',
                    local: 'kubernetes_CSI_00057.html'
                  }
                ]
              },
              {
                id: 1547,
                parentId: 1538,
                name: 'Cluster/Namespace/Dataset',
                local: 'kubernetes_CSI_00058.html',
                children: [
                  {
                    id: 1599,
                    parentId: 1547,
                    name: 'Viewing Information',
                    local: 'kubernetes_CSI_00059.html'
                  },
                  {
                    id: 1600,
                    parentId: 1547,
                    name: 'Managing Clusters',
                    local: 'kubernetes_CSI_00060.html'
                  },
                  {
                    id: 1601,
                    parentId: 1547,
                    name: 'Managing Namespace and Datasets',
                    local: 'kubernetes_CSI_00061.html'
                  }
                ]
              },
              {
                id: 1548,
                parentId: 1538,
                name: 'FAQs',
                local: 'kubernetes_CSI_00062.html',
                children: [
                  {
                    id: 1602,
                    parentId: 1548,
                    name: 'Logging In to the OceanProtect WebUI',
                    local: 'kubernetes_CSI_00063.html'
                  },
                  {
                    id: 1603,
                    parentId: 1548,
                    name: 'Logging In to DeviceManager',
                    local: 'kubernetes_CSI_00063_1.html'
                  },
                  {
                    id: 1604,
                    parentId: 1548,
                    name:
                      'Obtaining the Certificate Value During Token Authentication (for CCE)',
                    local: 'kubernetes_CSI_00079.html'
                  },
                  {
                    id: 1605,
                    parentId: 1548,
                    name:
                      'Pod Configuration in the Production Environment for Application-Consistent Backup (General)',
                    local: 'kubernetes_CSI_00066.html'
                  },
                  {
                    id: 1606,
                    parentId: 1548,
                    name:
                      'Pod Configuration in the Production Environment for Application-Consistent Backup (MySQL as the Containerized Application)',
                    local: 'kubernetes_CSI_00067.html'
                  },
                  {
                    id: 1607,
                    parentId: 1548,
                    name:
                      'Pod Configuration in the Production Environment for Application-Consistent Backup (openGauss as the Containerized Application)',
                    local: 'kubernetes_CSI_00068.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1539,
            parentId: 16,
            name: 'Kubernetes FlexVolume Data Protection',
            local: 'en-us_topic_0000001827039668.html',
            children: [
              {
                id: 1608,
                parentId: 1539,
                name: 'Backup',
                local: 'kubernetes_gud_00007.html',
                children: [
                  {
                    id: 1616,
                    parentId: 1608,
                    name: 'Preparations for Backup',
                    local: 'kubernetes_gud_00010.html'
                  },
                  {
                    id: 1617,
                    parentId: 1608,
                    name: 'Backing Up a Namespace or StatefulSet',
                    local: 'kubernetes_gud_00011.html',
                    children: [
                      {
                        id: 1618,
                        parentId: 1617,
                        name: 'Step 1: Registering a Cluster',
                        local: 'kubernetes_gud_00012.html'
                      },
                      {
                        id: 1619,
                        parentId: 1617,
                        name: 'Step 2: Authorizing a Resource',
                        local: 'kubernetes_gud_00013.html'
                      },
                      {
                        id: 1620,
                        parentId: 1617,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'kubernetes_gud_00014.html'
                      },
                      {
                        id: 1621,
                        parentId: 1617,
                        name: 'Step 4: Creating a Rate Limiting Policy',
                        local: 'kubernetes_gud_00015.html'
                      },
                      {
                        id: 1622,
                        parentId: 1617,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'kubernetes_gud_00016.html'
                      },
                      {
                        id: 1623,
                        parentId: 1617,
                        name: 'Step 6: Performing Backup',
                        local: 'kubernetes_gud_00017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1609,
                parentId: 1539,
                name: 'Replication',
                local: 'kubernetes_gud_00020.html',
                children: [
                  {
                    id: 1624,
                    parentId: 1609,
                    name: 'Replicating Kubernetes FlexVolume Copies',
                    local: 'kubernetes_gud_00023.html',
                    children: [
                      {
                        id: 1625,
                        parentId: 1624,
                        name: 'Planning the Replication Network',
                        local: 'kubernetes_gud_00024.html'
                      },
                      {
                        id: 1626,
                        parentId: 1624,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'kubernetes_gud_00025.html'
                      },
                      {
                        id: 1627,
                        parentId: 1624,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_3.html'
                      },
                      {
                        id: 1628,
                        parentId: 1624,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'kubernetes_gud_00026.html'
                      },
                      {
                        id: 1629,
                        parentId: 1624,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'kubernetes_gud_00027.html'
                      },
                      {
                        id: 1630,
                        parentId: 1624,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'kubernetes_gud_00028.html'
                      },
                      {
                        id: 1631,
                        parentId: 1624,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'kubernetes_gud_00029.html'
                      },
                      {
                        id: 1632,
                        parentId: 1624,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'kubernetes_gud_00029_a1.html'
                      },
                      {
                        id: 1633,
                        parentId: 1624,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'kubernetes_gud_00030.html'
                      },
                      {
                        id: 1634,
                        parentId: 1624,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'kubernetes_gud_00031.html'
                      },
                      {
                        id: 1635,
                        parentId: 1624,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'kubernetes_gud_00032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1610,
                parentId: 1539,
                name: 'Archiving',
                local: 'kubernetes_gud_00033.html',
                children: [
                  {
                    id: 1636,
                    parentId: 1610,
                    name: 'Archiving Kubernetes FlexVolume Backup Copies',
                    local: 'kubernetes_gud_00036.html',
                    children: [
                      {
                        id: 1638,
                        parentId: 1636,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'kubernetes_gud_00037.html',
                        children: [
                          {
                            id: 1640,
                            parentId: 1638,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'kubernetes_gud_00038.html'
                          },
                          {
                            id: 1641,
                            parentId: 1638,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'kubernetes_gud_00039.html'
                          }
                        ]
                      },
                      {
                        id: 1639,
                        parentId: 1636,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'kubernetes_gud_00040.html'
                      }
                    ]
                  },
                  {
                    id: 1637,
                    parentId: 1610,
                    name: 'Archiving Kubernetes FlexVolume Replication Copies',
                    local: 'kubernetes_gud_00041.html',
                    children: [
                      {
                        id: 1642,
                        parentId: 1637,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'kubernetes_gud_00042.html'
                      },
                      {
                        id: 1643,
                        parentId: 1637,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'kubernetes_gud_00043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1611,
                parentId: 1539,
                name: 'Restoration',
                local: 'kubernetes_gud_00044.html',
                children: [
                  {
                    id: 1644,
                    parentId: 1611,
                    name: 'Performing StatefulSet Restoration',
                    local: 'kubernetes_gud_00047.html'
                  }
                ]
              },
              {
                id: 1612,
                parentId: 1539,
                name: 'Global Search',
                local: 'kubernetes_gud_00044_a1.html',
                children: [
                  {
                    id: 1645,
                    parentId: 1612,
                    name: 'Global Search for Resources',
                    local: 'kubernetes_gud_00044_a2.html'
                  },
                  {
                    id: 1646,
                    parentId: 1612,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'kubernetes_gud_00044_a3.html'
                  }
                ]
              },
              {
                id: 1613,
                parentId: 1539,
                name: 'SLA ',
                local: 'kubernetes_gud_00051.html',
                children: [
                  {
                    id: 1647,
                    parentId: 1613,
                    name: 'About SLA',
                    local: 'kubernetes_gud_00052.html'
                  },
                  {
                    id: 1648,
                    parentId: 1613,
                    name: 'Viewing SLA Information',
                    local: 'kubernetes_gud_00053.html'
                  },
                  {
                    id: 1649,
                    parentId: 1613,
                    name: 'Managing SLAs',
                    local: 'kubernetes_gud_00054.html'
                  }
                ]
              },
              {
                id: 1614,
                parentId: 1539,
                name: 'Copies',
                local: 'kubernetes_gud_00055.html',
                children: [
                  {
                    id: 1650,
                    parentId: 1614,
                    name: 'Viewing Kubernetes FlexVolume Copy Information',
                    local: 'kubernetes_gud_00056.html'
                  },
                  {
                    id: 1651,
                    parentId: 1614,
                    name: 'Managing Kubernetes FlexVolume Copies',
                    local: 'kubernetes_gud_00057.html'
                  }
                ]
              },
              {
                id: 1615,
                parentId: 1539,
                name: 'Cluster, Namespace, and StatefulSet',
                local: 'kubernetes_gud_00058.html',
                children: [
                  {
                    id: 1652,
                    parentId: 1615,
                    name: 'Viewing Information',
                    local: 'kubernetes_gud_00059.html'
                  },
                  {
                    id: 1653,
                    parentId: 1615,
                    name: 'Managing Clusters',
                    local: 'kubernetes_gud_00060.html'
                  },
                  {
                    id: 1654,
                    parentId: 1615,
                    name: 'Managing Namespaces and StatefulSets',
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
        name: 'Cloud Platforms',
        local: 'en-us_topic_0000001948269725.html',
        children: [
          {
            id: 1655,
            parentId: 17,
            name: 'Huawei Cloud Stack Data Protection',
            local: 'en-us_topic_0000001827039672.html',
            children: [
              {
                id: 1659,
                parentId: 1655,
                name: 'Backup',
                local: 'hcs_gud_0007.html',
                children: [
                  {
                    id: 1667,
                    parentId: 1659,
                    name: 'Preparing for the Backup',
                    local: 'hcs_gud_0010.html'
                  },
                  {
                    id: 1668,
                    parentId: 1659,
                    name: 'Backing Up a Cloud Server or EVS Disk',
                    local: 'hcs_gud_0011.html',
                    children: [
                      {
                        id: 1669,
                        parentId: 1668,
                        name: 'Step 1: Obtaining the Certificate',
                        local: 'hcs_gud_0012.html'
                      },
                      {
                        id: 1670,
                        parentId: 1668,
                        name: 'Step 2: Registering Huawei Cloud Stack',
                        local: 'hcs_gud_0014.html'
                      },
                      {
                        id: 1671,
                        parentId: 1668,
                        name:
                          'Step 3: Adding a Tenant and Authorizing Resources',
                        local: 'hcs_gud_0015.html'
                      },
                      {
                        id: 1672,
                        parentId: 1668,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'hcs_gud_0016.html'
                      },
                      {
                        id: 1673,
                        parentId: 1668,
                        name:
                          'Step 5: (Optional) Creating a Rate Limiting Policy',
                        local: 'hcs_gud_0017.html'
                      },
                      {
                        id: 1674,
                        parentId: 1668,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'hcs_gud_0018.html'
                      },
                      {
                        id: 1675,
                        parentId: 1668,
                        name: 'Step 7: Performing Backup',
                        local: 'hcs_gud_0019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1660,
                parentId: 1655,
                name: 'Replication',
                local: 'hcs_gud_0022.html',
                children: [
                  {
                    id: 1676,
                    parentId: 1660,
                    name: 'Replicating a Huawei Cloud Stack Copy',
                    local: 'hcs_gud_0025.html',
                    children: [
                      {
                        id: 1677,
                        parentId: 1676,
                        name: 'Planning the Replication Network',
                        local: 'hcs_gud_0026.html'
                      },
                      {
                        id: 1678,
                        parentId: 1676,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'hcs_gud_0027.html'
                      },
                      {
                        id: 1679,
                        parentId: 1676,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'hcs_gud_0027_1.html'
                      },
                      {
                        id: 1680,
                        parentId: 1676,
                        name: 'Step 2: (Optional) Creating an IPsec Policy',
                        local: 'hcs_gud_0028.html'
                      },
                      {
                        id: 1681,
                        parentId: 1676,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'hcs_gud_0029.html'
                      },
                      {
                        id: 1682,
                        parentId: 1676,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'hcs_gud_0030.html'
                      },
                      {
                        id: 1683,
                        parentId: 1676,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'hcs_gud_0031.html'
                      },
                      {
                        id: 1684,
                        parentId: 1676,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002044303453.html'
                      },
                      {
                        id: 1685,
                        parentId: 1676,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'hcs_gud_0032.html'
                      },
                      {
                        id: 1686,
                        parentId: 1676,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'hcs_gud_0033.html'
                      },
                      {
                        id: 1687,
                        parentId: 1676,
                        name:
                          'Step 8: (Optional) Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'hcs_gud_0034.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1661,
                parentId: 1655,
                name: 'Archiving',
                local: 'hcs_gud_0035.html',
                children: [
                  {
                    id: 1688,
                    parentId: 1661,
                    name: 'Archiving Huawei Cloud Stack Backup Copies',
                    local: 'hcs_gud_0038.html',
                    children: [
                      {
                        id: 1690,
                        parentId: 1688,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'hcs_gud_0039.html',
                        children: [
                          {
                            id: 1692,
                            parentId: 1690,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'hcs_gud_0040.html'
                          },
                          {
                            id: 1693,
                            parentId: 1690,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'hcs_gud_0041.html'
                          }
                        ]
                      },
                      {
                        id: 1691,
                        parentId: 1688,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'hcs_gud_0042.html'
                      }
                    ]
                  },
                  {
                    id: 1689,
                    parentId: 1661,
                    name: 'Archiving Huawei Cloud Stack Replication Copies',
                    local: 'hcs_gud_0043.html',
                    children: [
                      {
                        id: 1694,
                        parentId: 1689,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'hcs_gud_0044.html'
                      },
                      {
                        id: 1695,
                        parentId: 1689,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'hcs_gud_0045.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1662,
                parentId: 1655,
                name: 'Restoration',
                local: 'hcs_gud_0046.html',
                children: [
                  {
                    id: 1696,
                    parentId: 1662,
                    name: 'Restoring a Cloud Server or EVS Disk',
                    local: 'hcs_gud_0049.html'
                  },
                  {
                    id: 1697,
                    parentId: 1662,
                    name: 'Restoring Files on an ECS',
                    local: 'hcs_gud_re1.html'
                  }
                ]
              },
              {
                id: 1663,
                parentId: 1655,
                name: 'Global Search',
                local: 'hcs_gud_gs1.html',
                children: [
                  {
                    id: 1698,
                    parentId: 1663,
                    name: 'About Global Search',
                    local: 'en-us_topic_0000002002687814.html'
                  },
                  {
                    id: 1699,
                    parentId: 1663,
                    name: 'Global Search for Copies',
                    local: 'hcs_gud_gs2.html'
                  },
                  {
                    id: 1700,
                    parentId: 1663,
                    name: 'Global Search for Resources',
                    local: 'hcs_gud_0050.html'
                  },
                  {
                    id: 1701,
                    parentId: 1663,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002038886953.html'
                  }
                ]
              },
              {
                id: 1664,
                parentId: 1655,
                name: 'SLA ',
                local: 'hcs_gud_0053.html',
                children: [
                  {
                    id: 1702,
                    parentId: 1664,
                    name: 'About SLA',
                    local: 'hcs_gud_0054.html'
                  },
                  {
                    id: 1703,
                    parentId: 1664,
                    name: 'Viewing SLA Information',
                    local: 'hcs_gud_0055.html'
                  },
                  {
                    id: 1704,
                    parentId: 1664,
                    name: 'Managing SLAs',
                    local: 'hcs_gud_0056.html'
                  }
                ]
              },
              {
                id: 1665,
                parentId: 1655,
                name: 'Copies',
                local: 'hcs_gud_0057.html',
                children: [
                  {
                    id: 1705,
                    parentId: 1665,
                    name: 'Viewing Huawei Cloud Stack Copy Information',
                    local: 'hcs_gud_0058.html'
                  },
                  {
                    id: 1706,
                    parentId: 1665,
                    name: 'Managing Huawei Cloud Stack Copies',
                    local: 'hcs_gud_0059.html'
                  }
                ]
              },
              {
                id: 1666,
                parentId: 1655,
                name: 'Huawei Cloud Stack Environment',
                local: 'hcs_gud_0060.html',
                children: [
                  {
                    id: 1707,
                    parentId: 1666,
                    name: 'Viewing Huawei Cloud Stack information',
                    local: 'hcs_gud_0061.html'
                  },
                  {
                    id: 1708,
                    parentId: 1666,
                    name:
                      'Managing Huawei Cloud Stack Registration Information',
                    local: 'hcs_gud_0062.html'
                  },
                  {
                    id: 1709,
                    parentId: 1666,
                    name: 'Managing Tenants',
                    local: 'hcs_gud_0063.html'
                  },
                  {
                    id: 1710,
                    parentId: 1666,
                    name: 'Managing Projects, Resource Sets, and ECSs',
                    local: 'hcs_gud_0064.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1656,
            parentId: 17,
            name: 'OpenStack Data Protection',
            local: 'en-us_topic_0000001873679145.html',
            children: [
              {
                id: 1711,
                parentId: 1656,
                name: 'Backup',
                local: 'openstack_00007.html',
                children: [
                  {
                    id: 1719,
                    parentId: 1711,
                    name: 'Preparing for the Backup',
                    local: 'openstack_00010.html'
                  },
                  {
                    id: 1720,
                    parentId: 1711,
                    name: 'Backing Up an OpenStack Cloud Server',
                    local: 'openstack_00011.html',
                    children: [
                      {
                        id: 1721,
                        parentId: 1720,
                        name: 'Step 1: Obtaining the Keystone V3 Address',
                        local: 'Open_Stack_00010_1.html'
                      },
                      {
                        id: 1722,
                        parentId: 1720,
                        name: 'Step 2: Obtaining the Certificate',
                        local: 'openstack_00012.html'
                      },
                      {
                        id: 1723,
                        parentId: 1720,
                        name: 'Step 3: Creating a User for Interconnection',
                        local: 'openstack_00014.html'
                      },
                      {
                        id: 1724,
                        parentId: 1720,
                        name: 'Step 4: Creating a Domain Administrator',
                        local: 'openstack_00015.html'
                      },
                      {
                        id: 1725,
                        parentId: 1720,
                        name: 'Step 5: Registering OpenStack',
                        local: 'openstack_00016.html'
                      },
                      {
                        id: 1726,
                        parentId: 1720,
                        name: 'Step 6: Adding a Domain',
                        local: 'openstack_00017.html'
                      },
                      {
                        id: 1727,
                        parentId: 1720,
                        name:
                          'Step 7: (Optional) Creating a Cloud Server Group',
                        local: 'openstack_000017_1.html'
                      },
                      {
                        id: 1728,
                        parentId: 1720,
                        name: 'Step 8: Creating a Rate Limiting Policy',
                        local: 'openstack_00019.html'
                      },
                      {
                        id: 1729,
                        parentId: 1720,
                        name:
                          'Step 9: (Optional) Enabling Backup Link Encryption',
                        local: 'openstack_00020.html'
                      },
                      {
                        id: 1730,
                        parentId: 1720,
                        name:
                          'Step 10: (Optional) Modifying the Snapshot Quota of a Project',
                        local: 'openstack_00021.html'
                      },
                      {
                        id: 1731,
                        parentId: 1720,
                        name: 'Step 11: Creating a Backup SLA',
                        local: 'openstack_00022.html'
                      },
                      {
                        id: 1732,
                        parentId: 1720,
                        name: 'Step 12: Performing Backup',
                        local: 'openstack_00023.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1712,
                parentId: 1656,
                name: 'Replication',
                local: 'openstack_00026.html',
                children: [
                  {
                    id: 1733,
                    parentId: 1712,
                    name: 'Replicating an OpenStack Copy',
                    local: 'openstack_00029.html',
                    children: [
                      {
                        id: 1734,
                        parentId: 1733,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'openstack_00031.html'
                      },
                      {
                        id: 1735,
                        parentId: 1733,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'openstack_000311.html'
                      },
                      {
                        id: 1736,
                        parentId: 1733,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'openstack_00032.html'
                      },
                      {
                        id: 1737,
                        parentId: 1733,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'openstack_00033.html'
                      },
                      {
                        id: 1738,
                        parentId: 1733,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'openstack_00034.html'
                      },
                      {
                        id: 1739,
                        parentId: 1733,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'openstack_00035.html'
                      },
                      {
                        id: 1740,
                        parentId: 1733,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'Open_Stack_000340.html'
                      },
                      {
                        id: 1741,
                        parentId: 1733,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'openstack_00036.html'
                      },
                      {
                        id: 1742,
                        parentId: 1733,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'openstack_00037.html'
                      },
                      {
                        id: 1743,
                        parentId: 1733,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'openstack_00038.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1713,
                parentId: 1656,
                name: 'Archiving',
                local: 'openstack_00039.html',
                children: [
                  {
                    id: 1744,
                    parentId: 1713,
                    name: 'Archiving OpenStack Backup Copies',
                    local: 'openstack_00042.html',
                    children: [
                      {
                        id: 1746,
                        parentId: 1744,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'openstack_00043.html',
                        children: [
                          {
                            id: 1748,
                            parentId: 1746,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'openstack_00044.html'
                          },
                          {
                            id: 1749,
                            parentId: 1746,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'openstack_00045.html'
                          }
                        ]
                      },
                      {
                        id: 1747,
                        parentId: 1744,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'openstack_00046.html'
                      }
                    ]
                  },
                  {
                    id: 1745,
                    parentId: 1713,
                    name: 'Archiving OpenStack Replication Copies',
                    local: 'openstack_00047.html',
                    children: [
                      {
                        id: 1750,
                        parentId: 1745,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'openstack_00048.html'
                      },
                      {
                        id: 1751,
                        parentId: 1745,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'openstack_00049.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1714,
                parentId: 1656,
                name: 'Restoration',
                local: 'openstack_00050.html',
                children: [
                  {
                    id: 1752,
                    parentId: 1714,
                    name: 'Restoring Cloud Servers',
                    local: 'openstack_00053.html'
                  },
                  {
                    id: 1753,
                    parentId: 1714,
                    name: 'Restoring Cloud Disks',
                    local: 'openstack_00054.html'
                  },
                  {
                    id: 1754,
                    parentId: 1714,
                    name:
                      'Restoring Files (Applicable to 1.6.0 and Later Versions)',
                    local: 'openstack_000541.html'
                  }
                ]
              },
              {
                id: 1715,
                parentId: 1656,
                name: 'Global Search',
                local: 'Open_Stack_000532.html',
                children: [
                  {
                    id: 1755,
                    parentId: 1715,
                    name: 'About Global Search',
                    local: 'openstack_00055.html'
                  },
                  {
                    id: 1756,
                    parentId: 1715,
                    name: 'Global Search for Copies',
                    local: 'vmware_gud_0076_2.html'
                  },
                  {
                    id: 1757,
                    parentId: 1715,
                    name: 'Global Search for Resources',
                    local: 'Open_Stack_000542.html'
                  },
                  {
                    id: 1758,
                    parentId: 1715,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'Open_Stack_000543.html'
                  }
                ]
              },
              {
                id: 1716,
                parentId: 1656,
                name: 'SLA ',
                local: 'openstack_00058.html',
                children: [
                  {
                    id: 1759,
                    parentId: 1716,
                    name: 'About SLA',
                    local: 'openstack_00059.html'
                  },
                  {
                    id: 1760,
                    parentId: 1716,
                    name: 'Viewing SLA Information',
                    local: 'openstack_00060.html'
                  },
                  {
                    id: 1761,
                    parentId: 1716,
                    name: 'Managing SLAs',
                    local: 'openstack_00061.html'
                  }
                ]
              },
              {
                id: 1717,
                parentId: 1656,
                name: 'Copies',
                local: 'openstack_00062.html',
                children: [
                  {
                    id: 1762,
                    parentId: 1717,
                    name: 'Viewing OpenStack Copy Data',
                    local: 'openstack_00063.html'
                  },
                  {
                    id: 1763,
                    parentId: 1717,
                    name: 'Managing OpenStack Copies',
                    local: 'openstack_00064.html'
                  }
                ]
              },
              {
                id: 1718,
                parentId: 1656,
                name: 'OpenStack Environment Information',
                local: 'openstack_00065.html',
                children: [
                  {
                    id: 1764,
                    parentId: 1718,
                    name: 'Viewing OpenStack Information',
                    local: 'openstack_00066.html'
                  },
                  {
                    id: 1765,
                    parentId: 1718,
                    name: 'Managing the OpenStack Cloud Platform',
                    local: 'openstack_00067.html'
                  },
                  {
                    id: 1766,
                    parentId: 1718,
                    name: 'Managing Domains',
                    local: 'openstack_00068.html'
                  },
                  {
                    id: 1767,
                    parentId: 1718,
                    name:
                      'Managing Projects/Cloud Servers or Cloud Server Groups (Applicable to 1.6.0 and Later Versions)',
                    local: 'openstack_00069.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1657,
            parentId: 17,
            name: 'Huawei Cloud Stack GaussDB Data Protection',
            local: 'en-us_topic_0000001826879800.html',
            children: [
              {
                id: 1768,
                parentId: 1657,
                name: 'Backup',
                local: 'hcs_gaussdb_00006.html',
                children: [
                  {
                    id: 1776,
                    parentId: 1768,
                    name: 'Preparing for the Backup',
                    local: 'hcs_gaussdb_00009.html'
                  },
                  {
                    id: 1777,
                    parentId: 1768,
                    name: 'Backing Up Huawei Cloud Stack GaussDB Instances',
                    local: 'hcs_gaussdb_00010.html',
                    children: [
                      {
                        id: 1778,
                        parentId: 1777,
                        name:
                          'Step 1: Registering a Huawei Cloud Stack GaussDB Project',
                        local: 'hcs_gaussdb_00011.html'
                      },
                      {
                        id: 1779,
                        parentId: 1777,
                        name:
                          'Step 2: (Optional) Enabling Backup Link Encryption',
                        local: 'hcs_gaussdb_00012.html'
                      },
                      {
                        id: 1780,
                        parentId: 1777,
                        name: 'Step 3: Creating a Rate Limiting Policy',
                        local: 'hcs_gaussdb_00013.html'
                      },
                      {
                        id: 1781,
                        parentId: 1777,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'hcs_gaussdb_00014.html'
                      },
                      {
                        id: 1782,
                        parentId: 1777,
                        name: 'Step 5: Performing Backup',
                        local: 'hcs_gaussdb_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1769,
                parentId: 1657,
                name: 'Replication',
                local: 'hcs_gaussdb_00018.html',
                children: [
                  {
                    id: 1783,
                    parentId: 1769,
                    name: 'Replicating Huawei Cloud Stack GaussDB Copies',
                    local: 'hcs_gaussdb_00021.html',
                    children: [
                      {
                        id: 1784,
                        parentId: 1783,
                        name: 'Planning the Replication Network',
                        local: 'hcs_gaussdb_00022.html'
                      },
                      {
                        id: 1785,
                        parentId: 1783,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'hcs_gaussdb_00023.html'
                      },
                      {
                        id: 1786,
                        parentId: 1783,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_14.html'
                      },
                      {
                        id: 1787,
                        parentId: 1783,
                        name: 'Step 2: (Optional) Creating an IPsec Policy',
                        local: 'hcs_gaussdb_00024.html'
                      },
                      {
                        id: 1788,
                        parentId: 1783,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'hcs_gaussdb_00025.html'
                      },
                      {
                        id: 1789,
                        parentId: 1783,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'hcs_gaussdb_00026.html'
                      },
                      {
                        id: 1790,
                        parentId: 1783,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'hcs_gaussdb_00027.html'
                      },
                      {
                        id: 1791,
                        parentId: 1783,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'hcs_gaussdb_0002700.html'
                      },
                      {
                        id: 1792,
                        parentId: 1783,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'hcs_gaussdb_00028.html'
                      },
                      {
                        id: 1793,
                        parentId: 1783,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'hcs_gaussdb_00029.html'
                      },
                      {
                        id: 1794,
                        parentId: 1783,
                        name:
                          'Step 8: (Optional) Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'hcs_gaussdb_00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1770,
                parentId: 1657,
                name: 'Archiving',
                local: 'hcs_gaussdb_00031.html',
                children: [
                  {
                    id: 1795,
                    parentId: 1770,
                    name: 'Archiving Huawei Cloud Stack GaussDB Copies',
                    local: 'hcs_gaussdb_00034.html',
                    children: [
                      {
                        id: 1797,
                        parentId: 1795,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'hcs_gaussdb_00035.html',
                        children: [
                          {
                            id: 1799,
                            parentId: 1797,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'hcs_gaussdb_00036.html'
                          },
                          {
                            id: 1800,
                            parentId: 1797,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'hcs_gaussdb_00037.html'
                          }
                        ]
                      },
                      {
                        id: 1798,
                        parentId: 1795,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'hcs_gaussdb_00038.html'
                      }
                    ]
                  },
                  {
                    id: 1796,
                    parentId: 1770,
                    name: 'Archiving Huawei Cloud Stack GaussDB Copies',
                    local: 'hcs_gaussdb_00039.html',
                    children: [
                      {
                        id: 1801,
                        parentId: 1796,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'hcs_gaussdb_00040.html'
                      },
                      {
                        id: 1802,
                        parentId: 1796,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'hcs_gaussdb_00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1771,
                parentId: 1657,
                name: 'Restoration',
                local: 'hcs_gaussdb_00042.html',
                children: [
                  {
                    id: 1803,
                    parentId: 1771,
                    name: 'Restoring Huawei Cloud Stack GaussDB Instances',
                    local: 'hcs_gaussdb_00045.html'
                  }
                ]
              },
              {
                id: 1772,
                parentId: 1657,
                name: 'Global Search',
                local: 'hcs_gaussdb_0004211.html',
                children: [
                  {
                    id: 1804,
                    parentId: 1772,
                    name: 'Global Search for Resources',
                    local: 'hcs_gaussdb_00046.html'
                  },
                  {
                    id: 1805,
                    parentId: 1772,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002050772613.html'
                  }
                ]
              },
              {
                id: 1773,
                parentId: 1657,
                name: 'SLA ',
                local: 'hcs_gaussdb_00049.html',
                children: [
                  {
                    id: 1806,
                    parentId: 1773,
                    name: 'About SLA',
                    local: 'hcs_gaussdb_000491.html'
                  },
                  {
                    id: 1807,
                    parentId: 1773,
                    name: 'Viewing SLA Information',
                    local: 'hcs_gaussdb_00051.html'
                  },
                  {
                    id: 1808,
                    parentId: 1773,
                    name: 'Managing SLAs',
                    local: 'hcs_gaussdb_00052.html'
                  }
                ]
              },
              {
                id: 1774,
                parentId: 1657,
                name: 'Copies',
                local: 'hcs_gaussdb_00053.html',
                children: [
                  {
                    id: 1809,
                    parentId: 1774,
                    name: 'Viewing Huawei Cloud Stack GaussDB Copy Information',
                    local: 'hcs_gaussdb_00054.html'
                  },
                  {
                    id: 1810,
                    parentId: 1774,
                    name: 'Managing Huawei Cloud Stack GaussDB Copies',
                    local: 'hcs_gaussdb_00055.html'
                  }
                ]
              },
              {
                id: 1775,
                parentId: 1657,
                name: 'Huawei Cloud Stack GaussDB',
                local: 'hcs_gaussdb_00056.html',
                children: [
                  {
                    id: 1811,
                    parentId: 1775,
                    name: 'Viewing Huawei Cloud Stack GaussDB Information',
                    local: 'hcs_gaussdb_00057.html'
                  },
                  {
                    id: 1812,
                    parentId: 1775,
                    name: 'Managing Huawei Cloud Stack GaussDB Projects',
                    local: 'hcs_gaussdb_00058.html'
                  },
                  {
                    id: 1813,
                    parentId: 1775,
                    name: 'Managing Instances',
                    local: 'hcs_gaussdb_00059.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1658,
            parentId: 17,
            name:
              'Alibaba Cloud Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001826879812.html',
            children: [
              {
                id: 1814,
                parentId: 1658,
                name: 'Backup',
                local: 'acloud_00007.html',
                children: [
                  {
                    id: 1822,
                    parentId: 1814,
                    name: 'Preparations for Backup',
                    local: 'acloud_00010.html'
                  },
                  {
                    id: 1823,
                    parentId: 1814,
                    name: 'Backing Up Alibaba Cloud Servers',
                    local: 'acloud_00011.html',
                    children: [
                      {
                        id: 1824,
                        parentId: 1823,
                        name:
                          'Step 4: Registering the Alibaba Cloud Organization',
                        local: 'acloud_00016.html'
                      },
                      {
                        id: 1825,
                        parentId: 1823,
                        name: 'Step 5: Creating a Rate Limiting Policy',
                        local: 'acloud_00017.html'
                      },
                      {
                        id: 1826,
                        parentId: 1823,
                        name:
                          'Step 6: (Optional) Enabling Backup Link Encryption',
                        local: 'acloud_00018.html'
                      },
                      {
                        id: 1827,
                        parentId: 1823,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'acloud_00019.html'
                      },
                      {
                        id: 1828,
                        parentId: 1823,
                        name: 'Step 8: Performing Backup',
                        local: 'acloud_00020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1815,
                parentId: 1658,
                name: 'Replication',
                local: 'acloud_00023.html',
                children: [
                  {
                    id: 1829,
                    parentId: 1815,
                    name: 'Replicating Alibaba Cloud Copies',
                    local: 'acloud_00026.html',
                    children: [
                      {
                        id: 1830,
                        parentId: 1829,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'acloud_001281.html'
                      },
                      {
                        id: 1831,
                        parentId: 1829,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances)',
                        local: 'acloud_001282.html'
                      },
                      {
                        id: 1832,
                        parentId: 1829,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'acloud_00030.html'
                      },
                      {
                        id: 1833,
                        parentId: 1829,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'acloud_00031.html'
                      },
                      {
                        id: 1834,
                        parentId: 1829,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'acloud_00032.html'
                      },
                      {
                        id: 1835,
                        parentId: 1829,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'Open_Stack_000340_0.html'
                      },
                      {
                        id: 1836,
                        parentId: 1829,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'acloud_00034.html'
                      },
                      {
                        id: 1837,
                        parentId: 1829,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'acloud_00035.html'
                      },
                      {
                        id: 1838,
                        parentId: 1829,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'acloud_00036.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1816,
                parentId: 1658,
                name: 'Archiving',
                local: 'acloud_00037.html',
                children: [
                  {
                    id: 1839,
                    parentId: 1816,
                    name: 'Archiving Alibaba Cloud Backup Copies',
                    local: 'acloud_00040.html',
                    children: [
                      {
                        id: 1841,
                        parentId: 1839,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'acloud_00041.html',
                        children: [
                          {
                            id: 1843,
                            parentId: 1841,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'acloud_00042.html'
                          },
                          {
                            id: 1844,
                            parentId: 1841,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'acloud_00043.html'
                          }
                        ]
                      },
                      {
                        id: 1842,
                        parentId: 1839,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'acloud_00044.html'
                      }
                    ]
                  },
                  {
                    id: 1840,
                    parentId: 1816,
                    name: 'Archiving Alibaba Cloud Replication Copies',
                    local: 'acloud_00045.html',
                    children: [
                      {
                        id: 1845,
                        parentId: 1840,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'acloud_00046.html'
                      },
                      {
                        id: 1846,
                        parentId: 1840,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'acloud_00047.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1817,
                parentId: 1658,
                name: 'Restoration',
                local: 'acloud_00048.html',
                children: [
                  {
                    id: 1847,
                    parentId: 1817,
                    name: 'Restoring Cloud Servers',
                    local: 'acloud_00051.html'
                  },
                  {
                    id: 1848,
                    parentId: 1817,
                    name: 'Restoring Cloud Disks',
                    local: 'acloud_00052.html'
                  },
                  {
                    id: 1849,
                    parentId: 1817,
                    name: 'Restoring Files',
                    local: 'acloud_00053.html'
                  }
                ]
              },
              {
                id: 1818,
                parentId: 1658,
                name: 'Global Search',
                local: 'en-us_topic_0000002014804512.html',
                children: [
                  {
                    id: 1850,
                    parentId: 1818,
                    name: 'About Global Search',
                    local: 'en-us_topic_0000002014646280.html'
                  },
                  {
                    id: 1851,
                    parentId: 1818,
                    name: 'Global Search for Copies',
                    local: 'vmware_gud_0076_4.html'
                  },
                  {
                    id: 1852,
                    parentId: 1818,
                    name: 'Global Search for Resources',
                    local: 'acloud_00054.html'
                  },
                  {
                    id: 1853,
                    parentId: 1818,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002050884109.html'
                  }
                ]
              },
              {
                id: 1819,
                parentId: 1658,
                name: 'SLA ',
                local: 'acloud_00057.html',
                children: [
                  {
                    id: 1854,
                    parentId: 1819,
                    name: 'About SLA',
                    local: 'acloud_00058.html'
                  },
                  {
                    id: 1855,
                    parentId: 1819,
                    name: 'Viewing SLA Information',
                    local: 'acloud_00059.html'
                  },
                  {
                    id: 1856,
                    parentId: 1819,
                    name: 'Managing SLAs',
                    local: 'acloud_00060.html'
                  }
                ]
              },
              {
                id: 1820,
                parentId: 1658,
                name: 'Copies',
                local: 'acloud_00061.html',
                children: [
                  {
                    id: 1857,
                    parentId: 1820,
                    name: 'Viewing Alibaba Cloud Copy Information',
                    local: 'acloud_00062.html'
                  },
                  {
                    id: 1858,
                    parentId: 1820,
                    name: 'Managing Alibaba Cloud Copies',
                    local: 'acloud_00063.html'
                  }
                ]
              },
              {
                id: 1821,
                parentId: 1658,
                name: 'Alibaba Cloud Environment Information',
                local: 'acloud_00064.html',
                children: [
                  {
                    id: 1859,
                    parentId: 1821,
                    name: 'Viewing Alibaba Cloud Resource Information',
                    local: 'acloud_00065.html'
                  },
                  {
                    id: 1860,
                    parentId: 1821,
                    name: 'Managing the Alibaba Cloud Platform',
                    local: 'acloud_00066.html'
                  },
                  {
                    id: 1861,
                    parentId: 1821,
                    name:
                      'Managing Availability Zones, Resource Sets, or Cloud Servers',
                    local: 'acloud_00067.html'
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
        name: 'Applications',
        local: 'en-us_topic_0000001918470740.html',
        children: [
          {
            id: 1862,
            parentId: 18,
            name:
              'Exchange Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001826879820.html',
            children: [
              {
                id: 1864,
                parentId: 1862,
                name: 'Backup',
                local: 'exchange_gud_00009.html',
                children: [
                  {
                    id: 1872,
                    parentId: 1864,
                    name: 'Preparations for Backup',
                    local: 'exchange_gud_00014.html',
                    children: [
                      {
                        id: 1875,
                        parentId: 1872,
                        name:
                          'Starting the Exchange Information Storage Service',
                        local: 'exchange_gud_00015.html'
                      },
                      {
                        id: 1876,
                        parentId: 1872,
                        name: 'Checking the Exchange Writer Status',
                        local: 'exchange_gud_00016.html'
                      },
                      {
                        id: 1877,
                        parentId: 1872,
                        name: 'Checking the Exchange Database Status',
                        local: 'exchange_gud_00017.html'
                      },
                      {
                        id: 1878,
                        parentId: 1872,
                        name:
                          'Configuring a Database Backup and Restoration Account',
                        local: 'exchange_gud_00018.html'
                      },
                      {
                        id: 1879,
                        parentId: 1872,
                        name:
                          'Configuring a Mailbox Backup and Restoration Account',
                        local: 'exchange_gud_00019.html'
                      }
                    ]
                  },
                  {
                    id: 1873,
                    parentId: 1864,
                    name:
                      'Backing Up an Exchange Single-Node System, Availability Group, or Database',
                    local: 'exchange_gud_00020.html',
                    children: [
                      {
                        id: 1880,
                        parentId: 1873,
                        name:
                          'Step 1: Registering an Exchange Single-Node System or Availability Group',
                        local: 'exchange_gud_00021.html'
                      },
                      {
                        id: 1881,
                        parentId: 1873,
                        name: 'Step 2: Creating a Rate Limiting Policy',
                        local: 'exchange_gud_00022.html'
                      },
                      {
                        id: 1882,
                        parentId: 1873,
                        name: 'Step 3: Creating a Backup SLA',
                        local: 'exchange_gud_00023.html'
                      },
                      {
                        id: 1883,
                        parentId: 1873,
                        name: 'Step 4: Performing Backup',
                        local: 'exchange_gud_00024.html'
                      }
                    ]
                  },
                  {
                    id: 1874,
                    parentId: 1864,
                    name: 'Backing Up an Exchange Mailbox',
                    local: 'exchange_gud_00027.html',
                    children: [
                      {
                        id: 1884,
                        parentId: 1874,
                        name:
                          'Step 1: Registering an Exchange Single-Node System or Availability Group',
                        local: 'exchange_gud_00028.html'
                      },
                      {
                        id: 1885,
                        parentId: 1874,
                        name: 'Step 2: Creating a Rate Limiting Policy',
                        local: 'exchange_gud_00029.html'
                      },
                      {
                        id: 1886,
                        parentId: 1874,
                        name: 'Step 3: Creating a Backup SLA',
                        local: 'exchange_gud_00030.html'
                      },
                      {
                        id: 1887,
                        parentId: 1874,
                        name: 'Step 4: Performing Backup',
                        local: 'exchange_gud_00031.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1865,
                parentId: 1862,
                name: 'Replication',
                local: 'exchange_gud_00034.html',
                children: [
                  {
                    id: 1888,
                    parentId: 1865,
                    name: 'Replicating an Exchange Database Copy',
                    local: 'exchange_gud_00036.html',
                    children: [
                      {
                        id: 1889,
                        parentId: 1888,
                        name: 'Planning the Replication Network',
                        local: 'exchange_gud_00037.html'
                      },
                      {
                        id: 1890,
                        parentId: 1888,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'exchange_gud_000371.html'
                      },
                      {
                        id: 1891,
                        parentId: 1888,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances)',
                        local: 'exchange_gud_000372.html'
                      },
                      {
                        id: 1892,
                        parentId: 1888,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'exchange_gud_00039.html'
                      },
                      {
                        id: 1893,
                        parentId: 1888,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'exchange_gud_00040.html'
                      },
                      {
                        id: 1894,
                        parentId: 1888,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'exchange_gud_00041.html'
                      },
                      {
                        id: 1895,
                        parentId: 1888,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'exchange_00460.html'
                      },
                      {
                        id: 1896,
                        parentId: 1888,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'exchange_gud_00043.html'
                      },
                      {
                        id: 1897,
                        parentId: 1888,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'exchange_gud_00044.html'
                      },
                      {
                        id: 1898,
                        parentId: 1888,
                        name:
                          'Step 8: (Optional) Creating a Reverse Replication or Cascaded Replication SLA',
                        local: 'exchange_gud_00045.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1866,
                parentId: 1862,
                name: 'Archiving',
                local: 'exchange_gud_00046.html',
                children: [
                  {
                    id: 1899,
                    parentId: 1866,
                    name: 'Archiving Exchange Backup Copies',
                    local: 'exchange_gud_00049.html',
                    children: [
                      {
                        id: 1901,
                        parentId: 1899,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'exchange_gud_00050.html',
                        children: [
                          {
                            id: 1903,
                            parentId: 1901,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'exchange_gud_00051.html'
                          },
                          {
                            id: 1904,
                            parentId: 1901,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'exchange_gud_00052.html'
                          }
                        ]
                      },
                      {
                        id: 1902,
                        parentId: 1899,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'exchange_gud_00053.html'
                      }
                    ]
                  },
                  {
                    id: 1900,
                    parentId: 1866,
                    name: 'Archiving Exchange Replication Copies',
                    local: 'exchange_gud_00054.html',
                    children: [
                      {
                        id: 1905,
                        parentId: 1900,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'exchange_gud_00055.html'
                      },
                      {
                        id: 1906,
                        parentId: 1900,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'exchange_gud_00056.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1867,
                parentId: 1862,
                name: 'Recovery',
                local: 'exchange_gud_00057.html',
                children: [
                  {
                    id: 1907,
                    parentId: 1867,
                    name:
                      'Restoring a Single-Node System or an Availability Group',
                    local: 'exchange_0067.html'
                  },
                  {
                    id: 1908,
                    parentId: 1867,
                    name: 'Restoring an Exchange Database',
                    local: 'exchange_gud_00063.html'
                  },
                  {
                    id: 1909,
                    parentId: 1867,
                    name: 'Restoring a Mailbox',
                    local: 'exchange_gud_00064.html'
                  },
                  {
                    id: 1910,
                    parentId: 1867,
                    name: 'Email-Level Restoration',
                    local: 'exchange_gud_00065.html'
                  },
                  {
                    id: 1911,
                    parentId: 1867,
                    name:
                      'Verifying the Restored Mailbox Data (Applicable to Microsoft Exchange Server 2010)',
                    local: 'exchange_gud_000651.html'
                  },
                  {
                    id: 1912,
                    parentId: 1867,
                    name:
                      'Verifying the Restored Mailbox Data (Applicable to Microsoft Exchange Server 2013 and Later Versions)',
                    local: 'exchange_gud_000652.html'
                  }
                ]
              },
              {
                id: 1868,
                parentId: 1862,
                name: 'Global Search',
                local: 'exchange_00611.html',
                children: [
                  {
                    id: 1913,
                    parentId: 1868,
                    name: 'Global Search for Resources',
                    local: 'exchange_gud_00066.html'
                  },
                  {
                    id: 1914,
                    parentId: 1868,
                    name: 'Tag-based Global Search',
                    local: 'exchange_00711.html'
                  }
                ]
              },
              {
                id: 1869,
                parentId: 1862,
                name: 'SLA',
                local: 'exchange_gud_00069.html',
                children: [
                  {
                    id: 1915,
                    parentId: 1869,
                    name: 'About SLA',
                    local: 'exchange_gud_00070.html'
                  },
                  {
                    id: 1916,
                    parentId: 1869,
                    name: 'Viewing SLA Information',
                    local: 'exchange_gud_00071.html'
                  },
                  {
                    id: 1917,
                    parentId: 1869,
                    name: 'Managing SLAs',
                    local: 'exchange_gud_00072.html'
                  }
                ]
              },
              {
                id: 1870,
                parentId: 1862,
                name: 'Copies',
                local: 'exchange_gud_00073.html',
                children: [
                  {
                    id: 1918,
                    parentId: 1870,
                    name: 'Viewing Exchange Copy Information',
                    local: 'exchange_gud_00074.html'
                  },
                  {
                    id: 1919,
                    parentId: 1870,
                    name: 'Managing Exchange Copies',
                    local: 'exchange_gud_00075.html'
                  }
                ]
              },
              {
                id: 1871,
                parentId: 1862,
                name: 'Managing Exchange',
                local: 'exchange_gud_00076.html',
                children: [
                  {
                    id: 1920,
                    parentId: 1871,
                    name: 'Viewing Exchange Environment Information',
                    local: 'exchange_gud_00077.html'
                  },
                  {
                    id: 1921,
                    parentId: 1871,
                    name:
                      'Managing Exchange Single-Node Systems or Availability Groups',
                    local: 'exchange_gud_00080.html'
                  },
                  {
                    id: 1922,
                    parentId: 1871,
                    name: 'Managing Databases or Mailboxes',
                    local: 'exchange_gud_00081.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1863,
            parentId: 18,
            name:
              'Active Directory Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001826879868.html',
            children: [
              {
                id: 1923,
                parentId: 1863,
                name: 'Backup',
                local: 'activedirectory_00007.html',
                children: [
                  {
                    id: 1931,
                    parentId: 1923,
                    name: 'Backing Up Active Directory',
                    local: 'activedirectory_00010.html',
                    children: [
                      {
                        id: 1932,
                        parentId: 1931,
                        name:
                          'Step 1: Enabling the Active Directory Recycle Bin',
                        local: 'en-us_topic_0000002020257438.html'
                      },
                      {
                        id: 1933,
                        parentId: 1931,
                        name:
                          'Step 2: Registering an Active Directory Domain Controller',
                        local: 'activedirectory_00011.html'
                      },
                      {
                        id: 1934,
                        parentId: 1931,
                        name: 'Step 3: Creating a Rate Limiting Policy',
                        local: 'activedirectory_00012.html'
                      },
                      {
                        id: 1935,
                        parentId: 1931,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'activedirectory_00013.html'
                      },
                      {
                        id: 1936,
                        parentId: 1931,
                        name: 'Step 5: Performing Backup',
                        local: 'activedirectory_00014.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1924,
                parentId: 1863,
                name: 'Replication',
                local: 'activedirectory_00017.html',
                children: [
                  {
                    id: 1937,
                    parentId: 1924,
                    name: 'Replicating an Active Directory Copy',
                    local: 'activedirectory_00020.html',
                    children: [
                      {
                        id: 1938,
                        parentId: 1937,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'ActiveDirectory-000241.html'
                      },
                      {
                        id: 1939,
                        parentId: 1937,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances)',
                        local: 'ActiveDirectory-000242.html'
                      },
                      {
                        id: 1940,
                        parentId: 1937,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'activedirectory_00023.html'
                      },
                      {
                        id: 1941,
                        parentId: 1937,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'activedirectory_00024.html'
                      },
                      {
                        id: 1942,
                        parentId: 1937,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'activedirectory_00025.html'
                      },
                      {
                        id: 1943,
                        parentId: 1937,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'ActiveDirectory-000290.html'
                      },
                      {
                        id: 1944,
                        parentId: 1937,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'activedirectory_00027.html'
                      },
                      {
                        id: 1945,
                        parentId: 1937,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'activedirectory_00028.html'
                      },
                      {
                        id: 1946,
                        parentId: 1937,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'activedirectory_00029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1925,
                parentId: 1863,
                name: 'Archiving',
                local: 'activedirectory_00030.html',
                children: [
                  {
                    id: 1947,
                    parentId: 1925,
                    name: 'Archiving Active Directory Backup Copies',
                    local: 'activedirectory_00033.html',
                    children: [
                      {
                        id: 1948,
                        parentId: 1947,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'activedirectory_00034.html',
                        children: [
                          {
                            id: 1950,
                            parentId: 1948,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'activedirectory_00035.html'
                          },
                          {
                            id: 1951,
                            parentId: 1948,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'activedirectory_00036.html'
                          }
                        ]
                      },
                      {
                        id: 1949,
                        parentId: 1947,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'activedirectory_00037.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1926,
                parentId: 1863,
                name: 'Restoration',
                local: 'activedirectory_00041.html',
                children: [
                  {
                    id: 1952,
                    parentId: 1926,
                    name:
                      'Restoring the System State of Active Directory in the Scenario with a Single Domain Controller',
                    local: 'activedirectory_00044.html'
                  },
                  {
                    id: 1953,
                    parentId: 1926,
                    name:
                      'Restoring Active Directory Objects in the Scenario with a Single Domain Controller',
                    local: 'activedirectory_00045.html'
                  },
                  {
                    id: 1954,
                    parentId: 1926,
                    name:
                      'Restoring the System State of Active Directory in the Scenario with Primary and Secondary Domain Controllers',
                    local: 'en-us_topic_0000002080463105.html'
                  },
                  {
                    id: 1955,
                    parentId: 1926,
                    name:
                      'Restoring Active Directory Objects in the Scenario with Primary and Secondary Domain Controllers',
                    local: 'en-us_topic_0000002044462372.html'
                  }
                ]
              },
              {
                id: 1927,
                parentId: 1863,
                name: 'Global Search',
                local: 'activedirectory_00046.html',
                children: [
                  {
                    id: 1956,
                    parentId: 1927,
                    name: 'Global Search for Resources',
                    local: 'activedirectory_00047.html'
                  },
                  {
                    id: 1957,
                    parentId: 1927,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'ActiveDirectory-000491.html'
                  }
                ]
              },
              {
                id: 1928,
                parentId: 1863,
                name: 'SLA ',
                local: 'activedirectory_00050.html',
                children: [
                  {
                    id: 1958,
                    parentId: 1928,
                    name: 'About SLA ',
                    local: 'activedirectory_000501.html'
                  },
                  {
                    id: 1959,
                    parentId: 1928,
                    name: 'Viewing SLA Information',
                    local: 'activedirectory_000502.html'
                  },
                  {
                    id: 1960,
                    parentId: 1928,
                    name: 'Managing SLAs',
                    local: 'activedirectory_00053.html'
                  }
                ]
              },
              {
                id: 1929,
                parentId: 1863,
                name: 'Copies',
                local: 'activedirectory_00054.html',
                children: [
                  {
                    id: 1961,
                    parentId: 1929,
                    name: 'Viewing Active Directory Copy Information',
                    local: 'activedirectory_00055.html'
                  },
                  {
                    id: 1962,
                    parentId: 1929,
                    name: 'Managing Active Directory Copies',
                    local: 'activedirectory_00056.html'
                  }
                ]
              },
              {
                id: 1930,
                parentId: 1863,
                name: 'Active Directory Domain Controller',
                local: 'activedirectory_00057.html',
                children: [
                  {
                    id: 1963,
                    parentId: 1930,
                    name:
                      'Viewing Information of Active Directory Domain Controllers',
                    local: 'activedirectory_00058.html'
                  },
                  {
                    id: 1964,
                    parentId: 1930,
                    name: 'Managing Active Directory Domain Controllers',
                    local: 'activedirectory_00059.html'
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
        name: 'File Systems',
        local: 'en-us_topic_0000001918630672.html',
        children: [
          {
            id: 1965,
            parentId: 19,
            name: 'NAS File Data Protection',
            local: 'product_documentation_000035.html',
            children: [
              {
                id: 1971,
                parentId: 1965,
                name: 'Backup',
                local: 'nas_s_0007.html',
                children: [
                  {
                    id: 1982,
                    parentId: 1971,
                    name: 'Preparations for Backup',
                    local: 'nas_s_0010.html'
                  },
                  {
                    id: 1983,
                    parentId: 1971,
                    name: 'Backing Up a NAS File System',
                    local: 'nas_s_0011.html',
                    children: [
                      {
                        id: 1985,
                        parentId: 1983,
                        name:
                          'Step 1: Obtaining the CA Certificate of a Storage Device',
                        local: 'nas_s_0012.html'
                      },
                      {
                        id: 1986,
                        parentId: 1983,
                        name: 'Step 2: Adding a Storage Device',
                        local: 'nas_s_0013.html'
                      },
                      {
                        id: 1987,
                        parentId: 1983,
                        name:
                          'Step 3: Creating a Logical Port for the Replication Network',
                        local: 'nas_s_0014.html'
                      },
                      {
                        id: 1988,
                        parentId: 1983,
                        name:
                          '(Optional) Step 4: Creating a Rate Limiting Policy',
                        local: 'nas_s_0015.html'
                      },
                      {
                        id: 1989,
                        parentId: 1983,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'nas_s_0016.html'
                      },
                      {
                        id: 1990,
                        parentId: 1983,
                        name: 'Step 6: Performing Backup',
                        local: 'nas_s_0017.html'
                      }
                    ]
                  },
                  {
                    id: 1984,
                    parentId: 1971,
                    name: 'Backing Up a NAS Share',
                    local: 'nas_s_0020.html',
                    children: [
                      {
                        id: 1991,
                        parentId: 1984,
                        name:
                          'Step 1: (Optional) Obtaining the CA Certificate of a Storage Device (Applicable to OceanStor V5/OceanStor Pacific/NetApp ONTAP)',
                        local: 'nas_s_0021.html'
                      },
                      {
                        id: 1992,
                        parentId: 1984,
                        name:
                          'Step 2: Adding a Storage Device (Applicable to OceanStor V5/OceanStor Pacific/NetApp ONTAP)',
                        local: 'nas_s_0022.html'
                      },
                      {
                        id: 1993,
                        parentId: 1984,
                        name:
                          'Step 3: Configuring NAS Share Information (Applicable to OceanStor V5/OceanStor Pacific/NetApp ONTAP)',
                        local: 'nas_s_0023.html'
                      },
                      {
                        id: 1994,
                        parentId: 1984,
                        name:
                          'Step 4: Configuring Access Permissions (Applicable to OceanStor V5/OceanStor Pacific/NetApp ONTAP)',
                        local: 'nas_s_0025.html'
                      },
                      {
                        id: 1995,
                        parentId: 1984,
                        name:
                          'Step 5: Registering a NAS Share (Applicable to Storage Devices Except OceanStor V5/OceanStor Pacific/NetApp ONTAP)',
                        local: 'nas_s_0024.html'
                      },
                      {
                        id: 1996,
                        parentId: 1984,
                        name:
                          '(Optional) Step 6: Creating a Rate Limiting Policy',
                        local: 'nas_s_0026.html'
                      },
                      {
                        id: 1997,
                        parentId: 1984,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'nas_s_0027.html'
                      },
                      {
                        id: 1998,
                        parentId: 1984,
                        name:
                          'Step 8: Enabling the NFSv4.1 Service (Applicable to X Series Backup Appliances)',
                        local: 'nas_s_0028.html'
                      },
                      {
                        id: 1999,
                        parentId: 1984,
                        name:
                          'Step 9: Enabling the NFSv4.1 Service (Applicable to E Series Backup Appliances)',
                        local: 'nas_s_0028_2.html'
                      },
                      {
                        id: 2000,
                        parentId: 1984,
                        name: 'Step 10: Performing Backup',
                        local: 'nas_s_0029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1972,
                parentId: 1965,
                name: 'Replication',
                local: 'nas_s_0032.html',
                children: [
                  {
                    id: 2001,
                    parentId: 1972,
                    name: 'Replicating NAS File System or NAS Share Copies',
                    local: 'nas_s_0035.html',
                    children: [
                      {
                        id: 2002,
                        parentId: 2001,
                        name: 'Planning the Replication Network',
                        local: 'nas_s_0036.html'
                      },
                      {
                        id: 2003,
                        parentId: 2001,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'nas_s_0037.html'
                      },
                      {
                        id: 2004,
                        parentId: 2001,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'nas_s_0037_1.html'
                      },
                      {
                        id: 2005,
                        parentId: 2001,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'nas_s_0038.html'
                      },
                      {
                        id: 2006,
                        parentId: 2001,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'nas_s_0039.html'
                      },
                      {
                        id: 2007,
                        parentId: 2001,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'nas_s_0040.html'
                      },
                      {
                        id: 2008,
                        parentId: 2001,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'nas_s_0041.html'
                      },
                      {
                        id: 2009,
                        parentId: 2001,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002022805444.html'
                      },
                      {
                        id: 2010,
                        parentId: 2001,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'nas_s_0042.html'
                      },
                      {
                        id: 2011,
                        parentId: 2001,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'nas_s_0043.html'
                      },
                      {
                        id: 2012,
                        parentId: 2001,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'nas_s_0044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1973,
                parentId: 1965,
                name: 'Archiving',
                local: 'nas_s_0045.html',
                children: [
                  {
                    id: 2013,
                    parentId: 1973,
                    name:
                      'Archiving Backup Copies of a NAS File System or NAS Share',
                    local: 'nas_s_0048.html',
                    children: [
                      {
                        id: 2015,
                        parentId: 2013,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'nas_s_0049.html',
                        children: [
                          {
                            id: 2017,
                            parentId: 2015,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'nas_s_0050.html'
                          },
                          {
                            id: 2018,
                            parentId: 2015,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'nas_s_0051.html'
                          }
                        ]
                      },
                      {
                        id: 2016,
                        parentId: 2013,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'nas_s_0052.html'
                      }
                    ]
                  },
                  {
                    id: 2014,
                    parentId: 1973,
                    name:
                      'Archiving Replication Copies of a NAS File System or NAS Share',
                    local: 'nas_s_0053.html',
                    children: [
                      {
                        id: 2019,
                        parentId: 2014,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'nas_s_0054.html'
                      },
                      {
                        id: 2020,
                        parentId: 2014,
                        name:
                          'Step 2: Creating Periodic Archiving for Replication Copies',
                        local: 'nas_s_0055.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1974,
                parentId: 1965,
                name: 'Restoration',
                local: 'nas_s_0056.html',
                children: [
                  {
                    id: 2021,
                    parentId: 1974,
                    name: 'Restoring a NAS File System',
                    local: 'nas_s_0059.html'
                  },
                  {
                    id: 2022,
                    parentId: 1974,
                    name: 'Restoring Files in a NAS File System',
                    local: 'nas_s_0060.html'
                  },
                  {
                    id: 2023,
                    parentId: 1974,
                    name: 'Restoring a NAS Share',
                    local: 'nas_s_0061.html'
                  },
                  {
                    id: 2024,
                    parentId: 1974,
                    name: 'Restoring Files in a NAS Share',
                    local: 'nas_s_0062.html'
                  }
                ]
              },
              {
                id: 1975,
                parentId: 1965,
                name: 'Global Search',
                local: 'nas_s_0073.html',
                children: [
                  {
                    id: 2025,
                    parentId: 1975,
                    name: 'About Global Search',
                    local: 'fc_gud_gs2.html'
                  },
                  {
                    id: 2026,
                    parentId: 1975,
                    name: 'Global Search for Copies',
                    local: 'nas_s_0074.html'
                  },
                  {
                    id: 2027,
                    parentId: 1975,
                    name: 'Global Search for Resources',
                    local: 'nas_s_0075.html'
                  },
                  {
                    id: 2028,
                    parentId: 1975,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002022936884.html'
                  }
                ]
              },
              {
                id: 1976,
                parentId: 1965,
                name: 'SLA ',
                local: 'nas_s_0078.html',
                children: [
                  {
                    id: 2029,
                    parentId: 1976,
                    name: 'About SLA',
                    local: 'nas_s_0079.html'
                  },
                  {
                    id: 2030,
                    parentId: 1976,
                    name: 'Viewing SLA Information',
                    local: 'nas_s_0080.html'
                  },
                  {
                    id: 2031,
                    parentId: 1976,
                    name: 'Managing SLAs',
                    local: 'nas_s_0081.html'
                  }
                ]
              },
              {
                id: 1977,
                parentId: 1965,
                name: 'Copies',
                local: 'nas_s_0082.html',
                children: [
                  {
                    id: 2032,
                    parentId: 1977,
                    name: 'Viewing Copy Information About a NAS File System',
                    local: 'nas_s_0083.html'
                  },
                  {
                    id: 2033,
                    parentId: 1977,
                    name: 'Managing Copies of a NAS File System',
                    local: 'nas_s_0084.html'
                  },
                  {
                    id: 2034,
                    parentId: 1977,
                    name: 'Viewing NAS Share Copy Information',
                    local: 'nas_s_0086.html'
                  },
                  {
                    id: 2035,
                    parentId: 1977,
                    name: 'Managing NAS Share Copies',
                    local: 'nas_s_0087.html'
                  }
                ]
              },
              {
                id: 1978,
                parentId: 1965,
                name: 'Storage Device Information',
                local: 'nas_s_0088.html',
                children: [
                  {
                    id: 2036,
                    parentId: 1978,
                    name: 'Viewing Storage Device Information',
                    local: 'nas_s_0089.html'
                  },
                  {
                    id: 2037,
                    parentId: 1978,
                    name: 'Managing Storage Device Information',
                    local: 'nas_s_0090.html'
                  }
                ]
              },
              {
                id: 1979,
                parentId: 1965,
                name: 'NAS File Systems',
                local: 'nas_s_0091.html',
                children: [
                  {
                    id: 2038,
                    parentId: 1979,
                    name: 'Viewing NAS File Systems',
                    local: 'nas_s_0092.html'
                  },
                  {
                    id: 2039,
                    parentId: 1979,
                    name: 'Managing NAS File Systems',
                    local: 'nas_s_0093.html'
                  }
                ]
              },
              {
                id: 1980,
                parentId: 1965,
                name: 'NAS Shares',
                local: 'nas_s_0094.html',
                children: [
                  {
                    id: 2040,
                    parentId: 1980,
                    name: 'Viewing NAS Share Information',
                    local: 'nas_s_0095.html'
                  },
                  {
                    id: 2041,
                    parentId: 1980,
                    name: 'Managing NAS Shares',
                    local: 'nas_s_0096.html'
                  }
                ]
              },
              {
                id: 1981,
                parentId: 1965,
                name: 'FAQs',
                local: 'nas_s_0097.html',
                children: [
                  {
                    id: 2042,
                    parentId: 1981,
                    name: 'Logging In to DeviceManager',
                    local: 'nas_s_dm.html'
                  },
                  {
                    id: 2043,
                    parentId: 1981,
                    name: 'Modifying Target Deduplication Settings',
                    local: 'en-us_topic_0000002062497610.html'
                  },
                  {
                    id: 2044,
                    parentId: 1981,
                    name: 'Viewing User Information',
                    local: 'en-us_topic_0000002064268578.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1966,
            parentId: 19,
            name:
              'NDMP NAS File System Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001952305809.html',
            children: [
              {
                id: 2045,
                parentId: 1966,
                name: 'Backup',
                local: 'nas_s_0007_0.html',
                children: [
                  {
                    id: 2054,
                    parentId: 2045,
                    name: 'Preparations for Backup',
                    local: 'nas_s_0010_0.html'
                  },
                  {
                    id: 2055,
                    parentId: 2045,
                    name: 'Backing Up an NDMP NAS File System',
                    local: 'nas_s_0011_0.html',
                    children: [
                      {
                        id: 2056,
                        parentId: 2055,
                        name: 'Step 1: Adding a Storage Device',
                        local: 'nas_s_0013_0.html'
                      },
                      {
                        id: 2057,
                        parentId: 2055,
                        name: 'Step 2: (Optional) Creating a File Directory',
                        local: 'nas_s_0212.html'
                      },
                      {
                        id: 2058,
                        parentId: 2055,
                        name:
                          'Step 3: (Optional) Creating a Rate Limiting Policy',
                        local: 'nas_s_0015_0.html'
                      },
                      {
                        id: 2059,
                        parentId: 2055,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'nas_s_0016_0.html'
                      },
                      {
                        id: 2060,
                        parentId: 2055,
                        name: 'Step 5: Performing Backup',
                        local: 'nas_s_0017_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2046,
                parentId: 1966,
                name: 'Replication',
                local: 'nas_s_0032_0.html',
                children: [
                  {
                    id: 2061,
                    parentId: 2046,
                    name: 'Replicating an NDMP NAS File System',
                    local: 'nas_s_0035_0.html',
                    children: [
                      {
                        id: 2062,
                        parentId: 2061,
                        name: 'Planning the Replication Network',
                        local: 'nas_s_0036_0.html'
                      },
                      {
                        id: 2063,
                        parentId: 2061,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'oracle_gud_000037_0.html'
                      },
                      {
                        id: 2064,
                        parentId: 2061,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances)',
                        local: 'en-us_topic_0000002096251332.html'
                      },
                      {
                        id: 2065,
                        parentId: 2061,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'nas_s_0038_0.html'
                      },
                      {
                        id: 2066,
                        parentId: 2061,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'nas_s_0039_0.html'
                      },
                      {
                        id: 2067,
                        parentId: 2061,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'nas_s_0040_0.html'
                      },
                      {
                        id: 2068,
                        parentId: 2061,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'nas_s_0041_0.html'
                      },
                      {
                        id: 2069,
                        parentId: 2061,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'nas_s_0042_0.html'
                      },
                      {
                        id: 2070,
                        parentId: 2061,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'nas_s_0043_0.html'
                      },
                      {
                        id: 2071,
                        parentId: 2061,
                        name:
                          'Step 8: (Optional) Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'nas_s_0044_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2047,
                parentId: 1966,
                name: 'Archiving',
                local: 'nas_s_0045_0.html',
                children: [
                  {
                    id: 2072,
                    parentId: 2047,
                    name: 'Archiving Copies of an NDMP NAS File System',
                    local: 'nas_s_0048_0.html',
                    children: [
                      {
                        id: 2074,
                        parentId: 2072,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'nas_s_0049_0.html',
                        children: [
                          {
                            id: 2076,
                            parentId: 2074,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'nas_s_0050_0.html'
                          },
                          {
                            id: 2077,
                            parentId: 2074,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'nas_s_0051_0.html'
                          }
                        ]
                      },
                      {
                        id: 2075,
                        parentId: 2072,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'nas_s_0052_0.html'
                      }
                    ]
                  },
                  {
                    id: 2073,
                    parentId: 2047,
                    name:
                      'Archiving Replication Copies of an NDMP NAS File System',
                    local: 'nas_s_0053_0.html',
                    children: [
                      {
                        id: 2078,
                        parentId: 2073,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'nas_s_0054_0.html'
                      },
                      {
                        id: 2079,
                        parentId: 2073,
                        name:
                          'Step 2: Creating Periodic Archiving for Replication Copies',
                        local: 'nas_s_0055_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2048,
                parentId: 1966,
                name: 'Restoration',
                local: 'nas_s_0056_0.html',
                children: [
                  {
                    id: 2080,
                    parentId: 2048,
                    name: 'Restoring an NDMP NAS File System',
                    local: 'nas_s_0059_0.html'
                  },
                  {
                    id: 2081,
                    parentId: 2048,
                    name: 'Restoring Files in an NDMP NAS File System',
                    local: 'nas_s_0060_0.html'
                  }
                ]
              },
              {
                id: 2049,
                parentId: 1966,
                name: 'Global Search',
                local: 'nas_s_0073_0.html',
                children: [
                  {
                    id: 2082,
                    parentId: 2049,
                    name: 'Global Search for Copies',
                    local: 'nas_s_0074_0.html'
                  },
                  {
                    id: 2083,
                    parentId: 2049,
                    name: 'Global Search for Resources',
                    local: 'nas_s_0075_0.html'
                  },
                  {
                    id: 2084,
                    parentId: 2049,
                    name: 'Tag-based Global Search',
                    local: 'nas_s_0275.html'
                  }
                ]
              },
              {
                id: 2050,
                parentId: 1966,
                name: 'SLA ',
                local: 'nas_s_0078_0.html',
                children: [
                  {
                    id: 2085,
                    parentId: 2050,
                    name: 'About SLA',
                    local: 'nas_s_0079_0.html'
                  },
                  {
                    id: 2086,
                    parentId: 2050,
                    name: 'Viewing SLA Information',
                    local: 'nas_s_0080_0.html'
                  },
                  {
                    id: 2087,
                    parentId: 2050,
                    name: 'Managing SLAs',
                    local: 'nas_s_0081_0.html'
                  }
                ]
              },
              {
                id: 2051,
                parentId: 1966,
                name: 'Copies',
                local: 'nas_s_0082_0.html',
                children: [
                  {
                    id: 2088,
                    parentId: 2051,
                    name:
                      'Viewing Copy Information About an NDMP NAS File System',
                    local: 'nas_s_0083_0.html'
                  },
                  {
                    id: 2089,
                    parentId: 2051,
                    name: 'Managing Copies of an NDMP NAS File System',
                    local: 'nas_s_0084_0.html'
                  }
                ]
              },
              {
                id: 2052,
                parentId: 1966,
                name: 'Storage Device Information',
                local: 'nas_s_0088_0.html',
                children: [
                  {
                    id: 2090,
                    parentId: 2052,
                    name: 'Viewing Storage Device Information',
                    local: 'nas_s_0089_0.html'
                  },
                  {
                    id: 2091,
                    parentId: 2052,
                    name: 'Managing Storage Device Information',
                    local: 'nas_s_0090_0.html'
                  }
                ]
              },
              {
                id: 2053,
                parentId: 1966,
                name: 'NDMP NAS File Systems',
                local: 'nas_s_0091_0.html',
                children: [
                  {
                    id: 2092,
                    parentId: 2053,
                    name: 'Viewing NDMP NAS File Systems',
                    local: 'nas_s_0092_0.html'
                  },
                  {
                    id: 2093,
                    parentId: 2053,
                    name: 'Managing an NDMP NAS File System',
                    local: 'nas_s_0093_0.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1967,
            parentId: 19,
            name: 'Fileset Data Protection',
            local: 'en-us_topic_0000001873679157.html',
            children: [
              {
                id: 2094,
                parentId: 1967,
                name: 'Backup',
                local: 'Files-0005_0.html',
                children: [
                  {
                    id: 2102,
                    parentId: 2094,
                    name: 'Mounting Object Storage to the ProtectAgent Host',
                    local: 'object-0006_0.html'
                  },
                  {
                    id: 2103,
                    parentId: 2094,
                    name: 'Backing up a Fileset',
                    local: 'Files-0008_0.html',
                    children: [
                      {
                        id: 2104,
                        parentId: 2103,
                        name: 'Step 1: (Optional) Creating a Fileset Template',
                        local: 'Files-0009_0.html'
                      },
                      {
                        id: 2105,
                        parentId: 2103,
                        name: 'Step 2: Creating a Fileset',
                        local: 'Files-0010.html'
                      },
                      {
                        id: 2106,
                        parentId: 2103,
                        name:
                          'Step 3: (Optional) Creating a Rate Limiting Policy',
                        local: 'Files-0011_0.html'
                      },
                      {
                        id: 2107,
                        parentId: 2103,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'Files-0012.html'
                      },
                      {
                        id: 2108,
                        parentId: 2103,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'Files-0013_0.html'
                      },
                      {
                        id: 2109,
                        parentId: 2103,
                        name: 'Step 6: Performing Backup',
                        local: 'Files-0014_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2095,
                parentId: 1967,
                name: 'Replication',
                local: 'oracle_gud_000035_14.html',
                children: [
                  {
                    id: 2110,
                    parentId: 2095,
                    name: 'Replicating Fileset Copies',
                    local: 'Files-0020_0.html',
                    children: [
                      {
                        id: 2111,
                        parentId: 2110,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.5.0) and OceanProtect E1000 (with the OceanProtect Used as Backup Storage))',
                        local: 'fc_gud_0026_11.html'
                      },
                      {
                        id: 2112,
                        parentId: 2110,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_18.html'
                      },
                      {
                        id: 2113,
                        parentId: 2110,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'Files-0023_0.html'
                      },
                      {
                        id: 2114,
                        parentId: 2110,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'Files-0023_1.html'
                      },
                      {
                        id: 2115,
                        parentId: 2110,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'Files-0025.html'
                      },
                      {
                        id: 2116,
                        parentId: 2110,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'Files-0026_0.html'
                      },
                      {
                        id: 2117,
                        parentId: 2110,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002010555368.html'
                      },
                      {
                        id: 2118,
                        parentId: 2110,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'oracle_gud_000040.html'
                      },
                      {
                        id: 2119,
                        parentId: 2110,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'Files-0028_0.html'
                      },
                      {
                        id: 2120,
                        parentId: 2110,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA ',
                        local: 'Files-0029_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2096,
                parentId: 1967,
                name: 'Archiving',
                local: 'Files-0030.html',
                children: [
                  {
                    id: 2121,
                    parentId: 2096,
                    name: 'Archiving Fileset Backup Copies',
                    local: 'Files-0033_0.html',
                    children: [
                      {
                        id: 2123,
                        parentId: 2121,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'Files-0034_0.html',
                        children: [
                          {
                            id: 2125,
                            parentId: 2123,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'Files-0035_0.html'
                          },
                          {
                            id: 2126,
                            parentId: 2123,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'Files-0036_0.html'
                          }
                        ]
                      },
                      {
                        id: 2124,
                        parentId: 2121,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'Files-0037_0.html'
                      }
                    ]
                  },
                  {
                    id: 2122,
                    parentId: 2096,
                    name: 'Archiving Fileset Replication Copies ',
                    local: 'Files-0038_0.html',
                    children: [
                      {
                        id: 2127,
                        parentId: 2122,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'Files-0039_0.html'
                      },
                      {
                        id: 2128,
                        parentId: 2122,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'Files-0040_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2097,
                parentId: 1967,
                name: 'Restoration',
                local: 'Files-0041_0.html',
                children: [
                  {
                    id: 2129,
                    parentId: 2097,
                    name: 'Restoring a Fileset',
                    local: 'Files-0044.html'
                  },
                  {
                    id: 2130,
                    parentId: 2097,
                    name: 'Restoring One or Multiple Files in a Fileset',
                    local: 'Files-0045.html'
                  }
                ]
              },
              {
                id: 2098,
                parentId: 1967,
                name: 'Global Search',
                local: 'Files-0056_0.html',
                children: [
                  {
                    id: 2131,
                    parentId: 2098,
                    name: 'Global Search for Copies',
                    local: 'Files-0057_0.html'
                  },
                  {
                    id: 2132,
                    parentId: 2098,
                    name: 'Global Search for Resources',
                    local: 'Files-0058_0.html'
                  },
                  {
                    id: 2133,
                    parentId: 2098,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002699236.html'
                  }
                ]
              },
              {
                id: 2099,
                parentId: 1967,
                name: 'SLA ',
                local: 'Files-0061.html',
                children: [
                  {
                    id: 2134,
                    parentId: 2099,
                    name: 'Viewing SLA Information',
                    local: 'Files-0063_0.html'
                  },
                  {
                    id: 2135,
                    parentId: 2099,
                    name: 'Managing SLAs',
                    local: 'Files-0064_0.html'
                  }
                ]
              },
              {
                id: 2100,
                parentId: 1967,
                name: 'Copies',
                local: 'Files-0065_0.html',
                children: [
                  {
                    id: 2136,
                    parentId: 2100,
                    name: 'Viewing Fileset Copy Information',
                    local: 'Files-0066_0.html'
                  },
                  {
                    id: 2137,
                    parentId: 2100,
                    name: 'Managing Fileset Copies',
                    local: 'Files-0067_0.html'
                  }
                ]
              },
              {
                id: 2101,
                parentId: 1967,
                name: 'Filesets',
                local: 'Files-0068_0.html',
                children: [
                  {
                    id: 2138,
                    parentId: 2101,
                    name: 'Viewing Fileset Information',
                    local: 'Files-0069.html'
                  },
                  {
                    id: 2139,
                    parentId: 2101,
                    name: 'Managing Filesets',
                    local: 'Files-0070_0.html'
                  },
                  {
                    id: 2140,
                    parentId: 2101,
                    name: 'Managing Fileset Templates',
                    local: 'Files-0071_0.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1968,
            parentId: 19,
            name:
              'Volume Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001826879836.html',
            children: [
              {
                id: 2141,
                parentId: 1968,
                name: 'Backup',
                local: 'volume-0007.html',
                children: [
                  {
                    id: 2150,
                    parentId: 2141,
                    name: 'Backing Up Volumes',
                    local: 'volume-0010.html',
                    children: [
                      {
                        id: 2151,
                        parentId: 2150,
                        name: 'Step 1: Creating a Volume',
                        local: 'volume-0011.html'
                      },
                      {
                        id: 2152,
                        parentId: 2150,
                        name: 'Step 2: Creating a Rate Limiting Policy',
                        local: 'volume-0012.html'
                      },
                      {
                        id: 2153,
                        parentId: 2150,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'volume-0013.html'
                      },
                      {
                        id: 2154,
                        parentId: 2150,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'volume-0014.html'
                      },
                      {
                        id: 2155,
                        parentId: 2150,
                        name: 'Step 5: Performing Backup',
                        local: 'volume-0015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2142,
                parentId: 1968,
                name: 'Replication',
                local: 'oracle_gud_000035_12.html',
                children: [
                  {
                    id: 2156,
                    parentId: 2142,
                    name: 'Replicating Volume Copies',
                    local: 'volume-0021.html',
                    children: [
                      {
                        id: 2157,
                        parentId: 2156,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_16.html'
                      },
                      {
                        id: 2158,
                        parentId: 2156,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'volume-0024.html'
                      },
                      {
                        id: 2159,
                        parentId: 2156,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'volume-0025.html'
                      },
                      {
                        id: 2160,
                        parentId: 2156,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'volume-0026.html'
                      },
                      {
                        id: 2161,
                        parentId: 2156,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'en-us_topic_0000002010594166.html'
                      },
                      {
                        id: 2162,
                        parentId: 2156,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'volume-0028.html'
                      },
                      {
                        id: 2163,
                        parentId: 2156,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'volume-0029.html'
                      },
                      {
                        id: 2164,
                        parentId: 2156,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA ',
                        local: 'volume-0030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2143,
                parentId: 1968,
                name: 'Archiving',
                local: 'volume-0031.html',
                children: [
                  {
                    id: 2165,
                    parentId: 2143,
                    name: 'Archiving Volume Backup Copies',
                    local: 'volume-0034.html',
                    children: [
                      {
                        id: 2167,
                        parentId: 2165,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'volume-0035.html',
                        children: [
                          {
                            id: 2169,
                            parentId: 2167,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'volume-0036.html'
                          },
                          {
                            id: 2170,
                            parentId: 2167,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'volume-0037.html'
                          }
                        ]
                      },
                      {
                        id: 2168,
                        parentId: 2165,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'volume-0038.html'
                      }
                    ]
                  },
                  {
                    id: 2166,
                    parentId: 2143,
                    name: 'Archiving Volume Replication Copies',
                    local: 'volume-0039.html',
                    children: [
                      {
                        id: 2171,
                        parentId: 2166,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'volume-0040.html'
                      },
                      {
                        id: 2172,
                        parentId: 2166,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'volume-0041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2144,
                parentId: 1968,
                name: 'Restoration',
                local: 'volume-0042.html',
                children: [
                  {
                    id: 2173,
                    parentId: 2144,
                    name: 'Restoring Volumes',
                    local: 'volume-0045.html'
                  },
                  {
                    id: 2174,
                    parentId: 2144,
                    name: 'Restoring One or Multiple Files in a Volume Copy',
                    local: 'volume-0046.html'
                  }
                ]
              },
              {
                id: 2145,
                parentId: 1968,
                name: 'Live Mount',
                local: 'volume-0047.html',
                children: [
                  {
                    id: 2175,
                    parentId: 2145,
                    name: 'About Live Mount',
                    local: 'volume-0048.html'
                  },
                  {
                    id: 2176,
                    parentId: 2145,
                    name: 'Constraints',
                    local: 'volume-0049.html'
                  },
                  {
                    id: 2177,
                    parentId: 2145,
                    name: 'Live Mounting Volumes',
                    local: 'volume-0050.html'
                  },
                  {
                    id: 2178,
                    parentId: 2145,
                    name: 'Managing Live Mount of Volumes',
                    local: 'volume-0051.html',
                    children: [
                      {
                        id: 2179,
                        parentId: 2178,
                        name: 'Viewing Live Mount Information of a Volume',
                        local: 'volume-0052.html'
                      },
                      {
                        id: 2180,
                        parentId: 2178,
                        name: 'Managing Live Mount',
                        local: 'volume-0053.html'
                      },
                      {
                        id: 2181,
                        parentId: 2178,
                        name: 'Creating a Mount Update Policy ',
                        local: 'volume-0054.html'
                      },
                      {
                        id: 2182,
                        parentId: 2178,
                        name: 'Managing a Mount Update Policy ',
                        local: 'volume-0055.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2146,
                parentId: 1968,
                name: 'Global Search',
                local: 'volume-0056.html',
                children: [
                  {
                    id: 2183,
                    parentId: 2146,
                    name: 'About Global Search',
                    local: 'volume_0056.html'
                  },
                  {
                    id: 2184,
                    parentId: 2146,
                    name: 'Global Search for Copies',
                    local: 'volume-0057.html'
                  },
                  {
                    id: 2185,
                    parentId: 2146,
                    name: 'Global Search for Resources',
                    local: 'volume-0058.html'
                  },
                  {
                    id: 2186,
                    parentId: 2146,
                    name: 'Tag-based Global Search',
                    local: 'en-us_topic_0000002002855252.html'
                  }
                ]
              },
              {
                id: 2147,
                parentId: 1968,
                name: 'SLA ',
                local: 'volume-0061.html',
                children: [
                  {
                    id: 2187,
                    parentId: 2147,
                    name: 'About SLA',
                    local: 'vmware_gud_000026_1.html'
                  },
                  {
                    id: 2188,
                    parentId: 2147,
                    name: 'Viewing SLA Information',
                    local: 'volume-0063.html'
                  },
                  {
                    id: 2189,
                    parentId: 2147,
                    name: 'Managing SLAs',
                    local: 'volume-0064.html'
                  }
                ]
              },
              {
                id: 2148,
                parentId: 1968,
                name: 'Copies',
                local: 'volume-0065.html',
                children: [
                  {
                    id: 2190,
                    parentId: 2148,
                    name: 'Viewing Volume Copy Information',
                    local: 'volume-0066.html'
                  },
                  {
                    id: 2191,
                    parentId: 2148,
                    name: 'Managing Volume Copies',
                    local: 'volume-0067.html'
                  }
                ]
              },
              {
                id: 2149,
                parentId: 1968,
                name: 'Volumes',
                local: 'volume-0068.html',
                children: [
                  {
                    id: 2192,
                    parentId: 2149,
                    name: 'Viewing Volume Information',
                    local: 'volume-0069.html'
                  },
                  {
                    id: 2193,
                    parentId: 2149,
                    name: 'Managing Volumes',
                    local: 'volume-0070.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1969,
            parentId: 19,
            name:
              'Object Storage Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001826879816.html',
            children: [
              {
                id: 2194,
                parentId: 1969,
                name: 'Backup',
                local: 'object-0007.html',
                children: [
                  {
                    id: 2202,
                    parentId: 2194,
                    name: 'Preparing for the Backup',
                    local: 'en-us_topic_0000001953327945.html',
                    children: [
                      {
                        id: 2204,
                        parentId: 2202,
                        name: 'Obtaining the Endpoint at the Production End',
                        local: 'en-us_topic_0000001926224624.html'
                      },
                      {
                        id: 2205,
                        parentId: 2202,
                        name: 'Obtaining the AK and SK at the Production End',
                        local: 'en-us_topic_0000001953344001.html'
                      }
                    ]
                  },
                  {
                    id: 2203,
                    parentId: 2194,
                    name: 'Backing Up Object Storage',
                    local: 'object-0010.html',
                    children: [
                      {
                        id: 2206,
                        parentId: 2203,
                        name: 'Step 1: Registering Object Storage',
                        local: 'object-0011.html'
                      },
                      {
                        id: 2207,
                        parentId: 2203,
                        name: 'Step 2: Creating an Object Set',
                        local: 'object-0012.html'
                      },
                      {
                        id: 2208,
                        parentId: 2203,
                        name:
                          'Step 3: (Optional) Creating a Rate Limiting Policy',
                        local: 'object-0013.html'
                      },
                      {
                        id: 2209,
                        parentId: 2203,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'object-0014.html'
                      },
                      {
                        id: 2210,
                        parentId: 2203,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'object-0015.html'
                      },
                      {
                        id: 2211,
                        parentId: 2203,
                        name: 'Step 6: Enabling the NFSv4.1 Service',
                        local: 'object-0016.html'
                      },
                      {
                        id: 2212,
                        parentId: 2203,
                        name: 'Step 7: Performing Backup',
                        local: 'object-0017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2195,
                parentId: 1969,
                name: 'Replication',
                local: 'oracle_gud_000035_10.html',
                children: [
                  {
                    id: 2213,
                    parentId: 2195,
                    name: 'Replicating an Object Storage Copy',
                    local: 'object-0023.html',
                    children: [
                      {
                        id: 2214,
                        parentId: 2213,
                        name: 'Planning the Replication Network',
                        local: 'object-0024.html'
                      },
                      {
                        id: 2215,
                        parentId: 2213,
                        name:
                          'Step 1: Creating a Replication Network Logical Port (Applicable to OceanProtect X Series Backup Appliances)',
                        local: 'fc_gud_0026_1_12.html'
                      },
                      {
                        id: 2216,
                        parentId: 2213,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'object-0026.html'
                      },
                      {
                        id: 2217,
                        parentId: 2213,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'object-0027.html'
                      },
                      {
                        id: 2218,
                        parentId: 2213,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'object-0028.html'
                      },
                      {
                        id: 2219,
                        parentId: 2213,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'en-us_topic_0000002046625105.html'
                      },
                      {
                        id: 2220,
                        parentId: 2213,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'fc_gud_0031_0.html'
                      },
                      {
                        id: 2221,
                        parentId: 2213,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'object-0031.html'
                      },
                      {
                        id: 2222,
                        parentId: 2213,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA ',
                        local: 'object-0032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2196,
                parentId: 1969,
                name: 'Archiving',
                local: 'object-0033.html',
                children: [
                  {
                    id: 2223,
                    parentId: 2196,
                    name: 'Archiving Object Set Backup Copies',
                    local: 'object-0036.html',
                    children: [
                      {
                        id: 2225,
                        parentId: 2223,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'object-0037.html',
                        children: [
                          {
                            id: 2227,
                            parentId: 2225,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'object-0038.html'
                          },
                          {
                            id: 2228,
                            parentId: 2225,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'en-us_topic_0000002098646725.html'
                          }
                        ]
                      },
                      {
                        id: 2226,
                        parentId: 2223,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'object-0040.html'
                      }
                    ]
                  },
                  {
                    id: 2224,
                    parentId: 2196,
                    name: 'Archiving Object Set Replication Copies',
                    local: 'object-0041.html',
                    children: [
                      {
                        id: 2229,
                        parentId: 2224,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'object-0042.html'
                      },
                      {
                        id: 2230,
                        parentId: 2224,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'object-0043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2197,
                parentId: 1969,
                name: 'Restoration',
                local: 'object-0044.html',
                children: [
                  {
                    id: 2231,
                    parentId: 2197,
                    name: 'Restoring Object Storage',
                    local: 'object-0047.html'
                  }
                ]
              },
              {
                id: 2198,
                parentId: 1969,
                name: 'Global Search',
                local: 'object-0048.html',
                children: [
                  {
                    id: 2232,
                    parentId: 2198,
                    name: 'Global Search for Copies',
                    local: 'object-0049.html'
                  },
                  {
                    id: 2233,
                    parentId: 2198,
                    name: 'Global Search for Resources',
                    local: 'object-0050.html'
                  },
                  {
                    id: 2234,
                    parentId: 2198,
                    name: 'Tag-based Global Search',
                    local: 'en-us_topic_0000002002688946.html'
                  }
                ]
              },
              {
                id: 2199,
                parentId: 1969,
                name: 'SLA ',
                local: 'object-0053.html',
                children: [
                  {
                    id: 2235,
                    parentId: 2199,
                    name: 'About SLA',
                    local: 'object-0054.html'
                  },
                  {
                    id: 2236,
                    parentId: 2199,
                    name: 'Viewing SLA Information',
                    local: 'object-0055.html'
                  },
                  {
                    id: 2237,
                    parentId: 2199,
                    name: 'Managing SLAs',
                    local: 'object-0056.html'
                  }
                ]
              },
              {
                id: 2200,
                parentId: 1969,
                name: 'Copies',
                local: 'object-0057.html',
                children: [
                  {
                    id: 2238,
                    parentId: 2200,
                    name: 'Viewing Object Storage Copy Information',
                    local: 'object-0058.html'
                  },
                  {
                    id: 2239,
                    parentId: 2200,
                    name: 'Managing Object Storage Copies',
                    local: 'object-0059.html'
                  }
                ]
              },
              {
                id: 2201,
                parentId: 1969,
                name: 'Object Storage',
                local: 'object-0060.html',
                children: [
                  {
                    id: 2240,
                    parentId: 2201,
                    name: 'Viewing Object Storage Information',
                    local: 'object-0061.html'
                  },
                  {
                    id: 2241,
                    parentId: 2201,
                    name: 'Managing Object Sets',
                    local: 'object-0062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1970,
            parentId: 19,
            name:
              'Common Share Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001873759353.html',
            children: [
              {
                id: 2242,
                parentId: 1970,
                name: 'Backup',
                local: 'Files-0005.html',
                children: [
                  {
                    id: 2250,
                    parentId: 2242,
                    name: 'Backing Up Common Share Resource Data',
                    local: 'Files-0008.html',
                    children: [
                      {
                        id: 2251,
                        parentId: 2250,
                        name: 'Step 1: Creating a Common Share',
                        local: 'Files-0009.html'
                      },
                      {
                        id: 2252,
                        parentId: 2250,
                        name: 'Step 2: Creating a Rate Limiting Policy',
                        local: 'Files-0011.html'
                      },
                      {
                        id: 2253,
                        parentId: 2250,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'commonshares_0013.html'
                      },
                      {
                        id: 2254,
                        parentId: 2250,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'Files-0013.html'
                      },
                      {
                        id: 2255,
                        parentId: 2250,
                        name: 'Step 5: Performing Backup',
                        local: 'Files-0014.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2243,
                parentId: 1970,
                name: 'Replication',
                local: 'oracle_gud_000035_4.html',
                children: [
                  {
                    id: 2256,
                    parentId: 2243,
                    name: 'Replicating a Common Share Copy',
                    local: 'Files-0019.html',
                    children: [
                      {
                        id: 2257,
                        parentId: 2256,
                        name:
                          'Step 1: Creating a Replication Network Logical Port(Applicable to OceanProtect X Series Backup Appliances (1.6.0 and Later Versions))',
                        local: 'fc_gud_0026_1_6.html'
                      },
                      {
                        id: 2258,
                        parentId: 2256,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'Files-0022.html'
                      },
                      {
                        id: 2259,
                        parentId: 2256,
                        name: '(Optional) Enabling Replication Link Encryption',
                        local: 'Files-0023.html'
                      },
                      {
                        id: 2260,
                        parentId: 2256,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'Files-0024.html'
                      },
                      {
                        id: 2261,
                        parentId: 2256,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'en-us_topic_0000002010591334.html'
                      },
                      {
                        id: 2262,
                        parentId: 2256,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'Files-0026.html'
                      },
                      {
                        id: 2263,
                        parentId: 2256,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'Files-0027.html'
                      },
                      {
                        id: 2264,
                        parentId: 2256,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA ',
                        local: 'Files-0028.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2244,
                parentId: 1970,
                name: 'Archiving',
                local: 'Files-0029.html',
                children: [
                  {
                    id: 2265,
                    parentId: 2244,
                    name: 'Archiving Backup Copies of a Common Share Resource',
                    local: 'Files-0032.html',
                    children: [
                      {
                        id: 2267,
                        parentId: 2265,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'Files-0033.html',
                        children: [
                          {
                            id: 2269,
                            parentId: 2267,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'Files-0034.html'
                          },
                          {
                            id: 2270,
                            parentId: 2267,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library) (Applicable to OceanProtect X Series Backup Appliances)',
                            local: 'Files-0035.html'
                          }
                        ]
                      },
                      {
                        id: 2268,
                        parentId: 2265,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'Files-0036.html'
                      }
                    ]
                  },
                  {
                    id: 2266,
                    parentId: 2244,
                    name:
                      'Archiving Replication Copies of a Common Share Resource',
                    local: 'Files-0037.html',
                    children: [
                      {
                        id: 2271,
                        parentId: 2266,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'Files-0038.html'
                      },
                      {
                        id: 2272,
                        parentId: 2266,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'Files-0039.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2245,
                parentId: 1970,
                name: 'Managing Share Information',
                local: 'commonshares_0046.html',
                children: [
                  {
                    id: 2273,
                    parentId: 2245,
                    name: 'Configuring Share Information',
                    local: 'commonshares_0047.html'
                  },
                  {
                    id: 2274,
                    parentId: 2245,
                    name: 'Viewing Share Information',
                    local: 'commonshares_0048.html'
                  },
                  {
                    id: 2275,
                    parentId: 2245,
                    name: 'Deleting Share Information',
                    local: 'commonshares_0049.html'
                  }
                ]
              },
              {
                id: 2246,
                parentId: 1970,
                name: 'Global Search',
                local: 'Files-0054.html',
                children: [
                  {
                    id: 2276,
                    parentId: 2246,
                    name: 'About Global Search',
                    local: 'en-us_topic_0000002126212221.html'
                  },
                  {
                    id: 2277,
                    parentId: 2246,
                    name: 'Global Search for Copies',
                    local: 'Files-0055.html'
                  },
                  {
                    id: 2278,
                    parentId: 2246,
                    name: 'Global Search for Resources',
                    local: 'Files-0056.html'
                  },
                  {
                    id: 2279,
                    parentId: 2246,
                    name: 'Tag-based Global Search',
                    local: 'en-us_topic_0000002002851498.html'
                  }
                ]
              },
              {
                id: 2247,
                parentId: 1970,
                name: 'SLA ',
                local: 'en-us_topic_0000001927513108.html',
                children: [
                  {
                    id: 2280,
                    parentId: 2247,
                    name: 'About SLA',
                    local: 'vmware_gud_000026.html'
                  },
                  {
                    id: 2281,
                    parentId: 2247,
                    name: 'Viewing SLA Information',
                    local: 'en-us_topic_0000001954672445.html'
                  },
                  {
                    id: 2282,
                    parentId: 2247,
                    name: 'Managing SLAs',
                    local: 'en-us_topic_0000001927353788.html'
                  }
                ]
              },
              {
                id: 2248,
                parentId: 1970,
                name: 'Copies',
                local: 'Files-0063.html',
                children: [
                  {
                    id: 2283,
                    parentId: 2248,
                    name:
                      'Viewing Copy Information About a Common Share Resource',
                    local: 'Files-0064.html'
                  },
                  {
                    id: 2284,
                    parentId: 2248,
                    name: 'Managing a Common Share Resource Copy',
                    local: 'Files-0065.html'
                  }
                ]
              },
              {
                id: 2249,
                parentId: 1970,
                name: 'Common Share',
                local: 'Files-0066.html',
                children: [
                  {
                    id: 2285,
                    parentId: 2249,
                    name: 'Viewing Common Share Information',
                    local: 'Files-0067.html'
                  },
                  {
                    id: 2286,
                    parentId: 2249,
                    name: 'Managing Common Shares',
                    local: 'Files-0068.html'
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
    name: 'Explore',
    local: 'helpcenter000084.html',
    children: [
      {
        id: 2287,
        parentId: 4,
        name: 'Restoration Drill',
        local: 'en-us_topic_0000001827336292.html',
        children: [
          {
            id: 2291,
            parentId: 2287,
            name: 'Creating a Drill Plan',
            local: 'Ransomware0011.html'
          },
          {
            id: 2292,
            parentId: 2287,
            name: 'Managing Drill Plans',
            local: 'Ransomware0012.html'
          },
          {
            id: 2293,
            parentId: 2287,
            name: 'Viewing the Restoration Drill Overview',
            local: 'Ransomware0013.html'
          }
        ]
      },
      {
        id: 2288,
        parentId: 4,
        name: 'Data Anonymization',
        local: 'helpcenter000108.html',
        children: [
          {
            id: 2294,
            parentId: 2288,
            name: 'Configuring Data Anonymization',
            local: 'data_masking_000009.html',
            children: [
              {
                id: 2299,
                parentId: 2294,
                name: 'Importing and Activating a License File',
                local: 'data_masking_000010.html'
              },
              {
                id: 2300,
                parentId: 2294,
                name: 'Adding Anonymization Rules',
                local: 'data_masking_000011.html'
              },
              {
                id: 2301,
                parentId: 2294,
                name: 'Adding Identification Rules',
                local: 'data_masking_000012.html'
              },
              {
                id: 2302,
                parentId: 2294,
                name: 'Creating Anonymization Policies',
                local: 'data_masking_000013.html'
              }
            ]
          },
          {
            id: 2295,
            parentId: 2288,
            name: 'Anonymizing Data in an Oracle Database',
            local: 'data_masking_000015.html'
          },
          {
            id: 2296,
            parentId: 2288,
            name: 'Managing Data Anonymization',
            local: 'data_masking_000018.html',
            children: [
              {
                id: 2303,
                parentId: 2296,
                name: 'Managing Anonymization Policies',
                local: 'data_masking_000019.html'
              },
              {
                id: 2304,
                parentId: 2296,
                name: 'Managing Identification Rules',
                local: 'data_masking_000020.html'
              },
              {
                id: 2305,
                parentId: 2296,
                name: 'Managing Anonymization Rules',
                local: 'data_masking_000021.html'
              }
            ]
          },
          {
            id: 2297,
            parentId: 2288,
            name: 'Anonymization Rule Types',
            local: 'data_masking_000022.html'
          },
          {
            id: 2298,
            parentId: 2288,
            name: 'Configuring Database Listening',
            local: 'data_masking_000024.html'
          }
        ]
      },
      {
        id: 2289,
        parentId: 4,
        name: 'Ransomware Protection',
        local: 'helpcenter000110.html',
        children: [
          {
            id: 2306,
            parentId: 2289,
            name: 'Configuring Ransomware Protection for Copies',
            local: 'ransome_0011.html',
            children: [
              {
                id: 2312,
                parentId: 2306,
                name: 'Creating a Ransomware Protection and WORM Policy',
                local: 'ransome_0012.html'
              }
            ]
          },
          {
            id: 2307,
            parentId: 2289,
            name:
              'Performing Ransomware Protection for Copies (Applicable to 1.5.0)',
            local: 'ransome_0013.html',
            children: [
              {
                id: 2313,
                parentId: 2307,
                name: 'VMware Copies',
                local: 'ransome_0014.html'
              },
              {
                id: 2314,
                parentId: 2307,
                name: 'NAS File System Copies',
                local: 'ransome_0015.html'
              },
              {
                id: 2315,
                parentId: 2307,
                name: 'NAS Share Copies',
                local: 'ransome_0016.html'
              },
              {
                id: 2316,
                parentId: 2307,
                name: 'Fileset Copies',
                local: 'ransome_0017.html'
              }
            ]
          },
          {
            id: 2308,
            parentId: 2289,
            name:
              'Performing Ransomware Protection for Copies (Applicable to 1.6.0 and Later Versions)',
            local: 'ransome16_001.html',
            children: [
              {
                id: 2317,
                parentId: 2308,
                name: 'VMware VM Copies',
                local: 'ransome16_002.html'
              },
              {
                id: 2318,
                parentId: 2308,
                name: 'NAS File System Copies',
                local: 'ransome16_003.html'
              },
              {
                id: 2319,
                parentId: 2308,
                name: 'NAS Share Copies',
                local: 'ransome16_004.html'
              },
              {
                id: 2320,
                parentId: 2308,
                name: 'Fileset Copies',
                local: 'ransome16_005.html'
              },
              {
                id: 2321,
                parentId: 2308,
                name: 'CNware VM Copies',
                local: 'ransome16_006.html'
              },
              {
                id: 2322,
                parentId: 2308,
                name: 'Huawei Cloud Stack Copies',
                local: 'ransome16_007.html'
              },
              {
                id: 2323,
                parentId: 2308,
                name: 'FusionCompute VM Copies',
                local: 'ransome16_008.html'
              },
              {
                id: 2324,
                parentId: 2308,
                name: 'OpenStack Cloud Server Copies',
                local: 'ransome16_009.html'
              },
              {
                id: 2325,
                parentId: 2308,
                name: 'Hyper-V VM Copies',
                local: 'ransome16_010.html'
              },
              {
                id: 2326,
                parentId: 2308,
                name: 'FusionOne Compute VM Copies',
                local: 'ransome160_012.html'
              }
            ]
          },
          {
            id: 2309,
            parentId: 2289,
            name: 'Managing Ransomware Protection for Copies',
            local: 'ransome_0018.html',
            children: [
              {
                id: 2327,
                parentId: 2309,
                name: 'Managing Detection Models',
                local: 'ransome_0019.html'
              },
              {
                id: 2328,
                parentId: 2309,
                name: 'Managing Ransomware Protection and WORM Policies',
                local: 'ransome_0020.html'
              },
              {
                id: 2329,
                parentId: 2309,
                name: 'Managing the Detection Mode',
                local: 'ransome_0021.html'
              },
              {
                id: 2330,
                parentId: 2309,
                name: 'Managing Ransomware Detection Copies',
                local: 'ransome_0022.html'
              },
              {
                id: 2331,
                parentId: 2309,
                name: 'Managing WORM Copies',
                local: 'ransome_0023.html'
              },
              {
                id: 2332,
                parentId: 2309,
                name:
                  'Managing Operation Restrictions on Infected Copies (Applicable to 1.6.0 and Later Versions)',
                local: 'ransome_dis_001.html',
                children: [
                  {
                    id: 2333,
                    parentId: 2332,
                    name: 'Adding Operation Restrictions on Infected Copies',
                    local: 'ransome_dis_002.html'
                  },
                  {
                    id: 2334,
                    parentId: 2332,
                    name:
                      'Viewing Information About Operation Restrictions on Infected Copies',
                    local: 'ransome_dis_003.html'
                  },
                  {
                    id: 2335,
                    parentId: 2332,
                    name: 'Modifying Operation Restrictions on Infected Copies',
                    local: 'ransome_dis_004.html'
                  },
                  {
                    id: 2336,
                    parentId: 2332,
                    name:
                      'Deleting Modifying Operation Restrictions on Infected Copies',
                    local: 'ransome_dis_005.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2310,
            parentId: 2289,
            name: 'Viewing Resource Detection Details (Applicable to 1.5.0)',
            local: 'ransome_0024.html',
            children: [
              {
                id: 2337,
                parentId: 2310,
                name: 'Viewing Detection Details of All Resources',
                local: 'ransome_0025.html'
              },
              {
                id: 2338,
                parentId: 2310,
                name: 'Viewing Detection Details of a Single Resource Type',
                local: 'ransome_0026.html'
              }
            ]
          },
          {
            id: 2311,
            parentId: 2289,
            name:
              'Viewing Resource Detection Details (Applicable to 1.6.0 and Later Versions)',
            local: 'ransome16_011.html',
            children: [
              {
                id: 2339,
                parentId: 2311,
                name: 'Viewing Detection Details of All Resources',
                local: 'ransome16_012.html'
              },
              {
                id: 2340,
                parentId: 2311,
                name: 'Viewing Detection Details of a Single Resource Type',
                local: 'ransome16_013.html'
              }
            ]
          }
        ]
      },
      {
        id: 2290,
        parentId: 4,
        name: 'Air Gap',
        local: 'helpcenter000112.html',
        children: [
          {
            id: 2341,
            parentId: 2290,
            name: 'Configuring Air Gap',
            local: 'AirGap0011.html',
            children: [
              {
                id: 2344,
                parentId: 2341,
                name: 'Creating an Air Gap Policy',
                local: 'AirGap0012.html'
              },
              {
                id: 2345,
                parentId: 2341,
                name: 'Associating an Air Gap Policy',
                local: 'AirGap0013.html'
              }
            ]
          },
          {
            id: 2342,
            parentId: 2290,
            name: 'Managing Air Gap Policies',
            local: 'AirGap0014.html',
            children: [
              {
                id: 2346,
                parentId: 2342,
                name: 'Viewing Air Gap Policies',
                local: 'AirGap0015.html'
              },
              {
                id: 2347,
                parentId: 2342,
                name: 'Modifying an Air Gap Policy',
                local: 'AirGap0016.html'
              },
              {
                id: 2348,
                parentId: 2342,
                name: 'Deleting an Air Gap Policy',
                local: 'AirGap0017.html'
              }
            ]
          },
          {
            id: 2343,
            parentId: 2290,
            name: 'Managing Storage Devices',
            local: 'AirGap0018.html',
            children: [
              {
                id: 2349,
                parentId: 2343,
                name: 'Viewing Storage Devices',
                local: 'AirGap0019.html'
              },
              {
                id: 2350,
                parentId: 2343,
                name:
                  'Modifying the Air Gap Policy Associated with a Storage Device',
                local: 'AirGap0020.html'
              },
              {
                id: 2351,
                parentId: 2343,
                name:
                  'Removing the Air Gap Policy Associated with a Storage Device',
                local: 'AirGap0021.html'
              },
              {
                id: 2352,
                parentId: 2343,
                name:
                  'Enabling the Air Gap Policy Associated with a Storage Device',
                local: 'AirGap0022.html'
              },
              {
                id: 2353,
                parentId: 2343,
                name:
                  'Disabling the Air Gap Policy Associated with a Storage Device',
                local: 'AirGap0023.html'
              },
              {
                id: 2354,
                parentId: 2343,
                name:
                  'Disconnecting the Replication Link of a Storage Device (Applicable to 1.6.0 and Later Versions)',
                local: 'en-us_topic_0000002037030445.html'
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
    name: 'Cluster High Availability',
    local: 'en-us_topic_0000001792345234.html',
    children: [
      {
        id: 2355,
        parentId: 5,
        name: 'Configuring Cluster HA',
        local: 'HA00014.html',
        children: [
          {
            id: 2358,
            parentId: 2355,
            name:
              'Adding an Internal Communication Network Plane for the Active Node',
            local: 'HA00015.html',
            children: [
              {
                id: 2363,
                parentId: 2358,
                name:
                  'Adding an Internal Communication Network Plane for the Active Node (Applicable to 1.5.0)',
                local: 'HA00016.html'
              },
              {
                id: 2364,
                parentId: 2358,
                name:
                  'Adding an Internal Communication Network for the Active Node (Applicable to 1.6.0 and Later Versions)',
                local: 'HA00017.html'
              }
            ]
          },
          {
            id: 2359,
            parentId: 2355,
            name:
              'Adding an Internal Communication Network for a Member Node (Applicable to 1.6.0 and Later Versions)',
            local: 'HA00018.html'
          },
          {
            id: 2360,
            parentId: 2355,
            name: 'Adding Member Nodes',
            local: 'HA00019.html'
          },
          {
            id: 2361,
            parentId: 2355,
            name: 'Adding HA Members',
            local: 'HA00020.html'
          },
          {
            id: 2362,
            parentId: 2355,
            name: '(Optional) Creating Backup Storage Unit Groups',
            local: 'HA00021.html'
          }
        ]
      },
      {
        id: 2356,
        parentId: 5,
        name: 'Using Cluster HA',
        local: 'HA00022.html'
      },
      {
        id: 2357,
        parentId: 5,
        name: 'Managing Cluster HA',
        local: 'HA00023.html',
        children: [
          {
            id: 2365,
            parentId: 2357,
            name: 'Managing Local Cluster Nodes',
            local: 'HA00024.html',
            children: [
              {
                id: 2372,
                parentId: 2365,
                name: 'Viewing a Local Cluster Node',
                local: 'HA00025.html'
              },
              {
                id: 2373,
                parentId: 2365,
                name: 'Managing a Standby or Member Node',
                local: 'HA00026.html',
                children: [
                  {
                    id: 2375,
                    parentId: 2373,
                    name: 'Modifying a Standby or Member Node',
                    local: 'HA00027.html'
                  },
                  {
                    id: 2376,
                    parentId: 2373,
                    name: 'Deleting a Member Node',
                    local: 'HA00028.html'
                  }
                ]
              },
              {
                id: 2374,
                parentId: 2365,
                name: 'Managing the HA',
                local: 'HA00029.html',
                children: [
                  {
                    id: 2377,
                    parentId: 2374,
                    name: 'Modifying HA Parameters',
                    local: 'HA00030.html'
                  },
                  {
                    id: 2378,
                    parentId: 2374,
                    name: 'Removing an HA Member',
                    local: 'HA00031.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2366,
            parentId: 2357,
            name: 'Managing Backup Storage Unit Groups',
            local: 'HA00032.html',
            children: [
              {
                id: 2379,
                parentId: 2366,
                name: 'Viewing a Backup Storage Unit Group',
                local: 'HA00033.html'
              },
              {
                id: 2380,
                parentId: 2366,
                name: 'Modifying a Backup Storage Unit Group',
                local: 'HA00034.html'
              },
              {
                id: 2381,
                parentId: 2366,
                name: 'Deleting a Backup Storage Unit Group',
                local: 'HA00035.html'
              }
            ]
          },
          {
            id: 2367,
            parentId: 2357,
            name: 'Managing Backup Storage Units (Applicable to 1.5.0)',
            local: 'HA00036.html',
            children: [
              {
                id: 2382,
                parentId: 2367,
                name: 'Viewing a Backup Storage Unit',
                local: 'HA00037.html'
              },
              {
                id: 2383,
                parentId: 2367,
                name: 'Creating a Backup Storage Unit',
                local: 'HA00038.html'
              },
              {
                id: 2384,
                parentId: 2367,
                name: 'Modifying a Backup Storage Unit',
                local: 'HA00039.html'
              },
              {
                id: 2385,
                parentId: 2367,
                name: 'Deleting a Backup Storage Unit',
                local: 'HA00040.html'
              },
              {
                id: 2386,
                parentId: 2367,
                name: 'Upgrading a Backup Storage Unit as a Member Node',
                local: 'HA00041.html'
              }
            ]
          },
          {
            id: 2368,
            parentId: 2357,
            name:
              'Managing Backup Storage Devices (Applicable to 1.6.0 and Later Versions)',
            local: 'HA00042.html',
            children: [
              {
                id: 2387,
                parentId: 2368,
                name: 'Viewing a Backup Storage Device',
                local: 'HA00043.html'
              },
              {
                id: 2388,
                parentId: 2368,
                name: 'Creating a Backup Storage Device',
                local: 'HA00044.html'
              },
              {
                id: 2389,
                parentId: 2368,
                name: 'Modifying a Backup Storage Device',
                local: 'HA00045.html'
              },
              {
                id: 2390,
                parentId: 2368,
                name: 'Deleting a Backup Storage Device',
                local: 'HA00046.html'
              },
              {
                id: 2391,
                parentId: 2368,
                name: 'Upgrading a Backup Storage Device as a Member Node',
                local: 'HA00047.html'
              }
            ]
          },
          {
            id: 2369,
            parentId: 2357,
            name:
              'Managing Backup Storage Units (Applicable to 1.6.0 and Later Versions)',
            local: 'HA00048.html',
            children: [
              {
                id: 2392,
                parentId: 2369,
                name: 'Viewing a Backup Storage Unit',
                local: 'HA00049.html'
              },
              {
                id: 2393,
                parentId: 2369,
                name: 'Creating a Backup Storage Unit',
                local: 'HA00050.html'
              },
              {
                id: 2394,
                parentId: 2369,
                name: 'Modifying a Backup Storage Unit',
                local: 'HA00051.html'
              },
              {
                id: 2395,
                parentId: 2369,
                name: 'Deleting a Backup Storage Unit',
                local: 'HA00052.html'
              }
            ]
          },
          {
            id: 2370,
            parentId: 2357,
            name:
              'Managing an Internal Communication Network Plane (Applicable to 1.5.0)',
            local: 'HA00053.html',
            children: [
              {
                id: 2396,
                parentId: 2370,
                name: 'Modifying an Internal Communication Network Plane',
                local: 'HA00054.html'
              },
              {
                id: 2397,
                parentId: 2370,
                name: 'Deleting an Internal Communication Network Plane',
                local: 'HA00055.html'
              }
            ]
          },
          {
            id: 2371,
            parentId: 2357,
            name:
              'Managing an Internal Communication Network (Applicable to 1.6.0 and Later Versions)',
            local: 'HA00056.html',
            children: [
              {
                id: 2398,
                parentId: 2371,
                name: 'Modifying an Internal Communication Network',
                local: 'HA00057.html'
              },
              {
                id: 2399,
                parentId: 2371,
                name: 'Deleting an Internal Communication Network',
                local: 'HA00058.html'
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
    name: 'Monitoring',
    local: 'admin-00134.html',
    children: [
      {
        id: 2400,
        parentId: 6,
        name: 'Managing Performance Statistics',
        local: 'admin-00135.html',
        children: [
          {
            id: 2404,
            parentId: 2400,
            name: 'Introduction to Performance Indicators',
            local: 'admin-00136.html'
          },
          {
            id: 2405,
            parentId: 2400,
            name: 'Configuring the Performance Statistics Switch',
            local: 'admin-00137.html'
          }
        ]
      },
      {
        id: 2401,
        parentId: 6,
        name: 'Managing Alarms and Events',
        local: 'admin-00139.html'
      },
      {
        id: 2402,
        parentId: 6,
        name: 'Managing Jobs',
        local: 'admin-00140.html',
        children: [
          {
            id: 2406,
            parentId: 2402,
            name: 'Viewing the Job Progress',
            local: 'admin-00141.html'
          },
          {
            id: 2407,
            parentId: 2402,
            name: 'Stopping Jobs',
            local: 'admin-00142.html'
          },
          {
            id: 2408,
            parentId: 2402,
            name: 'Downloading Jobs',
            local: 'admin-00143.html'
          }
        ]
      },
      {
        id: 2403,
        parentId: 6,
        name: 'Managing Reports',
        local: 'admin-00144.html',
        children: [
          {
            id: 2409,
            parentId: 2403,
            name: 'User Roles and Permissions',
            local: 'admin-00145.html'
          },
          {
            id: 2410,
            parentId: 2403,
            name: 'Creating a Report',
            local: 'admin-00146.html'
          },
          {
            id: 2411,
            parentId: 2403,
            name: 'Viewing Reports',
            local: 'admin-00147.html'
          },
          {
            id: 2412,
            parentId: 2403,
            name: 'Downloading Reports',
            local: 'admin-00148.html'
          },
          {
            id: 2413,
            parentId: 2403,
            name: 'Sending an Email',
            local: 'admin-00149.html'
          },
          {
            id: 2414,
            parentId: 2403,
            name: 'Deleting Reports',
            local: 'admin-00150.html'
          }
        ]
      }
    ]
  },
  {
    id: 7,
    parentId: 0,
    name: 'System',
    local: 'helpcenter000126.html',
    children: [
      {
        id: 2415,
        parentId: 7,
        name: 'Managing Users (Applicable to 1.5.0)',
        local: 'helpcenter000127.html',
        children: [
          {
            id: 2440,
            parentId: 2415,
            name: 'Overview of User Roles',
            local: 'helpcenter000128.html'
          },
          {
            id: 2441,
            parentId: 2415,
            name: 'Creating a User',
            local: 'helpcenter000129.html'
          },
          {
            id: 2442,
            parentId: 2415,
            name: 'Modifying a User',
            local: 'helpcenter000130.html'
          },
          {
            id: 2443,
            parentId: 2415,
            name: 'Locking Out a User',
            local: 'helpcenter000131.html'
          },
          {
            id: 2444,
            parentId: 2415,
            name: 'Unlocking a User',
            local: 'helpcenter000132.html'
          },
          {
            id: 2445,
            parentId: 2415,
            name: 'Deleting a User',
            local: 'helpcenter000133.html'
          },
          {
            id: 2446,
            parentId: 2415,
            name: 'Resetting the User Password',
            local: 'helpcenter000134.html'
          },
          {
            id: 2447,
            parentId: 2415,
            name: 'Resetting the System Administrator Password',
            local: 'helpcenter000135.html'
          }
        ]
      },
      {
        id: 2416,
        parentId: 7,
        name: 'Managing RBAC (Applicable to 1.6.0 and Later Versions)',
        local: 'admin-0055.html',
        children: [
          {
            id: 2448,
            parentId: 2416,
            name: 'Built-in User Roles',
            local: 'admin-0056.html'
          },
          {
            id: 2449,
            parentId: 2416,
            name: 'Creating a Role',
            local: 'en-us_topic_0000002059543622.html'
          },
          {
            id: 2450,
            parentId: 2416,
            name: 'Modifying a Role',
            local: 'en-us_topic_0000002059385278.html'
          },
          {
            id: 2451,
            parentId: 2416,
            name: 'Cloning a Role',
            local: 'en-us_topic_0000002095582433.html'
          },
          {
            id: 2452,
            parentId: 2416,
            name: 'Deleting a Role',
            local: 'en-us_topic_0000002095463905.html'
          },
          {
            id: 2453,
            parentId: 2416,
            name: 'Creating a Resource Set',
            local: 'en-us_topic_0000002059543630.html'
          },
          {
            id: 2454,
            parentId: 2416,
            name: 'Deleting a Resource Set',
            local: 'en-us_topic_0000002059385282.html'
          },
          {
            id: 2455,
            parentId: 2416,
            name: 'Modifying a Resource Set',
            local: 'en-us_topic_0000002095582437.html'
          },
          {
            id: 2456,
            parentId: 2416,
            name: 'Creating a User',
            local: 'admin-0057.html'
          },
          {
            id: 2457,
            parentId: 2416,
            name: 'Modifying a User',
            local: 'admin-0058.html'
          },
          {
            id: 2458,
            parentId: 2416,
            name: 'Locking Out a User',
            local: 'admin-0059.html'
          },
          {
            id: 2459,
            parentId: 2416,
            name: 'Unlocking a User',
            local: 'admin-0060.html'
          },
          {
            id: 2460,
            parentId: 2416,
            name: 'Deleting a User',
            local: 'admin-0061.html'
          },
          {
            id: 2461,
            parentId: 2416,
            name: 'Resetting the User Password',
            local: 'admin-0062.html'
          },
          {
            id: 2462,
            parentId: 2416,
            name: 'Resetting the System Administrator Password',
            local: 'admin-0063.html'
          },
          {
            id: 2463,
            parentId: 2416,
            name: 'Setting the Mailbox for Password Retrieval',
            local: 'admin-0064.html'
          }
        ]
      },
      {
        id: 2417,
        parentId: 7,
        name: 'Managing the SAML SSO Configuration',
        local: 'helpcenter000136.html',
        children: [
          {
            id: 2464,
            parentId: 2417,
            name: 'Creating the SAML SSO Configuration',
            local: 'helpcenter000137.html'
          },
          {
            id: 2465,
            parentId: 2417,
            name: 'Managing the SAML SSO Configuration',
            local: 'helpcenter000138.html',
            children: [
              {
                id: 2467,
                parentId: 2465,
                name: 'Activating or Disabling the SAML SSO Configuration',
                local: 'helpcenter000139.html'
              },
              {
                id: 2468,
                parentId: 2465,
                name: 'Modifying the SAML SSO Configuration',
                local: 'helpcenter000140.html'
              },
              {
                id: 2469,
                parentId: 2465,
                name: 'Deleting the SAML SSO Configuration',
                local: 'helpcenter000141.html'
              }
            ]
          },
          {
            id: 2466,
            parentId: 2417,
            name: 'Exporting Metadata',
            local: 'helpcenter000142.html'
          }
        ]
      },
      {
        id: 2418,
        parentId: 7,
        name: 'Managing Quotas and Functions',
        local: 'helpcenter000143.html',
        children: [
          {
            id: 2470,
            parentId: 2418,
            name: 'Viewing Quotas and Functions',
            local: 'helpcenter000144.html'
          },
          {
            id: 2471,
            parentId: 2418,
            name: 'Setting Quotas',
            local: 'helpcenter000145.html'
          },
          {
            id: 2472,
            parentId: 2418,
            name: 'Setting Functions',
            local: 'helpcenter000146.html'
          }
        ]
      },
      {
        id: 2419,
        parentId: 7,
        name:
          'Managing Backup Clusters (Applicable to OceanProtect X Series Backup Appliances)',
        local: 'admin-00067.html',
        children: [
          {
            id: 2473,
            parentId: 2419,
            name:
              'Managing Local Cluster Nodes (Not Supported by OceanProtect E1000)',
            local: 'en-us_topic_0000001839224397.html',
            children: [
              {
                id: 2475,
                parentId: 2473,
                name: 'Viewing a Local Cluster Node',
                local: 'en-us_topic_0000001839144469.html'
              },
              {
                id: 2476,
                parentId: 2473,
                name: 'Managing a Standby or Member Node',
                local: 'en-us_topic_0000001792345350.html',
                children: [
                  {
                    id: 2477,
                    parentId: 2476,
                    name: 'Modifying a Standby or Member Node',
                    local: 'en-us_topic_0000001792345230.html'
                  },
                  {
                    id: 2478,
                    parentId: 2476,
                    name: 'Deleting a Member Node',
                    local: 'en-us_topic_0000001792345338.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2474,
            parentId: 2419,
            name:
              'Managing Multi-domain Clusters (Not Supported by OceanProtect E1000)',
            local: 'en-us_topic_0000001839224421.html',
            children: [
              {
                id: 2479,
                parentId: 2474,
                name: 'Viewing Cluster Information',
                local: 'admin-00068.html'
              },
              {
                id: 2480,
                parentId: 2474,
                name: 'Adding an External Cluster',
                local: 'admin-00069.html'
              },
              {
                id: 2481,
                parentId: 2474,
                name: 'Modifying External Cluster Information',
                local: 'admin-00070.html'
              },
              {
                id: 2482,
                parentId: 2474,
                name: 'Deleting an External Cluster',
                local: 'admin-00071.html'
              },
              {
                id: 2483,
                parentId: 2474,
                name: 'Specifying an External Cluster as a Management Cluster',
                local: 'admin-00072.html'
              },
              {
                id: 2484,
                parentId: 2474,
                name: 'Specifying an External Cluster as a Managed Cluster',
                local: 'admin-00073.html'
              },
              {
                id: 2485,
                parentId: 2474,
                name: 'Authorization',
                local: 'admin-00074.html'
              },
              {
                id: 2486,
                parentId: 2474,
                name:
                  'Modifying Authorization of a Local Cluster Data Protection Administrator',
                local: 'admin-00075.html'
              },
              {
                id: 2487,
                parentId: 2474,
                name:
                  'Canceling Authorization of a Local Cluster Data Protection Administrator',
                local: 'admin-00076.html'
              }
            ]
          }
        ]
      },
      {
        id: 2420,
        parentId: 7,
        name: 'Managing Replication Clusters',
        local: 'en-us_topic_0000001839144457.html',
        children: [
          {
            id: 2488,
            parentId: 2420,
            name: 'Adding an External Cluster',
            local: 'en-us_topic_0000001839224425.html'
          },
          {
            id: 2489,
            parentId: 2420,
            name: 'Viewing Cluster Information',
            local: 'en-us_topic_0000001839144417.html'
          },
          {
            id: 2490,
            parentId: 2420,
            name: 'Modifying a Replication Cluster',
            local: 'en-us_topic_0000001839224361.html'
          },
          {
            id: 2491,
            parentId: 2420,
            name: 'Deleting a Replication Cluster',
            local: 'en-us_topic_0000001839144453.html'
          }
        ]
      },
      {
        id: 2421,
        parentId: 7,
        name: 'Managing Local Storage',
        local: 'admin-00078.html',
        children: [
          {
            id: 2492,
            parentId: 2421,
            name: 'Viewing Local Storage Information',
            local: 'admin-00079.html'
          },
          {
            id: 2493,
            parentId: 2421,
            name:
              'Configuring the Capacity Alarm Threshold of the Local Storage',
            local: 'admin-00080.html'
          },
          {
            id: 2494,
            parentId: 2421,
            name: 'Viewing Local Storage Authentication Information',
            local: 'admin-00081.html'
          },
          {
            id: 2495,
            parentId: 2421,
            name: 'Modifying Local Storage Authentication Information',
            local: 'admin-00082.html'
          },
          {
            id: 2496,
            parentId: 2421,
            name: 'Manually Reclaiming Space',
            local: 'admin-00087.html'
          }
        ]
      },
      {
        id: 2422,
        parentId: 7,
        name: 'Managing Object-based Storage',
        local: 'helpcenter000168.html',
        children: [
          {
            id: 2497,
            parentId: 2422,
            name: 'Adding Archive Storage',
            local: 'oracle_gud_000030.html'
          },
          {
            id: 2498,
            parentId: 2422,
            name: 'Importing Archive Storage Copies',
            local: 'helpcenter000170.html'
          },
          {
            id: 2499,
            parentId: 2422,
            name: 'Modifying Basic Information of an Archive Storage',
            local: 'helpcenter000171.html'
          },
          {
            id: 2500,
            parentId: 2422,
            name: 'Modifying the Capacity Alarm Threshold of Archive Storage',
            local: 'helpcenter000172.html'
          },
          {
            id: 2501,
            parentId: 2422,
            name: 'Viewing Archive Storage Information',
            local: 'helpcenter000173.html'
          },
          {
            id: 2502,
            parentId: 2422,
            name: 'Deleting Archive Storage',
            local: 'helpcenter000174.html'
          }
        ]
      },
      {
        id: 2423,
        parentId: 7,
        name:
          'Managing Tapes (Applicable to OceanProtect X Series Backup Appliances)',
        local: 'helpcenter000175.html',
        children: [
          {
            id: 2503,
            parentId: 2423,
            name: 'Managing Tape Libraries',
            local: 'helpcenter000176.html',
            children: [
              {
                id: 2505,
                parentId: 2503,
                name: 'Scanning Tape Libraries',
                local: 'helpcenter000177.html'
              },
              {
                id: 2506,
                parentId: 2503,
                name: 'Managing Drivers',
                local: 'helpcenter000178.html',
                children: [
                  {
                    id: 2508,
                    parentId: 2506,
                    name: 'Checking the Driver',
                    local: 'helpcenter000179.html'
                  },
                  {
                    id: 2509,
                    parentId: 2506,
                    name: 'Enabling a Driver',
                    local: 'helpcenter000180.html'
                  },
                  {
                    id: 2510,
                    parentId: 2506,
                    name: 'Disabling a Driver',
                    local: 'helpcenter000181.html'
                  }
                ]
              },
              {
                id: 2507,
                parentId: 2503,
                name: 'Managing Tapes',
                local: 'helpcenter000182.html',
                children: [
                  {
                    id: 2511,
                    parentId: 2507,
                    name: 'Viewing Tapes',
                    local: 'helpcenter000183.html'
                  },
                  {
                    id: 2512,
                    parentId: 2507,
                    name: 'Loading Tapes',
                    local: 'helpcenter000184.html'
                  },
                  {
                    id: 2513,
                    parentId: 2507,
                    name: 'Unloading Tapes',
                    local: 'helpcenter000185.html'
                  },
                  {
                    id: 2514,
                    parentId: 2507,
                    name: 'Deleting Tapes',
                    local: 'helpcenter000186.html'
                  },
                  {
                    id: 2515,
                    parentId: 2507,
                    name: 'Identifying Tapes',
                    local: 'helpcenter000187.html'
                  },
                  {
                    id: 2516,
                    parentId: 2507,
                    name: 'Marking a Tape as Empty',
                    local: 'helpcenter000188.html'
                  },
                  {
                    id: 2517,
                    parentId: 2507,
                    name: 'Erasing Tapes',
                    local: 'helpcenter000189.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2504,
            parentId: 2423,
            name: 'Managing Media Sets',
            local: 'helpcenter000190.html',
            children: [
              {
                id: 2518,
                parentId: 2504,
                name: 'Creating a Media Set',
                local: 'helpcenter000191.html'
              },
              {
                id: 2519,
                parentId: 2504,
                name: 'Viewing a Media Set',
                local: 'helpcenter000192.html'
              },
              {
                id: 2520,
                parentId: 2504,
                name: 'Modifying a Media Set',
                local: 'helpcenter000193.html'
              },
              {
                id: 2521,
                parentId: 2504,
                name: 'Deleting a Media Set',
                local: 'helpcenter000194.html'
              }
            ]
          }
        ]
      },
      {
        id: 2424,
        parentId: 7,
        name:
          'Viewing System Information (Applicable to OceanProtect X Series Backup Appliances)',
        local: 'helpcenter000195.html',
        children: [
          {
            id: 2522,
            parentId: 2424,
            name: 'Viewing System Version Information',
            local: 'helpcenter000196.html'
          },
          {
            id: 2523,
            parentId: 2424,
            name: 'Viewing the ESN of a Device',
            local: 'helpcenter000197.html'
          }
        ]
      },
      {
        id: 2425,
        parentId: 7,
        name: 'Managing Security Policies',
        local: 'admin_policy_000000.html'
      },
      {
        id: 2426,
        parentId: 7,
        name: 'Managing Certificates',
        local: 'admin_cer_000000.html',
        children: [
          {
            id: 2524,
            parentId: 2426,
            name: 'Viewing Certificate Information',
            local: 'admin_cer_000001.html'
          },
          {
            id: 2525,
            parentId: 2426,
            name: 'Adding an External Certificate',
            local: 'admin_cer_000002.html'
          },
          {
            id: 2526,
            parentId: 2426,
            name: 'Importing a Certificate',
            local: 'admin_cer_000003.html'
          },
          {
            id: 2527,
            parentId: 2426,
            name: 'Exporting Request Files',
            local: 'admin_cer_000004.html'
          },
          {
            id: 2528,
            parentId: 2426,
            name: 'Modifying Expiration Warning Days',
            local: 'admin_cer_000005.html'
          },
          {
            id: 2529,
            parentId: 2426,
            name:
              'Managing the Certificate Revocation List (Not Supported by E Series Appliances)',
            local: 'admin_cer_000007.html',
            children: [
              {
                id: 2532,
                parentId: 2529,
                name: 'Importing a CRL',
                local: 'admin_cer_000008.html'
              },
              {
                id: 2533,
                parentId: 2529,
                name: 'Viewing a CRL',
                local: 'admin_cer_000009.html'
              },
              {
                id: 2534,
                parentId: 2529,
                name: 'Downloading a CRL',
                local: 'admin_cer_000010.html'
              },
              {
                id: 2535,
                parentId: 2529,
                name: 'Deleting a CRL',
                local: 'admin_cer_000011.html'
              }
            ]
          },
          {
            id: 2530,
            parentId: 2426,
            name: 'Downloading a Certificate',
            local: 'admin_cer_000012.html'
          },
          {
            id: 2531,
            parentId: 2426,
            name: 'Deleting an External Certificate',
            local: 'admin_cer_000013.html'
          }
        ]
      },
      {
        id: 2427,
        parentId: 7,
        name: 'Host Trust Management',
        local: 'admin_host_000000.html'
      },
      {
        id: 2428,
        parentId: 7,
        name: 'Managing Logs',
        local: 'admin_log_000000.html'
      },
      {
        id: 2429,
        parentId: 7,
        name: 'Exporting Records',
        local: 'en-us_topic_0000001839144381.html'
      },
      {
        id: 2430,
        parentId: 7,
        name: 'Managing System Data Backup',
        local: 'admin_mngbackup_000000.html',
        children: [
          {
            id: 2536,
            parentId: 2430,
            name: 'Configuring Management Data Backup',
            local: 'admin_mngbackup_000001.html'
          },
          {
            id: 2537,
            parentId: 2430,
            name: 'Exporting a Management Data Backup',
            local: 'admin_mngbackup_000002.html'
          },
          {
            id: 2538,
            parentId: 2430,
            name: 'Deleting a Management Data Backup',
            local: 'admin_mngbackup_000003.html'
          },
          {
            id: 2539,
            parentId: 2430,
            name: 'Importing a Management Data Backup',
            local: 'admin_mngbackup_000004.html'
          },
          {
            id: 2540,
            parentId: 2430,
            name: 'Restoring Management Data',
            local: 'admin_mngbackup_000005.html'
          }
        ]
      },
      {
        id: 2431,
        parentId: 7,
        name: 'Managing the Email Service',
        local: 'admin_email_000000.html'
      },
      {
        id: 2432,
        parentId: 7,
        name:
          'Managing Event Dump (Applicable to OceanProtect X Series Backup Appliances)',
        local: 'admin_email_save_000000.html'
      },
      {
        id: 2433,
        parentId: 7,
        name: 'Managing SNMP Trap Notification',
        local: 'admin_snmp.html'
      },
      {
        id: 2434,
        parentId: 7,
        name:
          'Managing the SFTP Service (Applicable to 1.5.0) (Applicable to OceanProtect X Series Backup Appliances)',
        local: 'admin_sftp_000000.html',
        children: [
          {
            id: 2541,
            parentId: 2434,
            name: 'Enabling the SFTP Service',
            local: 'admin_sftp_000001.html'
          },
          {
            id: 2542,
            parentId: 2434,
            name: 'Viewing the SFTP Service',
            local: 'admin_sftp_000002.html'
          },
          {
            id: 2543,
            parentId: 2434,
            name: 'Creating an SFTP User',
            local: 'admin_sftp_000003.html'
          },
          {
            id: 2544,
            parentId: 2434,
            name: 'Changing SFTP User Password',
            local: 'admin_sftp_000004.html'
          },
          {
            id: 2545,
            parentId: 2434,
            name: 'Deleting an SFTP User',
            local: 'admin_sftp_000005.html'
          }
        ]
      },
      {
        id: 2435,
        parentId: 7,
        name:
          'Managing the SFTP Service (Applicable to 1.6.0 and Later Versions) (Applicable to OceanProtect X Series Backup Appliances)',
        local: 'admin-00261.html',
        children: [
          {
            id: 2546,
            parentId: 2435,
            name: 'Enabling the SFTP Service',
            local: 'admin-00262.html'
          },
          {
            id: 2547,
            parentId: 2435,
            name: 'Viewing the SFTP Service',
            local: 'admin-00263.html'
          },
          {
            id: 2548,
            parentId: 2435,
            name: 'Creating an SFTP User',
            local: 'admin-00264.html'
          },
          {
            id: 2549,
            parentId: 2435,
            name: 'Changing the Password of an SFTP User',
            local: 'admin-00265.html'
          },
          {
            id: 2550,
            parentId: 2435,
            name: 'Deleting an SFTP User',
            local: 'admin-00266.html'
          }
        ]
      },
      {
        id: 2436,
        parentId: 7,
        name: 'Managing the Device Time',
        local: 'admin_sys_time.html'
      },
      {
        id: 2437,
        parentId: 7,
        name: 'Configuring the LDAP Service',
        local: 'en-us_topic_0000001839144385.html'
      },
      {
        id: 2438,
        parentId: 7,
        name:
          'Managing Windows ADFS Configuration (Applicable to 1.6.0 and Later Versions)',
        local: 'admin-0077.html'
      },
      {
        id: 2439,
        parentId: 7,
        name:
          'Managing Backup Software Management (Applicable to 1.6.0 and Later Versions)',
        local: 'en-us_topic_0000001938830850.html',
        children: [
          {
            id: 2551,
            parentId: 2439,
            name: 'Adding Backup Software Management',
            local: 'en-us_topic_0000001965949181.html'
          },
          {
            id: 2552,
            parentId: 2439,
            name: 'Modifying Backup Software Management',
            local: 'en-us_topic_0000001966069409.html'
          },
          {
            id: 2553,
            parentId: 2439,
            name: 'Deleting Backup Software Management',
            local: 'en-us_topic_0000001938990182.html'
          }
        ]
      }
    ]
  }
];
topLanguage = 'en';
topMainPage = 'helpcenter000001.html';

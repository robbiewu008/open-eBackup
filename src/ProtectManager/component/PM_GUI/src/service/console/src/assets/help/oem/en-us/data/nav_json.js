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
        name: 'About the product',
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
                id: 24,
                parentId: 22,
                name: 'Viewing Agent Host Information',
                local: 'protectagent_install_0032.html'
              },
              {
                id: 25,
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
            id: 26,
            parentId: 13,
            name: 'Oracle Data Protection',
            local: 'product_documentation_000025.html',
            children: [
              {
                id: 43,
                parentId: 26,
                name: 'Backup',
                local: 'oracle_gud_0008.html',
                children: [
                  {
                    id: 52,
                    parentId: 43,
                    name: 'Preparations for Backup',
                    local: 'oracle_gud_0012.html'
                  },
                  {
                    id: 53,
                    parentId: 43,
                    name: 'Backing Up an Oracle Database',
                    local: 'oracle_gud_0013.html',
                    children: [
                      {
                        id: 55,
                        parentId: 53,
                        name:
                          'Step 1: Checking and Configuring the Database Environment',
                        local: 'oracle_gud_0015.html',
                        children: [
                          {
                            id: 64,
                            parentId: 55,
                            name:
                              'Checking and Configuring the Open State of the Oracle Database',
                            local: 'oracle_gud_0016.html'
                          },
                          {
                            id: 65,
                            parentId: 55,
                            name:
                              'Checking and Configuring the Archive Mode of the Oracle Database',
                            local: 'oracle_gud_0017.html'
                          },
                          {
                            id: 66,
                            parentId: 55,
                            name: 'Checking the Snapshot Control File Location',
                            local: 'oracle_gud_0020.html'
                          },
                          {
                            id: 67,
                            parentId: 55,
                            name:
                              'Checking the Storage Location of Cluster Data',
                            local: 'oracle_gud_00201.html'
                          }
                        ]
                      },
                      {
                        id: 56,
                        parentId: 53,
                        name:
                          'Step 2: Obtaining the CA Certificate of the Storage Resource (Applicable to Snapshot-based Backup at the Storage Layer)',
                        local: 'oracle_gud_ca.html'
                      },
                      {
                        id: 57,
                        parentId: 53,
                        name: 'Step 3: Registering a Cluster',
                        local: 'oracle_gud_0022.html'
                      },
                      {
                        id: 58,
                        parentId: 53,
                        name: 'Step 4: Registering a Database',
                        local: 'oracle_gud_0023.html'
                      },
                      {
                        id: 59,
                        parentId: 53,
                        name:
                          'Step 5: (Optional) Creating a Rate Limiting Policy',
                        local: 'oracle_gud_0024.html'
                      },
                      {
                        id: 60,
                        parentId: 53,
                        name:
                          'Step 6: (Optional) Enabling Backup Link Encryption',
                        local: 'oracle_gud_0025.html'
                      },
                      {
                        id: 61,
                        parentId: 53,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'oracle_gud_0026.html'
                      },
                      {
                        id: 62,
                        parentId: 53,
                        name:
                          'Step 8: Enabling BCT (Applicable to RMAN-based Backup)',
                        local: 'oracle_gud_0027.html'
                      },
                      {
                        id: 63,
                        parentId: 53,
                        name: 'Step 9: Performing Backup',
                        local: 'oracle_gud_0028.html'
                      }
                    ]
                  },
                  {
                    id: 54,
                    parentId: 43,
                    name:
                      '(Optional) Synchronizing Trap Configurations to an Oracle Host',
                    local: 'oracle_gud_0031.html'
                  }
                ]
              },
              {
                id: 44,
                parentId: 26,
                name: 'Replication',
                local: 'oracle_gud_0032.html',
                children: [
                  {
                    id: 68,
                    parentId: 44,
                    name: 'Replicating an Oracle Database Copy',
                    local: 'oracle_gud_0034.html',
                    children: [
                      {
                        id: 69,
                        parentId: 68,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'oracle_gud_0036.html'
                      },
                      {
                        id: 70,
                        parentId: 68,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'oracle_gud_0036_1.html'
                      },
                      {
                        id: 71,
                        parentId: 68,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'oracle_gud_0037.html'
                      },
                      {
                        id: 72,
                        parentId: 68,
                        name:
                          '(Optional) Step 3: Enabling Replication Link Encryption',
                        local: 'oracle_gud_0038.html'
                      },
                      {
                        id: 73,
                        parentId: 68,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'oracle_gud_0039.html'
                      },
                      {
                        id: 74,
                        parentId: 68,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'vmware_gud_0040.html'
                      },
                      {
                        id: 75,
                        parentId: 68,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002097717489.html'
                      },
                      {
                        id: 76,
                        parentId: 68,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'oracle_gud_0041.html'
                      },
                      {
                        id: 77,
                        parentId: 68,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'oracle_gud_0042.html'
                      },
                      {
                        id: 78,
                        parentId: 68,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'oracle_gud_0043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 45,
                parentId: 26,
                name: 'Archiving',
                local: 'oracle_gud_0044.html',
                children: [
                  {
                    id: 79,
                    parentId: 45,
                    name: 'Archiving Oracle Backup Copies',
                    local: 'oracle_gud_0047.html',
                    children: [
                      {
                        id: 81,
                        parentId: 79,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'oracle_gud_0048.html',
                        children: [
                          {
                            id: 83,
                            parentId: 81,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'oracle_gud_0049.html'
                          },
                          {
                            id: 84,
                            parentId: 81,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'oracle_gud_0050.html'
                          }
                        ]
                      },
                      {
                        id: 82,
                        parentId: 79,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'oracle_gud_0051.html'
                      }
                    ]
                  },
                  {
                    id: 80,
                    parentId: 45,
                    name: 'Archiving Oracle Replication Copies',
                    local: 'oracle_gud_0052.html',
                    children: [
                      {
                        id: 85,
                        parentId: 80,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'oracle_gud_0053.html'
                      },
                      {
                        id: 86,
                        parentId: 80,
                        name:
                          'Step 2: Creating Periodic Archives of Replication Copies',
                        local: 'oracle_gud_0054.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 46,
                parentId: 26,
                name: 'Restoration',
                local: 'oracle_gud_0055.html',
                children: [
                  {
                    id: 87,
                    parentId: 46,
                    name: 'Restoring an Oracle Database',
                    local: 'oracle_gud_0058.html'
                  },
                  {
                    id: 88,
                    parentId: 46,
                    name:
                      'Restoring a Single Table or Multiple Tables in an Oracle Database (Applicable to 1.6.0 and Later Versions)',
                    local: 'oracle_gud_0131.html'
                  },
                  {
                    id: 89,
                    parentId: 46,
                    name:
                      'Restoring a Single File or Multiple Files in an Oracle Database (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002028898929.html'
                  }
                ]
              },
              {
                id: 47,
                parentId: 26,
                name: 'Instant Recovery',
                local: 'oracle_gud_0059.html',
                children: [
                  {
                    id: 90,
                    parentId: 47,
                    name: 'Restoring an Oracle Database Instantly',
                    local: 'oracle_gud_0062.html'
                  }
                ]
              },
              {
                id: 48,
                parentId: 26,
                name: 'Global Search',
                local: 'oracle_gud_0072.html',
                children: [
                  {
                    id: 91,
                    parentId: 48,
                    name: 'Global Search for Resources',
                    local: 'oracle_gud_0073.html'
                  },
                  {
                    id: 92,
                    parentId: 48,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002038747309.html'
                  }
                ]
              },
              {
                id: 49,
                parentId: 26,
                name: 'SLA ',
                local: 'oracle_gud_0076.html',
                children: [
                  {
                    id: 93,
                    parentId: 49,
                    name: 'Viewing SLA Information',
                    local: 'oracle_gud_0078.html'
                  },
                  {
                    id: 94,
                    parentId: 49,
                    name: 'Managing SLAs',
                    local: 'oracle_gud_0079.html'
                  }
                ]
              },
              {
                id: 50,
                parentId: 26,
                name: 'Copies',
                local: 'oracle_gud_0080.html',
                children: [
                  {
                    id: 95,
                    parentId: 50,
                    name: 'Viewing Oracle Copy Information',
                    local: 'oracle_gud_0081.html'
                  },
                  {
                    id: 96,
                    parentId: 50,
                    name: 'Managing Oracle Copies',
                    local: 'oracle_gud_0082.html'
                  }
                ]
              },
              {
                id: 51,
                parentId: 26,
                name: 'Oracle Database Environment',
                local: 'oracle_gud_0083.html',
                children: [
                  {
                    id: 97,
                    parentId: 51,
                    name: 'Viewing the Oracle Database Environment',
                    local: 'oracle_gud_0084.html'
                  },
                  {
                    id: 98,
                    parentId: 51,
                    name: 'Managing Databases',
                    local: 'oracle_gud_0085.html'
                  },
                  {
                    id: 99,
                    parentId: 51,
                    name: 'Managing Database Clusters',
                    local: 'oracle_gud_0086.html'
                  }
                ]
              }
            ]
          },
          {
            id: 27,
            parentId: 13,
            name: 'MySQL/MariaDB/GreatSQL Data Protection',
            local: 'en-us_topic_0000001826879872.html',
            children: [
              {
                id: 100,
                parentId: 27,
                name: 'Backup',
                local: 'mysql-0007.html',
                children: [
                  {
                    id: 108,
                    parentId: 100,
                    name: 'Preparations for Backup',
                    local: 'mysql-0010.html'
                  },
                  {
                    id: 109,
                    parentId: 100,
                    name: 'Backing Up the MySQL/MariaDB/GreatSQL Database',
                    local: 'mysql-0011.html',
                    children: [
                      {
                        id: 110,
                        parentId: 109,
                        name:
                          'Step 1: Enabling MySQL/MariaDB/GreatSQL Database Permissions',
                        local: 'mysql-0012.html'
                      },
                      {
                        id: 111,
                        parentId: 109,
                        name: 'Step 2: Manually Configuring Soft Links',
                        local: 'mysql-0016.html'
                      },
                      {
                        id: 112,
                        parentId: 109,
                        name: 'Step 3: Manually Installing the Backup Tool',
                        local: 'mysql-0013.html',
                        children: [
                          {
                            id: 119,
                            parentId: 112,
                            name: 'Installing Mariabackup',
                            local: 'mysql-0014.html'
                          },
                          {
                            id: 120,
                            parentId: 112,
                            name:
                              'Installing the Software on Which the Percona XtraBackup Tool Depends',
                            local: 'mysql-0015.html'
                          }
                        ]
                      },
                      {
                        id: 113,
                        parentId: 109,
                        name:
                          'Step 4: Enabling Log Mode for the MySQL/MariaDB/GreatSQL Database',
                        local: 'mysql-0017.html'
                      },
                      {
                        id: 114,
                        parentId: 109,
                        name:
                          'Step 5: Registering a MySQL/MariaDB/GreatSQL Database',
                        local: 'mysql-0018.html'
                      },
                      {
                        id: 115,
                        parentId: 109,
                        name: 'Step 6: Creating a Rate Limiting Policy',
                        local: 'mysql-0019.html'
                      },
                      {
                        id: 116,
                        parentId: 109,
                        name:
                          'Step 7: (Optional) Enabling Backup Link Encryption',
                        local: 'mysql-0020.html'
                      },
                      {
                        id: 117,
                        parentId: 109,
                        name: 'Step 8: Creating a Backup SLA',
                        local: 'mysql-0021.html'
                      },
                      {
                        id: 118,
                        parentId: 109,
                        name: 'Step 9: Performing Backup',
                        local: 'mysql-0022.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 101,
                parentId: 27,
                name: 'Replication',
                local: 'mysql-00241.html',
                children: [
                  {
                    id: 121,
                    parentId: 101,
                    name: 'Replicating MySQL/MariaDB/GreatSQL Database Copies',
                    local: 'mysql-0028.html',
                    children: [
                      {
                        id: 122,
                        parentId: 121,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'mysql-00291.html'
                      },
                      {
                        id: 123,
                        parentId: 121,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'mysql-00292.html'
                      },
                      {
                        id: 124,
                        parentId: 121,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'mysql-0031.html'
                      },
                      {
                        id: 125,
                        parentId: 121,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'mysql-0032.html'
                      },
                      {
                        id: 126,
                        parentId: 121,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'mysql-0033.html'
                      },
                      {
                        id: 127,
                        parentId: 121,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'mysql-0034.html'
                      },
                      {
                        id: 128,
                        parentId: 121,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'mysql-00311.html'
                      },
                      {
                        id: 129,
                        parentId: 121,
                        name: 'Step: Adding a Target Cluster',
                        local: 'mysql-0035.html'
                      },
                      {
                        id: 130,
                        parentId: 121,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'mysql-0036.html'
                      },
                      {
                        id: 131,
                        parentId: 121,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'mysql-0037.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 102,
                parentId: 27,
                name: 'Archiving',
                local: 'mysql-0038.html',
                children: [
                  {
                    id: 132,
                    parentId: 102,
                    name: 'Archiving MySQL/MariaDB/GreatSQL Backup Copies',
                    local: 'mysql-0041.html',
                    children: [
                      {
                        id: 134,
                        parentId: 132,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'mysql-0042.html',
                        children: [
                          {
                            id: 136,
                            parentId: 134,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'mysql-0043.html'
                          },
                          {
                            id: 137,
                            parentId: 134,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'mysql-0044.html'
                          }
                        ]
                      },
                      {
                        id: 135,
                        parentId: 132,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'mysql-0045.html'
                      }
                    ]
                  },
                  {
                    id: 133,
                    parentId: 102,
                    name: 'Archiving MySQL/MariaDB/GreatSQL Replication Copies',
                    local: 'mysql-0046.html',
                    children: [
                      {
                        id: 138,
                        parentId: 133,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'mysql-0047.html'
                      },
                      {
                        id: 139,
                        parentId: 133,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'mysql-0048.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 103,
                parentId: 27,
                name: 'Restoration',
                local: 'mysql-0049.html',
                children: [
                  {
                    id: 140,
                    parentId: 103,
                    name: 'Restoring MySQL/MariaDB/GreatSQL Databases',
                    local: 'mysql-0052.html'
                  }
                ]
              },
              {
                id: 104,
                parentId: 27,
                name: 'Global Search',
                local: 'mysql-0053.html',
                children: [
                  {
                    id: 141,
                    parentId: 104,
                    name: 'Global Search for Resources',
                    local: 'mysql-0054.html'
                  },
                  {
                    id: 142,
                    parentId: 104,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'mysql-00522.html'
                  }
                ]
              },
              {
                id: 105,
                parentId: 27,
                name: 'SLA ',
                local: 'mysql-0070.html',
                children: [
                  {
                    id: 143,
                    parentId: 105,
                    name: 'About SLA',
                    local: 'mysql-0071.html'
                  },
                  {
                    id: 144,
                    parentId: 105,
                    name: 'Viewing SLA Information',
                    local: 'mysql-0072.html'
                  },
                  {
                    id: 145,
                    parentId: 105,
                    name: 'Managing SLAs',
                    local: 'mysql-0073.html'
                  }
                ]
              },
              {
                id: 106,
                parentId: 27,
                name: 'Copies',
                local: 'mysql-0074.html',
                children: [
                  {
                    id: 146,
                    parentId: 106,
                    name: 'Viewing MySQL/MariaDB/GreatSQL Copy Information',
                    local: 'mysql-0075.html'
                  },
                  {
                    id: 147,
                    parentId: 106,
                    name: 'Managing MySQL/MariaDB/GreatSQL Copies',
                    local: 'mysql-0076.html'
                  }
                ]
              },
              {
                id: 107,
                parentId: 27,
                name: 'MySQL/MariaDB/GreatSQL Database Environment',
                local: 'mysql-0077.html',
                children: [
                  {
                    id: 148,
                    parentId: 107,
                    name:
                      'Viewing MySQL/MariaDB/GreatSQL Database Environment Information',
                    local: 'mysql-0078.html'
                  },
                  {
                    id: 149,
                    parentId: 107,
                    name: 'Managing Databases',
                    local: 'mysql-0079.html'
                  },
                  {
                    id: 150,
                    parentId: 107,
                    name: 'Managing Database Instances',
                    local: 'mysql-00790.html'
                  },
                  {
                    id: 151,
                    parentId: 107,
                    name: 'Managing Database Clusters',
                    local: 'mysql-0080.html'
                  }
                ]
              }
            ]
          },
          {
            id: 28,
            parentId: 13,
            name: 'SQL Server Data Protection',
            local: 'en-us_topic_0000001826879832.html',
            children: [
              {
                id: 152,
                parentId: 28,
                name: 'Backup',
                local: 'sql-0005.html',
                children: [
                  {
                    id: 160,
                    parentId: 152,
                    name: 'Preparations for Backup',
                    local: 'sql-0008.html'
                  },
                  {
                    id: 161,
                    parentId: 152,
                    name: 'Backing Up a SQL Server Database',
                    local: 'sql-0009.html',
                    children: [
                      {
                        id: 162,
                        parentId: 161,
                        name:
                          'Step 1: Checking and Configuring the Database Environment',
                        local: 'sql-0010.html'
                      },
                      {
                        id: 163,
                        parentId: 161,
                        name: 'Step 2: Setting Windows PowerShell Permissions',
                        local: 'sql-0011.html'
                      },
                      {
                        id: 164,
                        parentId: 161,
                        name: 'Step 4: Enabling the sysadmin Permission',
                        local: 'sql-0013.html'
                      },
                      {
                        id: 165,
                        parentId: 161,
                        name:
                          'Step 5: Setting the Recovery Model for Log Backup',
                        local: 'sql-0014.html'
                      },
                      {
                        id: 166,
                        parentId: 161,
                        name: 'Step 3: Registering the SQL Server Database',
                        local: 'sql-0012.html'
                      },
                      {
                        id: 167,
                        parentId: 161,
                        name: 'Step 6: Creating a Rate Limiting Policy',
                        local: 'sql-0015.html'
                      },
                      {
                        id: 168,
                        parentId: 161,
                        name:
                          'Step 7: (Optional) Enabling Backup Link Encryption',
                        local: 'en-us_topic_0000002051554157.html'
                      },
                      {
                        id: 169,
                        parentId: 161,
                        name: 'Step 8: Creating a Backup SLA',
                        local: 'sql-0017.html'
                      },
                      {
                        id: 170,
                        parentId: 161,
                        name: 'Step 9: Performing Backup',
                        local: 'sql-0018.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 153,
                parentId: 28,
                name: 'Replication',
                local: 'oracle_gud_000035_9.html',
                children: [
                  {
                    id: 171,
                    parentId: 153,
                    name: 'Replicating SQL Server Database Copies',
                    local: 'sql-0023.html',
                    children: [
                      {
                        id: 172,
                        parentId: 171,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_9.html'
                      },
                      {
                        id: 173,
                        parentId: 171,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_14.html'
                      },
                      {
                        id: 174,
                        parentId: 171,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'sql-0026.html'
                      },
                      {
                        id: 175,
                        parentId: 171,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'sql-0027.html'
                      },
                      {
                        id: 176,
                        parentId: 171,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'sql-0028.html'
                      },
                      {
                        id: 177,
                        parentId: 171,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'sql-0029.html'
                      },
                      {
                        id: 178,
                        parentId: 171,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002010400808.html'
                      },
                      {
                        id: 179,
                        parentId: 171,
                        name: 'Step : Adding a Target Cluster',
                        local: 'sql-0030.html'
                      },
                      {
                        id: 180,
                        parentId: 171,
                        name: 'Step : Creating a Replication SLA',
                        local: 'sql-0031.html'
                      },
                      {
                        id: 181,
                        parentId: 171,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'sql-0032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 154,
                parentId: 28,
                name: 'Archiving',
                local: 'sql-0033.html',
                children: [
                  {
                    id: 182,
                    parentId: 154,
                    name: 'Archiving SQL Server Backup Copies',
                    local: 'sql-0036.html',
                    children: [
                      {
                        id: 184,
                        parentId: 182,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'sql-0037.html',
                        children: [
                          {
                            id: 186,
                            parentId: 184,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'sql-0038.html'
                          },
                          {
                            id: 187,
                            parentId: 184,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'sql-0039.html'
                          }
                        ]
                      },
                      {
                        id: 185,
                        parentId: 182,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'sql-0040.html'
                      }
                    ]
                  },
                  {
                    id: 183,
                    parentId: 154,
                    name: 'Archiving SQL Server Replication Copies',
                    local: 'sql-0041.html',
                    children: [
                      {
                        id: 188,
                        parentId: 183,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'sql-0042.html'
                      },
                      {
                        id: 189,
                        parentId: 183,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'sql-0043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 155,
                parentId: 28,
                name: 'Restoration',
                local: 'sql-0044.html',
                children: [
                  {
                    id: 190,
                    parentId: 155,
                    name: 'Restoring a SQL Server Database',
                    local: 'sql-0047.html'
                  },
                  {
                    id: 191,
                    parentId: 155,
                    name:
                      'Restoring One or More Databases in a SQL Server Instance',
                    local: 'sql-0152.html'
                  }
                ]
              },
              {
                id: 156,
                parentId: 28,
                name: 'Global Search',
                local: 'sql-0048.html',
                children: [
                  {
                    id: 192,
                    parentId: 156,
                    name: 'Global Search for Resources',
                    local: 'sql-0049.html'
                  },
                  {
                    id: 193,
                    parentId: 156,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002678126.html'
                  }
                ]
              },
              {
                id: 157,
                parentId: 28,
                name: 'SLA ',
                local: 'sql-0052.html',
                children: [
                  {
                    id: 194,
                    parentId: 157,
                    name: 'Viewing SLA Information',
                    local: 'sql-0054.html'
                  },
                  {
                    id: 195,
                    parentId: 157,
                    name: 'Managing SLAs',
                    local: 'sql-0055.html'
                  }
                ]
              },
              {
                id: 158,
                parentId: 28,
                name: 'Copies',
                local: 'sql-0056.html',
                children: [
                  {
                    id: 196,
                    parentId: 158,
                    name: 'Viewing SQL Server Copy Information',
                    local: 'sql-0057.html'
                  },
                  {
                    id: 197,
                    parentId: 158,
                    name: 'Managing SQL Server Database Copies',
                    local: 'sql-0058.html'
                  }
                ]
              },
              {
                id: 159,
                parentId: 28,
                name: 'SQL Server Database Environment',
                local: 'sql-0059.html',
                children: [
                  {
                    id: 198,
                    parentId: 159,
                    name:
                      'Viewing Environment Information of the SQL Server Database',
                    local: 'sql-0060.html'
                  },
                  {
                    id: 199,
                    parentId: 159,
                    name: 'Managing SQL Server',
                    local: 'sql-0061.html'
                  },
                  {
                    id: 200,
                    parentId: 159,
                    name: 'Managing SQL Server Database Clusters',
                    local: 'sql-0062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 29,
            parentId: 13,
            name: 'PostgreSQL Data Protection',
            local: 'en-us_topic_0000001826879840.html',
            children: [
              {
                id: 201,
                parentId: 29,
                name: 'Backup',
                local: 'postgresql-0006_a1.html',
                children: [
                  {
                    id: 209,
                    parentId: 201,
                    name: 'Preparations for Backup',
                    local: 'postgresql-0009_a1.html'
                  },
                  {
                    id: 210,
                    parentId: 201,
                    name: 'Backing Up PostgreSQL',
                    local: 'postgresql-0010-2.html',
                    children: [
                      {
                        id: 211,
                        parentId: 210,
                        name:
                          'Step 1: Checking and Enabling the sudo Permission of the PostgreSQL Database Installation User',
                        local: 'en-us_topic_0000001951390817.html'
                      },
                      {
                        id: 212,
                        parentId: 210,
                        name: 'Step 2: Enabling the Archive Mode',
                        local: 'postgresql-0010_0.html'
                      },
                      {
                        id: 213,
                        parentId: 210,
                        name:
                          'Step 3: Registering the Database of a Single PostgreSQL Instance',
                        local: 'postgresql-0011.html'
                      },
                      {
                        id: 214,
                        parentId: 210,
                        name:
                          'Step 4: Registering the Database of a PostgreSQL Cluster Instance',
                        local: 'postgresql-0012.html'
                      },
                      {
                        id: 215,
                        parentId: 210,
                        name: 'Step 5: Creating a Rate Limiting Policy',
                        local: 'postgresql-0013.html'
                      },
                      {
                        id: 216,
                        parentId: 210,
                        name:
                          'Step 6: (Optional) Enabling Backup Link Encryption',
                        local: 'postgresql-0014.html'
                      },
                      {
                        id: 217,
                        parentId: 210,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'postgresql-0015_a1.html'
                      },
                      {
                        id: 218,
                        parentId: 210,
                        name: 'Step 8: Performing Backup',
                        local: 'postgresql-0016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 202,
                parentId: 29,
                name: 'Replication',
                local: 'oracle_gud_000035_11.html',
                children: [
                  {
                    id: 219,
                    parentId: 202,
                    name: 'Replicating PostgreSQL Database Copies',
                    local: 'postgresql-0021.html',
                    children: [
                      {
                        id: 220,
                        parentId: 219,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'postgresql-0071_a1.html'
                      },
                      {
                        id: 221,
                        parentId: 219,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'postgresql-0072_a2.html'
                      },
                      {
                        id: 222,
                        parentId: 219,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'postgresql-0024.html'
                      },
                      {
                        id: 223,
                        parentId: 219,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'postgresql-0025.html'
                      },
                      {
                        id: 224,
                        parentId: 219,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'postgresql-0026.html'
                      },
                      {
                        id: 225,
                        parentId: 219,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'postgresql-0027.html'
                      },
                      {
                        id: 226,
                        parentId: 219,
                        name:
                          'Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'postgresql-0027_a1.html'
                      },
                      {
                        id: 227,
                        parentId: 219,
                        name: 'Step : Adding a Target Cluster',
                        local: 'postgresql-0028.html'
                      },
                      {
                        id: 228,
                        parentId: 219,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'postgresql-0033_a1.html'
                      },
                      {
                        id: 229,
                        parentId: 219,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'postgresql-0030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 203,
                parentId: 29,
                name: 'Archiving',
                local: 'postgresql-0031.html',
                children: [
                  {
                    id: 230,
                    parentId: 203,
                    name: 'Archiving PostgreSQL Backup Copies',
                    local: 'postgresql-0034.html',
                    children: [
                      {
                        id: 232,
                        parentId: 230,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'postgresql-0035.html',
                        children: [
                          {
                            id: 234,
                            parentId: 232,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'postgresql-0036.html'
                          },
                          {
                            id: 235,
                            parentId: 232,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'postgresql-0037.html'
                          }
                        ]
                      },
                      {
                        id: 233,
                        parentId: 230,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'postgresql-0038.html'
                      }
                    ]
                  },
                  {
                    id: 231,
                    parentId: 203,
                    name: 'Archiving PostgreSQL Replication Copies',
                    local: 'postgresql-0043_a1.html',
                    children: [
                      {
                        id: 236,
                        parentId: 231,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'postgresql-0044_a1.html'
                      },
                      {
                        id: 237,
                        parentId: 231,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'postgresql-0045_a1.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 204,
                parentId: 29,
                name: 'Restoration',
                local: 'postgresql-0042.html',
                children: [
                  {
                    id: 238,
                    parentId: 204,
                    name: 'Restoring PostgreSQL',
                    local: 'postgresql-0045.html'
                  }
                ]
              },
              {
                id: 205,
                parentId: 29,
                name: 'Global Search',
                local: 'postgresql-0027_a2.html',
                children: [
                  {
                    id: 239,
                    parentId: 205,
                    name: 'Global Search for Resources',
                    local: 'postgresql-0027_a3.html'
                  },
                  {
                    id: 240,
                    parentId: 205,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'postgresql-0027_a4.html'
                  }
                ]
              },
              {
                id: 206,
                parentId: 29,
                name: 'SLA ',
                local: 'postgresql-0050.html',
                children: [
                  {
                    id: 241,
                    parentId: 206,
                    name: 'About SLA',
                    local: 'postgresql-0055_a1.html'
                  },
                  {
                    id: 242,
                    parentId: 206,
                    name: 'Viewing SLA Information',
                    local: 'postgresql-0056_1q.html'
                  },
                  {
                    id: 243,
                    parentId: 206,
                    name: 'Managing SLAs',
                    local: 'postgresql-0053.html'
                  }
                ]
              },
              {
                id: 207,
                parentId: 29,
                name: 'Copies',
                local: 'postgresql-0058_a1.html',
                children: [
                  {
                    id: 244,
                    parentId: 207,
                    name: 'Viewing PostgreSQL Copy Information',
                    local: 'postgresql-0055.html'
                  },
                  {
                    id: 245,
                    parentId: 207,
                    name: 'Managing PostgreSQL Copies',
                    local: 'postgresql-0056.html'
                  }
                ]
              },
              {
                id: 208,
                parentId: 29,
                name: 'PostgreSQL Cluster Environment',
                local: 'postgresql-0057.html',
                children: [
                  {
                    id: 246,
                    parentId: 208,
                    name: 'Checking the PostgreSQL Environment',
                    local: 'postgresql-0058.html'
                  },
                  {
                    id: 247,
                    parentId: 208,
                    name: 'Managing PostgreSQL',
                    local: 'postgresql-0059.html'
                  },
                  {
                    id: 248,
                    parentId: 208,
                    name: 'Managing PostgreSQL Database Clusters',
                    local: 'postgresql-0060.html'
                  }
                ]
              }
            ]
          },
          {
            id: 30,
            parentId: 13,
            name: 'DB2 Data Protection',
            local: 'en-us_topic_0000001873759405.html',
            children: [
              {
                id: 249,
                parentId: 30,
                name: 'Backup',
                local: 'DB2-00007.html',
                children: [
                  {
                    id: 257,
                    parentId: 249,
                    name: 'Preparations for Backup',
                    local: 'DB2-00010.html'
                  },
                  {
                    id: 258,
                    parentId: 249,
                    name: 'Backing Up the DB2 Database and Tablespace Set',
                    local: 'DB2-00011.html',
                    children: [
                      {
                        id: 259,
                        parentId: 258,
                        name: 'Step 1: Registering the DB2 Database',
                        local: 'DB2-00012.html'
                      },
                      {
                        id: 260,
                        parentId: 258,
                        name: 'Step 2: Creating a DB2 Tablespace Set',
                        local: 'DB2-00013.html'
                      },
                      {
                        id: 261,
                        parentId: 258,
                        name: 'Step 3: Creating a Rate Limiting Policy',
                        local: 'DB2-00014.html'
                      },
                      {
                        id: 262,
                        parentId: 258,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'DB2-00015.html'
                      },
                      {
                        id: 263,
                        parentId: 258,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'DB2-00016.html'
                      },
                      {
                        id: 264,
                        parentId: 258,
                        name: 'Step 6: Performing Backup',
                        local: 'DB2-00017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 250,
                parentId: 30,
                name: 'Replication',
                local: 'oracle_gud_000035_13.html',
                children: [
                  {
                    id: 265,
                    parentId: 250,
                    name: 'DB2 Copy Replication',
                    local: 'DB2-00023.html',
                    children: [
                      {
                        id: 266,
                        parentId: 265,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_13.html'
                      },
                      {
                        id: 267,
                        parentId: 265,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_20.html'
                      },
                      {
                        id: 268,
                        parentId: 265,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'DB2-00026.html'
                      },
                      {
                        id: 269,
                        parentId: 265,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'DB2-00027.html'
                      },
                      {
                        id: 270,
                        parentId: 265,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'DB2-00028.html'
                      },
                      {
                        id: 271,
                        parentId: 265,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'DB2-00029.html'
                      },
                      {
                        id: 272,
                        parentId: 265,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'DB2-00024_a1.html'
                      },
                      {
                        id: 273,
                        parentId: 265,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'DB2-00030.html'
                      },
                      {
                        id: 274,
                        parentId: 265,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'DB2-00031.html'
                      },
                      {
                        id: 275,
                        parentId: 265,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'DB2-00032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 251,
                parentId: 30,
                name: 'Archiving',
                local: 'DB2-00033.html',
                children: [
                  {
                    id: 276,
                    parentId: 251,
                    name: 'Archiving DB2 Backup Copies',
                    local: 'DB2-00036.html',
                    children: [
                      {
                        id: 278,
                        parentId: 276,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'DB2-00037.html',
                        children: [
                          {
                            id: 280,
                            parentId: 278,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'DB2-00038.html'
                          },
                          {
                            id: 281,
                            parentId: 278,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'DB2-00039.html'
                          }
                        ]
                      },
                      {
                        id: 279,
                        parentId: 276,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'DB2-00040.html'
                      }
                    ]
                  },
                  {
                    id: 277,
                    parentId: 251,
                    name: 'Archiving DB2 Replication Copies',
                    local: 'DB2-00041.html',
                    children: [
                      {
                        id: 282,
                        parentId: 277,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'DB2-00042.html'
                      },
                      {
                        id: 283,
                        parentId: 277,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'DB2-00043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 252,
                parentId: 30,
                name: 'Restoration',
                local: 'DB2-00044.html',
                children: [
                  {
                    id: 284,
                    parentId: 252,
                    name: 'Restoring DB2 Databases or Tablespace Sets',
                    local: 'DB2-00047.html'
                  }
                ]
              },
              {
                id: 253,
                parentId: 30,
                name: 'Global Search',
                local: 'DB2-00039_a1.html',
                children: [
                  {
                    id: 285,
                    parentId: 253,
                    name: 'Global Search for Resources',
                    local: 'DB2-00039_a2.html'
                  },
                  {
                    id: 286,
                    parentId: 253,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'DB2-00039_a3.html'
                  }
                ]
              },
              {
                id: 254,
                parentId: 30,
                name: 'SLA ',
                local: 'DB2-00051.html',
                children: [
                  {
                    id: 287,
                    parentId: 254,
                    name: 'About SLA',
                    local: 'DB2-00052.html'
                  },
                  {
                    id: 288,
                    parentId: 254,
                    name: 'Viewing SLA Information',
                    local: 'DB2-00053.html'
                  },
                  {
                    id: 289,
                    parentId: 254,
                    name: 'Managing SLAs',
                    local: 'DB2-00054.html'
                  }
                ]
              },
              {
                id: 255,
                parentId: 30,
                name: 'Copies',
                local: 'DB2-00055.html',
                children: [
                  {
                    id: 290,
                    parentId: 255,
                    name: 'Viewing DB2 Copy Information',
                    local: 'DB2-00056.html'
                  },
                  {
                    id: 291,
                    parentId: 255,
                    name: 'Managing DB2 Copies',
                    local: 'DB2-00057.html'
                  }
                ]
              },
              {
                id: 256,
                parentId: 30,
                name: 'DB2 Cluster Environment',
                local: 'DB2-00058.html',
                children: [
                  {
                    id: 292,
                    parentId: 256,
                    name: 'Querying DB2 Information',
                    local: 'DB2-00059.html'
                  },
                  {
                    id: 293,
                    parentId: 256,
                    name: 'Managing DB2 Clusters or Tablespace Sets',
                    local: 'DB2-00060.html'
                  },
                  {
                    id: 294,
                    parentId: 256,
                    name: 'Managing DB2 Databases or Tablespace Sets',
                    local: 'DB2-00061.html'
                  }
                ]
              }
            ]
          },
          {
            id: 31,
            parentId: 13,
            name: 'Informix/GBase 8s Data Protection',
            local: 'en-us_topic_0000001873759417.html',
            children: [
              {
                id: 295,
                parentId: 31,
                name: 'Backup',
                local: 'informix-0007.html',
                children: [
                  {
                    id: 303,
                    parentId: 295,
                    name: 'Backing Up Informix/GBase 8s',
                    local: 'informix-0010.html',
                    children: [
                      {
                        id: 304,
                        parentId: 303,
                        name: 'Step 1: Configuring the XBSA Library Path',
                        local: 'informix-0011.html'
                      },
                      {
                        id: 305,
                        parentId: 303,
                        name:
                          'Step 2: Registering an Informix/GBase 8s Cluster',
                        local: 'informix-0012.html'
                      },
                      {
                        id: 306,
                        parentId: 303,
                        name:
                          'Step 3: Registering an Informix/GBase 8s Instance',
                        local: 'informix-0013.html'
                      },
                      {
                        id: 307,
                        parentId: 303,
                        name: 'Step 4: Creating a Rate Limiting Policy',
                        local: 'informix-0014.html'
                      },
                      {
                        id: 308,
                        parentId: 303,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'informix-0015.html'
                      },
                      {
                        id: 309,
                        parentId: 303,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'informix-0016.html'
                      },
                      {
                        id: 310,
                        parentId: 303,
                        name: 'Step 7: Performing Backup',
                        local: 'informix-0017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 296,
                parentId: 31,
                name: 'Replication',
                local: 'oracle_gud_000035.html',
                children: [
                  {
                    id: 311,
                    parentId: 296,
                    name: 'Replicating Informix/GBase 8s Database Copies',
                    local: 'informix-0023.html',
                    children: [
                      {
                        id: 312,
                        parentId: 311,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026.html'
                      },
                      {
                        id: 313,
                        parentId: 311,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1.html'
                      },
                      {
                        id: 314,
                        parentId: 311,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'informix-0026.html'
                      },
                      {
                        id: 315,
                        parentId: 311,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'informix-0027.html'
                      },
                      {
                        id: 316,
                        parentId: 311,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'informix-0028.html'
                      },
                      {
                        id: 317,
                        parentId: 311,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'informix-0029.html'
                      },
                      {
                        id: 318,
                        parentId: 311,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002046568669.html'
                      },
                      {
                        id: 319,
                        parentId: 311,
                        name: 'Step : Adding a Target Cluster',
                        local: 'informix-0030.html'
                      },
                      {
                        id: 320,
                        parentId: 311,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'informix-0031.html'
                      },
                      {
                        id: 321,
                        parentId: 311,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'informix-0032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 297,
                parentId: 31,
                name: 'Archiving',
                local: 'informix-0033.html',
                children: [
                  {
                    id: 322,
                    parentId: 297,
                    name: 'Archiving Informix/GBase 8s Backup Copies',
                    local: 'informix-0036.html',
                    children: [
                      {
                        id: 324,
                        parentId: 322,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'informix-0037.html',
                        children: [
                          {
                            id: 326,
                            parentId: 324,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'informix-0038.html'
                          },
                          {
                            id: 327,
                            parentId: 324,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'informix-0039.html'
                          }
                        ]
                      },
                      {
                        id: 325,
                        parentId: 322,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'informix-0040.html'
                      }
                    ]
                  },
                  {
                    id: 323,
                    parentId: 297,
                    name: 'Archiving Informix/GBase 8s Replication Copies',
                    local: 'informix-0041.html',
                    children: [
                      {
                        id: 328,
                        parentId: 323,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'informix-0042.html'
                      },
                      {
                        id: 329,
                        parentId: 323,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'informix-0043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 298,
                parentId: 31,
                name: 'Restoration',
                local: 'informix-0044.html',
                children: [
                  {
                    id: 330,
                    parentId: 298,
                    name: 'Restoring Informix/GBase 8s',
                    local: 'informix-0047.html'
                  }
                ]
              },
              {
                id: 299,
                parentId: 31,
                name: 'Global Search',
                local: 'informix-0048.html',
                children: [
                  {
                    id: 331,
                    parentId: 299,
                    name: 'Global Search for Resources',
                    local: 'informix-0049.html'
                  },
                  {
                    id: 332,
                    parentId: 299,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002038759717.html'
                  }
                ]
              },
              {
                id: 300,
                parentId: 31,
                name: 'SLA ',
                local: 'informix-0052.html',
                children: [
                  {
                    id: 333,
                    parentId: 300,
                    name: 'About SLA ',
                    local: 'informix-0053.html'
                  },
                  {
                    id: 334,
                    parentId: 300,
                    name: 'Viewing SLA Information',
                    local: 'informix-0054.html'
                  },
                  {
                    id: 335,
                    parentId: 300,
                    name: 'Managing SLAs',
                    local: 'informix-0055.html'
                  }
                ]
              },
              {
                id: 301,
                parentId: 31,
                name: 'Copies',
                local: 'informix-0056.html',
                children: [
                  {
                    id: 336,
                    parentId: 301,
                    name: 'Viewing Informix/GBase 8s Copy Information',
                    local: 'informix-0057.html'
                  },
                  {
                    id: 337,
                    parentId: 301,
                    name: 'Managing Informix/GBase 8s Copies',
                    local: 'informix-0058.html'
                  }
                ]
              },
              {
                id: 302,
                parentId: 31,
                name: 'Informix/GBase 8s Cluster Environment',
                local: 'informix-0059.html',
                children: [
                  {
                    id: 338,
                    parentId: 302,
                    name:
                      'Viewing the Informix/GBase 8s Environment Information',
                    local: 'informix-0060.html'
                  },
                  {
                    id: 339,
                    parentId: 302,
                    name: 'Managing Informix/GBase 8s',
                    local: 'informix-0061.html'
                  },
                  {
                    id: 340,
                    parentId: 302,
                    name: 'Managing Informix/GBase 8s Database Clusters',
                    local: 'informix-0062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 32,
            parentId: 13,
            name: 'openGauss Data Protection',
            local: 'en-us_topic_0000001873679197.html',
            children: [
              {
                id: 341,
                parentId: 32,
                name: 'Backup',
                local: 'opengauss-0107.html',
                children: [
                  {
                    id: 349,
                    parentId: 341,
                    name: 'Preparations for Backup',
                    local: 'opengauss-0110.html'
                  },
                  {
                    id: 350,
                    parentId: 341,
                    name: 'Backing Up openGauss/CMDB',
                    local: 'opengauss-0111.html',
                    children: [
                      {
                        id: 351,
                        parentId: 350,
                        name:
                          'Step1: Enabling the Archive Mode (Applicable Only to CMDB)',
                        local: 'postgresql-0010.html'
                      },
                      {
                        id: 352,
                        parentId: 350,
                        name: 'Step 2: Registering an openGauss/CMDB Cluster',
                        local: 'opengauss-0112.html'
                      },
                      {
                        id: 353,
                        parentId: 350,
                        name: 'Step 3: Creating a Rate Limiting Policy',
                        local: 'opengauss-0113.html'
                      },
                      {
                        id: 354,
                        parentId: 350,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'opengauss-0114.html'
                      },
                      {
                        id: 355,
                        parentId: 350,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'opengauss-0115.html'
                      },
                      {
                        id: 356,
                        parentId: 350,
                        name: 'Step 6: Performing Backup',
                        local: 'opengauss-0116.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 342,
                parentId: 32,
                name: 'Replication',
                local: 'opengauss-0119.html',
                children: [
                  {
                    id: 357,
                    parentId: 342,
                    name: 'Replicating openGauss/CMDB Database Copies',
                    local: 'opengauss-0122.html',
                    children: [
                      {
                        id: 358,
                        parentId: 357,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'opengauss-0124.html'
                      },
                      {
                        id: 359,
                        parentId: 357,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'opengauss-0125.html'
                      },
                      {
                        id: 360,
                        parentId: 357,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'opengauss-0126.html'
                      },
                      {
                        id: 361,
                        parentId: 357,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'opengauss-0127.html'
                      },
                      {
                        id: 362,
                        parentId: 357,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'opengauss-0128.html'
                      },
                      {
                        id: 363,
                        parentId: 357,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'opengauss-0129.html'
                      },
                      {
                        id: 364,
                        parentId: 357,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'opengauss-0026_a1.html'
                      },
                      {
                        id: 365,
                        parentId: 357,
                        name: 'Step : Adding a Target Cluster',
                        local: 'opengauss-0130.html'
                      },
                      {
                        id: 366,
                        parentId: 357,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'opengauss-0131.html'
                      },
                      {
                        id: 367,
                        parentId: 357,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA ',
                        local: 'opengauss-0132.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 343,
                parentId: 32,
                name: 'Archiving',
                local: 'opengauss-0133.html',
                children: [
                  {
                    id: 368,
                    parentId: 343,
                    name: 'Archiving openGauss/CMDB Backup Copies',
                    local: 'opengauss-0136.html',
                    children: [
                      {
                        id: 370,
                        parentId: 368,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'opengauss-0137.html',
                        children: [
                          {
                            id: 372,
                            parentId: 370,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'opengauss-0138.html'
                          },
                          {
                            id: 373,
                            parentId: 370,
                            name:
                              'Creating a Media Set (When Archive Storage Is a Tape Library)',
                            local: 'opengauss-0139.html'
                          }
                        ]
                      },
                      {
                        id: 371,
                        parentId: 368,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'opengauss-0140.html'
                      }
                    ]
                  },
                  {
                    id: 369,
                    parentId: 343,
                    name: 'Archiving openGauss/CMDB Replication Copies',
                    local: 'opengauss-0141.html',
                    children: [
                      {
                        id: 374,
                        parentId: 369,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'opengauss-0142.html'
                      },
                      {
                        id: 375,
                        parentId: 369,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'opengauss-0143.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 344,
                parentId: 32,
                name: 'Restoration',
                local: 'opengauss-0144.html',
                children: [
                  {
                    id: 376,
                    parentId: 344,
                    name: 'Restoring openGauss/CMDB',
                    local: 'opengauss-0147.html'
                  }
                ]
              },
              {
                id: 345,
                parentId: 32,
                name: 'Global Search',
                local: 'opengauss-0026_a2.html',
                children: [
                  {
                    id: 377,
                    parentId: 345,
                    name: 'Global Search for Resources',
                    local: 'opengauss-0026_a3.html'
                  },
                  {
                    id: 378,
                    parentId: 345,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'opengauss-0026_a4.html'
                  }
                ]
              },
              {
                id: 346,
                parentId: 32,
                name: 'SLA ',
                local: 'opengauss-0152.html',
                children: [
                  {
                    id: 379,
                    parentId: 346,
                    name: 'About SLA',
                    local: 'opengauss-0153.html'
                  },
                  {
                    id: 380,
                    parentId: 346,
                    name: 'Viewing SLA Information',
                    local: 'opengauss-0154.html'
                  },
                  {
                    id: 381,
                    parentId: 346,
                    name: 'Managing SLAs',
                    local: 'opengauss-0155.html'
                  }
                ]
              },
              {
                id: 347,
                parentId: 32,
                name: 'Copies',
                local: 'opgs_0060.html',
                children: [
                  {
                    id: 382,
                    parentId: 347,
                    name: 'Viewing openGauss/CMDB Copy Information',
                    local: 'opengauss-0157.html'
                  },
                  {
                    id: 383,
                    parentId: 347,
                    name: 'Managing openGauss/CMDB Copies',
                    local: 'opgs_0062.html'
                  }
                ]
              },
              {
                id: 348,
                parentId: 32,
                name: 'openGauss/CMDB Database Environments',
                local: 'opengauss-0159.html',
                children: [
                  {
                    id: 384,
                    parentId: 348,
                    name: 'Viewing openGauss/CMDB Information',
                    local: 'opengauss-0160.html'
                  },
                  {
                    id: 385,
                    parentId: 348,
                    name: 'Managing openGauss/CMDB',
                    local: 'opengauss-0161.html'
                  },
                  {
                    id: 386,
                    parentId: 348,
                    name: 'Managing openGauss/CMDB Clusters',
                    local: 'opengauss-0162.html'
                  }
                ]
              }
            ]
          },
          {
            id: 33,
            parentId: 13,
            name: 'GaussDB T Data Protection',
            local: 'en-us_topic_0000001827039680.html',
            children: [
              {
                id: 387,
                parentId: 33,
                name: 'Backup',
                local: 'gaussdbT_00006.html',
                children: [
                  {
                    id: 395,
                    parentId: 387,
                    name: 'Preparations for Backup',
                    local: 'gaussdbT_00009.html'
                  },
                  {
                    id: 396,
                    parentId: 387,
                    name: 'Backing Up a GaussDB T Database',
                    local: 'gaussdbT_00010.html',
                    children: [
                      {
                        id: 397,
                        parentId: 396,
                        name:
                          'Step 1: Checking and Configuring the Database Environment',
                        local: 'gaussdbT_00011.html'
                      },
                      {
                        id: 398,
                        parentId: 396,
                        name: 'Step 2: Setting the Redo Log Mode',
                        local: 'gaussdbT_00071.html'
                      },
                      {
                        id: 399,
                        parentId: 396,
                        name: 'Step 3: Registering the GaussDB T Database',
                        local: 'gaussdbT_00012.html'
                      },
                      {
                        id: 400,
                        parentId: 396,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'gaussdbT_00013.html'
                      },
                      {
                        id: 401,
                        parentId: 396,
                        name:
                          'Step 5: (Optional) Creating a Rate Limiting Policy',
                        local: 'gaussdbT_00014.html'
                      },
                      {
                        id: 402,
                        parentId: 396,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'gaussdbT_00015.html'
                      },
                      {
                        id: 403,
                        parentId: 396,
                        name: 'Step 7: Performing Backup',
                        local: 'gaussdbT_00016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 388,
                parentId: 33,
                name: 'Replication',
                local: 'gaussdbT_00019.html',
                children: [
                  {
                    id: 404,
                    parentId: 388,
                    name: 'Replicating a GaussDB T Database Copy',
                    local: 'gaussdbT_00021.html',
                    children: [
                      {
                        id: 405,
                        parentId: 404,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'gaussdbT_00023.html'
                      },
                      {
                        id: 406,
                        parentId: 404,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_13.html'
                      },
                      {
                        id: 407,
                        parentId: 404,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'gaussdbT_00024.html'
                      },
                      {
                        id: 408,
                        parentId: 404,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'gaussdbT_00025.html'
                      },
                      {
                        id: 409,
                        parentId: 404,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'gaussdbT_00026.html'
                      },
                      {
                        id: 410,
                        parentId: 404,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'gaussdbT_00027.html'
                      },
                      {
                        id: 411,
                        parentId: 404,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'gaussdbT_00027_a1.html'
                      },
                      {
                        id: 412,
                        parentId: 404,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'gaussdbT_00028.html'
                      },
                      {
                        id: 413,
                        parentId: 404,
                        name: 'Step : Creating a Replication SLA',
                        local: 'gaussdbT_00029.html'
                      },
                      {
                        id: 414,
                        parentId: 404,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'gaussdbT_00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 389,
                parentId: 33,
                name: 'Archiving',
                local: 'gaussdbT_00031.html',
                children: [
                  {
                    id: 415,
                    parentId: 389,
                    name: 'Archiving GaussDB T Backup Copies',
                    local: 'gaussdbT_00034.html',
                    children: [
                      {
                        id: 417,
                        parentId: 415,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'gaussdbT_00035.html',
                        children: [
                          {
                            id: 419,
                            parentId: 417,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'gaussdbT_00036.html'
                          },
                          {
                            id: 420,
                            parentId: 417,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'gaussdbT_00037.html'
                          }
                        ]
                      },
                      {
                        id: 418,
                        parentId: 415,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'gaussdbT_00038.html'
                      }
                    ]
                  },
                  {
                    id: 416,
                    parentId: 389,
                    name: 'Archiving GaussDB T Replication Copies ',
                    local: 'gaussdbT_00039.html',
                    children: [
                      {
                        id: 421,
                        parentId: 416,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'gaussdbT_00040.html'
                      },
                      {
                        id: 422,
                        parentId: 416,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'gaussdbT_00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 390,
                parentId: 33,
                name: 'Restoration',
                local: 'gaussdbT_00042.html',
                children: [
                  {
                    id: 423,
                    parentId: 390,
                    name: 'Restoring the GaussDB T Database',
                    local: 'gaussdbT_00045.html'
                  }
                ]
              },
              {
                id: 391,
                parentId: 33,
                name: 'Global Search',
                local: 'gaussdbT_00042_a1.html',
                children: [
                  {
                    id: 424,
                    parentId: 391,
                    name: 'Global Search for Resources',
                    local: 'gaussdbT_00042_a2.html'
                  },
                  {
                    id: 425,
                    parentId: 391,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'gaussdbT_00042_a3.html'
                  }
                ]
              },
              {
                id: 392,
                parentId: 33,
                name: 'SLA ',
                local: 'gaussdbT_00049.html',
                children: [
                  {
                    id: 426,
                    parentId: 392,
                    name: 'About SLA',
                    local: 'gaussdbT_00050.html'
                  },
                  {
                    id: 427,
                    parentId: 392,
                    name: 'Viewing SLA Information',
                    local: 'gaussdbT_00051.html'
                  },
                  {
                    id: 428,
                    parentId: 392,
                    name: 'Managing SLAs',
                    local: 'gaussdbT_00052.html'
                  }
                ]
              },
              {
                id: 393,
                parentId: 33,
                name: 'Copies',
                local: 'gaussdbT_00053.html',
                children: [
                  {
                    id: 429,
                    parentId: 393,
                    name: 'Viewing GaussDB T Database Copy Information',
                    local: 'gaussdbT_00054.html'
                  },
                  {
                    id: 430,
                    parentId: 393,
                    name: 'Managing GaussDB T Database Copies',
                    local: 'gaussdbT_00055.html'
                  }
                ]
              },
              {
                id: 394,
                parentId: 33,
                name: 'GaussDB T Database Environment',
                local: 'gaussdbT_00056.html',
                children: [
                  {
                    id: 431,
                    parentId: 394,
                    name:
                      'Viewing the GaussDB T Database Environment Information',
                    local: 'gaussdbT_00057.html'
                  },
                  {
                    id: 432,
                    parentId: 394,
                    name: 'Managing Databases',
                    local: 'gaussdbT_00058.html'
                  }
                ]
              }
            ]
          },
          {
            id: 34,
            parentId: 13,
            name: 'TiDB Data Protection',
            local: 'en-us_topic_0000001873759409.html',
            children: [
              {
                id: 433,
                parentId: 34,
                name: 'Overview',
                local: 'TIDB_00071.html',
                children: [
                  {
                    id: 443,
                    parentId: 433,
                    name: 'Function Overview',
                    local: 'TIDB_00072.html'
                  }
                ]
              },
              {
                id: 434,
                parentId: 34,
                name: 'Constraints',
                local: 'TIDB_00073.html'
              },
              {
                id: 435,
                parentId: 34,
                name: 'Backup',
                local: 'TiDB_00004.html',
                children: [
                  {
                    id: 444,
                    parentId: 435,
                    name: 'Preparing for the Backup',
                    local: 'TiDB_00007.html'
                  },
                  {
                    id: 445,
                    parentId: 435,
                    name: 'Backing Up TiDB Backup Resources',
                    local: 'TiDB_00008.html',
                    children: [
                      {
                        id: 446,
                        parentId: 445,
                        name: 'Step 1: Registering a TiDB Cluster',
                        local: 'TiDB_00009.html'
                      },
                      {
                        id: 447,
                        parentId: 445,
                        name: 'Step 2: Registering a TiDB Database',
                        local: 'TiDB_00010.html'
                      },
                      {
                        id: 448,
                        parentId: 445,
                        name: 'Step 3: Registering a TiDB Table Set',
                        local: 'TiDB_00011.html'
                      },
                      {
                        id: 449,
                        parentId: 445,
                        name: 'Step 4: Creating a Rate Limiting Policy',
                        local: 'TiDB_00012.html'
                      },
                      {
                        id: 450,
                        parentId: 445,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'TiDB_00013.html'
                      },
                      {
                        id: 451,
                        parentId: 445,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'TiDB_00014.html'
                      },
                      {
                        id: 452,
                        parentId: 445,
                        name: 'Step 7: Performing Backup',
                        local: 'TiDB_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 436,
                parentId: 34,
                name: 'Replication',
                local: 'TiDB_00018.html',
                children: [
                  {
                    id: 453,
                    parentId: 436,
                    name: 'Replicating TiDB Copies',
                    local: 'TiDB_00020.html',
                    children: [
                      {
                        id: 454,
                        parentId: 453,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'TiDB_00022.html'
                      },
                      {
                        id: 455,
                        parentId: 453,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'TiDB_000221.html'
                      },
                      {
                        id: 456,
                        parentId: 453,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'TiDB_00023.html'
                      },
                      {
                        id: 457,
                        parentId: 453,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'TiDB_00024.html'
                      },
                      {
                        id: 458,
                        parentId: 453,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'TiDB_00025.html'
                      },
                      {
                        id: 459,
                        parentId: 453,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'TiDB_00026.html'
                      },
                      {
                        id: 460,
                        parentId: 453,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'TiDB_0002600.html'
                      },
                      {
                        id: 461,
                        parentId: 453,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'TiDB_00027.html'
                      },
                      {
                        id: 462,
                        parentId: 453,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'TiDB_00028.html'
                      },
                      {
                        id: 463,
                        parentId: 453,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'TiDB_00029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 437,
                parentId: 34,
                name: 'Archiving',
                local: 'TiDB_00030.html',
                children: [
                  {
                    id: 464,
                    parentId: 437,
                    name: 'Archiving TiDB Backup Copies',
                    local: 'TiDB_00033.html',
                    children: [
                      {
                        id: 466,
                        parentId: 464,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'TiDB_00034.html',
                        children: [
                          {
                            id: 468,
                            parentId: 466,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'TiDB_00035.html'
                          },
                          {
                            id: 469,
                            parentId: 466,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'TiDB_00036.html'
                          }
                        ]
                      },
                      {
                        id: 467,
                        parentId: 464,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'TiDB_00037.html'
                      }
                    ]
                  },
                  {
                    id: 465,
                    parentId: 437,
                    name: 'Archiving TiDB Replication Copies',
                    local: 'TiDB_00038.html',
                    children: [
                      {
                        id: 470,
                        parentId: 465,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'TiDB_00039.html'
                      },
                      {
                        id: 471,
                        parentId: 465,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'TiDB_00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 438,
                parentId: 34,
                name: 'Restoration',
                local: 'TiDB_00041.html',
                children: [
                  {
                    id: 472,
                    parentId: 438,
                    name: 'Restoring TiDB Backup Resources',
                    local: 'TiDB_00044.html'
                  },
                  {
                    id: 473,
                    parentId: 438,
                    name:
                      'Restoring One or More Tables in TiDB Backup Resources',
                    local: 'TiDB_00045.html'
                  }
                ]
              },
              {
                id: 439,
                parentId: 34,
                name: 'Global Search',
                local: 'TiDB_00046.html',
                children: [
                  {
                    id: 474,
                    parentId: 439,
                    name: 'Global Search for Resources',
                    local: 'TiDB_00047.html'
                  },
                  {
                    id: 475,
                    parentId: 439,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
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
                    name: 'About SLA',
                    local: 'TiDB_000501.html'
                  },
                  {
                    id: 477,
                    parentId: 440,
                    name: 'Viewing SLA Information',
                    local: 'TiDB_00052.html'
                  },
                  {
                    id: 478,
                    parentId: 440,
                    name: 'Managing SLAs',
                    local: 'TiDB_00053.html'
                  }
                ]
              },
              {
                id: 441,
                parentId: 34,
                name: 'Copies',
                local: 'TiDB_00054.html',
                children: [
                  {
                    id: 479,
                    parentId: 441,
                    name: 'Viewing TiDB Copy Information',
                    local: 'TiDB_00055.html'
                  },
                  {
                    id: 480,
                    parentId: 441,
                    name: 'Managing TiDB Copies',
                    local: 'TiDB_00056.html'
                  }
                ]
              },
              {
                id: 442,
                parentId: 34,
                name: 'TiDB Cluster Environment',
                local: 'TiDB_00057.html',
                children: [
                  {
                    id: 481,
                    parentId: 442,
                    name: 'Querying TiDB Information',
                    local: 'TiDB_00058.html'
                  },
                  {
                    id: 482,
                    parentId: 442,
                    name: 'Managing TiDB Clusters',
                    local: 'TiDB_00059.html'
                  },
                  {
                    id: 483,
                    parentId: 442,
                    name: 'Managing Databases',
                    local: 'TiDB_00060.html'
                  },
                  {
                    id: 484,
                    parentId: 442,
                    name: 'Managing Table Sets',
                    local: 'TiDB_00061.html'
                  }
                ]
              }
            ]
          },
          {
            id: 35,
            parentId: 13,
            name: 'OceanBase Data Protection',
            local: 'en-us_topic_0000001826879852.html',
            children: [
              {
                id: 485,
                parentId: 35,
                name: 'Backup',
                local: 'oceanbase_00005.html',
                children: [
                  {
                    id: 493,
                    parentId: 485,
                    name: 'Preparations for Backup',
                    local: 'oceanbase_00008.html'
                  },
                  {
                    id: 494,
                    parentId: 485,
                    name: 'Backing Up OceanBase',
                    local: 'oceanbase_00009.html',
                    children: [
                      {
                        id: 495,
                        parentId: 494,
                        name:
                          'Step 1: Checking and Enabling the NFSv4.1 Service',
                        local: 'en-us_topic_0000001839342213.html'
                      },
                      {
                        id: 496,
                        parentId: 494,
                        name: 'Step 2: Registering an OceanBase Cluster',
                        local: 'oceanbase_00010.html'
                      },
                      {
                        id: 497,
                        parentId: 494,
                        name: 'Step 3: Registering an OceanBase Tenant Set',
                        local: 'oceanbase_00011.html'
                      },
                      {
                        id: 498,
                        parentId: 494,
                        name:
                          'Step 4: (Optional) Creating a Rate Limiting Policy',
                        local: 'oceanbase_00012.html'
                      },
                      {
                        id: 499,
                        parentId: 494,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'oceanbase_00013.html'
                      },
                      {
                        id: 500,
                        parentId: 494,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'oceanbase_00014.html'
                      },
                      {
                        id: 501,
                        parentId: 494,
                        name: 'Step 7: Performing Backup',
                        local: 'oceanbase_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 486,
                parentId: 35,
                name: 'Replication',
                local: 'oracle_gud_000035_3.html',
                children: [
                  {
                    id: 502,
                    parentId: 486,
                    name: 'Replicating OceanBase Copies',
                    local: 'oceanbase_00020.html',
                    children: [
                      {
                        id: 503,
                        parentId: 502,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_4.html'
                      },
                      {
                        id: 504,
                        parentId: 502,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_6.html'
                      },
                      {
                        id: 505,
                        parentId: 502,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'oceanbase_00023.html'
                      },
                      {
                        id: 506,
                        parentId: 502,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'oceanbase_00024.html'
                      },
                      {
                        id: 507,
                        parentId: 502,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'oceanbase_00025.html'
                      },
                      {
                        id: 508,
                        parentId: 502,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'oceanbase_00026.html'
                      },
                      {
                        id: 509,
                        parentId: 502,
                        name:
                          'Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'oceanbase_00026_a1.html'
                      },
                      {
                        id: 510,
                        parentId: 502,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'oceanbase_00027.html'
                      },
                      {
                        id: 511,
                        parentId: 502,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'oceanbase_00028.html'
                      },
                      {
                        id: 512,
                        parentId: 502,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'oceanbase_00029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 487,
                parentId: 35,
                name: 'Archiving',
                local: 'oceanbase_00030.html',
                children: [
                  {
                    id: 513,
                    parentId: 487,
                    name: 'Archiving OceanBase Backup Copies',
                    local: 'oceanbase_00033.html',
                    children: [
                      {
                        id: 515,
                        parentId: 513,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'oceanbase_00034.html',
                        children: [
                          {
                            id: 517,
                            parentId: 515,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'oceanbase_00035.html'
                          },
                          {
                            id: 518,
                            parentId: 515,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'oceanbase_00036.html'
                          }
                        ]
                      },
                      {
                        id: 516,
                        parentId: 513,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'oceanbase_00037.html'
                      }
                    ]
                  },
                  {
                    id: 514,
                    parentId: 487,
                    name: 'Archiving OceanBase Replication Copies',
                    local: 'oceanbase_00038.html',
                    children: [
                      {
                        id: 519,
                        parentId: 514,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'oceanbase_00039.html'
                      },
                      {
                        id: 520,
                        parentId: 514,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'oceanbase_00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 488,
                parentId: 35,
                name: 'Restoration',
                local: 'oceanbase_00041.html',
                children: [
                  {
                    id: 521,
                    parentId: 488,
                    name: 'Restoring OceanBase',
                    local: 'oceanbase_00044.html'
                  }
                ]
              },
              {
                id: 489,
                parentId: 35,
                name: 'Global Search',
                local: 'oceanbase_00026_a2.html',
                children: [
                  {
                    id: 522,
                    parentId: 489,
                    name: 'Global Search for Resources',
                    local: 'oceanbase_00026_a3.html'
                  },
                  {
                    id: 523,
                    parentId: 489,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'oceanbase_00026_a4.html'
                  }
                ]
              },
              {
                id: 490,
                parentId: 35,
                name: 'SLA ',
                local: 'oceanbase_00050.html',
                children: [
                  {
                    id: 524,
                    parentId: 490,
                    name: 'About SLA',
                    local: 'oceanbase_00051.html'
                  },
                  {
                    id: 525,
                    parentId: 490,
                    name: 'Viewing SLA Information',
                    local: 'oceanbase_00052.html'
                  },
                  {
                    id: 526,
                    parentId: 490,
                    name: 'Managing SLAs',
                    local: 'oceanbase_00053.html'
                  }
                ]
              },
              {
                id: 491,
                parentId: 35,
                name: 'Copies',
                local: 'oceanbase_00054.html',
                children: [
                  {
                    id: 527,
                    parentId: 491,
                    name: 'Viewing OceanBase Copy Information',
                    local: 'oceanbase_00055.html'
                  },
                  {
                    id: 528,
                    parentId: 491,
                    name: 'Managing OceanBase Copies',
                    local: 'oceanbase_00056.html'
                  }
                ]
              },
              {
                id: 492,
                parentId: 35,
                name: 'OceanBase Cluster Environment',
                local: 'oceanbase_00057.html',
                children: [
                  {
                    id: 529,
                    parentId: 492,
                    name: 'Viewing the OceanBase Environment',
                    local: 'oceanbase_00058.html'
                  },
                  {
                    id: 530,
                    parentId: 492,
                    name: 'Managing Clusters',
                    local: 'oceanbase_00059.html'
                  },
                  {
                    id: 531,
                    parentId: 492,
                    name: 'Managing Tenant Sets',
                    local: 'oceanbase_00060.html'
                  }
                ]
              }
            ]
          },
          {
            id: 36,
            parentId: 13,
            name: 'TDSQL Data Protection',
            local: 'en-us_topic_0000001827039708.html',
            children: [
              {
                id: 532,
                parentId: 36,
                name: 'Backup',
                local: 'tdsql_gud_006.html',
                children: [
                  {
                    id: 540,
                    parentId: 532,
                    name: 'Constraints',
                    local: 'tdsql_gud_082.html'
                  },
                  {
                    id: 541,
                    parentId: 532,
                    name: 'Preparations for Backup',
                    local: 'tdsql_gud_081.html'
                  },
                  {
                    id: 542,
                    parentId: 532,
                    name: 'Backing Up TDSQL Databases',
                    local: 'tdsql_gud_009.html',
                    children: [
                      {
                        id: 543,
                        parentId: 542,
                        name:
                          'Step 1: Enabling the TDSQL Database Permission (Applicable to Non-distributed Instances)',
                        local: 'tdsql_gud_010.html'
                      },
                      {
                        id: 544,
                        parentId: 542,
                        name:
                          'Step 2: Enabling the Automatic zkmeta Backup Function (Applicable to Distributed Instances)',
                        local: 'tdsql_gud_080.html'
                      },
                      {
                        id: 545,
                        parentId: 542,
                        name: 'Step 3: Registering a TDSQL Database',
                        local: 'tdsql_gud_011.html'
                      },
                      {
                        id: 546,
                        parentId: 542,
                        name:
                          'Step 4: (Optional) Creating a Rate Limiting Policy',
                        local: 'tdsql_gud_012.html'
                      },
                      {
                        id: 547,
                        parentId: 542,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'tdsql_gud_013.html'
                      },
                      {
                        id: 548,
                        parentId: 542,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'tdsql_gud_014.html'
                      },
                      {
                        id: 549,
                        parentId: 542,
                        name: 'Step 7: Performing Backup',
                        local: 'tdsql_gud_015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 533,
                parentId: 36,
                name: 'Replication',
                local: 'tdsql_gud_018.html',
                children: [
                  {
                    id: 550,
                    parentId: 533,
                    name: 'Replicating TDSQL Database Copies',
                    local: 'tdsql_gud_020.html',
                    children: [
                      {
                        id: 551,
                        parentId: 550,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'tdsql_gud_022.html'
                      },
                      {
                        id: 552,
                        parentId: 550,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'tdsql_gud_0026_1.html'
                      },
                      {
                        id: 553,
                        parentId: 550,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'tdsql_gud_023.html'
                      },
                      {
                        id: 554,
                        parentId: 550,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'tdsql_gud_024.html'
                      },
                      {
                        id: 555,
                        parentId: 550,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'tdsql_gud_025.html'
                      },
                      {
                        id: 556,
                        parentId: 550,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to Version 1.5.0)',
                        local: 'tdsql_gud_026.html'
                      },
                      {
                        id: 557,
                        parentId: 550,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002044460125.html'
                      },
                      {
                        id: 558,
                        parentId: 550,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'tdsql_gud_027.html'
                      },
                      {
                        id: 559,
                        parentId: 550,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'tdsql_gud_028.html'
                      },
                      {
                        id: 560,
                        parentId: 550,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'tdsql_gud_029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 534,
                parentId: 36,
                name: 'Archiving',
                local: 'tdsql_gud_030.html',
                children: [
                  {
                    id: 561,
                    parentId: 534,
                    name: 'Archiving TDSQL Backup Copies',
                    local: 'tdsql_gud_033.html',
                    children: [
                      {
                        id: 563,
                        parentId: 561,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'tdsql_gud_034.html',
                        children: [
                          {
                            id: 565,
                            parentId: 563,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'tdsql_gud_035.html'
                          },
                          {
                            id: 566,
                            parentId: 563,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'tdsql_gud_036.html'
                          }
                        ]
                      },
                      {
                        id: 564,
                        parentId: 561,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'tdsql_gud_037.html'
                      }
                    ]
                  },
                  {
                    id: 562,
                    parentId: 534,
                    name: 'Archiving TDSQL Replication Copies ',
                    local: 'tdsql_gud_038.html',
                    children: [
                      {
                        id: 567,
                        parentId: 562,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'tdsql_gud_039.html'
                      },
                      {
                        id: 568,
                        parentId: 562,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'tdsql_gud_040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 535,
                parentId: 36,
                name: 'Restoration',
                local: 'tdsql_gud_041.html',
                children: [
                  {
                    id: 569,
                    parentId: 535,
                    name: 'Restoring a TDSQL Database',
                    local: 'tdsql_gud_044.html'
                  }
                ]
              },
              {
                id: 536,
                parentId: 36,
                name: 'Global Search',
                local: 'tdsql_gud_045.html',
                children: [
                  {
                    id: 570,
                    parentId: 536,
                    name: 'Global Search for Resources',
                    local: 'tdsql_gud_047.html'
                  },
                  {
                    id: 571,
                    parentId: 536,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002686458.html'
                  }
                ]
              },
              {
                id: 537,
                parentId: 36,
                name: 'SLA ',
                local: 'tdsql_gud_059.html',
                children: [
                  {
                    id: 572,
                    parentId: 537,
                    name: 'About SLA ',
                    local: 'tdsql_gud_060.html'
                  },
                  {
                    id: 573,
                    parentId: 537,
                    name: 'Viewing SLA Information',
                    local: 'tdsql_gud_061.html'
                  },
                  {
                    id: 574,
                    parentId: 537,
                    name: 'Managing SLAs',
                    local: 'tdsql_gud_062.html'
                  }
                ]
              },
              {
                id: 538,
                parentId: 36,
                name: 'Copies',
                local: 'tdsql_gud_063.html',
                children: [
                  {
                    id: 575,
                    parentId: 538,
                    name: 'Viewing TDSQL Copy Information',
                    local: 'tdsql_gud_064.html'
                  },
                  {
                    id: 576,
                    parentId: 538,
                    name: 'Managing TDSQL Copies',
                    local: 'tdsql_gud_065.html'
                  }
                ]
              },
              {
                id: 539,
                parentId: 36,
                name: 'TDSQL Database Environment',
                local: 'tdsql_gud_066.html',
                children: [
                  {
                    id: 577,
                    parentId: 539,
                    name: 'Viewing TDSQL Database Environment Information',
                    local: 'tdsql_gud_067.html'
                  },
                  {
                    id: 578,
                    parentId: 539,
                    name: 'Managing Database Clusters',
                    local: 'tdsql_gud_069.html'
                  },
                  {
                    id: 579,
                    parentId: 539,
                    name: 'Managing Database Instances',
                    local: 'tdsql_gud_068.html'
                  }
                ]
              }
            ]
          },
          {
            id: 37,
            parentId: 13,
            name: 'Dameng Data Protection',
            local: 'en-us_topic_0000001873759369.html',
            children: [
              {
                id: 580,
                parentId: 37,
                name: 'Backup',
                local: 'dameng-00007.html',
                children: [
                  {
                    id: 588,
                    parentId: 580,
                    name: 'Preparations for Backup',
                    local: 'en-us_topic_0000002012732625.html'
                  },
                  {
                    id: 589,
                    parentId: 580,
                    name: 'Backing Up Dameng',
                    local: 'dameng-00011.html',
                    children: [
                      {
                        id: 590,
                        parentId: 589,
                        name: 'Step 1: Enabling the DmAPService',
                        local: 'dameng-00012.html'
                      },
                      {
                        id: 591,
                        parentId: 589,
                        name:
                          'Step 2: Enabling Local Archiving for the Database',
                        local: 'dameng-00013.html'
                      },
                      {
                        id: 592,
                        parentId: 589,
                        name: 'Step 3: Registering the Dameng Database',
                        local: 'dameng-00014.html'
                      },
                      {
                        id: 593,
                        parentId: 589,
                        name:
                          'Step 4: (Optional) Creating a Rate Limiting Policy',
                        local: 'en-us_topic_0000001976337238.html'
                      },
                      {
                        id: 594,
                        parentId: 589,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'dameng-00016.html'
                      },
                      {
                        id: 595,
                        parentId: 589,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'dameng-00017.html'
                      },
                      {
                        id: 596,
                        parentId: 589,
                        name: 'Step 7: Executing Backup',
                        local: 'dameng-00018.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 581,
                parentId: 37,
                name: 'Replication',
                local: 'oracle_gud_000035_1.html',
                children: [
                  {
                    id: 597,
                    parentId: 581,
                    name: 'Replicating Dameng Database Copies',
                    local: 'dameng-00024.html',
                    children: [
                      {
                        id: 598,
                        parentId: 597,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_2.html'
                      },
                      {
                        id: 599,
                        parentId: 597,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_3.html'
                      },
                      {
                        id: 600,
                        parentId: 597,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'dameng-00027.html'
                      },
                      {
                        id: 601,
                        parentId: 597,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'dameng-00028.html'
                      },
                      {
                        id: 602,
                        parentId: 597,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'dameng-00029.html'
                      },
                      {
                        id: 603,
                        parentId: 597,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'dameng-00030.html'
                      },
                      {
                        id: 604,
                        parentId: 597,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'dameng-00027_a1.html'
                      },
                      {
                        id: 605,
                        parentId: 597,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'dameng-00031.html'
                      },
                      {
                        id: 606,
                        parentId: 597,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'dameng-00032.html'
                      },
                      {
                        id: 607,
                        parentId: 597,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'dameng-00033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 582,
                parentId: 37,
                name: 'Archiving',
                local: 'dameng-00034.html',
                children: [
                  {
                    id: 608,
                    parentId: 582,
                    name: 'Archiving Dameng Backup Copies',
                    local: 'dameng-00037.html',
                    children: [
                      {
                        id: 610,
                        parentId: 608,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'dameng-00038.html',
                        children: [
                          {
                            id: 612,
                            parentId: 610,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'dameng-00039.html'
                          },
                          {
                            id: 613,
                            parentId: 610,
                            name:
                              'Creating a Media Set (When Archive Storage Is a Tape Library)',
                            local: 'dameng-00040.html'
                          }
                        ]
                      },
                      {
                        id: 611,
                        parentId: 608,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'dameng-00041.html'
                      }
                    ]
                  },
                  {
                    id: 609,
                    parentId: 582,
                    name: 'Archiving Dameng Replication Copies',
                    local: 'dameng-00042.html',
                    children: [
                      {
                        id: 614,
                        parentId: 609,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'dameng-00043.html'
                      },
                      {
                        id: 615,
                        parentId: 609,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'dameng-00044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 583,
                parentId: 37,
                name: 'Restoration',
                local: 'dameng-00045.html',
                children: [
                  {
                    id: 616,
                    parentId: 583,
                    name: 'Restoring Dameng',
                    local: 'dameng-00048.html'
                  }
                ]
              },
              {
                id: 584,
                parentId: 37,
                name: 'Global Search',
                local: 'dameng-00027_a2.html',
                children: [
                  {
                    id: 617,
                    parentId: 584,
                    name: 'Global Search for Resources',
                    local: 'dameng-00027_a3.html'
                  },
                  {
                    id: 618,
                    parentId: 584,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'dameng-00027_a4.html'
                  }
                ]
              },
              {
                id: 585,
                parentId: 37,
                name: 'SLA ',
                local: 'dameng-00053.html',
                children: [
                  {
                    id: 619,
                    parentId: 585,
                    name: 'About SLA',
                    local: 'dameng-00054.html'
                  },
                  {
                    id: 620,
                    parentId: 585,
                    name: 'Viewing SLA Information',
                    local: 'dameng-00055.html'
                  },
                  {
                    id: 621,
                    parentId: 585,
                    name: 'Managing SLAs',
                    local: 'dameng-00056.html'
                  }
                ]
              },
              {
                id: 586,
                parentId: 37,
                name: 'Copies',
                local: 'dameng-00057.html',
                children: [
                  {
                    id: 622,
                    parentId: 586,
                    name: 'Viewing Dameng Copy Information',
                    local: 'dameng-00058.html'
                  },
                  {
                    id: 623,
                    parentId: 586,
                    name: 'Managing Dameng Copies',
                    local: 'dameng-00059.html'
                  }
                ]
              },
              {
                id: 587,
                parentId: 37,
                name: 'Dameng Environment',
                local: 'dameng-00060.html',
                children: [
                  {
                    id: 624,
                    parentId: 587,
                    name: 'Viewing Dameng Environment Information',
                    local: 'dameng-00061.html'
                  },
                  {
                    id: 625,
                    parentId: 587,
                    name: 'Managing Dameng Databases',
                    local: 'dameng-00062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 38,
            parentId: 13,
            name: 'Kingbase Data Protection',
            local: 'en-us_topic_0000001827039700.html',
            children: [
              {
                id: 626,
                parentId: 38,
                name: 'Backup',
                local: 'kingbase-00005.html',
                children: [
                  {
                    id: 634,
                    parentId: 626,
                    name: 'Preparing for the Backup',
                    local: 'dameng-00008_0.html'
                  },
                  {
                    id: 635,
                    parentId: 626,
                    name: 'Backing Up a Kingbase Instance',
                    local: 'kingbase-00008.html',
                    children: [
                      {
                        id: 636,
                        parentId: 635,
                        name: 'Step 1: Initializing sys_rman',
                        local: 'en-us_topic_0000002015631765.html'
                      },
                      {
                        id: 637,
                        parentId: 635,
                        name:
                          'Step 2: Registering the Database of a Single Kingbase Instance',
                        local: 'kingbase-00009.html'
                      },
                      {
                        id: 638,
                        parentId: 635,
                        name:
                          'Step 3: Registering the Database of a Kingbase Cluster Instance',
                        local: 'kingbase-00010.html'
                      },
                      {
                        id: 639,
                        parentId: 635,
                        name: 'Step 4: Creating a Rate Limiting Policy',
                        local: 'kingbase-00011.html'
                      },
                      {
                        id: 640,
                        parentId: 635,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'kingbase-00012.html'
                      },
                      {
                        id: 641,
                        parentId: 635,
                        name: 'Step : Creating a Backup SLA',
                        local: 'kingbase-00013.html'
                      },
                      {
                        id: 642,
                        parentId: 635,
                        name: 'Step : Executing Backup',
                        local: 'kingbase-00014.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 627,
                parentId: 38,
                name: 'Replication',
                local: 'oracle_gud_000035_2.html',
                children: [
                  {
                    id: 643,
                    parentId: 627,
                    name: 'Replicating Kingbase Copies',
                    local: 'kingbase-00020.html',
                    children: [
                      {
                        id: 644,
                        parentId: 643,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_3.html'
                      },
                      {
                        id: 645,
                        parentId: 643,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_5.html'
                      },
                      {
                        id: 646,
                        parentId: 643,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'kingbase-00023.html'
                      },
                      {
                        id: 647,
                        parentId: 643,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'kingbase-00024.html'
                      },
                      {
                        id: 648,
                        parentId: 643,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'kingbase-00025.html'
                      },
                      {
                        id: 649,
                        parentId: 643,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'kingbase-00026.html'
                      },
                      {
                        id: 650,
                        parentId: 643,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'kingbase-00025_a1.html'
                      },
                      {
                        id: 651,
                        parentId: 643,
                        name: 'Step : Adding a Target Cluster',
                        local: 'kingbase-00027.html'
                      },
                      {
                        id: 652,
                        parentId: 643,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'kingbase-00028.html'
                      },
                      {
                        id: 653,
                        parentId: 643,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'kingbase-00029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 628,
                parentId: 38,
                name: 'Archiving',
                local: 'kingbase-00030.html',
                children: [
                  {
                    id: 654,
                    parentId: 628,
                    name: 'Archiving Kingbase Backup Copies',
                    local: 'kingbase-00033.html',
                    children: [
                      {
                        id: 656,
                        parentId: 654,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'kingbase-00034.html',
                        children: [
                          {
                            id: 658,
                            parentId: 656,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'kingbase-00035.html'
                          },
                          {
                            id: 659,
                            parentId: 656,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'kingbase-00036.html'
                          }
                        ]
                      },
                      {
                        id: 657,
                        parentId: 654,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'kingbase-00037.html'
                      }
                    ]
                  },
                  {
                    id: 655,
                    parentId: 628,
                    name: 'Archiving Kingbase Replication Copies',
                    local: 'kingbase-00038.html',
                    children: [
                      {
                        id: 660,
                        parentId: 655,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'kingbase-00039.html'
                      },
                      {
                        id: 661,
                        parentId: 655,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'kingbase-00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 629,
                parentId: 38,
                name: 'Restoration',
                local: 'kingbase-00041.html',
                children: [
                  {
                    id: 662,
                    parentId: 629,
                    name: 'Restoring a Kingbase Instance',
                    local: 'kingbase-00044.html'
                  }
                ]
              },
              {
                id: 630,
                parentId: 38,
                name: 'Global Search',
                local: 'kingbase-00025_a2.html',
                children: [
                  {
                    id: 663,
                    parentId: 630,
                    name: 'Global Search for Resources',
                    local: 'kingbase-00025_a3.html'
                  },
                  {
                    id: 664,
                    parentId: 630,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'kingbase-00025_a4.html'
                  }
                ]
              },
              {
                id: 631,
                parentId: 38,
                name: 'SLA ',
                local: 'kingbase-00049.html',
                children: [
                  {
                    id: 665,
                    parentId: 631,
                    name: 'About SLA',
                    local: 'kingbase-00049_0.html'
                  },
                  {
                    id: 666,
                    parentId: 631,
                    name: 'Viewing SLA Information',
                    local: 'kingbase-00051.html'
                  },
                  {
                    id: 667,
                    parentId: 631,
                    name: 'Managing SLAs',
                    local: 'kingbase-00052.html'
                  }
                ]
              },
              {
                id: 632,
                parentId: 38,
                name: 'Copies',
                local: 'kingbase-00053.html',
                children: [
                  {
                    id: 668,
                    parentId: 632,
                    name: 'Viewing Kingbase Copy Information',
                    local: 'kingbase-00054.html'
                  },
                  {
                    id: 669,
                    parentId: 632,
                    name: 'Managing Kingbase Copies',
                    local: 'kingbase-00055.html'
                  }
                ]
              },
              {
                id: 633,
                parentId: 38,
                name: 'Kingbase Cluster Environment',
                local: 'kingbase-00056.html',
                children: [
                  {
                    id: 670,
                    parentId: 633,
                    name: 'Checking the Kingbase Environment',
                    local: 'kingbase-00057.html'
                  },
                  {
                    id: 671,
                    parentId: 633,
                    name: 'Managing Kingbase',
                    local: 'kingbase-00058.html'
                  },
                  {
                    id: 672,
                    parentId: 633,
                    name: 'Managing Kingbase Database Clusters',
                    local: 'kingbase-00059.html'
                  }
                ]
              }
            ]
          },
          {
            id: 39,
            parentId: 13,
            name: 'GoldenDB Data Protection',
            local: 'en-us_topic_0000001873759373.html',
            children: [
              {
                id: 673,
                parentId: 39,
                name: 'Backup',
                local: 'goldendb-00005.html',
                children: [
                  {
                    id: 681,
                    parentId: 673,
                    name: 'Preparations for Backup',
                    local: 'goldendb-00008.html'
                  },
                  {
                    id: 682,
                    parentId: 673,
                    name: 'Backing Up a GoldenDB Database',
                    local: 'goldendb-00009.html',
                    children: [
                      {
                        id: 683,
                        parentId: 682,
                        name: 'Step 1: Registering a GoldenDB Cluster',
                        local: 'goldendb-00010.html'
                      },
                      {
                        id: 684,
                        parentId: 682,
                        name: 'Step 2: Creating a GoldenDB Instance',
                        local: 'goldendb-00011.html'
                      },
                      {
                        id: 685,
                        parentId: 682,
                        name: 'Step 3: Creating a Rate Limiting Policy ',
                        local: 'goldendb-00012.html'
                      },
                      {
                        id: 686,
                        parentId: 682,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'goldendb-00013.html'
                      },
                      {
                        id: 687,
                        parentId: 682,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'goldendb-00014.html'
                      },
                      {
                        id: 688,
                        parentId: 682,
                        name: 'Step 6: Performing Backup',
                        local: 'goldendb-00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 674,
                parentId: 39,
                name: 'Replication',
                local: 'oracle_gud_000035_0.html',
                children: [
                  {
                    id: 689,
                    parentId: 674,
                    name: 'Replicating GoldenDB Copies',
                    local: 'goldendb-00021.html',
                    children: [
                      {
                        id: 690,
                        parentId: 689,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_0.html'
                      },
                      {
                        id: 691,
                        parentId: 689,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_1.html'
                      },
                      {
                        id: 692,
                        parentId: 689,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'goldendb-00024.html'
                      },
                      {
                        id: 693,
                        parentId: 689,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'goldendb-00025.html'
                      },
                      {
                        id: 694,
                        parentId: 689,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'goldendb-00026.html'
                      },
                      {
                        id: 695,
                        parentId: 689,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'goldendb-00027.html'
                      },
                      {
                        id: 696,
                        parentId: 689,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002010566636.html'
                      },
                      {
                        id: 697,
                        parentId: 689,
                        name: 'Step : Adding a Target Cluster',
                        local: 'goldendb-00028.html'
                      },
                      {
                        id: 698,
                        parentId: 689,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'goldendb-00029.html'
                      },
                      {
                        id: 699,
                        parentId: 689,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'goldendb-00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 675,
                parentId: 39,
                name: 'Archiving',
                local: 'goldendb-00031.html',
                children: [
                  {
                    id: 700,
                    parentId: 675,
                    name: 'Archiving GoldenDB Backup Copies',
                    local: 'goldendb-00034.html',
                    children: [
                      {
                        id: 702,
                        parentId: 700,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'goldendb-00035.html',
                        children: [
                          {
                            id: 704,
                            parentId: 702,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'goldendb-00036.html'
                          },
                          {
                            id: 705,
                            parentId: 702,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'goldendb-00037.html'
                          }
                        ]
                      },
                      {
                        id: 703,
                        parentId: 700,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'goldendb-00038.html'
                      }
                    ]
                  },
                  {
                    id: 701,
                    parentId: 675,
                    name: 'Archiving GoldenDB Replication Copies ',
                    local: 'goldendb-00039.html',
                    children: [
                      {
                        id: 706,
                        parentId: 701,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'goldendb-00040.html'
                      },
                      {
                        id: 707,
                        parentId: 701,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'goldendb-00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 676,
                parentId: 39,
                name: 'Restoration',
                local: 'goldendb-00042.html',
                children: [
                  {
                    id: 708,
                    parentId: 676,
                    name: 'Restoring GoldenDB',
                    local: 'goldendb-00045.html'
                  }
                ]
              },
              {
                id: 677,
                parentId: 39,
                name: 'Global Search',
                local: 'goldendb-00046.html',
                children: [
                  {
                    id: 709,
                    parentId: 677,
                    name: 'Global Search for Resources',
                    local: 'goldendb-00048.html'
                  },
                  {
                    id: 710,
                    parentId: 677,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002838402.html'
                  }
                ]
              },
              {
                id: 678,
                parentId: 39,
                name: 'SLA ',
                local: 'goldendb-00051.html',
                children: [
                  {
                    id: 711,
                    parentId: 678,
                    name: 'About SLA',
                    local: 'goldendb-00052.html'
                  },
                  {
                    id: 712,
                    parentId: 678,
                    name: 'Viewing SLA Information',
                    local: 'goldendb-00053.html'
                  },
                  {
                    id: 713,
                    parentId: 678,
                    name: 'Managing SLAs',
                    local: 'goldendb-00054.html'
                  }
                ]
              },
              {
                id: 679,
                parentId: 39,
                name: 'Copies',
                local: 'goldendb-00055.html',
                children: [
                  {
                    id: 714,
                    parentId: 679,
                    name: 'Viewing GoldenDB Copy Information',
                    local: 'goldendb-00056.html'
                  },
                  {
                    id: 715,
                    parentId: 679,
                    name: 'Managing GoldenDB Copies',
                    local: 'goldendb-00057.html'
                  }
                ]
              },
              {
                id: 680,
                parentId: 39,
                name: 'GoldenDB Cluster Environment',
                local: 'goldendb-00058.html',
                children: [
                  {
                    id: 716,
                    parentId: 680,
                    name: 'Viewing the GoldenDB Environment',
                    local: 'goldendb-00059.html'
                  },
                  {
                    id: 717,
                    parentId: 680,
                    name: 'Managing Instances',
                    local: 'goldendb-00060.html'
                  },
                  {
                    id: 718,
                    parentId: 680,
                    name: 'Management Clusters',
                    local: 'goldendb-00061.html'
                  }
                ]
              }
            ]
          },
          {
            id: 40,
            parentId: 13,
            name: 'GaussDB Data Protection',
            local: 'en-us_topic_0000001827039692.html',
            children: [
              {
                id: 719,
                parentId: 40,
                name: 'Backup',
                local: 'TPOPS_GaussDB_00006.html',
                children: [
                  {
                    id: 727,
                    parentId: 719,
                    name: 'Preparing for the Backup',
                    local: 'TPOPS_GaussDB_00009.html'
                  },
                  {
                    id: 728,
                    parentId: 719,
                    name: 'Backing Up GaussDB Instances',
                    local: 'TPOPS_GaussDB_00010.html',
                    children: [
                      {
                        id: 729,
                        parentId: 728,
                        name:
                          'Step 1: Obtaining the Address and Port of the Management Plane',
                        local: 'TPOPS_GaussDB_00014.html'
                      },
                      {
                        id: 730,
                        parentId: 728,
                        name:
                          'Step 2: Enabling the XBSA Backup Whitelist on the TPOPS Node',
                        local: 'TPOPS_GaussDB_00013.html'
                      },
                      {
                        id: 731,
                        parentId: 728,
                        name: 'Step 3: Registering the GaussDB Project',
                        local: 'TPOPS_GaussDB_00015.html'
                      },
                      {
                        id: 732,
                        parentId: 728,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'TPOPS_GaussDB_00016.html'
                      },
                      {
                        id: 733,
                        parentId: 728,
                        name: 'Step 5: Creating a Rate Limiting Policy',
                        local: 'TPOPS_GaussDB_00017.html'
                      },
                      {
                        id: 734,
                        parentId: 728,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'TPOPS_GaussDB_00018.html'
                      },
                      {
                        id: 735,
                        parentId: 728,
                        name: 'Step 7: Performing Backup',
                        local: 'TPOPS_GaussDB_00019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 720,
                parentId: 40,
                name: 'Replication',
                local: 'TPOPS_GaussDB_00022.html',
                children: [
                  {
                    id: 736,
                    parentId: 720,
                    name: 'Replicating GaussDB Copies',
                    local: 'TPOPS_GaussDB_00024.html',
                    children: [
                      {
                        id: 737,
                        parentId: 736,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'TPOPS_GaussDB_00026.html'
                      },
                      {
                        id: 738,
                        parentId: 736,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'TPOPS_GaussDB_000260.html'
                      },
                      {
                        id: 739,
                        parentId: 736,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'TPOPS_GaussDB_00027.html'
                      },
                      {
                        id: 740,
                        parentId: 736,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'TPOPS_GaussDB_00028.html'
                      },
                      {
                        id: 741,
                        parentId: 736,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'TPOPS_GaussDB_00029.html'
                      },
                      {
                        id: 742,
                        parentId: 736,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'TPOPS_GaussDB_00030.html'
                      },
                      {
                        id: 743,
                        parentId: 736,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'TPOPS_GaussDB_000300.html'
                      },
                      {
                        id: 744,
                        parentId: 736,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'TPOPS_GaussDB_00031.html'
                      },
                      {
                        id: 745,
                        parentId: 736,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'TPOPS_GaussDB_00032.html'
                      },
                      {
                        id: 746,
                        parentId: 736,
                        name:
                          'Step 8: (Optional) Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'TPOPS_GaussDB_00033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 721,
                parentId: 40,
                name: 'Archiving',
                local: 'TPOPS_GaussDB_00034.html',
                children: [
                  {
                    id: 747,
                    parentId: 721,
                    name: 'Archiving GaussDB Backup Copies',
                    local: 'TPOPS_GaussDB_00037.html',
                    children: [
                      {
                        id: 749,
                        parentId: 747,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'TPOPS_GaussDB_00038.html',
                        children: [
                          {
                            id: 751,
                            parentId: 749,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'TPOPS_GaussDB_00039.html'
                          },
                          {
                            id: 752,
                            parentId: 749,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'TPOPS_GaussDB_00040.html'
                          }
                        ]
                      },
                      {
                        id: 750,
                        parentId: 747,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'TPOPS_GaussDB_00041.html'
                      }
                    ]
                  },
                  {
                    id: 748,
                    parentId: 721,
                    name: 'Archiving GaussDB Replication Copies',
                    local: 'TPOPS_GaussDB_00042.html',
                    children: [
                      {
                        id: 753,
                        parentId: 748,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'TPOPS_GaussDB_00043.html'
                      },
                      {
                        id: 754,
                        parentId: 748,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'TPOPS_GaussDB_00044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 722,
                parentId: 40,
                name: 'Restoration',
                local: 'TPOPS_GaussDB_00045.html',
                children: [
                  {
                    id: 755,
                    parentId: 722,
                    name: 'Restoring GaussDB Instances',
                    local: 'TPOPS_GaussDB_00048.html'
                  }
                ]
              },
              {
                id: 723,
                parentId: 40,
                name: 'Global Search',
                local: 'TPOPS_GaussDB_000451.html',
                children: [
                  {
                    id: 756,
                    parentId: 723,
                    name: 'Global Search for Resources',
                    local: 'TPOPS_GaussDB_00049.html'
                  },
                  {
                    id: 757,
                    parentId: 723,
                    name: '全局标签搜索（适用于1.6.0及后续版本）',
                    local: 'TPOPS_GaussDB_000491.html'
                  }
                ]
              },
              {
                id: 724,
                parentId: 40,
                name: 'SLA ',
                local: 'TPOPS_GaussDB_00052.html',
                children: [
                  {
                    id: 758,
                    parentId: 724,
                    name: 'About SLA ',
                    local: 'TPOPS_GaussDB_000540.html'
                  },
                  {
                    id: 759,
                    parentId: 724,
                    name: 'Viewing SLA Information',
                    local: 'TPOPS_GaussDB_00054.html'
                  },
                  {
                    id: 760,
                    parentId: 724,
                    name: 'Managing SLAs',
                    local: 'TPOPS_GaussDB_00055.html'
                  }
                ]
              },
              {
                id: 725,
                parentId: 40,
                name: 'Copies',
                local: 'TPOPS_GaussDB_00056.html',
                children: [
                  {
                    id: 761,
                    parentId: 725,
                    name: 'Viewing GaussDB Copy Information',
                    local: 'TPOPS_GaussDB_00057.html'
                  },
                  {
                    id: 762,
                    parentId: 725,
                    name: 'Managing GaussDB Copies',
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
                    id: 763,
                    parentId: 726,
                    name: 'Viewing GaussDB Information',
                    local: 'TPOPS_GaussDB_00060.html'
                  },
                  {
                    id: 764,
                    parentId: 726,
                    name: 'Managing GaussDB Projects',
                    local: 'TPOPS_GaussDB_00061.html'
                  },
                  {
                    id: 765,
                    parentId: 726,
                    name: 'Managing Instances',
                    local: 'TPOPS_GaussDB_00062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 41,
            parentId: 13,
            name: 'GBase 8a Data Protection',
            local: 'en-us_topic_0000001873759389.html',
            children: [
              {
                id: 766,
                parentId: 41,
                name: 'Backup',
                local: 'GBase_8a_00007.html',
                children: [
                  {
                    id: 774,
                    parentId: 766,
                    name: 'Preparations for Backup',
                    local: 'GBase_8a_00010.html'
                  },
                  {
                    id: 775,
                    parentId: 766,
                    name: 'Backing Up a GBase 8a Database',
                    local: 'GBase_8a_00011.html',
                    children: [
                      {
                        id: 776,
                        parentId: 775,
                        name: 'Step 1 Registering the GBase 8a Database',
                        local: 'GBase_8a_00012.html'
                      },
                      {
                        id: 777,
                        parentId: 775,
                        name: 'Step 2 Creating a Rate Limiting Policy',
                        local: 'GBase_8a_00013.html'
                      },
                      {
                        id: 778,
                        parentId: 775,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'GBase_8a_00014.html'
                      },
                      {
                        id: 779,
                        parentId: 775,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'GBase_8a_00015.html'
                      },
                      {
                        id: 780,
                        parentId: 775,
                        name: 'Step 5: Performing Backup',
                        local: 'GBase_8a_00016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 767,
                parentId: 41,
                name: 'Replication',
                local: 'oracle_gud_000035_8.html',
                children: [
                  {
                    id: 781,
                    parentId: 767,
                    name: 'Replicating a GBase 8a Database Copy',
                    local: 'GBase_8a_00021.html',
                    children: [
                      {
                        id: 782,
                        parentId: 781,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'GBase_8a_00023.html'
                      },
                      {
                        id: 783,
                        parentId: 781,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'GBase_8a_000231.html'
                      },
                      {
                        id: 784,
                        parentId: 781,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'GBase_8a_00024.html'
                      },
                      {
                        id: 785,
                        parentId: 781,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'GBase_8a_00025.html'
                      },
                      {
                        id: 786,
                        parentId: 781,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'GBase_8a_00026.html'
                      },
                      {
                        id: 787,
                        parentId: 781,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'GBase_8a_00027.html'
                      },
                      {
                        id: 788,
                        parentId: 781,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'GBase_8a_0002600.html'
                      },
                      {
                        id: 789,
                        parentId: 781,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'GBase_8a_00028.html'
                      },
                      {
                        id: 790,
                        parentId: 781,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'GBase_8a_00029.html'
                      },
                      {
                        id: 791,
                        parentId: 781,
                        name:
                          '(Optional) Step 8: Creating a Bidirectional Replication or Cascading Replication SLA',
                        local: 'GBase_8a_00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 768,
                parentId: 41,
                name: 'Archiving',
                local: 'GBase_8a_00031.html',
                children: [
                  {
                    id: 792,
                    parentId: 768,
                    name: 'Archiving GBase 8a Backup Copies',
                    local: 'GBase_8a_00034.html',
                    children: [
                      {
                        id: 794,
                        parentId: 792,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'GBase_8a_00035.html',
                        children: [
                          {
                            id: 796,
                            parentId: 794,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'GBase_8a_00036.html'
                          },
                          {
                            id: 797,
                            parentId: 794,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'GBase_8a_00037.html'
                          }
                        ]
                      },
                      {
                        id: 795,
                        parentId: 792,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'GBase_8a_00038.html'
                      }
                    ]
                  },
                  {
                    id: 793,
                    parentId: 768,
                    name: 'Archiving GBase 8a Replication Copies',
                    local: 'GBase_8a_00039.html',
                    children: [
                      {
                        id: 798,
                        parentId: 793,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'GBase_8a_00040.html'
                      },
                      {
                        id: 799,
                        parentId: 793,
                        name:
                          'Step 2 Creating a Periodic Archive Policy for Replication Copies',
                        local: 'GBase_8a_00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 769,
                parentId: 41,
                name: 'Restoration',
                local: 'GBase_8a_00042.html',
                children: [
                  {
                    id: 800,
                    parentId: 769,
                    name: 'Restoring a GBase 8a Database',
                    local: 'GBase_8a_00045.html'
                  }
                ]
              },
              {
                id: 770,
                parentId: 41,
                name: 'Global Search',
                local: 'GBase_8a_000411.html',
                children: [
                  {
                    id: 801,
                    parentId: 770,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'GBase_8a_000451.html'
                  }
                ]
              },
              {
                id: 771,
                parentId: 41,
                name: 'SLA ',
                local: 'GBase_8a_00049.html',
                children: [
                  {
                    id: 802,
                    parentId: 771,
                    name: 'Viewing SLA Information',
                    local: 'GBase_8a_00051.html'
                  },
                  {
                    id: 803,
                    parentId: 771,
                    name: 'Managing SLAs',
                    local: 'GBase_8a_00052.html'
                  }
                ]
              },
              {
                id: 772,
                parentId: 41,
                name: 'Copies',
                local: 'GBase_8a_00053.html',
                children: [
                  {
                    id: 804,
                    parentId: 772,
                    name: 'Viewing GBase 8a Copy Details',
                    local: 'GBase_8a_00054.html'
                  },
                  {
                    id: 805,
                    parentId: 772,
                    name: 'Managing GBase 8a Copies',
                    local: 'GBase_8a_00055.html'
                  }
                ]
              },
              {
                id: 773,
                parentId: 41,
                name: 'GBase 8a Database Environment',
                local: 'GBase_8a_00056.html',
                children: [
                  {
                    id: 806,
                    parentId: 773,
                    name:
                      'Viewing the GBase 8a Database Environment Information',
                    local: 'GBase_8a_00057.html'
                  },
                  {
                    id: 807,
                    parentId: 773,
                    name: 'Managing Databases',
                    local: 'GBase_8a_00058.html'
                  }
                ]
              }
            ]
          },
          {
            id: 42,
            parentId: 13,
            name: 'SAP HANA Data Protection',
            local: 'en-us_topic_0000001873679193.html',
            children: [
              {
                id: 808,
                parentId: 42,
                name: 'Backup',
                local: 'SAP_HANA_0008.html',
                children: [
                  {
                    id: 818,
                    parentId: 808,
                    name: 'Preparations for Backup',
                    local: 'SAP_HANA_0011.html'
                  },
                  {
                    id: 819,
                    parentId: 808,
                    name:
                      'Backing Up the SAP HANA Database (General Database Path)',
                    local: 'SAP_HANA_0012.html',
                    children: [
                      {
                        id: 821,
                        parentId: 819,
                        name:
                          'Step 1: Registering the SAP HANA Database (File Backup Mode)',
                        local: 'SAP_HANA_0013.html'
                      },
                      {
                        id: 822,
                        parentId: 819,
                        name: 'Step 2 Creating a Rate Limiting Policy',
                        local: 'SAP_HANA_0014.html'
                      },
                      {
                        id: 823,
                        parentId: 819,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'SAP_HANA_0015.html'
                      },
                      {
                        id: 824,
                        parentId: 819,
                        name: 'Step 4: Configuring Log Backup.',
                        local: 'SAP_HANA_0016.html'
                      },
                      {
                        id: 825,
                        parentId: 819,
                        name: '***',
                        local: 'en-us_topic_0000002058629252.html'
                      },
                      {
                        id: 826,
                        parentId: 819,
                        name: 'Step 6: Performing Backup',
                        local: 'SAP_HANA_00024_b2.html'
                      }
                    ]
                  },
                  {
                    id: 820,
                    parentId: 808,
                    name:
                      'Backing Up the SAP HANA Database (SAP HANA Application Entry) (Only for 1.6.0 and Later Versions)',
                    local: 'SAP_HANA_0019.html',
                    children: [
                      {
                        id: 827,
                        parentId: 820,
                        name:
                          'Step 1: Registering the SAP HANA Database (Backint Backup Mode)',
                        local: 'SAP_HANA_0020.html'
                      },
                      {
                        id: 828,
                        parentId: 820,
                        name: 'Step 2 Creating a Rate Limiting Policy',
                        local: 'SAP_HANA_0022.html'
                      },
                      {
                        id: 829,
                        parentId: 820,
                        name: '***',
                        local: 'en-us_topic_0000002058628320.html'
                      },
                      {
                        id: 830,
                        parentId: 820,
                        name: 'Step 5: Performing Backup',
                        local: 'SAP_HANA_0025.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 809,
                parentId: 42,
                name: 'Replication',
                local: 'SAP_HANA_0028.html',
                children: [
                  {
                    id: 831,
                    parentId: 809,
                    name: 'Replicating an SAP HANA Database Copy',
                    local: 'SAP_HANA_0031.html',
                    children: [
                      {
                        id: 832,
                        parentId: 831,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'SAP_HANA_0033.html'
                      },
                      {
                        id: 833,
                        parentId: 831,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'SAP_HANA_0034.html'
                      },
                      {
                        id: 834,
                        parentId: 831,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'SAP_HANA_0035.html'
                      },
                      {
                        id: 835,
                        parentId: 831,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'SAP_HANA_0036.html'
                      },
                      {
                        id: 836,
                        parentId: 831,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'SAP_HANA_0037.html'
                      },
                      {
                        id: 837,
                        parentId: 831,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'SAP_HANA_0038.html'
                      },
                      {
                        id: 838,
                        parentId: 831,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'SAP_HANA_00037_a1.html'
                      },
                      {
                        id: 839,
                        parentId: 831,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'SAP_HANA_0039.html'
                      },
                      {
                        id: 840,
                        parentId: 831,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'SAP_HANA_0040.html'
                      },
                      {
                        id: 841,
                        parentId: 831,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'SAP_HANA_0041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 810,
                parentId: 42,
                name: 'Archiving',
                local: 'SAP_HANA_0042.html',
                children: [
                  {
                    id: 842,
                    parentId: 810,
                    name: 'Archiving SAP HANA Backup Copies',
                    local: 'SAP_HANA_0045.html',
                    children: [
                      {
                        id: 844,
                        parentId: 842,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'SAP_HANA_0046.html',
                        children: [
                          {
                            id: 846,
                            parentId: 844,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'SAP_HANA_0047.html'
                          },
                          {
                            id: 847,
                            parentId: 844,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'SAP_HANA_0048.html'
                          }
                        ]
                      },
                      {
                        id: 845,
                        parentId: 842,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'SAP_HANA_0049.html'
                      }
                    ]
                  },
                  {
                    id: 843,
                    parentId: 810,
                    name: 'Archiving SAP HANA Replication Copies',
                    local: 'SAP_HANA_0050.html',
                    children: [
                      {
                        id: 848,
                        parentId: 843,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'SAP_HANA_0051.html'
                      },
                      {
                        id: 849,
                        parentId: 843,
                        name:
                          'Step 2 Creating a Periodic Archive Policy for Replication Copies',
                        local: 'SAP_HANA_0052.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 811,
                parentId: 42,
                name: 'Restoration',
                local: 'SAP_HANA_0054.html',
                children: [
                  {
                    id: 850,
                    parentId: 811,
                    name:
                      'Restoring an SAP HANA Database (General Database Path)',
                    local: 'SAP_HANA_0057.html'
                  }
                ]
              },
              {
                id: 812,
                parentId: 42,
                name: 'Global Search',
                local: 'SAP_HANA_00053_a1.html',
                children: [
                  {
                    id: 851,
                    parentId: 812,
                    name: 'Global Search for Resources',
                    local: 'SAP_HANA_00053_a2.html'
                  },
                  {
                    id: 852,
                    parentId: 812,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'SAP_HANA_00053_a3.html'
                  }
                ]
              },
              {
                id: 813,
                parentId: 42,
                name: 'SLA ',
                local: 'SAP_HANA_0063.html',
                children: [
                  {
                    id: 853,
                    parentId: 813,
                    name: 'About SLA',
                    local: 'SAP_HANA_00063_qe.html'
                  },
                  {
                    id: 854,
                    parentId: 813,
                    name: 'Viewing SLA Information',
                    local: 'SAP_HANA_0065.html'
                  },
                  {
                    id: 855,
                    parentId: 813,
                    name: 'Managing SLAs',
                    local: 'SAP_HANA_0066.html'
                  }
                ]
              },
              {
                id: 814,
                parentId: 42,
                name: 'Copies',
                local: 'SAP_HANA_0067.html',
                children: [
                  {
                    id: 856,
                    parentId: 814,
                    name: 'Viewing SAP HANA Copy Details',
                    local: 'SAP_HANA_0068.html'
                  },
                  {
                    id: 857,
                    parentId: 814,
                    name: 'Managing SAP HANA Database Copies',
                    local: 'SAP_HANA_0069.html'
                  }
                ]
              },
              {
                id: 815,
                parentId: 42,
                name: '***',
                local: 'SAP_HANA_00067_as11.html',
                children: [
                  {
                    id: 858,
                    parentId: 815,
                    name: 'Viewing SAP HANA Copy Details',
                    local: 'SAP_HANA_00067_as12.html'
                  },
                  {
                    id: 859,
                    parentId: 815,
                    name: 'Managing SAP HANA Database Copies',
                    local: 'SAP_HANA_00067_as13.html'
                  }
                ]
              },
              {
                id: 816,
                parentId: 42,
                name: 'SAP HANA Database Environment (General Database Path)',
                local: 'SAP_HANA_0070.html',
                children: [
                  {
                    id: 860,
                    parentId: 816,
                    name:
                      'Viewing the SAP HANA Database Environment Information',
                    local: 'SAP_HANA_0071.html'
                  },
                  {
                    id: 861,
                    parentId: 816,
                    name: 'Managing Databases',
                    local: 'SAP_HANA_0072.html'
                  }
                ]
              },
              {
                id: 817,
                parentId: 42,
                name: 'SAP HANA数据库环境（SAP HANA应用入口）',
                local: 'SAP_HANA_0073.html',
                children: [
                  {
                    id: 862,
                    parentId: 817,
                    name: '查看SAP HANA数据库环境信息',
                    local: 'SAP_HANA_0074.html'
                  },
                  {
                    id: 863,
                    parentId: 817,
                    name: '管理实例',
                    local: 'SAP_HANA_0075.html'
                  },
                  {
                    id: 864,
                    parentId: 817,
                    name: '管理数据库的保护',
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
            id: 865,
            parentId: 14,
            name: 'ClickHouse Data Protection',
            local: 'en-us_topic_0000001873759365.html',
            children: [
              {
                id: 873,
                parentId: 865,
                name: 'Backup',
                local: 'clickhouse-0003.html',
                children: [
                  {
                    id: 882,
                    parentId: 873,
                    name: 'Preparing for the Backup',
                    local: 'clickhouse-0006.html'
                  },
                  {
                    id: 883,
                    parentId: 873,
                    name: 'Backing Up the ClickHouse Database and Table Set',
                    local: 'clickhouse-0007.html',
                    children: [
                      {
                        id: 884,
                        parentId: 883,
                        name: 'Step 1: Registering a ClickHouse Cluster',
                        local: 'clickhouse-0008.html'
                      },
                      {
                        id: 885,
                        parentId: 883,
                        name: 'Step 2: Creating a ClickHouse Table Set',
                        local: 'clickhouse-0009.html'
                      },
                      {
                        id: 886,
                        parentId: 883,
                        name: 'Step 3: Creating a Rate Limiting Policy',
                        local: 'clickhouse-0010.html'
                      },
                      {
                        id: 887,
                        parentId: 883,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'clickhouse-0011.html'
                      },
                      {
                        id: 888,
                        parentId: 883,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'clickhouse-0012.html'
                      },
                      {
                        id: 889,
                        parentId: 883,
                        name: 'Step 6: Performing Backup',
                        local: 'clickhouse-0013.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 874,
                parentId: 865,
                name: 'Replication',
                local: 'oracle_gud_000035_7.html',
                children: [
                  {
                    id: 890,
                    parentId: 874,
                    name: 'ClickHouse Replication Copy',
                    local: 'clickhouse-0019.html',
                    children: [
                      {
                        id: 891,
                        parentId: 890,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_8.html'
                      },
                      {
                        id: 892,
                        parentId: 890,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_12.html'
                      },
                      {
                        id: 893,
                        parentId: 890,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'clickhouse-0022.html'
                      },
                      {
                        id: 894,
                        parentId: 890,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'clickhouse-0023.html'
                      },
                      {
                        id: 895,
                        parentId: 890,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'clickhouse-0024.html'
                      },
                      {
                        id: 896,
                        parentId: 890,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'clickhouse-0025.html'
                      },
                      {
                        id: 897,
                        parentId: 890,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002010575970.html'
                      },
                      {
                        id: 898,
                        parentId: 890,
                        name: 'Step : Adding a Target Cluster',
                        local: 'clickhouse-0026.html'
                      },
                      {
                        id: 899,
                        parentId: 890,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'oracle_gud_000041.html'
                      },
                      {
                        id: 900,
                        parentId: 890,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication/Cascading Replication SLA',
                        local: 'clickhouse-0028.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 875,
                parentId: 865,
                name: 'Archiving',
                local: 'clickhouse-0029.html',
                children: [
                  {
                    id: 901,
                    parentId: 875,
                    name: 'Archiving ClickHouse Backup Copies',
                    local: 'clickhouse-0032.html',
                    children: [
                      {
                        id: 903,
                        parentId: 901,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'clickhouse-0033.html',
                        children: [
                          {
                            id: 905,
                            parentId: 903,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'clickhouse-0034.html'
                          },
                          {
                            id: 906,
                            parentId: 903,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'clickhouse-0035.html'
                          }
                        ]
                      },
                      {
                        id: 904,
                        parentId: 901,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'clickhouse-0036.html'
                      }
                    ]
                  },
                  {
                    id: 902,
                    parentId: 875,
                    name: 'Archiving ClickHouse Replication Copies',
                    local: 'clickhouse-0037.html',
                    children: [
                      {
                        id: 907,
                        parentId: 902,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'clickhouse-0038.html'
                      },
                      {
                        id: 908,
                        parentId: 902,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'clickhouse-0039.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 876,
                parentId: 865,
                name: 'Restoration',
                local: 'clickhouse-0040.html',
                children: [
                  {
                    id: 909,
                    parentId: 876,
                    name: 'Restoring a ClickHouse Database or Table Set',
                    local: 'clickhouse-0043.html'
                  }
                ]
              },
              {
                id: 877,
                parentId: 865,
                name: 'Global Search',
                local: 'en-us_topic_0000002038764373.html',
                children: [
                  {
                    id: 910,
                    parentId: 877,
                    name: 'Global Search for Resources',
                    local: 'clickhouse-0044.html'
                  },
                  {
                    id: 911,
                    parentId: 877,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002844410.html'
                  }
                ]
              },
              {
                id: 878,
                parentId: 865,
                name: 'Data Deduplication and Compression',
                local: 'clickhouse-0045.html',
                children: [
                  {
                    id: 912,
                    parentId: 878,
                    name: 'About Data Deduplication and Compression',
                    local: 'clickhouse-0046.html'
                  }
                ]
              },
              {
                id: 879,
                parentId: 865,
                name: 'SLA',
                local: 'clickhouse-0047.html',
                children: [
                  {
                    id: 913,
                    parentId: 879,
                    name: 'About SLA',
                    local: 'clickhouse-0048.html'
                  },
                  {
                    id: 914,
                    parentId: 879,
                    name: 'Viewing SLA Information',
                    local: 'clickhouse-0049.html'
                  },
                  {
                    id: 915,
                    parentId: 879,
                    name: 'Managing SLAs',
                    local: 'clickhouse-0050.html'
                  }
                ]
              },
              {
                id: 880,
                parentId: 865,
                name: 'Copies',
                local: 'clickhouse-0051.html',
                children: [
                  {
                    id: 916,
                    parentId: 880,
                    name: 'Viewing ClickHouse Copy Information',
                    local: 'clickhouse-0052.html'
                  },
                  {
                    id: 917,
                    parentId: 880,
                    name: 'Managing ClickHouse Copies',
                    local: 'clickhouse-0053.html'
                  }
                ]
              },
              {
                id: 881,
                parentId: 865,
                name: 'ClickHouse Cluster Environment',
                local: 'clickhouse-0054.html',
                children: [
                  {
                    id: 918,
                    parentId: 881,
                    name: 'Querying ClickHouse Information',
                    local: 'clickhouse-0055.html'
                  },
                  {
                    id: 919,
                    parentId: 881,
                    name: 'Managing ClickHouse Clusters/Table Sets',
                    local: 'clickhouse-0056.html'
                  },
                  {
                    id: 920,
                    parentId: 881,
                    name: 'Managing ClickHouse Database/Table Set Protection',
                    local: 'clickhouse-0057.html'
                  }
                ]
              }
            ]
          },
          {
            id: 866,
            parentId: 14,
            name: 'GaussDB (DWS) Data Protection',
            local: 'product_documentation_000029.html',
            children: [
              {
                id: 921,
                parentId: 866,
                name: 'Backup',
                local: 'DWS_00006.html',
                children: [
                  {
                    id: 930,
                    parentId: 921,
                    name: 'Preparing for the Backup',
                    local: 'DWS_00009.html'
                  },
                  {
                    id: 931,
                    parentId: 921,
                    name: 'Backing Up GaussDB (DWS)',
                    local: 'DWS_00010.html',
                    children: [
                      {
                        id: 932,
                        parentId: 931,
                        name: 'Step 1: Registering a GaussDB (DWS) Cluster',
                        local: 'DWS_00014.html'
                      },
                      {
                        id: 933,
                        parentId: 931,
                        name:
                          'Step 2: Creating a GaussDB (DWS) Schema Set/Table Set',
                        local: 'DWS_00015.html'
                      },
                      {
                        id: 934,
                        parentId: 931,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'DWS_00016.html'
                      },
                      {
                        id: 935,
                        parentId: 931,
                        name: 'Step 4: Create a Rate Limiting Policy.',
                        local: 'DWS_00017.html'
                      },
                      {
                        id: 936,
                        parentId: 931,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'DWS_00018.html'
                      },
                      {
                        id: 937,
                        parentId: 931,
                        name: 'Step 6: Performing Backup',
                        local: 'DWS_00019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 922,
                parentId: 866,
                name: 'Replication',
                local: 'DWS_00022.html',
                children: [
                  {
                    id: 938,
                    parentId: 922,
                    name: 'Replicating a GaussDB (DWS) Backup Copies',
                    local: 'DWS_00024.html',
                    children: [
                      {
                        id: 939,
                        parentId: 938,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'DWS_00026.html'
                      },
                      {
                        id: 940,
                        parentId: 938,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_18.html'
                      },
                      {
                        id: 941,
                        parentId: 938,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'DWS_00027.html'
                      },
                      {
                        id: 942,
                        parentId: 938,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'DWS_00028.html'
                      },
                      {
                        id: 943,
                        parentId: 938,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'DWS_00029.html'
                      },
                      {
                        id: 944,
                        parentId: 938,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'DWS_00030.html'
                      },
                      {
                        id: 945,
                        parentId: 938,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'DWS_00031_a1.html'
                      },
                      {
                        id: 946,
                        parentId: 938,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'DWS_00031.html'
                      },
                      {
                        id: 947,
                        parentId: 938,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'DWS_00031_b1.html'
                      },
                      {
                        id: 948,
                        parentId: 938,
                        name:
                          'Step 8: (Optional) Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'DWS_00033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 923,
                parentId: 866,
                name: 'Archiving',
                local: 'DWS_00034.html',
                children: [
                  {
                    id: 949,
                    parentId: 923,
                    name: 'Archiving GaussDB (DWS) Backup Copies',
                    local: 'DWS_00037.html',
                    children: [
                      {
                        id: 951,
                        parentId: 949,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'DWS_00038.html',
                        children: [
                          {
                            id: 953,
                            parentId: 951,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'DWS_00039.html'
                          },
                          {
                            id: 954,
                            parentId: 951,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'DWS_00040.html'
                          }
                        ]
                      },
                      {
                        id: 952,
                        parentId: 949,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'DWS_00041.html'
                      }
                    ]
                  },
                  {
                    id: 950,
                    parentId: 923,
                    name: 'Archiving GaussDB (DWS) Replication Copies',
                    local: 'DWS_00042.html',
                    children: [
                      {
                        id: 955,
                        parentId: 950,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'DWS_00043.html'
                      },
                      {
                        id: 956,
                        parentId: 950,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'DWS_00044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 924,
                parentId: 866,
                name: 'Restoration',
                local: 'DWS_00045.html',
                children: [
                  {
                    id: 957,
                    parentId: 924,
                    name: 'Restoring the GaussDB (DWS)',
                    local: 'DWS_00048.html'
                  }
                ]
              },
              {
                id: 925,
                parentId: 866,
                name: 'Global Search',
                local: 'DWS_00045_a1.html',
                children: [
                  {
                    id: 958,
                    parentId: 925,
                    name: 'Global Search for Resources',
                    local: 'DWS_00045_a2.html'
                  },
                  {
                    id: 959,
                    parentId: 925,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'DWS_00045_a3.html'
                  }
                ]
              },
              {
                id: 926,
                parentId: 866,
                name: 'Data Deduplication and Compression',
                local: 'DWS_00050.html',
                children: [
                  {
                    id: 960,
                    parentId: 926,
                    name: 'About Data Deduplication and Compression',
                    local: 'DWS_00051.html'
                  }
                ]
              },
              {
                id: 927,
                parentId: 866,
                name: 'SLA',
                local: 'DWS_00052.html',
                children: [
                  {
                    id: 961,
                    parentId: 927,
                    name: 'About SLA',
                    local: 'DWS_00053.html'
                  },
                  {
                    id: 962,
                    parentId: 927,
                    name: 'Viewing SLA Information',
                    local: 'DWS_00054.html'
                  },
                  {
                    id: 963,
                    parentId: 927,
                    name: 'Managing SLAs',
                    local: 'DWS_00055.html'
                  }
                ]
              },
              {
                id: 928,
                parentId: 866,
                name: 'Copies',
                local: 'DWS_00056.html',
                children: [
                  {
                    id: 964,
                    parentId: 928,
                    name: 'Viewing GaussDB (DWS) Copy Information',
                    local: 'DWS_00057.html'
                  },
                  {
                    id: 965,
                    parentId: 928,
                    name: 'Managing GaussDB (DWS) Copies',
                    local: 'DWS_00058.html'
                  }
                ]
              },
              {
                id: 929,
                parentId: 866,
                name: 'GaussDB (DWS) Cluster Environment',
                local: 'DWS_00059.html',
                children: [
                  {
                    id: 966,
                    parentId: 929,
                    name: 'Querying GaussDB (DWS) Information',
                    local: 'DWS_00060.html'
                  },
                  {
                    id: 967,
                    parentId: 929,
                    name: 'Managing the GaussDB (DWS) Cluster',
                    local: 'DWS_00061.html'
                  },
                  {
                    id: 968,
                    parentId: 929,
                    name: 'Managing GaussDB (DWS)',
                    local: 'DWS_00062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 867,
            parentId: 14,
            name: 'HBase Data Protection',
            local: 'product_documentation_000033.html',
            children: [
              {
                id: 969,
                parentId: 867,
                name: 'Backup',
                local: 'hbase_00007.html',
                children: [
                  {
                    id: 977,
                    parentId: 969,
                    name: 'Preparing for the Backup',
                    local: 'hbase_00010.html'
                  },
                  {
                    id: 978,
                    parentId: 969,
                    name: 'Backing Up HBase Backup Sets',
                    local: 'hbase_00011.html',
                    children: [
                      {
                        id: 979,
                        parentId: 978,
                        name: 'Step 1: Registering an HBase Cluster',
                        local: 'hbase_00012.html'
                      },
                      {
                        id: 980,
                        parentId: 978,
                        name: 'Step 2: Creating an HBase Backup Set',
                        local: 'hbase_00013.html'
                      },
                      {
                        id: 981,
                        parentId: 978,
                        name: 'Step 3: Creating a Rate Limiting Policy',
                        local: 'hbase_00014.html'
                      },
                      {
                        id: 982,
                        parentId: 978,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'hbase_00015.html'
                      },
                      {
                        id: 983,
                        parentId: 978,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'hbase_00016.html'
                      },
                      {
                        id: 984,
                        parentId: 978,
                        name: 'Step 6: Performing Backup',
                        local: 'hbase_00017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 970,
                parentId: 867,
                name: 'Replication',
                local: 'hbase_00020.html',
                children: [
                  {
                    id: 985,
                    parentId: 970,
                    name: 'Replicating HBase Copies',
                    local: 'hbase_00023.html',
                    children: [
                      {
                        id: 986,
                        parentId: 985,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'hbase_00025.html'
                      },
                      {
                        id: 987,
                        parentId: 985,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'hbase_00026_a1.html'
                      },
                      {
                        id: 988,
                        parentId: 985,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'hbase_00026.html'
                      },
                      {
                        id: 989,
                        parentId: 985,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'hbase_00028_a1.html'
                      },
                      {
                        id: 990,
                        parentId: 985,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'hbase_00028.html'
                      },
                      {
                        id: 991,
                        parentId: 985,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'hbase_00029.html'
                      },
                      {
                        id: 992,
                        parentId: 985,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002046619645.html'
                      },
                      {
                        id: 993,
                        parentId: 985,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'hbase_00030.html'
                      },
                      {
                        id: 994,
                        parentId: 985,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'hbase_00031.html'
                      },
                      {
                        id: 995,
                        parentId: 985,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'hbase_00032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 971,
                parentId: 867,
                name: 'Archiving',
                local: 'hbase_00033.html',
                children: [
                  {
                    id: 996,
                    parentId: 971,
                    name: 'Archiving HBase Backup Copies',
                    local: 'hbase_00036.html',
                    children: [
                      {
                        id: 998,
                        parentId: 996,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'hbase_00037.html',
                        children: [
                          {
                            id: 1000,
                            parentId: 998,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'hbase_00038.html'
                          },
                          {
                            id: 1001,
                            parentId: 998,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'hbase_00039.html'
                          }
                        ]
                      },
                      {
                        id: 999,
                        parentId: 996,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'hbase_00040.html'
                      }
                    ]
                  },
                  {
                    id: 997,
                    parentId: 971,
                    name: 'Archiving HBase Replication Copies',
                    local: 'hbase_00041.html',
                    children: [
                      {
                        id: 1002,
                        parentId: 997,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'hbase_00042.html'
                      },
                      {
                        id: 1003,
                        parentId: 997,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'hbase_00043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 972,
                parentId: 867,
                name: 'Restoration',
                local: 'hbase_00044.html',
                children: [
                  {
                    id: 1004,
                    parentId: 972,
                    name: 'Restoring HBase Backup Sets',
                    local: 'hbase_00048_a1.html'
                  },
                  {
                    id: 1005,
                    parentId: 972,
                    name:
                      'Restoring One or Multiple Tables in an HBase Backup Set',
                    local: 'hbase_00048.html'
                  }
                ]
              },
              {
                id: 973,
                parentId: 867,
                name: 'Global Search',
                local: 'hbase_00049.html',
                children: [
                  {
                    id: 1006,
                    parentId: 973,
                    name: 'Global Search for Resources',
                    local: 'hbase_00050.html'
                  },
                  {
                    id: 1007,
                    parentId: 973,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002038765521.html'
                  }
                ]
              },
              {
                id: 974,
                parentId: 867,
                name: 'SLA',
                local: 'hbase_00053.html',
                children: [
                  {
                    id: 1008,
                    parentId: 974,
                    name: 'Viewing SLA Information',
                    local: 'hbase_00055.html'
                  },
                  {
                    id: 1009,
                    parentId: 974,
                    name: 'Managing SLAs',
                    local: 'hbase_00056.html'
                  }
                ]
              },
              {
                id: 975,
                parentId: 867,
                name: 'Copies',
                local: 'hbase_00057.html',
                children: [
                  {
                    id: 1010,
                    parentId: 975,
                    name: 'Viewing HBase Copy Information',
                    local: 'hbase_00060_a1.html'
                  },
                  {
                    id: 1011,
                    parentId: 975,
                    name: 'Managing HBase Copies',
                    local: 'hbase_00059.html'
                  }
                ]
              },
              {
                id: 976,
                parentId: 867,
                name: 'HBase Cluster Environment',
                local: 'hbase_00060.html',
                children: [
                  {
                    id: 1012,
                    parentId: 976,
                    name: 'Querying HBase Information',
                    local: 'hbase_00061.html'
                  },
                  {
                    id: 1013,
                    parentId: 976,
                    name: 'Managing HBase Clusters',
                    local: 'hbase_00064_a1.html'
                  },
                  {
                    id: 1014,
                    parentId: 976,
                    name: 'Managing Backup Set Protection',
                    local: 'hbase_00065_a1.html'
                  }
                ]
              }
            ]
          },
          {
            id: 868,
            parentId: 14,
            name: 'Hive Data Protection',
            local: 'en-us_topic_0000001827039684.html',
            children: [
              {
                id: 1015,
                parentId: 868,
                name: 'Overview',
                local: 'hive_00003.html',
                children: [
                  {
                    id: 1024,
                    parentId: 1015,
                    name: 'Function Overview',
                    local: 'hive_00005.html'
                  },
                  {
                    id: 1025,
                    parentId: 1015,
                    name: 'Constraints',
                    local: 'hive_00006.html'
                  }
                ]
              },
              {
                id: 1016,
                parentId: 868,
                name: 'Backup',
                local: 'hive_00008.html',
                children: [
                  {
                    id: 1026,
                    parentId: 1016,
                    name: 'Preparing for the Backup',
                    local: 'hive_00011.html'
                  },
                  {
                    id: 1027,
                    parentId: 1016,
                    name: 'Backing Up Hive Backup Sets',
                    local: 'hive_00012.html',
                    children: [
                      {
                        id: 1028,
                        parentId: 1027,
                        name:
                          'Step 1: Enabling the Snapshot Function for the Directory Where the Database Table Resides',
                        local: 'hive_00013.html'
                      },
                      {
                        id: 1029,
                        parentId: 1027,
                        name:
                          'Step 2: (Optional) Generating and Obtaining a Certificate',
                        local: 'hive_00015.html'
                      },
                      {
                        id: 1030,
                        parentId: 1027,
                        name: 'Step 3: Registering a Hive Cluster',
                        local: 'hive_00016.html'
                      },
                      {
                        id: 1031,
                        parentId: 1027,
                        name: 'Step 4: Creating a Hive Backup Set',
                        local: 'hive_00017.html'
                      },
                      {
                        id: 1032,
                        parentId: 1027,
                        name: 'Step 5: Creating a Rate Limiting Policy',
                        local: 'hive_00018.html'
                      },
                      {
                        id: 1033,
                        parentId: 1027,
                        name:
                          'Step 6: (Optional) Enabling Backup Link Encryption',
                        local: 'hive_00018_a1.html'
                      },
                      {
                        id: 1034,
                        parentId: 1027,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'hive_00020.html'
                      },
                      {
                        id: 1035,
                        parentId: 1027,
                        name: 'Step 8: Performing Backup',
                        local: 'hive_00021.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1017,
                parentId: 868,
                name: 'Replication',
                local: 'hive_00024.html',
                children: [
                  {
                    id: 1036,
                    parentId: 1017,
                    name: 'Replicating Hive Copies',
                    local: 'hive_00027.html',
                    children: [
                      {
                        id: 1037,
                        parentId: 1036,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'hive_00029.html'
                      },
                      {
                        id: 1038,
                        parentId: 1036,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'hive_00030.html'
                      },
                      {
                        id: 1039,
                        parentId: 1036,
                        name: '(Optional) Step 2: Creating an IPsec Policy ',
                        local: 'hive_00031.html'
                      },
                      {
                        id: 1040,
                        parentId: 1036,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'hive_00032.html'
                      },
                      {
                        id: 1041,
                        parentId: 1036,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'hive_00033.html'
                      },
                      {
                        id: 1042,
                        parentId: 1036,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'hive_00034.html'
                      },
                      {
                        id: 1043,
                        parentId: 1036,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002010579886.html'
                      },
                      {
                        id: 1044,
                        parentId: 1036,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'hive_00035.html'
                      },
                      {
                        id: 1045,
                        parentId: 1036,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'hive_00036.html'
                      },
                      {
                        id: 1046,
                        parentId: 1036,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'hive_00037.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1018,
                parentId: 868,
                name: 'Archiving',
                local: 'hive_00038.html',
                children: [
                  {
                    id: 1047,
                    parentId: 1018,
                    name: 'Archiving Hive Backup Copies',
                    local: 'hive_00041.html',
                    children: [
                      {
                        id: 1049,
                        parentId: 1047,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'hive_00042.html',
                        children: [
                          {
                            id: 1051,
                            parentId: 1049,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'hive_00043.html'
                          },
                          {
                            id: 1052,
                            parentId: 1049,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'hive_00044.html'
                          }
                        ]
                      },
                      {
                        id: 1050,
                        parentId: 1047,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'hive_00045.html'
                      }
                    ]
                  },
                  {
                    id: 1048,
                    parentId: 1018,
                    name: 'Archiving Hive Replication Copies',
                    local: 'hive_00046.html',
                    children: [
                      {
                        id: 1053,
                        parentId: 1048,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'hive_00047.html'
                      },
                      {
                        id: 1054,
                        parentId: 1048,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'hive_00048.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1019,
                parentId: 868,
                name: 'Restoration',
                local: 'hive_00049.html',
                children: [
                  {
                    id: 1055,
                    parentId: 1019,
                    name: 'Restoring Hive Backup Sets',
                    local: 'hive_00051.html'
                  },
                  {
                    id: 1056,
                    parentId: 1019,
                    name:
                      'Restoring One or Multiple Tables in a Hive Backup Set',
                    local: 'hive_00052.html'
                  }
                ]
              },
              {
                id: 1020,
                parentId: 868,
                name: 'Global Search',
                local: 'hive_00053.html',
                children: [
                  {
                    id: 1057,
                    parentId: 1020,
                    name: 'Global Search for Resources',
                    local: 'hive_00054.html'
                  },
                  {
                    id: 1058,
                    parentId: 1020,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002845722.html'
                  }
                ]
              },
              {
                id: 1021,
                parentId: 868,
                name: 'SLA',
                local: 'hive_00057.html',
                children: [
                  {
                    id: 1059,
                    parentId: 1021,
                    name: 'Viewing SLA Information',
                    local: 'hive_00059.html'
                  },
                  {
                    id: 1060,
                    parentId: 1021,
                    name: 'Managing SLAs',
                    local: 'hive_00060.html'
                  }
                ]
              },
              {
                id: 1022,
                parentId: 868,
                name: 'Copies',
                local: 'hive_00061.html',
                children: [
                  {
                    id: 1061,
                    parentId: 1022,
                    name: 'Querying Hive Copy Information',
                    local: 'hive_00062.html'
                  },
                  {
                    id: 1062,
                    parentId: 1022,
                    name: 'Managing Hive Copies',
                    local: 'hive_00063.html'
                  }
                ]
              },
              {
                id: 1023,
                parentId: 868,
                name: 'Hive Cluster Environment',
                local: 'hive_00064.html',
                children: [
                  {
                    id: 1063,
                    parentId: 1023,
                    name: 'Querying Hive Information',
                    local: 'hive_00065.html'
                  },
                  {
                    id: 1064,
                    parentId: 1023,
                    name: 'Managing Hive Clusters',
                    local: 'hive_00066.html'
                  },
                  {
                    id: 1065,
                    parentId: 1023,
                    name: 'Managing Backup Sets',
                    local: 'hive_00067.html'
                  }
                ]
              }
            ]
          },
          {
            id: 869,
            parentId: 14,
            name: 'MongoDB Data Protection',
            local: 'en-us_topic_0000001873679221.html',
            children: [
              {
                id: 1066,
                parentId: 869,
                name: 'Backup',
                local: 'mongodb-0007.html',
                children: [
                  {
                    id: 1074,
                    parentId: 1066,
                    name: 'Preparations for Backup',
                    local: 'mongodb-0010.html'
                  },
                  {
                    id: 1075,
                    parentId: 1066,
                    name: 'Backing Up MongoDB Databases',
                    local: 'mongodb-0011.html',
                    children: [
                      {
                        id: 1076,
                        parentId: 1075,
                        name: 'Step 1: Registering a MongoDB Instance',
                        local: 'mongodb-0012.html'
                      },
                      {
                        id: 1077,
                        parentId: 1075,
                        name:
                          'Step 2: (Optional) Creating a Rate Limiting Policy',
                        local: 'mongodb-0013.html'
                      },
                      {
                        id: 1078,
                        parentId: 1075,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'mongodb-0014.html'
                      },
                      {
                        id: 1079,
                        parentId: 1075,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'mongodb-0015.html'
                      },
                      {
                        id: 1080,
                        parentId: 1075,
                        name: 'Step 5: Executing Backup',
                        local: 'mongodb-0016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1067,
                parentId: 869,
                name: 'Replication',
                local: 'oracle_gud_000035_6.html',
                children: [
                  {
                    id: 1081,
                    parentId: 1067,
                    name: 'Replicating MongoDB Copies',
                    local: 'mongodb-0022.html',
                    children: [
                      {
                        id: 1082,
                        parentId: 1081,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_7.html'
                      },
                      {
                        id: 1083,
                        parentId: 1081,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_11.html'
                      },
                      {
                        id: 1084,
                        parentId: 1081,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'mongodb-0025.html'
                      },
                      {
                        id: 1085,
                        parentId: 1081,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'mongodb-0026.html'
                      },
                      {
                        id: 1086,
                        parentId: 1081,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'mongodb-0027.html'
                      },
                      {
                        id: 1087,
                        parentId: 1081,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'mongodb-0028.html'
                      },
                      {
                        id: 1088,
                        parentId: 1081,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002010413522.html'
                      },
                      {
                        id: 1089,
                        parentId: 1081,
                        name: 'Step : Adding a Target Cluster',
                        local: 'mongodb-0029.html'
                      },
                      {
                        id: 1090,
                        parentId: 1081,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'mongodb-0030.html'
                      },
                      {
                        id: 1091,
                        parentId: 1081,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'mongodb-0031.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1068,
                parentId: 869,
                name: 'Archiving',
                local: 'mongodb-0032.html',
                children: [
                  {
                    id: 1092,
                    parentId: 1068,
                    name: 'Archiving MongoDB Backup Copies',
                    local: 'mongodb-0035.html',
                    children: [
                      {
                        id: 1094,
                        parentId: 1092,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'mongodb-0036.html',
                        children: [
                          {
                            id: 1096,
                            parentId: 1094,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'mongodb-0037.html'
                          },
                          {
                            id: 1097,
                            parentId: 1094,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'mongodb-0038.html'
                          }
                        ]
                      },
                      {
                        id: 1095,
                        parentId: 1092,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'mongodb-0039.html'
                      }
                    ]
                  },
                  {
                    id: 1093,
                    parentId: 1068,
                    name: 'Archiving MongoDB Replication Copies',
                    local: 'mongodb-0040.html',
                    children: [
                      {
                        id: 1098,
                        parentId: 1093,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'mongodb-0041.html'
                      },
                      {
                        id: 1099,
                        parentId: 1093,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'mongodb-0042.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1069,
                parentId: 869,
                name: 'Restoration',
                local: 'mongodb-0043.html',
                children: [
                  {
                    id: 1100,
                    parentId: 1069,
                    name: 'Restoring MongoDB',
                    local: 'mongodb-0046.html'
                  }
                ]
              },
              {
                id: 1070,
                parentId: 869,
                name: 'Global Search',
                local: 'mongodb-0047.html',
                children: [
                  {
                    id: 1101,
                    parentId: 1070,
                    name: 'Global Search for Resources',
                    local: 'mongodb-0048.html'
                  },
                  {
                    id: 1102,
                    parentId: 1070,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002841954.html'
                  }
                ]
              },
              {
                id: 1071,
                parentId: 869,
                name: 'SLA ',
                local: 'en-us_topic_0000001954665865.html',
                children: [
                  {
                    id: 1103,
                    parentId: 1071,
                    name: 'About SLA',
                    local: 'vmware_gud_000026_0.html'
                  },
                  {
                    id: 1104,
                    parentId: 1071,
                    name: 'Viewing SLA Information',
                    local: 'en-us_topic_0000001927506528.html'
                  },
                  {
                    id: 1105,
                    parentId: 1071,
                    name: 'Managing SLAs',
                    local: 'en-us_topic_0000001927347172.html'
                  }
                ]
              },
              {
                id: 1072,
                parentId: 869,
                name: 'Copies',
                local: 'mongodb-0055.html',
                children: [
                  {
                    id: 1106,
                    parentId: 1072,
                    name: 'Viewing MongoDB Copy Information',
                    local: 'mongodb-0056.html'
                  },
                  {
                    id: 1107,
                    parentId: 1072,
                    name: 'Managing MongoDB Copies',
                    local: 'mongodb-0057.html'
                  }
                ]
              },
              {
                id: 1073,
                parentId: 869,
                name: 'MongoDB Environment',
                local: 'mongodb-0058.html',
                children: [
                  {
                    id: 1108,
                    parentId: 1073,
                    name: 'Viewing the MongoDB Environment',
                    local: 'mongodb-0059.html'
                  },
                  {
                    id: 1109,
                    parentId: 1073,
                    name: 'Managing MongoDB',
                    local: 'mongodb-0060.html'
                  }
                ]
              }
            ]
          },
          {
            id: 870,
            parentId: 14,
            name: 'Elasticsearch Data Protection',
            local: 'en-us_topic_0000001873759397.html',
            children: [
              {
                id: 1110,
                parentId: 870,
                name: 'Backup',
                local: 'ES_gud_00008.html',
                children: [
                  {
                    id: 1118,
                    parentId: 1110,
                    name: 'Preparing for the Backup',
                    local: 'ES_gud_00011.html'
                  },
                  {
                    id: 1119,
                    parentId: 1110,
                    name: 'Backing Up an Elasticsearch Cluster',
                    local: 'ES_gud_00012.html',
                    children: [
                      {
                        id: 1120,
                        parentId: 1119,
                        name:
                          'Step 1: (Optional) Enabling the Security Encryption Mode',
                        local: 'ES_gud_00013.html'
                      },
                      {
                        id: 1121,
                        parentId: 1119,
                        name:
                          'Step 2: Creating and Configuring a Mount Directory',
                        local: 'ES_gud_00014.html'
                      },
                      {
                        id: 1122,
                        parentId: 1119,
                        name: 'Step 3: Registering an Elasticsearch Cluster',
                        local: 'ES_gud_00015.html'
                      },
                      {
                        id: 1123,
                        parentId: 1119,
                        name: 'Step 4: Creating an Elasticsearch Backup Set',
                        local: 'ES_gud_00016.html'
                      },
                      {
                        id: 1124,
                        parentId: 1119,
                        name: 'Step 5: Creating a Rate Limiting Policy',
                        local: 'ES_gud_00017.html'
                      },
                      {
                        id: 1125,
                        parentId: 1119,
                        name:
                          'Step 6: (Optional) Enabling Backup Link Encryption',
                        local: 'ES_gud_00018.html'
                      },
                      {
                        id: 1126,
                        parentId: 1119,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'ES_gud_00019.html'
                      },
                      {
                        id: 1127,
                        parentId: 1119,
                        name: 'Step 8: Performing Backup',
                        local: 'ES_gud_00020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1111,
                parentId: 870,
                name: 'Replication',
                local: 'ES_gud_00023.html',
                children: [
                  {
                    id: 1128,
                    parentId: 1111,
                    name: 'Replicating an Elasticsearch Copy',
                    local: 'ES_gud_00026.html',
                    children: [
                      {
                        id: 1129,
                        parentId: 1128,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'ES_gud_00028.html'
                      },
                      {
                        id: 1130,
                        parentId: 1128,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'ES_gud_00029.html'
                      },
                      {
                        id: 1131,
                        parentId: 1128,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'ES_gud_00030.html'
                      },
                      {
                        id: 1132,
                        parentId: 1128,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'ES_gud_00029_1a.html'
                      },
                      {
                        id: 1133,
                        parentId: 1128,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'ES_gud_00032.html'
                      },
                      {
                        id: 1134,
                        parentId: 1128,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'ES_gud_00033.html'
                      },
                      {
                        id: 1135,
                        parentId: 1128,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002046700657.html'
                      },
                      {
                        id: 1136,
                        parentId: 1128,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'ES_gud_00034.html'
                      },
                      {
                        id: 1137,
                        parentId: 1128,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'ES_gud_00035.html'
                      },
                      {
                        id: 1138,
                        parentId: 1128,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'ES_gud_00036.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1112,
                parentId: 870,
                name: 'Archiving',
                local: 'ES_gud_00037.html',
                children: [
                  {
                    id: 1139,
                    parentId: 1112,
                    name: 'Archiving Elasticsearch Backup Copies',
                    local: 'ES_gud_00040.html',
                    children: [
                      {
                        id: 1141,
                        parentId: 1139,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'ES_gud_00041.html',
                        children: [
                          {
                            id: 1143,
                            parentId: 1141,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'ES_gud_00042.html'
                          },
                          {
                            id: 1144,
                            parentId: 1141,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'ES_gud_00043.html'
                          }
                        ]
                      },
                      {
                        id: 1142,
                        parentId: 1139,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'ES_gud_00044.html'
                      }
                    ]
                  },
                  {
                    id: 1140,
                    parentId: 1112,
                    name: 'Archiving Elasticsearch Replication Copies',
                    local: 'ES_gud_00045.html',
                    children: [
                      {
                        id: 1145,
                        parentId: 1140,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'ES_gud_00046.html'
                      },
                      {
                        id: 1146,
                        parentId: 1140,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'ES_gud_00047.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1113,
                parentId: 870,
                name: 'Restoration',
                local: 'ES_gud_00048.html',
                children: [
                  {
                    id: 1147,
                    parentId: 1113,
                    name: 'Restoring an Elasticsearch Backup Set',
                    local: 'ES_gud_00051.html'
                  },
                  {
                    id: 1148,
                    parentId: 1113,
                    name:
                      'Restoring One or Multiple Indexes in an Elasticsearch Backup Set',
                    local: 'ES_gud_00052.html'
                  }
                ]
              },
              {
                id: 1114,
                parentId: 870,
                name: 'Global Search',
                local: 'en-us_topic_0000002014774340.html',
                children: [
                  {
                    id: 1149,
                    parentId: 1114,
                    name: 'Global Search for Resources',
                    local: 'oracle_gud_000045.html'
                  },
                  {
                    id: 1150,
                    parentId: 1114,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002014774344.html'
                  }
                ]
              },
              {
                id: 1115,
                parentId: 870,
                name: 'SLA',
                local: 'ES_gud_00057.html',
                children: [
                  {
                    id: 1151,
                    parentId: 1115,
                    name: 'Viewing SLA Information',
                    local: 'ES_gud_00059.html'
                  },
                  {
                    id: 1152,
                    parentId: 1115,
                    name: 'Managing SLAs',
                    local: 'ES_gud_00060.html'
                  }
                ]
              },
              {
                id: 1116,
                parentId: 870,
                name: 'Copies',
                local: 'ES_gud_00061.html',
                children: [
                  {
                    id: 1153,
                    parentId: 1116,
                    name: 'Querying Elasticsearch Copy Information',
                    local: 'ES_gud_00062.html'
                  },
                  {
                    id: 1154,
                    parentId: 1116,
                    name: 'Managing Elasticsearch Copies',
                    local: 'ES_gud_00063.html'
                  }
                ]
              },
              {
                id: 1117,
                parentId: 870,
                name: 'Elasticsearch Cluster Environment',
                local: 'ES_gud_00063_a1.html',
                children: [
                  {
                    id: 1155,
                    parentId: 1117,
                    name: 'Querying Elasticsearch Information',
                    local: 'ES_gud_00065.html'
                  },
                  {
                    id: 1156,
                    parentId: 1117,
                    name: 'Managing Elasticsearch Clusters',
                    local: 'ES_gud_00066.html'
                  },
                  {
                    id: 1157,
                    parentId: 1117,
                    name: 'Managing Backup Sets',
                    local: 'ES_gud_00067_a1.html'
                  }
                ]
              }
            ]
          },
          {
            id: 871,
            parentId: 14,
            name: 'HDFS Data Protection',
            local: 'product_documentation_000031.html',
            children: [
              {
                id: 1158,
                parentId: 871,
                name: 'Backup',
                local: 'hdfs_00008.html',
                children: [
                  {
                    id: 1166,
                    parentId: 1158,
                    name: 'Preparing for the Backup',
                    local: 'hdfs_00011.html'
                  },
                  {
                    id: 1167,
                    parentId: 1158,
                    name: 'Backing Up HDFS Filesets',
                    local: 'hdfs_00012.html',
                    children: [
                      {
                        id: 1168,
                        parentId: 1167,
                        name:
                          'Step 1: Enabling the Snapshot Function for the HDFS Directory',
                        local: 'hdfs_00013.html'
                      },
                      {
                        id: 1169,
                        parentId: 1167,
                        name: 'Step 2: Checking the HDFS ACL Switch Status',
                        local: 'hdfs_00014.html'
                      },
                      {
                        id: 1170,
                        parentId: 1167,
                        name: 'Step 3: Registering an HDFS Cluster',
                        local: 'hdfs_00015.html'
                      },
                      {
                        id: 1171,
                        parentId: 1167,
                        name: 'Step 4: Creating an HDFS Fileset',
                        local: 'hdfs_00016.html'
                      },
                      {
                        id: 1172,
                        parentId: 1167,
                        name: 'Step 5: Creating a Rate Limiting Policy',
                        local: 'hdfs_00017.html'
                      },
                      {
                        id: 1173,
                        parentId: 1167,
                        name:
                          'Step 6: (Optional) Enabling Backup Link Encryption',
                        local: 'hdfs_00018.html'
                      },
                      {
                        id: 1174,
                        parentId: 1167,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'hdfs_00019.html'
                      },
                      {
                        id: 1175,
                        parentId: 1167,
                        name: 'Step 8: Performing Backup',
                        local: 'hdfs_00020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1159,
                parentId: 871,
                name: 'Replication',
                local: 'hdfs_00023.html',
                children: [
                  {
                    id: 1176,
                    parentId: 1159,
                    name: 'Replicating HDFS Copies',
                    local: 'hdfs_00026.html',
                    children: [
                      {
                        id: 1177,
                        parentId: 1176,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'hdfs_00028.html'
                      },
                      {
                        id: 1178,
                        parentId: 1176,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'hdfs_00029.html'
                      },
                      {
                        id: 1179,
                        parentId: 1176,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'hdfs_00030.html'
                      },
                      {
                        id: 1180,
                        parentId: 1176,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'hdfs_00031.html'
                      },
                      {
                        id: 1181,
                        parentId: 1176,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'hdfs_00032.html'
                      },
                      {
                        id: 1182,
                        parentId: 1176,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'hdfs_00033.html'
                      },
                      {
                        id: 1183,
                        parentId: 1176,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002046697729.html'
                      },
                      {
                        id: 1184,
                        parentId: 1176,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'hdfs_00034.html'
                      },
                      {
                        id: 1185,
                        parentId: 1176,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'hdfs_00035.html'
                      },
                      {
                        id: 1186,
                        parentId: 1176,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'hdfs_00036.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1160,
                parentId: 871,
                name: 'Archiving',
                local: 'hdfs_00037.html',
                children: [
                  {
                    id: 1187,
                    parentId: 1160,
                    name: 'Archiving HDFS Backup Copies',
                    local: 'hdfs_00040.html',
                    children: [
                      {
                        id: 1189,
                        parentId: 1187,
                        name: 'Step 1: Adding the Archive Storage',
                        local: 'hdfs_00041.html',
                        children: [
                          {
                            id: 1191,
                            parentId: 1189,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'hdfs_00042.html'
                          },
                          {
                            id: 1192,
                            parentId: 1189,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'hdfs_00043.html'
                          }
                        ]
                      },
                      {
                        id: 1190,
                        parentId: 1187,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'hdfs_00044.html'
                      }
                    ]
                  },
                  {
                    id: 1188,
                    parentId: 1160,
                    name: 'Archiving HDFS Replication Copies',
                    local: 'hdfs_00045.html',
                    children: [
                      {
                        id: 1193,
                        parentId: 1188,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'hdfs_00046.html'
                      },
                      {
                        id: 1194,
                        parentId: 1188,
                        name:
                          'Step 2: Creating Periodic Archiving for Replication Copies',
                        local: 'hdfs_00047.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1161,
                parentId: 871,
                name: 'Restoration',
                local: 'hdfs_00047_1a.html',
                children: [
                  {
                    id: 1195,
                    parentId: 1161,
                    name: 'Restoring HDFS Filesets',
                    local: 'hdfs_00050.html'
                  }
                ]
              },
              {
                id: 1162,
                parentId: 871,
                name: 'Global Search',
                local: 'hdfs_00052.html',
                children: [
                  {
                    id: 1196,
                    parentId: 1162,
                    name: 'Global Search for Copies',
                    local: 'hdfs_00053.html'
                  },
                  {
                    id: 1197,
                    parentId: 1162,
                    name: 'Global Search for Resources',
                    local: 'hdfs_00054.html'
                  },
                  {
                    id: 1198,
                    parentId: 1162,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002686646.html'
                  }
                ]
              },
              {
                id: 1163,
                parentId: 871,
                name: 'SLA',
                local: 'hdfs_00057.html',
                children: [
                  {
                    id: 1199,
                    parentId: 1163,
                    name: 'Viewing SLA Information',
                    local: 'hdfs_00059.html'
                  },
                  {
                    id: 1200,
                    parentId: 1163,
                    name: 'Managing SLAs',
                    local: 'hdfs_00060.html'
                  }
                ]
              },
              {
                id: 1164,
                parentId: 871,
                name: 'Copies',
                local: 'hdfs_00060_1a.html',
                children: [
                  {
                    id: 1201,
                    parentId: 1164,
                    name: 'Querying HDFS Copy Information',
                    local: 'hdfs_00062.html'
                  },
                  {
                    id: 1202,
                    parentId: 1164,
                    name: 'Managing HDFS Copies',
                    local: 'hdfs_00063.html'
                  }
                ]
              },
              {
                id: 1165,
                parentId: 871,
                name: 'HDFS Cluster Environment',
                local: 'hdfs_00064.html',
                children: [
                  {
                    id: 1203,
                    parentId: 1165,
                    name: 'Querying HDFS Information',
                    local: 'hdfs_00065.html'
                  },
                  {
                    id: 1204,
                    parentId: 1165,
                    name: 'Managing HDFS Clusters',
                    local: 'hdfs_00066.html'
                  },
                  {
                    id: 1205,
                    parentId: 1165,
                    name: 'Managing HDFS Filesets',
                    local: 'hdfs_00067.html'
                  }
                ]
              }
            ]
          },
          {
            id: 872,
            parentId: 14,
            name: 'Redis Data Protection',
            local: 'en-us_topic_0000001873759393.html',
            children: [
              {
                id: 1206,
                parentId: 872,
                name: 'Backup',
                local: 'redis-00003.html',
                children: [
                  {
                    id: 1215,
                    parentId: 1206,
                    name: 'Preparing for the Backup',
                    local: 'redis-00006.html'
                  },
                  {
                    id: 1216,
                    parentId: 1206,
                    name: 'Backing Up a Redis Cluster',
                    local: 'redis-00007.html',
                    children: [
                      {
                        id: 1217,
                        parentId: 1216,
                        name: 'Step 1: Registering a Redis Cluster',
                        local: 'redis-00008.html'
                      },
                      {
                        id: 1218,
                        parentId: 1216,
                        name: 'Step 2 Creating a Rate Limiting Policy',
                        local: 'redis-00009.html'
                      },
                      {
                        id: 1219,
                        parentId: 1216,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'redis-00010.html'
                      },
                      {
                        id: 1220,
                        parentId: 1216,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'redis-00011.html'
                      },
                      {
                        id: 1221,
                        parentId: 1216,
                        name: 'Step 5: Performing Backup',
                        local: 'redis-00012.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1207,
                parentId: 872,
                name: 'Replication',
                local: 'oracle_gud_000035_5.html',
                children: [
                  {
                    id: 1222,
                    parentId: 1207,
                    name: 'Replicating Redis Copies',
                    local: 'redis-00018.html',
                    children: [
                      {
                        id: 1223,
                        parentId: 1222,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_6.html'
                      },
                      {
                        id: 1224,
                        parentId: 1222,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_10.html'
                      },
                      {
                        id: 1225,
                        parentId: 1222,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'redis-00021.html'
                      },
                      {
                        id: 1226,
                        parentId: 1222,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'redis-00022.html'
                      },
                      {
                        id: 1227,
                        parentId: 1222,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'redis-00023.html'
                      },
                      {
                        id: 1228,
                        parentId: 1222,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'redis-00024.html'
                      },
                      {
                        id: 1229,
                        parentId: 1222,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002046614601.html'
                      },
                      {
                        id: 1230,
                        parentId: 1222,
                        name: 'Step : Adding a Target Cluster',
                        local: 'redis-00025.html'
                      },
                      {
                        id: 1231,
                        parentId: 1222,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'redis-00026.html'
                      },
                      {
                        id: 1232,
                        parentId: 1222,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'redis-00027.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1208,
                parentId: 872,
                name: 'Archiving',
                local: 'redis-00028.html',
                children: [
                  {
                    id: 1233,
                    parentId: 1208,
                    name: 'Archiving Redis Backup Copies',
                    local: 'redis-00031.html',
                    children: [
                      {
                        id: 1235,
                        parentId: 1233,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'redis-00032.html',
                        children: [
                          {
                            id: 1237,
                            parentId: 1235,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'redis-00033.html'
                          },
                          {
                            id: 1238,
                            parentId: 1235,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'redis-00034.html'
                          }
                        ]
                      },
                      {
                        id: 1236,
                        parentId: 1233,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'redis-00035.html'
                      }
                    ]
                  },
                  {
                    id: 1234,
                    parentId: 1208,
                    name: 'Archiving Redis Replication Copies',
                    local: 'redis-00036.html',
                    children: [
                      {
                        id: 1239,
                        parentId: 1234,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'redis-00037.html'
                      },
                      {
                        id: 1240,
                        parentId: 1234,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'redis-00038.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1209,
                parentId: 872,
                name: 'Restoration',
                local: 'redis-00039.html',
                children: [
                  {
                    id: 1241,
                    parentId: 1209,
                    name: 'Restoring a Redis Cluster',
                    local: 'redis-00042.html'
                  }
                ]
              },
              {
                id: 1210,
                parentId: 872,
                name: 'Global Search',
                local: 'en-us_topic_0000002038763381.html',
                children: [
                  {
                    id: 1242,
                    parentId: 1210,
                    name: 'Global Search for Resources',
                    local: 'redis-00043.html'
                  },
                  {
                    id: 1243,
                    parentId: 1210,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002843406.html'
                  }
                ]
              },
              {
                id: 1211,
                parentId: 872,
                name: 'Data Deduplication and Compression',
                local: 'redis-00044.html',
                children: [
                  {
                    id: 1244,
                    parentId: 1211,
                    name: 'About Data Deduplication and Compression',
                    local: 'redis-00045.html'
                  }
                ]
              },
              {
                id: 1212,
                parentId: 872,
                name: 'SLA',
                local: 'redis-00046.html',
                children: [
                  {
                    id: 1245,
                    parentId: 1212,
                    name: 'About SLA',
                    local: 'redis-00047.html'
                  },
                  {
                    id: 1246,
                    parentId: 1212,
                    name: 'Viewing SLA Information',
                    local: 'redis-00048.html'
                  },
                  {
                    id: 1247,
                    parentId: 1212,
                    name: 'Managing SLAs',
                    local: 'redis-00049.html'
                  }
                ]
              },
              {
                id: 1213,
                parentId: 872,
                name: 'Copies',
                local: 'redis-00050.html',
                children: [
                  {
                    id: 1248,
                    parentId: 1213,
                    name: 'Viewing Redis Copy Information',
                    local: 'redis-00051.html'
                  },
                  {
                    id: 1249,
                    parentId: 1213,
                    name: 'Managing Redis Copies',
                    local: 'redis-00052.html'
                  }
                ]
              },
              {
                id: 1214,
                parentId: 872,
                name: 'Redis Cluster Environments',
                local: 'redis-00053.html',
                children: [
                  {
                    id: 1250,
                    parentId: 1214,
                    name: 'Querying Redis Information',
                    local: 'redis-00054.html'
                  },
                  {
                    id: 1251,
                    parentId: 1214,
                    name: 'Managing Redis Clusters',
                    local: 'redis-00055.html'
                  },
                  {
                    id: 1252,
                    parentId: 1214,
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
            id: 1253,
            parentId: 15,
            name: 'VMware Data Protection',
            local: 'product_documentation_000027.html',
            children: [
              {
                id: 1258,
                parentId: 1253,
                name: 'Backup',
                local: 'vmware_gud_0007_0.html',
                children: [
                  {
                    id: 1267,
                    parentId: 1258,
                    name: 'Preparing for the Backup',
                    local: 'vmware_gud_0014_0.html'
                  },
                  {
                    id: 1268,
                    parentId: 1258,
                    name: 'Backing Up a VMware VM',
                    local: 'vmware_gud_0015_0.html',
                    children: [
                      {
                        id: 1269,
                        parentId: 1268,
                        name: 'Step 1: Checking and Installing VMware Tools',
                        local: 'vmware_gud_0016.html'
                      },
                      {
                        id: 1270,
                        parentId: 1268,
                        name:
                          'Step 2: Checking and Enabling the vmware-vapi-endpoint Service',
                        local: 'vmware_gud_0017.html'
                      },
                      {
                        id: 1271,
                        parentId: 1268,
                        name:
                          'Step 3: Configuring the Application-Consistent Backup Script',
                        local: 'vmware_gud_0018.html',
                        children: [
                          {
                            id: 1280,
                            parentId: 1271,
                            name: 'DB2 Database',
                            local: 'vmware_gud_0019.html'
                          },
                          {
                            id: 1281,
                            parentId: 1271,
                            name: 'Oracle Database',
                            local: 'vmware_gud_0020.html'
                          },
                          {
                            id: 1282,
                            parentId: 1271,
                            name: 'Sybase Database',
                            local: 'vmware_gud_0021.html'
                          },
                          {
                            id: 1283,
                            parentId: 1271,
                            name: 'MySQL Database',
                            local: 'vmware_gud_0022.html'
                          }
                        ]
                      },
                      {
                        id: 1272,
                        parentId: 1268,
                        name: 'Step 4: Obtaining the VMware Certificate',
                        local: 'vmware_gud_0023.html'
                      },
                      {
                        id: 1273,
                        parentId: 1268,
                        name:
                          'Step 5: Registering the VMware Virtualization Environment',
                        local: 'vmware_gud_0024_0.html'
                      },
                      {
                        id: 1274,
                        parentId: 1268,
                        name:
                          'Step 6: (Optional) Creating a VMware VM Group (Applicable to 1.6.0 and Later Versions)',
                        local: 'vmware_gud_0024_1.html'
                      },
                      {
                        id: 1275,
                        parentId: 1268,
                        name:
                          'Step 7: (Optional) Creating a Rate Limiting Policy',
                        local: 'vmware_gud_0025_0.html'
                      },
                      {
                        id: 1276,
                        parentId: 1268,
                        name:
                          'Step 8: (Optional) Enabling Backup Link Encryption',
                        local: 'vmware_gud_0026.html'
                      },
                      {
                        id: 1277,
                        parentId: 1268,
                        name:
                          'Step 9: (Optional) Logging In to the iSCSI Initiator',
                        local: 'vmware_gud_iscsi.html'
                      },
                      {
                        id: 1278,
                        parentId: 1268,
                        name: 'Step 10: Creating a Backup SLA',
                        local: 'vmware_gud_0027_0.html'
                      },
                      {
                        id: 1279,
                        parentId: 1268,
                        name: 'Step 11: Performing Backup',
                        local: 'vmware_gud_0028_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1259,
                parentId: 1253,
                name: 'Replication',
                local: 'vmware_gud_0031_0.html',
                children: [
                  {
                    id: 1284,
                    parentId: 1259,
                    name: 'Replicating a VMware VM Copy',
                    local: 'vmware_gud_0034_0.html',
                    children: [
                      {
                        id: 1285,
                        parentId: 1284,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'vmware_gud_0036.html'
                      },
                      {
                        id: 1286,
                        parentId: 1284,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_21.html'
                      },
                      {
                        id: 1287,
                        parentId: 1284,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'vmware_gud_0037_0.html'
                      },
                      {
                        id: 1288,
                        parentId: 1284,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'vmware_gud_0038_0.html'
                      },
                      {
                        id: 1289,
                        parentId: 1284,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'vmware_gud_0039_0.html'
                      },
                      {
                        id: 1290,
                        parentId: 1284,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'vmware_gud_0040_1.html'
                      },
                      {
                        id: 1291,
                        parentId: 1284,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002008299624.html'
                      },
                      {
                        id: 1292,
                        parentId: 1284,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'vmware_gud_0041_0.html'
                      },
                      {
                        id: 1293,
                        parentId: 1284,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'vmware_gud_0042_0.html'
                      },
                      {
                        id: 1294,
                        parentId: 1284,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'vmware_gud_0043_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1260,
                parentId: 1253,
                name: 'Archiving',
                local: 'vmware_gud_0044_0.html',
                children: [
                  {
                    id: 1295,
                    parentId: 1260,
                    name: 'Archiving VMware Backup Copies',
                    local: 'vmware_gud_0047_0.html',
                    children: [
                      {
                        id: 1297,
                        parentId: 1295,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'vmware_gud_0048_0.html',
                        children: [
                          {
                            id: 1299,
                            parentId: 1297,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'vmware_gud_0049_0.html'
                          },
                          {
                            id: 1300,
                            parentId: 1297,
                            name:
                              'Creating a Media Set (When Archive Storage Is a Tape Library)',
                            local: 'vmware_gud_0050_0.html'
                          }
                        ]
                      },
                      {
                        id: 1298,
                        parentId: 1295,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'vmware_gud_0051_0.html'
                      }
                    ]
                  },
                  {
                    id: 1296,
                    parentId: 1260,
                    name: 'Archiving VMware Replication Copies',
                    local: 'vmware_gud_0052_0.html',
                    children: [
                      {
                        id: 1301,
                        parentId: 1296,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'vmware_gud_0053_0.html'
                      },
                      {
                        id: 1302,
                        parentId: 1296,
                        name:
                          'Step 2: Creating Periodic Archives of Replication Copies',
                        local: 'vmware_gud_0054_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1261,
                parentId: 1253,
                name: 'Restoration',
                local: 'vmware_gud_0055_0.html',
                children: [
                  {
                    id: 1303,
                    parentId: 1261,
                    name: 'Restoring VMware VMs',
                    local: 'vmware_gud_0058_0.html'
                  },
                  {
                    id: 1304,
                    parentId: 1261,
                    name: 'Restoring VMware VM Disks',
                    local: 'vmware_gud_0059_0.html'
                  }
                ]
              },
              {
                id: 1262,
                parentId: 1253,
                name: 'Instant Recovery',
                local: 'vmware_gud_0061.html',
                children: [
                  {
                    id: 1305,
                    parentId: 1262,
                    name: 'Instant Recovery of VMware VMs',
                    local: 'vmware_gud_0064.html'
                  }
                ]
              },
              {
                id: 1263,
                parentId: 1253,
                name: 'Global Search',
                local: 'vmware_gud_0074_0.html',
                children: [
                  {
                    id: 1306,
                    parentId: 1263,
                    name: 'About Global Search',
                    local: 'vmware_gud_0075.html'
                  },
                  {
                    id: 1307,
                    parentId: 1263,
                    name: 'Global Search for Copies',
                    local: 'vmware_gud_0076_2.html'
                  },
                  {
                    id: 1308,
                    parentId: 1263,
                    name: 'Global Search for Resources',
                    local: 'vmware_gud_0077_0.html'
                  },
                  {
                    id: 1309,
                    parentId: 1263,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002038754917.html'
                  }
                ]
              },
              {
                id: 1264,
                parentId: 1253,
                name: 'SLA ',
                local: 'vmware_gud_0080_0.html',
                children: [
                  {
                    id: 1310,
                    parentId: 1264,
                    name: 'Viewing SLA Information',
                    local: 'vmware_gud_0082_0.html'
                  },
                  {
                    id: 1311,
                    parentId: 1264,
                    name: 'Managing SLAs',
                    local: 'vmware_gud_0083_0.html'
                  }
                ]
              },
              {
                id: 1265,
                parentId: 1253,
                name: 'Copies',
                local: 'vmware_gud_0084_0.html',
                children: [
                  {
                    id: 1312,
                    parentId: 1265,
                    name: 'Viewing VMware Copy Data',
                    local: 'vmware_gud_0085_0.html'
                  },
                  {
                    id: 1313,
                    parentId: 1265,
                    name: 'Managing VMware Copies',
                    local: 'vmware_gud_0086_0.html'
                  }
                ]
              },
              {
                id: 1266,
                parentId: 1253,
                name: 'VMware Virtualization Environment',
                local: 'vmware_gud_0087_0.html',
                children: [
                  {
                    id: 1314,
                    parentId: 1266,
                    name:
                      'Querying Information About the VMware Virtualization Environment',
                    local: 'vmware_gud_0088_0.html'
                  },
                  {
                    id: 1315,
                    parentId: 1266,
                    name: 'Managing VMware Registration Information',
                    local: 'vmware_gud_0089_0.html'
                  },
                  {
                    id: 1316,
                    parentId: 1266,
                    name: 'Managing Clusters/Hosts/VMs/VM Groups',
                    local: 'vmware_gud_0090_0.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1254,
            parentId: 15,
            name: 'FusionCompute Data Protection',
            local: 'en-us_topic_0000001873679177.html',
            children: [
              {
                id: 1317,
                parentId: 1254,
                name: 'Backup',
                local: 'fc_gud_0009_0.html',
                children: [
                  {
                    id: 1325,
                    parentId: 1317,
                    name: 'Preparations for Backup',
                    local: 'fc_gud_0012_0.html'
                  },
                  {
                    id: 1326,
                    parentId: 1317,
                    name: 'Backing Up a FusionCompute VM',
                    local: 'fc_gud_0013_0.html',
                    children: [
                      {
                        id: 1327,
                        parentId: 1326,
                        name:
                          'Step 1: Create a FusionCompute Interconnection User',
                        local: 'fc_gud_0014_0.html'
                      },
                      {
                        id: 1328,
                        parentId: 1326,
                        name:
                          'Step 2: Registering the FusionCompute Virtualization Environment',
                        local: 'fc_gud_0015_0.html'
                      },
                      {
                        id: 1329,
                        parentId: 1326,
                        name:
                          'Step 3: (Optional) Creating a FusionCompute VM Group (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0024_1_0.html'
                      },
                      {
                        id: 1330,
                        parentId: 1326,
                        name:
                          'Step 4: (Optional) Creating a Rate Limiting Policy',
                        local: 'fc_gud_0016_0.html'
                      },
                      {
                        id: 1331,
                        parentId: 1326,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'fc_gud_0017_0.html'
                      },
                      {
                        id: 1332,
                        parentId: 1326,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'fc_gud_0018_0.html'
                      },
                      {
                        id: 1333,
                        parentId: 1326,
                        name: 'Step 7: Performing Backup',
                        local: 'fc_gud_0019_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1318,
                parentId: 1254,
                name: 'Replication',
                local: 'fc_gud_0022_0.html',
                children: [
                  {
                    id: 1334,
                    parentId: 1318,
                    name: 'Replicating a FusionCompute VM Copy',
                    local: 'fc_gud_0024_0.html',
                    children: [
                      {
                        id: 1335,
                        parentId: 1334,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_11.html'
                      },
                      {
                        id: 1336,
                        parentId: 1334,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_16.html'
                      },
                      {
                        id: 1337,
                        parentId: 1334,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'fc_gud_0027_0.html'
                      },
                      {
                        id: 1338,
                        parentId: 1334,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'fc_gud_0028_0.html'
                      },
                      {
                        id: 1339,
                        parentId: 1334,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'fc_gud_0029_0.html'
                      },
                      {
                        id: 1340,
                        parentId: 1334,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'fc_gud_0030_0.html'
                      },
                      {
                        id: 1341,
                        parentId: 1334,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002008271350.html'
                      },
                      {
                        id: 1342,
                        parentId: 1334,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'fc_gud_0031_0.html'
                      },
                      {
                        id: 1343,
                        parentId: 1334,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'fc_gud_0032_0.html'
                      },
                      {
                        id: 1344,
                        parentId: 1334,
                        name:
                          '(Optional) Step 8: Creating a Bidirectional Replication or Cascading Replication SLA',
                        local: 'fc_gud_0033_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1319,
                parentId: 1254,
                name: 'Archiving',
                local: 'fc_gud_0034_0.html',
                children: [
                  {
                    id: 1345,
                    parentId: 1319,
                    name: 'Archiving FusionCompute Backup Copies',
                    local: 'fc_gud_0037_0.html',
                    children: [
                      {
                        id: 1347,
                        parentId: 1345,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'fc_gud_0038_0.html',
                        children: [
                          {
                            id: 1349,
                            parentId: 1347,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'fc_gud_0039_0.html'
                          },
                          {
                            id: 1350,
                            parentId: 1347,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'fc_gud_0040_0.html'
                          }
                        ]
                      },
                      {
                        id: 1348,
                        parentId: 1345,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'fc_gud_0041_0.html'
                      }
                    ]
                  },
                  {
                    id: 1346,
                    parentId: 1319,
                    name: 'Archiving FusionCompute Replication Copies',
                    local: 'fc_gud_0042_0.html',
                    children: [
                      {
                        id: 1351,
                        parentId: 1346,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'fc_gud_0043_0.html'
                      },
                      {
                        id: 1352,
                        parentId: 1346,
                        name:
                          'Step 2: Creating Periodic Archives of Replication Copies',
                        local: 'fc_gud_0044_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1320,
                parentId: 1254,
                name: 'Restoration',
                local: 'fc_gud_0045_0.html',
                children: [
                  {
                    id: 1353,
                    parentId: 1320,
                    name: 'Restoring FusionCompute VMs',
                    local: 'fc_gud_0048_0.html'
                  },
                  {
                    id: 1354,
                    parentId: 1320,
                    name: 'Restoring FusionCompute VM Disks',
                    local: 'fc_gud_0049_0.html'
                  }
                ]
              },
              {
                id: 1321,
                parentId: 1254,
                name: 'Global Search',
                local: 'fc_gud_gs1_0.html',
                children: [
                  {
                    id: 1355,
                    parentId: 1321,
                    name: 'About Global Search',
                    local: 'en-us_topic_0000002038884249.html'
                  },
                  {
                    id: 1356,
                    parentId: 1321,
                    name: 'Global Search for Copies',
                    local: 'fc_gud_gs3_0.html'
                  },
                  {
                    id: 1357,
                    parentId: 1321,
                    name: 'Global Search for Resources',
                    local: 'fc_gud_0050_0.html'
                  },
                  {
                    id: 1358,
                    parentId: 1321,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002642984.html'
                  }
                ]
              },
              {
                id: 1322,
                parentId: 1254,
                name: 'SLA ',
                local: 'fc_gud_0053_0.html',
                children: [
                  {
                    id: 1359,
                    parentId: 1322,
                    name: 'Viewing SLA Information',
                    local: 'fc_gud_0055_0.html'
                  },
                  {
                    id: 1360,
                    parentId: 1322,
                    name: 'Managing SLAs',
                    local: 'fc_gud_0056_0.html'
                  }
                ]
              },
              {
                id: 1323,
                parentId: 1254,
                name: 'Copies',
                local: 'fc_gud_0057_0.html',
                children: [
                  {
                    id: 1361,
                    parentId: 1323,
                    name: 'Viewing FusionCompute Copy Data',
                    local: 'fc_gud_0058_0.html'
                  },
                  {
                    id: 1362,
                    parentId: 1323,
                    name: 'Managing FusionCompute Copies',
                    local: 'fc_gud_0059_0.html'
                  }
                ]
              },
              {
                id: 1324,
                parentId: 1254,
                name: 'FusionCompute Virtualization Environment',
                local: 'fc_gud_0060_0.html',
                children: [
                  {
                    id: 1363,
                    parentId: 1324,
                    name:
                      'Querying Information About the FusionCompute Virtualization Environment',
                    local: 'fc_gud_0061_0.html'
                  },
                  {
                    id: 1364,
                    parentId: 1324,
                    name: 'Managing FusionCompute Registration Information',
                    local: 'fc_gud_0062_0.html'
                  },
                  {
                    id: 1365,
                    parentId: 1324,
                    name: 'Managing Clusters/Hosts/VMs/VM Groups',
                    local: 'fc_gud_0063_0.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1255,
            parentId: 15,
            name:
              'CNware Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001873679209.html',
            children: [
              {
                id: 1366,
                parentId: 1255,
                name: 'Backup',
                local: 'CNware_00007.html',
                children: [
                  {
                    id: 1375,
                    parentId: 1366,
                    name: 'Preparing for the Backup',
                    local: 'CNware_00015.html'
                  },
                  {
                    id: 1376,
                    parentId: 1366,
                    name: 'Backing Up CNware VMs',
                    local: 'CNware_00016.html',
                    children: [
                      {
                        id: 1377,
                        parentId: 1376,
                        name: 'Step 1: Obtaining the CNware Certificate',
                        local: 'CNware_00024.html'
                      },
                      {
                        id: 1378,
                        parentId: 1376,
                        name:
                          'Step 2: Registering the CNware Virtualization Environment',
                        local: 'CNware_00025.html'
                      },
                      {
                        id: 1379,
                        parentId: 1376,
                        name: 'Step 3: (Optional) Creating a CNware VM Group',
                        local: 'cnware_00014.html'
                      },
                      {
                        id: 1380,
                        parentId: 1376,
                        name:
                          'Step 4: (Optional) Creating a Rate Limiting Policy',
                        local: 'CNware_00026.html'
                      },
                      {
                        id: 1381,
                        parentId: 1376,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'CNware_00027.html'
                      },
                      {
                        id: 1382,
                        parentId: 1376,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'CNware_00029.html'
                      },
                      {
                        id: 1383,
                        parentId: 1376,
                        name: 'Step 7: Performing Backup',
                        local: 'CNware_00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1367,
                parentId: 1255,
                name: 'Replication',
                local: 'CNware_00033.html',
                children: [
                  {
                    id: 1384,
                    parentId: 1367,
                    name: 'Replicating a CNware VM Copy',
                    local: 'CNware_00036.html',
                    children: [
                      {
                        id: 1385,
                        parentId: 1384,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'CNware_00038.html'
                      },
                      {
                        id: 1386,
                        parentId: 1384,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_0.html'
                      },
                      {
                        id: 1387,
                        parentId: 1384,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'CNware_00039.html'
                      },
                      {
                        id: 1388,
                        parentId: 1384,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'CNware_00040.html'
                      },
                      {
                        id: 1389,
                        parentId: 1384,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'CNware_00041.html'
                      },
                      {
                        id: 1390,
                        parentId: 1384,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'CNware_00042.html'
                      },
                      {
                        id: 1391,
                        parentId: 1384,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'cnware_000300.html'
                      },
                      {
                        id: 1392,
                        parentId: 1384,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'CNware_00043.html'
                      },
                      {
                        id: 1393,
                        parentId: 1384,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'CNware_00044.html'
                      },
                      {
                        id: 1394,
                        parentId: 1384,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'CNware_00045.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1368,
                parentId: 1255,
                name: 'Archiving',
                local: 'CNware_00046.html',
                children: [
                  {
                    id: 1395,
                    parentId: 1368,
                    name: 'Archiving CNware Backup Copies',
                    local: 'CNware_00049.html',
                    children: [
                      {
                        id: 1397,
                        parentId: 1395,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'CNware_00050.html',
                        children: [
                          {
                            id: 1399,
                            parentId: 1397,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'CNware_00051.html'
                          },
                          {
                            id: 1400,
                            parentId: 1397,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'CNware_00052.html'
                          }
                        ]
                      },
                      {
                        id: 1398,
                        parentId: 1395,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'CNware_00053.html'
                      }
                    ]
                  },
                  {
                    id: 1396,
                    parentId: 1368,
                    name: 'Archiving CNware Replication Copies',
                    local: 'CNware_00054.html',
                    children: [
                      {
                        id: 1401,
                        parentId: 1396,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'CNware_00055.html'
                      },
                      {
                        id: 1402,
                        parentId: 1396,
                        name:
                          'Step 2: Creating Periodic Archives of Replication Copies',
                        local: 'CNware_00056.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1369,
                parentId: 1255,
                name: 'Restoration',
                local: 'CNware_00057.html',
                children: [
                  {
                    id: 1403,
                    parentId: 1369,
                    name: 'Restoring CNware VMs',
                    local: 'CNware_00060.html'
                  },
                  {
                    id: 1404,
                    parentId: 1369,
                    name: 'Restoring CNware VM Disks',
                    local: 'CNware_00061.html'
                  }
                ]
              },
              {
                id: 1370,
                parentId: 1255,
                name: 'Instant Recovery',
                local: 'CNware_00063.html',
                children: [
                  {
                    id: 1405,
                    parentId: 1370,
                    name: 'Instant Recovery of CNware VMs',
                    local: 'CNware_00066.html'
                  }
                ]
              },
              {
                id: 1371,
                parentId: 1255,
                name: 'Global Search',
                local: 'CNware_00076.html',
                children: [
                  {
                    id: 1406,
                    parentId: 1371,
                    name: 'About Global Search',
                    local: 'cnware_000641.html'
                  },
                  {
                    id: 1407,
                    parentId: 1371,
                    name: 'Global Search for Copies',
                    local: 'cnware_000642.html'
                  },
                  {
                    id: 1408,
                    parentId: 1371,
                    name: 'Global Search for Resources',
                    local: 'CNware_00079.html'
                  },
                  {
                    id: 1409,
                    parentId: 1371,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'cnware_000671.html'
                  }
                ]
              },
              {
                id: 1372,
                parentId: 1255,
                name: 'SLA ',
                local: 'CNware_00082.html',
                children: [
                  {
                    id: 1410,
                    parentId: 1372,
                    name: 'Viewing SLA Information',
                    local: 'CNware_00084.html'
                  },
                  {
                    id: 1411,
                    parentId: 1372,
                    name: 'Managing SLAs',
                    local: 'CNware_00085.html'
                  }
                ]
              },
              {
                id: 1373,
                parentId: 1255,
                name: 'Copies',
                local: 'CNware_00086.html',
                children: [
                  {
                    id: 1412,
                    parentId: 1373,
                    name: 'Viewing CNware Copy Data',
                    local: 'CNware_00087.html'
                  },
                  {
                    id: 1413,
                    parentId: 1373,
                    name: 'Managing CNware Copies',
                    local: 'CNware_00088.html'
                  }
                ]
              },
              {
                id: 1374,
                parentId: 1255,
                name: 'CNware Virtualization Environment',
                local: 'CNware_00089.html',
                children: [
                  {
                    id: 1414,
                    parentId: 1374,
                    name:
                      'Querying Information About the CNware Virtualization Environment',
                    local: 'CNware_00090.html'
                  },
                  {
                    id: 1415,
                    parentId: 1374,
                    name: 'Managing CNware Registration Information',
                    local: 'CNware_00091.html'
                  },
                  {
                    id: 1416,
                    parentId: 1374,
                    name: 'Management VMs/Hosts/Clusters',
                    local: 'CNware_00092.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1256,
            parentId: 15,
            name:
              'Hyper-V Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000002037019572.html',
            children: [
              {
                id: 1417,
                parentId: 1256,
                name: 'Backup',
                local: 'vmware_gud_0007.html',
                children: [
                  {
                    id: 1425,
                    parentId: 1417,
                    name: 'Preparing for the Backup',
                    local: 'vmware_gud_0014.html'
                  },
                  {
                    id: 1426,
                    parentId: 1417,
                    name: 'Backing Up a Hyper-V VM',
                    local: 'vmware_gud_0015.html',
                    children: [
                      {
                        id: 1427,
                        parentId: 1426,
                        name:
                          'Step 1: Registering a Hyper-V Virtualization Environment',
                        local: 'vmware_gud_0024.html'
                      },
                      {
                        id: 1428,
                        parentId: 1426,
                        name:
                          'Step 2: (Optional) Creating a Rate Limiting Policy',
                        local: 'vmware_gud_0025.html'
                      },
                      {
                        id: 1429,
                        parentId: 1426,
                        name: 'Step 3: Creating a Backup SLA',
                        local: 'vmware_gud_0027.html'
                      },
                      {
                        id: 1430,
                        parentId: 1426,
                        name: 'Step 4: Performing Backup',
                        local: 'vmware_gud_0028.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1418,
                parentId: 1256,
                name: 'Replication',
                local: 'vmware_gud_0031.html',
                children: [
                  {
                    id: 1431,
                    parentId: 1418,
                    name: 'Replicating Hyper-V VM Copies',
                    local: 'vmware_gud_0034.html',
                    children: [
                      {
                        id: 1432,
                        parentId: 1431,
                        name:
                          'Creating a Logical Port for the Replication Network',
                        local: 'fc_gud_0026_1_9.html'
                      },
                      {
                        id: 1433,
                        parentId: 1431,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'vmware_gud_0037.html'
                      },
                      {
                        id: 1434,
                        parentId: 1431,
                        name:
                          '(Optional) Step 3: Enabling Replication Link Encryption',
                        local: 'vmware_gud_0038.html'
                      },
                      {
                        id: 1435,
                        parentId: 1431,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'vmware_gud_0039.html'
                      },
                      {
                        id: 1436,
                        parentId: 1431,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'vmware_gud_0040_0.html'
                      },
                      {
                        id: 1437,
                        parentId: 1431,
                        name: 'Step 6: Adding a Replication Cluster',
                        local: 'vmware_gud_0041.html'
                      },
                      {
                        id: 1438,
                        parentId: 1431,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'vmware_gud_0042.html'
                      },
                      {
                        id: 1439,
                        parentId: 1431,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'vmware_gud_0043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1419,
                parentId: 1256,
                name: 'Archiving',
                local: 'vmware_gud_0044.html',
                children: [
                  {
                    id: 1440,
                    parentId: 1419,
                    name: 'Archiving Hyper-V Backup Copies',
                    local: 'vmware_gud_0047.html',
                    children: [
                      {
                        id: 1442,
                        parentId: 1440,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'vmware_gud_0048.html',
                        children: [
                          {
                            id: 1444,
                            parentId: 1442,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'vmware_gud_0049.html'
                          },
                          {
                            id: 1445,
                            parentId: 1442,
                            name:
                              'Creating a Media Set (When Archive Storage Is a Tape Library)',
                            local: 'vmware_gud_0050.html'
                          }
                        ]
                      },
                      {
                        id: 1443,
                        parentId: 1440,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'vmware_gud_0051.html'
                      }
                    ]
                  },
                  {
                    id: 1441,
                    parentId: 1419,
                    name: 'Archiving Hyper-V Replication Copies',
                    local: 'vmware_gud_0052.html',
                    children: [
                      {
                        id: 1446,
                        parentId: 1441,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'vmware_gud_0053.html'
                      },
                      {
                        id: 1447,
                        parentId: 1441,
                        name:
                          'Step 2: Creating Periodic Archives of Replication Copies',
                        local: 'vmware_gud_0054.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1420,
                parentId: 1256,
                name: 'Restoration',
                local: 'vmware_gud_0055.html',
                children: [
                  {
                    id: 1448,
                    parentId: 1420,
                    name: 'Restoring Hyper-V VMs',
                    local: 'vmware_gud_0058.html'
                  },
                  {
                    id: 1449,
                    parentId: 1420,
                    name: 'Restoring Hyper-V VM Disks',
                    local: 'vmware_gud_0059.html'
                  }
                ]
              },
              {
                id: 1421,
                parentId: 1256,
                name: 'Global Search',
                local: 'vmware_gud_0074.html',
                children: [
                  {
                    id: 1450,
                    parentId: 1421,
                    name: 'Global Search for Copies',
                    local: 'vmware_gud_0076.html'
                  },
                  {
                    id: 1451,
                    parentId: 1421,
                    name: 'Global Search for Resources',
                    local: 'vmware_gud_0077.html'
                  },
                  {
                    id: 1452,
                    parentId: 1421,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002096124161.html'
                  }
                ]
              },
              {
                id: 1422,
                parentId: 1256,
                name: 'SLA ',
                local: 'vmware_gud_0080.html',
                children: [
                  {
                    id: 1453,
                    parentId: 1422,
                    name: 'Viewing SLA Information',
                    local: 'vmware_gud_0082.html'
                  },
                  {
                    id: 1454,
                    parentId: 1422,
                    name: 'Managing SLAs',
                    local: 'vmware_gud_0083.html'
                  }
                ]
              },
              {
                id: 1423,
                parentId: 1256,
                name: 'Copies',
                local: 'vmware_gud_0084.html',
                children: [
                  {
                    id: 1455,
                    parentId: 1423,
                    name: 'Viewing Hyper-V Copy Information',
                    local: 'vmware_gud_0085.html'
                  },
                  {
                    id: 1456,
                    parentId: 1423,
                    name: 'Managing Hyper-V Copies',
                    local: 'vmware_gud_0086.html'
                  }
                ]
              },
              {
                id: 1424,
                parentId: 1256,
                name: 'Hyper-V Virtualization Environment',
                local: 'vmware_gud_0087.html',
                children: [
                  {
                    id: 1457,
                    parentId: 1424,
                    name:
                      'Viewing Information About a Hyper-V Virtualization Environment',
                    local: 'vmware_gud_0088.html'
                  },
                  {
                    id: 1458,
                    parentId: 1424,
                    name: 'Managing Hyper-V Registration Information',
                    local: 'vmware_gud_0089.html'
                  },
                  {
                    id: 1459,
                    parentId: 1424,
                    name: 'Managing Clusters/Hosts/VMs/VM Groups',
                    local: 'vmware_gud_0090.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1257,
            parentId: 15,
            name:
              'FusionOne Compute Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000002085703925.html',
            children: [
              {
                id: 1460,
                parentId: 1257,
                name: 'Backup',
                local: 'fc_gud_0009.html',
                children: [
                  {
                    id: 1468,
                    parentId: 1460,
                    name: 'Preparations for Backup',
                    local: 'fc_gud_0012.html'
                  },
                  {
                    id: 1469,
                    parentId: 1460,
                    name: 'Backing Up a FusionOne Compute VM',
                    local: 'fc_gud_0013.html',
                    children: [
                      {
                        id: 1470,
                        parentId: 1469,
                        name:
                          'Step 1: Create a FusionOne Compute Interconnection User',
                        local: 'fc_gud_0014.html'
                      },
                      {
                        id: 1471,
                        parentId: 1469,
                        name:
                          'Step 2: Registering the FusionOne Compute Virtualization Environment',
                        local: 'fc_gud_0015.html'
                      },
                      {
                        id: 1472,
                        parentId: 1469,
                        name:
                          'Step 3: (Optional) Creating a FusionOne Compute VM Group',
                        local: 'fc_gud_0024_1.html'
                      },
                      {
                        id: 1473,
                        parentId: 1469,
                        name:
                          'Step 4: (Optional) Creating a Rate Limiting Policy',
                        local: 'fc_gud_0016.html'
                      },
                      {
                        id: 1474,
                        parentId: 1469,
                        name:
                          'Step 5: (Optional) Enabling Backup Link Encryption',
                        local: 'fc_gud_0017.html'
                      },
                      {
                        id: 1475,
                        parentId: 1469,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'fc_gud_0018.html'
                      },
                      {
                        id: 1476,
                        parentId: 1469,
                        name: 'Step 7: Performing Backup',
                        local: 'fc_gud_0019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1461,
                parentId: 1257,
                name: 'Replication',
                local: 'fc_gud_0022.html',
                children: [
                  {
                    id: 1477,
                    parentId: 1461,
                    name: 'Replicating a FusionOne Compute VM Copy',
                    local: 'fc_gud_0024.html',
                    children: [
                      {
                        id: 1478,
                        parentId: 1477,
                        name:
                          'Step 1: Creating a Replication Network Logical Port',
                        local: 'fc_gud_0026_1_2.html'
                      },
                      {
                        id: 1479,
                        parentId: 1477,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'fc_gud_0027.html'
                      },
                      {
                        id: 1480,
                        parentId: 1477,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'fc_gud_0028.html'
                      },
                      {
                        id: 1481,
                        parentId: 1477,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'fc_gud_0029.html'
                      },
                      {
                        id: 1482,
                        parentId: 1477,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'foc_gud_0031.html'
                      },
                      {
                        id: 1483,
                        parentId: 1477,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'fc_gud_0031.html'
                      },
                      {
                        id: 1484,
                        parentId: 1477,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'fc_gud_0032.html'
                      },
                      {
                        id: 1485,
                        parentId: 1477,
                        name:
                          '(Optional) Step 8: Creating a Bidirectional Replication or Cascading Replication SLA',
                        local: 'fc_gud_0033.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1462,
                parentId: 1257,
                name: 'Archiving',
                local: 'fc_gud_0034.html',
                children: [
                  {
                    id: 1486,
                    parentId: 1462,
                    name: 'Archiving FusionOne Compute Backup Copies',
                    local: 'fc_gud_0037.html',
                    children: [
                      {
                        id: 1488,
                        parentId: 1486,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'fc_gud_0038.html',
                        children: [
                          {
                            id: 1490,
                            parentId: 1488,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'fc_gud_0039.html'
                          },
                          {
                            id: 1491,
                            parentId: 1488,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'fc_gud_0040.html'
                          }
                        ]
                      },
                      {
                        id: 1489,
                        parentId: 1486,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'fc_gud_0041.html'
                      }
                    ]
                  },
                  {
                    id: 1487,
                    parentId: 1462,
                    name: 'Archiving FusionOne Compute Replication Copies',
                    local: 'fc_gud_0042.html',
                    children: [
                      {
                        id: 1492,
                        parentId: 1487,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'fc_gud_0043.html'
                      },
                      {
                        id: 1493,
                        parentId: 1487,
                        name:
                          'Step 2: Creating Periodic Archives of Replication Copies',
                        local: 'fc_gud_0044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1463,
                parentId: 1257,
                name: 'Restoration',
                local: 'fc_gud_0045.html',
                children: [
                  {
                    id: 1494,
                    parentId: 1463,
                    name: 'Restoring FusionOne Compute VMs',
                    local: 'fc_gud_0048.html'
                  },
                  {
                    id: 1495,
                    parentId: 1463,
                    name: 'Restoring FusionOne Compute VM Disks',
                    local: 'fc_gud_0049.html'
                  }
                ]
              },
              {
                id: 1464,
                parentId: 1257,
                name: 'Global Search',
                local: 'fc_gud_gs1.html',
                children: [
                  {
                    id: 1496,
                    parentId: 1464,
                    name: 'About Global Search',
                    local: 'en-us_topic_0000002038884797.html'
                  },
                  {
                    id: 1497,
                    parentId: 1464,
                    name: 'Global Search for Copies',
                    local: 'fc_gud_gs3.html'
                  },
                  {
                    id: 1498,
                    parentId: 1464,
                    name: 'Global Search for Resources',
                    local: 'fc_gud_0050.html'
                  },
                  {
                    id: 1499,
                    parentId: 1464,
                    name: 'Tag-based Global Search',
                    local: 'en-us_topic_0000002002488232.html'
                  }
                ]
              },
              {
                id: 1465,
                parentId: 1257,
                name: 'SLA ',
                local: 'fc_gud_0053.html',
                children: [
                  {
                    id: 1500,
                    parentId: 1465,
                    name: 'Viewing SLA Information',
                    local: 'fc_gud_0055.html'
                  },
                  {
                    id: 1501,
                    parentId: 1465,
                    name: 'Managing SLAs',
                    local: 'fc_gud_0056.html'
                  }
                ]
              },
              {
                id: 1466,
                parentId: 1257,
                name: 'Copies',
                local: 'fc_gud_0057.html',
                children: [
                  {
                    id: 1502,
                    parentId: 1466,
                    name: 'Viewing FusionOne Compute Copy Data',
                    local: 'fc_gud_0058.html'
                  },
                  {
                    id: 1503,
                    parentId: 1466,
                    name: 'Managing FusionOne Compute Copies',
                    local: 'fc_gud_0059.html'
                  }
                ]
              },
              {
                id: 1467,
                parentId: 1257,
                name: 'FusionOne Compute Virtualization Environment',
                local: 'fc_gud_0060.html',
                children: [
                  {
                    id: 1504,
                    parentId: 1467,
                    name:
                      'Querying Information About the FusionOne Compute Virtualization Environment',
                    local: 'fc_gud_0061.html'
                  },
                  {
                    id: 1505,
                    parentId: 1467,
                    name: 'Managing FusionOne Compute Registration Information',
                    local: 'fc_gud_0062.html'
                  },
                  {
                    id: 1506,
                    parentId: 1467,
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
            id: 1507,
            parentId: 16,
            name: 'Kubernetes CSI Data Protection',
            local: 'en-us_topic_0000001873759377.html',
            children: [
              {
                id: 1509,
                parentId: 1507,
                name: 'Backup',
                local: 'kubernetes_CSI_00006.html',
                children: [
                  {
                    id: 1518,
                    parentId: 1509,
                    name: 'Preparing for Backup (Applicable to FusionCompute)',
                    local: 'kubernetes_CSI_000091.html',
                    children: [
                      {
                        id: 1523,
                        parentId: 1518,
                        name:
                          'Uploading the Kubernetes Installation Package to the Image Repository',
                        local: 'kubernetes_CSI_00069.html'
                      },
                      {
                        id: 1524,
                        parentId: 1518,
                        name: 'Obtaining the kubeconfig Configuration File',
                        local: 'kubernetes_CSI_00065.html'
                      }
                    ]
                  },
                  {
                    id: 1519,
                    parentId: 1509,
                    name: 'Preparing for Backup (Applicable to CCE)',
                    local: 'kubernetes_CSI_000092.html',
                    children: [
                      {
                        id: 1525,
                        parentId: 1519,
                        name:
                          'Uploading and Updating the Kubernetes Image Package',
                        local: 'kubernetes_CSI_00102.html'
                      },
                      {
                        id: 1526,
                        parentId: 1519,
                        name: 'Obtaining the kubeconfig Configuration File',
                        local: 'kubernetes_CSI_00078.html'
                      }
                    ]
                  },
                  {
                    id: 1520,
                    parentId: 1509,
                    name: 'Preparing for Backup (Applicable to OpenShift)',
                    local: 'kubernetes_CSI_00078_2.html',
                    children: [
                      {
                        id: 1527,
                        parentId: 1520,
                        name:
                          'Uploading the Kubernetes Installation Package and Obtaining the Image Name and Tag Information',
                        local: 'kubernetes_CSI_00078_3.html'
                      },
                      {
                        id: 1528,
                        parentId: 1520,
                        name: 'Obtaining the kubeconfig Configuration File',
                        local: 'kubernetes_CSI_00078_4.html'
                      },
                      {
                        id: 1529,
                        parentId: 1520,
                        name: 'Obtaining the Token Information',
                        local: 'kubernetes_CSI_00078_5.html'
                      }
                    ]
                  },
                  {
                    id: 1521,
                    parentId: 1509,
                    name:
                      'Preparing for Backup (Applicable to Native Kubernetes)',
                    local: 'kubernetes_CSI_00078_6.html',
                    children: [
                      {
                        id: 1530,
                        parentId: 1521,
                        name:
                          'Uploading the Kubernetes Installation Package to the Kubernetes Cluster',
                        local: 'kubernetes_CSI_00078_7.html'
                      },
                      {
                        id: 1531,
                        parentId: 1521,
                        name: 'Obtaining the kubeconfig Configuration File',
                        local: 'kubernetes_CSI_00078_8.html'
                      }
                    ]
                  },
                  {
                    id: 1522,
                    parentId: 1509,
                    name: 'Backing Up Namespaces or Datasets',
                    local: 'kubernetes_CSI_00010.html',
                    children: [
                      {
                        id: 1532,
                        parentId: 1522,
                        name:
                          'Step 1: (Optional) Querying Node Labels of the Kubernetes Cluster',
                        local: 'kubernetes_CSI_00010_1.html'
                      },
                      {
                        id: 1533,
                        parentId: 1522,
                        name:
                          'Step 2: (Optional) Generating a Token with the Minimum Permissions',
                        local: 'kubernetes_CSI_00077.html'
                      },
                      {
                        id: 1534,
                        parentId: 1522,
                        name: 'Step 3: Registering a Cluster',
                        local: 'kubernetes_CSI_00011.html'
                      },
                      {
                        id: 1535,
                        parentId: 1522,
                        name: 'Step 4: Registering a Dataset',
                        local: 'kubernetes_CSI_00012.html'
                      },
                      {
                        id: 1536,
                        parentId: 1522,
                        name: 'Step 5: Authorizing a Resource',
                        local: 'kubernetes_CSI_00013.html'
                      },
                      {
                        id: 1537,
                        parentId: 1522,
                        name: 'Step 6: Creating a Rate Limiting Policy',
                        local: 'kubernetes_CSI_00014.html'
                      },
                      {
                        id: 1538,
                        parentId: 1522,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'kubernetes_CSI_00015.html'
                      },
                      {
                        id: 1539,
                        parentId: 1522,
                        name: 'Step 8: Performing Backup',
                        local: 'kubernetes_CSI_00016.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1510,
                parentId: 1507,
                name: 'Replication',
                local: 'kubernetes_CSI_00019.html',
                children: [
                  {
                    id: 1540,
                    parentId: 1510,
                    name: 'Replicating Kubernetes CSI Copies',
                    local: 'kubernetes_CSI_00022.html',
                    children: [
                      {
                        id: 1541,
                        parentId: 1540,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'kubernetes_CSI_00024.html'
                      },
                      {
                        id: 1542,
                        parentId: 1540,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'kubernetes_CSI_00024_1.html'
                      },
                      {
                        id: 1543,
                        parentId: 1540,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'kubernetes_CSI_00025.html'
                      },
                      {
                        id: 1544,
                        parentId: 1540,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'kubernetes_CSI_00026.html'
                      },
                      {
                        id: 1545,
                        parentId: 1540,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'kubernetes_CSI_00027.html'
                      },
                      {
                        id: 1546,
                        parentId: 1540,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'kubernetes_CSI_00028.html'
                      },
                      {
                        id: 1547,
                        parentId: 1540,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'kubernetes_CSI_00028_a1.html'
                      },
                      {
                        id: 1548,
                        parentId: 1540,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'kubernetes_CSI_00029.html'
                      },
                      {
                        id: 1549,
                        parentId: 1540,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'kubernetes_CSI_00030.html'
                      },
                      {
                        id: 1550,
                        parentId: 1540,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'kubernetes_CSI_00031.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1511,
                parentId: 1507,
                name: 'Archiving',
                local: 'kubernetes_CSI_00032.html',
                children: [
                  {
                    id: 1551,
                    parentId: 1511,
                    name: 'Archiving Kubernetes CSI Backup Copies',
                    local: 'kubernetes_CSI_00035.html',
                    children: [
                      {
                        id: 1553,
                        parentId: 1551,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'kubernetes_CSI_00036.html',
                        children: [
                          {
                            id: 1555,
                            parentId: 1553,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'kubernetes_CSI_00037.html'
                          },
                          {
                            id: 1556,
                            parentId: 1553,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'kubernetes_CSI_00038.html'
                          }
                        ]
                      },
                      {
                        id: 1554,
                        parentId: 1551,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'kubernetes_CSI_00039.html'
                      }
                    ]
                  },
                  {
                    id: 1552,
                    parentId: 1511,
                    name: 'Archiving Kubernetes CSI Replication Copies',
                    local: 'kubernetes_CSI_00040.html',
                    children: [
                      {
                        id: 1557,
                        parentId: 1552,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'kubernetes_CSI_00041.html'
                      },
                      {
                        id: 1558,
                        parentId: 1552,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'kubernetes_CSI_00042.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1512,
                parentId: 1507,
                name: 'Restoration',
                local: 'kubernetes_CSI_00043.html',
                children: [
                  {
                    id: 1559,
                    parentId: 1512,
                    name: 'Restoring Namespaces or Datasets',
                    local: 'kubernetes_CSI_00046.html'
                  },
                  {
                    id: 1560,
                    parentId: 1512,
                    name: 'Restoring PVCs',
                    local: 'kubernetes_CSI_00047.html'
                  }
                ]
              },
              {
                id: 1513,
                parentId: 1507,
                name: 'Global Search',
                local: 'kubernetes_CSI_00043_a1.html',
                children: [
                  {
                    id: 1561,
                    parentId: 1513,
                    name: 'Global Search for Resources',
                    local: 'kubernetes_CSI_00043_a2.html'
                  },
                  {
                    id: 1562,
                    parentId: 1513,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'kubernetes_CSI_00043_a3.html'
                  }
                ]
              },
              {
                id: 1514,
                parentId: 1507,
                name: 'SLA ',
                local: 'kubernetes_CSI_00051.html',
                children: [
                  {
                    id: 1563,
                    parentId: 1514,
                    name: 'About SLA',
                    local: 'kubernetes_CSI_00052.html'
                  },
                  {
                    id: 1564,
                    parentId: 1514,
                    name: 'Viewing SLA Information',
                    local: 'kubernetes_CSI_00053.html'
                  },
                  {
                    id: 1565,
                    parentId: 1514,
                    name: 'Managing SLAs',
                    local: 'kubernetes_CSI_00054.html'
                  }
                ]
              },
              {
                id: 1515,
                parentId: 1507,
                name: 'Copies',
                local: 'kubernetes_CSI_00055.html',
                children: [
                  {
                    id: 1566,
                    parentId: 1515,
                    name: 'Viewing Kubernetes CSI Copy Information',
                    local: 'kubernetes_CSI_00056.html'
                  },
                  {
                    id: 1567,
                    parentId: 1515,
                    name: 'Managing Kubernetes CSI Copies',
                    local: 'kubernetes_CSI_00057.html'
                  }
                ]
              },
              {
                id: 1516,
                parentId: 1507,
                name: 'Cluster/Namespace/Dataset',
                local: 'kubernetes_CSI_00058.html',
                children: [
                  {
                    id: 1568,
                    parentId: 1516,
                    name: 'Viewing Information',
                    local: 'kubernetes_CSI_00059.html'
                  },
                  {
                    id: 1569,
                    parentId: 1516,
                    name: 'Managing Clusters',
                    local: 'kubernetes_CSI_00060.html'
                  },
                  {
                    id: 1570,
                    parentId: 1516,
                    name: 'Managing Namespace and Datasets',
                    local: 'kubernetes_CSI_00061.html'
                  }
                ]
              },
              {
                id: 1517,
                parentId: 1507,
                name: 'FAQs',
                local: 'kubernetes_CSI_00062.html',
                children: [
                  {
                    id: 1571,
                    parentId: 1517,
                    name:
                      'Obtaining the Certificate Value During Token Authentication (for CCE)',
                    local: 'kubernetes_CSI_00079.html'
                  },
                  {
                    id: 1572,
                    parentId: 1517,
                    name:
                      'Pod Configuration in the Production Environment for Application-Consistent Backup (General)',
                    local: 'kubernetes_CSI_00066.html'
                  },
                  {
                    id: 1573,
                    parentId: 1517,
                    name:
                      'Pod Configuration in the Production Environment for Application-Consistent Backup (MySQL as the Containerized Application)',
                    local: 'kubernetes_CSI_00067.html'
                  },
                  {
                    id: 1574,
                    parentId: 1517,
                    name:
                      'Pod Configuration in the Production Environment for Application-Consistent Backup (openGauss as the Containerized Application)',
                    local: 'kubernetes_CSI_00068.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1508,
            parentId: 16,
            name: 'Kubernetes FlexVolume Data Protection',
            local: 'en-us_topic_0000001827039668.html',
            children: [
              {
                id: 1575,
                parentId: 1508,
                name: 'Backup',
                local: 'kubernetes_gud_00007.html',
                children: [
                  {
                    id: 1583,
                    parentId: 1575,
                    name: 'Preparations for Backup',
                    local: 'kubernetes_gud_00010.html'
                  },
                  {
                    id: 1584,
                    parentId: 1575,
                    name: 'Backing Up a Namespace or StatefulSet',
                    local: 'kubernetes_gud_00011.html',
                    children: [
                      {
                        id: 1585,
                        parentId: 1584,
                        name: 'Step 1: Registering a Cluster',
                        local: 'kubernetes_gud_00012.html'
                      },
                      {
                        id: 1586,
                        parentId: 1584,
                        name: 'Step 2: Authorizing a Resource',
                        local: 'kubernetes_gud_00013.html'
                      },
                      {
                        id: 1587,
                        parentId: 1584,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'kubernetes_gud_00014.html'
                      },
                      {
                        id: 1588,
                        parentId: 1584,
                        name: 'Step 4: Creating a Rate Limiting Policy',
                        local: 'kubernetes_gud_00015.html'
                      },
                      {
                        id: 1589,
                        parentId: 1584,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'kubernetes_gud_00016.html'
                      },
                      {
                        id: 1590,
                        parentId: 1584,
                        name: 'Step 6: Performing Backup',
                        local: 'kubernetes_gud_00017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1576,
                parentId: 1508,
                name: 'Replication',
                local: 'kubernetes_gud_00020.html',
                children: [
                  {
                    id: 1591,
                    parentId: 1576,
                    name: 'Replicating Kubernetes FlexVolume Copies',
                    local: 'kubernetes_gud_00023.html',
                    children: [
                      {
                        id: 1592,
                        parentId: 1591,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'kubernetes_gud_00025.html'
                      },
                      {
                        id: 1593,
                        parentId: 1591,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_4.html'
                      },
                      {
                        id: 1594,
                        parentId: 1591,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'kubernetes_gud_00026.html'
                      },
                      {
                        id: 1595,
                        parentId: 1591,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'kubernetes_gud_00027.html'
                      },
                      {
                        id: 1596,
                        parentId: 1591,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'kubernetes_gud_00028.html'
                      },
                      {
                        id: 1597,
                        parentId: 1591,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'kubernetes_gud_00029.html'
                      },
                      {
                        id: 1598,
                        parentId: 1591,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'kubernetes_gud_00029_a1.html'
                      },
                      {
                        id: 1599,
                        parentId: 1591,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'kubernetes_gud_00030.html'
                      },
                      {
                        id: 1600,
                        parentId: 1591,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'kubernetes_gud_00031.html'
                      },
                      {
                        id: 1601,
                        parentId: 1591,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'kubernetes_gud_00032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1577,
                parentId: 1508,
                name: 'Archiving',
                local: 'kubernetes_gud_00033.html',
                children: [
                  {
                    id: 1602,
                    parentId: 1577,
                    name: 'Archiving Kubernetes FlexVolume Backup Copies',
                    local: 'kubernetes_gud_00036.html',
                    children: [
                      {
                        id: 1604,
                        parentId: 1602,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'kubernetes_gud_00037.html',
                        children: [
                          {
                            id: 1606,
                            parentId: 1604,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'kubernetes_gud_00038.html'
                          },
                          {
                            id: 1607,
                            parentId: 1604,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'kubernetes_gud_00039.html'
                          }
                        ]
                      },
                      {
                        id: 1605,
                        parentId: 1602,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'kubernetes_gud_00040.html'
                      }
                    ]
                  },
                  {
                    id: 1603,
                    parentId: 1577,
                    name: 'Archiving Kubernetes FlexVolume Replication Copies',
                    local: 'kubernetes_gud_00041.html',
                    children: [
                      {
                        id: 1608,
                        parentId: 1603,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'kubernetes_gud_00042.html'
                      },
                      {
                        id: 1609,
                        parentId: 1603,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'kubernetes_gud_00043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1578,
                parentId: 1508,
                name: 'Restoration',
                local: 'kubernetes_gud_00044.html',
                children: [
                  {
                    id: 1610,
                    parentId: 1578,
                    name: 'Performing StatefulSet Restoration',
                    local: 'kubernetes_gud_00047.html'
                  }
                ]
              },
              {
                id: 1579,
                parentId: 1508,
                name: 'Global Search',
                local: 'kubernetes_gud_00044_a1.html',
                children: [
                  {
                    id: 1611,
                    parentId: 1579,
                    name: 'Global Search for Resources',
                    local: 'kubernetes_gud_00044_a2.html'
                  },
                  {
                    id: 1612,
                    parentId: 1579,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'kubernetes_gud_00044_a3.html'
                  }
                ]
              },
              {
                id: 1580,
                parentId: 1508,
                name: 'SLA ',
                local: 'kubernetes_gud_00051.html',
                children: [
                  {
                    id: 1613,
                    parentId: 1580,
                    name: 'About SLA',
                    local: 'kubernetes_gud_00052.html'
                  },
                  {
                    id: 1614,
                    parentId: 1580,
                    name: 'Viewing SLA Information',
                    local: 'kubernetes_gud_00053.html'
                  },
                  {
                    id: 1615,
                    parentId: 1580,
                    name: 'Managing SLAs',
                    local: 'kubernetes_gud_00054.html'
                  }
                ]
              },
              {
                id: 1581,
                parentId: 1508,
                name: 'Copies',
                local: 'kubernetes_gud_00055.html',
                children: [
                  {
                    id: 1616,
                    parentId: 1581,
                    name: 'Viewing Kubernetes FlexVolume Copy Information',
                    local: 'kubernetes_gud_00056.html'
                  },
                  {
                    id: 1617,
                    parentId: 1581,
                    name: 'Managing Kubernetes FlexVolume Copies',
                    local: 'kubernetes_gud_00057.html'
                  }
                ]
              },
              {
                id: 1582,
                parentId: 1508,
                name: 'Cluster, Namespace, and StatefulSet',
                local: 'kubernetes_gud_00058.html',
                children: [
                  {
                    id: 1618,
                    parentId: 1582,
                    name: 'Viewing Information',
                    local: 'kubernetes_gud_00059.html'
                  },
                  {
                    id: 1619,
                    parentId: 1582,
                    name: 'Managing Clusters',
                    local: 'kubernetes_gud_00060.html'
                  },
                  {
                    id: 1620,
                    parentId: 1582,
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
            id: 1621,
            parentId: 17,
            name: 'Huawei Cloud Stack Data Protection',
            local: 'en-us_topic_0000001827039672.html',
            children: [
              {
                id: 1625,
                parentId: 1621,
                name: 'Backup',
                local: 'hcs_gud_0007.html',
                children: [
                  {
                    id: 1633,
                    parentId: 1625,
                    name: 'Backing Up a Cloud Server or EVS Disk',
                    local: 'hcs_gud_0011.html',
                    children: [
                      {
                        id: 1634,
                        parentId: 1633,
                        name: 'Step 1: Obtaining the Certificate',
                        local: 'hcs_gud_0012.html'
                      },
                      {
                        id: 1635,
                        parentId: 1633,
                        name: 'Step 2: Registering Huawei Cloud Stack',
                        local: 'hcs_gud_0014.html'
                      },
                      {
                        id: 1636,
                        parentId: 1633,
                        name:
                          'Step 3: Adding a Tenant and Authorizing Resources',
                        local: 'hcs_gud_0015.html'
                      },
                      {
                        id: 1637,
                        parentId: 1633,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'hcs_gud_0016.html'
                      },
                      {
                        id: 1638,
                        parentId: 1633,
                        name:
                          '(Optional) Step 5: Creating a Rate Limiting Policy',
                        local: 'hcs_gud_0017.html'
                      },
                      {
                        id: 1639,
                        parentId: 1633,
                        name: 'Step 6: Creating a Backup SLA',
                        local: 'hcs_gud_0018.html'
                      },
                      {
                        id: 1640,
                        parentId: 1633,
                        name: 'Step 7: Performing Backup',
                        local: 'hcs_gud_0019.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1626,
                parentId: 1621,
                name: 'Replication',
                local: 'hcs_gud_0022.html',
                children: [
                  {
                    id: 1641,
                    parentId: 1626,
                    name: 'Replicating a Huawei Cloud Stack Copy',
                    local: 'hcs_gud_0025.html',
                    children: [
                      {
                        id: 1642,
                        parentId: 1641,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'hcs_gud_0027.html'
                      },
                      {
                        id: 1643,
                        parentId: 1641,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'hcs_gud_0027_1.html'
                      },
                      {
                        id: 1644,
                        parentId: 1641,
                        name: 'Step 2: (Optional) Creating an IPsec Policy',
                        local: 'hcs_gud_0028.html'
                      },
                      {
                        id: 1645,
                        parentId: 1641,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'hcs_gud_0029.html'
                      },
                      {
                        id: 1646,
                        parentId: 1641,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'hcs_gud_0030.html'
                      },
                      {
                        id: 1647,
                        parentId: 1641,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'hcs_gud_0031.html'
                      },
                      {
                        id: 1648,
                        parentId: 1641,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002044303453.html'
                      },
                      {
                        id: 1649,
                        parentId: 1641,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'hcs_gud_0032.html'
                      },
                      {
                        id: 1650,
                        parentId: 1641,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'hcs_gud_0033.html'
                      },
                      {
                        id: 1651,
                        parentId: 1641,
                        name:
                          'Step 8: (Optional) Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'hcs_gud_0034.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1627,
                parentId: 1621,
                name: 'Archiving',
                local: 'hcs_gud_0035.html',
                children: [
                  {
                    id: 1652,
                    parentId: 1627,
                    name: 'Archiving Huawei Cloud Stack Backup Copies',
                    local: 'hcs_gud_0038.html',
                    children: [
                      {
                        id: 1654,
                        parentId: 1652,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'hcs_gud_0039.html',
                        children: [
                          {
                            id: 1656,
                            parentId: 1654,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'hcs_gud_0040.html'
                          },
                          {
                            id: 1657,
                            parentId: 1654,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'hcs_gud_0041.html'
                          }
                        ]
                      },
                      {
                        id: 1655,
                        parentId: 1652,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'hcs_gud_0042.html'
                      }
                    ]
                  },
                  {
                    id: 1653,
                    parentId: 1627,
                    name: 'Archiving Huawei Cloud Stack Replication Copies',
                    local: 'hcs_gud_0043.html',
                    children: [
                      {
                        id: 1658,
                        parentId: 1653,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'hcs_gud_0044.html'
                      },
                      {
                        id: 1659,
                        parentId: 1653,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'hcs_gud_0045.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1628,
                parentId: 1621,
                name: 'Restoration',
                local: 'hcs_gud_0046.html',
                children: [
                  {
                    id: 1660,
                    parentId: 1628,
                    name: 'Restoring a Cloud Server or EVS Disk',
                    local: 'hcs_gud_0049.html'
                  },
                  {
                    id: 1661,
                    parentId: 1628,
                    name: 'Restoring Files on an ECS',
                    local: 'hcs_gud_re1.html'
                  }
                ]
              },
              {
                id: 1629,
                parentId: 1621,
                name: 'Global Search',
                local: 'hcs_gud_gs1.html',
                children: [
                  {
                    id: 1662,
                    parentId: 1629,
                    name: 'About Global Search',
                    local: 'en-us_topic_0000002002687814.html'
                  },
                  {
                    id: 1663,
                    parentId: 1629,
                    name: 'Global Search for Copies',
                    local: 'hcs_gud_gs2.html'
                  },
                  {
                    id: 1664,
                    parentId: 1629,
                    name: 'Global Search for Resources',
                    local: 'hcs_gud_0050.html'
                  },
                  {
                    id: 1665,
                    parentId: 1629,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002038886953.html'
                  }
                ]
              },
              {
                id: 1630,
                parentId: 1621,
                name: 'SLA ',
                local: 'hcs_gud_0053.html',
                children: [
                  {
                    id: 1666,
                    parentId: 1630,
                    name: 'About SLA',
                    local: 'hcs_gud_0054.html'
                  },
                  {
                    id: 1667,
                    parentId: 1630,
                    name: 'Viewing SLA Information',
                    local: 'hcs_gud_0055.html'
                  },
                  {
                    id: 1668,
                    parentId: 1630,
                    name: 'Managing SLAs',
                    local: 'hcs_gud_0056.html'
                  }
                ]
              },
              {
                id: 1631,
                parentId: 1621,
                name: 'Copies',
                local: 'hcs_gud_0057.html',
                children: [
                  {
                    id: 1669,
                    parentId: 1631,
                    name: 'Viewing Huawei Cloud Stack Copy Information',
                    local: 'hcs_gud_0058.html'
                  },
                  {
                    id: 1670,
                    parentId: 1631,
                    name: 'Managing Huawei Cloud Stack Copies',
                    local: 'hcs_gud_0059.html'
                  }
                ]
              },
              {
                id: 1632,
                parentId: 1621,
                name: 'Huawei Cloud Stack Environment',
                local: 'hcs_gud_0060.html',
                children: [
                  {
                    id: 1671,
                    parentId: 1632,
                    name: 'Viewing Huawei Cloud Stack information',
                    local: 'hcs_gud_0061.html'
                  },
                  {
                    id: 1672,
                    parentId: 1632,
                    name:
                      'Managing Huawei Cloud Stack Registration Information',
                    local: 'hcs_gud_0062.html'
                  },
                  {
                    id: 1673,
                    parentId: 1632,
                    name: 'Managing Tenants',
                    local: 'hcs_gud_0063.html'
                  },
                  {
                    id: 1674,
                    parentId: 1632,
                    name: 'Managing Projects, Resource Sets, and ECSs',
                    local: 'hcs_gud_0064.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1622,
            parentId: 17,
            name: 'OpenStack Data Protection',
            local: 'en-us_topic_0000001873679145.html',
            children: [
              {
                id: 1675,
                parentId: 1622,
                name: 'Backup',
                local: 'openstack_00007.html',
                children: [
                  {
                    id: 1683,
                    parentId: 1675,
                    name: 'Preparing for the Backup',
                    local: 'openstack_00010.html'
                  },
                  {
                    id: 1684,
                    parentId: 1675,
                    name: 'Backing Up an OpenStack Cloud Server',
                    local: 'openstack_00011.html',
                    children: [
                      {
                        id: 1685,
                        parentId: 1684,
                        name: 'Step 1: Obtaining the Keystone V3 Address',
                        local: 'Open_Stack_00010_1.html'
                      },
                      {
                        id: 1686,
                        parentId: 1684,
                        name: 'Step 2: Obtaining the Certificate',
                        local: 'openstack_00012.html'
                      },
                      {
                        id: 1687,
                        parentId: 1684,
                        name: 'Step 3: Creating a User for Interconnection',
                        local: 'openstack_00014.html'
                      },
                      {
                        id: 1688,
                        parentId: 1684,
                        name: 'Step 4: Creating a Domain Administrator',
                        local: 'openstack_00015.html'
                      },
                      {
                        id: 1689,
                        parentId: 1684,
                        name: 'Step 5: Registering OpenStack',
                        local: 'openstack_00016.html'
                      },
                      {
                        id: 1690,
                        parentId: 1684,
                        name: 'Step 6: Adding a Domain',
                        local: 'openstack_00017.html'
                      },
                      {
                        id: 1691,
                        parentId: 1684,
                        name:
                          'Step 7: (Optional) Creating a Cloud Server Group',
                        local: 'openstack_000017_1.html'
                      },
                      {
                        id: 1692,
                        parentId: 1684,
                        name: 'Step 8: Creating a Rate Limiting Policy',
                        local: 'openstack_00019.html'
                      },
                      {
                        id: 1693,
                        parentId: 1684,
                        name:
                          'Step 9: (Optional) Enabling Backup Link Encryption',
                        local: 'openstack_00020.html'
                      },
                      {
                        id: 1694,
                        parentId: 1684,
                        name:
                          'Step 10: (Optional) Modifying the Snapshot Quota of a Project',
                        local: 'openstack_00021.html'
                      },
                      {
                        id: 1695,
                        parentId: 1684,
                        name: 'Step 11: Creating a Backup SLA',
                        local: 'openstack_00022.html'
                      },
                      {
                        id: 1696,
                        parentId: 1684,
                        name: 'Step 12: Performing Backup',
                        local: 'openstack_00023.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1676,
                parentId: 1622,
                name: 'Replication',
                local: 'openstack_00026.html',
                children: [
                  {
                    id: 1697,
                    parentId: 1676,
                    name: 'Replicating an OpenStack Copy',
                    local: 'openstack_00029.html',
                    children: [
                      {
                        id: 1698,
                        parentId: 1697,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'openstack_00031.html'
                      },
                      {
                        id: 1699,
                        parentId: 1697,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'openstack_000311.html'
                      },
                      {
                        id: 1700,
                        parentId: 1697,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'openstack_00032.html'
                      },
                      {
                        id: 1701,
                        parentId: 1697,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'openstack_00033.html'
                      },
                      {
                        id: 1702,
                        parentId: 1697,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'openstack_00034.html'
                      },
                      {
                        id: 1703,
                        parentId: 1697,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'openstack_00035.html'
                      },
                      {
                        id: 1704,
                        parentId: 1697,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'Open_Stack_000340.html'
                      },
                      {
                        id: 1705,
                        parentId: 1697,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'openstack_00036.html'
                      },
                      {
                        id: 1706,
                        parentId: 1697,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'openstack_00037.html'
                      },
                      {
                        id: 1707,
                        parentId: 1697,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'openstack_00038.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1677,
                parentId: 1622,
                name: 'Archiving',
                local: 'openstack_00039.html',
                children: [
                  {
                    id: 1708,
                    parentId: 1677,
                    name: 'Archiving OpenStack Backup Copies',
                    local: 'openstack_00042.html',
                    children: [
                      {
                        id: 1710,
                        parentId: 1708,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'openstack_00043.html',
                        children: [
                          {
                            id: 1712,
                            parentId: 1710,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'openstack_00044.html'
                          },
                          {
                            id: 1713,
                            parentId: 1710,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'openstack_00045.html'
                          }
                        ]
                      },
                      {
                        id: 1711,
                        parentId: 1708,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'openstack_00046.html'
                      }
                    ]
                  },
                  {
                    id: 1709,
                    parentId: 1677,
                    name: 'Archiving OpenStack Replication Copies',
                    local: 'openstack_00047.html',
                    children: [
                      {
                        id: 1714,
                        parentId: 1709,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'openstack_00048.html'
                      },
                      {
                        id: 1715,
                        parentId: 1709,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'openstack_00049.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1678,
                parentId: 1622,
                name: 'Restoration',
                local: 'openstack_00050.html',
                children: [
                  {
                    id: 1716,
                    parentId: 1678,
                    name: 'Restoring Cloud Servers',
                    local: 'openstack_00053.html'
                  },
                  {
                    id: 1717,
                    parentId: 1678,
                    name: 'Restoring Cloud Disks',
                    local: 'openstack_00054.html'
                  },
                  {
                    id: 1718,
                    parentId: 1678,
                    name:
                      'Restoring Files (Applicable to 1.6.0 and Later Versions)',
                    local: 'openstack_000541.html'
                  }
                ]
              },
              {
                id: 1679,
                parentId: 1622,
                name: 'Global Search',
                local: 'Open_Stack_000532.html',
                children: [
                  {
                    id: 1719,
                    parentId: 1679,
                    name: 'Global Search for Resources',
                    local: 'openstack_00055.html'
                  },
                  {
                    id: 1720,
                    parentId: 1679,
                    name: 'Global Search for Copies',
                    local: 'vmware_gud_0076_0.html'
                  },
                  {
                    id: 1721,
                    parentId: 1679,
                    name: 'Global Search for Resources',
                    local: 'Open_Stack_000542.html'
                  },
                  {
                    id: 1722,
                    parentId: 1679,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'Open_Stack_000543.html'
                  }
                ]
              },
              {
                id: 1680,
                parentId: 1622,
                name: 'SLA ',
                local: 'openstack_00058.html',
                children: [
                  {
                    id: 1723,
                    parentId: 1680,
                    name: 'About SLA',
                    local: 'openstack_00059.html'
                  },
                  {
                    id: 1724,
                    parentId: 1680,
                    name: 'Viewing SLA Information',
                    local: 'openstack_00060.html'
                  },
                  {
                    id: 1725,
                    parentId: 1680,
                    name: 'Managing SLAs',
                    local: 'openstack_00061.html'
                  }
                ]
              },
              {
                id: 1681,
                parentId: 1622,
                name: 'Copies',
                local: 'openstack_00062.html',
                children: [
                  {
                    id: 1726,
                    parentId: 1681,
                    name: 'Viewing OpenStack Copy Data',
                    local: 'openstack_00063.html'
                  },
                  {
                    id: 1727,
                    parentId: 1681,
                    name: 'Managing OpenStack Copies',
                    local: 'openstack_00064.html'
                  }
                ]
              },
              {
                id: 1682,
                parentId: 1622,
                name: 'OpenStack Environment Information',
                local: 'openstack_00065.html',
                children: [
                  {
                    id: 1728,
                    parentId: 1682,
                    name: 'Viewing OpenStack Information',
                    local: 'openstack_00066.html'
                  },
                  {
                    id: 1729,
                    parentId: 1682,
                    name: 'Managing the OpenStack Cloud Platform',
                    local: 'openstack_00067.html'
                  },
                  {
                    id: 1730,
                    parentId: 1682,
                    name: 'Managing Domains',
                    local: 'openstack_00068.html'
                  },
                  {
                    id: 1731,
                    parentId: 1682,
                    name:
                      'Managing Projects/Cloud Servers or Cloud Server Groups (Applicable to 1.6.0 and Later Versions)',
                    local: 'openstack_00069.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1623,
            parentId: 17,
            name: 'Huawei Cloud Stack GaussDB Data Protection',
            local: 'en-us_topic_0000001826879800.html',
            children: [
              {
                id: 1732,
                parentId: 1623,
                name: 'Backup',
                local: 'hcs_gaussdb_00006.html',
                children: [
                  {
                    id: 1740,
                    parentId: 1732,
                    name: 'Preparing for the Backup',
                    local: 'hcs_gaussdb_00009.html'
                  },
                  {
                    id: 1741,
                    parentId: 1732,
                    name: 'Backing Up Huawei Cloud Stack GaussDB Instances',
                    local: 'hcs_gaussdb_00010.html',
                    children: [
                      {
                        id: 1742,
                        parentId: 1741,
                        name:
                          'Step 1: Registering a Huawei Cloud Stack GaussDB Project',
                        local: 'hcs_gaussdb_00011.html'
                      },
                      {
                        id: 1743,
                        parentId: 1741,
                        name:
                          'Step 2: (Optional) Enabling Backup Link Encryption',
                        local: 'hcs_gaussdb_00012.html'
                      },
                      {
                        id: 1744,
                        parentId: 1741,
                        name: 'Step 3: Creating a Rate Limiting Policy',
                        local: 'hcs_gaussdb_00013.html'
                      },
                      {
                        id: 1745,
                        parentId: 1741,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'hcs_gaussdb_00014.html'
                      },
                      {
                        id: 1746,
                        parentId: 1741,
                        name: 'Step 5: Performing Backup',
                        local: 'hcs_gaussdb_00015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1733,
                parentId: 1623,
                name: 'Replication',
                local: 'hcs_gaussdb_00018.html',
                children: [
                  {
                    id: 1747,
                    parentId: 1733,
                    name: 'Replicating Huawei Cloud Stack GaussDB Copies',
                    local: 'hcs_gaussdb_00021.html',
                    children: [
                      {
                        id: 1748,
                        parentId: 1747,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'hcs_gaussdb_00023.html'
                      },
                      {
                        id: 1749,
                        parentId: 1747,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_17.html'
                      },
                      {
                        id: 1750,
                        parentId: 1747,
                        name: 'Step 2: (Optional) Creating an IPsec Policy',
                        local: 'hcs_gaussdb_00024.html'
                      },
                      {
                        id: 1751,
                        parentId: 1747,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'hcs_gaussdb_00025.html'
                      },
                      {
                        id: 1752,
                        parentId: 1747,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'hcs_gaussdb_00026.html'
                      },
                      {
                        id: 1753,
                        parentId: 1747,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'hcs_gaussdb_00027.html'
                      },
                      {
                        id: 1754,
                        parentId: 1747,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'hcs_gaussdb_0002700.html'
                      },
                      {
                        id: 1755,
                        parentId: 1747,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'hcs_gaussdb_00028.html'
                      },
                      {
                        id: 1756,
                        parentId: 1747,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'hcs_gaussdb_00029.html'
                      },
                      {
                        id: 1757,
                        parentId: 1747,
                        name:
                          'Step 8: (Optional) Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'hcs_gaussdb_00030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1734,
                parentId: 1623,
                name: 'Archiving',
                local: 'hcs_gaussdb_00031.html',
                children: [
                  {
                    id: 1758,
                    parentId: 1734,
                    name: 'Archiving Huawei Cloud Stack GaussDB Copies',
                    local: 'hcs_gaussdb_00034.html',
                    children: [
                      {
                        id: 1760,
                        parentId: 1758,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'hcs_gaussdb_00035.html',
                        children: [
                          {
                            id: 1762,
                            parentId: 1760,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'hcs_gaussdb_00036.html'
                          },
                          {
                            id: 1763,
                            parentId: 1760,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'hcs_gaussdb_00037.html'
                          }
                        ]
                      },
                      {
                        id: 1761,
                        parentId: 1758,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'hcs_gaussdb_00038.html'
                      }
                    ]
                  },
                  {
                    id: 1759,
                    parentId: 1734,
                    name: 'Archiving Huawei Cloud Stack GaussDB Copies',
                    local: 'hcs_gaussdb_00039.html',
                    children: [
                      {
                        id: 1764,
                        parentId: 1759,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'hcs_gaussdb_00040.html'
                      },
                      {
                        id: 1765,
                        parentId: 1759,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'hcs_gaussdb_00041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1735,
                parentId: 1623,
                name: 'Restoration',
                local: 'hcs_gaussdb_00042.html',
                children: [
                  {
                    id: 1766,
                    parentId: 1735,
                    name: 'Restoring Huawei Cloud Stack GaussDB Instances',
                    local: 'hcs_gaussdb_00045.html'
                  }
                ]
              },
              {
                id: 1736,
                parentId: 1623,
                name: 'Global Search',
                local: 'hcs_gaussdb_0004211.html',
                children: [
                  {
                    id: 1767,
                    parentId: 1736,
                    name: 'Global Search for Resources',
                    local: 'hcs_gaussdb_00046.html'
                  },
                  {
                    id: 1768,
                    parentId: 1736,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002050772613.html'
                  }
                ]
              },
              {
                id: 1737,
                parentId: 1623,
                name: 'SLA ',
                local: 'hcs_gaussdb_00049.html',
                children: [
                  {
                    id: 1769,
                    parentId: 1737,
                    name: 'About SLA',
                    local: 'hcs_gaussdb_000491.html'
                  },
                  {
                    id: 1770,
                    parentId: 1737,
                    name: 'Viewing SLA Information',
                    local: 'hcs_gaussdb_00051.html'
                  },
                  {
                    id: 1771,
                    parentId: 1737,
                    name: 'Managing SLAs',
                    local: 'hcs_gaussdb_00052.html'
                  }
                ]
              },
              {
                id: 1738,
                parentId: 1623,
                name: 'Copies',
                local: 'hcs_gaussdb_00053.html',
                children: [
                  {
                    id: 1772,
                    parentId: 1738,
                    name: 'Viewing Huawei Cloud Stack GaussDB Copy Information',
                    local: 'hcs_gaussdb_00054.html'
                  },
                  {
                    id: 1773,
                    parentId: 1738,
                    name: 'Managing Huawei Cloud Stack GaussDB Copies',
                    local: 'hcs_gaussdb_00055.html'
                  }
                ]
              },
              {
                id: 1739,
                parentId: 1623,
                name: 'Huawei Cloud Stack GaussDB',
                local: 'hcs_gaussdb_00056.html',
                children: [
                  {
                    id: 1774,
                    parentId: 1739,
                    name: 'Viewing Huawei Cloud Stack GaussDB Information',
                    local: 'hcs_gaussdb_00057.html'
                  },
                  {
                    id: 1775,
                    parentId: 1739,
                    name: 'Managing Huawei Cloud Stack GaussDB Projects',
                    local: 'hcs_gaussdb_00058.html'
                  },
                  {
                    id: 1776,
                    parentId: 1739,
                    name: 'Managing Instances',
                    local: 'hcs_gaussdb_00059.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1624,
            parentId: 17,
            name:
              'Alibaba Cloud Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001826879812.html',
            children: [
              {
                id: 1777,
                parentId: 1624,
                name: 'Backup',
                local: 'acloud_00007.html',
                children: [
                  {
                    id: 1785,
                    parentId: 1777,
                    name: 'Preparations for Backup',
                    local: 'acloud_00010.html'
                  },
                  {
                    id: 1786,
                    parentId: 1777,
                    name: 'Backing Up Alibaba Cloud Servers',
                    local: 'acloud_00011.html',
                    children: [
                      {
                        id: 1787,
                        parentId: 1786,
                        name:
                          'Step 4: Registering the Alibaba Cloud Organization',
                        local: 'acloud_00016.html'
                      },
                      {
                        id: 1788,
                        parentId: 1786,
                        name: 'Step 5: Creating a Rate Limiting Policy',
                        local: 'acloud_00017.html'
                      },
                      {
                        id: 1789,
                        parentId: 1786,
                        name:
                          'Step 6: (Optional) Enabling Backup Link Encryption',
                        local: 'acloud_00018.html'
                      },
                      {
                        id: 1790,
                        parentId: 1786,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'acloud_00019.html'
                      },
                      {
                        id: 1791,
                        parentId: 1786,
                        name: 'Step 8: Performing Backup',
                        local: 'acloud_00020.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1778,
                parentId: 1624,
                name: 'Replication',
                local: 'acloud_00023.html',
                children: [
                  {
                    id: 1792,
                    parentId: 1778,
                    name: 'Replicating Alibaba Cloud Copies',
                    local: 'acloud_00026.html',
                    children: [
                      {
                        id: 1793,
                        parentId: 1792,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'acloud_00028.html'
                      },
                      {
                        id: 1794,
                        parentId: 1792,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'acloud_00029.html'
                      },
                      {
                        id: 1795,
                        parentId: 1792,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'acloud_00030.html'
                      },
                      {
                        id: 1796,
                        parentId: 1792,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'acloud_00031.html'
                      },
                      {
                        id: 1797,
                        parentId: 1792,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'acloud_00032.html'
                      },
                      {
                        id: 1798,
                        parentId: 1792,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'acloud_00033.html'
                      },
                      {
                        id: 1799,
                        parentId: 1792,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'Open_Stack_000340_0.html'
                      },
                      {
                        id: 1800,
                        parentId: 1792,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'acloud_00034.html'
                      },
                      {
                        id: 1801,
                        parentId: 1792,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'acloud_00035.html'
                      },
                      {
                        id: 1802,
                        parentId: 1792,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'acloud_00036.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1779,
                parentId: 1624,
                name: 'Archiving',
                local: 'acloud_00037.html',
                children: [
                  {
                    id: 1803,
                    parentId: 1779,
                    name: 'Archiving Alibaba Cloud Backup Copies',
                    local: 'acloud_00040.html',
                    children: [
                      {
                        id: 1805,
                        parentId: 1803,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'acloud_00041.html',
                        children: [
                          {
                            id: 1807,
                            parentId: 1805,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'acloud_00042.html'
                          },
                          {
                            id: 1808,
                            parentId: 1805,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'acloud_00043.html'
                          }
                        ]
                      },
                      {
                        id: 1806,
                        parentId: 1803,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'acloud_00044.html'
                      }
                    ]
                  },
                  {
                    id: 1804,
                    parentId: 1779,
                    name: 'Archiving Alibaba Cloud Replication Copies',
                    local: 'acloud_00045.html',
                    children: [
                      {
                        id: 1809,
                        parentId: 1804,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'acloud_00046.html'
                      },
                      {
                        id: 1810,
                        parentId: 1804,
                        name:
                          'Step 2: Creating a Periodic Archive Plan for Replication Copies',
                        local: 'acloud_00047.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1780,
                parentId: 1624,
                name: 'Restoration',
                local: 'acloud_00048.html',
                children: [
                  {
                    id: 1811,
                    parentId: 1780,
                    name: 'Restoring Cloud Servers',
                    local: 'acloud_00051.html'
                  },
                  {
                    id: 1812,
                    parentId: 1780,
                    name: 'Restoring Cloud Disks',
                    local: 'acloud_00052.html'
                  },
                  {
                    id: 1813,
                    parentId: 1780,
                    name: 'Restoring Files',
                    local: 'acloud_00053.html'
                  }
                ]
              },
              {
                id: 1781,
                parentId: 1624,
                name: 'Global Search',
                local: 'en-us_topic_0000002014804512.html',
                children: [
                  {
                    id: 1814,
                    parentId: 1781,
                    name: 'About Global Search',
                    local: 'en-us_topic_0000002014646280.html'
                  },
                  {
                    id: 1815,
                    parentId: 1781,
                    name: 'Global Search for Copies',
                    local: 'vmware_gud_0076_1.html'
                  },
                  {
                    id: 1816,
                    parentId: 1781,
                    name: 'Global Search for Resources',
                    local: 'acloud_00054.html'
                  },
                  {
                    id: 1817,
                    parentId: 1781,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002050884109.html'
                  }
                ]
              },
              {
                id: 1782,
                parentId: 1624,
                name: 'SLA ',
                local: 'acloud_00057.html',
                children: [
                  {
                    id: 1818,
                    parentId: 1782,
                    name: 'About SLA',
                    local: 'acloud_00058.html'
                  },
                  {
                    id: 1819,
                    parentId: 1782,
                    name: 'Viewing SLA Information',
                    local: 'acloud_00059.html'
                  },
                  {
                    id: 1820,
                    parentId: 1782,
                    name: 'Managing SLAs',
                    local: 'acloud_00060.html'
                  }
                ]
              },
              {
                id: 1783,
                parentId: 1624,
                name: 'Copies',
                local: 'acloud_00061.html',
                children: [
                  {
                    id: 1821,
                    parentId: 1783,
                    name: 'Viewing Alibaba Cloud Copy Information',
                    local: 'acloud_00062.html'
                  },
                  {
                    id: 1822,
                    parentId: 1783,
                    name: 'Managing Alibaba Cloud Copies',
                    local: 'acloud_00063.html'
                  }
                ]
              },
              {
                id: 1784,
                parentId: 1624,
                name: 'Alibaba Cloud Environment Information',
                local: 'acloud_00064.html',
                children: [
                  {
                    id: 1823,
                    parentId: 1784,
                    name: 'Viewing Alibaba Cloud Resource Information',
                    local: 'acloud_00065.html'
                  },
                  {
                    id: 1824,
                    parentId: 1784,
                    name: 'Managing the Alibaba Cloud Platform',
                    local: 'acloud_00066.html'
                  },
                  {
                    id: 1825,
                    parentId: 1784,
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
            id: 1826,
            parentId: 18,
            name:
              'Exchange Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001826879820.html',
            children: [
              {
                id: 1828,
                parentId: 1826,
                name: 'Backup',
                local: 'exchange_gud_00009.html',
                children: [
                  {
                    id: 1835,
                    parentId: 1828,
                    name: 'Preparations for Backup',
                    local: 'exchange_gud_00014.html',
                    children: [
                      {
                        id: 1838,
                        parentId: 1835,
                        name:
                          'Starting the Exchange Information Storage Service',
                        local: 'exchange_gud_00015.html'
                      },
                      {
                        id: 1839,
                        parentId: 1835,
                        name: 'Checking the Exchange Writer Status',
                        local: 'exchange_gud_00016.html'
                      },
                      {
                        id: 1840,
                        parentId: 1835,
                        name: 'Checking the Exchange Database Status',
                        local: 'exchange_gud_00017.html'
                      },
                      {
                        id: 1841,
                        parentId: 1835,
                        name:
                          'Configuring a Database Backup and Restoration Account',
                        local: 'exchange_gud_00018.html'
                      },
                      {
                        id: 1842,
                        parentId: 1835,
                        name:
                          'Configuring a Mailbox Backup and Restoration Account',
                        local: 'exchange_gud_00019.html'
                      }
                    ]
                  },
                  {
                    id: 1836,
                    parentId: 1828,
                    name:
                      'Backing Up an Exchange Single-Node System, Availability Group, or Database',
                    local: 'exchange_gud_00020.html',
                    children: [
                      {
                        id: 1843,
                        parentId: 1836,
                        name:
                          'Step 1: Registering an Exchange Single-Node System or Availability Group',
                        local: 'exchange_gud_00021.html'
                      },
                      {
                        id: 1844,
                        parentId: 1836,
                        name: 'Step 2: Creating a Rate Limiting Policy',
                        local: 'exchange_gud_00022.html'
                      },
                      {
                        id: 1845,
                        parentId: 1836,
                        name: 'Step 3: Creating a Backup SLA',
                        local: 'exchange_gud_00023.html'
                      },
                      {
                        id: 1846,
                        parentId: 1836,
                        name: 'Step 4: Performing the Backup',
                        local: 'exchange_gud_00024.html'
                      }
                    ]
                  },
                  {
                    id: 1837,
                    parentId: 1828,
                    name: 'Backing Up an Exchange Mailbox',
                    local: 'exchange_gud_00027.html',
                    children: [
                      {
                        id: 1847,
                        parentId: 1837,
                        name:
                          'Step 1: Registering an Exchange Single-Node System or Availability Group',
                        local: 'exchange_gud_00028.html'
                      },
                      {
                        id: 1848,
                        parentId: 1837,
                        name: 'Step 2: Creating a Rate Limiting Policy',
                        local: 'exchange_gud_00029.html'
                      },
                      {
                        id: 1849,
                        parentId: 1837,
                        name: 'Step 3: Creating a Backup SLA',
                        local: 'exchange_gud_00030.html'
                      },
                      {
                        id: 1850,
                        parentId: 1837,
                        name: 'Step 4: Performing Backup',
                        local: 'exchange_gud_00031.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1829,
                parentId: 1826,
                name: 'Replication',
                local: 'exchange_gud_00034.html',
                children: [
                  {
                    id: 1851,
                    parentId: 1829,
                    name: 'Replicating an Exchange Database Copy',
                    local: 'exchange_gud_00036.html',
                    children: [
                      {
                        id: 1852,
                        parentId: 1851,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'exchange_gud_000381.html'
                      },
                      {
                        id: 1853,
                        parentId: 1851,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'exchange_00460.html'
                      },
                      {
                        id: 1854,
                        parentId: 1851,
                        name: 'Step : Creating a Replication SLA',
                        local: 'exchange_gud_00044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1830,
                parentId: 1826,
                name: 'Recovery',
                local: 'exchange_gud_00057.html',
                children: [
                  {
                    id: 1855,
                    parentId: 1830,
                    name:
                      'Restoring a Single-Node System or an Availability Group',
                    local: 'exchange_0067.html'
                  },
                  {
                    id: 1856,
                    parentId: 1830,
                    name: 'Restoring an Exchange Database',
                    local: 'exchange_gud_00063.html'
                  },
                  {
                    id: 1857,
                    parentId: 1830,
                    name: 'Restoring a Mailbox',
                    local: 'exchange_gud_00064.html'
                  },
                  {
                    id: 1858,
                    parentId: 1830,
                    name: 'Email-Level Restoration',
                    local: 'exchange_gud_00065.html'
                  },
                  {
                    id: 1859,
                    parentId: 1830,
                    name:
                      'Verifying the Restored Mailbox Data (Applicable to Microsoft Exchange Server 2010)',
                    local: 'exchange_gud_000651.html'
                  },
                  {
                    id: 1860,
                    parentId: 1830,
                    name:
                      'Verifying the Restored Mailbox Data (Applicable to Microsoft Exchange Server 2013 and Later Versions)',
                    local: 'exchange_gud_000652.html'
                  }
                ]
              },
              {
                id: 1831,
                parentId: 1826,
                name: 'Global Search',
                local: 'exchange_00611.html',
                children: [
                  {
                    id: 1861,
                    parentId: 1831,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'exchange_00711.html'
                  }
                ]
              },
              {
                id: 1832,
                parentId: 1826,
                name: 'SLA',
                local: 'exchange_gud_00069.html',
                children: [
                  {
                    id: 1862,
                    parentId: 1832,
                    name: 'About SLA',
                    local: 'exchange_gud_00070.html'
                  },
                  {
                    id: 1863,
                    parentId: 1832,
                    name: 'Viewing SLA Information',
                    local: 'exchange_gud_00071.html'
                  },
                  {
                    id: 1864,
                    parentId: 1832,
                    name: 'Managing SLAs',
                    local: 'exchange_gud_00072.html'
                  }
                ]
              },
              {
                id: 1833,
                parentId: 1826,
                name: 'Copies',
                local: 'exchange_gud_00073.html',
                children: [
                  {
                    id: 1865,
                    parentId: 1833,
                    name: 'Viewing Exchange Copy Information',
                    local: 'exchange_gud_00074.html'
                  },
                  {
                    id: 1866,
                    parentId: 1833,
                    name: 'Managing Exchange Copies',
                    local: 'exchange_gud_00075.html'
                  }
                ]
              },
              {
                id: 1834,
                parentId: 1826,
                name: 'Managing Exchange',
                local: 'exchange_gud_00076.html',
                children: [
                  {
                    id: 1867,
                    parentId: 1834,
                    name: 'Viewing Exchange Environment Information',
                    local: 'exchange_gud_00077.html'
                  },
                  {
                    id: 1868,
                    parentId: 1834,
                    name:
                      'Managing Exchange Single-Node Systems or Availability Groups',
                    local: 'exchange_gud_00080.html'
                  },
                  {
                    id: 1869,
                    parentId: 1834,
                    name: 'Managing Databases or Mailboxes',
                    local: 'exchange_gud_00081.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1827,
            parentId: 18,
            name:
              'Active Directory Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001826879868.html',
            children: [
              {
                id: 1870,
                parentId: 1827,
                name: 'Backup',
                local: 'activedirectory_00007.html',
                children: [
                  {
                    id: 1878,
                    parentId: 1870,
                    name: 'Backing Up Active Directory',
                    local: 'activedirectory_00010.html',
                    children: [
                      {
                        id: 1879,
                        parentId: 1878,
                        name:
                          'Step 1: Enabling the Active Directory Recycle Bin',
                        local: 'en-us_topic_0000002020257438.html'
                      },
                      {
                        id: 1880,
                        parentId: 1878,
                        name:
                          'Step 2: Registering an Active Directory Domain Controller',
                        local: 'activedirectory_00011.html'
                      },
                      {
                        id: 1881,
                        parentId: 1878,
                        name: 'Step 3: Creating a Rate Limiting Policy',
                        local: 'activedirectory_00012.html'
                      },
                      {
                        id: 1882,
                        parentId: 1878,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'activedirectory_00013.html'
                      },
                      {
                        id: 1883,
                        parentId: 1878,
                        name: 'Step 5: Performing Backup',
                        local: 'activedirectory_00014.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1871,
                parentId: 1827,
                name: 'Replication',
                local: 'activedirectory_00017.html',
                children: [
                  {
                    id: 1884,
                    parentId: 1871,
                    name: 'Replicating an Active Directory Copy',
                    local: 'activedirectory_00020.html',
                    children: [
                      {
                        id: 1885,
                        parentId: 1884,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'activedirectory_00022.html'
                      },
                      {
                        id: 1886,
                        parentId: 1884,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_8.html'
                      },
                      {
                        id: 1887,
                        parentId: 1884,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'activedirectory_00023.html'
                      },
                      {
                        id: 1888,
                        parentId: 1884,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'activedirectory_00024.html'
                      },
                      {
                        id: 1889,
                        parentId: 1884,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'activedirectory_00025.html'
                      },
                      {
                        id: 1890,
                        parentId: 1884,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'activedirectory_00026.html'
                      },
                      {
                        id: 1891,
                        parentId: 1884,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'ActiveDirectory-000290.html'
                      },
                      {
                        id: 1892,
                        parentId: 1884,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'activedirectory_00027.html'
                      },
                      {
                        id: 1893,
                        parentId: 1884,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'activedirectory_00028.html'
                      },
                      {
                        id: 1894,
                        parentId: 1884,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'activedirectory_00029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1872,
                parentId: 1827,
                name: 'Archiving',
                local: 'activedirectory_00030.html',
                children: [
                  {
                    id: 1895,
                    parentId: 1872,
                    name: 'Archiving Active Directory Backup Copies',
                    local: 'activedirectory_00033.html',
                    children: [
                      {
                        id: 1897,
                        parentId: 1895,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'activedirectory_00034.html',
                        children: [
                          {
                            id: 1899,
                            parentId: 1897,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'activedirectory_00035.html'
                          },
                          {
                            id: 1900,
                            parentId: 1897,
                            name:
                              'Creating a Media Set (When Archive Storage Is a Tape Library)',
                            local: 'activedirectory_00036.html'
                          }
                        ]
                      },
                      {
                        id: 1898,
                        parentId: 1895,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'activedirectory_00037.html'
                      }
                    ]
                  },
                  {
                    id: 1896,
                    parentId: 1872,
                    name: 'Archiving Active Directory Replication Copies',
                    local: 'activedirectory_00038.html',
                    children: [
                      {
                        id: 1901,
                        parentId: 1896,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'activedirectory_00039.html'
                      },
                      {
                        id: 1902,
                        parentId: 1896,
                        name:
                          'Step 2: Creating a Periodic Archive Plan of Replication Copies',
                        local: 'activedirectory_00040.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1873,
                parentId: 1827,
                name: 'Restoration',
                local: 'activedirectory_00041.html',
                children: [
                  {
                    id: 1903,
                    parentId: 1873,
                    name:
                      'Restoring the System State of Active Directory in the Scenario with a Single Domain Controller',
                    local: 'activedirectory_00044.html'
                  },
                  {
                    id: 1904,
                    parentId: 1873,
                    name:
                      'Restoring Active Directory Objects in the Scenario with a Single Domain Controller',
                    local: 'activedirectory_00045.html'
                  },
                  {
                    id: 1905,
                    parentId: 1873,
                    name:
                      'Restoring the System State of Active Directory in the Scenario with Primary and Backup Domain Controllers',
                    local: 'en-us_topic_0000002080463105.html'
                  },
                  {
                    id: 1906,
                    parentId: 1873,
                    name:
                      'Restoring Active Directory Objects in the Scenario with Primary and Secondary Domain Controllers',
                    local: 'en-us_topic_0000002044462372.html'
                  }
                ]
              },
              {
                id: 1874,
                parentId: 1827,
                name: 'Global Search',
                local: 'activedirectory_00046.html',
                children: [
                  {
                    id: 1907,
                    parentId: 1874,
                    name: 'Global Search for Resources',
                    local: 'activedirectory_00047.html'
                  },
                  {
                    id: 1908,
                    parentId: 1874,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'ActiveDirectory-000491.html'
                  }
                ]
              },
              {
                id: 1875,
                parentId: 1827,
                name: 'SLA ',
                local: 'activedirectory_00050.html',
                children: [
                  {
                    id: 1909,
                    parentId: 1875,
                    name: 'About SLA ',
                    local: 'activedirectory_000501.html'
                  },
                  {
                    id: 1910,
                    parentId: 1875,
                    name: 'Viewing SLA Information',
                    local: 'activedirectory_000502.html'
                  },
                  {
                    id: 1911,
                    parentId: 1875,
                    name: 'Managing SLAs',
                    local: 'activedirectory_00053.html'
                  }
                ]
              },
              {
                id: 1876,
                parentId: 1827,
                name: 'Copies',
                local: 'activedirectory_00054.html',
                children: [
                  {
                    id: 1912,
                    parentId: 1876,
                    name: 'Viewing Active Directory Copy Information',
                    local: 'activedirectory_00055.html'
                  },
                  {
                    id: 1913,
                    parentId: 1876,
                    name: 'Managing Active Directory Copies',
                    local: 'activedirectory_00056.html'
                  }
                ]
              },
              {
                id: 1877,
                parentId: 1827,
                name: 'Active Directory Domain Controller',
                local: 'activedirectory_00057.html',
                children: [
                  {
                    id: 1914,
                    parentId: 1877,
                    name:
                      'Viewing Information of Active Directory Domain Controllers',
                    local: 'activedirectory_00058.html'
                  },
                  {
                    id: 1915,
                    parentId: 1877,
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
            id: 1916,
            parentId: 19,
            name: 'NAS File Data Protection',
            local: 'product_documentation_000035.html',
            children: [
              {
                id: 1922,
                parentId: 1916,
                name: 'Backup',
                local: 'nas_s_0007.html',
                children: [
                  {
                    id: 1933,
                    parentId: 1922,
                    name: 'Preparations for Backup',
                    local: 'nas_s_0010.html'
                  },
                  {
                    id: 1934,
                    parentId: 1922,
                    name: 'Backing Up a NAS File System',
                    local: 'nas_s_0011.html',
                    children: [
                      {
                        id: 1936,
                        parentId: 1934,
                        name:
                          'Step 1: Obtaining the CA Certificate of a Storage Device',
                        local: 'nas_s_0012.html'
                      },
                      {
                        id: 1937,
                        parentId: 1934,
                        name: 'Step 2: Adding a Storage Device',
                        local: 'nas_s_0013.html'
                      },
                      {
                        id: 1938,
                        parentId: 1934,
                        name:
                          'Step 3: Creating a Logical Port for the Replication Network',
                        local: 'nas_s_0014.html'
                      },
                      {
                        id: 1939,
                        parentId: 1934,
                        name:
                          '(Optional) Step 4: Creating a Rate Limiting Policy',
                        local: 'nas_s_0015.html'
                      },
                      {
                        id: 1940,
                        parentId: 1934,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'nas_s_0016.html'
                      },
                      {
                        id: 1941,
                        parentId: 1934,
                        name: 'Step 6: Performing Backup',
                        local: 'nas_s_0017.html'
                      }
                    ]
                  },
                  {
                    id: 1935,
                    parentId: 1922,
                    name: 'Backing Up a NAS Share',
                    local: 'nas_s_0020.html',
                    children: [
                      {
                        id: 1942,
                        parentId: 1935,
                        name:
                          'Step 1: (Optional) Obtaining the CA Certificate of the Storage Device (Applicable to OceanStor V5/OceanStor Pacific/NetApp ONTAP)',
                        local: 'nas_s_0021.html'
                      },
                      {
                        id: 1943,
                        parentId: 1935,
                        name:
                          'Step 2: Adding a Storage Device (Applicable to OceanStor V5/OceanStor Pacific/NetApp ONTAP)',
                        local: 'nas_s_0022.html'
                      },
                      {
                        id: 1944,
                        parentId: 1935,
                        name:
                          'Step 3: Configuring NAS Share Information (Applicable to OceanStor V5/OceanStor Pacific/NetApp ONTAP)',
                        local: 'nas_s_0023.html'
                      },
                      {
                        id: 1945,
                        parentId: 1935,
                        name:
                          'Step 4: Configuring Access Permissions (Applicable to OceanStor V5/OceanStor Pacific/NetApp ONTAP)',
                        local: 'nas_s_0025.html'
                      },
                      {
                        id: 1946,
                        parentId: 1935,
                        name:
                          'Step 5: Registering a NAS Share (Applicable to Storage Devices Except OceanStor V5/OceanStor Pacific/NetApp ONTAP)',
                        local: 'nas_s_0024.html'
                      },
                      {
                        id: 1947,
                        parentId: 1935,
                        name:
                          '(Optional) Step 6: Creating a Rate Limiting Policy',
                        local: 'nas_s_0026.html'
                      },
                      {
                        id: 1948,
                        parentId: 1935,
                        name: 'Step 7: Creating a Backup SLA',
                        local: 'nas_s_0027.html'
                      },
                      {
                        id: 1949,
                        parentId: 1935,
                        name:
                          'Step 8: Enabling the NFSv4.1 Service (Applicable to Some Models)',
                        local: 'nas_s_0028.html'
                      },
                      {
                        id: 1950,
                        parentId: 1935,
                        name: 'Step 9: Performing Backup',
                        local: 'nas_s_0029.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1923,
                parentId: 1916,
                name: 'Replication',
                local: 'nas_s_0032.html',
                children: [
                  {
                    id: 1951,
                    parentId: 1923,
                    name: 'Replicating NAS File System or NAS Share Copies',
                    local: 'nas_s_0035.html',
                    children: [
                      {
                        id: 1952,
                        parentId: 1951,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'nas_s_0037.html'
                      },
                      {
                        id: 1953,
                        parentId: 1951,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'nas_s_0037_1.html'
                      },
                      {
                        id: 1954,
                        parentId: 1951,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'nas_s_0038.html'
                      },
                      {
                        id: 1955,
                        parentId: 1951,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'nas_s_0039.html'
                      },
                      {
                        id: 1956,
                        parentId: 1951,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'nas_s_0040.html'
                      },
                      {
                        id: 1957,
                        parentId: 1951,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'nas_s_0041.html'
                      },
                      {
                        id: 1958,
                        parentId: 1951,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002022805444.html'
                      },
                      {
                        id: 1959,
                        parentId: 1951,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'nas_s_0042.html'
                      },
                      {
                        id: 1960,
                        parentId: 1951,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'nas_s_0043.html'
                      },
                      {
                        id: 1961,
                        parentId: 1951,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA',
                        local: 'nas_s_0044.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1924,
                parentId: 1916,
                name: 'Archiving',
                local: 'nas_s_0045.html',
                children: [
                  {
                    id: 1962,
                    parentId: 1924,
                    name:
                      'Archiving Backup Copies of a NAS File System or NAS Share',
                    local: 'nas_s_0048.html',
                    children: [
                      {
                        id: 1964,
                        parentId: 1962,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'nas_s_0049.html',
                        children: [
                          {
                            id: 1966,
                            parentId: 1964,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'nas_s_0050.html'
                          },
                          {
                            id: 1967,
                            parentId: 1964,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'nas_s_0051.html'
                          }
                        ]
                      },
                      {
                        id: 1965,
                        parentId: 1962,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'nas_s_0052.html'
                      }
                    ]
                  },
                  {
                    id: 1963,
                    parentId: 1924,
                    name:
                      'Archiving Replication Copies of a NAS File System or NAS Share',
                    local: 'nas_s_0053.html',
                    children: [
                      {
                        id: 1968,
                        parentId: 1963,
                        name:
                          'Step 1: Creating an Archive SLA for Replication Copies',
                        local: 'nas_s_0054.html'
                      },
                      {
                        id: 1969,
                        parentId: 1963,
                        name:
                          'Step 2: Creating Periodic Archiving for Replication Copies',
                        local: 'nas_s_0055.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1925,
                parentId: 1916,
                name: 'Restoration',
                local: 'nas_s_0056.html',
                children: [
                  {
                    id: 1970,
                    parentId: 1925,
                    name: 'Restoring a NAS File System',
                    local: 'nas_s_0059.html'
                  },
                  {
                    id: 1971,
                    parentId: 1925,
                    name: 'Restoring Files in a NAS File System',
                    local: 'nas_s_0060.html'
                  },
                  {
                    id: 1972,
                    parentId: 1925,
                    name: 'Restoring a NAS Share',
                    local: 'nas_s_0061.html'
                  },
                  {
                    id: 1973,
                    parentId: 1925,
                    name: 'Restoring Files in a NAS Share',
                    local: 'nas_s_0062.html'
                  }
                ]
              },
              {
                id: 1926,
                parentId: 1916,
                name: 'Global Search',
                local: 'nas_s_0073.html',
                children: [
                  {
                    id: 1974,
                    parentId: 1926,
                    name: 'Global Search for Copies',
                    local: 'nas_s_0074.html'
                  },
                  {
                    id: 1975,
                    parentId: 1926,
                    name: 'Global Search for Resources',
                    local: 'nas_s_0075.html'
                  },
                  {
                    id: 1976,
                    parentId: 1926,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002022936884.html'
                  }
                ]
              },
              {
                id: 1927,
                parentId: 1916,
                name: 'SLA ',
                local: 'nas_s_0078.html',
                children: [
                  {
                    id: 1977,
                    parentId: 1927,
                    name: 'About SLA',
                    local: 'nas_s_0079.html'
                  },
                  {
                    id: 1978,
                    parentId: 1927,
                    name: 'Viewing SLA Information',
                    local: 'nas_s_0080.html'
                  },
                  {
                    id: 1979,
                    parentId: 1927,
                    name: 'Managing SLAs',
                    local: 'nas_s_0081.html'
                  }
                ]
              },
              {
                id: 1928,
                parentId: 1916,
                name: 'Copies',
                local: 'nas_s_0082.html',
                children: [
                  {
                    id: 1980,
                    parentId: 1928,
                    name: 'Viewing Copy Information About a NAS File System',
                    local: 'nas_s_0083.html'
                  },
                  {
                    id: 1981,
                    parentId: 1928,
                    name: 'Managing Copies of a NAS File System',
                    local: 'nas_s_0084.html'
                  },
                  {
                    id: 1982,
                    parentId: 1928,
                    name: 'Viewing NAS Share Copy Information',
                    local: 'nas_s_0086.html'
                  },
                  {
                    id: 1983,
                    parentId: 1928,
                    name: 'Managing NAS Share Copies',
                    local: 'nas_s_0087.html'
                  }
                ]
              },
              {
                id: 1929,
                parentId: 1916,
                name: 'Storage Device Information',
                local: 'nas_s_0088.html',
                children: [
                  {
                    id: 1984,
                    parentId: 1929,
                    name: 'Viewing Storage Device Information',
                    local: 'nas_s_0089.html'
                  },
                  {
                    id: 1985,
                    parentId: 1929,
                    name: 'Managing Storage Device Information',
                    local: 'nas_s_0090.html'
                  }
                ]
              },
              {
                id: 1930,
                parentId: 1916,
                name: 'NAS File Systems',
                local: 'nas_s_0091.html',
                children: [
                  {
                    id: 1986,
                    parentId: 1930,
                    name: 'Viewing NAS File Systems',
                    local: 'nas_s_0092.html'
                  },
                  {
                    id: 1987,
                    parentId: 1930,
                    name: 'Managing NAS File Systems',
                    local: 'nas_s_0093.html'
                  }
                ]
              },
              {
                id: 1931,
                parentId: 1916,
                name: 'NAS Shares',
                local: 'nas_s_0094.html',
                children: [
                  {
                    id: 1988,
                    parentId: 1931,
                    name: 'Viewing NAS Share Information',
                    local: 'nas_s_0095.html'
                  },
                  {
                    id: 1989,
                    parentId: 1931,
                    name: 'Managing NAS Shares',
                    local: 'nas_s_0096.html'
                  }
                ]
              },
              {
                id: 1932,
                parentId: 1916,
                name: 'FAQs',
                local: 'nas_s_0097.html'
              }
            ]
          },
          {
            id: 1917,
            parentId: 19,
            name:
              'NDMP NAS File System Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001952305809.html',
            children: [
              {
                id: 1990,
                parentId: 1917,
                name: 'Backup',
                local: 'nas_s_0007_0.html',
                children: [
                  {
                    id: 2000,
                    parentId: 1990,
                    name: 'Preparations for Backup',
                    local: 'nas_s_0010_0.html'
                  },
                  {
                    id: 2001,
                    parentId: 1990,
                    name: 'Backing Up an NDMP NAS File System',
                    local: 'nas_s_0011_0.html',
                    children: [
                      {
                        id: 2002,
                        parentId: 2001,
                        name: 'Step 1: Adding a Storage Device',
                        local: 'nas_s_0013_0.html'
                      },
                      {
                        id: 2003,
                        parentId: 2001,
                        name:
                          'Step 2: (Optional) Creating a Rate Limiting Policy',
                        local: 'nas_s_0015_0.html'
                      },
                      {
                        id: 2004,
                        parentId: 2001,
                        name: 'Step 3: Creating a Backup SLA',
                        local: 'nas_s_0016_0.html'
                      },
                      {
                        id: 2005,
                        parentId: 2001,
                        name: 'Step 4: Performing Backup',
                        local: 'nas_s_0017_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1991,
                parentId: 1917,
                name: 'Replication',
                local: 'nas_s_0032_0.html',
                children: [
                  {
                    id: 2006,
                    parentId: 1991,
                    name: 'Replicating an NDMP NAS File System',
                    local: 'nas_s_0035_0.html',
                    children: [
                      {
                        id: 2007,
                        parentId: 2006,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'nas_s_0037_1_0.html'
                      },
                      {
                        id: 2008,
                        parentId: 2006,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'nas_s_0038_0.html'
                      },
                      {
                        id: 2009,
                        parentId: 2006,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'nas_s_0039_0.html'
                      },
                      {
                        id: 2010,
                        parentId: 2006,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'nas_s_0040_0.html'
                      },
                      {
                        id: 2011,
                        parentId: 2006,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002046636865.html'
                      },
                      {
                        id: 2012,
                        parentId: 2006,
                        name: 'Step 6: Adding a Target Cluster',
                        local: 'foc_gud_0032.html'
                      },
                      {
                        id: 2013,
                        parentId: 2006,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'nas_s_0043_0.html'
                      },
                      {
                        id: 2014,
                        parentId: 2006,
                        name:
                          'Step 8: (Optional) Creating a Reverse Replication/Cascaded Replication SLA',
                        local: 'nas_s_0044_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1992,
                parentId: 1917,
                name: 'Archiving',
                local: 'nas_s_0045_0.html',
                children: [
                  {
                    id: 2015,
                    parentId: 1992,
                    name: 'Archiving Copies of an NDMP NAS File System',
                    local: 'nas_s_0048_0.html',
                    children: [
                      {
                        id: 2017,
                        parentId: 2015,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'nas_s_0049_0.html',
                        children: [
                          {
                            id: 2019,
                            parentId: 2017,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'nas_s_0050_0.html'
                          },
                          {
                            id: 2020,
                            parentId: 2017,
                            name:
                              'Creating a Media Set (When the Archive Storage Is a Tape Library)',
                            local: 'nas_s_0051_0.html'
                          }
                        ]
                      },
                      {
                        id: 2018,
                        parentId: 2015,
                        name:
                          'Step 2: Creating an Archive SLA for Backup Copies',
                        local: 'oracle_gud_000031.html'
                      }
                    ]
                  },
                  {
                    id: 2016,
                    parentId: 1992,
                    name:
                      'Archiving Replication Copies of an NDMP NAS File System',
                    local: 'nas_s_0053_0.html',
                    children: [
                      {
                        id: 2021,
                        parentId: 2016,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'nas_s_0054_0.html'
                      },
                      {
                        id: 2022,
                        parentId: 2016,
                        name:
                          'Step 2: Creating Periodic Archiving for Replication Copies',
                        local: 'nas_s_0055_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 1993,
                parentId: 1917,
                name: 'Restoration',
                local: 'nas_s_0056_0.html',
                children: [
                  {
                    id: 2023,
                    parentId: 1993,
                    name: 'Restoring an NDMP NAS File System',
                    local: 'nas_s_0059_0.html'
                  },
                  {
                    id: 2024,
                    parentId: 1993,
                    name: 'Restoring Files in an NDMP NAS File System',
                    local: 'nas_s_0060_0.html'
                  }
                ]
              },
              {
                id: 1994,
                parentId: 1917,
                name: 'Global Search',
                local: 'nas_s_0073_0.html',
                children: [
                  {
                    id: 2025,
                    parentId: 1994,
                    name: 'Global Search for Copies',
                    local: 'nas_s_0074_0.html'
                  },
                  {
                    id: 2026,
                    parentId: 1994,
                    name: 'Global Search for Resources',
                    local: 'nas_s_0075_0.html'
                  },
                  {
                    id: 2027,
                    parentId: 1994,
                    name: 'Global Search for Tags',
                    local: 'en-us_topic_0000002038775953.html'
                  }
                ]
              },
              {
                id: 1995,
                parentId: 1917,
                name: 'SLA ',
                local: 'nas_s_0078_0.html',
                children: [
                  {
                    id: 2028,
                    parentId: 1995,
                    name: 'About SLA',
                    local: 'nas_s_0079_0.html'
                  },
                  {
                    id: 2029,
                    parentId: 1995,
                    name: 'Viewing SLA Information',
                    local: 'nas_s_0080_0.html'
                  },
                  {
                    id: 2030,
                    parentId: 1995,
                    name: 'Managing SLAs',
                    local: 'nas_s_0081_0.html'
                  }
                ]
              },
              {
                id: 1996,
                parentId: 1917,
                name: 'Copies',
                local: 'nas_s_0082_0.html',
                children: [
                  {
                    id: 2031,
                    parentId: 1996,
                    name:
                      'Viewing Copy Information About an NDMP NAS File System',
                    local: 'nas_s_0083_0.html'
                  },
                  {
                    id: 2032,
                    parentId: 1996,
                    name: 'Managing Copies of an NDMP NAS File System',
                    local: 'nas_s_0084_0.html'
                  }
                ]
              },
              {
                id: 1997,
                parentId: 1917,
                name: 'Storage Device Information',
                local: 'nas_s_0088_0.html',
                children: [
                  {
                    id: 2033,
                    parentId: 1997,
                    name: 'Viewing Storage Device Information',
                    local: 'nas_s_0089_0.html'
                  },
                  {
                    id: 2034,
                    parentId: 1997,
                    name: 'Managing Storage Device Information',
                    local: 'nas_s_0090_0.html'
                  }
                ]
              },
              {
                id: 1998,
                parentId: 1917,
                name: 'NDMP NAS File Systems',
                local: 'nas_s_0091_0.html',
                children: [
                  {
                    id: 2035,
                    parentId: 1998,
                    name: 'Viewing NDMP NAS File Systems',
                    local: 'nas_s_0092_0.html'
                  },
                  {
                    id: 2036,
                    parentId: 1998,
                    name: 'Managing an NDMP NAS File System',
                    local: 'nas_s_0093_0.html'
                  }
                ]
              },
              {
                id: 1999,
                parentId: 1917,
                name: 'FAQs',
                local: 'nas_s_0097_0.html',
                children: [
                  {
                    id: 2037,
                    parentId: 1999,
                    name:
                      'Some of the Concurrent Jobs on the Same NDMP Server Failed',
                    local: 'en-us_topic_0000002017987564.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1918,
            parentId: 19,
            name: 'Fileset Data Protection',
            local: 'en-us_topic_0000001873679157.html',
            children: [
              {
                id: 2038,
                parentId: 1918,
                name: 'Backup',
                local: 'Files-0005_0.html',
                children: [
                  {
                    id: 2046,
                    parentId: 2038,
                    name: 'Mounting Object Storage to the ProtectAgent Host',
                    local: 'object-0006_0.html'
                  },
                  {
                    id: 2047,
                    parentId: 2038,
                    name: 'Backing up a Fileset',
                    local: 'Files-0008_0.html',
                    children: [
                      {
                        id: 2048,
                        parentId: 2047,
                        name: 'Step 1: (Optional) Creating a Fileset Template',
                        local: 'Files-0009_0.html'
                      },
                      {
                        id: 2049,
                        parentId: 2047,
                        name: 'Step 2: Creating a Fileset',
                        local: 'Files-0010.html'
                      },
                      {
                        id: 2050,
                        parentId: 2047,
                        name:
                          'Step 3: (Optional) Creating a Rate Limiting Policy',
                        local: 'Files-0011_0.html'
                      },
                      {
                        id: 2051,
                        parentId: 2047,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'Files-0012.html'
                      },
                      {
                        id: 2052,
                        parentId: 2047,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'Files-0013_0.html'
                      },
                      {
                        id: 2053,
                        parentId: 2047,
                        name: 'Step 6: Performing Backup',
                        local: 'Files-0014_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2039,
                parentId: 1918,
                name: 'Replication',
                local: 'oracle_gud_000035_14.html',
                children: [
                  {
                    id: 2054,
                    parentId: 2039,
                    name: 'Replicating Fileset Copies',
                    local: 'Files-0020_0.html',
                    children: [
                      {
                        id: 2055,
                        parentId: 2054,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_14.html'
                      },
                      {
                        id: 2056,
                        parentId: 2054,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_22.html'
                      },
                      {
                        id: 2057,
                        parentId: 2054,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'Files-0023_0.html'
                      },
                      {
                        id: 2058,
                        parentId: 2054,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'Files-0024_0.html'
                      },
                      {
                        id: 2059,
                        parentId: 2054,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'Files-0025_0.html'
                      },
                      {
                        id: 2060,
                        parentId: 2054,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'Files-0026_0.html'
                      },
                      {
                        id: 2061,
                        parentId: 2054,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002010555368.html'
                      },
                      {
                        id: 2062,
                        parentId: 2054,
                        name: 'Step: Adding a Target Cluster',
                        local: 'Files-0027_0.html'
                      },
                      {
                        id: 2063,
                        parentId: 2054,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'Files-0028_0.html'
                      },
                      {
                        id: 2064,
                        parentId: 2054,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA ',
                        local: 'Files-0029_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2040,
                parentId: 1918,
                name: 'Archiving',
                local: 'Files-0030.html',
                children: [
                  {
                    id: 2065,
                    parentId: 2040,
                    name: 'Archiving Fileset Backup Copies',
                    local: 'Files-0033_0.html',
                    children: [
                      {
                        id: 2067,
                        parentId: 2065,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'Files-0034_0.html',
                        children: [
                          {
                            id: 2069,
                            parentId: 2067,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'Files-0035_0.html'
                          },
                          {
                            id: 2070,
                            parentId: 2067,
                            name:
                              'Creating a Media Set (When Archive Storage Is a Tape Library)',
                            local: 'Files-0036_0.html'
                          }
                        ]
                      },
                      {
                        id: 2068,
                        parentId: 2065,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'Files-0037_0.html'
                      }
                    ]
                  },
                  {
                    id: 2066,
                    parentId: 2040,
                    name: 'Archiving Fileset Replication Copies ',
                    local: 'Files-0038_0.html',
                    children: [
                      {
                        id: 2071,
                        parentId: 2066,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'Files-0039_0.html'
                      },
                      {
                        id: 2072,
                        parentId: 2066,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'Files-0040_0.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2041,
                parentId: 1918,
                name: 'Restoration',
                local: 'Files-0041_0.html',
                children: [
                  {
                    id: 2073,
                    parentId: 2041,
                    name: 'Restoring a Fileset',
                    local: 'Files-0044.html'
                  },
                  {
                    id: 2074,
                    parentId: 2041,
                    name: 'Restoring One or Multiple Files in a Fileset',
                    local: 'Files-0045.html'
                  }
                ]
              },
              {
                id: 2042,
                parentId: 1918,
                name: 'Global Search',
                local: 'Files-0056_0.html',
                children: [
                  {
                    id: 2075,
                    parentId: 2042,
                    name: 'Global Search for Copies',
                    local: 'Files-0057_0.html'
                  },
                  {
                    id: 2076,
                    parentId: 2042,
                    name: 'Global Search for Resources',
                    local: 'Files-0058_0.html'
                  },
                  {
                    id: 2077,
                    parentId: 2042,
                    name:
                      'Tag-based Global Search (Applicable to 1.6.0 and Later Versions)',
                    local: 'en-us_topic_0000002002699236.html'
                  }
                ]
              },
              {
                id: 2043,
                parentId: 1918,
                name: 'SLA ',
                local: 'Files-0061.html',
                children: [
                  {
                    id: 2078,
                    parentId: 2043,
                    name: 'Viewing SLA Information',
                    local: 'Files-0063_0.html'
                  },
                  {
                    id: 2079,
                    parentId: 2043,
                    name: 'Managing SLAs',
                    local: 'Files-0064_0.html'
                  }
                ]
              },
              {
                id: 2044,
                parentId: 1918,
                name: 'Copies',
                local: 'Files-0065_0.html',
                children: [
                  {
                    id: 2080,
                    parentId: 2044,
                    name: 'Viewing Fileset Copy Information',
                    local: 'Files-0066_0.html'
                  },
                  {
                    id: 2081,
                    parentId: 2044,
                    name: 'Managing Fileset Copies',
                    local: 'Files-0067_0.html'
                  }
                ]
              },
              {
                id: 2045,
                parentId: 1918,
                name: 'Filesets',
                local: 'Files-0068_0.html',
                children: [
                  {
                    id: 2082,
                    parentId: 2045,
                    name: 'Viewing Fileset Information',
                    local: 'Files-0069.html'
                  },
                  {
                    id: 2083,
                    parentId: 2045,
                    name: 'Managing Filesets',
                    local: 'Files-0070_0.html'
                  },
                  {
                    id: 2084,
                    parentId: 2045,
                    name: 'Managing Fileset Templates',
                    local: 'Files-0071_0.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1919,
            parentId: 19,
            name:
              'Volume Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001826879836.html',
            children: [
              {
                id: 2085,
                parentId: 1919,
                name: 'Backup',
                local: 'volume-0007.html',
                children: [
                  {
                    id: 2093,
                    parentId: 2085,
                    name: 'Backing Up Volumes',
                    local: 'volume-0010.html',
                    children: [
                      {
                        id: 2094,
                        parentId: 2093,
                        name: 'Step 1: Creating a Volume',
                        local: 'volume-0011.html'
                      },
                      {
                        id: 2095,
                        parentId: 2093,
                        name: 'Step 2: Creating a Rate Limiting Policy',
                        local: 'volume-0012.html'
                      },
                      {
                        id: 2096,
                        parentId: 2093,
                        name:
                          'Step 3: (Optional) Enabling Backup Link Encryption',
                        local: 'volume-0013.html'
                      },
                      {
                        id: 2097,
                        parentId: 2093,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'volume-0014.html'
                      },
                      {
                        id: 2098,
                        parentId: 2093,
                        name: 'Step 5: Performing Backup',
                        local: 'volume-0015.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2086,
                parentId: 1919,
                name: 'Replication',
                local: 'oracle_gud_000035_12.html',
                children: [
                  {
                    id: 2099,
                    parentId: 2086,
                    name: 'Replicating Volume Copies',
                    local: 'volume-0021.html',
                    children: [
                      {
                        id: 2100,
                        parentId: 2099,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_12.html'
                      },
                      {
                        id: 2101,
                        parentId: 2099,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_19.html'
                      },
                      {
                        id: 2102,
                        parentId: 2099,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'volume-0024.html'
                      },
                      {
                        id: 2103,
                        parentId: 2099,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'volume-0025.html'
                      },
                      {
                        id: 2104,
                        parentId: 2099,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'volume-0026.html'
                      },
                      {
                        id: 2105,
                        parentId: 2099,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'volume-0027.html'
                      },
                      {
                        id: 2106,
                        parentId: 2099,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002010594166.html'
                      },
                      {
                        id: 2107,
                        parentId: 2099,
                        name: 'Step: Adding a Target Cluster',
                        local: 'volume-0028.html'
                      },
                      {
                        id: 2108,
                        parentId: 2099,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'volume-0029.html'
                      },
                      {
                        id: 2109,
                        parentId: 2099,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA ',
                        local: 'volume-0030.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2087,
                parentId: 1919,
                name: 'Archiving',
                local: 'volume-0031.html',
                children: [
                  {
                    id: 2110,
                    parentId: 2087,
                    name: 'Archiving Volume Backup Copies',
                    local: 'volume-0034.html',
                    children: [
                      {
                        id: 2112,
                        parentId: 2110,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'volume-0035.html',
                        children: [
                          {
                            id: 2114,
                            parentId: 2112,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'volume-0036.html'
                          },
                          {
                            id: 2115,
                            parentId: 2112,
                            name:
                              'Creating a Media Set (When Archive Storage Is a Tape Library)',
                            local: 'volume-0037.html'
                          }
                        ]
                      },
                      {
                        id: 2113,
                        parentId: 2110,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'volume-0038.html'
                      }
                    ]
                  },
                  {
                    id: 2111,
                    parentId: 2087,
                    name: 'Archiving Volume Replication Copies',
                    local: 'volume-0039.html',
                    children: [
                      {
                        id: 2116,
                        parentId: 2111,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'volume-0040.html'
                      },
                      {
                        id: 2117,
                        parentId: 2111,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'volume-0041.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2088,
                parentId: 1919,
                name: 'Restoration',
                local: 'volume-0042.html',
                children: [
                  {
                    id: 2118,
                    parentId: 2088,
                    name: 'Restoring Volumes',
                    local: 'volume-0045.html'
                  },
                  {
                    id: 2119,
                    parentId: 2088,
                    name: 'Restoring One or Multiple Files in a Volume Copy',
                    local: 'volume-0046.html'
                  }
                ]
              },
              {
                id: 2089,
                parentId: 1919,
                name: 'Global Search',
                local: 'volume-0056.html',
                children: [
                  {
                    id: 2120,
                    parentId: 2089,
                    name: 'Global Search for Copies',
                    local: 'volume-0057.html'
                  },
                  {
                    id: 2121,
                    parentId: 2089,
                    name: 'Global Search for Resources',
                    local: 'volume-0058.html'
                  },
                  {
                    id: 2122,
                    parentId: 2089,
                    name: 'Tag-based Global Search',
                    local: 'en-us_topic_0000002002855252.html'
                  }
                ]
              },
              {
                id: 2090,
                parentId: 1919,
                name: 'SLA ',
                local: 'volume-0061.html',
                children: [
                  {
                    id: 2123,
                    parentId: 2090,
                    name: 'About SLA',
                    local: 'vmware_gud_000026_1.html'
                  },
                  {
                    id: 2124,
                    parentId: 2090,
                    name: 'Viewing SLA Information',
                    local: 'volume-0063.html'
                  },
                  {
                    id: 2125,
                    parentId: 2090,
                    name: 'Managing SLAs',
                    local: 'volume-0064.html'
                  }
                ]
              },
              {
                id: 2091,
                parentId: 1919,
                name: 'Copies',
                local: 'volume-0065.html',
                children: [
                  {
                    id: 2126,
                    parentId: 2091,
                    name: 'Viewing Volume Copy Information',
                    local: 'volume-0066.html'
                  },
                  {
                    id: 2127,
                    parentId: 2091,
                    name: 'Managing Volume Copies',
                    local: 'volume-0067.html'
                  }
                ]
              },
              {
                id: 2092,
                parentId: 1919,
                name: 'Volumes',
                local: 'volume-0068.html',
                children: [
                  {
                    id: 2128,
                    parentId: 2092,
                    name: 'Viewing Volume Information',
                    local: 'volume-0069.html'
                  },
                  {
                    id: 2129,
                    parentId: 2092,
                    name: 'Managing Volumes',
                    local: 'volume-0070.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1920,
            parentId: 19,
            name:
              'Object Storage Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001826879816.html',
            children: [
              {
                id: 2130,
                parentId: 1920,
                name: 'Backup',
                local: 'object-0007.html',
                children: [
                  {
                    id: 2138,
                    parentId: 2130,
                    name: 'Preparing for the Backup',
                    local: 'en-us_topic_0000001953327945.html',
                    children: [
                      {
                        id: 2140,
                        parentId: 2138,
                        name: 'Obtaining the Endpoint at the Production End',
                        local: 'en-us_topic_0000001926224624.html'
                      },
                      {
                        id: 2141,
                        parentId: 2138,
                        name: 'Obtaining the AK and SK at the Production End',
                        local: 'en-us_topic_0000001953344001.html'
                      }
                    ]
                  },
                  {
                    id: 2139,
                    parentId: 2130,
                    name: 'Backing Up Object Storage',
                    local: 'object-0010.html',
                    children: [
                      {
                        id: 2142,
                        parentId: 2139,
                        name: 'Step 1: Registering Object Storage',
                        local: 'object-0011.html'
                      },
                      {
                        id: 2143,
                        parentId: 2139,
                        name: 'Step 2: Creating an Object Set',
                        local: 'object-0012.html'
                      },
                      {
                        id: 2144,
                        parentId: 2139,
                        name:
                          'Step 3: (Optional) Creating a Rate Limiting Policy',
                        local: 'object-0013.html'
                      },
                      {
                        id: 2145,
                        parentId: 2139,
                        name:
                          'Step 4: (Optional) Enabling Backup Link Encryption',
                        local: 'object-0014.html'
                      },
                      {
                        id: 2146,
                        parentId: 2139,
                        name: 'Step 5: Creating a Backup SLA',
                        local: 'object-0015.html'
                      },
                      {
                        id: 2147,
                        parentId: 2139,
                        name: 'Step 7: Performing Backup',
                        local: 'object-0017.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2131,
                parentId: 1920,
                name: 'Replication',
                local: 'oracle_gud_000035_10.html',
                children: [
                  {
                    id: 2148,
                    parentId: 2131,
                    name: 'Replicating an Object Storage Copy',
                    local: 'object-0023.html',
                    children: [
                      {
                        id: 2149,
                        parentId: 2148,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_10.html'
                      },
                      {
                        id: 2150,
                        parentId: 2148,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_15.html'
                      },
                      {
                        id: 2151,
                        parentId: 2148,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'object-0026.html'
                      },
                      {
                        id: 2152,
                        parentId: 2148,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'object-0027.html'
                      },
                      {
                        id: 2153,
                        parentId: 2148,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'object-0028.html'
                      },
                      {
                        id: 2154,
                        parentId: 2148,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.5.0)',
                        local: 'fc_gud_0030.html'
                      },
                      {
                        id: 2155,
                        parentId: 2148,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002046625105.html'
                      },
                      {
                        id: 2156,
                        parentId: 2148,
                        name: 'Step: Adding a Target Cluster',
                        local: 'object-0030.html'
                      },
                      {
                        id: 2157,
                        parentId: 2148,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'object-0031.html'
                      },
                      {
                        id: 2158,
                        parentId: 2148,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA ',
                        local: 'object-0032.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2132,
                parentId: 1920,
                name: 'Archiving',
                local: 'object-0033.html',
                children: [
                  {
                    id: 2159,
                    parentId: 2132,
                    name: 'Archiving Object Set Backup Copies',
                    local: 'object-0036.html',
                    children: [
                      {
                        id: 2161,
                        parentId: 2159,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'object-0037.html',
                        children: [
                          {
                            id: 2163,
                            parentId: 2161,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'object-0038.html'
                          },
                          {
                            id: 2164,
                            parentId: 2161,
                            name:
                              'Creating a Media Set (When Archive Storage Is a Tape Library)',
                            local: 'object-0039.html'
                          }
                        ]
                      },
                      {
                        id: 2162,
                        parentId: 2159,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'object-0040.html'
                      }
                    ]
                  },
                  {
                    id: 2160,
                    parentId: 2132,
                    name: 'Archiving Object Set Replication Copies',
                    local: 'object-0041.html',
                    children: [
                      {
                        id: 2165,
                        parentId: 2160,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'object-0042.html'
                      },
                      {
                        id: 2166,
                        parentId: 2160,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'object-0043.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2133,
                parentId: 1920,
                name: 'Restoration',
                local: 'object-0044.html',
                children: [
                  {
                    id: 2167,
                    parentId: 2133,
                    name: 'Restoring Object Storage',
                    local: 'object-0047.html'
                  }
                ]
              },
              {
                id: 2134,
                parentId: 1920,
                name: 'Global Search',
                local: 'object-0048.html',
                children: [
                  {
                    id: 2168,
                    parentId: 2134,
                    name: 'Global Search for Copies',
                    local: 'object-0049.html'
                  },
                  {
                    id: 2169,
                    parentId: 2134,
                    name: 'Global Search for Resources',
                    local: 'object-0050.html'
                  },
                  {
                    id: 2170,
                    parentId: 2134,
                    name: 'Tag-based Global Search',
                    local: 'en-us_topic_0000002002688946.html'
                  }
                ]
              },
              {
                id: 2135,
                parentId: 1920,
                name: 'SLA ',
                local: 'object-0053.html',
                children: [
                  {
                    id: 2171,
                    parentId: 2135,
                    name: 'About SLA',
                    local: 'object-0054.html'
                  },
                  {
                    id: 2172,
                    parentId: 2135,
                    name: 'Viewing SLA Information',
                    local: 'object-0055.html'
                  },
                  {
                    id: 2173,
                    parentId: 2135,
                    name: 'Managing SLAs',
                    local: 'object-0056.html'
                  }
                ]
              },
              {
                id: 2136,
                parentId: 1920,
                name: 'Copies',
                local: 'object-0057.html',
                children: [
                  {
                    id: 2174,
                    parentId: 2136,
                    name: 'Viewing Object Storage Copy Information',
                    local: 'object-0058.html'
                  },
                  {
                    id: 2175,
                    parentId: 2136,
                    name: 'Managing Object Storage Copies',
                    local: 'object-0059.html'
                  }
                ]
              },
              {
                id: 2137,
                parentId: 1920,
                name: 'Object Storage',
                local: 'object-0060.html',
                children: [
                  {
                    id: 2176,
                    parentId: 2137,
                    name: 'Viewing Object Storage Information',
                    local: 'object-0061.html'
                  },
                  {
                    id: 2177,
                    parentId: 2137,
                    name: 'Managing Object Sets',
                    local: 'object-0062.html'
                  }
                ]
              }
            ]
          },
          {
            id: 1921,
            parentId: 19,
            name:
              'Common Share Data Protection (Applicable to 1.6.0 and Later Versions)',
            local: 'en-us_topic_0000001873759353.html',
            children: [
              {
                id: 2178,
                parentId: 1921,
                name: 'Backup',
                local: 'Files-0005.html',
                children: [
                  {
                    id: 2185,
                    parentId: 2178,
                    name: 'Backing Up Common Share Resource Data',
                    local: 'Files-0008.html',
                    children: [
                      {
                        id: 2186,
                        parentId: 2185,
                        name: 'Step 1: Creating a Common Share',
                        local: 'Files-0009.html'
                      },
                      {
                        id: 2187,
                        parentId: 2185,
                        name: 'Step 2: Creating a Rate Limiting Policy',
                        local: 'Files-0011.html'
                      },
                      {
                        id: 2188,
                        parentId: 2185,
                        name:
                          '(Optional) Step 3: Enabling Backup Link Encryption',
                        local: 'commonshares_0013.html'
                      },
                      {
                        id: 2189,
                        parentId: 2185,
                        name: 'Step 4: Creating a Backup SLA',
                        local: 'Files-0013.html'
                      },
                      {
                        id: 2190,
                        parentId: 2185,
                        name: 'Step 5: Performing Backup',
                        local: 'Files-0014.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2179,
                parentId: 1921,
                name: 'Replication',
                local: 'oracle_gud_000035_4.html',
                children: [
                  {
                    id: 2191,
                    parentId: 2179,
                    name: 'Replicating a Common Share Copy',
                    local: 'Files-0019.html',
                    children: [
                      {
                        id: 2192,
                        parentId: 2191,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.5.0)',
                        local: 'fc_gud_0026_5.html'
                      },
                      {
                        id: 2193,
                        parentId: 2191,
                        name:
                          'Step 1: Creating a Logical Port for the Replication Network (Applicable to 1.6.0 and Later Versions)',
                        local: 'fc_gud_0026_1_7.html'
                      },
                      {
                        id: 2194,
                        parentId: 2191,
                        name: '(Optional) Step 2: Creating an IPsec Policy',
                        local: 'Files-0022.html'
                      },
                      {
                        id: 2195,
                        parentId: 2191,
                        name:
                          'Step 3: (Optional) Enabling Replication Link Encryption',
                        local: 'Files-0023.html'
                      },
                      {
                        id: 2196,
                        parentId: 2191,
                        name: 'Step 4: Downloading and Importing a Certificate',
                        local: 'Files-0024.html'
                      },
                      {
                        id: 2197,
                        parentId: 2191,
                        name: 'Step 5: Creating a Remote Device Administrator',
                        local: 'Files-0025.html'
                      },
                      {
                        id: 2198,
                        parentId: 2191,
                        name:
                          'Step 5: Creating a Remote Device Administrator (Applicable to 1.6.0 and Later Versions)',
                        local: 'en-us_topic_0000002010591334.html'
                      },
                      {
                        id: 2199,
                        parentId: 2191,
                        name: 'Step: Adding a Target Cluster',
                        local: 'Files-0026.html'
                      },
                      {
                        id: 2200,
                        parentId: 2191,
                        name: 'Step 7: Creating a Replication SLA',
                        local: 'Files-0027.html'
                      },
                      {
                        id: 2201,
                        parentId: 2191,
                        name:
                          '(Optional) Step 8: Creating a Reverse Replication or Cascading Replication SLA ',
                        local: 'Files-0028.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2180,
                parentId: 1921,
                name: 'Archiving',
                local: 'Files-0029.html',
                children: [
                  {
                    id: 2202,
                    parentId: 2180,
                    name: 'Archiving Backup Copies of a Common Share Resource',
                    local: 'Files-0032.html',
                    children: [
                      {
                        id: 2204,
                        parentId: 2202,
                        name: 'Step 1: Adding Archive Storage',
                        local: 'Files-0033.html',
                        children: [
                          {
                            id: 2206,
                            parentId: 2204,
                            name:
                              'Adding Object Storage (When the Archive Storage Is Object Storage)',
                            local: 'Files-0034.html'
                          },
                          {
                            id: 2207,
                            parentId: 2204,
                            name:
                              'Creating a Media Set (When Archive Storage Is a Tape Library)',
                            local: 'Files-0035.html'
                          }
                        ]
                      },
                      {
                        id: 2205,
                        parentId: 2202,
                        name: 'Step 2: Creating an Archive SLA',
                        local: 'Files-0036.html'
                      }
                    ]
                  },
                  {
                    id: 2203,
                    parentId: 2180,
                    name:
                      'Archiving Replication Copies of a Common Share Resource',
                    local: 'Files-0037.html',
                    children: [
                      {
                        id: 2208,
                        parentId: 2203,
                        name: 'Step 1: Creating an Archive SLA',
                        local: 'Files-0038.html'
                      },
                      {
                        id: 2209,
                        parentId: 2203,
                        name:
                          'Step 2: Creating a Periodic Archive Policy for Replication Copies',
                        local: 'Files-0039.html'
                      }
                    ]
                  }
                ]
              },
              {
                id: 2181,
                parentId: 1921,
                name: 'Managing Share Information',
                local: 'commonshares_0046.html',
                children: [
                  {
                    id: 2210,
                    parentId: 2181,
                    name: 'Configuring Share Information',
                    local: 'commonshares_0047.html'
                  },
                  {
                    id: 2211,
                    parentId: 2181,
                    name: 'Viewing Share Information',
                    local: 'commonshares_0048.html'
                  },
                  {
                    id: 2212,
                    parentId: 2181,
                    name: 'Deleting Share Information',
                    local: 'commonshares_0049.html'
                  }
                ]
              },
              {
                id: 2182,
                parentId: 1921,
                name: 'SLA ',
                local: 'en-us_topic_0000001927513108.html',
                children: [
                  {
                    id: 2213,
                    parentId: 2182,
                    name: 'About SLA',
                    local: 'vmware_gud_000026.html'
                  },
                  {
                    id: 2214,
                    parentId: 2182,
                    name: 'Viewing SLA Information',
                    local: 'en-us_topic_0000001954672445.html'
                  },
                  {
                    id: 2215,
                    parentId: 2182,
                    name: 'Managing SLAs',
                    local: 'en-us_topic_0000001927353788.html'
                  }
                ]
              },
              {
                id: 2183,
                parentId: 1921,
                name: 'Copies',
                local: 'Files-0063.html',
                children: [
                  {
                    id: 2216,
                    parentId: 2183,
                    name:
                      'Viewing Copy Information About a Common Share Resource',
                    local: 'Files-0064.html'
                  },
                  {
                    id: 2217,
                    parentId: 2183,
                    name: 'Managing a Common Share Resource Copy',
                    local: 'Files-0065.html'
                  }
                ]
              },
              {
                id: 2184,
                parentId: 1921,
                name: 'Common Share',
                local: 'Files-0066.html',
                children: [
                  {
                    id: 2218,
                    parentId: 2184,
                    name: 'Viewing Common Share Information',
                    local: 'Files-0067.html'
                  },
                  {
                    id: 2219,
                    parentId: 2184,
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
        id: 2220,
        parentId: 4,
        name: 'Restoration Drill',
        local: 'en-us_topic_0000001827336292.html',
        children: [
          {
            id: 2224,
            parentId: 2220,
            name: 'Creating a Drill Plan',
            local: 'Ransomware0011.html'
          },
          {
            id: 2225,
            parentId: 2220,
            name: 'Managing Drill Plans',
            local: 'Ransomware0012.html'
          },
          {
            id: 2226,
            parentId: 2220,
            name: 'Viewing the Restoration Drill Overview',
            local: 'Ransomware0013.html'
          }
        ]
      },
      {
        id: 2221,
        parentId: 4,
        name: 'Data Anonymization',
        local: 'helpcenter000108.html',
        children: [
          {
            id: 2227,
            parentId: 2221,
            name: 'Configuring Data Anonymization',
            local: 'data_masking_000009.html',
            children: [
              {
                id: 2232,
                parentId: 2227,
                name: 'Importing and Activating a License File',
                local: 'data_masking_000010.html'
              },
              {
                id: 2233,
                parentId: 2227,
                name: 'Adding Anonymization Rules',
                local: 'data_masking_000011.html'
              },
              {
                id: 2234,
                parentId: 2227,
                name: 'Adding Identification Rules',
                local: 'data_masking_000012.html'
              },
              {
                id: 2235,
                parentId: 2227,
                name: 'Creating Anonymization Policies',
                local: 'data_masking_000013.html'
              }
            ]
          },
          {
            id: 2228,
            parentId: 2221,
            name: 'Anonymizing Data in an Oracle Database',
            local: 'data_masking_000015.html'
          },
          {
            id: 2229,
            parentId: 2221,
            name: 'Managing Data Anonymization',
            local: 'data_masking_000018.html',
            children: [
              {
                id: 2236,
                parentId: 2229,
                name: 'Managing Anonymization Policies',
                local: 'data_masking_000019.html'
              },
              {
                id: 2237,
                parentId: 2229,
                name: 'Managing Identification Rules',
                local: 'data_masking_000020.html'
              },
              {
                id: 2238,
                parentId: 2229,
                name: 'Managing Anonymization Rules',
                local: 'data_masking_000021.html'
              }
            ]
          },
          {
            id: 2230,
            parentId: 2221,
            name: 'Anonymization Rule Types',
            local: 'data_masking_000022.html'
          },
          {
            id: 2231,
            parentId: 2221,
            name: 'Configuring Database Listening',
            local: 'data_masking_000024.html'
          }
        ]
      },
      {
        id: 2222,
        parentId: 4,
        name: 'Ransomware Protection',
        local: 'helpcenter000110.html',
        children: [
          {
            id: 2239,
            parentId: 2222,
            name: 'Configuring Ransomware Protection for Copies',
            local: 'ransome_0011.html',
            children: [
              {
                id: 2245,
                parentId: 2239,
                name: 'Creating a Ransomware Protection and WORM Policy',
                local: 'ransome_0012.html'
              }
            ]
          },
          {
            id: 2240,
            parentId: 2222,
            name:
              'Performing Ransomware Protection for Copies (Applicable to 1.5.0)',
            local: 'ransome_0013.html',
            children: [
              {
                id: 2246,
                parentId: 2240,
                name: 'VMware Copies',
                local: 'ransome_0014.html'
              },
              {
                id: 2247,
                parentId: 2240,
                name: 'NAS File System Copies',
                local: 'ransome_0015.html'
              },
              {
                id: 2248,
                parentId: 2240,
                name: 'NAS Share Copies',
                local: 'ransome_0016.html'
              },
              {
                id: 2249,
                parentId: 2240,
                name: 'Fileset Copies',
                local: 'ransome_0017.html'
              }
            ]
          },
          {
            id: 2241,
            parentId: 2222,
            name:
              'Performing Ransomware Protection for Copies (Applicable to 1.6.0 and Later Versions)',
            local: 'ransome16_001.html',
            children: [
              {
                id: 2250,
                parentId: 2241,
                name: 'VMware VM Copies',
                local: 'ransome16_002.html'
              },
              {
                id: 2251,
                parentId: 2241,
                name: 'NAS File System Copies',
                local: 'ransome16_003.html'
              },
              {
                id: 2252,
                parentId: 2241,
                name: 'NAS Share Copies',
                local: 'ransome16_004.html'
              },
              {
                id: 2253,
                parentId: 2241,
                name: 'Fileset Copies',
                local: 'ransome16_005.html'
              },
              {
                id: 2254,
                parentId: 2241,
                name: 'CNware VM Copies',
                local: 'ransome16_006.html'
              },
              {
                id: 2255,
                parentId: 2241,
                name: 'Huawei Cloud Stack Copies',
                local: 'ransome16_007.html'
              },
              {
                id: 2256,
                parentId: 2241,
                name: 'FusionCompute VM Copies',
                local: 'ransome16_008.html'
              },
              {
                id: 2257,
                parentId: 2241,
                name: 'OpenStack Cloud Server Copies',
                local: 'ransome16_009.html'
              },
              {
                id: 2258,
                parentId: 2241,
                name: 'Hyper-V VM Copies',
                local: 'ransome16_010.html'
              },
              {
                id: 2259,
                parentId: 2241,
                name: 'FusionOne Compute VM Copies',
                local: 'ransome160_012.html'
              }
            ]
          },
          {
            id: 2242,
            parentId: 2222,
            name: 'Managing Ransomware Protection for Copies',
            local: 'ransome_0018.html',
            children: [
              {
                id: 2260,
                parentId: 2242,
                name: 'Managing Detection Models',
                local: 'ransome_0019.html'
              },
              {
                id: 2261,
                parentId: 2242,
                name: 'Managing Ransomware Protection and WORM Policies',
                local: 'ransome_0020.html'
              },
              {
                id: 2262,
                parentId: 2242,
                name: 'Managing the Detection Mode',
                local: 'ransome_0021.html'
              },
              {
                id: 2263,
                parentId: 2242,
                name: 'Managing Ransomware Detection Copies',
                local: 'ransome_0022.html'
              },
              {
                id: 2264,
                parentId: 2242,
                name: 'Managing WORM Copies',
                local: 'ransome_0023.html'
              },
              {
                id: 2265,
                parentId: 2242,
                name:
                  'Managing Operation Restrictions on Infected Copies (Applicable to 1.6.0 and Later Versions)',
                local: 'ransome_dis_001.html',
                children: [
                  {
                    id: 2266,
                    parentId: 2265,
                    name: 'Adding Operation Restrictions on Infected Copies',
                    local: 'ransome_dis_002.html'
                  },
                  {
                    id: 2267,
                    parentId: 2265,
                    name:
                      'Viewing Information About Operation Restrictions on Infected Copies',
                    local: 'ransome_dis_003.html'
                  },
                  {
                    id: 2268,
                    parentId: 2265,
                    name: 'Modifying Operation Restrictions on Infected Copies',
                    local: 'ransome_dis_004.html'
                  },
                  {
                    id: 2269,
                    parentId: 2265,
                    name:
                      'Deleting Modifying Operation Restrictions on Infected Copies',
                    local: 'ransome_dis_005.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2243,
            parentId: 2222,
            name: 'Viewing Resource Detection Details (Applicable to 1.5.0)',
            local: 'ransome_0024.html',
            children: [
              {
                id: 2270,
                parentId: 2243,
                name: 'Viewing Detection Details of All Resources',
                local: 'ransome_0025.html'
              },
              {
                id: 2271,
                parentId: 2243,
                name: 'Viewing Detection Details of a Single Resource Type',
                local: 'ransome_0026.html'
              }
            ]
          },
          {
            id: 2244,
            parentId: 2222,
            name:
              'Viewing Resource Detection Details (Applicable to 1.6.0 and Later Versions)',
            local: 'ransome16_011.html',
            children: [
              {
                id: 2272,
                parentId: 2244,
                name: 'Viewing Detection Details of All Resources',
                local: 'ransome16_012.html'
              },
              {
                id: 2273,
                parentId: 2244,
                name: 'Viewing Detection Details of a Single Resource Type',
                local: 'ransome16_013.html'
              }
            ]
          }
        ]
      },
      {
        id: 2223,
        parentId: 4,
        name: 'Air Gap',
        local: 'helpcenter000112.html',
        children: [
          {
            id: 2274,
            parentId: 2223,
            name: 'Configuring Air Gap',
            local: 'AirGap0011.html',
            children: [
              {
                id: 2277,
                parentId: 2274,
                name: 'Creating an Air Gap Policy',
                local: 'AirGap0012.html'
              },
              {
                id: 2278,
                parentId: 2274,
                name: 'Associating an Air Gap Policy',
                local: 'AirGap0013.html'
              }
            ]
          },
          {
            id: 2275,
            parentId: 2223,
            name: 'Managing Air Gap Policies',
            local: 'AirGap0014.html',
            children: [
              {
                id: 2279,
                parentId: 2275,
                name: 'Viewing Air Gap Policies',
                local: 'AirGap0015.html'
              },
              {
                id: 2280,
                parentId: 2275,
                name: 'Modifying an Air Gap Policy',
                local: 'AirGap0016.html'
              },
              {
                id: 2281,
                parentId: 2275,
                name: 'Deleting an Air Gap Policy',
                local: 'AirGap0017.html'
              }
            ]
          },
          {
            id: 2276,
            parentId: 2223,
            name: 'Managing Storage Devices',
            local: 'AirGap0018.html',
            children: [
              {
                id: 2282,
                parentId: 2276,
                name: 'Viewing Storage Devices',
                local: 'AirGap0019.html'
              },
              {
                id: 2283,
                parentId: 2276,
                name:
                  'Modifying the Air Gap Policy Associated with a Storage Device',
                local: 'AirGap0020.html'
              },
              {
                id: 2284,
                parentId: 2276,
                name:
                  'Removing the Air Gap Policy Associated with a Storage Device',
                local: 'AirGap0021.html'
              },
              {
                id: 2285,
                parentId: 2276,
                name:
                  'Enabling the Air Gap Policy Associated with a Storage Device',
                local: 'AirGap0022.html'
              },
              {
                id: 2286,
                parentId: 2276,
                name:
                  'Disabling the Air Gap Policy Associated with a Storage Device',
                local: 'AirGap0023.html'
              },
              {
                id: 2287,
                parentId: 2276,
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
        id: 2288,
        parentId: 5,
        name: 'Configuring Cluster HA',
        local: 'HA00014.html',
        children: [
          {
            id: 2291,
            parentId: 2288,
            name:
              'Adding an Internal Communication Network Plane for the Active Node',
            local: 'HA00015.html',
            children: [
              {
                id: 2296,
                parentId: 2291,
                name:
                  'Adding an Internal Communication Network Plane for the Active Node (Applicable to 1.5.0)',
                local: 'HA00016.html'
              },
              {
                id: 2297,
                parentId: 2291,
                name:
                  'Adding an Internal Communication Network for the Active Node (Applicable to 1.6.0 and Later Versions)',
                local: 'HA00017.html'
              }
            ]
          },
          {
            id: 2292,
            parentId: 2288,
            name:
              'Adding an Internal Communication Network for a Member Node (Applicable to 1.6.0 and Later Versions)',
            local: 'HA00018.html'
          },
          {
            id: 2293,
            parentId: 2288,
            name: 'Adding Member Nodes',
            local: 'HA00019.html'
          },
          {
            id: 2294,
            parentId: 2288,
            name: 'Adding HA Members',
            local: 'HA00020.html'
          },
          {
            id: 2295,
            parentId: 2288,
            name: '(Optional) Creating Backup Storage Unit Groups',
            local: 'HA00021.html'
          }
        ]
      },
      {
        id: 2289,
        parentId: 5,
        name: 'Using Cluster HA',
        local: 'HA00022.html'
      },
      {
        id: 2290,
        parentId: 5,
        name: 'Managing Cluster HA',
        local: 'HA00023.html',
        children: [
          {
            id: 2298,
            parentId: 2290,
            name: 'Managing Local Cluster Nodes',
            local: 'HA00024.html',
            children: [
              {
                id: 2305,
                parentId: 2298,
                name: 'Viewing a Local Cluster Node',
                local: 'HA00025.html'
              },
              {
                id: 2306,
                parentId: 2298,
                name: 'Managing a Standby or Member Node',
                local: 'HA00026.html',
                children: [
                  {
                    id: 2308,
                    parentId: 2306,
                    name: 'Modifying a Standby or Member Node',
                    local: 'HA00027.html'
                  },
                  {
                    id: 2309,
                    parentId: 2306,
                    name: 'Deleting a Member Node',
                    local: 'HA00028.html'
                  }
                ]
              },
              {
                id: 2307,
                parentId: 2298,
                name: 'Managing the HA',
                local: 'HA00029.html',
                children: [
                  {
                    id: 2310,
                    parentId: 2307,
                    name: 'Modifying HA Parameters',
                    local: 'HA00030.html'
                  },
                  {
                    id: 2311,
                    parentId: 2307,
                    name: 'Removing an HA Member',
                    local: 'HA00031.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2299,
            parentId: 2290,
            name: 'Managing a Backup Storage Unit Group',
            local: 'HA00032.html',
            children: [
              {
                id: 2312,
                parentId: 2299,
                name: 'Viewing a Backup Storage Unit Group',
                local: 'HA00033.html'
              },
              {
                id: 2313,
                parentId: 2299,
                name: 'Modifying a Backup Storage Unit Group',
                local: 'HA00034.html'
              },
              {
                id: 2314,
                parentId: 2299,
                name: 'Deleting a Backup Storage Unit Group',
                local: 'HA00035.html'
              }
            ]
          },
          {
            id: 2300,
            parentId: 2290,
            name: 'Managing Backup Storage Units (Applicable to 1.5.0)',
            local: 'HA00036.html',
            children: [
              {
                id: 2315,
                parentId: 2300,
                name: 'Viewing a Backup Storage Unit',
                local: 'HA00037.html'
              },
              {
                id: 2316,
                parentId: 2300,
                name: 'Creating a Backup Storage Unit',
                local: 'HA00038.html'
              },
              {
                id: 2317,
                parentId: 2300,
                name: 'Modifying a Backup Storage Unit',
                local: 'HA00039.html'
              },
              {
                id: 2318,
                parentId: 2300,
                name: 'Deleting a Backup Storage Unit',
                local: 'HA00040.html'
              },
              {
                id: 2319,
                parentId: 2300,
                name: 'Upgrading a Backup Storage Unit as a Member Node',
                local: 'HA00041.html'
              }
            ]
          },
          {
            id: 2301,
            parentId: 2290,
            name:
              'Managing Backup Storage Devices (Applicable to 1.6.0 and Later Versions)',
            local: 'HA00042.html',
            children: [
              {
                id: 2320,
                parentId: 2301,
                name: 'Viewing a Backup Storage Device',
                local: 'HA00043.html'
              },
              {
                id: 2321,
                parentId: 2301,
                name: 'Creating a Backup Storage Device',
                local: 'HA00044.html'
              },
              {
                id: 2322,
                parentId: 2301,
                name: 'Modifying a Backup Storage Device',
                local: 'HA00045.html'
              },
              {
                id: 2323,
                parentId: 2301,
                name: 'Deleting a Backup Storage Device',
                local: 'HA00046.html'
              },
              {
                id: 2324,
                parentId: 2301,
                name: 'Upgrading a Backup Storage Device as a Member Node',
                local: 'HA00047.html'
              }
            ]
          },
          {
            id: 2302,
            parentId: 2290,
            name:
              'Managing Backup Storage Units (Applicable to 1.6.0 and Later Versions)',
            local: 'HA00048.html',
            children: [
              {
                id: 2325,
                parentId: 2302,
                name: 'Viewing a Backup Storage Unit',
                local: 'HA00049.html'
              },
              {
                id: 2326,
                parentId: 2302,
                name: 'Creating a Backup Storage Unit',
                local: 'HA00050.html'
              },
              {
                id: 2327,
                parentId: 2302,
                name: 'Modifying a Backup Storage Unit',
                local: 'HA00051.html'
              },
              {
                id: 2328,
                parentId: 2302,
                name: 'Deleting a Backup Storage Unit',
                local: 'HA00052.html'
              }
            ]
          },
          {
            id: 2303,
            parentId: 2290,
            name:
              'Managing an Internal Communication Network Plane (Applicable to 1.5.0)',
            local: 'HA00053.html',
            children: [
              {
                id: 2329,
                parentId: 2303,
                name: 'Modifying an Internal Communication Network Plane',
                local: 'HA00054.html'
              },
              {
                id: 2330,
                parentId: 2303,
                name: 'Deleting an Internal Communication Network Plane',
                local: 'HA00055.html'
              }
            ]
          },
          {
            id: 2304,
            parentId: 2290,
            name:
              'Managing an Internal Communication Network (Applicable to 1.6.0 and Later Versions)',
            local: 'HA00056.html',
            children: [
              {
                id: 2331,
                parentId: 2304,
                name: 'Modifying an Internal Communication Network',
                local: 'HA00057.html'
              },
              {
                id: 2332,
                parentId: 2304,
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
        id: 2333,
        parentId: 6,
        name: 'Managing Performance Statistics',
        local: 'admin-00135.html',
        children: [
          {
            id: 2337,
            parentId: 2333,
            name: 'Introduction to Performance Indicators',
            local: 'admin-00136.html'
          },
          {
            id: 2338,
            parentId: 2333,
            name: 'Configuring the Performance Statistics Switch',
            local: 'admin-00137.html'
          }
        ]
      },
      {
        id: 2334,
        parentId: 6,
        name: 'Managing Alarms and Events',
        local: 'admin-00139.html'
      },
      {
        id: 2335,
        parentId: 6,
        name: 'Managing Jobs',
        local: 'admin-00140.html',
        children: [
          {
            id: 2339,
            parentId: 2335,
            name: 'Viewing the Job Progress',
            local: 'admin-00141.html'
          },
          {
            id: 2340,
            parentId: 2335,
            name: 'Stopping Jobs',
            local: 'admin-00142.html'
          },
          {
            id: 2341,
            parentId: 2335,
            name: 'Downloading Jobs',
            local: 'admin-00143.html'
          }
        ]
      },
      {
        id: 2336,
        parentId: 6,
        name: 'Managing Reports',
        local: 'admin-00144.html',
        children: [
          {
            id: 2342,
            parentId: 2336,
            name: 'User Roles and Permissions',
            local: 'admin-00145.html'
          },
          {
            id: 2343,
            parentId: 2336,
            name: 'Creating a Report',
            local: 'admin-00146.html'
          },
          {
            id: 2344,
            parentId: 2336,
            name: 'Viewing Reports',
            local: 'admin-00147.html'
          },
          {
            id: 2345,
            parentId: 2336,
            name: 'Downloading Reports',
            local: 'admin-00148.html'
          },
          {
            id: 2346,
            parentId: 2336,
            name: 'Sending an Email',
            local: 'admin-00149.html'
          },
          {
            id: 2347,
            parentId: 2336,
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
        id: 2348,
        parentId: 7,
        name: 'Managing Users (Applicable to 1.5.0)',
        local: 'helpcenter000127.html',
        children: [
          {
            id: 2373,
            parentId: 2348,
            name: 'Overview of User Roles',
            local: 'helpcenter000128.html'
          },
          {
            id: 2374,
            parentId: 2348,
            name: 'Creating a User',
            local: 'helpcenter000129.html'
          },
          {
            id: 2375,
            parentId: 2348,
            name: 'Modifying a User',
            local: 'helpcenter000130.html'
          },
          {
            id: 2376,
            parentId: 2348,
            name: 'Locking Out a User',
            local: 'helpcenter000131.html'
          },
          {
            id: 2377,
            parentId: 2348,
            name: 'Unlock a User',
            local: 'helpcenter000132.html'
          },
          {
            id: 2378,
            parentId: 2348,
            name: 'Removing a User',
            local: 'helpcenter000133.html'
          },
          {
            id: 2379,
            parentId: 2348,
            name: 'Resetting the User Password',
            local: 'helpcenter000134.html'
          },
          {
            id: 2380,
            parentId: 2348,
            name: 'Resetting the System Administrator Password',
            local: 'helpcenter000135.html'
          }
        ]
      },
      {
        id: 2349,
        parentId: 7,
        name: 'Managing RBAC (Applicable to 1.6.0 and Later Versions)',
        local: 'admin-0055.html',
        children: [
          {
            id: 2381,
            parentId: 2349,
            name: 'Built-in User Roles',
            local: 'admin-0056.html'
          },
          {
            id: 2382,
            parentId: 2349,
            name: 'Creating a Role',
            local: 'en-us_topic_0000002059543622.html'
          },
          {
            id: 2383,
            parentId: 2349,
            name: 'Modifying a Role',
            local: 'en-us_topic_0000002059385278.html'
          },
          {
            id: 2384,
            parentId: 2349,
            name: 'Cloning a Role',
            local: 'en-us_topic_0000002095582433.html'
          },
          {
            id: 2385,
            parentId: 2349,
            name: 'Deleting a Role',
            local: 'en-us_topic_0000002095463905.html'
          },
          {
            id: 2386,
            parentId: 2349,
            name: 'Creating a Resource Set',
            local: 'en-us_topic_0000002059543630.html'
          },
          {
            id: 2387,
            parentId: 2349,
            name: 'Deleting a Resource Set',
            local: 'en-us_topic_0000002059385282.html'
          },
          {
            id: 2388,
            parentId: 2349,
            name: 'Modifying a Resource Set',
            local: 'en-us_topic_0000002095582437.html'
          },
          {
            id: 2389,
            parentId: 2349,
            name: 'Creating a User',
            local: 'admin-0057.html'
          },
          {
            id: 2390,
            parentId: 2349,
            name: 'Modifying a User',
            local: 'admin-0058.html'
          },
          {
            id: 2391,
            parentId: 2349,
            name: 'Locking Out a User',
            local: 'admin-0059.html'
          },
          {
            id: 2392,
            parentId: 2349,
            name: 'Unlock a User',
            local: 'admin-0060.html'
          },
          {
            id: 2393,
            parentId: 2349,
            name: 'Removing a User',
            local: 'admin-0061.html'
          },
          {
            id: 2394,
            parentId: 2349,
            name: 'Resetting the User Password',
            local: 'admin-0062.html'
          },
          {
            id: 2395,
            parentId: 2349,
            name: 'Resetting the System Administrator Password',
            local: 'admin-0063.html'
          },
          {
            id: 2396,
            parentId: 2349,
            name: 'Setting the Mailbox for Password Retrieval',
            local: 'admin-0064.html'
          }
        ]
      },
      {
        id: 2350,
        parentId: 7,
        name: 'Managing the SAML SSO Configuration',
        local: 'helpcenter000136.html',
        children: [
          {
            id: 2397,
            parentId: 2350,
            name: 'Creating the SAML SSO Configuration',
            local: 'helpcenter000137.html'
          },
          {
            id: 2398,
            parentId: 2350,
            name: 'Managing the SAML SSO Configuration',
            local: 'helpcenter000138.html',
            children: [
              {
                id: 2400,
                parentId: 2398,
                name: 'Activating or Disabling the SAML SSO Configuration',
                local: 'helpcenter000139.html'
              },
              {
                id: 2401,
                parentId: 2398,
                name: 'Modifying the SAML SSO Configuration',
                local: 'helpcenter000140.html'
              },
              {
                id: 2402,
                parentId: 2398,
                name: 'Deleting the SAML SSO Configuration',
                local: 'helpcenter000141.html'
              }
            ]
          },
          {
            id: 2399,
            parentId: 2350,
            name: 'Exporting Metadata',
            local: 'helpcenter000142.html'
          }
        ]
      },
      {
        id: 2351,
        parentId: 7,
        name: 'Managing Quotas and Functions',
        local: 'helpcenter000143.html',
        children: [
          {
            id: 2403,
            parentId: 2351,
            name: 'Viewing Quotas and Functions',
            local: 'helpcenter000144.html'
          },
          {
            id: 2404,
            parentId: 2351,
            name: 'Setting Quotas',
            local: 'helpcenter000145.html'
          },
          {
            id: 2405,
            parentId: 2351,
            name:
              'Setting Functions (Applicable to 1.6.RC1 and Earlier Versions)',
            local: 'helpcenter000146.html'
          }
        ]
      },
      {
        id: 2352,
        parentId: 7,
        name: 'Managing Backup Clusters',
        local: 'admin-00067.html',
        children: [
          {
            id: 2406,
            parentId: 2352,
            name: 'Manages Local Cluster Nodes',
            local: 'en-us_topic_0000001839224397.html',
            children: [
              {
                id: 2408,
                parentId: 2406,
                name: 'Viewing a Local Cluster Node',
                local: 'en-us_topic_0000001839144469.html'
              },
              {
                id: 2409,
                parentId: 2406,
                name: 'Managing a Standby or Member Node',
                local: 'en-us_topic_0000001792345350.html',
                children: [
                  {
                    id: 2410,
                    parentId: 2409,
                    name: 'Modifying a Standby or Member Node',
                    local: 'en-us_topic_0000001792345230.html'
                  },
                  {
                    id: 2411,
                    parentId: 2409,
                    name: 'Deleting a Member Node',
                    local: 'en-us_topic_0000001792345338.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2407,
            parentId: 2352,
            name: 'Managing Multi-domain Clusters',
            local: 'en-us_topic_0000001839224421.html',
            children: [
              {
                id: 2412,
                parentId: 2407,
                name: 'Viewing Cluster Information',
                local: 'admin-00068.html'
              },
              {
                id: 2413,
                parentId: 2407,
                name: 'Adding an External Cluster',
                local: 'admin-00069.html'
              },
              {
                id: 2414,
                parentId: 2407,
                name: 'Modifying External Cluster Information',
                local: 'admin-00070.html'
              },
              {
                id: 2415,
                parentId: 2407,
                name: 'Deleting an External Cluster',
                local: 'admin-00071.html'
              },
              {
                id: 2416,
                parentId: 2407,
                name: 'Specifying an External Cluster as a Management Cluster',
                local: 'admin-00072.html'
              },
              {
                id: 2417,
                parentId: 2407,
                name: 'Specifying an External Cluster as a Managed Cluster',
                local: 'admin-00073.html'
              },
              {
                id: 2418,
                parentId: 2407,
                name: 'Authorization',
                local: 'admin-00074.html'
              },
              {
                id: 2419,
                parentId: 2407,
                name:
                  'Modifying Authorization of a Local Cluster Data Protection Administrator',
                local: 'admin-00075.html'
              },
              {
                id: 2420,
                parentId: 2407,
                name:
                  'Canceling Authorization of a Local Cluster Data Protection Administrator',
                local: 'admin-00076.html'
              }
            ]
          }
        ]
      },
      {
        id: 2353,
        parentId: 7,
        name: 'Managing Replication Clusters',
        local: 'en-us_topic_0000001839144457.html',
        children: [
          {
            id: 2421,
            parentId: 2353,
            name: 'Adding an External Cluster',
            local: 'en-us_topic_0000001839224425.html'
          },
          {
            id: 2422,
            parentId: 2353,
            name: 'Viewing Cluster Information',
            local: 'en-us_topic_0000001839144417.html'
          },
          {
            id: 2423,
            parentId: 2353,
            name: 'Modifying a Replication Cluster',
            local: 'en-us_topic_0000001839224361.html'
          },
          {
            id: 2424,
            parentId: 2353,
            name: 'Deleting a Replication Cluster',
            local: 'en-us_topic_0000001839144453.html'
          }
        ]
      },
      {
        id: 2354,
        parentId: 7,
        name: 'Managing Local Storage',
        local: 'admin-00078.html',
        children: [
          {
            id: 2425,
            parentId: 2354,
            name: 'Viewing Local Storage Information',
            local: 'admin-00079.html'
          },
          {
            id: 2426,
            parentId: 2354,
            name:
              'Configuring the Capacity Alarm Threshold of the Local Storage',
            local: 'admin-00080.html'
          },
          {
            id: 2427,
            parentId: 2354,
            name: 'Viewing Local Storage Authentication Information',
            local: 'admin-00081.html'
          },
          {
            id: 2428,
            parentId: 2354,
            name: 'Modifying Local Storage Authentication Information',
            local: 'admin-00082.html'
          },
          {
            id: 2429,
            parentId: 2354,
            name: 'Manually Reclaiming Space',
            local: 'admin-00087.html'
          }
        ]
      },
      {
        id: 2355,
        parentId: 7,
        name: 'Managing Object-based Storage',
        local: 'helpcenter000168.html',
        children: [
          {
            id: 2430,
            parentId: 2355,
            name: 'Adding Archive Storage',
            local: 'oracle_gud_000030.html'
          },
          {
            id: 2431,
            parentId: 2355,
            name: 'Importing Archive Storage Copies',
            local: 'helpcenter000170.html'
          },
          {
            id: 2432,
            parentId: 2355,
            name: 'Modifying Basic Information of an Archive Storage',
            local: 'helpcenter000171.html'
          },
          {
            id: 2433,
            parentId: 2355,
            name: 'Modifying the Capacity Alarm Threshold of Archive Storage',
            local: 'helpcenter000172.html'
          },
          {
            id: 2434,
            parentId: 2355,
            name: 'Viewing Archive Storage Information',
            local: 'helpcenter000173.html'
          },
          {
            id: 2435,
            parentId: 2355,
            name: 'Deleting an Archive Storage',
            local: 'helpcenter000174.html'
          }
        ]
      },
      {
        id: 2356,
        parentId: 7,
        name: 'Managing Tapes',
        local: 'helpcenter000175.html',
        children: [
          {
            id: 2436,
            parentId: 2356,
            name: 'Managing Tape Libraries',
            local: 'helpcenter000176.html',
            children: [
              {
                id: 2438,
                parentId: 2436,
                name: 'Scanning Tape Libraries',
                local: 'helpcenter000177.html'
              },
              {
                id: 2439,
                parentId: 2436,
                name: 'Managing Drivers',
                local: 'helpcenter000178.html',
                children: [
                  {
                    id: 2441,
                    parentId: 2439,
                    name: 'Checking the Driver',
                    local: 'helpcenter000179.html'
                  },
                  {
                    id: 2442,
                    parentId: 2439,
                    name: 'Enabling a Driver',
                    local: 'helpcenter000180.html'
                  },
                  {
                    id: 2443,
                    parentId: 2439,
                    name: 'Disabling a Driver',
                    local: 'helpcenter000181.html'
                  }
                ]
              },
              {
                id: 2440,
                parentId: 2436,
                name: 'Managing Tapes',
                local: 'helpcenter000182.html',
                children: [
                  {
                    id: 2444,
                    parentId: 2440,
                    name: 'Viewing Tapes',
                    local: 'helpcenter000183.html'
                  },
                  {
                    id: 2445,
                    parentId: 2440,
                    name: 'Loading Tapes',
                    local: 'helpcenter000184.html'
                  },
                  {
                    id: 2446,
                    parentId: 2440,
                    name: 'Unloading Tapes',
                    local: 'helpcenter000185.html'
                  },
                  {
                    id: 2447,
                    parentId: 2440,
                    name: 'Deleting Tapes',
                    local: 'helpcenter000186.html'
                  },
                  {
                    id: 2448,
                    parentId: 2440,
                    name: 'Identifying Tapes',
                    local: 'helpcenter000187.html'
                  },
                  {
                    id: 2449,
                    parentId: 2440,
                    name: 'Marking a Tape as Empty',
                    local: 'helpcenter000188.html'
                  },
                  {
                    id: 2450,
                    parentId: 2440,
                    name: 'Erasing Tapes',
                    local: 'helpcenter000189.html'
                  }
                ]
              }
            ]
          },
          {
            id: 2437,
            parentId: 2356,
            name: 'Managing Media Sets',
            local: 'helpcenter000190.html',
            children: [
              {
                id: 2451,
                parentId: 2437,
                name: 'Creating a Media Set',
                local: 'helpcenter000191.html'
              },
              {
                id: 2452,
                parentId: 2437,
                name: 'Viewing a Media Set',
                local: 'helpcenter000192.html'
              },
              {
                id: 2453,
                parentId: 2437,
                name: 'Modifying a Media Set',
                local: 'helpcenter000193.html'
              },
              {
                id: 2454,
                parentId: 2437,
                name: 'Deleting a Media Set',
                local: 'helpcenter000194.html'
              }
            ]
          }
        ]
      },
      {
        id: 2357,
        parentId: 7,
        name: 'Viewing System Information',
        local: 'helpcenter000195.html',
        children: [
          {
            id: 2455,
            parentId: 2357,
            name: 'Viewing System Version Information',
            local: 'helpcenter000196.html'
          },
          {
            id: 2456,
            parentId: 2357,
            name: 'Viewing the ESN of a Device',
            local: 'helpcenter000197.html'
          }
        ]
      },
      {
        id: 2358,
        parentId: 7,
        name: 'Managing Security Policies',
        local: 'admin_policy_000000.html'
      },
      {
        id: 2359,
        parentId: 7,
        name: 'Managing Certificates',
        local: 'admin_cer_000000.html',
        children: [
          {
            id: 2457,
            parentId: 2359,
            name: 'Viewing Certificate Information',
            local: 'admin_cer_000001.html'
          },
          {
            id: 2458,
            parentId: 2359,
            name: 'Adding an External Certificate',
            local: 'admin_cer_000002.html'
          },
          {
            id: 2459,
            parentId: 2359,
            name: 'Importing a Certificate',
            local: 'admin_cer_000003.html'
          },
          {
            id: 2460,
            parentId: 2359,
            name: 'Exporting Request Files',
            local: 'admin_cer_000004.html'
          },
          {
            id: 2461,
            parentId: 2359,
            name: 'Modifying Expiration Warning Days',
            local: 'admin_cer_000005.html'
          },
          {
            id: 2462,
            parentId: 2359,
            name: 'Managing the Certificate Revocation List',
            local: 'admin_cer_000007.html',
            children: [
              {
                id: 2465,
                parentId: 2462,
                name: 'Importing a CRL',
                local: 'admin_cer_000008.html'
              },
              {
                id: 2466,
                parentId: 2462,
                name: 'Viewing a CRL',
                local: 'admin_cer_000009.html'
              },
              {
                id: 2467,
                parentId: 2462,
                name: 'Downloading a CRL',
                local: 'admin_cer_000010.html'
              },
              {
                id: 2468,
                parentId: 2462,
                name: 'Deleting a CRL',
                local: 'admin_cer_000011.html'
              }
            ]
          },
          {
            id: 2463,
            parentId: 2359,
            name: 'Downloading a Certificate',
            local: 'admin_cer_000012.html'
          },
          {
            id: 2464,
            parentId: 2359,
            name: 'Deleting an External Certificate',
            local: 'admin_cer_000013.html'
          }
        ]
      },
      {
        id: 2360,
        parentId: 7,
        name: 'Host Trust Management',
        local: 'admin_host_000000.html'
      },
      {
        id: 2361,
        parentId: 7,
        name: 'Managing Logs',
        local: 'admin_log_000000.html'
      },
      {
        id: 2362,
        parentId: 7,
        name: 'Exporting Records',
        local: 'en-us_topic_0000001839144381.html'
      },
      {
        id: 2363,
        parentId: 7,
        name: 'Managing System Data Backup',
        local: 'admin_mngbackup_000000.html',
        children: [
          {
            id: 2469,
            parentId: 2363,
            name: 'Configuring Management Data Backup',
            local: 'admin_mngbackup_000001.html'
          },
          {
            id: 2470,
            parentId: 2363,
            name: 'Exporting a Management Data Backup',
            local: 'admin_mngbackup_000002.html'
          },
          {
            id: 2471,
            parentId: 2363,
            name: 'Deleting a Management Data Backup',
            local: 'admin_mngbackup_000003.html'
          },
          {
            id: 2472,
            parentId: 2363,
            name: 'Importing a Management Data Backup',
            local: 'admin_mngbackup_000004.html'
          },
          {
            id: 2473,
            parentId: 2363,
            name: 'Restoring Management Data',
            local: 'admin_mngbackup_000005.html'
          }
        ]
      },
      {
        id: 2364,
        parentId: 7,
        name: 'Managing the Email Service',
        local: 'admin_email_000000.html'
      },
      {
        id: 2365,
        parentId: 7,
        name: 'Managing Event Dump',
        local: 'admin_email_save_000000.html'
      },
      {
        id: 2366,
        parentId: 7,
        name: 'Managing SNMP Trap Notification',
        local: 'admin_snmp.html'
      },
      {
        id: 2367,
        parentId: 7,
        name: 'Managing the SFTP Service (Applicable to 1.5.0)',
        local: 'admin_sftp_000000.html',
        children: [
          {
            id: 2474,
            parentId: 2367,
            name: 'Enabling the SFTP Service',
            local: 'admin_sftp_000001.html'
          },
          {
            id: 2475,
            parentId: 2367,
            name: 'Viewing the SFTP Service',
            local: 'admin_sftp_000002.html'
          },
          {
            id: 2476,
            parentId: 2367,
            name: 'Creating an SFTP User',
            local: 'admin_sftp_000003.html'
          },
          {
            id: 2477,
            parentId: 2367,
            name: 'Changing SFTP User Password',
            local: 'admin_sftp_000004.html'
          },
          {
            id: 2478,
            parentId: 2367,
            name: 'Deleting an SFTP User',
            local: 'admin_sftp_000005.html'
          }
        ]
      },
      {
        id: 2368,
        parentId: 7,
        name:
          'Managing the SFTP Service (Applicable to 1.6.0 and Later Versions)',
        local: 'admin-00261.html',
        children: [
          {
            id: 2479,
            parentId: 2368,
            name: 'Enabling the SFTP Service',
            local: 'admin-00262.html'
          },
          {
            id: 2480,
            parentId: 2368,
            name: 'Viewing the SFTP Service',
            local: 'admin-00263.html'
          },
          {
            id: 2481,
            parentId: 2368,
            name: 'Creating an SFTP User',
            local: 'admin-00264.html'
          },
          {
            id: 2482,
            parentId: 2368,
            name: 'Changing SFTP User Password',
            local: 'admin-00265.html'
          },
          {
            id: 2483,
            parentId: 2368,
            name: 'Deleting an SFTP User',
            local: 'admin-00266.html'
          }
        ]
      },
      {
        id: 2369,
        parentId: 7,
        name: 'Managing the Device Time',
        local: 'admin_sys_time.html'
      },
      {
        id: 2370,
        parentId: 7,
        name: 'Configuring the LDAP Service',
        local: 'en-us_topic_0000001839144385.html'
      },
      {
        id: 2371,
        parentId: 7,
        name:
          'Managing Windows ADFS Configuration (Applicable to 1.6.0 and Later Versions)',
        local: 'admin-0077.html'
      },
      {
        id: 2372,
        parentId: 7,
        name:
          'Managing Backup Software Management (Applicable to 1.6.0 and Later Versions)',
        local: 'en-us_topic_0000001938830850.html',
        children: [
          {
            id: 2484,
            parentId: 2372,
            name: 'Adding Backup Software Management',
            local: 'en-us_topic_0000001965949181.html'
          },
          {
            id: 2485,
            parentId: 2372,
            name: 'Modifying Backup Software Management',
            local: 'en-us_topic_0000001966069409.html'
          },
          {
            id: 2486,
            parentId: 2372,
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

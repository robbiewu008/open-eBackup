/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include <iostream>
#include <memory>
#include "common/CommonMock.h"
#include "protect_engines/openstack/OpenStackProtectEngine.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
#include "repository_handlers/mock/FileSystemHandlerMock.h"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Invoke;;
using ::testing::A;
using AppProtect::ApplicationEnvironment;
using AppProtect::ResourceResultByPage;
using AppProtect::QueryByPage;
using AppProtect::Application;
using AppProtect::ApplicationResource;
using AppProtect::ActionResult;

using namespace VirtPlugin;
using namespace OpenStackPlugin;


namespace HDT_TEST {

std::string getProjectsResponse = "{\
	\"links\": {\
		\"self\": \"https://identity.az236.dc236.huawei.com/identity/v3/projects?domain_id=default\",\
		\"previous\": null,\
		\"next\": null\
	},\
	\"projects\": [{\
			\"is_domain\": false,\
			\"description\": \"\",\
			\"links\": {\
				\"self\": \"https://identity.az236.dc236.huawei.com/identity/v3/projects/e3aba2c380994af29f629aeae0f75564\"\
			},\
			\"enabled\": true,\
			\"id\": \"e3aba2c380994af29f629aeae0f75564\",\
			\"parent_id\": null,\
			\"domain_id\": \"default\",\
			\"name\": \"service\"\
		},\
		{\
			\"is_domain\": false,\
			\"description\": \"\",\
			\"links\": {\
				\"self\": \"https://identity.az236.dc236.huawei.com/identity/v3/projects/13f648f23eac4f3abc870eef7f41bc56\"\
			},\
			\"enabled\": true,\
			\"id\": \"13f648f23eac4f3abc870eef7f41bc56\",\
			\"parent_id\": null,\
			\"domain_id\": \"default\",\
			\"name\": \"admin\"\
		},\
		{\
			\"is_domain\": false,\
			\"description\": \"the network project for dc236\",\
			\"links\": {\
				\"self\": \"https://identity.az236.dc236.huawei.com/identity/v3/projects/28a23331b60747bfb2bd8606ba930396\"\
			},\
			\"enabled\": true,\
			\"id\": \"28a23331b60747bfb2bd8606ba930396\",\
			\"parent_id\": \"default\",\
			\"domain_id\": \"default\",\
			\"name\": \"dc_network_dc236\"\
		}\
	]\
}";

std::string getServersListResponseBody = "{\
	\"servers\": [{\
		\"status\": \"ACTIVE\",\
		\"updated\": \"2023-01-05T15:12:50Z\",\
		\"hostId\": \"552c065ee873220d61eb0efc86eb97d19a076e543d11389c52cca397\",\
		\"OS-EXT-SRV-ATTR:host\": \"6B71238C-6BF4-A8B6-EC11-3E66A42F2ABF\",\
		\"addresses\": {\
			\"external_api\": [{\
				\"OS-EXT-IPS-MAC:mac_addr\": \"fa:16:3e:5c:aa:94\",\
				\"version\": 4,\
				\"addr\": \"10.9.0.86\",\
				\"OS-EXT-IPS:type\": \"fixed\"\
			}]\
		},\
		\"links\": [{\
			\"href\": \"https://compute.az236.dc236.huawei.com/v2.1/542ada72ff644cd2819446ada95920cb/servers/ec4e9110-e3a1-4160-9362-9c22635bf289\",\
			\"rel\": \"self\"\
		}, {\
			\"href\": \"https://compute.az236.dc236.huawei.com/542ada72ff644cd2819446ada95920cb/servers/ec4e9110-e3a1-4160-9362-9c22635bf289\",\
			\"rel\": \"bookmark\"\
		}],\
		\"key_name\": null,\
		\"image\": {\
			\"id\": \"8b67c617-4238-4b20-bf10-1db1e8b12f25\",\
			\"links\": [{\
				\"href\": \"https://compute.az236.dc236.huawei.com/542ada72ff644cd2819446ada95920cb/images/8b67c617-4238-4b20-bf10-1db1e8b12f25\",\
				\"rel\": \"bookmark\"\
			}]\
		},\
		\"OS-EXT-STS:task_state\": null,\
		\"OS-EXT-STS:vm_state\": \"active\",\
		\"OS-EXT-SRV-ATTR:instance_name\": \"instance-00000045\",\
		\"OS-SRV-USG:launched_at\": \"2023-01-03T08:32:45.000000\",\
		\"OS-EXT-SRV-ATTR:hypervisor_hostname\": \"6B71238C-6BF4-A8B6-EC11-3E66A42F2ABF\",\
		\"flavor\": {\
			\"id\": \"87012f16-54d6-4c4d-bf89-e52e0d447308\",\
			\"links\": [{\
				\"href\": \"https://compute.az236.dc236.huawei.com/542ada72ff644cd2819446ada95920cb/flavors/87012f16-54d6-4c4d-bf89-e52e0d447308\",\
				\"rel\": \"bookmark\"\
			}]\
		},\
		\"id\": \"ec4e9110-e3a1-4160-9362-9c22635bf289\",\
		\"OS-SRV-USG:terminated_at\": null,\
		\"OS-EXT-AZ:availability_zone\": \"az236.dc236\",\
		\"user_id\": \"fda09b00f4f04e5fb5b897a4d9bc234d\",\
		\"name\": \"xjptest\",\
		\"created\": \"2023-01-03T08:31:31Z\",\
		\"tenant_id\": \"542ada72ff644cd2819446ada95920cb\",\
		\"OS-DCF:diskConfig\": \"MANUAL\",\
		\"os-extended-volumes:volumes_attached\": [],\
		\"accessIPv4\": \"\",\
		\"accessIPv6\": \"\",\
		\"progress\": 0,\
		\"OS-EXT-STS:power_state\": 1,\
		\"config_drive\": \"\",\
		\"metadata\": {\
			\"cascaded.instance_extrainfo\": \"current_mem:6144,max_mem:4194304,max_cpu:4,cpu_num_for_one_plug:1,org_cpu:4,xml_support_live_resize:False,org_mem:6144,iohang_timeout:0,current_cpu:4,num_of_mem_plug:0\"\
		}\
	}, {\
		\"status\": \"ACTIVE\",\
		\"updated\": \"2023-01-05T15:12:50Z\",\
		\"hostId\": \"644d0a5781e9aafd889f08fe53e8407980bf226fce772756d93ae803\",\
		\"OS-EXT-SRV-ATTR:host\": \"F3B0238C-6BF4-D6A4-EC11-3F664EFE9165\",\
		\"addresses\": {\
			\"external_om\": [{\
				\"OS-EXT-IPS-MAC:mac_addr\": \"fa:16:3e:6e:27:45\",\
				\"version\": 4,\
				\"addr\": \"40.5.5.61\",\
				\"OS-EXT-IPS:type\": \"fixed\"\
			}]\
		},\
		\"links\": [{\
			\"href\": \"https://compute.az236.dc236.huawei.com/v2.1/542ada72ff644cd2819446ada95920cb/servers/764d94e0-edb9-4fd6-b390-724477f5ff35\",\
			\"rel\": \"self\"\
		}, {\
			\"href\": \"https://compute.az236.dc236.huawei.com/542ada72ff644cd2819446ada95920cb/servers/764d94e0-edb9-4fd6-b390-724477f5ff35\",\
			\"rel\": \"bookmark\"\
		}],\
		\"key_name\": null,\
		\"image\": \"\",\
		\"OS-EXT-STS:task_state\": null,\
		\"OS-EXT-STS:vm_state\": \"active\",\
		\"OS-EXT-SRV-ATTR:instance_name\": \"instance-00000023\",\
		\"OS-SRV-USG:launched_at\": \"2022-05-26T12:27:12.000000\",\
		\"OS-EXT-SRV-ATTR:hypervisor_hostname\": \"F3B0238C-6BF4-D6A4-EC11-3F664EFE9165\",\
		\"flavor\": {\
			\"id\": \"87012f16-54d6-4c4d-bf89-e52e0d447308\",\
			\"links\": [{\
				\"href\": \"https://compute.az236.dc236.huawei.com/542ada72ff644cd2819446ada95920cb/flavors/87012f16-54d6-4c4d-bf89-e52e0d447308\",\
				\"rel\": \"bookmark\"\
			}]\
		},\
		\"id\": \"764d94e0-edb9-4fd6-b390-724477f5ff35\",\
		\"OS-SRV-USG:terminated_at\": null,\
		\"OS-EXT-AZ:availability_zone\": \"az236.dc236\",\
		\"user_id\": \"192da37c781e4e22bfeffca4338f026b\",\
		\"name\": \"EulerOS25\",\
		\"created\": \"2022-05-26T12:27:04Z\",\
		\"tenant_id\": \"542ada72ff644cd2819446ada95920cb\",\
		\"OS-DCF:diskConfig\": \"MANUAL\",\
		\"os-extended-volumes:volumes_attached\": [{\
			\"id\": \"47db81f7-8a70-4339-b627-12a67fe0c4a8\"\
		}],\
		\"accessIPv4\": \"\",\
		\"accessIPv6\": \"\",\
		\"progress\": 0,\
		\"OS-EXT-STS:power_state\": 1,\
		\"config_drive\": \"\",\
		\"metadata\": {\
			\"cascaded.instance_extrainfo\": \"current_mem:6144,max_mem:4194304,max_cpu:4,cpu_num_for_one_plug:1,org_cpu:4,xml_support_live_resize:False,org_mem:6144,iohang_timeout:0,current_cpu:4,num_of_mem_plug:0\",\
			\"__bootDev\": \"hd,cdrom,network\"\
		}\
	}]\
}";

std::string getServersResponseBody = "{\
	\"server\": {\
		\"status\": \"ACTIVE\",\
		\"updated\": \"2022-11-21T13:17:29Z\",\
		\"hostId\": \"4897ed443d920a2bdc4414d67ca1129ed7c5db7628a22dec8c21ba00\",\
		\"OS-EXT-SRV-ATTR:host\": \"2E3C238C-6BF4-5EA6-EC11-3166840F6C4D\",\
		\"addresses\": {\
			\"external_api\": [{\
				\"OS-EXT-IPS-MAC:mac_addr\": \"fa:16:3e:f1:5b:b6\",\
				\"version\": 4,\
				\"addr\": \"10.9.0.2\",\
				\"OS-EXT-IPS:type\": \"fixed\"\
			}]\
		},\
		\"links\": [{\
				\"href\": \"https://compute.az236.dc236.huawei.com/v2.1/13f648f23eac4f3abc870eef7f41bc56/servers/9eb08b18-439b-470c-b28b-75df424d18fd\",\
				\"rel\": \"self\"\
			},\
			{\
				\"href\": \"https://compute.az236.dc236.huawei.com/13f648f23eac4f3abc870eef7f41bc56/servers/9eb08b18-439b-470c-b28b-75df424d18fd\",\
				\"rel\": \"bookmark\"\
			}],\
		\"key_name\": null,\
		\"image\": {\
			\"id\": \"59f6d58a-25b7-4b7c-a46b-8c40ade28d43\",\
			\"links\": [{\
				\"href\": \"https://compute.az236.dc236.huawei.com/13f648f23eac4f3abc870eef7f41bc56/images/59f6d58a-25b7-4b7c-a46b-8c40ade28d43\",\
				\"rel\": \"bookmark\"\
			}]\
		},\
		\"OS-EXT-STS:task_state\": null,\
		\"OS-EXT-STS:vm_state\": \"active\",\
		\"OS-EXT-SRV-ATTR:instance_name\": \"instance-00000040\",\
		\"OS-SRV-USG:launched_at\": \"2022-11-21T12:45:03.000000\",\
		\"OS-EXT-SRV-ATTR:hypervisor_hostname\": \"2E3C238C-6BF4-5EA6-EC11-3166840F6C4D\",\
		\"flavor\": {\
			\"id\": \"57f41ef7-74d8-4781-b7e3-831a57b6b06a\",\
			\"links\": [{\
				\"href\": \"https://compute.az236.dc236.huawei.com/13f648f23eac4f3abc870eef7f41bc56/flavors/57f41ef7-74d8-4781-b7e3-831a57b6b06a\",\
				\"rel\": \"bookmark\"\
			}]\
		},\
		\"id\": \"9eb08b18-439b-470c-b28b-75df424d18fd\",\
		\"OS-SRV-USG:terminated_at\": null,\
		\"OS-EXT-AZ:availability_zone\": \"az236.dc236\",\
		\"user_id\": \"278c058da02b44d484dc8b99a77063de\",\
		\"name\": \"xujianping\",\
		\"created\": \"2022-11-21T12:42:06Z\",\
		\"tenant_id\": \"13f648f23eac4f3abc870eef7f41bc56\",\
		\"OS-DCF:diskConfig\": \"MANUAL\",\
		\"os-extended-volumes:volumes_attached\": [],\
		\"accessIPv4\": \"\",\
		\"accessIPv6\": \"\",\
		\"progress\": 0,\
		\"OS-EXT-STS:power_state\": 1,\
		\"config_drive\": \"\",\
		\"metadata\": {\
			\"cascaded.instance_extrainfo\": \"current_mem:6144,max_mem:4194304,max_cpu:4,cpu_num_for_one_plug:1,org_cpu:4,xml_support_live_resize:False,org_mem:6144,iohang_timeout:0,current_cpu:4,num_of_mem_plug:0\"\
		}\
	}\
}";

std::string createServerResponse = "{\
    \"server\": {\
        \"OS-DCF:diskConfig\": \"AUTO\",\
        \"adminPass\": \"6NpUwoz2QDRN\",\
        \"id\": \"f5dc173b-6804-445a-a6d8-c705dad5b5eb\",\
        \"links\": [\
            {\
                \"href\": \"http://openstack.example.com/v2/6f70656e737461636b20342065766572/servers/f5dc173b-6804-445a-a6d8-c705dad5b5eb\",\
                \"rel\": \"self\"\
            },\
            {\
                \"href\": \"http://openstack.example.com/6f70656e737461636b20342065766572/servers/f5dc173b-6804-445a-a6d8-c705dad5b5eb\",\
                \"rel\": \"bookmark\"\
            }\
        ],\
        \"security_groups\": [\
            {\
                \"name\": \"default\"\
            }\
        ]\
    }\
}";

std::string getProjectVolumesResponse = "{\
	\"volumes\": [{\
			\"attachments\": [{\
				\"server_id\": \"45f25073-d5f2-499f-87f9-4f4b5ff6d9f2\",\
				\"attachment_id\": \"f7f561f1-7b87-4405-8f72-e5456e5f37fd\",\
				\"attached_at\": \"2022-11-29T02:27:12.333791\",\
				\"host_name\": null,\
				\"volume_id\": \"811201f8-7730-4c54-952f-f6c0e0ef2974\",\
				\"device\": \"/dev/vda\",\
				\"id\": \"811201f8-7730-4c54-952f-f6c0e0ef2974\"\
			}],\
			\"availability_zone\": \"az236.dc236\",\
			\"os-vol-host-attr:host\": \"cinder@FusionSphereOpenstack#FusionSphereOpenstack-ma\",\
			\"encrypted\": false,\
			\"os-volume-replication:extended_status\": null,\
			\"snapshot_id\": null,\
			\"wwn\": \"658f987100b749bc4812215400001b25\",\
			\"id\": \"811201f8-7730-4c54-952f-f6c0e0ef2974\",\
			\"size\": 40,\
			\"user_id\": \"278c058da02b44d484dc8b99a77063de\",\
			\"os-vol-tenant-attr:tenant_id\": \"28a23331b60747bfb2bd8606ba930396\",\
			\"os-vol-mig-status-attr:migstat\": null,\
			\"metadata\": {\
				\"StorageType\": \"OceanStorV5\",\
				\"take_over_lun_wwn\": \"--\",\
				\"attached_mode\": \"rw\",\
				\"readonly\": \"False\",\
				\"lun_wwn\": \"658f987100b749bc4812215400001b25\"\
			},\
			\"status\": \"in-use\",\
			\"volume_image_metadata\": {\
				\"disk_format\": \"iso\",\
				\"architecture\": \"x86_64\",\
				\"container_format\": \"bare\",\
				\"hw_disk_bus\": \"virtio\"\
			},\
			\"backup_id\": null,\
			\"description\": \"\",\
			\"multiattach\": false,\
			\"os-volume-replication:driver_data\": \"{\\\"lun_id\\\": \\\"6949\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\",\
			\"consistencygroup_id\": null,\
			\"os-vol-pro-location-attr:provider_location\": \"6949\",\
			\"os-vol-mig-status-attr:name_id\": null,\
			\"name\": \"Backup_volume_123\",\
			\"bootable\": \"true\",\
			\"created_at\": \"2022-11-29T02:26:43.582123\",\
			\"volume_type\": null,\
			\"shareable\": false,\
			\"os-vendor-ext-attr:vendor_driver_metadata\": {}\
		},\
		{\
			\"attachments\": [{\
				\"server_id\": \"0448fe3d-0a72-4a30-9d28-6aead0a1d1b8\",\
				\"attachment_id\": \"f6ea777c-3bc9-4132-9dab-f4b9fd6d0c81\",\
				\"attached_at\": \"2022-11-29T02:25:28.107888\",\
				\"host_name\": null,\
				\"volume_id\": \"921cec57-9e73-4508-9308-00d88eaa97ed\",\
				\"device\": \"/dev/vda\",\
				\"id\": \"921cec57-9e73-4508-9308-00d88eaa97ed\"\
			}],\
			\"availability_zone\": \"az236.dc236\",\
			\"os-vol-host-attr:host\": \"cinder@FusionSphereOpenstack#FusionSphereOpenstack-ma\",\
			\"updated_at\": \"2022-11-29T02:25:28.135472\",\
			\"replication_status\": \"disabled\",\
			\"snapshot_id\": null,\
			\"wwn\": \"658f987100b749bc4811bcd300001b23\",\
			\"id\": \"921cec57-9e73-4508-9308-00d88eaa97ed\",\
			\"size\": 40,\
			\"user_id\": \"278c058da02b44d484dc8b99a77063de\",\
			\"os-vol-tenant-attr:tenant_id\": \"28a23331b60747bfb2bd8606ba930396\",\
			\"os-vol-mig-status-attr:migstat\": null,\
			\"metadata\": {\
				\"StorageType\": \"OceanStorV5\",\
				\"take_over_lun_wwn\": \"--\",\
				\"attached_mode\": \"rw\",\
				\"readonly\": \"False\",\
				\"lun_wwn\": \"658f987100b749bc4811bcd300001b23\"\
			},\
			\"status\": \"in-use\",\
			\"volume_image_metadata\": {\
				\"architecture\": \"x86_64\",\
				\"container_format\": \"bare\",\
				\"hw_disk_bus\": \"virtio\"\
			},\
			\"backup_id\": null,\
			\"description\": \"\",\
			\"multiattach\": false,\
			\"os-volume-replication:driver_data\": \"{\\\"lun_id\\\": \\\"6947\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\",\
			\"source_volid\": null,\
			\"consistencygroup_id\": null,\
			\"os-vol-pro-location-attr:provider_location\": \"6947\",\
			\"os-vol-mig-status-attr:name_id\": null,\
			\"name\": \"\",\
			\"bootable\": \"true\",\
			\"created_at\": \"2022-11-29T02:25:00.328411\",\
			\"volume_type\": null,\
			\"shareable\": false,\
			\"os-vendor-ext-attr:vendor_driver_metadata\": {}\
		},\
		{\
			\"attachments\": [{\
				\"server_id\": \"85c80531-a24c-4732-9f45-228c8301478f\",\
				\"attachment_id\": \"6b7a281e-dcde-47d5-8d4a-6a4e9748111f\",\
				\"attached_at\": \"2022-11-29T02:26:39.847005\",\
				\"host_name\": null,\
				\"volume_id\": \"787c2a44-d8f2-41de-ba04-a70fce8201bd\",\
				\"device\": \"/dev/vda\",\
				\"id\": \"787c2a44-d8f2-41de-ba04-a70fce8201bd\"\
			}],\
			\"availability_zone\": \"az236.dc236\",\
			\"os-vol-host-attr:host\": \"cinder@FusionSphereOpenstack#FusionSphereOpenstack-ma\",\
			\"encrypted\": false,\
			\"updated_at\": \"2022-11-29T02:26:39.874180\",\
			\"os-volume-replication:extended_status\": null,\
			\"replication_status\": \"disabled\",\
			\"snapshot_id\": null,\
			\"wwn\": \"658f987100b749bc4811cd7700001b24\",\
			\"id\": \"787c2a44-d8f2-41de-ba04-a70fce8201bd\",\
			\"size\": 40,\
			\"user_id\": \"278c058da02b44d484dc8b99a77063de\",\
			\"os-vol-tenant-attr:tenant_id\": \"28a23331b60747bfb2bd8606ba930396\",\
			\"os-vol-mig-status-attr:migstat\": null,\
			\"metadata\": {\
				\"StorageType\": \"OceanStorV5\",\
				\"take_over_lun_wwn\": \"--\",\
				\"attached_mode\": \"rw\",\
				\"readonly\": \"False\",\
				\"lun_wwn\": \"658f987100b749bc4811cd7700001b24\"\
			},\
			\"status\": \"in-use\",\
			\"volume_image_metadata\": {\
				\"architecture\": \"x86_64\",\
				\"container_format\": \"bare\",\
				\"hw_disk_bus\": \"virtio\"\
			},\
			\"backup_id\": null,\
			\"description\": \"\",\
			\"multiattach\": false,\
			\"os-volume-replication:driver_data\": \"{\\\"lun_id\\\": \\\"6948\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\",\
			\"source_volid\": null,\
			\"consistencygroup_id\": null,\
			\"os-vol-pro-location-attr:provider_location\": \"6948\",\
			\"os-vol-mig-status-attr:name_id\": null,\
			\"name\": \"\",\
			\"bootable\": \"true\",\
			\"created_at\": \"2022-11-29T02:23:25.771826\",\
			\"volume_type\": null,\
			\"shareable\": false,\
			\"os-vendor-ext-attr:vendor_driver_metadata\": {}\
		}\
	]\
}";

std::string getVolumeResponse = "{\
    \"volume\": {\
		\"attachments\": [{\
			\"server_id\": \"85c80531-a24c-4732-9f45-228c8301478f\",\
			\"attachment_id\": \"6b7a281e-dcde-47d5-8d4a-6a4e9748111f\",\
			\"attached_at\": \"2022-11-29T02:26:39.847005\",\
			\"host_name\": null,\
			\"volume_id\": \"787c2a44-d8f2-41de-ba04-a70fce8201bd\",\
			\"device\": \"/dev/vda\",\
			\"id\": \"787c2a44-d8f2-41de-ba04-a70fce8201bd\"\
		}],\
		\"availability_zone\": \"az236.dc236\",\
		\"os-vol-host-attr:host\": \"cinder@FusionSphereOpenstack#FusionSphereOpenstack-ma\",\
		\"encrypted\": false,\
		\"updated_at\": \"2022-11-29T02:26:39.874180\",\
		\"os-volume-replication:extended_status\": null,\
		\"replication_status\": \"disabled\",\
		\"snapshot_id\": null,\
		\"wwn\": \"658f987100b749bc4811cd7700001b24\",\
		\"id\": \"787c2a44-d8f2-41de-ba04-a70fce8201bd\",\
		\"size\": 40,\
		\"user_id\": \"278c058da02b44d484dc8b99a77063de\",\
		\"os-vol-tenant-attr:tenant_id\": \"28a23331b60747bfb2bd8606ba930396\",\
		\"os-vol-mig-status-attr:migstat\": null,\
		\"metadata\": {\
			\"StorageType\": \"OceanStorV5\",\
			\"take_over_lun_wwn\": \"--\",\
			\"attached_mode\": \"rw\",\
			\"readonly\": \"False\",\
			\"lun_wwn\": \"658f987100b749bc4811cd7700001b24\"\
		},\
		\"status\": \"in-use\",\
		\"volume_image_metadata\": {\
			\"architecture\": \"x86_64\",\
			\"container_format\": \"bare\",\
			\"hw_disk_bus\": \"virtio\"\
		},\
		\"backup_id\": null,\
		\"description\": \"\",\
		\"multiattach\": false,\
		\"os-volume-replication:driver_data\": \"{\\\"lun_id\\\": \\\"6948\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\",\
		\"source_volid\": null,\
		\"consistencygroup_id\": null,\
		\"os-vol-pro-location-attr:provider_location\": \"6948\",\
		\"os-vol-mig-status-attr:name_id\": null,\
		\"name\": \"\",\
		\"bootable\": \"true\",\
		\"created_at\": \"2022-11-29T02:23:25.771826\",\
		\"volume_type\": null,\
		\"shareable\": false,\
		\"os-vendor-ext-attr:vendor_driver_metadata\": {}\
	}\
}";

std::string getServerDeatilNotFindReponse = "{\
    \"itemNotFound\": {\
        \"message\": \"Instance 1564 could not be found.\",\
        \"code\": 404\
    }\
}";

std::string createVolumeResponse = "{\
    \"volume\": {\
        \"id\": \"908ec073-c515-4803-8f11-7e55ad8c7a25\",\
        \"status\": \"creating\",\
        \"size\": 10,\
        \"availability_zone\": \"az129.dc174\",\
        \"created_at\": \"2023-10-12T14:42:24.064699\",\
        \"updated_at\": null,\
        \"attachments\": [],\
        \"name\": \"az_test_sys1\",\
        \"description\": null,\
        \"volume_type\": \"__DEFAULT__\",\
        \"snapshot_id\": null,\
        \"source_volid\": null,\
        \"metadata\": {},\
        \"links\": [\
            {\
                \"rel\": \"self\",\
                \"href\": \"https://volume.az129.dc174.huawei.com/v2/a99c1de20c0a4181a446a39179ad1dbd/volumes/908ec073-c515-4803-8f11-7e55ad8c7a25\"\
            },\
            {\
                \"rel\": \"bookmark\",\
                \"href\": \"https://volume.az129.dc174.huawei.com/a99c1de20c0a4181a446a39179ad1dbd/volumes/908ec073-c515-4803-8f11-7e55ad8c7a25\"\
            }\
        ],\
        \"user_id\": \"7f74b3cf7c6b473b94351ae616503b87\",\
        \"bootable\": \"false\",\
        \"encrypted\": false,\
        \"replication_status\": null,\
        \"consistencygroup_id\": null,\
        \"multiattach\": false,\
        \"migration_status\": null,\
        \"shareable\": false\
    }\
}";


std::string getVolumeAvailableResponse = "{\
    \"volume\": {\
		\"availability_zone\": \"az236.dc236\",\
		\"os-vol-host-attr:host\": \"cinder@FusionSphereOpenstack#FusionSphereOpenstack-ma\",\
		\"encrypted\": false,\
		\"updated_at\": \"2022-11-29T02:26:39.874180\",\
		\"os-volume-replication:extended_status\": null,\
		\"replication_status\": \"disabled\",\
		\"snapshot_id\": null,\
		\"wwn\": \"658f987100b749bc4811cd7700001b24\",\
		\"id\": \"787c2a44-d8f2-41de-ba04-a70fce8201bd\",\
		\"size\": 40,\
		\"user_id\": \"278c058da02b44d484dc8b99a77063de\",\
		\"os-vol-tenant-attr:tenant_id\": \"28a23331b60747bfb2bd8606ba930396\",\
		\"os-vol-mig-status-attr:migstat\": null,\
		\"metadata\": {\
			\"StorageType\": \"OceanStorV5\",\
			\"take_over_lun_wwn\": \"--\",\
			\"attached_mode\": \"rw\",\
			\"readonly\": \"False\",\
			\"lun_wwn\": \"658f987100b749bc4811cd7700001b24\"\
		},\
		\"status\": \"available\",\
		\"volume_image_metadata\": {\
			\"architecture\": \"x86_64\",\
			\"container_format\": \"bare\",\
			\"hw_disk_bus\": \"virtio\"\
		},\
		\"backup_id\": null,\
		\"description\": \"\",\
		\"multiattach\": false,\
		\"os-volume-replication:driver_data\": \"{\\\"lun_id\\\": \\\"6948\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\",\
		\"source_volid\": null,\
		\"consistencygroup_id\": null,\
		\"os-vol-pro-location-attr:provider_location\": \"6948\",\
		\"os-vol-mig-status-attr:name_id\": null,\
		\"name\": \"\",\
		\"bootable\": \"true\",\
		\"created_at\": \"2022-11-29T02:23:25.771826\",\
		\"volume_type\": null,\
		\"shareable\": false,\
		\"os-vendor-ext-attr:vendor_driver_metadata\": {}\
	}\
}";

std::string getDomainsResponse = "{\
	\"domains\": [{\
		\"links\": {\
			\"self\": \"https://identity.az236.dc236.huawei.com/identity/v3/domains/default\"\
		},\
		\"description\": \"\",\
		\"name\": \"Default\",\
		\"enabled\": true,\
		\"id\": \"default\"\
	}],\
	\"links\": {\
		\"self\": \"https://identity.az236.dc236.huawei.com/identity/v3/domains?name=Default\",\
		\"previous\": null,\
		\"next\": null\
	}\
}";

std::string getNetworksResponse = "{\
	\"networks\": [{\
		\"provider:physical_network\": \"_192_168\",\
		\"ipv6_address_scope\": null,\
		\"port_security_enabled\": true,\
		\"provider:network_type\": \"vlan\",\
		\"id\": \"097d1095-d37d-4b86-ad4a-3e0c4b0f5875\",\
		\"router:external\": true,\
		\"availability_zone_hints\": [],\
		\"availability_zones\": [\
			\"nova\"\
		],\
		\"provider:segmentation_id\": 1,\
		\"net_scope\": null,\
		\"ipv4_address_scope\": null,\
		\"shared\": true,\
		\"project_id\": \"44a5d00ae2124633be265f9f249e8360\",\
		\"status\": \"ACTIVE\",\
		\"subnets\": [\
			\"4471e17e-553b-4099-9651-5ca96c53486b\"\
		],\
		\"description\": \"\",\
		\"tags\": [],\
		\"updated_at\": \"2023-02-09T16:35:45\",\
		\"is_default\": false,\
		\"qos_policy_id\": null,\
		\"name\": \"_192_168\",\
		\"admin_state_up\": true,\
		\"tenant_id\": \"44a5d00ae2124633be265f9f249e8360\",\
		\"created_at\": \"2023-02-09T16:35:45\",\
		\"mtu\": 1500,\
		\"vlan_transparent\": false\
	}]\
}";

std::string getVolumeTypesResponse = "{\
	\"volume_types\": [{\
			\"name\": \"disk_redhat\",\
			\"qos_specs_id\": null,\
			\"extra_specs\": {\
				\"HW:compression\": \"True\",\
				\"volume_backend_name\": \"StoragePool001\",\
				\"drivers:LUNType\": \"Thin\",\
				\"HW:deduplication\": \"True\",\
				\"HW:availability_zone\": \"az10279.dc10279\"\
			},\
			\"os-volume-type-access:is_public\": true,\
			\"is_public\": true,\
			\"id\": \"3e49c8f8-5b15-4ef5-817b-9cf4c2f9f889\",\
			\"description\": null\
		},\
		{\
			\"name\": \"disk_centos\",\
			\"qos_specs_id\": null,\
			\"extra_specs\": {\
				\"HW:compression\": \"\",\
				\"volume_backend_name\": \"StoragePool001\",\
				\"drivers:LUNType\": \"Thin\",\
				\"HW:deduplication\": \"\",\
				\"HW:availability_zone\": \"az10279.dc10279\"\
			},\
			\"os-volume-type-access:is_public\": true,\
			\"is_public\": true,\
			\"id\": \"f747b0c4-4ad6-481a-8779-4de0b48897a9\",\
			\"description\": null\
		},\
		{\
			\"name\": \"omVolumeType0\",\
			\"qos_specs_id\": null,\
			\"extra_specs\": {\
				\"volume_backend_name\": \"StoragePool001\",\
				\"HW:availability_zone\": \"az10279.dc10279\"\
			},\
			\"os-volume-type-access:is_public\": true,\
			\"is_public\": true,\
			\"id\": \"f0bd4281-df06-4704-bcb2-02d0dea5bdcd\",\
			\"description\": null\
		},\
		{\
			\"name\": \"disk\",\
			\"qos_specs_id\": null,\
			\"extra_specs\": {\
				\"volume_backend_name\": \"StoragePool001\",\
				\"drivers:LUNType\": \"NONE\",\
				\"HW:availability_zone\": \"az10279.dc10279\"\
			},\
			\"os-volume-type-access:is_public\": true,\
			\"is_public\": true,\
			\"id\": \"9e2a01a9-26c3-442b-8d54-3800742db7aa\",\
			\"description\": null\
		}\
	]\
}";

std::string getAZsResponse ="{\
    \"availabilityZoneInfo\": [\
        {\
            \"zoneName\": \"internal\",\
            \"zoneState\": {\
                \"available\": true\
            },\
            \"hosts\": {\
                \"54E7338C-6BF4-6CB2-EC11-846ECA42B43A\": {\
                    \"nova-console\": {\
                        \"available\": true,\
                        \"active\": true,\
                        \"updated_at\": \"2023-12-04T11:49:01.672998\"\
                    },\
                    \"nova-scheduler\": {\
                        \"available\": true,\
                        \"active\": true,\
                        \"updated_at\": \"2023-12-04T11:49:03.199227\"\
                    },\
                    \"nova-conductor\": {\
                        \"available\": true,\
                        \"active\": true,\
                        \"updated_at\": \"2023-12-04T11:49:18.044240\"\
                    }\
                },\
                \"277A0F8E-2A8C-4091-EC11-19593AAF0637\": {\
                    \"nova-scheduler\": {\
                        \"available\": true,\
                        \"active\": true,\
                        \"updated_at\": \"2023-12-04T11:49:00.001061\"\
                    },\
                    \"nova-conductor\": {\
                        \"available\": true,\
                        \"active\": true,\
                        \"updated_at\": \"2023-12-04T11:49:09.707874\"\
                    },\
                    \"nova-console\": {\
                        \"available\": true,\
                        \"active\": true,\
                        \"updated_at\": \"2023-12-04T11:49:16.258216\"\
                    }\
                },\
                \"368D0F8E-2A8C-1BA5-EC11-E90B0487984E\": {\
                    \"nova-console\": {\
                        \"available\": true,\
                        \"active\": true,\
                        \"updated_at\": \"2023-12-04T11:49:09.755334\"\
                    },\
                    \"nova-scheduler\": {\
                        \"available\": true,\
                        \"active\": true,\
                        \"updated_at\": \"2023-12-04T11:49:18.322257\"\
                    },\
                    \"nova-conductor\": {\
                        \"available\": true,\
                        \"active\": true,\
                        \"updated_at\": \"2023-12-04T11:49:20.063633\"\
                    }\
                }\
            }\
        },\
        {\
            \"zoneName\": \"az129.dc174\",\
            \"zoneState\": {\
                \"available\": true\
            },\
            \"hosts\": {\
                \"54E7338C-6BF4-6CB2-EC11-846ECA42B43A\": {\
                    \"nova-compute\": {\
                        \"available\": true,\
                        \"active\": true,\
                        \"updated_at\": \"2023-12-04T11:48:55.187729\"\
                    }\
                },\
                \"4CAD9F61-8901-E711-A49B-4CF95D24B4F4\": {\
                    \"nova-compute\": {\
                        \"available\": true,\
                        \"active\": true,\
                        \"updated_at\": \"2023-12-04T11:49:19.570782\"\
                    }\
                },\
                \"277A0F8E-2A8C-4091-EC11-19593AAF0637\": {\
                    \"nova-compute\": {\
                        \"available\": true,\
                        \"active\": true,\
                        \"updated_at\": \"2023-12-04T11:49:01.285400\"\
                    }\
                },\
                \"368D0F8E-2A8C-1BA5-EC11-E90B0487984E\": {\
                    \"nova-compute\": {\
                        \"available\": true,\
                        \"active\": true,\
                        \"updated_at\": \"2023-12-04T11:49:19.302984\"\
                    }\
                }\
            }\
        }\
    ]\
}";

std::string getFlavorsResponse = "{\
	\"flavors\": [{\
		\"name\": \"1cpu100G\",\
		\"links\": [{\
				\"href\": \"https://compute.az10279.dc10279.huawei.com/v2.1/b417f7c49a754897a2f9c91eb21f9728/flavors/aad7a130-537d-47ed-871f-2c8959ecf52d\",\
				\"rel\": \"self\"\
			},\
			{\
				\"href\": \"https://compute.az10279.dc10279.huawei.com/b417f7c49a754897a2f9c91eb21f9728/flavors/aad7a130-537d-47ed-871f-2c8959ecf52d\",\
				\"rel\": \"bookmark\"\
			}\
		],\
		\"ram\": 6144,\
		\"OS-FLV-DISABLED:disabled\": false,\
		\"vcpus\": 1,\
		\"swap\": \"\",\
		\"os-flavor-access:is_public\": true,\
		\"rxtx_factor\": 1.0,\
		\"OS-FLV-EXT-DATA:ephemeral\": 0,\
		\"disk\": 0,\
		\"id\": \"aad7a130-537d-47ed-871f-2c8959ecf52d\"\
	}]\
}";

std::string getServerShutOffResponse = "{\
    \"server\": {\
        \"id\": \"65c57111-c00a-467c-b4e7-696399c7c39e\",\
        \"name\": \"dcc-test02\",\
        \"status\": \"SHUTOFF\",\
        \"tenant_id\": \"a99c1de20c0a4181a446a39179ad1dbd\",\
        \"user_id\": \"7f74b3cf7c6b473b94351ae616503b87\",\
        \"metadata\": {},\
        \"hostId\": \"ca9f08483085a94531f43caebe297f4736b043c45f6e4ab744d9504d\",\
        \"image\": \"\",\
        \"flavor\": {\
            \"id\": \"1be9a169-d864-48b0-b1ad-f08b45ec4a72\",\
            \"links\": [\
                {\
                    \"rel\": \"bookmark\",\
                    \"href\": \"https://compute.az129.dc174.huawei.com/a99c1de20c0a4181a446a39179ad1dbd/flavors/1be9a169-d864-48b0-b1ad-f08b45ec4a72\"\
                }\
            ]\
        },\
        \"created\": \"2023-11-09T09:18:36Z\",\
        \"updated\": \"2023-11-09T09:22:29Z\",\
        \"addresses\": {\
            \"_192_168\": [\
                {\
                    \"version\": 4,\
                    \"addr\": \"192.168.0.188\",\
                    \"OS-EXT-IPS:type\": \"fixed\",\
                    \"OS-EXT-IPS-MAC:mac_addr\": \"fa:16:3e:9e:a3:b0\"\
                }\
            ],\
            \"external_api\": [\
                {\
                    \"version\": 4,\
                    \"addr\": \"8.40.137.40\",\
                    \"OS-EXT-IPS:type\": \"fixed\",\
                    \"OS-EXT-IPS-MAC:mac_addr\": \"fa:16:3e:f0:ac:e5\"\
                }\
            ]\
        },\
        \"accessIPv4\": \"\",\
        \"accessIPv6\": \"\",\
        \"links\": [\
            {\
                \"rel\": \"self\",\
                \"href\": \"https://compute.az129.dc174.huawei.com/v2.1/a99c1de20c0a4181a446a39179ad1dbd/servers/65c57111-c00a-467c-b4e7-696399c7c39e\"\
            },\
            {\
                \"rel\": \"bookmark\",\
                \"href\": \"https://compute.az129.dc174.huawei.com/a99c1de20c0a4181a446a39179ad1dbd/servers/65c57111-c00a-467c-b4e7-696399c7c39e\"\
            }\
        ],\
        \"OS-DCF:diskConfig\": \"MANUAL\",\
        \"OS-EXT-AZ:availability_zone\": \"az129.dc174\",\
        \"config_drive\": \"\",\
        \"key_name\": null,\
        \"OS-SRV-USG:launched_at\": \"2023-11-09T09:18:45.000000\",\
        \"OS-SRV-USG:terminated_at\": null,\
        \"security_groups\": [\
            {\
                \"name\": \"default\"\
            },\
            {\
                \"name\": \"default\"\
            }\
        ],\
        \"OS-EXT-SRV-ATTR:host\": \"277A0F8E-2A8C-4091-EC11-19593AAF0637\",\
        \"OS-EXT-SRV-ATTR:instance_name\": \"instance-00000126\",\
        \"OS-EXT-SRV-ATTR:hypervisor_hostname\": \"277A0F8E-2A8C-4091-EC11-19593AAF0637\",\
        \"OS-EXT-STS:task_state\": null,\
        \"OS-EXT-STS:vm_state\": \"stopped\",\
        \"OS-EXT-STS:power_state\": 4,\
        \"os-extended-volumes:volumes_attached\": [\
            {\
                \"id\": \"27a91b3b-4054-4ad2-a71e-9ba15b893fec\"\
            }\
        ]\
    }\
}";

std::string g_getServicesResponse = "{\
    \"services\": [{\
    \"name\": \"keystonev3\",\
    \"links\": {\"self\": \"https://identity.az236.dc236.huawei.com/identity/v3/services/55be894213894090833dbabd7c679ca3\"},\
    \"enabled\": true,\
    \"type\": \"identityv3\",\
    \"id\": \"55be894213894090833dbabd7c679ca3\",\
    \"description\": \"Keystone Identity Service V3\"}]\
}";

std::string appExtendInfo = "{\
	\"description\": \"\",\
	\"domainName\": \"Default\",\
	\"enabled\": true,\
	\"domainId\": \"default\"\
}";

std::string appExtendHasDomain = "{\
	\"description\": \"\",\
	\"domainName\": \"Default\",\
	\"enabled\": true,\
	\"domainId\": \"default\",\
    \"isDomain\": \"true\"\
}";

std::string appExtendHashealthCheck = "{\
	\"description\": \"\",\
	\"domainName\": \"Default\",\
	\"enabled\": true,\
	\"domainId\": \"default\",\
    \"healthCheck\": \"true\"\
}";

std::string appExtendHashealthCheckOfDomain = "{\
	\"description\": \"\",\
	\"domainName\": \"Default\",\
	\"enabled\": true,\
	\"domainId\": \"default\",\
    \"healthCheck\": \"true\",\
    \"isDomain\": \"true\"\
}";

const std::string g_volumeInfoStr = "{\
    \"extendInfo\": \"\",\
    \"metaData\": \"\",\
    \"moRef\": \"c69a9760-cb4d-44b1-b2fe-d5ab6ba19a88\",\
    \"targetFolderLocation\": \"0e2d4d8215b35d8b4eb632e9841d3fc9d1a3208749a15f34abb30b12\",\
    \"volList\": [{\
        \"datastore\": {\
            \"dcMoRef\": \"\",\
            \"extendInfo\": \"{\\\"volId\\\":\\\"323\\\",\\\"volName\\\":\\\"baolong-volume-3ac7-scsi\\\",\\\"volWwn\\\":\\\"658f987100b749bc2d53e46c00000143\\\"}\",\
            \"ip\": \"\",\
            \"moRef\": \"2102351NPT10J3000001\",\
            \"name\": \"\",\
            \"poolId\": \"\",\
            \"port\": \"\",\
            \"type\": \"OceanStorV5\"\
        },\
        \"extendInfo\": \"\",\
        \"metadata\": \"\",\
        \"moRef\": \"256\",\
        \"name\": \"volume-ec68\",\
        \"slotId\": \"\",\
        \"type\": \"normal\",\
        \"uuid\": \"c1f1465b-6a63-4343-9761-7ad3e413f4ef\",\
        \"vmMoRef\": \"c69a9760-cb4d-44b1-b2fe-d5ab6ba19a88\",\
        \"volSizeInBytes\": 10737418240\
    }]\
}";

const std::string g_volumeSnapDetailStr = "{\"snapshot\":{\"status\":\"available\",\"size\":10,\"metadata\":{},\"name\":\"VirtPlugin_ecs-4d45-0001-volume-0000_26B49564-E4CE-4330-AAD6-D2C42AF96CD3\",\"volume_id\":\"c1f1465b-6a63-4343-9761-7ad3e413f4ef\", \"provider_auth\": \"{\\\"lun_id\\\": \\\"6604\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\", \"created_at\":\"2022-07-21T07:00:21.862040\",\"description\":\"VirtPlugin backup. Job 3dd2c7fd-adbe-a1a1-5a36-0e2e7695c967.\",\"updated_at\":null}}";
const std::string g_volumeSnapDetailStrCreating = "{\"snapshot\":{\"status\":\"creating\",\"size\":10,\"metadata\":{},\"name\":\"VirtPlugin_ecs-4d45-0001-volume-0000_26B49564-E4CE-4330-AAD6-D2C42AF96CD3\",\"volume_id\":\"c1f1465b-6a63-4343-9761-7ad3e413f4ef\",\"provider_auth\": \"{\\\"lun_id\\\": \\\"6604\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\", \"created_at\":\"2022-07-21T07:00:21.862040\",\"description\":\"VirtPlugin backup. Job 3dd2c7fd-adbe-a1a1-5a36-0e2e7695c967.\",\"updated_at\":null}}";
const std::string g_volumeSnapDetailStrDeleting = "{\"snapshot\":{\"status\":\"deleting\",\"size\":10,\"metadata\":{},\"name\":\"VirtPlugin_ecs-4d45-0001-volume-0000_26B49564-E4CE-4330-AAD6-D2C42AF96CD3\",\"volume_id\":\"c1f1465b-6a63-4343-9761-7ad3e413f4ef\",\"provider_auth\": \"{\\\"lun_id\\\": \\\"6604\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\", \"created_at\":\"2022-07-21T07:00:21.862040\",\"description\":\"VirtPlugin backup. Job 3dd2c7fd-adbe-a1a1-5a36-0e2e7695c967.\",\"updated_at\":null}}";
const std::string g_volumeSnapDetailStrError = "{\"snapshot\":{\"status\":\"error\",\"size\":10,\"metadata\":{},\"name\":\"VirtPlugin_ecs-4d45-0001-volume-0000_26B49564-E4CE-4330-AAD6-D2C42AF96CD3\",\"volume_id\":\"c1f1465b-6a63-4343-9761-7ad3e413f4ef\",\"provider_auth\": \"{\\\"lun_id\\\": \\\"6604\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\", \"created_at\":\"2022-07-21T07:00:21.862040\",\"description\":\"VirtPlugin backup. Job 3dd2c7fd-adbe-a1a1-5a36-0e2e7695c967.\",\"updated_at\":null}}";

static const std::string t_volExtendInfo =
    "{\"targetVolume\":\"{\\\"uuid\\\":\\\"53f52bff-8ffa-46ae-98b4-a1f20aa99a7a\\\"}\"}";
int32_t g_httpSendCount = 0;


bool StubSetBckupInfo(AppProtect::BackupJob &backupInfo)
{
    backupInfo.requestId = "123";
    backupInfo.jobId = "123";
    backupInfo.protectEnv.__set_id("136");
    backupInfo.protectEnv.__set_name("OpenStackPlugin");
    backupInfo.protectEnv.__set_type("Virtual");
    backupInfo.protectEnv.__set_endpoint("https://10.9.6.2:443/v2/13f648f23eac4f3abc870eef7f41bc56");
    backupInfo.protectEnv.auth.__set_authkey("admin");
    backupInfo.protectEnv.auth.__set_authPwd("xxxxxxxx");
	backupInfo.protectEnv.auth.__set_extendInfo(
		"{\"certification\":\"\",\"enableCert\":\"0\",\"revocationlist\":\"\"}");
    backupInfo.protectEnv.__set_extendInfo(
        "{\"projectId\":\"projectId\",\"domainName\":\"domains\"}");
    backupInfo.protectObject.__set_extendInfo("{\"projectId\":\"projectId\",\"domainName\":\"domains\"}");
}

bool StubSetBckupExtendInfo(AppProtect::BackupJob &backupInfo)
{
    backupInfo.requestId = "123";
    backupInfo.jobId = "123";
    backupInfo.__set_extendInfo(
        "{\"projectId\":\"projectId\",\"domainName\":\"domains\",\"open_consistent_snapshots\":\"true\"}");
    backupInfo.protectEnv.__set_id("136");
    backupInfo.protectEnv.__set_name("OpenStackPlugin");
    backupInfo.protectEnv.__set_type("Virtual");
    backupInfo.protectEnv.__set_endpoint("https://10.9.6.2:443/v2/13f648f23eac4f3abc870eef7f41bc56");
    backupInfo.protectEnv.auth.__set_authkey("admin");
    backupInfo.protectEnv.auth.__set_authPwd("xxxxxxxx");
	backupInfo.protectEnv.auth.__set_extendInfo(
		"{\"certification\":\"\",\"enableCert\":\"0\",\"revocationlist\":\"\"}");
    backupInfo.protectEnv.__set_extendInfo(
        "{\"projectId\":\"projectId\",\"domainName\":\"domains\"}");
    backupInfo.protectObject.__set_extendInfo("{\"projectId\":\"projectId\",\"domainName\":\"domains\"}");
}

bool StubSetTagertBckupInfo(AppProtect::BackupJob &backupInfo)
{
    backupInfo.requestId = "123";
    backupInfo.jobId = "123";
    backupInfo.protectEnv.__set_id("136");
    backupInfo.protectEnv.__set_name("OpenStackPlugin");
    backupInfo.protectEnv.__set_type("Virtual");
    backupInfo.protectEnv.__set_endpoint("https://10.9.6.2:443/v2/13f648f23eac4f3abc870eef7f41bc56");
    backupInfo.protectEnv.auth.__set_authkey("admin");
    backupInfo.protectEnv.auth.__set_authPwd("xxxxxxxx");
	backupInfo.protectEnv.auth.__set_extendInfo(
		"{\"certification\":\"\",\"enableCert\":\"0\",\"revocationlist\":\"\"}");
    backupInfo.protectEnv.__set_extendInfo(
        "{\"projectId\":\"projectId\",\"domainName\":\"domains\"}");
    ApplicationResource backupVol;
    backupVol.type = "volume";
    backupVol.id = "1-1-1-1";                                                  // 源卷
    backupInfo.protectSubObject = {backupVol};
}

bool StubSetTagertBckupExtendInfo(AppProtect::BackupJob &backupInfo)
{
    backupInfo.requestId = "123";
    backupInfo.jobId = "123";
    backupInfo.__set_extendInfo(
        "{\"projectId\":\"projectId\",\"domainName\":\"domains\",\"open_consistent_snapshots\":\"true\"}");
    backupInfo.protectEnv.__set_id("136");
    backupInfo.protectEnv.__set_name("OpenStackPlugin");
    backupInfo.protectEnv.__set_type("Virtual");
    backupInfo.protectEnv.__set_endpoint("https://10.9.6.2:443/v2/13f648f23eac4f3abc870eef7f41bc56");
    backupInfo.protectEnv.auth.__set_authkey("admin");
    backupInfo.protectEnv.auth.__set_authPwd("xxxxxxxx");
	backupInfo.protectEnv.auth.__set_extendInfo(
		"{\"certification\":\"\",\"enableCert\":\"0\",\"revocationlist\":\"\"}");
    backupInfo.protectEnv.__set_extendInfo(
        "{\"projectId\":\"projectId\",\"domainName\":\"domains\"}");
    ApplicationResource backupVol;
    backupVol.type = "volume";
    backupVol.id = "1-1-1-1";                                                  // 源卷
    backupInfo.protectSubObject = {backupVol};
}


void StubSetRestoreInfo(AppProtect::RestoreJob &restoreInfo)
{
    restoreInfo.requestId = "123";
    restoreInfo.jobId = "123";
    restoreInfo.targetEnv.__set_id("136");
    restoreInfo.targetEnv.__set_name("HcsPlanet");
    restoreInfo.targetEnv.__set_type("Virtual");
    restoreInfo.targetEnv.__set_endpoint("demo.com");
    restoreInfo.targetEnv.auth.__set_authkey("bss_admin");
    restoreInfo.targetEnv.auth.__set_authPwd("xxxxxxxx");
    restoreInfo.targetEnv.auth.__set_extendInfo(
        "{\"certification\":\"cert\",\"enableCert\":\"1\",\"revocationlist\":\"\"}");
    restoreInfo.targetEnv.__set_extendInfo(
        "{\"projectId\":\"e38d227edcce4631be20bfa5aad7130b\",\"regionId\":\"sc-cd-1\"}");
    // 卷参数
    ApplicationResource restoreVol;
    restoreVol.type = "volume";
    restoreVol.id = "1-1-1-1";                                                  // 源卷
    restoreVol.__set_extendInfo(t_volExtendInfo);  // 目标卷信息
    restoreInfo.restoreSubObjects = {restoreVol};
    restoreInfo.targetObject.__set_id("10a4e361-c981-46f2-b9ba-d7ff9c601693");
    restoreInfo.targetObject.__set_extendInfo("{\"projectId\":\"projectId\",\"domainName\":\"domains\"}");
    restoreInfo.__set_extendInfo("{\"restoreLevel\":\"1\"}");
    AppProtect::Copy copyInfo;
    AppProtect::StorageRepository repo;
    repo.__set_repositoryType(RepositoryDataType::META_REPOSITORY);
    repo.path.push_back("/tmp/");
    copyInfo.repositories.push_back(repo);
    repo.__set_repositoryType(RepositoryDataType::CACHE_REPOSITORY);
    copyInfo.repositories.push_back(repo);
    restoreInfo.copies.push_back(copyInfo);
}

void StubSetVMRestoreInfo(AppProtect::RestoreJob &restoreInfo)
{
    restoreInfo.requestId = "123";
    restoreInfo.jobId = "123";
    restoreInfo.targetEnv.__set_id("136");
    restoreInfo.targetEnv.__set_name("Openstack");
    restoreInfo.targetEnv.__set_type("Virtual");
    restoreInfo.targetEnv.__set_endpoint("demo.com");
    restoreInfo.targetEnv.auth.__set_authkey("cloud_admin");
    restoreInfo.targetEnv.auth.__set_authPwd("xxxxxxxx");
    restoreInfo.targetEnv.auth.__set_extendInfo(
        "{\"certification\":\"cert\",\"enableCert\":\"1\",\"revocationlist\":\"\"}");
    restoreInfo.targetEnv.__set_extendInfo(
        "{\"projectId\":\"e38d227edcce4631be20bfa5aad7130b\",\"regionId\":\"sc-cd-1\"}");
    // 卷参数
    ApplicationResource restoreVol;
    restoreVol.type = "volume";
    restoreVol.id = "1-1-1-1";                                                  // 源卷
    restoreVol.__set_extendInfo(t_volExtendInfo);  // 目标卷信息
    restoreInfo.restoreSubObjects = {restoreVol};
    restoreInfo.targetObject.__set_id("10a4e361-c981-46f2-b9ba-d7ff9c601693");
    restoreInfo.targetObject.__set_extendInfo("{\"projectId\":\"projectId\",\"domainName\":\"domains\"}");
    restoreInfo.__set_extendInfo("{\"restoreLevel\":\"0\"}");
    AppProtect::Copy copyInfo;
    AppProtect::StorageRepository repo;
    copyInfo.repositories.push_back(repo);
    restoreInfo.copies.push_back(copyInfo);

}

void StubSetVMRestoreInfoRecoverIp(AppProtect::RestoreJob &restoreInfo)
{
    restoreInfo.requestId = "123";
    restoreInfo.jobId = "123";
    restoreInfo.targetEnv.__set_id("136");
    restoreInfo.targetEnv.__set_name("Openstack");
    restoreInfo.targetEnv.__set_type("Virtual");
    restoreInfo.targetEnv.__set_endpoint("demo.com");
    restoreInfo.targetEnv.auth.__set_authkey("cloud_admin");
    restoreInfo.targetEnv.auth.__set_authPwd("xxxxxxxx");
    restoreInfo.targetEnv.auth.__set_extendInfo(
            "{\"certification\":\"cert\",\"enableCert\":\"1\",\"revocationlist\":\"\"}");
    restoreInfo.targetEnv.__set_extendInfo(
            "{\"projectId\":\"e38d227edcce4631be20bfa5aad7130b\",\"regionId\":\"sc-cd-1\"}");
    // 卷参数
    ApplicationResource restoreVol;
    restoreVol.type = "volume";
    restoreVol.id = "1-1-1-1";                                                  // 源卷
    restoreVol.__set_extendInfo(t_volExtendInfo);  // 目标卷信息
    restoreInfo.restoreSubObjects = {restoreVol};
    restoreInfo.targetObject.__set_id("10a4e361-c981-46f2-b9ba-d7ff9c601693");
    Json::Value extendBody;
    extendBody["projectId"] = "projectId";
    extendBody["domainName"] = "domains";
    extendBody["flavor"] = "{\"id\":\"1024\"}";
    extendBody["network"] = "[{\"id\":\"network1\",\"ip\":\"1.2.3.4\"},{\"id\":\"network2\",\"ip\":\"\"},{\"id\":\"network3\"}]";
    extendBody["availabilityZone"] = "{\"name\":\"az1\"}";
    Json::FastWriter fastWriter;
    std::string extendStr =  fastWriter.write(extendBody);
    restoreInfo.targetObject.__set_extendInfo(extendStr);
    restoreInfo.__set_extendInfo("{\"restoreLevel\":\"0\"}");
    AppProtect::Copy copyInfo;
    AppProtect::StorageRepository repo;
    copyInfo.repositories.push_back(repo);
    restoreInfo.copies.push_back(copyInfo);
}

static bool Stub_SaveCertToFile(const std::string& fileName)
{
    return true;
}

static std::string GetVolumeListFromFile()
{
    NewCreatedVolumeList volList;
    Volume voltest1;
    voltest1.m_id = "1-1-1-1";
    volList.m_volumelist.push_back(voltest1);
    std::string volListStr;
    Module::JsonHelper::StructToJsonString(volList, volListStr);
    return volListStr;
}

static int32_t ReadVolumeListStub(std::string &buf, size_t size)
{
    std::string return_value = GetVolumeListFromFile();
    buf = return_value;
    return size;
}

static int32_t WriteFileSuccessStub(const std::string &content)
{
    return content.length();
}
/* Stub for FileSystemHandler */
static std::shared_ptr<RepositoryHandler> CreateFSHandlerStub(
    const AppProtect::StorageRepository &storageRepo)
{
    static std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(GetVolumeListFromFile().length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(ReadVolumeListStub));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(WriteFileSuccessStub)));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

class OpenStackProtectEngineTest : public testing::Test {
protected:
    void SetUp()
    {
		m_stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest);
		m_stub.set(ADDR(OpenStackPlugin::OpenStackTokenMgr, GetToken), StubGetToken);
        StubSetBckupInfo(m_backupInfo);
		StubSetRestoreInfo(m_restoreInfo);
        InitLogger();
		stubGetHttpStatusPtr = nullptr;
		m_stub.set(sleep, Stub_TimeSleep);
		m_stub.set(ADDR(CertManger, SaveCertToFile), Stub_SaveCertToFile);
        m_app.id = "28a23331b60747bfb2bd8606ba930396"; // domain id
        m_app.name = "default";
        m_app.extendInfo = appExtendInfo;
        g_httpSendCount = 0;
    };
    void TearDown()
    {
        m_stub.reset(ADDR(Module::IHttpClient, GetInstance));
		m_stub.reset(ADDR(OpenStackPlugin::OpenStackTokenMgr, GetToken));
        m_stub.reset(sleep);
		m_stub.reset(ADDR(CertManger, SaveCertToFile));
    };
    void InitLogger();
    Stub m_stub;
    OpenStackProtectEngine m_opSEngine;

private:
    AppProtect::BackupJob m_backupInfo;
    AppProtect::RestoreJob m_restoreInfo;
    Application m_app; // application map to domain


};

void OpenStackProtectEngineTest::InitLogger()
{
    std::string logFileName = "virt_plugin_openstack_engine_test.log";
    std::string logFilePath = "/tmp/log/";
    int logLevel = DEBUG;
    int logFileCount = 10;
    int logFileSize = 30;
    Module::CLogger::GetInstance().Init(
        logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
}

void StubSetAppEnvUseCertInfo(ApplicationEnvironment& appEnv)
{
    appEnv.auth.authkey = "cloud_admin";
	appEnv.auth.authPwd = "Huawei@123";
	appEnv.endpoint = "https://identity.az236.dc236.huawei.com:443/identity/v3";
	appEnv.auth.__set_extendInfo(
        "{\"certification\":\"cert\",\"enableCert\":\"1\",\"revocationlist\":\"\"}");
}

void StubSetAppEnvNoUseCertInfo(ApplicationEnvironment& appEnv)
{
    appEnv.auth.authkey = "cloud_admin";
	appEnv.auth.authPwd = "Huawei@123";
	appEnv.endpoint = "https://identity.az236.dc236.huawei.com:443/identity/v3";
	appEnv.auth.__set_extendInfo(
        "{\"certification\":\"\",\"enableCert\":\"0\",\"revocationlist\":\"\"}");
}

/*
 * 测试用例： 1. 调用OpenStackProtectEngine ListApplicationResourceV2接口获取domain列表成功
 * 前置条件:  调用rest获取doamin列表成功
 * CHECK点: 
 */
TEST_F(OpenStackProtectEngineTest, ListDomains_SUCC)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"domain\"}";
	request.applications.push_back(m_app);
	g_httpResponsebody = getDomainsResponse;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 1);
}

/*
 * 测试用例： 1. 调用OpenStackProtectEngine ListApplicationResourceV2接口获取项目列表成功
 * 前置条件:  调用rest获取项目列表成功
 * CHECK点: 
 */
TEST_F(OpenStackProtectEngineTest, ListProject_SUCC)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"project\"}";
	request.applications.push_back(m_app);
	g_httpResponsebody = getProjectsResponse;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 3);
}

/*
 * 测试用例： 1. 调用OpenStackProtectEngine ListApplicationResourceV2接口获取项目列表失败 
 * 前置条件:  调用rest获取项目列表失败
 * CHECK点: page.items为空
 */
TEST_F(OpenStackProtectEngineTest, ListProject_FAIL)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"project\"}";
	request.applications.push_back(m_app);
	g_httpStatusCode = 400;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 0);
}

/*
 * 测试用例： 1. 主要验证keystone updateToken函数调用
 * 前置条件:  调用rest获取项目列表返回401，
 * CHECK点: page.items == 3
 */
TEST_F(OpenStackProtectEngineTest, ListProject_SUCC_UPDATE_TOKEN)
{
	stubGetHttpStatusPtr = Customize_GetStatusCode;
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"project\"}";
	request.applications.push_back(m_app);
	g_httpResponsebody = getProjectsResponse;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 3);
}

/*
 * 测试用例： 1. 调用OpenStackProtectEngine ListApplicationResourceV2接口获取项目中云服务器成功
 * 前置条件:  调用rest获取项目云服务器成功
 * CHECK点: 
 */
TEST_F(OpenStackProtectEngineTest, ListProjectServers_SUCC)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"server\"}";
	request.applications.push_back(m_app);
	g_httpStatusCode = 200;
	g_httpResponsebody = getServersListResponseBody;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 2);
}

/*
 * 测试用例： 1. 调用OpenStackProtectEngine ListApplicationResourceV2接口获取项目中云服务器失败 
 * 前置条件:  调用rest获取项目云服务器失败
 * CHECK点: page.items为空
 */
TEST_F(OpenStackProtectEngineTest, ListProjectServers_FAIL)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"server\"}";
	request.applications.push_back(m_app);
	g_httpStatusCode = 401;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 0);
}

/*
 * 测试用例： 1. 调用OpenStackProtectEngine ListApplicationResourceV2接口获取项目中卷成功
 * 前置条件:  调用rest获取项目云服务器成功
 * CHECK点: 
 */
TEST_F(OpenStackProtectEngineTest, ListProjectVolumes_SUCC)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"volume\"}";
	request.applications.push_back(m_app);
	g_httpStatusCode = 200;
	g_httpResponsebody = getProjectVolumesResponse;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 3);
}

/*
 * 测试用例： 1. 调用OpenStackProtectEngine ListApplicationResourceV2接口获取项目中云服务器失败 
 * 前置条件:  调用rest获取项目云服务器失败
 * CHECK点: page.items为空
 */
TEST_F(OpenStackProtectEngineTest, ListProjectVolumes_FAIL)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"volume\"}";
	request.applications.push_back(m_app);
	g_httpStatusCode = 401;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 0);
}

/*
 * 测试用例： Openstack CheckApplication获取keystone v3信息成功 
 * 前置条件:  调用rest获取domain信息成功
 * CHECK点: 返回码为0
 */
TEST_F(OpenStackProtectEngineTest, CheckApplicationSuccess)
{
	ApplicationEnvironment appEnv;
	StubSetAppEnvUseCertInfo(appEnv);
	ActionResult returnValue;
    m_app.extendInfo = appExtendHashealthCheck;

	g_httpStatusCode = 200;
	g_httpResponsebody = g_getServicesResponse;
    m_opSEngine.CheckApplication(returnValue, appEnv, m_app);
	EXPECT_EQ(returnValue.code, 0);
}

/*
 * 测试用例： Openstack CheckApplication检查domain信息成功 
 * 前置条件:  调用rest检查domain信息成功
 * CHECK点: 返回码为0
 */
TEST_F(OpenStackProtectEngineTest, CheckApplicationSuccessOfDomain)
{
	ApplicationEnvironment appEnv;
	StubSetAppEnvUseCertInfo(appEnv);
	ActionResult returnValue;
    m_app.extendInfo = appExtendHashealthCheckOfDomain;

	g_httpStatusCode = Module::SC_CREATED;
    m_opSEngine.CheckApplication(returnValue, appEnv, m_app);
	EXPECT_EQ(returnValue.code, 0);
}

/*
 * 测试用例： Openstack CheckApplication获取service信息失败 
 * 前置条件:  调用rest获取service信息失败
 * CHECK点: 返回码为非0 
 */
TEST_F(OpenStackProtectEngineTest, CheckApplicationFail)
{
	ApplicationEnvironment appEnv;
    m_app.extendInfo = appExtendInfo;

	StubSetAppEnvUseCertInfo(appEnv);
	ActionResult returnValue;

	// 证书失效
	int httpErrCodeBak = g_httpErrCode;
	g_httpStatusCode = 0;
	g_httpErrCode = 42;
    m_opSEngine.CheckApplication(returnValue, appEnv, m_app);
	EXPECT_EQ(returnValue.code, 1677931024);
	g_httpErrCode = httpErrCodeBak;

	// 连接失败或用户名密码验证失败
	g_httpStatusCode = 401;
    m_opSEngine.CheckApplication(returnValue, appEnv, m_app);
	EXPECT_EQ(returnValue.code, 1077949061);

	// 用户被锁定
	g_httpStatusCode = 401;
	g_httpResponsebody = "The account is locked for user admin";
    m_opSEngine.CheckApplication(returnValue, appEnv, m_app);
	EXPECT_EQ(returnValue.code, 1577210067);
}

static int32_t GetDomainDetal_Stub(void* obj)
{
    return SUCCESS;
}

/*
 * 测试用例： Openstack DiscoverAppCluster获取domainservice信息成功 
 * 前置条件:  调用rest获取domain信息成功, 获取service信息成功
 * CHECK点: returnEnv.entendinfo == appExtendInfo
 */
TEST_F(OpenStackProtectEngineTest, DiscoverAppClusterSuccess)
{
	ApplicationEnvironment appEnv;
	StubSetAppEnvNoUseCertInfo(appEnv);
	g_httpStatusCode = 200;
	g_httpResponsebody = g_getServicesResponse;
	ApplicationEnvironment returnEnv;
    m_opSEngine.DiscoverAppCluster(returnEnv, appEnv, m_app);
    std::string result = "{\"cps_ip\":\"\",\"register_service\":\"\","
        "\"service_id\":\"55be894213894090833dbabd7c679ca3\",\"service_name\":\"keystonev3\",\"service_type\":\"identityv3\"}\n";
	EXPECT_EQ(returnEnv.extendInfo, result);
}

/*
 * 测试用例： Openstack DiscoverAppCluster获取domain和service信息失败 
 * 前置条件:  调用rest获取domain信息失败
 * CHECK点: returnEnv.entendinfo为空 
 */
TEST_F(OpenStackProtectEngineTest, DiscoverAppClusterFail)
{
	ApplicationEnvironment appEnv;
	StubSetAppEnvNoUseCertInfo(appEnv);
    // 获取domain失败
	g_httpStatusCode = 400;
	ApplicationEnvironment returnEnv;
    m_opSEngine.DiscoverAppCluster(returnEnv, appEnv, m_app);
	EXPECT_EQ(returnEnv.extendInfo, "");
}

int32_t Stub_UnlockServerRequestSucc(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
    return SUCCESS;
}

int32_t Stub_UnlockServerRequestFail(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetStatusCode(static_cast<uint32_t>(Module::SC_BAD_REQUEST));
    return SUCCESS;
}

/**
 * 测试用例：成功锁定虚拟机
 * 前置条件：锁定虚拟机成功
 * Check点：PreHook返回SUCCESS
 */
TEST_F(OpenStackProtectEngineTest, PreHook_lockVM_SUCC)
{
    m_stub.set(ADDR(HttpClient, Send), Stub_UnlockServerRequestSucc);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    ExecHookParam param;
    param.stage = JobStage::POST_JOB;
    int ret = openstackProtectEngineHandler->PreHook(param);
    EXPECT_EQ(ret, SUCCESS);
}

static int32_t StubGetVolumeDetail(const std::string &volId, Volume &volDetail)
{
    Module::JsonHelper::JsonStringToStruct(getVolumeResponse, volDetail);
    return SUCCESS;
}

static int32_t StubDetachVolumeHandle(const VolInfo &volObj, const std::string& serverId)
{
    return SUCCESS;
}

int32_t Stub_DetachDeleteVolumeSucc(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int count = 0;
    if (count == 0) { // Get volume detail
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        count ++;
    } else if (count == 1) { // GetServerWhichAttachedVolume
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        count ++;
    } else if (count == 2) { // detach volume
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
        count ++;
    } else if (count == 3) { // get if volume detached
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
        count ++;
    } else if (count == 4) { // delete volume
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
        count ++;
    } else if (count == 5) { // confirm if volume deleted
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_NOT_FOUND));
        count ++;
    }
    return SUCCESS;
}

int32_t Stub_AlarmVolumeSucc(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int count = 0;
    if (count == 0) { // Get volume detail
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        count ++;
    }
    return SUCCESS;
}

/**
 * 测试用例：备份OpenStack POST JOB前置删除克隆卷成功
 * 前置条件：删除克隆卷成功
 * Check点：PreHook返回SUCCESS
 */
TEST_F(OpenStackProtectEngineTest, ClearResidualVolumesAlarm_success)
{
    //m_stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), CreateFSHandlerStub);
    m_stub.set(ADDR(HttpClient, Send), Stub_AlarmVolumeSucc);
    AppProtect::StorageRepository repo;
    auto jobHandler = CommonFormJobHandler(m_backupInfo, repo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandler, "123", "");
    ExecHookParam param;
    param.stage = JobStage::POST_JOB;
    openstackProtectEngineHandler->InitRepoHandler();
    EXPECT_EQ(openstackProtectEngineHandler->InitJobPara(), true);
    NewCreatedVolumeList deleteFailVols;
    std::string newVolumeInfoFile = "/tmp//VIRTUAL_PLUGIN_METADATA/snapshot_create_volume.info";
    openstackProtectEngineHandler->DeleteVolumesAction(newVolumeInfoFile, deleteFailVols);
    openstackProtectEngineHandler->SendDeleteVolumeFailedAlarm(deleteFailVols.m_volumelist);
    std::string deleteVolumeInfoFile = "/tmp//VIRTUAL_PLUGIN_METADATA/volumes.tobedeleted";
    openstackProtectEngineHandler->SaveDeleteFailVolumes(deleteVolumeInfoFile, deleteFailVols);
    openstackProtectEngineHandler->ClearVolumesAlarm();
    m_stub.reset(ADDR(HttpClient, Send));
    //m_stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
}

/**
 * 测试用例：备份OpenStack POST JOB前置删除克隆卷成功
 * 前置条件：删除克隆卷成功
 * Check点：PreHook返回SUCCESS
 */
TEST_F(OpenStackProtectEngineTest, PreHook_Delete_CloneVolume_sccess)
{
    m_stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), CreateFSHandlerStub);
    m_stub.set(ADDR(HttpClient, Send), Stub_DetachDeleteVolumeSucc);
    AppProtect::StorageRepository repo;
    auto jobHandler = CommonFormJobHandler(m_backupInfo, repo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandler, "123", "");
    ExecHookParam param;
    param.stage = JobStage::POST_JOB;
    int ret = openstackProtectEngineHandler->PreHook(param);
    EXPECT_EQ(ret, SUCCESS);
    m_stub.reset(ADDR(HttpClient, Send));
    m_stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
}

int32_t Stub_DeleteSnapshotCreateVolumesSucc(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int count = 0;
    if (count == 0) { // Get snapshot volumes
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getProjectVolumesResponse);
        count ++;
    } else if (count == 1) { // GetServerWhichAttachedVolume
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        count ++;
    } else if (count == 2) { // detach volume
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
        count ++;
    } else if (count == 3) { // get if volume detached
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
        count ++;
    } else if (count == 4) { // delete volume
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
        count ++;
    } else if (count >= 5) { // confirm if volume deleted
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_NOT_FOUND));
        count ++;
    }
    return SUCCESS;
}

/**
 * 测试用例：备份OpenStack 删除快照及其克隆卷成功
 * 前置条件：删除快照及其克隆卷成功
 * Check点：DeleteSnapshotCreateVolumes返回true
 */
TEST_F(OpenStackProtectEngineTest, DeleteSnapshotCreateVolumes_Succ)
{
    m_stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), CreateFSHandlerStub);
    m_stub.set(ADDR(HttpClient, Send), Stub_DeleteSnapshotCreateVolumesSucc);
    AppProtect::StorageRepository repo;
    auto jobHandler = CommonFormJobHandler(m_backupInfo, repo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandler, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    VolSnapInfo snapInfo;
    snapInfo.m_snapshotId = "1-2-3-4-snap";
    int ret = openstackProtectEngineHandler->DeleteSnapshotCreateVolumes(snapInfo);
    EXPECT_EQ(ret, true);
    m_stub.reset(ADDR(HttpClient, Send));
    m_stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
}

std::string StubConfigReaderBackupResult(std::string section, std::string keyName)
{
    return "active,stopped,suspended,in-use";
}

int32_t StubCheckBackupSendRequest(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int32_t count = 0;
    if (count == 0) {
        response->SetSuccess(true);
        response->SetGetBody(getServersResponseBody);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        count = 1;
    } else {
        response->SetSuccess(true);
        response->SetGetBody(getVolumeResponse);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
    }
    return SUCCESS;
}

/**
 * 测试用例：CheckBeforeRecover接口测试
 * 前置条件：所有检查点均满足备份条件
 * Check点：测试用例：CheckBeforeRecover接口测试返回SUCCESS
 */
TEST_F(OpenStackProtectEngineTest, CheckBeforeResoverSucc)
{
    m_stub.set(ADDR(HttpClient, Send), StubCheckBackupSendRequest);
    m_stub.set(ADDR(Module::ConfigReader, getString), StubConfigReaderBackupResult);
    VMInfo vmInfo;
    vmInfo.m_uuid = "1-2-2-1";
    VolInfo restoreVol;
    restoreVol.m_uuid = "1-1-1-1";
    restoreVol.m_volSizeInBytes = 10;
    restoreVol.m_metadata = getVolumeResponse;
    restoreVol.m_bootable = "true";
    vmInfo.m_volList.push_back(restoreVol);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    int ret = openstackProtectEngineHandler->CheckBeforeRecover(vmInfo);
    EXPECT_EQ(ret, SUCCESS);
    m_stub.reset(ADDR(HttpClient, Send));
    m_stub.reset(ADDR(Module::ConfigReader, getString));
}


/**
 * 测试用例：AllowBackupInLocalNode接口测试
 * 前置条件：所有检查点均满足备份条件
 * Check点：测试用例：AllowBackupInLocalNode接口测试返回SUCCESS
 */
TEST_F(OpenStackProtectEngineTest, AllowBackupInLocalNodeSucc)
{

    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    AppProtect::SubJob subJobObj;
    int32_t errorCode;
    g_httpStatusCode = 200;
    g_httpResponsebody = g_getServicesResponse;
    int ret = openstackProtectEngineHandler->AllowRestoreInLocalNode(m_restoreInfo, errorCode);
    EXPECT_EQ(ret, SUCCESS);
    ret = openstackProtectEngineHandler->AllowRestoreSubJobInLocalNode(m_restoreInfo, subJobObj, errorCode);
    EXPECT_EQ(ret, SUCCESS);
    ret = openstackProtectEngineHandler->AllowBackupInLocalNode(m_backupInfo, errorCode);
    EXPECT_EQ(ret, SUCCESS);
    ret = openstackProtectEngineHandler->AllowBackupSubJobInLocalNode(m_backupInfo, subJobObj, errorCode);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例：PostHook锁定虚拟机失败
 * 前置条件：锁定虚拟机失败
 * Check点：PostHook返回FAILED
 */
TEST_F(OpenStackProtectEngineTest, PreHook_lockVM_FAIL)
{
    m_stub.set(ADDR(HttpClient, Send), Stub_UnlockServerRequestFail);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    ExecHookParam param;
    param.stage = JobStage::POST_JOB;
    int ret = openstackProtectEngineHandler->PreHook(param);
    EXPECT_EQ(ret, FAILED);
    m_stub.reset(ADDR(HttpClient, Send));
}

/**
 * 测试用例：成功解锁定虚拟机
 * 前置条件：解锁定虚拟机成功
 * Check点：PreHook返回SUCCESS
 */
TEST_F(OpenStackProtectEngineTest, PreHook_UnlockVM_SUCC)
{
    m_stub.set(ADDR(HttpClient, Send), Stub_UnlockServerRequestSucc);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    ExecHookParam param;
    param.stage = JobStage::PRE_PREREQUISITE;
    int ret = openstackProtectEngineHandler->PostHook(param);
    EXPECT_EQ(ret, SUCCESS);
    m_stub.reset(ADDR(HttpClient, Send));
}

/**
 * 测试用例：解锁虚拟机失败
 * 前置条件：发送消息解锁定虚拟机失败
 * Check点：PreHook返回FAILED
 */
TEST_F(OpenStackProtectEngineTest, PreHook_UnlockVM_FAIL)
{
    m_stub.set(ADDR(HttpClient, Send), Stub_UnlockServerRequestFail);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    ExecHookParam param;
    param.stage = JobStage::PRE_PREREQUISITE;
    int ret = openstackProtectEngineHandler->PostHook(param);
    EXPECT_EQ(ret, FAILED);
    m_stub.reset(ADDR(HttpClient, Send));
}


std::string OpenStack_ConfigReaderBackupResult(std::string section, std::string keyName)
{
    return "active,stopped,suspended,in-use";
}

/**
 * 测试用例：CheckBeforeBackup接口测试
 * 前置条件：所有检查点均满足备份条件
 * Check点：CheckBeforeBackup返回SUCCESS
 */
TEST_F(OpenStackProtectEngineTest, CheckBeforeBackupSucc)
{
	m_stub.set(ADDR(Module::ConfigReader, getString), OpenStack_ConfigReaderBackupResult);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    g_httpStatusCode = 200;
    g_httpResponsebody = getServersResponseBody;
    int ret = openstackProtectEngineHandler->CheckBeforeBackup();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例：CheckBeforeBackup接口测试
 * 前置条件：获取虚拟机状态失败
 * Check点：CheckBeforeBackup返回FAILED
 */
TEST_F(OpenStackProtectEngineTest, CheckBeforeBackupFailed)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    g_httpStatusCode = 500;
    g_httpResponsebody = "";
    int ret = openstackProtectEngineHandler->CheckBeforeBackup();
    EXPECT_EQ(ret, FAILED);
}

static std::string g_serverStatus = "ERROR";
static ServerDetail GetServerDetails_status_Stub(void* obj)
{
	ServerDetail server;
	server.m_hostServerInfo.m_uuid = "123";
	server.m_hostServerInfo.m_status = g_serverStatus;
    server.m_hostServerInfo.m_name = "test_machine";
    server.m_hostServerInfo.m_hostId = "hostId";

    ServerVolume volume;
    volume.m_uuid = "test_uuid";
    server.m_hostServerInfo.m_osExtendedVolumesvolumesAttached.push_back(volume);
	return server;
}

static ServerDetail GetServerDetails_Exchangestatus_Stub(void* obj)
{
	ServerDetail server;
	server.m_hostServerInfo.m_uuid = "123";
	server.m_hostServerInfo.m_status = g_serverStatus;
	if (g_serverStatus == "SHUTOFF") {
		g_serverStatus = "ACTIVE";
	} else {
		g_serverStatus = "SHUTOFF";
	}
	return server;
}

static bool GetServerDetailsSucc_Stub(void* obj, ServerDetail server)
{
	server.m_hostServerInfo.m_uuid = "123";
	server.m_hostServerInfo.m_status = g_serverStatus;
	return true;
}

static bool GetServerDetailsFail_Stub(void* obj, ServerDetail server)
{
	return false;
}

/**
 * 测试用例：开机虚拟机成功
 * 前置条件：虚拟机状态可被开机，或虚拟机已开机
 * Check点：返回成功
 */
TEST_F(OpenStackProtectEngineTest, PowerOnMachineSucc)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
	openstackProtectEngineHandler->m_commonWaitInterval = 1;
	openstackProtectEngineHandler->m_commonWaitTimes = 3;
	VMInfo vmInfo;
	vmInfo.m_uuid = "123";

    Stub stub;
    stub.set(ADDR(GetServerDetailsResponse, GetServerDetails), GetServerDetails_status_Stub);

	// 虚拟机已开机
    g_httpStatusCode = 202;
    g_httpResponsebody = getServersResponseBody;
	g_serverStatus = "ACTIVE";
    int ret = openstackProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, SUCCESS);

	stub.reset(ADDR(GetServerDetailsResponse, GetServerDetails));
    stub.set(ADDR(GetServerDetailsResponse, GetServerDetails), GetServerDetails_Exchangestatus_Stub);
	// 虚拟机开机成功, 由shutoff变为active
	g_serverStatus = "SHUTOFF";
    ret = openstackProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, SUCCESS);

	stub.reset(ADDR(GetServerDetailsResponse, GetServerDetails));
}

/**
 * 测试用例：开机虚拟机失败
 * 前置条件：网络异常场景下,导致的失败
 * Check点：返回失败
 */
TEST_F(OpenStackProtectEngineTest, PowerOnMachineNetworkFailed)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
	openstackProtectEngineHandler->m_commonWaitInterval = 1;
	openstackProtectEngineHandler->m_commonWaitTimes = 3;
	VMInfo vmInfo;
	vmInfo.m_uuid = "123";

    Stub stub;
    stub.set(ADDR(OpenStackProtectEngine, GetServerDetails), GetServerDetailsFail_Stub);

	// 获取虚拟机信息失败，导致虚拟机开机失败
    int ret = openstackProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, FAILED);

	stub.reset(ADDR(OpenStackProtectEngine, GetServerDetails));
    stub.set(ADDR(OpenStackProtectEngine, GetServerDetails), GetServerDetailsSucc_Stub);
	// 开机命令发送执行失败，导致开机失败
    g_httpStatusCode = 500;
	g_serverStatus = "SHUTOFF";
	ret = openstackProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, FAILED);
	stub.reset(ADDR(OpenStackProtectEngine, GetServerDetails));
}

/**
 * 测试用例：开机虚拟机失败
 * 前置条件：设备异常场景下,导致的失败
 * Check点：返回失败
 */
TEST_F(OpenStackProtectEngineTest, PowerOnMachineDeviceFailed)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
	openstackProtectEngineHandler->m_commonWaitInterval = 1;
	openstackProtectEngineHandler->m_commonWaitTimes = 3;
	VMInfo vmInfo;
	vmInfo.m_uuid = "123";

    Stub stub;
    stub.set(ADDR(GetServerDetailsResponse, GetServerDetails), GetServerDetails_status_Stub);

	// 虚拟机不支持开机
    g_httpResponsebody = getServersResponseBody;
	g_serverStatus = "BUILD";
    int ret = openstackProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, FAILED);

	// 虚拟机状态一直为关机
	g_serverStatus = "SHUTOFF";
    ret = openstackProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, FAILED);
	stub.reset(ADDR(GetServerDetailsResponse, GetServerDetails));
}

/**
 * 测试用例：关机虚拟机成功
 * 前置条件：虚拟机状态可被关机，或虚拟机已关机
 * Check点：返回成功
 */
TEST_F(OpenStackProtectEngineTest, PowerOffMachineSucc)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
	openstackProtectEngineHandler->m_commonWaitInterval = 1;
	openstackProtectEngineHandler->m_commonWaitTimes = 3;
	VMInfo vmInfo;
	vmInfo.m_uuid = "123";

    Stub stub;
    stub.set(ADDR(GetServerDetailsResponse, GetServerDetails), GetServerDetails_status_Stub);

	// 虚拟机已关机
    g_httpStatusCode = 202;
    g_httpResponsebody = getServersResponseBody;
	g_serverStatus = "SHUTOFF";
    int ret = openstackProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, SUCCESS);

	stub.reset(ADDR(GetServerDetailsResponse, GetServerDetails));
    stub.set(ADDR(GetServerDetailsResponse, GetServerDetails), GetServerDetails_Exchangestatus_Stub);
	// 虚拟机关机成功, 由active变为shutoff
	g_serverStatus = "ACTIVE";
    ret = openstackProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, SUCCESS);
	stub.reset(ADDR(GetServerDetailsResponse, GetServerDetails));
}

/**
 * 测试用例：关机虚拟机失败
 * 前置条件：网络异常场景下,导致的失败
 * Check点：返回失败
 */
TEST_F(OpenStackProtectEngineTest, PowerOffMachineNetworkFailed)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
	openstackProtectEngineHandler->m_commonWaitInterval = 1;
	openstackProtectEngineHandler->m_commonWaitTimes = 3;
	VMInfo vmInfo;
	vmInfo.m_uuid = "123";

    Stub stub;
    stub.set(ADDR(OpenStackProtectEngine, GetServerDetails), GetServerDetailsFail_Stub);

	// 获取虚拟机信息失败，导致虚拟机关机失败
    int ret = openstackProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, FAILED);

	stub.reset(ADDR(OpenStackProtectEngine, GetServerDetails));
    stub.set(ADDR(OpenStackProtectEngine, GetServerDetails), GetServerDetailsSucc_Stub);
	// 关机命令发送执行失败，导致关机失败
    g_httpStatusCode = 500;
	g_serverStatus = "ACTIVE";
	ret = openstackProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, FAILED);
	stub.reset(ADDR(OpenStackProtectEngine, GetServerDetails));
}

/**
 * 测试用例：关机虚拟机失败
 * 前置条件：设备异常场景下,导致的失败
 * Check点：返回失败
 */
TEST_F(OpenStackProtectEngineTest, PowerOffMachineDeviceFailed)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
	openstackProtectEngineHandler->m_commonWaitInterval = 1;
	openstackProtectEngineHandler->m_commonWaitTimes = 3;
	VMInfo vmInfo;
	vmInfo.m_uuid = "123";

    Stub stub;
    stub.set(ADDR(GetServerDetailsResponse, GetServerDetails), GetServerDetails_status_Stub);

	// 虚拟机不支持关机
    g_httpStatusCode = 200;
    g_httpResponsebody = getServersResponseBody;
	g_serverStatus = "BUILD";
    int ret = openstackProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, FAILED);

	// 虚拟机关机失败, 一直状态为光机
	g_serverStatus = "ACTIVE";
    ret = openstackProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, FAILED);
	stub.reset(ADDR(GetServerDetailsResponse, GetServerDetails));
}

static bool g_attachVolumeFlag = false;

static bool GetVolumeDetailsSuccForAttach_Stub(void* obj, const std::string &volId, Volume volume)
{
	volume.m_id = volId;
	VolumeAttachment attachment;
	attachment.m_device = "/mnt/volume";
	attachment.m_serverId = "10a4e361-c981-46f2-b9ba-d7ff9c601693";
	attachment.m_volumeId = volId;
	volume.m_attachPoints.push_back(attachment);
	volume.m_status = "in-use";
	return true;
}

static bool GetVolumeDetailsSuccForAttach_Exchange_Stub(void* obj, const std::string &volId, Volume volume)
{
	volume.m_id = volId;
	if (!g_attachVolumeFlag) {
		VolumeAttachment attachment;
		attachment.m_device = "/mnt/volume";
		attachment.m_serverId = "10a4e361-c981-46f2-b9ba-d7ff9c601693";
		attachment.m_volumeId = volId;
		volume.m_attachPoints.push_back(attachment);
		volume.m_status = "in-use";
		g_attachVolumeFlag = true;
	} else {
		volume.m_attachPoints.clear();
		volume.m_status = "available";
		g_attachVolumeFlag = false;
	}
	return true;
}

static bool GetVolumeDetailsSuccForNoAttach_Stub(void* obj, const std::string &volId, Volume volume)
{
	volume.m_id = volId;
	volume.m_status = "available";
	return true;
}

static int32_t DetachVolumeHandleSucc_Stub(void* obj)
{
	return SUCCESS;
}

static int32_t DetachVolumeHandleFail_Stub(void* obj)
{
	return FAILED;
}

static void StubReturnNull()
{
    return;
}

static int32_t StubAttachVolumeSuccess(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_httpSendCount == 0) { // 请求挂载卷成功
        response->SetSuccess(true);
        response->SetStatusCode(Module::SC_OK);
        response->SetGetBody("{}");
        g_httpSendCount++;
        return SUCCESS;
    }
    if (g_httpSendCount == 1) { // 查询卷消息成功
        response->SetSuccess(true);
        response->SetStatusCode(Module::SC_OK);
        response->SetGetBody(getVolumeResponse);
        g_httpSendCount++;
        return SUCCESS;
    }
}


/**
 * 测试用例：挂载卷成功
 * 前置条件：卷检查挂载结果符合预期，或卷已被挂载
 * Check点：返回成功
 */
TEST_F(OpenStackProtectEngineTest, AttachVolumeSucc)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
	openstackProtectEngineHandler->m_commonWaitInterval = 1;
	openstackProtectEngineHandler->m_commonWaitTimes = 3;
	VolInfo volInfo;
	volInfo.m_uuid = "vol-123";
	VolAttachMents attachment("/mnt/volume");
	volInfo.m_attachPoints.push_back(attachment);

    Stub stub;
    stub.set(ADDR(OpenStackProtectEngine, GetVolumeDetail), GetVolumeDetailsSuccForAttach_Stub);
    stub.set(ADDR(OpenStackProtectEngine, DetachVolumeHandle), DetachVolumeHandleSucc_Stub);
    stub.set(ADDR(OpenStackProtectEngine, UpdateTargetServerId), DetachVolumeHandleSucc_Stub);

	// 卷已挂载
    g_httpStatusCode = static_cast<uint32_t>(Module::SC_ACCEPTED);
    g_httpResponsebody = getVolumeResponse;
    openstackProtectEngineHandler->m_serverId = "10a4e361-c981-46f2-b9ba-d7ff9c601693";
    int ret = openstackProtectEngineHandler->AttachVolume(volInfo);
    EXPECT_EQ(ret, SUCCESS);

	stub.reset(ADDR(OpenStackProtectEngine, GetVolumeDetail));
    stub.set(ADDR(OpenStackProtectEngine, GetVolumeDetail), GetVolumeDetailsSuccForAttach_Exchange_Stub);
    stub.set(ADDR(HttpClient, Send), StubAttachVolumeSuccess);
	// 卷挂载成功, attachment出现
	g_attachVolumeFlag = true;           // 未挂载->挂载
    ret = openstackProtectEngineHandler->AttachVolume(volInfo);
    EXPECT_EQ(ret, SUCCESS);

	stub.reset(ADDR(OpenStackProtectEngine, GetVolumeDetail));
	stub.reset(ADDR(OpenStackProtectEngine, DetachVolumeHandle));
    stub.reset(ADDR(OpenStackProtectEngine, UpdateTargetServerId));
    stub.reset(ADDR(HttpClient, Send));
}

/**
 * 测试用例：卷挂载失败
 * 前置条件：卷一直没挂载成功
 * Check点：返回失败
 */
TEST_F(OpenStackProtectEngineTest, AttachVolumeFailed)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
	openstackProtectEngineHandler->m_commonWaitInterval = 1;
	openstackProtectEngineHandler->m_commonWaitTimes = 3;
	VolInfo volInfo;
	volInfo.m_uuid = "vol-123";
	VolAttachMents attachment("/mnt/volume");
	volInfo.m_attachPoints.push_back(attachment);

    Stub stub;
    stub.set(ADDR(OpenStackProtectEngine, GetVolumeDetail), GetVolumeDetailsSuccForNoAttach_Stub);
    stub.set(ADDR(OpenStackProtectEngine, DetachVolumeHandle), DetachVolumeHandleSucc_Stub);
    stub.set(ADDR(OpenStackProtectEngine, UpdateTargetServerId), DetachVolumeHandleSucc_Stub);
    openstackProtectEngineHandler->m_serverId = "10a4e361-c981-46f2-b9ba-d7ff9c601693";

	// 发送挂载请求失败
    g_httpStatusCode = static_cast<uint32_t>(Module::SC_BAD_REQUEST);
    g_httpResponsebody = "";
    int ret = openstackProtectEngineHandler->AttachVolume(volInfo);
    EXPECT_EQ(ret, FAILED);

	// 卷一直未挂载成功
	g_httpStatusCode = static_cast<uint32_t>(Module::SC_ACCEPTED);
    g_httpResponsebody = "";
    ret = openstackProtectEngineHandler->AttachVolume(volInfo);
    EXPECT_EQ(ret, FAILED);

	stub.reset(ADDR(OpenStackProtectEngine, GetVolumeDetail));
	stub.reset(ADDR(OpenStackProtectEngine, DetachVolumeHandle));
    stub.reset(ADDR(OpenStackProtectEngine, UpdateTargetServerId));
}

int32_t StubGetVolumeStatuAttaching(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int count = 0;
    if (count <= 15) { // Get volume attaching
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "attaching");
        response->SetGetBody(volResp);
        count ++;
    } else { // Get volume in use
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        std::string volResp = getVolumeResponse;
        response->SetGetBody(volResp);
        count ++;
    }
    return SUCCESS;
}

/*
 * 测试用例： cinder尝试打开成功
 * 前置条件:  无
 * CHECK点: OPEN返回SUCCESS
 */
TEST_F(OpenStackProtectEngineTest, TEST_Wait_Volume_ExpandTime_OK)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    m_stub.set(ADDR(HttpClient, Send), StubGetVolumeStatuAttaching);
    std::vector<std::string> intermediateState = {"reserved", "attaching"};
    std::string volumeId = "1-1-1-1";
    int32_t ret = openstackProtectEngineHandler->DoWaitVolumeStatus(volumeId, "in-use", intermediateState);
    EXPECT_EQ(ret, 0);
    m_stub.reset(ADDR(HttpClient, Send));
}

static int32_t StubDetachVolumeSuccess(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_httpSendCount == 0) { // 请求卸载卷成功
        response->SetSuccess(true);
        response->SetStatusCode(Module::SC_ACCEPTED);
        g_httpSendCount++;
        return SUCCESS;
    }
    if (g_httpSendCount == 1) { // 查询卷消息成功
        response->SetSuccess(true);
        response->SetStatusCode(Module::SC_OK);
        response->SetGetBody(getVolumeAvailableResponse);
        g_httpSendCount++;
        return SUCCESS;
    }
}

/**
 * 测试用例：卸载卷成功
 * 前置条件：卷检查卸载结果符合预期，或卷已被卸载
 * Check点：返回成功
 */
TEST_F(OpenStackProtectEngineTest, DetachVolumeSucc)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
	openstackProtectEngineHandler->m_commonWaitInterval = 1;
	openstackProtectEngineHandler->m_commonWaitTimes = 3;
    openstackProtectEngineHandler->m_serverId = "10a4e361-c981-46f2-b9ba-d7ff9c601693";
	VolInfo volInfo;
	volInfo.m_uuid = "vol-123";
	VolAttachMents attachment("/mnt/volume");
	volInfo.m_attachPoints.push_back(attachment);

    Stub stub;
    stub.set(ADDR(OpenStackProtectEngine, GetVolumeDetail), GetVolumeDetailsSuccForNoAttach_Stub);
    stub.set(ADDR(OpenStackProtectEngine, UpdateTargetServerId), DetachVolumeHandleSucc_Stub);

	// 卷已卸载
    g_httpStatusCode = static_cast<uint32_t>(Module::SC_ACCEPTED);
    g_httpResponsebody = "";
    int ret = openstackProtectEngineHandler->DetachVolume(volInfo);
    EXPECT_EQ(ret, SUCCESS);

	stub.reset(ADDR(OpenStackProtectEngine, GetVolumeDetail));
    stub.set(ADDR(OpenStackProtectEngine, GetVolumeDetail), GetVolumeDetailsSuccForAttach_Exchange_Stub);
    stub.set(ADDR(HttpClient, Send), StubDetachVolumeSuccess);
	// 卷卸载成功, attachment消失
	g_attachVolumeFlag = false;           // 挂载->卸载
    ret = openstackProtectEngineHandler->DetachVolume(volInfo);
    EXPECT_EQ(ret, SUCCESS);

	stub.reset(ADDR(OpenStackProtectEngine, GetVolumeDetail));
    stub.reset(ADDR(OpenStackProtectEngine, UpdateTargetServerId));
    stub.reset(ADDR(HttpClient, Send));
}

/**
 * 测试用例：卷卸载失败
 * 前置条件：卷一直没卸载成功
 * Check点：返回失败
 */
TEST_F(OpenStackProtectEngineTest, DetachVolumeFailed)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
	openstackProtectEngineHandler->m_commonWaitInterval = 1;
	openstackProtectEngineHandler->m_commonWaitTimes = 3;
    openstackProtectEngineHandler->m_serverId = "10a4e361-c981-46f2-b9ba-d7ff9c601693";
	VolInfo volInfo;
	volInfo.m_uuid = "vol-123";
	VolAttachMents attachment("/mnt/volume");
	volInfo.m_attachPoints.push_back(attachment);

    Stub stub;
    stub.set(ADDR(OpenStackProtectEngine, GetVolumeDetail), GetVolumeDetailsSuccForAttach_Stub);
    stub.set(ADDR(OpenStackProtectEngine, UpdateTargetServerId), DetachVolumeHandleSucc_Stub);

	// 发送卸载请求失败
    g_httpStatusCode = static_cast<uint32_t>(Module::SC_BAD_REQUEST);
    g_httpResponsebody = "";
    int ret = openstackProtectEngineHandler->DetachVolume(volInfo);
    EXPECT_EQ(ret, FAILED);

	// 卷一直未卸载成功
	g_httpStatusCode = static_cast<uint32_t>(Module::SC_ACCEPTED);
    g_httpResponsebody = "";
    ret = openstackProtectEngineHandler->DetachVolume(volInfo);
    EXPECT_EQ(ret, FAILED);

	stub.reset(ADDR(OpenStackProtectEngine, GetVolumeDetail));
    stub.reset(ADDR(OpenStackProtectEngine, UpdateTargetServerId));
}

/**
 * 测试用例：获取虚拟机元数据信息成功
 * 前置条件：获取虚拟机信息成功
 * Check点：CheckBeforeBackup返回SUCCESS
 */
TEST_F(OpenStackProtectEngineTest, GetMachineMetadata)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    g_httpStatusCode = 200;
    g_httpResponsebody = getServersResponseBody;
    VMInfo vmInfo;
    int retVal = openstackProtectEngineHandler->GetMachineMetadata(vmInfo);
    EXPECT_EQ(retVal, SUCCESS);
}

/**
 * 测试用例：获取虚拟机元数据信息成功
 * 前置条件：获取虚拟机信息成功
 * Check点：CheckBeforeBackup返回SUCCESS
 */
TEST_F(OpenStackProtectEngineTest, GetMachineMetadata1)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    g_httpStatusCode = 200;
    g_httpResponsebody = getVolumeResponse;
    Stub stub;
    stub.set(ADDR(GetServerDetailsResponse, GetServerDetails), GetServerDetails_status_Stub);
    VMInfo vmInfo;
    int retVal = openstackProtectEngineHandler->GetMachineMetadata(vmInfo);
    EXPECT_EQ(retVal, SUCCESS);
}

/**
 * 测试用例：获取虚拟机元数据信息失败
 * 前置条件：获取卷信息失败
 * Check点：CheckBeforeBackup返回FAILED
 */
TEST_F(OpenStackProtectEngineTest, GetMachineMetadataFailed)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    g_httpStatusCode = 500;
    g_httpResponsebody = "";
    VMInfo vmInfo;
    int retVal = openstackProtectEngineHandler->GetMachineMetadata(vmInfo);
    EXPECT_EQ(retVal, FAILED);
}

static int32_t Stub_SendRequestTo_DeleteSnapshot_success(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_httpSendCount == 0) { // 第一次删除快照，接口返回SC_ACCEPTED
        response->SetSuccess(true);
        response->SetStatusCode(Module::SC_ACCEPTED);
        g_httpSendCount++;
        return SUCCESS;
    }
    if (g_httpSendCount ==1) { // 查询快照的状态一直不为已删除
        response->SetSuccess(true);
        response->SetStatusCode(Module::SC_NOT_FOUND);
        g_httpSendCount++;
        return SUCCESS;
    }
}

/*
 * 测试用例：删除快照成功
 * 前置条件：删除快照cinder接口返回成功，并且快照被删除
 * CHECK点：删除快照成功
 */
TEST_F(OpenStackProtectEngineTest, DeleteSnapshot_Success)
{
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_DeleteSnapshot_success);
    stub.set(sleep, Stub_TimeSleep);

    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);

    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);

    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    EXPECT_EQ(openstackProtectEngineHandler->DeleteSnapshot(snapshot), SUCCESS);
    g_httpSendCount = 0;
    stub.reset(ADDR(HttpClient, Send));
}

static bool Stub_OpenStackConsistentSnapshot_InitParam_Success(std::vector<AppProtect::StorageRepository> backupRepos,
    const std::shared_ptr<VirtPlugin::CertManger> &certMgr, const std::string &requestId)
{
    return true;
}
static bool Stub_DoCreateConsistencySnapshot_SUCCESS(const std::vector<VolInfo> &volList,
    SnapshotInfo &snapshot, std::string &errCode)
{
    return true;
}
static int32_t Stub_DeleteConsistencySnapshot_SUCCESS(const SnapshotInfo &snapshot)
{
    return SUCCESS;
}
/*
 * 测试用例：删除快照成功
 * 前置条件：删除快照cinder接口返回成功，并且快照被删除
 * CHECK点：删除快照成功
 */
TEST_F(OpenStackProtectEngineTest, DeleteConsistenSnapshot_Success)
{
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_DeleteSnapshot_success);
    stub.set(sleep, Stub_TimeSleep);
    StubSetTagertBckupExtendInfo(m_backupInfo);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);

    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);

    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    stub.set(ADDR(OpenStackConsistentSnapshot, InitParam), Stub_OpenStackConsistentSnapshot_InitParam_Success);
    stub.set(ADDR(OpenStackConsistentSnapshot, DeleteConsistencySnapshot), Stub_DeleteConsistencySnapshot_SUCCESS);
    EXPECT_EQ(openstackProtectEngineHandler->DeleteSnapshot(snapshot), SUCCESS);
    g_httpSendCount = 0;
    stub.reset(ADDR(OpenStackConsistentSnapshot, InitParam));
    stub.reset(ADDR(OpenStackConsistentSnapshot, DeleteConsistencySnapshot));
    stub.reset(ADDR(HttpClient, Send));
}

static int32_t Stub_SendRequestTo_DeleteSnapshot_Success_When_No_Snapshot(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetStatusCode(404);
    return SUCCESS;
}

/* 测试用例：删除快照成功
 * 前置条件：删除快照cinder接口返回成功, 快照不存在
 * CHECK点：删除快照成功
 */
TEST_F(OpenStackProtectEngineTest, DeleteSnapshot_Success_When_No_Snapshot)
{
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_DeleteSnapshot_Success_When_No_Snapshot);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);

    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    EXPECT_EQ(openstackProtectEngineHandler->DeleteSnapshot(snapshot), SUCCESS);
    stub.reset(ADDR(HttpClient, Send));
}

/*
 * 测试用例：删除快照失败
 * 前置条件：删除快照cinder接口返回失败
 * CHECK点：删除快照失败
 */
TEST_F(OpenStackProtectEngineTest, DeleteSnapshot_Failed)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);

    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    g_httpStatusCode = 500;
    g_httpResponsebody = "";
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    EXPECT_EQ(openstackProtectEngineHandler->DeleteSnapshot(snapshot), FAILED);
}

static int32_t Stub_SendRequestTo_DeleteSnapshot_When_Reset_Failed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_httpSendCount == 0) { // 第一次删除快照，接口返回502
        response->SetSuccess(true);
        response->SetStatusCode(502);
        g_httpSendCount++;
        return SUCCESS;
    }
    if (g_httpSendCount >=1 && g_httpSendCount <=SNAPSHOT_RETRY_TIMES) { // 查询快照的状态一直不为已删除
        response->SetSuccess(true);
        response->SetGetBody(g_volumeSnapDetailStr);
        response->SetStatusCode(200);
        g_httpSendCount++;
        return SUCCESS;
    }
    if (g_httpSendCount == SNAPSHOT_RETRY_TIMES+1) { // 查询快照状态为deleting
        response->SetSuccess(true);
        response->SetGetBody(g_volumeSnapDetailStrDeleting);
        response->SetStatusCode(200);
        g_httpSendCount++;
        return SUCCESS;
    }
    if (g_httpSendCount == SNAPSHOT_RETRY_TIMES+2) { // 重置快照状态为error
        response->SetSuccess(true);
        response->SetStatusCode(500);
        response->SetGetBody("");
        g_httpSendCount++;
        return SUCCESS;
    }
}

static int32_t Stub_SendRequestTo_DeleteSnapshot_When_Reset_Success(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_httpSendCount == 0) { // 第一次删除快照，接口返回502
        response->SetSuccess(true);
        response->SetStatusCode(502);
        g_httpSendCount++;
        return SUCCESS;
    }
    if (g_httpSendCount >=1 && g_httpSendCount <=SNAPSHOT_RETRY_TIMES) { // 查询快照的状态一直不为已删除
        response->SetSuccess(true);
        response->SetGetBody(g_volumeSnapDetailStr);
        response->SetStatusCode(200);
        g_httpSendCount++;
        return SUCCESS;
    }
    if (g_httpSendCount == SNAPSHOT_RETRY_TIMES+1) { // 查询快照状态为deleting
        response->SetSuccess(true);
        response->SetGetBody(g_volumeSnapDetailStrDeleting);
        response->SetStatusCode(200);
        g_httpSendCount++;
        return SUCCESS;
    }
    if (g_httpSendCount == SNAPSHOT_RETRY_TIMES+2) { // 重置快照状态为error
        response->SetSuccess(true);
        response->SetStatusCode(500);
        response->SetGetBody("");
        g_httpSendCount++;
        return SUCCESS;
    }
}

/*
 * 测试用例：删除快照失败
 * 前置条件：删除快照cinder接口返回失败，快照状态重置失败
 * CHECK点：删除快照失败
 */
TEST_F(OpenStackProtectEngineTest, DeleteSnapshot_Failed_WhenResetSuccess)
{
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_DeleteSnapshot_When_Reset_Success);
    stub.set(sleep, Stub_TimeSleep);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);

    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    EXPECT_EQ(openstackProtectEngineHandler->DeleteSnapshot(snapshot), FAILED);
    g_httpSendCount = 0;
    stub.reset(ADDR(HttpClient, Send));
}

/*
 * 测试用例：重置快照状态为error后删除快照成功
 * 前置条件：删除快照失败后重置快照状态为error成功，再次删除快照成功
 * CHECK点：删除快照成功
 */
TEST_F(OpenStackProtectEngineTest, DeleteSnapshotSuccess_WhenResetSccess)
{

}

static int32_t Stub_SendRequestTo_CreateSnapshot_Success(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetGetBody(g_volumeSnapDetailStr);
    response->SetStatusCode(202);
    return SUCCESS;
}

int32_t Stub_OpenStackConfigReaderSnapshotResult(std::string section, std::string keyName)
{
    return 10;
}

static int32_t Stub_SendRequestTo_CreateSnapshot_Failed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(false);
    response->SetStatusCode(404);
    return FAILED;
}

/*
 * 测试用例：创建快照成功
 * 前置条件：创建快照cinder接口返回成功
 * CHECK点：创建快照成功
 */
TEST_F(OpenStackProtectEngineTest, CreateSnapshot_Success)
{
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_CreateSnapshot_Success);
    stub.set(ADDR(Module::ConfigReader, getInt), Stub_OpenStackConfigReaderSnapshotResult);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);

    SnapshotInfo snapshot;
    std::string errCode;
    EXPECT_EQ(openstackProtectEngineHandler->CreateSnapshot(snapshot, errCode), SUCCESS);
    EXPECT_EQ(snapshot.m_volSnapList[0].m_volUuid, openstackProtectEngineHandler->m_vmInfo.m_volList[0].m_uuid);
    m_stub.reset(ADDR(HttpClient, Send));
}

/*
 * 测试用例：创建快照失败
 * 前置条件：创建快照cinder接口返回失败
 * CHECK点：创建快照失败
 */
TEST_F(OpenStackProtectEngineTest, CreateSnapshot_Failed)
{
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_CreateSnapshot_Failed);
    stub.set(ADDR(Module::ConfigReader, getInt), Stub_OpenStackConfigReaderSnapshotResult);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);

    SnapshotInfo snapshot;
    std::string errCode;
    EXPECT_EQ(openstackProtectEngineHandler->CreateSnapshot(snapshot, errCode), FAILED);
    m_stub.reset(ADDR(HttpClient, Send));
}

static int32_t Insufficient_Snapshot_Quota_Failed(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetGetBody(g_volumeSnapDetailStr);
    response->SetStatusCode(413);
    return SUCCESS;
}

/*
 * 测试用例：创建快照失败
 * 前置条件：快照配额不足返回失败
 * CHECK点：创建快照失败
 */
TEST_F(OpenStackProtectEngineTest, CreateSnapshot_Failed_Insufficient_Quota)
{
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Insufficient_Snapshot_Quota_Failed);
    stub.set(ADDR(Module::ConfigReader, getInt), Stub_OpenStackConfigReaderSnapshotResult);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);

    SnapshotInfo snapshot;
    std::string errCode;
    EXPECT_EQ(openstackProtectEngineHandler->CreateSnapshot(snapshot, errCode), FAILED);
    EXPECT_EQ(errCode, "1577209935");
}


static int32_t Stub_SendRequestTo_CreateSnapshot_WhenStatusError(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_httpSendCount == 0) {
        response->SetSuccess(true);
        response->SetStatusCode(202);
        response->SetGetBody(g_volumeSnapDetailStrCreating);
        g_httpSendCount++;
        return SUCCESS;
    } else if (g_httpSendCount == 1) {
        response->SetSuccess(true);
        response->SetStatusCode(200);
        response->SetGetBody(g_volumeSnapDetailStrError);
        g_httpSendCount++;
        return SUCCESS;
    } else if (g_httpSendCount == 2) {
        response->SetSuccess(true);
        response->SetStatusCode(202);
        g_httpSendCount++;
        return SUCCESS;
    } else if (g_httpSendCount == 3) {
        response->SetSuccess(true);
        response->SetStatusCode(404);
        response->SetGetBody("{\"itemNotFound\":{\"message\":\"Snapshot 30e15ed1-5ab5-4049-97a3-f7001b0707c5 could not be found.\",\"code\":404}}");
        g_httpSendCount++;
        return SUCCESS;
    }
}

/*
 * 测试用例：创建快照失败
 * 前置条件：创建快照，快照状态为error
 * CHECK点：创建快照失败
 */
TEST_F(OpenStackProtectEngineTest, CreateSnapshot_WhenSnapStatusError)
{
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_CreateSnapshot_WhenStatusError);
    stub.set(ADDR(Module::ConfigReader, getInt), Stub_OpenStackConfigReaderSnapshotResult);
    stub.set(sleep, Stub_TimeSleep);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);

    SnapshotInfo snapshot;
    std::string errCode;
    EXPECT_EQ(openstackProtectEngineHandler->CreateSnapshot(snapshot, errCode), FAILED);
    g_httpSendCount = 0;
    m_stub.reset(ADDR(HttpClient, Send));
}


 /* 测试用例：创建指定快照失败
 * 前置条件：指定卷信息为空
 * CHECK点：创建快照失败
 */
TEST_F(OpenStackProtectEngineTest, CreateSnapshot_WhenTagertVolumeListempty)
{
    StubSetTagertBckupInfo(m_backupInfo);
    printf("%s\n",m_backupInfo.protectSubObject[0].id.c_str());
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);

    SnapshotInfo snapshot;
    std::string errCode;
    EXPECT_EQ(openstackProtectEngineHandler->CreateSnapshot(snapshot, errCode), FAILED);
}

/*
 * 测试用例：创建指定快照成功
 * 前置条件：创建快照cinder接口返回成功
 * CHECK点：创建快照成功
 */
TEST_F(OpenStackProtectEngineTest, CreateSnapshot_TargetSuccess)
{
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_CreateSnapshot_Success);
    stub.set(ADDR(Module::ConfigReader, getInt), Stub_OpenStackConfigReaderSnapshotResult);
    StubSetTagertBckupInfo(m_backupInfo);
    m_backupInfo.protectSubObject[0].id = "c1f1465b-6a63-4343-9761-7ad3e413f4ef";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);

    SnapshotInfo snapshot;
    std::string errCode;
    EXPECT_EQ(openstackProtectEngineHandler->CreateSnapshot(snapshot, errCode), SUCCESS);
    EXPECT_EQ(snapshot.m_volSnapList[0].m_volUuid, openstackProtectEngineHandler->m_vmInfo.m_volList[0].m_uuid);
}

/*
 * 测试用例：创建指定快照成功
 * 前置条件：创建快照cinder接口返回成功
 * CHECK点：创建快照成功
 */
TEST_F(OpenStackProtectEngineTest, CreateConsistenSnapshot_TargetSuccess)
{
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_CreateSnapshot_Success);
    stub.set(ADDR(Module::ConfigReader, getInt), Stub_OpenStackConfigReaderSnapshotResult);
    StubSetTagertBckupExtendInfo(m_backupInfo);
    m_backupInfo.protectSubObject[0].id = "c1f1465b-6a63-4343-9761-7ad3e413f4ef";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);
    stub.set(ADDR(OpenStackConsistentSnapshot, InitParam), Stub_OpenStackConsistentSnapshot_InitParam_Success);
    stub.set(ADDR(OpenStackConsistentSnapshot, DoCreateConsistencySnapshot), Stub_DoCreateConsistencySnapshot_SUCCESS);
    SnapshotInfo snapshot;
    std::string errCode;
    EXPECT_EQ(openstackProtectEngineHandler->CreateSnapshot(snapshot, errCode), SUCCESS);
    stub.reset(ADDR(OpenStackConsistentSnapshot, InitParam));
    stub.reset(ADDR(OpenStackConsistentSnapshot, DoCreateConsistencySnapshot));
}

/*
 * 测试用例：创建指定快照成功
 * 前置条件：创建快照cinder接口返回成功
 * CHECK点：创建快照成功
 */
TEST_F(OpenStackProtectEngineTest, CreateConsistenSnapshot_ALLSuccess)
{
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_CreateSnapshot_Success);
    stub.set(ADDR(Module::ConfigReader, getInt), Stub_OpenStackConfigReaderSnapshotResult);
    StubSetBckupExtendInfo(m_backupInfo);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);

    SnapshotInfo snapshot;
    std::string errCode;
    stub.set(ADDR(OpenStackConsistentSnapshot, InitParam), Stub_OpenStackConsistentSnapshot_InitParam_Success);
    stub.set(ADDR(OpenStackConsistentSnapshot, DoCreateConsistencySnapshot), Stub_DoCreateConsistencySnapshot_SUCCESS);
    EXPECT_EQ(openstackProtectEngineHandler->CreateSnapshot(snapshot, errCode), SUCCESS);
    stub.reset(ADDR(OpenStackConsistentSnapshot, InitParam));
    stub.reset(ADDR(OpenStackConsistentSnapshot, DoCreateConsistencySnapshot));
}


/**
 * 测试用例：生成卷匹配对信息成功
 * 前置条件：获取卷信息成功
 * Check点：返回SUCCESS
 */
TEST_F(OpenStackProtectEngineTest, GenVolPairSucc)
{
    Stub stub;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->m_metaRepoHandler = std::make_shared<FileSystemHandler>();
    openstackProtectEngineHandler->m_cacheRepoHandler = std::make_shared<FileSystemHandler>();
    openstackProtectEngineHandler->m_metaRepoPath = "\\tmp";
    openstackProtectEngineHandler->m_cacheRepoPath = "\\tmp";
    stub.set(ADDR(OpenStackProtectEngine, InitRepoHandler), StubReturnNull);
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    g_httpStatusCode = 200;
    g_httpResponsebody = getVolumeResponse;

    VMInfo vmInfo;
	VolInfo copyVol;
	copyVol.m_uuid = "53f52bff-8ffa-46ae-98b4-a1f20aa99a7a";
	ApplicationResource targetVolResource;
    targetVolResource.type = "volume";
    targetVolResource.id = "1-1-1-1";
    targetVolResource.__set_extendInfo(t_volExtendInfo);  // 目标卷信息
	VolMatchPairInfo volPairs;
    int retVal = openstackProtectEngineHandler->GenVolPair(vmInfo, copyVol, targetVolResource, volPairs);
	EXPECT_EQ(volPairs.m_volPairList.size(), 1);
    stub.reset(ADDR(OpenStackProtectEngine, InitRepoHandler));
}

/**
 * 测试用例：生成卷匹配对信息失败
 * 前置条件：获取卷信息失败
 * Check点：返回FAILED
 */
TEST_F(OpenStackProtectEngineTest, GenVolPairFail)
{
    Stub stub;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->m_metaRepoHandler = std::make_shared<FileSystemHandler>();
    openstackProtectEngineHandler->m_cacheRepoHandler = std::make_shared<FileSystemHandler>();
    openstackProtectEngineHandler->m_metaRepoPath = "\\tmp";
    openstackProtectEngineHandler->m_cacheRepoPath = "\\tmp";
    stub.set(ADDR(OpenStackProtectEngine, InitRepoHandler), StubReturnNull);
    bool ret = openstackProtectEngineHandler->InitJobPara();
    EXPECT_EQ(ret, true);
    g_httpStatusCode = 500;
    g_httpResponsebody = "";

    VMInfo vmInfo;
	VolInfo copyVol;
	ApplicationResource targetVolResource;
	VolMatchPairInfo volPairs;
    int retVal = openstackProtectEngineHandler->GenVolPair(vmInfo, copyVol, targetVolResource, volPairs);
	EXPECT_EQ(volPairs.m_volPairList.size(), 0);
    stub.reset(ADDR(OpenStackProtectEngine, InitRepoHandler));
}

/**
 * 测试用例：整机恢复生成卷匹配对信息成功
 * 前置条件：
 * Check点：返回SUCCESS
 */
// TEST_F(OpenStackProtectEngineTest, VMRestore_GenVolPair_Succ)
// {
//     std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
//     std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
//     jobInfo->SetJobInfo(data);
//     std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
//     std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
//     bool ret = openstackProtectEngineHandler->InitJobPara();
//     EXPECT_EQ(ret, true);
//     g_httpStatusCode = 500;
//     g_httpResponsebody = "";

//     VMInfo vmInfo;
// 	VolInfo copyVol;
// 	ApplicationResource targetVolResource;
// 	VolMatchPairInfo volPairs;
//     int retVal = openstackProtectEngineHandler->GenVolPair(vmInfo, copyVol, targetVolResource, volPairs);
// 	EXPECT_EQ(volPairs.m_volPairList.size(), 0);
// }

/*
 * 测试用例：查询快照是否存在
 * 前置条件：快照存在
 * CHECK点：查询快照存在
 */
TEST_F(OpenStackProtectEngineTest, QuerySnapshot_Success)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);
    g_httpStatusCode = 200;
    g_httpResponsebody = g_volumeSnapDetailStr;

    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    EXPECT_EQ(openstackProtectEngineHandler->QuerySnapshotExists(snapshot), SUCCESS);
}

/*
 * 测试用例：查询快照是否存在
 * 前置条件：快照不存在
 * CHECK点：查询快照不存在
 */
TEST_F(OpenStackProtectEngineTest, QuerySnapshot_Failed)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    Module::JsonHelper::JsonStringToStruct(g_volumeInfoStr, openstackProtectEngineHandler->m_vmInfo);
    g_httpStatusCode = 404;
    g_httpResponsebody = "";

    SnapshotInfo snapshot;
    VolSnapInfo volSnap;
    volSnap.m_snapshotId = "123";
    snapshot.m_volSnapList.push_back(volSnap);
    EXPECT_EQ(openstackProtectEngineHandler->QuerySnapshotExists(snapshot), SUCCESS);
    EXPECT_TRUE(snapshot.m_deleted);
}

std::string snapListstr = "{\
    \"snapshots\": [\
        {\
            \"id\": \"38e0d44d-50c9-4b1a-87df-7f8e32f8fd45\",\
            \"created_at\": \"2023-02-24T08:17:41.111651\",\
            \"updated_at\": \"2023-02-24T08:17:42.058511\",\
            \"name\": \"Protect_e1ea27aa-5226-4bb0-b3b0-dcf212559e1d_SNAP_1677226659\",\
            \"description\": \"**PLEASE DO NOT CHANGE THE SNAPSHOT NAME AND THIS DESCRIPTION**\",\
            \"volume_id\": \"e1ea27aa-5226-4bb0-b3b0-dcf212559e1d\",\
            \"status\": \"available\",\
            \"size\": 2,\
            \"metadata\": {},\
            \"os-extended-snapshot-attributes:provider_location\": \"{\\\"offset\\\": 0, \\\"storage_type\\\": \\\"FusionStorage\\\", \\\"ip\\\": \\\"8.40.97.234\\\", \\\"pool\\\": 0, \\\"snap_name\\\": \\\"snapshot-38e0d44d-50c9-4b1a-87df-7f8e32f8fd45\\\"}\",\
            \"os-extended-snapshot-attributes:project_id\": \"7ccb7aeb46dc4e47a6b71d52e6c01592\",\
            \"os-extended-snapshot-attributes:progress\": \"100%\"\
        },\
        {\
            \"id\": \"e837fa15-b2b1-4d6c-8438-03a36dd5b3c9\",\
            \"created_at\": \"2023-02-23T14:50:19.594564\",\
            \"updated_at\": \"2023-02-23T14:50:20.866007\",\
            \"name\": \"Protect_e1ea27aa-5226-4bb0-b3b0-dcf212559e1d_SNAP_1677163817\",\
            \"description\": \"**PLEASE DO NOT CHANGE THE SNAPSHOT NAME AND THIS DESCRIPTION**\",\
            \"volume_id\": \"e1ea27aa-5226-4bb0-b3b0-dcf212559e1d\",\
            \"status\": \"available\",\
            \"size\": 2,\
            \"metadata\": {},\
            \"os-extended-snapshot-attributes:provider_location\": \"{\\\"offset\\\": 0, \\\"storage_type\\\": \\\"FusionStorage\\\", \\\"ip\\\": \\\"8.40.97.234\\\", \\\"pool\\\": 0, \\\"snap_name\\\": \\\"snapshot-e837fa15-b2b1-4d6c-8438-03a36dd5b3c9\\\"}\",\
            \"os-extended-snapshot-attributes:project_id\": \"7ccb7aeb46dc4e47a6b71d52e6c01592\",\
            \"os-extended-snapshot-attributes:progress\": \"100%\"\
        }\
    ]\
}";

/*
 * 测试用例：查询卷所对应的快照列表成功
 * 前置条件：返回查询卷的快照列表
 * CHECK点：返回2个快照
 */
TEST_F(OpenStackProtectEngineTest, QueryVolumeSnapShots_succ)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    g_httpStatusCode = 200;
    g_httpResponsebody = snapListstr;
    VolInfo volObj;
    volObj.m_uuid = "1-1-1-1";
    std::vector<VolSnapInfo> snapshotList;
    int ret = openstackProtectEngineHandler->GetSnapshotsOfVolume(volObj, snapshotList);
    EXPECT_EQ(snapshotList.size(), 2);
}

/*
 * 测试用例：执行卷信息日志组装
 * 前置条件：未设置
 * CHECK点：日志信息组装符合预期
 */
TEST_F(OpenStackProtectEngineTest, GetDeleteFailedVolInfo)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    std::vector<Volume> volumeList;
    Volume vol0, vol1;
    vol0.m_id = "volumeId0";
    vol1.m_id = "volumeId1";
    vol0.m_name = "volume-0";
    vol1.m_name = "volume-1";
    volumeList.push_back(vol0);
    volumeList.push_back(vol1);
    std::string volumeStr = "[volume id: volumeId0 volume name: volume-0][volume id: volumeId1 volume name: volume-1]";
    EXPECT_EQ(openstackProtectEngineHandler->GetDeleteFailedVolInfo(volumeList), volumeStr);
}
/*
 * 测试用例：获取卷快照
 * 前置条件：
 * CHECK点：
 */
TEST_F(OpenStackProtectEngineTest, GetVolumeSnapshot_SUCCESS)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    g_httpStatusCode = 200;
    g_httpResponsebody = snapListstr;
    SnapshotDetailsMsg snapDetail;
    EXPECT_EQ(openstackProtectEngineHandler->GetVolumeSnapshot(snapDetail), true);
}

/*
 * 测试用例：获取卷handler
 * 前置条件：
 * CHECK点：
 */
static int32_t Stub_Initvolume_Success()
{
    return SUCCESS;
}

TEST_F(OpenStackProtectEngineTest, GetVolumeHandler_SUCCESS)
{
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    VolInfo Vol;
	Vol.m_uuid = "53f52bff-8ffa-46ae-98b4-a1f20aa99a7a";
    std::shared_ptr<VolumeHandler> volumehandler;
    m_stub.set(ADDR(CinderVolumeHandler,InitializeVolumeInfo), Stub_Initvolume_Success);
    EXPECT_EQ(openstackProtectEngineHandler->GetVolumeHandler(Vol, volumehandler), SUCCESS);
}

/*
 * 测试用例：获取网络信息成功
 * 前置条件：调用生产环境获取网络信息成功
 * CHECK点：
 */
TEST_F(OpenStackProtectEngineTest, GetNetworks_SUCC)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"network\"}";
	request.applications.push_back(m_app);
    g_httpStatusCode = 200;
	g_httpResponsebody = getNetworksResponse;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 1);
}

/*
 * 测试用例：获取网络信息失败
 * 前置条件：调用生产环境获取网络信息失败
 * CHECK点：
 */
TEST_F(OpenStackProtectEngineTest, GetNetworks_FAIL)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"network\"}";
	request.applications.push_back(m_app);
	g_httpStatusCode = 400;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 0);
}

/*
 * 测试用例：获取volumetypes成功
 * 前置条件：调用生产环境卷类型列表成功
 * CHECK点：
 */
TEST_F(OpenStackProtectEngineTest, GetVolumeTypes_SUCC)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"volumeType\"}";
	request.applications.push_back(m_app);
    g_httpStatusCode = 200;
	g_httpResponsebody = getVolumeTypesResponse;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 4);
}

/*
 * 测试用例：获取volumetypes失败
 * 前置条件：调用生产环境卷类型列表失败
 * CHECK点：
 */
TEST_F(OpenStackProtectEngineTest, GetVolumeTypes_FAIL)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"volumeType\"}";
	request.applications.push_back(m_app);
	g_httpStatusCode = 400;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 0);
}

/*
 * 测试用例：获取flavor成功
 * 前置条件：调用生产环境获取flavor成功
 * CHECK点：
 */
TEST_F(OpenStackProtectEngineTest, GetFlavors_SUCC)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"flavor\"}";
	request.applications.push_back(m_app);
    g_httpStatusCode = 200;
	g_httpResponsebody = getFlavorsResponse;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 1);
}

/*
 * 测试用例：获取flavor失败
 * 前置条件：调用生产环境获取flavor失败
 * CHECK点：
 */
TEST_F(OpenStackProtectEngineTest, GetFlavors_FAIL)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"flavor\"}";
	request.applications.push_back(m_app);
	g_httpStatusCode = 401;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 0);
}

int32_t Stub_DeleteMachineSucc(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int count = 0;
    if (count == 0) { // Delete server
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_NO_CONTENT));
        count ++;
    } else if (count == 1) { // GetServerDetail
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_NOT_FOUND));
        response->SetGetBody(getServerDeatilNotFindReponse);
        count ++;
    } else if (count == 2) { // Delete volume
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
        count ++;
    } else if (count == 3) { // Get volume detail
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_NOT_FOUND));
        count ++;
    }
    return SUCCESS;
}

/*
 * 测试用例：删除虚拟机成功
 * 前置条件：
 * CHECK点：
 */
TEST_F(OpenStackProtectEngineTest, DeleteMachine_SUCC)
{
    m_stub.set(ADDR(HttpClient, Send), Stub_DeleteMachineSucc);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    VolInfo vol;
    vol.m_uuid = "456";
	VMInfo vmInfo;
	vmInfo.m_uuid = "123";
    vmInfo.m_volList.push_back(vol);
    int ret = openstackProtectEngineHandler->DeleteMachine(vmInfo);
    EXPECT_EQ(ret, SUCCESS);
    m_stub.reset(ADDR(HttpClient, Send));
}

int32_t Stub_CreateVolumeSucc(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int count = 0;
    if (count == 0) { // create volume
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
        response->SetGetBody(createVolumeResponse);
        count ++;
    } else if (count == 1) { // GetVolumeDetail
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeAvailableResponse);
        count ++;
    }
    return SUCCESS;
}

/*
 * 测试用例：创建卷成功
 * 前置条件：
 * CHECK点：
 */
 TEST_F(OpenStackProtectEngineTest, CreateVolume_SUCC)
 {
    m_stub.set(ADDR(HttpClient, Send), Stub_CreateVolumeSucc);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    VolInfo backupVol;
    backupVol.m_volSizeInBytes = 123;
    backupVol.m_volumeType = "iscsi";
    backupVol.m_name = "volume";
    DatastoreInfo dsInfo;
    VolInfo newVol;
    int ret = openstackProtectEngineHandler->CreateVolume(backupVol, "", "", dsInfo, newVol);
    EXPECT_EQ(ret, SUCCESS);
    m_stub.reset(ADDR(HttpClient, Send));
 }

static std::string GetVolumeInfoFromFile()
{
    Volume vol;
    vol.m_id = "1-1-1-1";
    std::string volStr;
    Module::JsonHelper::StructToJsonString(vol, volStr);
    return volStr;
}

static int32_t ReadVolumeStub(std::string &buf, size_t size)
{
    std::string return_value = GetVolumeInfoFromFile();
    buf = return_value;
    INFOLOG("str %s", return_value.c_str());
    return size;
}

 /*
 * 测试用例：获取AvailabilityZone成功
 * 前置条件：调用生产环境可用分区列表成功
 * CHECK点：
 */
TEST_F(OpenStackProtectEngineTest, GetAvailabilityZone_SUCC)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"availabilityZone\"}";
	request.applications.push_back(m_app);
    g_httpStatusCode = 200;
	g_httpResponsebody = getAZsResponse;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 1);
}

/*
 * 测试用例：获取volumetypes失败
 * 前置条件：调用生产环境卷类型列表失败
 * CHECK点：
 */
TEST_F(OpenStackProtectEngineTest, GetAvailabilityZone_FAIL)
{
    ResourceResultByPage page;
    ListResourceRequest request;
	StubSetAppEnvNoUseCertInfo(request.appEnv);
	request.condition.conditions = "{\"resourceType\": \"volumeType\"}";
	request.applications.push_back(m_app);
	g_httpStatusCode = 400;
    m_opSEngine.ListApplicationResourceV2(page, request);
	EXPECT_EQ(page.items.size(), 0);
}

int32_t Stub_SendCreateMachineSucc(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int count = 0;
    if (count == 0) { // create server
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
        response->SetGetBody(createServerResponse);
        count ++;
    } else if (count == 1) { // GetServerDetail
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_NOT_FOUND));
        response->SetGetBody(getServersResponseBody);
        count ++;
    }
    return SUCCESS;
}

/*
 * 测试用例：创建虚拟机成功
 * 前置条件：
 * CHECK点：
 */
TEST_F(OpenStackProtectEngineTest, SendCreateMachine_SUCC)
{
    m_stub.set(ADDR(HttpClient, Send), Stub_SendCreateMachineSucc);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    openstackProtectEngineHandler->InitJobPara();
    VMInfo vmInfo;
    OpenStackServerInfo serverExtendInfo;
    std::string sysVolumeId = "123";
    int ret = openstackProtectEngineHandler->SendCreateMachineRequest(vmInfo, serverExtendInfo, sysVolumeId);
    EXPECT_EQ(ret, SUCCESS);
    m_stub.reset(ADDR(HttpClient, Send));
}
/*
 * 测试用例：组装创建虚拟请求时，根据外部指定IP的各种情况，设置请求体中的网卡配置
 * 前置条件：外部指定ip，外部未指定ip，外部指定ip为空
 * CHECK点：指定ip须传递准确，未指定ip或指定ip未空，则不能传递空ip给Openstack
 */
TEST_F(OpenStackProtectEngineTest, FormCreateServerBody_NETWORK_IP_CONFIG_TEST)
{
    StubSetVMRestoreInfoRecoverIp(m_restoreInfo);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(m_restoreInfo);
    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> jobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, jobInfo);
    std::shared_ptr<OpenStackProtectEngine> openstackProtectEngineHandler = std::make_shared<OpenStackProtectEngine>(jobHandle, "123", "");
    EXPECT_NE(openstackProtectEngineHandler, nullptr);
    OpenStackServerInfo serverExtendInfo;
    openstackProtectEngineHandler->InitJobPara();
    int32_t ret = openstackProtectEngineHandler->BuildNewServerInfo(serverExtendInfo);
    std::string sysVolumeId = "123";
    std::string reqJsonStr = openstackProtectEngineHandler->FormCreateServerBody(serverExtendInfo, sysVolumeId);
    std::cout << "FormCreateServerBody return reqJsonStr:" << reqJsonStr << std::endl;
    INFOLOG("FormCreateServerBody return reqJsonStr: %s", reqJsonStr.c_str());
    Json::Value resultValue1;
    EXPECT_NE(reqJsonStr, "");
    Json::CharReaderBuilder charReaderBuilder;
    std::unique_ptr<Json::CharReader> pCharReader(charReaderBuilder.newCharReader());
    EXPECT_NE(pCharReader, nullptr);
    const char* begin = reqJsonStr.c_str();
    const char* end = begin + reqJsonStr.size();
    std::string strError;
    bool parseRet = false;
    try {
    parseRet = pCharReader->parse(begin, end, &resultValue1, &strError);
    } catch (std::exception& e) {
    EXPECT_EQ(true, false);
    }
    EXPECT_EQ(resultValue1.isMember("server"), true);
    Json::Value resultValue = resultValue1["server"];
    EXPECT_EQ(resultValue.isMember("networks"), true);
    EXPECT_EQ(resultValue["networks"].size(), 3);
    EXPECT_EQ(resultValue["networks"][0]["uuid"].asString(), "network1");
    EXPECT_EQ(resultValue["networks"][0]["fixed_ip"].asString(), "1.2.3.4");
    EXPECT_EQ(resultValue["networks"][1]["uuid"].asString(), "network2");
    EXPECT_EQ(resultValue["networks"][1].isMember("fixed_ip"), false);
    EXPECT_EQ(resultValue["networks"][2]["uuid"].asString(), "network3");
    EXPECT_EQ(resultValue["networks"][2].isMember("fixed_ip"), false);
}
}
#!/usr/bin/python3
# -*- coding: utf-8 -*-
from ansible.module_utils.basic import AnsibleModule

from ansible_collections.oceanprotect.ansible.plugins.module_utils.api_template.Pm import PmManager
class OceanProtectManager(AnsibleModule):
    """Class representing a OceanProtectManager Ansible Module."""

    def __init__(self, argument_spec, **kwargs):
        """Initializes a OceanProtectManager Ansible Module.

            Args:
                argument_spec   (dict)  --  Arguments that are specific to the module, keys is argument name,
                value is a dictionary of keys type and  required, value is built in type and bool respectively.

        """
        super(OceanProtectManager, self).__init__(argument_spec)
        self.argument_spec = argument_spec
        self.result = {}
        if (self.params.get("manager_hostname")
                and self.params.get("manager_username")
                and self.params.get("manager_password")):
            self.PmManager = PmManager(ip=self.params["manager_hostname"],
                                       user=self.params["manager_username"],
                                       pwd=self.params["manager_password"], )
        else:
            self.fail_json(**{"msg":"not find user info"})

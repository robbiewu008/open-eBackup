from server.models.bridge import DpServerBase


class DpserverE6000(DpServerBase):
    def __init__(self, address, deploy_type):
        super().__init__(address, deploy_type)

    def prepare_env(self):
        pass

    def credentials(self):
        pass

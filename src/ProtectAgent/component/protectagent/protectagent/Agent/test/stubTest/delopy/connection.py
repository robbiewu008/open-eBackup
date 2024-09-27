class IConnection(object):
    def __init__(self, host, username, password, port=22):
        pass

    def exec(self, cmd, except_str='#', timeout=10):
        pass

    def close(self):
        pass

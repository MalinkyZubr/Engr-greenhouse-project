class DeviceDisconnectedException(Exception):
    def __init__(self, device_id):
        message: str = f"The device with ID {device_id} is disconnected!"
        super(Exception, self).__init__(message)
        

class DeviceErrorException(Exception):
    pass


class DeviceHaltedException(Exception):
    def __init__(self, device_id):
        message: str = f"The device with ID {device_id} is halted!"
        super(Exception, self).__init__(message)
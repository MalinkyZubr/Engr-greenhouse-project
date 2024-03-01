class getDeviceData(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM DATA WHERE DeviceID = %(name)s;"""
    def query(self, cursor, device_id):
        return cursor.execute(self.query_str, (device_id,))
    
    
class getLatestDeviceData(DatabaseQuery):
    return_type=ReturnTypes.SINGLE_OBJECT()
    query = \
    f"""SELECT * FROM Data WHERE DeviceID = %d ORDER BY DateCollected DESC limit 1;"""
    def query(self, cursor, device_id):
        return cursor.execute(self.query_str, (device_id,))
    

class getDeviceDataInRange(DatabaseQuery):
    query_str = \
    f"""SELECT * 
    FROM Data
    WHERE DateCollected 
            BETWEEN  str_to_date(%s, '%Y-%m-%d')
                AND   str_to_date(%s, '%Y-%m-%d')
        AND DeviceID = %s;"""
    def query(self, cursor, device_id, start_date, end_date):
        return cursor.execute(self.query_str, (start_date, end_date, device_id))
    
    
class getDeviceID(DatabaseQuery):
    return_type=ReturnTypes.SINGLE_ELEMENT(0)
    query_str = \
    f"""SELECT DeviceID FROM RegisteredDevices
    WHERE DeviceName = %s;"""
    def query(self, cursor, device_name):
        return cursor.execute(self.query_str, (device_name,))
    
    
class getDeviceName(DatabaseQuery):
    return_type=ReturnTypes.SINGLE_ELEMENT(1)
    query_str = \
    f"""SELECT DeviceName FROM RegisteredDevices
    WHERE DeviceID = %s;"""
    def query(self, cursor, device_id):
        return cursor.execute(self.query_str, (device_id,))
    
    
class getDeviceStatus(DatabaseQuery):
    return_type=ReturnTypes.SINGLE_ELEMENT(6)
    query_str = \
    f"""SELECT DeviceStatus FROM RegisteredDevices
    WHERE DeviceID = %s;"""
    def query(self, cursor, device_id):
        return cursor.execute(self.query_str, (device_id,))


class getDevices(DatabaseQuery):
    query_str = \
    """SELECT * FROM RegisteredDevices;"""
    def query(self, cursor):
        return cursor.execute(self.query_str)
    
    

class getDevice(DatabaseQuery):
    return_type=ReturnTypes.SINGLE_OBJECT()
    query_str = \
    f"""SELECT * FROM RegisteredDevices
    WHERE DeviceID = %s;"""
    def query(self, cursor, device_id):
        return cursor.execute(self.query_str, (device_id,))


class configureDevice(DatabaseQuery):
    query_str = \
    f"""UPDATE RegisteredDevices
    SET DeviceName = COALESCE(%(DeviceName)s, DeviceName), PresetID = COALESCE(%(PresetID)s, PresetID), ProjectID = COALESCE(%(ProjectID)s, ProjectID), DeviceStatus = COALESCE(%(DeviceStatus)s, DeviceStatus), DeviceIP = COALESCE(%(DeviceIP)s, DeviceIP)
    WHERE DeviceID = %(DeviceID)s;"""
    def query(self, cursor, device_id, device_name=None, preset_id=None, project_id=None, status=None, device_ip=None):
        return cursor.execute(self.query_str, {"DeviceName":device_name, "ProjectID":project_id, "PresetID":preset_id, "DeviceStatus": status, "DeviceID":device_id, "DeviceIP":device_ip})
    

class registerDevice(DatabaseQuery):
    query_str = \
    f"""INSERT INTO RegisteredDevices (DeviceName, PresetID, ProjectID, DeviceStatus)
    VALUES (%s, %s, %s, %s, %s, %s);"""
    def query(self, cursor, device_name, preset_id, project_id, device_status):
        return cursor.execute(self.query_str, (device_name, preset_id, project_id, device_status))
    
    
class reregisterDevice(DatabaseQuery):
    query_str = \
    f"""INSERT INTO RegisteredDevices (DeviceName, DeviceID, DeviceIP, DeviceMAC, PresetID, ProjectID, DeviceStatus)
    VALUES (%s, %s %s, %s, %s, %s, %s);"""
    def query(self, cursor, device_name, device_id, device_ip, device_mac, preset_id, project_id, device_status):
        return cursor.execute(self.query_str, (device_name, device_id, device_ip, device_mac, preset_id, project_id, device_status))


class deleteDevice(DatabaseQuery):
    query_str = \
    f"""DELETE FROM RegisteredDevices WHERE DeviceID = %s;"""
    def query(self, cursor, device_id):
        return cursor.execute(self.query_str, (device_id,))
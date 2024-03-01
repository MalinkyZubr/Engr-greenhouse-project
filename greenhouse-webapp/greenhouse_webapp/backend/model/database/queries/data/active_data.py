class getProjectData(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM Data WHERE ProjectID = %s;"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query_str, (project_id,))
    
    
class getProjectDataInRange(DatabaseQuery):
    query_str = \
    f"""SELECT * 
    FROM Data
    WHERE DateCollected 
            BETWEEN  str_to_date(%s, '%Y-%m-%d')
                AND   str_to_date(%s, '%Y-%m-%d')
        AND ProjectID = %s;"""
    def query(self, cursor, project_id, start_date, end_date):
        return cursor.execute(self.query_str, (start_date, end_date, project_id))
    
    
class insertData(DatabaseQuery):
    query_str = \
    f"""INSERT INTO Data (DeviceID, ProjectID, Temperature, Humidity, Moisture, LightExposure)
    VALUES (%s, %s, %s, %s, %s, %s, %s, %s);"""
    def query(self, cursor, device_id, project_id, temperature, humidity, moisture, light_exposure):
        return cursor.execute(self.query_str, (device_id, project_id, temperature, humidity, moisture, light_exposure))
    
    
class insertDataAtTime(DatabaseQuery):
    query_str = \
    f"""INSERT INTO Data (DeviceID, ProjectID, DateCollected, Temperature, Humidity, Moisture, LightExposure)
    VALUES (%s, %s, str_to_date(%s, '%Y-%m-%d %H:%i:%s'), %s, %s, %s, %s, %s, %s);"""
    def query(self, cursor, device_id, project_id, temperature, humidity, moisture, light_exposure, datetime_collected):
        return cursor.execute(self.query_str, (device_id, project_id, datetime_collected, temperature, humidity, moisture, light_exposure))
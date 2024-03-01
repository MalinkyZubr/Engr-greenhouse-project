class getPresetName(DatabaseQuery):
    return_type=ReturnTypes.SINGLE_ELEMENT(1)
    query_str = \
    f"""SELECT PresetName FROM Presets
    WHERE PresetID = %s;"""
    def query(self, cursor, preset_id):
        return cursor.execute(self.query_str, (preset_id,))
    
    
class getPresetID(DatabaseQuery):
    return_type=ReturnTypes.SINGLE_ELEMENT(0)
    query_str = \
    f"""SELECT PresetID FROM Presets WHERE PresetName = %s;"""
    def query(self, cursor, preset_name):
        return cursor.execute(self.query_str, (preset_name,))


class getPresets(DatabaseQuery):
    query_str = \
    """SELECT * FROM Presets;"""
    def query(self, cursor):
        return cursor.execute(self.query_str)


class getPreset(DatabaseQuery):
    return_type=ReturnTypes.SINGLE_OBJECT()
    query_str = \
    f"""SELECT * FROM Presets WHERE PresetID = %s;"""
    def query(self, cursor, preset_id):
        return cursor.execute(self.query_str, (preset_id,))


class createPreset(DatabaseQuery):
    query_str = \
    f"""INSERT INTO Presets (PresetName, Temperature, Humidity, Moisture, LightExposure, IRExposure)
    VALUES (%s, %s, %s, %s, %s, %s);"""
    def query(self, cursor, preset_name, temperature, humidity, moisture, light_exposure, ir_exposure):
        return cursor.execute(self.query_str, (preset_name, temperature, humidity, moisture, light_exposure, ir_exposure))


class updatePreset(DatabaseQuery):
    query_str = \
    f"""UPDATE Presets
    SET PresetName = COALESCE(%(PresetName)s, PresetName), Temperature = COALESCE(%(Temperature)s, Temperature), Humidity = COALESCE(%(Humidity)s, Humidity), Moisture = COALESCE(%(Moisture)s, Moisture), LightExposure = COALESCE(%(LightExposure)s, LightExposure), IRExposure = COALESCE(%(IRExposure)s, IRExposure)
    WHERE PresetID = %(presetID)s;"""
    def query(self, cursor, preset_id, preset_name=None, temperature=None, humidity=None, moisture=None, light_exposure=None, ir_exposure=None):
        return cursor.execute(self.query_str, {"PresetName":preset_name, "Temperature":temperature, "Humidity":humidity, "Moisture":moisture, "LightExposure":light_exposure, "IRExposure":ir_exposure, "presetID":preset_id})


class deletePreset(DatabaseQuery):
    query_str = \
    f"""DELETE FROM Presets WHERE PresetID = %s;
    UPDATE RegisteredDevices
    SET PresetID = -1
    WHERE PresetID = %s;"""
    def query(self, cursor, preset_id):
        return cursor.execute(self.query_str, (preset_id, preset_id))
    
    
class getPresetAssociatedDevices(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM RegisteredDevices WHERE PresetID = %s;"""
    def query(self, cursor, preset_id):
        return cursor.execute(self.query_str, (preset_id,))
    
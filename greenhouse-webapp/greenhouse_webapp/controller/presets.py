from pydantic import BaseModel, Field
from starlette.responses import FileResponse

from DBIntRouter import APIDRouter
from frontend_paths import PRESET, CREATE_PRESET

router = APIDRouter(
    prefix="/presets"
)


class PresetSchema(BaseModel):
    daytime_temp: float = Field(gt=0)
    humidity: float = Field(gt=0, lt=100)
    moisture: float = Field(gt=0, lt=100)
    light_exposure: float = Field(gt=0, lt=100000)
    ir_exposure: float = Field(gt=0, lt=100000)
    
    
@router.get("/")
async def load_webpage():
    """
    load the webpage for presets
    """
    return FileResponse(PRESET)
    
@router.get("/list_presets")
async def list_presets():
    """
    List the current presets loaded into the database
    """
    presets = router.database_connector.execute('getPresets')
    presets = [preset[2] for preset in presets]
    return presets

@router.get("/{preset_name}")
async def show_preset(preset_name: str):
    """
    Retrieve all the features of a specific preset
    """
    preset_id = router.database_connector.execute("getPresetID", preset_name)
    preset = router.database_connector.execute('getPreset', preset_id)
    return preset

@router.get("/createPreset")
async def get_creation_page():
    """
    display the preset creation page
    """
    return FileResponse(CREATE_PRESET)

@router.post("/createPreset/{preset_name}")
async def create_preset(preset_name: str, preset_info: PresetSchema):
    """
    Create a preset and load it into the database
    """
    router.database_connector.execute('createPreset', 
        preset_name, 
        preset_info.daytime_temp, 
        preset_info.humidity,
        preset_info.moisture,
        preset_info.light_exposure,
        preset_info.ir_exposure)
    return "Successfully Created New Preset!"
    
@router.put("/{preset_name}")
async def update_preset(preset_name, preset_info):
    """
    update a preset's fields
    """
    router.database_connector.execute('updatePreset', 
        router.database_connector.execute('getPresetID', preset_name),
        preset_name=preset_name, 
        temperature=preset_info.daytime_temp, 
        humidity=preset_info.humidity,
        moisture=preset_info.moisture,
        light_exposure=preset_info.light_exposure,
        ir_exposure=preset_info.ir_exposure)
    return "Successful update"

@router.delete("/{preset_name}")
async def delete_preset(preset_name):
    """
    delete a specified preset
    """
    preset_id = router.database_connector.execute('getPresetID', preset_name)
    router.database_connector.execute("deletePreset", preset_id)
    return "Successfully deleted!"
    
@router.get("/{preset_name}/devices")
async def get_associated_devices(preset_name):
    """
    get a list of all devices associated with the select preset
    """
    preset_id = router.database_connector.execute('getPresetID', preset_name)
    devices = router.database_connector.execute("getPresetAssociatedDevices", preset_id)
    return devices
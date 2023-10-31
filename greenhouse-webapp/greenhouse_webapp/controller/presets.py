from pydantic import BaseModel, Field
from DBIntRouter import APIDRouter

router = APIDRouter(
    prefix="/presets"
)


class PresetSchema(BaseModel):
    preset_name: str
    daytime_temp: float = Field(gt=0)
    humidity: float = Field(gt=0, lt=100)
    moisture: float = Field(gt=0, lt=100)
    light_exposure: float = Field(gt=0, lt=100000)
    ir_exposure: float = Field(gt=0, lt=100000)
    
    
@router.get("/")
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

@router.post("/createPreset")
async def create_preset(preset_info: PresetSchema):
    """
    Create a preset and load it into the database
    """
    router.database_connector.execute('createPreset', 
        preset_info.preset_name, 
        preset_info.daytime_temp, 
        preset_info.humidity,
        preset_info.moisture,
        preset_info.light_exposure,
        preset_info.ir_exposure)

@router.put("/{preset_name}")
async def update_preset():
    """
    update a preset's fields
    """
    pass

@router.delete()
async def delete_preset():
    """
    delete a specified preset
    """
    pass
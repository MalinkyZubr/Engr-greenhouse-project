from pydantic import BaseModel, Field
from starlette.responses import FileResponse
from fastapi import Request
from fastapi.responses import JSONResponse, HTMLResponse
from jinja2 import Template
from requests import Response

from greenhouse_webapp.controller.APIDRouter import APIDRouter
from controller.frontend_paths import PRESET, CREATE_PRESET
from controller.device_management import change_preset, DeviceRegistrationPreset

router = APIDRouter(
    prefix="/presets"
)


class PresetSchema(BaseModel):
    daytime_temp: float = Field(gt=0)
    humidity: float = Field(gt=0, lt=100)
    moisture: float = Field(gt=0, lt=100)
    light_exposure: float = Field(gt=0, lt=100000)
    
    
@router.get("/availablePresets/{preset_name}")
async def load_webpage(request: Request, preset_name: str) -> HTMLResponse:
    """load the webpage for the preset in question

    Args:
        request (Request): request requried for serving the template page
        preset_name (str): preset name to get the webpage for

    Returns:
        Template: webpage template to be rendered
    """
    return router.template_paths.TemplateResponse("preset.html", {"request":request, "preset_name":preset_name})
    
@router.get("/list_presets")
async def list_presets() -> list[tuple]:
    """return a list of all presets

    Returns:
        list[tuple]: list of presets available
    """
    presets: list[tuple] = router.database_connector.execute('getPresets')
    
    return presets

@router.get("/availablePresets/{preset_name}/preset_info")
async def show_preset(preset_name: str) -> tuple:
    """return data for the specified preset

    Args:
        preset_name (str): name of the desired preset

    Returns:
        tuple: all features of the preset
    """
    preset_id: int = router.database_connector.execute("getPresetID", preset_name)
    preset: tuple = router.database_connector.execute('getPreset', preset_id)
    
    return preset

@router.get("/createPreset")
async def get_creation_page() -> FileResponse:
    """return the webpage for creating a new preset

    Returns:
        FileResponse: static html page for preset creation
    """
    return FileResponse(CREATE_PRESET)

@router.post("/createPreset/{preset_name}")
async def create_preset(preset_name: str, preset_info: PresetSchema) -> JSONResponse:
    """create a preset and load it into the database

    Args:
        preset_name (str): name of the new preset
        preset_info (PresetSchema): schema containing critical information about preset to be used in database entry

    Returns:
        JSONResponse: success message
    """
    router.database_connector.execute('createPreset', 
        preset_name, 
        preset_info.daytime_temp, 
        preset_info.humidity,
        preset_info.moisture,
        preset_info.light_exposure,
        preset_info.ir_exposure)
    return JSONResponse("Successfully Created New Preset!", 200)
    
@router.put("/availablePresets/{preset_name}")
async def update_preset(preset_name: str, preset_info: PresetSchema) -> JSONResponse:
    """update a preset and sync all connected devices to the new settings

    Args:
        preset_name (str): preset name to modify
        preset_info (PresetSchema): schema containing all necessary preset information

    Returns:
        JSONResponse: successful return
    """
    preset_id: int = router.database_connector.execute('getPresetID', preset_name)
    router.database_connector.execute('updatePreset', 
        preset_id=preset_id,
        preset_name=preset_name, 
        temperature=preset_info.daytime_temp, 
        humidity=preset_info.humidity,
        moisture=preset_info.moisture,
        light_exposure=preset_info.light_exposure)
    
    connected_devices: list[tuple] = router.database_connector.execute("getPresetAssociateDevices", preset_id)
    
    request_body: DeviceRegistrationPreset = DeviceRegistrationPreset(
        temperature = preset_info.daytime_temp,
        humidity=preset_info.humidity,
        moisture=preset_info.moisture,
        hours_daylight=preset_info.light_exposure,
        preset_id=preset_id) # make sure that the frontend is actually sending hours daylight
    
    for device in connected_devices:
        response: Response = await change_preset.call(device[2], data=request_body)
    
    return JSONResponse("Successful update", 200)

@router.delete("/availablePresets/{preset_name}")
async def delete_preset(preset_name: str) -> JSONResponse:
    """delete a preset, and remove the preset from all associated devices

    Args:
        preset_name (str): name of the preset to delete

    Returns:
        JSONResponse: successful response 200
    """
    preset_id: int = router.database_connector.execute('getPresetID', preset_name)
    connected_devices: list[tuple] = router.database_connector.execute("getPresetAssociateDevices", preset_id)

    router.database_connector.execute("deletePreset", preset_id)
    
    request_body: DeviceRegistrationPreset = DeviceRegistrationPreset(
        temperature=0,
        humidity=0,
        moisture=0,
        hours_daylight=0,
        preset_id=-1) # make sure that the frontend is actually sending hours daylight
    
    for device in connected_devices:
        
        response: Response = await change_preset.call(device[2], data=request_body)
        
    return JSONResponse("Successfully deleted!", 200)
    
@router.get("/availablePresets/{preset_name}/devices")
async def get_associated_devices(preset_name: str) -> list[tuple | None]:
    """get list of devices associated with the preset

    Args:
        preset_name (str): name of the preset to get the associated devices of

    Returns:
        list[tuple | None]: list of aassociated devices
    """
    preset_id = router.database_connector.execute('getPresetID', preset_name)
    devices = router.database_connector.execute("getPresetAssociatedDevices", preset_id)
    
    return devices
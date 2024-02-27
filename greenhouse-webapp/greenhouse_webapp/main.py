import sys
sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")

from fastapi import Depends, FastAPI, Request
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from typing import Annotated, Literal
from fastapi.responses import RedirectResponse
import uvicorn
import asyncio
import os
import json
from starlette.responses import FileResponse

from controller.device_interface import router as device_interface_router
from controller.device_management import router as device_management_router
from controller.presets import router as preset_router
from controller.projects import router as project_router
from controller.frontend_paths import DASHBOARD, CSS, STATIC, JS, CONFIG, ICON, LOCAL_DIR
from model.data_interface import DatabaseInterface
from model.device_initialization import DeviceManager, Device
from model.data_retrieving import DataRetriever


SERVER_CONFIG = os.path.join(CONFIG, "controller.json")


database_connector = DatabaseInterface()
device_manager = DeviceManager("huehue", database_connector)
data_visualizer = DataRetriever()

routers = {'device_interface':device_interface_router,
           'device_manager':device_management_router,
           'presets':preset_router,
           'projects':project_router}

cors_middleware = {
    "allow_origins":["*"],
    "allow_credentials":True,
    "allow_methods":["*"],
    "allow_headers":["*"]
}

for router in routers.values():
    router.database_connector = database_connector
    router.device_manager = device_manager
    router.data_retriever = data_visualizer
    
app = FastAPI()
app.mount("/static", StaticFiles(directory=STATIC), name="static")
app.mount("/scripts", StaticFiles(directory=JS), name="js")

app.include_router(device_interface_router)
app.include_router(device_management_router)
app.include_router(preset_router)
app.include_router(project_router)

app.add_middleware(
    CORSMiddleware,
    **cors_middleware
)


@app.middleware("http")
async def check_aging(request: Request, next_call): # auto updates the device status
    """Critical middleware which governs device status. Any device that goes a specified period of time without sending a message to the server will be designated 'disconnected'
    This middleware tracks every request made by devices to the server, and resets their disconnect timers accordingly. Each request is embedded with a cookie contianing the device id
    which allows easy tracking in tandem with the DeviceManager in the router. 
    
    Args:
        request (Request): raw request containing all encapsulated data from the device
        next_call (api call): the next api call to be executed should all middleware checks pass

    Returns:
        response, Any: the response to the next_call the device is requesting
    """
    if "/interface" in str(request.url):
        reported_device_id: int = int(request.cookies["device_id"])
        reported_device_status: Literal["ACTIVE", "IDLE"] = request.cookies["status"]
        
        router.database_connector.execute("configureDevice", reported_device_id, status=reported_device_status)
            
        active_list_device_entry: Device = router.device_manager.active_device_list[reported_device_id]
            
        active_list_device_entry.update_time() # make sure the timeout doesnt go off...
        
    response = await next_call(request)
    
    return response  


# @app.exception_handler(404)
# async def handle404(_, __):
#     return RedirectResponse("/")

def load_config():
    with open(SERVER_CONFIG, 'r') as f:
        return json.loads(f.read())

@app.get("/")
async def serve_dashboard():
    return FileResponse(DASHBOARD)

@app.get("/config")
async def serve_config():
    # to deprecate when solidified
    return load_config()

@app.get("/test")
async def test():
    return "hi"

if __name__ == "__main__":
    config_data = load_config()
    uvicorn.run(app, host=config_data['host'], port=config_data['port'])
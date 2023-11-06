import sys
sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")

from fastapi import Depends, FastAPI
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from typing import Annotated
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
from model.device_initialization import DeviceManager

SERVER_CONFIG = os.path.join(CONFIG, "controller.json")

database_connector = DatabaseInterface()
device_manager = DeviceManager("huehue")

routers = {'device_interface':device_interface_router,
           'device_manager':device_management_router,
           'presets':preset_router,
           'projects':project_router}

for router in routers.values():
    router.database_connector = database_connector
    router.device_manager = device_manager

app = FastAPI()
app.mount("/static", StaticFiles(directory=STATIC), name="static")
app.mount("/scripts", StaticFiles(directory=JS), name="js")
app.mount("/config", StaticFiles(directory=CONFIG), name="config")
app.mount("/favicon.ico", StaticFiles(directory=ICON), name="favicon.ico")

app.include_router(device_interface_router)
app.include_router(device_management_router)
app.include_router(preset_router)
app.include_router(project_router)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"]
)
# @app.exception_handler(404)
# async def handle404(_, __):
#     return RedirectResponse("/")

@app.get("/")
async def serve_dashboard():
    return FileResponse(DASHBOARD)


@app.get("/test")
async def hi():
    return "hi"

def load_config():
    with open(SERVER_CONFIG, 'r') as f:
        return json.loads(f.read())

if __name__ == "__main__":
    config_data = load_config()
    uvicorn.run(app, host=config_data['host'], port=config_data['port'])
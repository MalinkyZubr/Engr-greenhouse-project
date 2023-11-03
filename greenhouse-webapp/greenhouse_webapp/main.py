import sys
sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")

from fastapi import Depends, FastAPI
from fastapi.staticfiles import StaticFiles
from typing import Annotated
from fastapi.responses import RedirectResponse
import uvicorn
import asyncio
from starlette.responses import FileResponse

from controller.device_interface import router as device_interface_router
from controller.device_management import router as device_management_router
from controller.presets import router as preset_router
from controller.projects import router as project_router
from controller.frontend_paths import DASHBOARD, CSS, STATIC
from model.data_interface import DatabaseInterface
from model.device_initialization import DeviceManager

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

app.include_router(device_interface_router)
app.include_router(device_management_router)
app.include_router(preset_router)
app.include_router(project_router)

@app.exception_handler(404)
async def handle404(_, __):
    return RedirectResponse("/")

@app.get("/")
async def serve_dashboard():
    return FileResponse(DASHBOARD)


@app.get("/hi")
async def hi():
    return "hi"


if __name__ == "__main__":
    uvicorn.run(app, host="127.0.0.1", port=8000)
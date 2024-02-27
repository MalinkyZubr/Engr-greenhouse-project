from pydantic import BaseModel, Field
from typing import Optional, Literal
from fastapi.staticfiles import StaticFiles
from starlette.responses import FileResponse
from fastapi import Request
from fastapi.responses import HTMLResponse, JSONResponse
from jinja2 import Template
from requests import Response

from greenhouse_webapp.controller.APIDRouter import APIDRouter
from controller.frontend_paths import PROJECT, CREATE_PROJECT, STATIC, CSS, ARCHIVE, CONFIG
from controller.device_management import change_identifier, DeviceConfigurationSchema
from model.timing import MAX_TIME, MIN_TIME, get_today_tomorrow, convert_time_formats

router = APIDRouter(
    prefix="/projects"
)

router.mount("/projects/static", StaticFiles(directory=STATIC), name="static")
    

class DateQuerySchema(BaseModel): # add input validation here
    start_date: str
    end_date: str
        
    
@router.get("/projects/{project_name}")
async def serve_project_webpage(request: Request, project_name: str) -> HTMLResponse:
    """serve the template for the project requested

    Args:
        request (Request): raw request necessary for template
        project_name (str): name of the project to serve the webpage for

    Returns:
        Template: webpage template for project
    """
    request.scope.pop("path_params") # dunno what this does but im too scared to remove it, works fine with it here, no time to figure it out
    
    return router.template_paths.TemplateResponse("project.html", {"request":request, "project_name":project_name}) # find a way to integrate project name

@router.post("/createProject/{project_name}")
async def create_project(project_name: str) -> JSONResponse: # add the description field to store in database
    """route to create a new project in the database

    Args:
        project_name (str): name for the new project

    Returns:
        JSONResponse: success response 200
    """
    router.database_connector.execute('addProject',
        project_name)
    
    return JSONResponse("successfully created", 200)

@router.get("/createProject")
async def load_project_creation_page() -> FileResponse:
    """load a static webpage from which the user can create a new project

    Returns:
        FileResponse: static webpage for project creation
    """
    return FileResponse(CREATE_PROJECT)

@router.put("/project/{project_name}")
async def update_project(project_name: str, new_name: str) -> JSONResponse:
    """update a project based on its name, give it a new name

    Args:
        project_name (str): current name of project
        new_name (str): name to change project to

    Returns:
        JSONResponse: 200 success response
    """
    project_id: int = router.database_connector.execute("getProjectID", project_name)
    router.database_connector.execute('updateProject', project_id, new_name)
    
    return JSONResponse("successfully updated", 200)
    
@router.get("/list_projects")
async def list_projects() -> list[tuple | None] | None:
    """get a list of projects currently loaded to the database

    Returns:
        list[tuple | None]: list of projects to return
    """
    projects: list[tuple] = router.database_connector.execute("getActiveProjects")
    
    return projects

@router.get("/projects/{project_name}/projectinfo")
async def get_project(project_name: str) -> tuple:
    """route to get data on a specific project

    Args:
        project_name (str): name of project to get data on

    Returns:
        tuple: project data to return
    """
    project_id: int = router.database_connector.execute("getProjectID", project_name)
    project: tuple = router.database_connector.execute("getProject", project_id)
    
    return project

@router.get("/projects/{project_name}/data")
async def get_project_data(project_name: str) -> list[tuple | None]:
    """get all data associated with a specific project

    Args:
        project_name (str): project name to get data from

    Returns:
        list[tuple | None]: list of datapoints associated with the project
    """
    project_id: int = router.database_connector.execute("getProjectID", project_name)
    data_points: list[tuple] = router.database_connector.execute("getProjectData", project_id)
    
    return data_points

@router.get("/projects/{project_name}/download_csv")
async def download_project_data_csv(project_name: str) -> FileResponse:
    """gather all project data, and package it in a csv file for further analysis later

    Args:
        project_name (str): project name to get csv for

    Returns:
        FileResponse: csv file containing all project data
    """
    project_id: int = router.database_connector.execute("getProjectID", project_name)
    data_points: list[tuple] = router.database_connector.execute("getProjectData", project_id)
    csv_path: str = router.data_retriever.generate_csv(data_points, project_name)
    
    return FileResponse(path=csv_path, media_type="application/octet-stream", filename=f"{project_name}.csv")
    
@router.post("/projects/{project_name}/data_visualization/{data_type}")
async def get_project_data_visualized(project_name: str, data_type: Literal["Temperature", "Moisture", "LightExposure", "Humidity"], date_information: DateQuerySchema) -> FileResponse:
    """Gather data from a range of dates to be visualized on the web portal

    Args:
        project_name (str): name of project to get data date range for
        data_type (Literal[&quot;Temperature&quot;, &quot;Moisture&quot;, &quot;LightExposure&quot;, &quot;Humidity&quot;]): what data type to retrieve for visualization

    Returns:
        FileResponse: PNG image of visualized data
    """
    if not date_information.start_date and not date_information.end_date:
        date_information.start_date, date_information.end_date = get_today_tomorrow()
    else:
        date_information.start_date, date_information.end_date =  convert_time_formats(date_information.start_date), convert_time_formats(date_information.end_date)
        
    project_id: int = router.database_connector.execute("getProjectID", project_name)
    data_points: list[tuple] = router.database_connector.execute("getProjectDataInRange", project_id, date_information.start_date, date_information.end_date)
    image_path: str = router.data_retriever.generate_data_image(data_points, data_type, project_name, router.database_connector)
    
    return FileResponse(image_path, media_type="image/png")

@router.get("/projects/{project_name}/devices")
async def get_associated_devices(project_name: str) -> list[tuple]:
    """get all devices associated with the project in question

    Args:
        project_name (str): project name to get devices for

    Returns:
        list[tuple]: list of associated devices
    """
    project_id: int = router.database_connector.execute("getProjectID", project_name)
    devices: list[tuple] = router.database_connector.execute("getProjectDevices", project_id)
    
    return devices

@router.patch("projects/{project_name}/archive")
async def archive_project(project_name: str) -> JSONResponse:
    """archive a project. This involves moving all of the project data to the ArchiveData table, and unassigning all devices from that project

    Args:
        project_name (str): name of project to archive
    Returns:
        JSONResponse: 200 success
    """
    project_id: int = router.database_connector.execute("getProjectID", project_name)
    
    project_associated_devices: list[tuple] = router.database_connector.execute("getProjectDevices", project_id)
    
    router.database_connector.execute("archiveProject", project_id)
    
    configuration_message: DeviceConfigurationSchema = DeviceConfigurationSchema(
        device_name="",
        device_id="",
        project_id=-1
    )
    
    for device in project_associated_devices:
        device_ip: str = device[2]
        device_id: int = device[0]
        
        configuration_message.device_name = device[1]
        configuration_message.device_id = device_id
        
        response: Response = await change_identifier.call(device_ip, configuration_message)
        
    return JSONResponse("successfully archived project", 200)

@router.delete("projects/{project_name}")
async def delete_project(project_name:str) -> JSONResponse:
    """delete a project from the archived projects list, along with all of its data

    Args:
        project_name (str): project name to have deleted from archived projects

    Returns:
        JSONResponse: 200 success
    """
    project_id: int = router.database_connector.execute("getProjectID", project_name)
    router.database_connector.execute("deleteProject", project_id)
    
    return JSONResponse("successfully deleted project", 200)
    
@router.get("/archived/{project_name}")
async def serve_archived_webpage(project_name: str, request: Request) -> HTMLResponse:
    """serve the webpage template for an archived project

    Args:
        project_name (str): name of the archived project to serve
        request (Request): request necessary for serving template

    Returns:
        Template: template to serve to the frontend client
    """
    return router.template_paths.TemplateResponse("archived.html", {"request":request, "project_name":project_name})

@router.get("/archived/projects")
async def get_archived_projects() -> list[tuple | None]:
    """list the archived projects

    Returns:
        list[tuple | None]: archived projects list
    """
    return router.database_connector.execute("getArchivedProjects")

@router.get("/archived/{archived_project_name}")
async def get_archived_project(archived_project_name: str) -> tuple:
    """get a specific archived project and all its characteristics

    Args:
        archived_project_name (str): the name of the archived project to get data from

    Returns:
        tuple: project data retrieved from the database
    """
    project_id: int = router.database_connector.execute("getArchiveProjectID", archived_project_name)
    
    return router.database_connector.execute("getArchivedProject", project_id)
    

@router.get("/archived/{archived_project_name}/data")
async def get_archived_data(archived_project_name: str) -> list[tuple | None]:
    """get the archived data associated with the archived project

    Args:
        archived_project_name (str): name of the archived project from which to get data

    Returns:
        list[tuple]: data list for the archived project
    """
    project_id: int = router.database_connector.execute("getArchiveProjectID", archived_project_name)
    
    return router.database_connector.execute("getProjectData", project_id)
    
    
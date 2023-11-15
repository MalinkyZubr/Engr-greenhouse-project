from pydantic import BaseModel, Field
from typing import Optional
from fastapi.staticfiles import StaticFiles
from starlette.responses import FileResponse
from fastapi import Request
from fastapi.responses import HTMLResponse

from controller.DBIntRouter import APIDRouter
from controller.frontend_paths import PROJECT, CREATE_PROJECT, STATIC, CSS, ARCHIVE, CONFIG
from model.timing import MAX_TIME, MIN_TIME, get_today_tomorrow, convert_time_formats

router = APIDRouter(
    prefix="/projects"
)

router.mount("/projects/static", StaticFiles(directory=STATIC), name="static")


class ActiveProjectSchema(BaseModel):
    project_name: str
    devices: list[str]
    
    
class DateQuerySchema(BaseModel): # add input validation here
    start_date: str | int
    end_date: str | int
    
    
def reassign_devices(project_info: ActiveProjectSchema, project_id: int):
    for device_name in project_info.devices:
        id = router.database_connector.execute('getDeviceID', device_name)
        router.database_connector.execute('configureDevice', 
            id,
            project_id=project_id)
    
@router.get("/projects/{project_name}")
async def serve_project_webpage(request: Request, project_name):
    """
    serve the webpage for the projects page
    """
    request.scope.pop("path_params")
    return router.template_paths.TemplateResponse("project.html", {"request":request, "project_name":project_name}) # find a way to integrate project name

@router.get("/projects/assignProject")
async def serve_project_assignment_page(request: Request, device_name):
    return router.template_paths.TemplateResponse("assign_project.html", {"request":request, "device_name": device_name})

@router.post("/createProject/{project_name}")
async def create_project(project_name, project_info: ActiveProjectSchema): # add the description field to store in database
    """
    Create a new project and put it into the database
    """
    router.database_connector.execute('addProject',
        project_name)
    project_id=router.database_connector.execute('getProjectID', project_info.project_name)
    reassign_devices(project_info, project_id)
    return "successfully created"

@router.get("/createProject")
async def load_project_creation_page():
    """
    Load the page for project creation
    """
    return FileResponse(CREATE_PROJECT)

@router.put("/project/{project_name}")
async def update_project(project_name: str, project_info: ActiveProjectSchema):
    """
    Update a project definition
    """
    project_id = router.database_connector.execute("getProjectID", project_name)
    router.database_connector.execute('updateProject', project_id, project_info.project_name)
    reassign_devices(project_info, project_id)
    return "successfully updated"
    
@router.get("/list_projects")
async def list_projects():
    """
    List currently available projects
    """
    projects = router.database_connector.execute("getActiveProjects")
    return projects

@router.get("/projects/{project_name}/projectinfo")
async def get_project(project_name):
    """
    get a specified project
    """
    project_id = router.database_connector.execute("getProjectID", project_name)
    project = router.database_connector.execute("getProject", project_id)
    return project

@router.get("/projects/{project_name}/data")
async def get_project_data(project_name):
    project_id = router.database_connector.execute("getProjectID", project_name)
    data_points = router.database_connector.execute("getProjectData", project_id)
    return data_points

@router.get("/projects/{project_name}/download_csv")
async def download_project_data_csv(project_name):
    project_id = router.database_connector.execute("getProjectID", project_name)
    data_points = router.database_connector.execute("getProjectData", project_id)
    csv_path = router.data_retriever.generate_csv(data_points, project_name)
    
    return FileResponse(path=csv_path, media_type="application/octet-stream", filename=f"{project_name}.csv")
    
@router.post("/projects/{project_name}/data_visualization/{data_type}")
async def get_project_data_visualized(project_name, data_type, date_information: DateQuerySchema) -> FileResponse:
    if not date_information.start_date and not date_information.end_date:
        date_information.start_date, date_information.end_date = get_today_tomorrow()
    else:
        date_information.start_date, date_information.end_date =  convert_time_formats(date_information.start_date), convert_time_formats(date_information.end_date)
        
    project_id = router.database_connector.execute("getProjectID", project_name)
    data_points = router.database_connector.execute("getProjectDataInRange", project_id, date_information.start_date, date_information.end_date)
    image_path = router.data_retriever.generate_data_image(data_points, data_type, project_name, router.database_connector)
    
    return FileResponse(image_path, media_type="image/png")

@router.get("/projects/{project_name}/devices")
async def get_associated_devices(project_name):
    project_id = router.database_connector.execute("getProjectID", project_name)
    devices = router.database_connector.execute("getProjectDevices", project_id)
    return devices

@router.patch("projects/{project_name}/archive")
async def archive_project(project_name):
    """
    archive a project so that it is inactive, but can still be used later
    """
    project_id = router.database_connector.execute("getProjectID", project_name)
    router.database_connector.execute("archiveProject", project_id)

@router.delete("projects/{project_name}")
async def delete_project(project_name):
    """
    permanently delete a project from the database (archived only)
    """
    project_id = router.database_connector.execute("getProjectID", project_name)
    router.database_connector.execute("deleteProject", project_id)
    return "successfully deleted project"
    
@router.get("/archived/{project_name}")
async def serve_archived_webpage(project_name, request: Request):
    """
    serve the archivist webpage
    """
    return router.template_paths.TemplateResponse("archived.html", {"request":request, "project_name":project_name})

@router.get("/archived/projects")
async def get_archived_projects():
    """
    get as list of archived projects
    """
    return router.database_connector.execute("getArchivedProjects")

@router.get("/archived/{archived_project_name}")
async def get_archived_project(archived_project_name):
    project_id = router.database_connector.execute("getArchiveProjectID", archived_project_name)
    return router.database_connector.execute("getArchivedProject", project_id)
    

@router.get("/archived/{archived_project_name}/data")
async def get_archived_data(archived_project_name):
    """
    get archived data of archived project
    """
    project_id = router.database_connector.execute("getArchiveProjectID", archived_project_name)
    return router.database_connector.execute("getProjectData", project_id)
    
    
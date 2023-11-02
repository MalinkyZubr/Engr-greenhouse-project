from DBIntRouter import APIDRouter
from pydantic import BaseModel, Field
from starlette.responses import FileResponse

from frontend_paths import PROJECT, CREATE_PROJECT

router = APIDRouter(
    prefix="/projects"
)


class ActiveProjectSchema(BaseModel):
    project_name: str
    devices: list[str]
    
    
def reassign_devices(project_info: ActiveProjectSchema, project_id: int):
    for device_name in project_info.devices:
        id = router.database_connector.execute('getDeviceID', device_name)
        router.database_connector.execute('configureDevice', 
            id,
            project_id=project_id)
    
@router.get("/{project_name}")
async def serve_project_webpage(project_name):
    """
    serve the webpage for the projects page
    """
    return FileResponse(PROJECT) # find a way to integrate project name

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

@router.put("/{project_name}")
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

@router.get("/{project_name}")
async def get_project(project_name):
    """
    get a specified project
    """
    project_id = router.database_connector.execute("getProjectID", project_name)
    project = router.database_connector.execute("getProject", project_id)
    return project

@router.get("{project_name}/data")
async def get_project_data(project_name):
    project_id = router.database_connector.execute("getProjectID", project_name)
    data_points = router.database_connector.execute("getProjectData", project_id)
    return data_points

@router.get("{project_name}/devices")
async def get_associated_devices(project_name):
    project_id = router.database_connector.execute("getProjectID", project_name)
    devices = router.database_connector.execute("getProjectDevices", project_id)
    return devices

@router.patch("/{project_name}")
async def archive_project(project_name):
    """
    archive a project so that it is inactive, but can still be used later
    """
    project_id = router.database_connector.execute("getProjectID", project_name)
    router.database_connector.execute("archiveProject", project_id)

@router.delete("/{project_name}")
async def delete_project(project_name):
    """
    permanently delete a project from the database (archived only)
    """
    project_id = router.database_connector.execute("getProjectID", project_name)
    router.database_connector.execute("deleteProject", project_id)
    return "successfully deleted project"
    
@router.get("/archived")
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
    
    
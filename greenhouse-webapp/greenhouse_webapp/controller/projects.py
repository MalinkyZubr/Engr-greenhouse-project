from DBIntRouter import APIDRouter
from pydantic import BaseModel, Field

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


@router.post("/createProject")
async def create_project(project_info: ActiveProjectSchema):
    """
    Create a new project and put it into the database
    """
    router.database_connector.execute('addProject',
        project_info.project_name)
    project_id=router.database_connector.execute('getProjectID', project_info.project_name)
    reassign_devices(project_info, project_id)
    return "successfully created"

@router.put("/{project_name}")
async def update_project(project_name: str, project_info: ActiveProjectSchema):
    """
    Update a project definition
    """
    project_id = router.database_connector.execute("getProjectID", project_name)
    router.database_connector.execute('updateProject', project_id, project_info.project_name)
    reassign_devices(project_info, project_id)
    return "successfully updated"
    
@router.get("/")
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
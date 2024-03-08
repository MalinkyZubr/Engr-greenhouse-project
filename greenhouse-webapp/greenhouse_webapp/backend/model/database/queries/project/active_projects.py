from pydantic import BaseModel
from model.database.queries.base_query import NoReturnQuery, MetadataListQuery, MetadataObjectQuery
from model.database.queries.query_schemas_shared import QueryByID, QueryByName, QueryNameID


class getProjectID(MetadataObjectQuery[QueryByName]):
    query_str = \
    f"""SELECT ProjectID FROM ActiveProjects WHERE ProjectName = %(name)s;"""


class getActiveProjects(MetadataListQuery()[None]):
    query_str = \
    f"""SELECT * FROM ActiveProjects;"""


class getProject(MetadataObjectQuery[QueryByID]):
    query_str = \
    f"""SELECT * FROM ActiveProjects WHERE ProjectID = %(id)s;"""
    
    
class CreateProjectQuery(QueryByName):
    project_description: str = ""
    
    
class addProject(NoReturnQuery[CreateProjectQuery]):
    query_str = \
    f"""INSERT INTO ActiveProjects (ProjectName, ProjectDescription)
    VALUES (%(name)s, %(project_description)s);"""


class UpdateProjectQuery(CreateProjectQuery, QueryByID):
    pass

    
class updateProject(NoReturnQuery[UpdateProjectQuery]):
    query_str = \
    f"""UPDATE ActiveProjects
    SET ProjectName = COALESCE(%(name)s, ProjectName), ProjectDescription = COALESCE(%(project_description), ProjectDescription)
    WHERE ProjectID = %(id)s;"""


class archiveProject(NoReturnQuery[QueryByID]):
    query_str = \
    f"""INSERT INTO ArchivedProjects(ProjectID, ProjectName, DateStarted)
    VALUES SELECT * FROM ActiveProjects
    WHERE ProjectID = %(id)s;
    DELETE FROM ActiveProjects WHERE ProjectID = %(id)s;
    
    INSERT INTO ArchivedData(DeviceID, ProjectID, DateCollected, Temperature, Humidity, Moisture, LightExposure)
    VALUES SELECT * FROM Data
    WHERE ProjectID = %(id)s
    DELETE FROM Data WHERE ProjectID = %(id)s;
    
    UPDATE RegisteredDevices
    SET ProjectID = -1
    WHERE ProjectID = %(id)s;"""


class deleteProject(NoReturnQuery[QueryByID]):
    query_str = \
    f"""DELETE FROM ArchivedProjects WHERE ProjectID = %(id)s;
    DELETE FROM ArchivedData WHERE ProjectID = %(id)s;"""
    
    
class getProjectName(MetadataObjectQuery[QueryByID]):
    query_str = \
    f"""SELECT ProjectName FROM ActiveProjects
    WHERE ProjectID = %(id)s;"""
    
    
class getProjectDevices(MetadataListQuery[QueryByID]):
    query_str = \
    f"""SELECT * FROM RegisteredDevices WHERE ProjectID = %(id)s;"""
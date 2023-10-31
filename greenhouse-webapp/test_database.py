import pytest
from greenhouse_webapp.model.data_interface import DatabaseInterface


database_interface = DatabaseInterface()

print(database_interface.execute("getActiveProjects"))

projects = database_interface.execute("getActiveProjects")

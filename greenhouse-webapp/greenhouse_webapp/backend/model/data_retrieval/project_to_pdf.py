import os
from jinja2 import Template, Environment, PackageLoader, select_autoescape
from xhtml2pdf import pisa
from model.database.database_manager import METADATA_OBJECT, METADATA_LIST, DATA_SCHEMA
from model.data_retrieval.data_conversions import convert_to_chart, generate_file_name


LOCAL_DIR = os.path.dirname(os.path.abspath(__file__))
TEMP_PDF = os.path.join(LOCAL_DIR, r"/temp-pdf")


env: Environment = Environment(
    loader=PackageLoader(LOCAL_DIR),
    autoescape=select_autoescape()
)
template: Template = env.get_template("project_pdf_template.html")

    
def generate_html(project_metadata: METADATA_OBJECT, device_metadata: METADATA_LIST, project_data: DATA_SCHEMA) -> str:
    return template.render(
        image_path=convert_to_chart(project_data),
        devices=device_metadata
        **project_metadata,
    )
    
def generate_pdf(project_metadata: METADATA_OBJECT, device_metadata: METADATA_LIST, project_data: DATA_SCHEMA) -> str:
    file_name: str = generate_file_name(project_metadata["ProjectName"], "pdf")
    with open(f"{TEMP_PDF}/{file_name}") as f:
        pisa_status = pisa.CreatePDF(
            generate_html(project_metadata, device_metadata, project_data),
            dest=f
        )
    if pisa_status.err:
        raise IOError(f"Failed to write the project PDF for {project_metadata["ProjectName"]}")

    return file_name
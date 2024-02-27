import json
import os


absolute_path = os.path.dirname(__file__)
config_path = "../config/conf.json"
full_config_path = os.path.join(absolute_path, config_path)


def get_config(key: str) -> dict:
    with open(full_config_path, "r") as f:
        file_content: str = f.read()
        file_content_json: dict = json.loads(file_content)
    
    return file_content_json[key]
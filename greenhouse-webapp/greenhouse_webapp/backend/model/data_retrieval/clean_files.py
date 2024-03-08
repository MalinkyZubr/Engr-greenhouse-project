import os
import asyncio
import time
from typing import Awaitable

from model.data_retrieval.data_conversions import TEMP_CSV, TEMP_IMAGES
from model.task_manager.task_manager import Task


MAX_FILE_LIFETIME = 1800
        
        
class FileCleaner(Task):
    def get_temp_files(self, temp_paths: list[str]) -> None:
        all_files: list[str] = []
        
        for temp_path in temp_paths:
            all_files += [f"{temp_path}/{file}" for file in os.listdir(temp_path)]
            
        return all_files
    
    async def task(self) -> Awaitable:
        """the image and csv files are temporary, they should be erased at regular intervals"""
        files_to_check = self.get_temp_files([TEMP_CSV, TEMP_IMAGES])

        for file in files_to_check:
            if time.time() - os.path.getmtime(file) > MAX_FILE_LIFETIME:
                os.remove(file)
                
        await asyncio.sleep(900)
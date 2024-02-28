import matplotlib.pyplot as plt
import matplotlib.dates as dates
from matplotlib import figure as Figure
import os
import asyncio
import time
import csv
from typing import Literal

from greenhouse_webapp.utilities.timing import matplot_times
from greenhouse_webapp.model.database.data_interface import DatabaseInterface


LOCAL_DIR = os.path.dirname(os.path.abspath(__file__))
TEMP_IMAGES = os.path.join(LOCAL_DIR, "/temp-images")


class DataRetriever:
    """Class used for retrieving and formatting data for different applications, such as csv and visualization"""
    
    def filter_data_project_vis(self, provided_data: list[tuple], data_type: Literal["Temperature", "Humidity", "Moisture", "LightExposure"], data_interface: DatabaseInterface) -> dict[str, list[tuple, tuple]]:
        """filter data for use in visual matplotlib plots. Necessitates that data be in a format such that x axis (time) and y axis (measurement) are in separate arrays

        Args:
            provided_data (list[tuple]): aggregate of data provided, with all different measurement types
            data_type (Literal[&quot;Temperature&quot;, &quot;Humidity&quot;, &quot;Moisture&quot;, &quot;LightExposure&quot;]): which datatype to use on the plot
            data_interface (DatabaseInterface): the database connector to use for retrieving device information

        Returns:
            ldict[str, list[tuple, tuple]]: list of 2 tuples, the first is the x axis, the second the y axis. Each device has their own entry of this form in the dict
        """
        devices = dict()
        for datapoint in provided_data:
            data_value: float = datapoint[self.data_mapping[data_type]]
            device_id: int = datapoint[self.data_mapping["DeviceID"]]
            timestamp: float = matplot_times(datapoint[self.data_mapping["DateCollected"]])
            
            try:
                devices[device_id].append((timestamp,data_value))
            except:
                devices[device_id] = []
                devices[device_id].append((timestamp,data_value))
        
        named_device_data: dict[str, list[tuple, tuple]] = dict()
        
        for device_id, value in devices.items():
            try:
                device_name: str = data_interface.execute("getDeviceName", device_id)[0][0]
            except IndexError:
                device_name: str = device_id
            named_device_data[device_name] = zip(*value) # prep the data to be displayed. The zip function flattens the data so that there is one tuple of all x values, and one tuple of all y

        return named_device_data
    
    def generate_csv(self, provided_data: list[tuple], project_name: str) -> str:
        """generate a CSV file from the data provided to be returned to the client on the frontend

        Args:
            provided_data (list[tuple]): aggregate of all data provided, with all measurement types
            project_name (str): name of the project from which the data was extracted

        Returns:
            str: the name of the filepath for the newly generated CSV
        """
        csv_file_name: str = f"{LOCAL_DIR}/temp-csv/{project_name}.csv"
        
        with open(csv_file_name, mode='w') as csv_file:
            csv_headers: list[str] = list(self.data_mapping.keys())
            
            writer: csv.DictWriter = csv.DictWriter(csv_file, fieldnames=csv_headers)
            writer.writeheader()
            
            for datapoint in provided_data:
                writer.writerow({
                    "DeviceID":datapoint[0],
                    "ProjectID":datapoint[1],
                    "DateCollected":datapoint[2],
                    "Temperature":datapoint[3],
                    "Humidity":datapoint[4],
                    "Moisture":datapoint[5],
                    "LightExposure":datapoint[6],
            })
        return csv_file_name
        
    def generate_data_image(self, provided_data: list[tuple], data_type: Literal["Temperature", "Humidity", "Moisture", "LightExposure"], project_name: str, data_interface: DatabaseInterface) -> str:
        """outer function to generate images from project data using matplotlib. Generates PNG images

        Args:
            provided_data (list[tuple]): aggregate of all project data with all measurement types
            data_type (str): data_type (Literal[&quot;Temperature&quot;, &quot;Humidity&quot;, &quot;Moisture&quot;, &quot;LightExposure&quot;]): which datatype to use on the plot
            project_name (str): name of project from which the data is taken
            data_interface (DatabaseInterface): database interface to get data from

        Returns:
            str: returns the path to a temporary png file containing the graph of the matplotlib graph. If there is no data, the function returns the "no-data" image
        """
        if provided_data:
            filtered_data: dict[str, list[tuple, tuple]] = self.filter_data_project_vis(provided_data, data_type, data_interface)
        
            file_name: str = f"{LOCAL_DIR}/temp-images/{project_name}_{data_type}.png"
        
            figure: Figure = plt.figure()
            plt.gcf().set_size_inches(10, 6)
            plot = figure.add_subplot()
            
            plt.xlabel("Datetime")
            plt.ylabel(data_type)
            plt.title(f"{data_type} for Project '{project_name}' Over Time")
            plot.xaxis.set_major_formatter(dates.DateFormatter("%m-%d-%Y %H:%M:%S"))
            plt.xticks(rotation=45, ha='right')
            
            for device_name, device_data in filtered_data.items():
                plot.plot(*device_data, "-o", label=device_name)
            plot.legend()
            
            with open(file_name, 'w') as f:
                f.write(" ")
            plt.tight_layout()
            plt.savefig(file_name)
            plt.close()
        else:
            file_name: str = f"{LOCAL_DIR}/perm-images/no_data.png"
        
        return file_name
    
    async def clean_files(self):
        """the image and csv files are temporary, they should be erased at regular intervals"""
        while True:
            images: list = [f"{LOCAL_DIR}/temp-images/{image}" for image in os.listdir(f"{LOCAL_DIR}/temp-images")]
            csv: list = [f"{LOCAL_DIR}/temp-csv/{csv_file}" for csv_file in os.listdir(f"{LOCAL_DIR}/temp-csv")]
            for image in images:
                if time.time() - os.path.getmtime(image) > 1800:
                    os.remove(image)
            for csv_file in csv:
                if time.time() - os.path.getmtime(csv_file) > 1800:
                    os.remove(csv_file)
            await asyncio.sleep(900)
    
    
        

            
        
    
from datetime import datetime, timedelta
import matplotlib.dates as dates

MAX_TIME = datetime.fromtimestamp(32536799990).strftime("%y-%m-%d")
MIN_TIME = datetime.fromtimestamp(0).strftime("%y-%m-%d")


def get_today_tomorrow() -> tuple[str, str]:
    today = datetime.today().strftime("%y-%m-%d")
    tomorrow = (datetime.today()+timedelta(1)).strftime("%y-%m-%d")
    return today, tomorrow

def convert_time_formats(m_d_y):
    return datetime.strftime(datetime.strptime(m_d_y, "%m-%d-%y"), "%y-%m-%d")

def matplot_times(desired_datetime: datetime):
    return datetime.strptime(datetime.strftime(desired_datetime, "%m-%d-%Y %H:%M:%S"), "%m-%d-%Y %H:%M:%S")

def get_difference(start_datetime: datetime, end_datetime: datetime):
    difference_delta = end_datetime - start_datetime
    seconds = difference_delta.seconds
    minutes = seconds / 60
    seconds = seconds % 60
    hours = minutes / 60
    minutes = hours % 60
    
    return f"{hours}:{minutes}:{seconds}"
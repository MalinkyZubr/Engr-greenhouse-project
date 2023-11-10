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
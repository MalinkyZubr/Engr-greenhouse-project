from datetime import datetime, timedelta

MAX_TIME = datetime.fromtimestamp(32536799990).strftime("%y-%m-%d")
MIN_TIME = datetime.fromtimestamp(0).strftime("%y-%m-%d")


def get_today_tomorrow() -> tuple[str, str]:
    today = datetime.today().strftime("%y-%m-%d")
    tomorrow = (datetime.today()+timedelta(1)).strftime("%y-%m-%d")
    return today, tomorrow
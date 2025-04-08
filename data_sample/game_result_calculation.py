#!/usr/bin/python3

""" generate graphics by game data """

import os
import sqlite3
import datetime
import time
from PIL import Image, ImageDraw, ImageFont
import dateparser
import yaml

EPOCH_TIME = datetime.datetime(1970, 1, 1)
OUTPUT_DIR = "game_result"

if not os.path.isdir(OUTPUT_DIR):
    os.mkdir(OUTPUT_DIR)

CONFIG = None
with open("config.yml", "rt", encoding="utf-8") as _config:
    try:
        CONFIG = yaml.safe_load(_config)
    except yaml.YAMLError as exc:
        print(exc)

GAME_START = dateparser.parse(CONFIG['game']['start'], settings={'TIMEZONE': '+0000'})
GAME_START = (GAME_START - EPOCH_TIME).total_seconds()
GAME_START_MS = GAME_START * 1000

GAME_END = dateparser.parse(CONFIG['game']['end'], settings={'TIMEZONE': '+0000'})
GAME_END = (GAME_END - EPOCH_TIME).total_seconds()
GAME_END_MS = GAME_END * 1000
GAME_BREAK_START = int(
    dateparser.parse(
        CONFIG['game']['coffee_break_start'],
        settings={'TIMEZONE': '+0000'}
    ).strftime('%s')
)
GAME_BREAK_END = int(
    dateparser.parse(
        CONFIG['game']['coffee_break_end'],
        settings={'TIMEZONE': '+0000'}
    ).strftime('%s')
)
TEAMS = {}

GAME_LENGTH = GAME_END_MS - GAME_START_MS

RENDER_DATA = {
    "rows": [],
    "names": {},
    "data": [],
    "marks": [],
}

SERVICE_IDS = []
for checker in CONFIG["checkers"]:
    RENDER_DATA["rows"].append(checker["id"])
    RENDER_DATA["names"][checker["id"]] = checker["service_name"]
    SERVICE_IDS.append(checker["id"])

print(GAME_LENGTH)
#  print(1743135813970)

for team in CONFIG["teams"]:
    TEAMS[team["id"]] = team["name"]

# print(GAME_END - GAME_START)

# with open("game_result/put_flags_statistics.csv", "wt", encoding="utf-8") as _put_flags:
#     _all_flags = {"all": 0}
#     connection = sqlite3.connect('db/flags_checker_put_results.db')
#     cursor = connection.cursor()
#     _line = "service,all"
#     for team in CONFIG["teams"]:
#         teamid = team["id"]
#         _line += "," + teamid
#         _all_flags[teamid] = 0
#     _put_flags.write(_line + "\n")
#     for checker in CONFIG["checkers"]:
#         _line = ""
#         serviceid = checker["id"]
#         _line += serviceid + ","
#         # print(checker["service_name"])
#         cursor.execute(
#              'SELECT COUNT(*) FROM flags_checker_put_results WHERE serviceid = ?', (serviceid,)
#         )
#         results = cursor.fetchall()
#         for row in results:
#             _all_flags["all"] += row[0]
#             _line += str(row[0])
#         for team in CONFIG["teams"]:
#             teamid = team["id"]
#             cursor.execute(
#             'SELECT COUNT(*) FROM flags_checker_put_results WHERE serviceid = ? AND teamid = ?',
#             (serviceid,teamid,)
#             )
#             results = cursor.fetchall()
#             for row in results:
#                 _line += "," + str(row[0])
#                 _all_flags[teamid] += row[0]
#         _put_flags.write(_line + "\n")
#     _line = "," + str(_all_flags["all"])
#     for team in CONFIG["teams"]:
#         teamid = team["id"]
#         _line += "," + str(_all_flags[teamid])
#     _put_flags.write(_line + "\n")
#     _put_flags.write(",,\n")
#     _put_flags.write(",,\n")
#     service_states = ["up", "down", "corrupt", "mumble", "shit"]
#     _line = "service"
#     for _st in service_states:
#         _line += "," + _st
#     _put_flags.write(_line + "\n")
#     for checker in CONFIG["checkers"]:
#         _line = ""
#         serviceid = checker["id"]
#         _line += serviceid
#         for _st in service_states:
#             # print(checker["service_name"])
#             cursor.execute(
#             'SELECT COUNT(*) FROM flags_checker_put_results WHERE serviceid = ? AND result = ?',
#             (serviceid,_st)
#             )
#             results = cursor.fetchall()
#             for row in results:
#                 _line += "," + str(row[0])
#         _put_flags.write(_line + "\n")
#
#     connection.close()


class GraphGameResult:
    """ GraphGameResult """

    def __init__(self, options):
        self.__options = options
        self.__padd_left = options["padding"]["left"]
        self.__padd_right = options["padding"]["right"]
        self.__padd_top = options["padding"]["top"]
        self.__padd_bottom = options["padding"]["bottom"]
        self.__im = Image.new(
            'RGBA',
            (self.__options['width'], self.__options['height']),
            (32, 32, 32, 255)
        )
        self.__draw = ImageDraw.Draw(self.__im)

    def draw_axis(self):
        """ draw axis """
        # horizontal bottom
        self.__draw.line(
            (
                self.__padd_left,
                self.__options['height'] - self.__padd_bottom,
                self.__options['width'] - self.__padd_right,
                self.__options['height'] - self.__padd_bottom
            ),
            fill=(255, 255, 255, 255)
        )

        # horizontal top
        self.__draw.line(
            (
                self.__padd_left,
                self.__padd_top,
                self.__options['width'] - self.__padd_right,
                self.__padd_top,
            ),
            fill=(255, 255, 255, 255)
        )

        # vertical bottom
        self.__draw.line(
            (
                self.__padd_left,
                self.__padd_top,
                self.__padd_left,
                self.__options['height'] - self.__padd_bottom
            ),
            fill=(255, 255, 255, 255)
        )
        utc_time = time.gmtime(int(self.__options["ts_x_start_caption"]/1000))
        ts_printable = time.strftime("%Y-%m-%d %H:%M:%S+00:00 (UTC)", utc_time)

        self.__draw.text(
            (self.__padd_left, self.__options['height'] - self.__padd_bottom),
            ts_printable,  # CONFIG['game']['start'],
            font=self.__options['font'],
            fill=(255, 255, 255, 255)
        )
        self.__draw.text(
            (self.__options['width'] / 2 - 100, self.__padd_top - 40),
            self.__options['game_name'],
            font=self.__options['font'],
            fill=(255, 255, 255, 255)
        )
        self.__draw.text(
            (self.__options['width'] / 2 - 100, self.__padd_top - 20),
            "(Stollen Flags)",
            font=self.__options['font'],
            fill=(255, 255, 255, 255)
        )

    def get_columns_count_px(self):
        """ get_columns_count_px """
        return self.__options['width'] - self.__padd_left - self.__padd_right

    def get_row_count_px(self):
        """ get_row_count_px """
        return self.__options['height'] - self.__padd_top - self.__padd_bottom

    def draw_data(self, render_data):
        """ draw_data """
        max_y_height = 0
        # print("_data", render_data["data"][-1])
        _last_column = render_data["data"][-1]
        for _serviceid in render_data["rows"]:
            max_y_height += _last_column[_serviceid]

        self.__draw.text(
            (self.__padd_left, self.__padd_top - 20),
            str(max_y_height),
            font=self.__options['font'],
            fill=(255, 255, 255, 255)
        )
        height_per_flag = self.get_row_count_px() / max_y_height

        min_y = 1
        for _serviceid in render_data["rows"]:
            height_y = int(height_per_flag * _last_column[_serviceid])
            self.__draw.text(
                (
                    self.__options['width'] - self.__padd_right + 10,
                    self.__options['height'] - self.__padd_bottom - min_y - height_y/2 - 8,
                ),
                render_data["names"][_serviceid],
                font=self.__options['font'],
                fill=(255, 255, 255, 255)
            )
            min_y += height_y

        render_pos = self.__padd_left
        for _data in render_data["data"]:
            i = 0
            render_pos += 1
            min_y = 1
            # print(_data)
            for _serviceid in render_data["rows"]:
                height_y = int(height_per_flag * _data[_serviceid])
                # print(height_y)
                self.__draw.line(
                    (
                        render_pos,
                        self.__options['height'] - self.__padd_bottom - min_y,
                        render_pos,
                        self.__options['height'] - self.__padd_bottom - min_y - height_y
                    ),
                    fill=self.__options['colors'][i]
                )
                min_y += height_y
                i += 1

        for _data in render_data["marks"]:
            # vertical
            self.__draw.line(
                (
                    self.__padd_left + _data['x'],
                    self.__options['height'] - self.__padd_bottom,
                    self.__padd_left + _data['x'],
                    self.__options['height'] - self.__padd_bottom - _data['y']
                ),
                fill=(255, 255, 255, 255)
            )
            # horizontal
            self.__draw.line(
                (
                    self.__padd_left + _data['x'],
                    self.__options['height'] - self.__padd_bottom - _data['y'],
                    self.__padd_left + _data['x'] + 20,
                    self.__options['height'] - self.__padd_bottom - _data['y']
                ),
                fill=(255, 255, 255, 255)
            )
            self.__draw.text(
                (
                    self.__padd_left + _data['x'],
                    self.__options['height'] - self.__padd_bottom - _data['y'] - 20,
                ),
                _data["caption"],
                font=self.__options['font'],
                fill=(255, 255, 255, 255)
            )

    def save(self, path):
        """ save """
        self.__im.save(path)


res = GraphGameResult({
    "width": 1920,
    "height": 1080,
    "ts_x_start_caption": GAME_START_MS,
    "game_name": CONFIG['game']['name'],
    # "font": ImageFont.truetype("Pillow/Tests/fonts/FreeMono.ttf", 18),
    "font": ImageFont.truetype("dejavu-fonts-ttf-2.37/ttf/DejaVuSans.ttf", 18),
    "padding": {
        "top": 60,
        "bottom": 24,
        "left": 24,
        "right": 200,
    },
    "colors": [
        (223, 187, 177, 255),
        (245, 100, 118, 255),
        (228, 63, 111, 255),
        (172, 55, 117, 255),
        (94, 67, 82, 255),
        (143, 203, 155, 255),
        (91, 146, 121, 255),
        (141, 148, 186, 255),
    ]
})

res.draw_axis()

flags_stolen = sqlite3.connect('db/flags_stolen.db')
cursor = flags_stolen.cursor()

CURRENT_TIME_MS = GAME_START_MS
SEC_PER_PIX = GAME_LENGTH / res.get_columns_count_px()

print("SEC_PER_PIX", SEC_PER_PIX)
print("GAME_START_MS", CURRENT_TIME_MS)

POS_Y = 100
for _serviceid in SERVICE_IDS:
    cursor.execute(
        'SELECT thief_teamid, date_action FROM flags_stolen WHERE serviceid = ? LIMIT 0,1',
        (_serviceid,)
    )
    results = cursor.fetchall()
    for row in results:
        thief_teamid = row[0]
        date_action = row[1]
        teamname = TEAMS[thief_teamid]
        _ts = date_action - GAME_START_MS
        # utc_time = time.gmtime(int(date_action/1000))
        # ts_printable = time.strftime("%Y-%m-%d %H:%M:%S+00:00 (UTC)", utc_time)
        _posx = int(_ts / SEC_PER_PIX)
        # print("_posx", _posx)
        # print("ts_printable", ts_printable)
        new_mark = {
            "caption": "FB: " + RENDER_DATA["names"][_serviceid] + " (" + teamname + ")",
            "x": _posx,
            "y": POS_Y,
        }
        POS_Y += 100
        RENDER_DATA["marks"].append(new_mark)
        # date_action

while CURRENT_TIME_MS < GAME_END_MS:
    CURRENT_TIME_MS += SEC_PER_PIX
    ts = CURRENT_TIME_MS/1000
    # utc_time = time.gmtime(int(ts))
    # ts_printable = time.strftime("%Y-%m-%d %H:%M:%S+00:00 (UTC)", utc_time)
    # print(ts_printable)
    # print("CURRENT_TIME_MS", CURRENT_TIME_MS)
    _column_data = {}
    for _serviceid in SERVICE_IDS:
        cursor.execute(
            'SELECT COUNT(*) FROM flags_stolen WHERE serviceid = ? AND date_action <= ?',
            (_serviceid, int(CURRENT_TIME_MS),)
        )
        results = cursor.fetchall()
        for row in results:
            _column_data[_serviceid] = int(row[0])
    RENDER_DATA["data"].append(_column_data)
    # print(_column_data)
    # if render_pos > 200:
    #     sys.exit(0)

res.draw_data(RENDER_DATA)
res.save("game_result/flag_stole_counter.png")

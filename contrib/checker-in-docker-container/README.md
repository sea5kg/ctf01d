# checker-as-service

It's concept in current time. Development in progress.


## POST data format:

- `timeout`: timeout
- `mask`: *mask* of host, for example: `"10.22.{}.3"`
- `command`: command (`put` or `check`)
- `checker`: `[host, port]`
- `flags`: array of flags

Flag format:

Array of:
- host substring
- flag_id
- flag (value)

Example json:
```
{
  "timeout": 5,
  "mask": "service{}",
  "command": "check",
  "checker": ["localhost", 4109],
  "flags": [
      ["1", "run_test_NukYI5LV6h", "c01d5cab-ff8b-9771-e492-133050588587"],
      ["2", "run_test_gGdVLcCjE7", "c01dad6c-4fc1-5787-4202-417f81383286"],
      ["3", "run_test_c6hET9lPrR", "c01d2165-d46e-2535-e21d-2d1824383276"],
      ["4", "run_test_WBJMDRa9yK", "c01d88cb-1821-43d6-6894-be3494757323"],
      ["5", "run_test_Rtkd6qWjRE", "c01dd670-43d7-458f-7aa7-731910962441"],
      ["6", "run_test_PujksbRcy1", "c01dbcea-2ca1-6a7b-4680-be7b60448216"],
      ["7", "run_test_NiuczwyrlQ", "c01d93be-b44a-bbab-5005-42cb27909855"],
      ["8", "run_test_5xGaMQ5TbX", "c01d660d-b45e-18ef-dd60-f1c873170438"],
      ["9", "run_test_wn9JKMp7Eb", "c01dc8cc-55cb-7a52-105d-9be852903760"],
      ["10", "run_test_YFxnmBJmgk", "c01de2d2-27dc-13d7-4043-244593700652"],
      ["11", "run_test_erslvY4yHr", "c01d2cad-2b0a-98be-14c3-35fb78483134"],
      ["12", "run_test_jb9CC5zua1", "c01db8e9-4456-96d4-9985-024e13954292"],
      ["13", "run_test_bfDmtjEQ7i", "c01dfdee-829c-a3dd-83d5-4bbe45901963"],
      ["14", "run_test_ejJQg8MTZa", "c01d90f1-2d3c-ca2c-5771-511362925427"],
      ["15", "run_test_4Rhr64IRQV", "c01dc0a7-bc71-5841-3904-0c8d20063884"],
      ["16", "run_test_2gaRgY79ms", "c01d7d2e-73c3-07ea-3567-10d544440980"],
      ["17", "run_test_2YiD8vUs49", "c01da469-f8c9-c10a-c506-7a6e24401093"],
      ["18", "run_test_QyxcZW4XYt", "c01db2bd-2123-86a6-fef7-775b50072744"],
      ["19", "run_test_cWe4OHHRCs", "c01d0e4a-0b0c-838d-6f62-e04978444387"],
      ["20", "run_test_JMw40RIWgW", "c01dbaf1-ed2e-a2d8-11a9-7b7c41033202"],
      ["21", "run_test_Xpx7sHqpdi", "c01dc2e2-cac9-5d1f-a30f-90c851543738"],
      ["22", "run_test_Wpp7B2TiNV", "c01dcc95-1985-4ba5-cb84-3df899074580"],
      ["23", "run_test_ngfEvgstwm", "c01dbf55-30ee-b617-fca3-db3081178820"],
      ["25", "run_test_jUtoZRRule", "c01dc916-4d1f-1da5-067c-122c96031782"]
  ]
}
```

for example commands run checkers:

- `./checker.py service1 put run_test_NukYI5LV6h c01d5cab-ff8b-9771-e492-133050588587`
- `./checker.py service2 put run_test_gGdVLcCjE7 c01dad6c-4fc1-5787-4202-417f81383286`
- ...
- `./checker.py service14 put run_test_ejJQg8MTZa c01d90f1-2d3c-ca2c-5771-511362925427`
- ...
- !!! skipped `service24`
- `./checker.py service25 put run_test_jUtoZRRule c01dc916-4d1f-1da5-067c-122c96031782`


## Expected output format:

```
{
  "1": 101,
  "2": 101,
  "3": 101,
  "4": 104,
  "5": 101,
  "6": 102,
  "7": 101,
  "8": 103,
  "9": 101,
  "10": 101,
  "11": 101,
  "12": 101,
  "13": 101,
  "14": 101,
  "15": 101,
  "16": 101,
  "17": 101,
  "18": 101,
  "19": 101,
  "20": 101,
  "21": 101,
  "22": 101,
  "23": 101,
  "25": 101
}
```

About states:

- 101 - up - the flag putting/checking into the service is successful
- 102 - corrupt - service is available (available tcp connection) but it's impossible to put/get the flag
- 103 - mumble - checker-script worked long time than allowed (this state will be set by ctf01d)
- 104 - down - service is not available (maybe blocked port or service is down)
- (any) - shit - problems in checker (this state will be set by ctf01d), for checker developers
- 500? - problems with checker-as-service (notify about inside problems)

So:

- `service4` - is down state
- `service6` - is corrupted state
- `service8` - is mumble state
- another - is up
- `service24` - skipped

# Changelog

## [v0.7.2] - (2026 June ??)

- Fixed increment summary flags stollen

## [v0.7.1] - (2026 June 08)

- Added Prometheus exposition at GET /api/v1/metrics ([#113](https://github.com/sea5kg/ctf01d/pull/113)). Disabled by default; enable via `scoreboard.prometheus-metrics-endpoint.enabled` and restrict scrapers with `scoreboard.prometheus-metrics-endpoint.allowed-for` (comma-separated wildcard patterns, e.g. `10.10.100.*, 127.0.*`). Not listed clients get HTTP 403.
- Added Grafana dashboard and Prometheus example for the metrics endpoint ([#115](https://github.com/sea5kg/ctf01d/pull/115))
- Implemented suffix/prefix for ip-or-host. Also allowed host name.
- Add thread watcher for hot reload
- Implemented hot reload for option: prometheus-metrics-endpoint.enabled
- Updated HowardHinnant/date to v3.0.4
- Splitted logging for web-server to different file (but error will be duplicates to main log file)
- Fixed './pm.py make-test-game' for compatible with v0.7.0, Change default jury public port to 80 by default
- Fixed disabling ActionAutomation if disabled icon animation
- Redesign logging system for checker (every checker now will be write info messages to self log file)
- Fixed infinity first-blood
- Replaced to free font 'AmazingGroteskDemi'
- Renamed /api/v1/myip -> /api/v1/my-ip

## [v0.7.0] - (2026 June 4)

- Fixed #84: Added /api/v1/game/current-time
- Fixed #67: m_nFlagLifeTimeInMin changed to seconds
- Moved and renamed some source files to src/ctf01d
- Added background for status up and down
- Fixed #85: Add optionality disable animation.
- Renamed class 'firstblood' to first-blood. Removed previous implementation first blood
- Fixed names in html
- Add team slider overlay to scoreboard
- Fixed readonly for tooltip
- Removed examples and added documentation
- Fixed #84: Added /api/v1/game/current-time
- Fixed #105. Calculate sha1 by file after extract files (and use it for update in future)
- Redesign flag_lives as Employ (now alive_flags)
- Fixed #110: Add CTF01D_USER and implement extract files by owner not by root
- Redesign header in scoreboard and implemented service logo
- Added new endpoints /logo/big/team, /logo/team, /logo/big/service, /logo/service
- Fixed #107: Add new field to config: Logo of service (also big and big-team)
- Fixed #103. Added support system environment CTF01D_PORT
- Redesigned section 'checkers' -> 'services'
- Redesign teams section config. Added teams.config and teams.list
- Added check team.type
- Redesign reading team_config and renamed 'ip_address' to 'ip-or-host'
- Start redesign team_config to reading like a vars, added new fields 'big-logo' and 'type'
- Added new settings for scoreboard subsection 'prometheus-metrics-endpoint'
- Renamed 'htmlfolder' to 'html-dir-path' (fix wording)
- Improved test game config generation
- Added expose port to docker-compose sample (if user changed base port)
- Partial merge fixes from branch 'src/ctf01d/objects/ctf01d_service_statistics.cpp'
- Improved './pm.py make-test-game' added possible set jury host
- Minor updates libpm/data/attacker_example_service.py
- Cleanup about clean
- Redesign parsing service config. Added logo / big-logo to service. Added a lot of icons for service icons
- Redesign reading service_def
- Renamed 'time_sleep_between_run_scripts_in_sec' -> 'round_in_seconds'


## [v0.6.1] - (2026 May 30)

- Added style-hackerdom scoreboard frontend
- Moved parse command line to main.cpp and removed unused wsjcpp-arguments
- remove flag live time and fix script permissions. close #87
- Set permissions on extracted files #89
- Added new helper Ctf01dFilesWatcher and unit-tests for it
- Refactored code
- Moved part of functionality from ctf01d_http_server to employ_web_server
- Moved from 'employees' to 'ctf01d/employees'
- Fix indents in index.html
- Added 'DOCTYPE html' to index.html
- Redesign libhv logs to custom
- Init empty EmployWebServer and moved start web server to employ_web_server
- Updated wsjcpp-employees from v0.1.2 to v0.2.1
- Updated copyrights
- Moved few objects from different places to src/ctf01d/objects/
- Removed wsjcpp_validators
- eliminate flag-submission race by moving dedup check under scoreboard mutex
- Moved src/wsjcpp-resources -> src/wsjcpp_resources
- Added new sub command for make test sample game './pm.py make-test-game -t 3 -s 5
- Redesign game vars to var_*
- Renamed 'start' to 'start_utc', 'end' to 'end_utc'
- Fixed magic numbers in check m_nFlagLifetimeInMin
- Renamed 'm_nFlagTimeLiveInMin' to 'm_nFlagLifetimeInMin'
- Renamed parameter 'flag_timelive_in_min' -> 'flag_lifetime_in_min'
- Moved 'send_random_flags.py' to './pm.py tests -r send_random_flags'
- Removed outdated tests/test_ram
- Simplify run jury for tests. Added new test for find memory leaks
- Fix absolute path for work-dir via argument command line
- Redesign test_path_traversal/test.sh to ./pm.py tests -r path_traversal
- Redesign ctf01d_http_server -> employ_web_server
- Fixed unpack resources


## [v0.6.0] - (2026 May 20)

- Scoring has been moved to a separate class and a test has been added.
- Fixed #93 Add summary activity
- Fixed #97. (Fixed critical vulnerability: path traversal).
- Fixed restart after coffee-break (thanks for gpt-5.5)
- Fixed grammar and wrong in messages
- Fixed spellings in source code.
- Added contrib/checker-in-docker-container
- Added https://ctf-gameserver.org/submission/#example to documentation/SIMILAR_SOFTWARE.md
- Added and applied font DejaVuSans
- Added comments for AI-Assistants
- Added script after game: game_result_calculation.py
- Added test for path traversal. Updated 'WsjcppCore::doNormalizePath' -> 'wsjcpp::normalizeFilePath'. Forgot wsjcpp.yml update
- Added libpm and pm.py
- Added .vscode/settings.json
- Updated contrib/docker-build-stages to debian:13
- Updated wsjcpp_core from 0.2.4 to 0.2.5
- Updated wsjcpp-yaml v0.1.7 -> v0.1.10
- Updated nlohmann_json v3.9.1 -> v3.12.0 (Reinstalled)
- Updated sqlite from to 3.49.1 to 3.53.1 from sqlite-amalgamation-3530100.zip (https://www.sqlite.org/download.html)
- Updated minor in documentation
- Updated copyrights in some files
- Removed distribution section from wsjcpp.yml
- Redesign pointers to std::shared_ptr.
- Reorganized source files.
- Removed unused 'src/store' and subcommands 'teams' and 'services'
- Removed contrib/docker-build-stages


## [v0.5.5] - (2025 Apr 1)

- Added preinstalled 'ruby-sqlite3'
- Removed from preinstalled 'nokogiri'
- Optimize size scoreboard
- Refactoring README to documentation directory
- game-simulation moved to test_game_simulation
- Fix #91 FirstBlood redesign. Fix bug with design. And added hint with information
- Fix #86 chmod 776 for checker service directory (checker could not write to database)
- Fix #88 Check on start sames IP and block start


## [v0.5.4] - (2025 Mar 27)

- Added `contrib/auto_static_ip_for_vulnbox` - Script automatically set static IP for VULNBOX machine on start
- Added preinstalled 'bs4', mimesis
- Added preinstalled 'gem install sqlite && gem install nokogiri'
- Remember request IP to flag_attempts


## [v0.5.3] - (2025 Mar 23)

- Add utility ping to docker image
- Updated libhv from 1.3.1 to 1.3.3 from (https://github.com/ithewei/libhv)
- Updated sqlite from 3.43.2 to 3.49.1 from sqlite-amalgamation-3490100.zip (https://www.sqlite.org/download.html)
- Updated wsjcpp-yaml from v0.1.1 to v0.1.7 (https://github.com/wsjcpp/wsjcpp-yaml)
- Updated config - extend to 6 services and 30 teams
- Fix #76 change icons for attack and defence to understooble
- Fix #75 show time in human readable in scoreboard
- Updated year copyright
- Updated ctf01d-stage-release (added python packages grpcio grpcio-tools protobuf tzdata, added ruby-full)
- Added new api handler `http://{HOST}:{PORT}/api/v1/myip` for detect myip
- Prepare tests/test_sample_game_network_6x30 for check network configuration + jury base on test services (6 services x 30 teams)


## [v0.5.2] - (2023 Nov 18)

- Fix #69 implemented SLA
- Implemented firstblood action
- Up build stages to debian:12
- Added python3 library faker to docker image ctf01d
- Improved logging for mumble exit code checker
- Fix #70 redesign calculation attack points like RuCTF 2023
- PEP8 and pylint for example_service1
- Fixed automatization action - only one element can be animated in onetime
- Fix #72 team logo update when file was changed


## [v0.5.1] - (2023 Nov 16)

- Added show time left in scoreboard in seconds #65
- Redesign database from mysql/mariadb to sqlite3 (build-in)
- Removed requirements by mysqlclient


## [v0.5.0] - (2023 Nov 12)

- Added src libhv v1.3.1 (https://github.com/ithewei/libhv)
- Removed wsjcpp-light-web-server
- Redesign code http server to libhv
- Updated wsjcpp-core to v0.2.3
- Up c++ to 17
- Added src sqlite3 v3.43.2 (https://www.sqlite.org/)
- Added Copyrights to source files
- Added 20 icon for teams
- Fixed random scoreboard
- Minor fixed legend activity
- Fix #64 changed mumble icon to Zzz
- Fix crash on start (sometimes was crash on start)
- Changed format of flag and updated resources


## [v0.4.5] - (2023 Mar 25)

- Removed python2 from ctf01d docker image
- Added `nano` and `vim` to ctf01d docker image
- Fixed init default configs on first-start container
- Implemented command `ctf01d teams search <keys>`
- Added file `ctf01d-store/all-stores.json`
- Added file `ctf01d-store/teams-examples.json`
- Fixed read all from pipe of checker


## [v0.4.4] - 2020-09-22 (2020 Sep 22)

- Copied script fhqjad-store from fhq-server
- Fixed #52 added 'ctf01d teams list'
- Added to main find workdir authomaticly
- Fixed #53 added 'ctf01d services list'
- Updated README.md
- Updated wsjcpp-core to v0.2.1
- Updated wsjcpp-arguments to v0.2.1


## [v0.4.3] - 2020-09-12 (2020 Sep 12)

- Migrated to wsjcpp system c++ packages management
- Installed wsjcpp arguments
- Updated wsjcpp-core and installed original nlohmann_json package
- Added unit test for service_costs_and_statistics
- Added 'for developers' to readme
- Implemented '-db-host hostname' parameter
- Prepare Docker files for ctf01d-stage-build / ctf01d-stage-release
- Implemented script 'game-simulation/ctf01d-assistent.py' with jury / service1_py / service2_go
- Implemented game-simulation/vulnbox/service3_php
- Create EmployFlags, EmployScoreboard. 
- Updated version to 0.4.3
- Renamed freehackquest/fhq-jury-ad to sea-kg/ctf01d
- Fixed #41 Renamed fhq-jury-ad to ctf01d
- Updated rules and screen in README.md
- Redesign parse arguments to wsjcpp-arguments model
- Renamed testing to game-simulation
- Added unit test for flag.
- Redesign fill json for statistics by services
- Redesign Config to EmployConfig
- Renamed class Service to Ctf01dServiceDef
- Renamed class Team to Ctf01dTeamDef
- Renamed class Flag to Ctf01dFlag
- Redesign extract sample files and config
- Redesign Dockerfile for ctf01d
- Redesign calculation costs
- Removed vulnbox
- Removed not need anymore file update_resources.py
- Fixed english translates in README.md file
- Fixed clean flags

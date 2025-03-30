
# Game Simulation

It's necessary for testing in conditions close to real game
- 3 teams
- 4 services (written in different languages)
- 5 subnetworks (with masquerade - base on docker network)

Requirements:
* `$ pip3 install docker`

Start:
```
$ cd ~/ctf01d.git/game-simulation/
$ ./ctf01d-assistent.py start
```

After this command has run successfully, you can look for:

  * Scoreboard - http://localhost:8080
  * team1 - service1_py : `nc 10.10.11.1 4101`
  * team2 - service1_py : `nc 10.10.12.1 4101`
  * team3 - service1_py : `nc 10.10.13.1 4101`
  * team1 - service2_go : http://10.10.11.1:4102
  * team2 - service2_go : http://10.10.12.1:4102
  * team3 - service2_go : http://10.10.13.1:4102

To remove all images, containers and networks:
```
$ cd ~/ctf01d.git/game-simulation/
$ ./ctf01d-assistent.py clean
```

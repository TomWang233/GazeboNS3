# Drone-Relay-Simulation

## Design

## System Setup and Configuration

It is recommended to use a Linux system to run the simulation, as the Docker container uses the host's display to render the gazebo client and hardware graphics acceleration inside a Docker container is difficult on MacOS and Windows. Ubuntu is assumed here, but equivalent instructions should work for other distributions.

1. **Install Docker**

Based on the instructions [here](https://docs.docker.com/engine/install/).

Add Docker GPG key.
```
sudo apt-get update
sudo apt-get install ca-certificates curl
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc
sudo chmod a+r /etc/apt/keyrings/docker.asc
```

Add the repository to apt.
```
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt-get update
```

Install Docker and Docker Compose.
```
sudo apt-get install docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
```

2. **Clone the repository**
`git clone https://github.com/byersrwn/Drone-Relay-Simulation.git`

3. **`cd` into gazebo/**

4. **Run `docker build -t new-gazebo .` This will build the docker image, and takes a while.**

5. **Check the graphics card DRI id by running `ls -l /dev/dri` and note the id of the card (this can be done at the same time as the previous step). It should be something like `card0`.
If it turns out to be card1 instead, replace `card0` with `card1` in the `docker-compose.yml` file.**

6. **Run `xhost +local:host`**
This will allow the gazebo client to open up in a new window from inside the docker container.

7. **Run `docker compose up` This will start the gazebo server and client. The client will open up in a new window in 3D mode.**

If the window shows up with a black screen
> Edit the `gazebo/data/start_gazebo.sh` and set `LEGACY_GRAPHICS` to 1

./DroneControl tcp://127.0.0.1:5760 tcp://127.0.0.1:5770 tcp://127.0.0.1:5780 tcp://127.0.0.1:5790


Docker Image Available Here:
[Image](https://mines0-my.sharepoint.com/:u:/g/personal/dcauwe_mines_edu/EeDEDPnJ705FjHVtfEfni2MBd9QIngid16izsCu_rN_rww?e=epFDtO)

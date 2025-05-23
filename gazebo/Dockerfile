# Start from ubuntu:24.04 and run these commands to setup the image
# dpkg --print-architecture
# apt-get update
# apt-get install -y lsb-release gnupg
# sh -c 'echo "deb http://packages.osrfoundation.org/gazebo/ubuntu-stable `lsb_release -cs` main" > /etc/apt/sources.list.d/gazebo-stable.list'
# wget http://packages.osrfoundation.org/gazebo.key -O - | apt-key add -
# apt-get update
# apt-get install -y gz-ionic

FROM ubuntu:24.04

 # Install Gazebo
RUN dpkg --print-architecture
RUN apt-get update
RUN apt-get install -y lsb-release gnupg wget curl
RUN curl https://packages.osrfoundation.org/gazebo.gpg --output /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg
RUN echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] http://packages.osrfoundation.org/gazebo/ubuntu-stable $(lsb_release -cs) main" | tee /etc/apt/sources.list.d/gazebo-stable.list > /dev/null
RUN apt-get update
RUN apt-get install -y gz-harmonic

# Install Ardupilot
ARG DEBIAN_FRONTEND=noninteractive
ARG USER_NAME=ardupilot
ARG USER_UID=1001
ARG USER_GID=1001
ARG SKIP_AP_EXT_ENV=0
ARG SKIP_AP_GRAPHIC_ENV=0
ARG SKIP_AP_COV_ENV=1
ARG SKIP_AP_GIT_CHECK=1
ARG DO_AP_STM_ENV=1

RUN groupadd ${USER_NAME} --gid ${USER_GID} && useradd -l -m ${USER_NAME} -u ${USER_UID} -g ${USER_GID} -s /bin/bash

RUN apt-get update && apt-get install --no-install-recommends -y \
    lsb-release \
    sudo \
    tzdata \
    git \
    default-jre \
    bash-completion

RUN git clone --recurse-submodules https://github.com/ArduPilot/ardupilot.git
WORKDIR /ardupilot

RUN echo "Current directory: $(pwd)"
RUN echo "Directory Listing: $(ls -la)"

# COPY Tools/environment_install/install-prereqs-ubuntu.sh /ardupilot/Tools/environment_install/
# COPY Tools/completion /ardupilot/Tools/completion/

# Create non root user for pip
RUN echo "ardupilot ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/${USER_NAME}
RUN chmod 0440 /etc/sudoers.d/${USER_NAME}

RUN chown -R ${USER_NAME}:${USER_NAME} /${USER_NAME}

USER ${USER_NAME}

RUN SKIP_AP_EXT_ENV=$SKIP_AP_EXT_ENV SKIP_AP_GRAPHIC_ENV=$SKIP_AP_GRAPHIC_ENV SKIP_AP_COV_ENV=$SKIP_AP_COV_ENV SKIP_AP_GIT_CHECK=$SKIP_AP_GIT_CHECK \
    DO_AP_STM_ENV=$DO_AP_STM_ENV \
    # AP_DOCKER_BUILD=1 \
    USER=${USER_NAME} \
    Tools/environment_install/install-prereqs-ubuntu.sh -y

WORKDIR /ardupilot
RUN PATH=/usr/lib/ccache:/ardupilot/Tools/autotest:/opt/gcc-arm-none-eabi-10-2020-q4-major/bin:/home/ardupilot/venv-ardupilot/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin /ardupilot/modules/waf/waf-light configure --board sitl
RUN PATH=/usr/lib/ccache:/ardupilot/Tools/autotest:/opt/gcc-arm-none-eabi-10-2020-q4-major/bin:/home/ardupilot/venv-ardupilot/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin /ardupilot/modules/waf/waf-light build --target bin/arducopter

# RUN sudo apt install python3-pip -y
# RUN pip3 install --break-system-packages empy future
# COPY ./data/entrypoint.sh /ardupilot/entrypoint.sh
# RUN /ardupilot/entrypoint.sh
# RUN /ardupilot/modules/waf/waf-light configure --board sitl
# RUN /ardupilot/modules/waf/waf-light build --target bin/arducopter
# RUN sim_vehicle.py -v ArduCopter

RUN sudo apt install -y libgz-sim8-dev rapidjson-dev
RUN sudo apt install -y libopencv-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-bad gstreamer1.0-libav gstreamer1.0-gl

RUN mkdir -p /ardupilot/gz_ws/src
WORKDIR /ardupilot/gz_ws/src

RUN echo "Current directory: $(pwd)"

RUN git clone https://github.com/ArduPilot/ardupilot_gazebo

RUN export GZ_VERSION=harmonic

RUN echo "Current directory: $(pwd)"

WORKDIR ardupilot_gazebo

RUN echo "Current directory: $(pwd)"

RUN mkdir build
WORKDIR build
RUN cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
RUN make -j4

# RUN export GZ_SIM_SYSTEM_PLUGIN_PATH="$HOME/gz_ws/src/ardupilot_gazebo/build:${GZ_SIM_SYSTEM_PLUGIN_PATH}"
# RUN export GZ_SIM_RESOURCE_PATH="$HOME/gz_ws/src/ardupilot_gazebo/models:$HOME/gz_ws/src/ardupilot_gazebo/worlds:${GZ_SIM_RESOURCE_PATH}"

# Lastly, build ardupilot


# Use GCC 9 as the base image
FROM gcc:9

# Update the package list and install necessary libraries
RUN apt-get update && \
    apt-get install -y libssl-dev \
                       libboost-system-dev \
                       libboost-thread-dev \
                       cmake \
                       git

RUN apt-get install -y nlohmann-json3-dev

# Clone SQLiteCpp from GitHub (if needed)
RUN git clone https://github.com/SRombauts/SQLiteCpp.git && \
    cd SQLiteCpp && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install

# Copy your project files into the Docker container
COPY . /root/projects/cardsServer

# Set the working directory inside the container
WORKDIR /root/projects/cardsServer

# Create a build directory, run cmake, and build the project
RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make

# Expose the port that your server listens on
EXPOSE 8080

# Command to run the server when the container starts
CMD ["./build/CardsServer"]

FROM ubuntu:22.04

# Install required packages and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libcurl4-openssl-dev \
    libssl-dev \
    libboost-all-dev \
    libgflags-dev \
    libgoogle-glog-dev \
    libfmt-dev \
    zlib1g-dev \
    libdouble-conversion-dev \
    wget \
    python3 \
    python3-pip \
    # Folly dependencies (but not Folly itself as we build it from source)
    libfmt-dev \
    libgflags-dev \
    libgoogle-glog-dev \
    libevent-dev \
    libdouble-conversion-dev \
    libboost-all-dev \
    liblz4-dev \
    liblzma-dev \
    libzstd-dev \
    libsnappy-dev \
    libssl-dev \
    libbz2-dev \
    libunwind-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install FastFloat (required by Folly)
WORKDIR /tmp
RUN git clone https://github.com/fastfloat/fast_float.git \
    && cd fast_float \
    && mkdir -p build \
    && cd build \
    && cmake .. \
    && make install \
    && cd /tmp \
    && rm -rf fast_float


# Set up workspace directory
WORKDIR /agents-cpp

# Copy only CMakeLists.txt first to optimize Docker cache
COPY CMakeLists.txt .

# Copy the entire project
COPY . .

# Create build directory
RUN mkdir -p build

# Build the project
WORKDIR /agents-cpp/build
RUN cmake .. && make -j$(nproc)

# Set workdir back to project root
WORKDIR /agents-cpp

# Set default command to run the shell
CMD ["/bin/bash"] 
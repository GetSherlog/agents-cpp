#!/bin/bash

# Make script exit on any error
set -e

# Function to display usage instructions
function show_usage {
  echo "Usage: ./docker-run.sh [COMMAND]"
  echo ""
  echo "Commands:"
  echo "  build       Build the Docker image"
  echo "  run         Run the Docker container in interactive mode"
  echo "  exec        Execute a command in the running container"
  echo "  build-proj  Build the C++ project inside the container"
  echo "  example     Run an example (e.g., ./docker-run.sh example simple_agent)"
  echo "  clean       Remove Docker container and volume"
  echo "  help        Show this help message"
  echo ""
  echo "Examples:"
  echo "  ./docker-run.sh build         # Build the Docker image"
  echo "  ./docker-run.sh run           # Run the container with a shell"
  echo "  ./docker-run.sh exec make     # Run make in the existing container"
  echo "  ./docker-run.sh build-proj    # Build the project inside the container"
  echo "  ./docker-run.sh example simple_agent # Run the simple_agent example"
}

# Check if docker-compose is installed
if ! command -v docker-compose &> /dev/null; then
  echo "Error: docker-compose is not installed. Please install Docker and docker-compose."
  exit 1
fi

# Process commands
case "$1" in
  build)
    echo "Building Docker image..."
    docker-compose build
    ;;
  run)
    echo "Running Docker container in interactive mode..."
    docker-compose run --rm agents-cpp
    ;;
  exec)
    shift
    if [ "$#" -eq 0 ]; then
      echo "Error: No command specified for exec."
      show_usage
      exit 1
    fi
    echo "Executing command in container: $@"
    docker-compose exec agents-cpp "$@"
    ;;
  build-proj)
    echo "Building the C++ project inside the container..."
    docker-compose run --rm agents-cpp bash -c "cd /agents-cpp/build && cmake .. && make -j$(nproc)"
    ;;
  example)
    if [ -z "$2" ]; then
      echo "Error: Please specify an example to run."
      echo "Available examples:"
      docker-compose run --rm agents-cpp ls -1 /agents-cpp/build/bin/examples/
      exit 1
    fi
    echo "Running example: $2"
    docker-compose run --rm agents-cpp /agents-cpp/build/bin/examples/$2
    ;;
  clean)
    echo "Cleaning up Docker resources..."
    docker-compose down -v
    ;;
  help|--help|-h)
    show_usage
    ;;
  *)
    if [ -z "$1" ]; then
      show_usage
    else
      echo "Error: Unknown command '$1'"
      show_usage
      exit 1
    fi
    ;;
esac 
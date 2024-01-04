# Cards Server

## Overview
Cards Server is a C++ based server application designed to manage and store cards for learning purposes. It provides a RESTful API for clients, allowing users to create, retrieve, update, and delete cards. Each consists of a term and its translation and related to folder(Now for MVP only one default folder is supported).

## Features
- CRUD operations for cards.
- Support for multiple clients with unique IDs. ( Now for MVP client sends it's own id but later service like OAuth will be added )
- SQLite database integration for persistent storage.
- Thread-safe operations ensuring stability under concurrent access.

## Prerequisites
- GCC Compiler (version 9 or later)
- Boost Libraries
- SQLite
- Docker (optional for containerized deployment)

## Installation and Setup
1. **Clone the repository:**

2. **Building the Server:**
- Without Docker:
  ```
  git clone https://github.com/microsoft/vcpkg.git
  cd vcpkg
  ./bootstrap-vcpkg.sh
  ./vcpkg integrate install
  ./vcpkg install sqlitecpp
  ./vcpkg install nlohmann-json
  mkdir build
  cd build
  cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
  make
  ```
- With Docker:
  ```
  docker-compose build
  ```
  
- With VS code:

use tasks.json


## Running the Server
- Without Docker:
./build/CardsServer

- With Docker:
  ```
  docker-compose up
  docker-compose down
  ```

- With VS code:

use tasks.json


## API Endpoints
- `POST /cards`: Create a new card.
- `GET /cards/{cardId}`: Retrieve a specific card by ID.
- `GET /cards`: Retrieve all cards.
- `PUT /cards/{cardId}`: Update an existing card.
- `DELETE /cards/{cardId}`: Delete a specific card.

## Usage Example
To create a new card:

curl -X POST "http://localhost:8080/cards" \
     -H "Content-Type: application/json" \
     -H "X-Client-ID: Example Id" \
     -d '{"folderId": 1, "term": "Example Term", "translation": "Example Translation"}'

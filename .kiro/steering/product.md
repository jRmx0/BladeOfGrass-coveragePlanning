# Product Overview

BladeOfGrass-coveragePlanning is a web-based coverage path planning application for robotics and autonomous systems.

## Core Functionality
- Interactive canvas interface for defining environments with boundaries and obstacles
- Coverage path planning algorithms (boustrophedon cellular decomposition)
- Real-time visualization of planned paths
- Export functionality for generated coverage paths

## Architecture
- C backend server using Mongoose web framework
- Vanilla JavaScript frontend with HTML5 Canvas
- JSON-based API communication between frontend and backend
- Standalone executable that serves both API and static files

## Use Cases
- Robotics path planning for coverage tasks (lawn mowing)
- Educational tool for understanding coverage algorithms
- Research platform for testing different decomposition strategies
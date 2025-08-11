.PHONY: build run clean build-and-run help

help:
	@echo Available commands:
	@echo make build     		- Build the project
	@echo make run         	- Run the built program
	@echo make build-and-run 	- Build and run in one step
	@echo make clean       	- Clean build files

build:
	@cd src/app && make

run:
	@cd src/app/build && main.exe & exit 0

build-and-run: build run

clean:
	@cd src/app && make clean
	@if exist temp\dec_out rmdir /S /Q temp\dec_out
	@if exist temp\env_in rmdir /S /Q temp\env_in
	@mkdir temp\dec_out
	@mkdir temp\env_in
.PHONY: compile run clean build-and-run help

help:
	@echo Available commands:
	@echo make compile     	- Compile the project
	@echo make run         	- Run the compiled program
	@echo make build-and-run 	- Compile and run in one step
	@echo make clean       	- Clean build files

compile:
	@cd src/app && make

run:
	@cd src/app/build && main.exe & exit 0

build-and-run: compile run

clean:
	@cd src/app && make clean
	@if exist temp\dec_out rmdir /S /Q temp\dec_out
	@if exist temp\env_in rmdir /S /Q temp\env_in
	@mkdir temp\dec_out
	@mkdir temp\env_in
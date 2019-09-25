
# GoLite Compiler

## Group 09 members
- Nathan Sandum 260716168
- Jiawen Wang 260683456
- Grégoire Moreau 260874685

### Scripts (from template)
* `programs`: Test programs are organized by compilation phase and by expected result. Valid programs should output `OK` and status code `0`, while invalid programs should output `Error: <description>` and status code `1`.
  * `Scan+parse`: Runs both the scanner and parser phases
* `src`: Source code for the assignment
* `build.sh`: Builds your compiler using `Make` or similar. You should replace the commands here if necessary to build your compiler
* `run.sh`: Runs the compiler using two arguments (mode - $1 and input file - $2).
* `test.sh`: Automatically run the compiler against test programs in the programs directory and checks the output
* `execute.sh` Generates code and compiles it

### Additional Scripts

* `prettytest.sh`: Automatically run the compiler and pretty printer to ensure pretty printer output is valid
* `reftest.sh`: Automatically `ssh` into the `teaching.cs.mcgill.ca` server to test against the reference compiler at `~cs520/golitec`

### Usage Notes
* `codegen.c` should be compiled with -lm, since we make use of `math.h`

### Known Issues

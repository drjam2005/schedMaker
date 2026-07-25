# Overkill Schedule Maker

`scheds.txt` uses tab-separated rows copied from the registrar table. The
columns are: subject code, subject, units, section, day/time, room, instructor,
and open slots. Rows marked `CLOSED` are ignored. Time values may use `AM`,
`PM`, or the registrar's `NN` suffix for noon.

```text
SUBJECT CODE	SUBJECT	UNITS	SECTION	DAY - TIME	ROOM	INSTRUCTOR	OPEN SLOTS
CSDC100	Introduction to Computing	3	ZC12Am	TTH 06:00PM - 07:30PM	AL211A/CSLAB1	K.G. Tapel	23
```


# Building
- Dependencies
    - GCC
    - CMake
- ```sh
  $ mkdir build
  $ cd build
  $ cmake ..
  $ make
  ```
# Executing
    ```sh
    $ cd ./build
    $ <edit scheds.txt>
    $ ./Scheduler
    ```

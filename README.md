# Overkill Schedule Maker
- format for scheds.txt
```
// subject
SUBJ: <subj-code>
  PROF: <prof-name>                       // 2 spaces indentation
    <section>                             // 4 spaces indentationes
      <date> <startTime> <endTime> <room> // 6 spaces indentationes
      ...                                 // 6 spaces indentationes
      <date> <startTime> <endTime> <room> // 6 spaces indentationes
```

example:
```
SUBJ: CSDC105
  PROF: Professor1
    ZC21Am
      MW 9:00 10:30 room1
      TTH 9:00 10:00 room2
        
    ZC22Am
      MW 10:30 12:00 room3
      TTH 10:00 11:00 room4
    
  PROF: Professor2
    ZT21Am
      MW 18:00 19:00 room5
      TTH 16:30 18:00 room6

SUBJ: CSMC221
  PROF: Professor3
    ZC21Am
      TTH 13:30 15:00 room7
    
    ZC22Am
      TTH 15:00 16:30 room8
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
    $ ./exec
    ```

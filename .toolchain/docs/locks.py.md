# .toolchain/tools/locks.py

`locks.py` holds the locks of the uses of the machine: `.toolchain/docs/test.py.md`, *The uses*.

A lock is the file `<use>.lock` in the folder of the locks. The variable `MC23T_LOCKS` names that folder. With no such variable, it is `mc23t-locks` in the temporary folder of the system.

On Linux and macOS, that folder is the same for each user. `locks.py` makes it writable by each user, and each file of a lock readable and writable by each user. Thus the runs of each user of the machine share the locks.
On Windows, the temporary folder is of 1 user. Thus only the runs of that user share the locks, unless `MC23T_LOCKS` names a folder of each user.

A lock is a lock of the system: `msvcrt.locking` on Windows, and `fcntl.flock` on the other systems. Thus a run that ends, also by a crash, gives back each lock that it holds.
Only an answer of the system that another run holds the lock makes a run wait. Each other error of the system stops the run.

```sh
python .toolchain/tools/locks.py <folder> <seconds> <use> ...
```

The command takes the locks of the uses in the folder, and writes `held <uses>`. It gives them back when its input ends, or after the seconds. A case uses it to hold a use while another run waits.

## Functions

| Function | Result |
| --- | --- |
| `folder()` | Gives the folder of the locks. |
| `took(handle)` | Takes the lock of an open file, and gives true. It gives false at once if another run holds it. It raises each other error of the system. |
| `gave(handle)` | Gives back the lock of an open file, and closes it. |
| `opened(name)` | Makes the folder and the file of a use if they are not there, with the modes for each user, and opens the file. |
| `mine(name)` | Gives the lock of a use for the threads of this run. |
| `held(names, group)` | Takes the lock of each use, in the order of the names, and gives them back at the end. For each use, it first takes the lock of `mine`, so that only 1 thread of a run waits for the file of the use. While it waits for the file, it writes the line of a wait: `.toolchain/docs/test.py.md`, *The uses*. |
| `main()` | Reads the folder, the seconds and the uses, and holds them until its input ends or the seconds pass. |

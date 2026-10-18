# src/lib/backend/backend.c

This file opens the library of a backend and finds its eighteen entries by name.
`docs/intern/lib/backend/backend.h.md`, *The entries of a backend*, tells what each entry is for.

## The steps

1. Open the library with `os_lib_open`. If it does not open, refuse it by its path.
2. Find each entry by its name, in the order of `BACKEND_ENTRY_NAMES`.
3. After the first entry, compare the number that it gives with `BACKEND_ABI`.
4. If an entry is missing or the number is another, write why in `err`, and close the library.

`find_entries` fills `backend_api_t` as an array, because each member of the struct is a function pointer.

## The number of the backend seam

A library gives the number of the backend seam that it was built against from its first entry, `backend_get_abi`.
A library of another number is not asked for the other entries.
`docs/intern/lib/bridge/bridge_create.c.md`, *The numbers of the seams*, tells why each seam has a number.

## Functions

| Function | Result |
| --- | --- |
| `find_entries` | Fills `api` with the entries of the library. Gives how many it took: all of them, or the index of the entry where it stopped. |
| `say_stopped` | Writes in `err` why `find_entries` stopped: the library lacks the entry, or the first entry gave another number. |
| `backend_open` | Opens the library of a backend and finds its entries. Gives the library, or null with `err` that tells why, and the library closed. |

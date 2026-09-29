# Open MPI — native Windows port

> **This fork adds a native Windows port of Open MPI** (branch
> [`win32-msvc-port`](https://github.com/WHUT666/ompi/tree/win32-msvc-port)):
> real Open MPI built with **MSVC / Visual Studio 2022 + CMake** —
> shared DLLs, C bindings, TCP communication, and the PMIx/PRRTE
> launcher stack running natively (no Cygwin/MSYS2 runtime, no
> POSIX-emulation launcher).
>
> The matching submodule changes live on the same branch in
> [`WHUT666/openpmix`](https://github.com/WHUT666/openpmix/tree/win32-msvc-port)
> and
> [`WHUT666/prrte`](https://github.com/WHUT666/prrte/tree/win32-msvc-port);
> this branch's `.gitmodules` already points at those forks, so
> `git clone --recursive` gets the whole working tree.
>
> **What works today** (validated on Windows 11, VS2022, Release):
> - `mpicc` compiles MPI C programs against the native DLLs
> - `mpirun` / `mpiexec` / `prterun` launch local jobs:
>   `mpirun -np 4 ring_c` passes a token around real MPI ranks over TCP
> - Persistent DVM: `prte --daemonize --report-uri uri.txt`,
>   `prun --dvm-uri file:uri.txt -n 2 hello_c`, `pterm --dvm-uri ...`
> - PMIx client/server over `tcp4`, PRRTE OOB/RML, IOF stdout/stderr
>   forwarding, child reaping via a polling `waitpid` registry
>
> **Not yet validated:** multi-node launches (the `plm/ssh` path builds
> but needs a second Windows host), Fortran/C++ bindings, and most
> `make check` coverage.
>
> **Layout:** Windows portability lives in `opal/win32/` (a POSIX shim
> over Winsock/CRT/CreateProcess) and generated forwarding headers; the
> CMake build system is in `cmake/` + the `CMakeLists.txt` files.
> Higher layers keep only small `#ifdef _WIN32` blocks.

## Upstream README

[The Open MPI Project](https://www.open-mpi.org/) is an open source
implementation of the [Message Passing Interface (MPI)
specification](https://www.mpi-forum.org/docs/) that is developed and
maintained by a consortium of academic, research, and industry
partners.  Open MPI is therefore able to combine the expertise,
technologies, and resources from all across the High Performance
Computing community in order to build the best MPI library available.
Open MPI offers advantages for system and software vendors,
application developers and computer science researchers.

## Official documentation

The Open MPI documentation can be viewed in the following ways:

1. Online at https://docs.open-mpi.org/
1. In self-contained (i.e., suitable for local viewing, without an
   internet connection) in official distribution tarballs under
   `docs/_build/html/index.html`.

## Building the documentation locally

The source code for Open MPI's docs can be found in the Open MPI Git
repository under the `docs` folder.

Developers who clone the Open MPI Git repository will not have the
HTML documentation and man pages by default; it must be built.
Instructions for how to build the Open MPI documentation can be found
here:
https://docs.open-mpi.org/en/main/developers/prerequisites.html#sphinx-and-therefore-python.

# Generate all OMPI-ABI C binding sources from the .c.in templates.
#
# Mirrors the Automake rules in ompi/mpi/c/Makefile.am:
#   <x>.c.in        -> <x>_ompi_generated.c        (flavor "ompi")
#   <x>.c.in_nbc    -> <x>_ompi_generated.c        (--suppress_bc)
#   <x>.c.in_obc    -> <x>_ompi_generated.c        (--suppress_nbc)
#
# Usage: gen_bindings.py <srcdir> <builddir> <outdir>
# Emits a CMake list file <outdir>/bindings_sources.cmake with
# set(BINDINGS_SOURCES ...) for the mpi target.

import glob
import os
import subprocess
import sys

def main():
    srcdir, builddir, outdir = sys.argv[1], sys.argv[2], sys.argv[3]
    os.makedirs(outdir, exist_ok=True)
    gen = os.path.join(srcdir, "ompi", "mpi", "bindings", "bindings.py")
    cdir = os.path.join(srcdir, "ompi", "mpi", "c")

    jobs = []  # (template, output_basename, extra_args)
    for tmpl in sorted(glob.glob(os.path.join(cdir, "*.c.in"))):
        base = os.path.basename(tmpl)[:-len(".c.in")]
        jobs.append((tmpl, base + "_ompi_generated.c", []))
    for tmpl in sorted(glob.glob(os.path.join(cdir, "*.c.in_nbc"))):
        base = os.path.basename(tmpl)[:-len(".c.in_nbc")]
        jobs.append((tmpl, base + "_ompi_generated.c", ["--suppress_bc"]))
    for tmpl in sorted(glob.glob(os.path.join(cdir, "*.c.in_obc"))):
        base = os.path.basename(tmpl)[:-len(".c.in_obc")]
        jobs.append((tmpl, base + "_ompi_generated.c", ["--suppress_nbc"]))

    outputs = []
    failed = 0
    for tmpl, outbase, extra in jobs:
        out = os.path.join(outdir, outbase)
        cmd = [sys.executable, gen,
               "--builddir", builddir, "--srcdir", srcdir,
               "--output", out, "c", "source", "ompi"] + extra + [tmpl]
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode != 0:
            sys.stderr.write("bindings gen failed: %s\n%s\n" % (tmpl, r.stderr))
            failed += 1
        else:
            outputs.append(out)
    if failed:
        sys.exit(1)

    with open(os.path.join(outdir, "bindings_sources.cmake"), "w") as f:
        f.write("set(BINDINGS_SOURCES\n")
        for o in outputs:
            f.write("    \"%s\"\n" % o.replace("\\", "/"))
        f.write(")\n")
    print("gen_bindings: generated %d sources in %s" % (len(outputs), outdir))

if __name__ == "__main__":
    main()

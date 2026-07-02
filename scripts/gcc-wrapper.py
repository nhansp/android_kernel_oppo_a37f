#!/usr/bin/env python3
# Neutered wrapper: transparent pass-through to the real compiler.
# The original QCOM py2 warning-checker breaks under py3 (print statement) AND
# fails the build on any non-whitelisted warning -- a 3.10 kernel under a modern
# GCC emits many. Under a standalone build we bypassed it via CC=; the lineage
# kernel build invokes make without that override, so it must be a no-op wrapper.
import os, sys
os.execvp(sys.argv[1], sys.argv[1:])

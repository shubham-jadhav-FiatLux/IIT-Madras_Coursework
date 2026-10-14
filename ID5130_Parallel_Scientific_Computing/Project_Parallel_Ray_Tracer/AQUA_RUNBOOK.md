# AQUA Runbook — actual working flow

End-to-end procedure for reproducing all paper measurements on the IIT Madras
AQUA cluster, validated through multiple submission cycles. Every step here is
the *actual* working command, not the original plan; pitfalls discovered along
the way are baked into the scripts (`cluster/aqua_env.sh`, `cluster/job_paper.pbs`)
or noted inline.

---

## TL;DR — the seven commands

```bash
# 1. Push code (NOT build/, NOT results/)
rsync -avz --delete --exclude build/ --exclude results/ -e 'ssh -p <port>' \
    submission/ <user>@<aqua-host>:~/rt_paper/

# 2. Build on AQUA (run this on AQUA after fresh rsync)
ssh -p <port> <user>@<aqua-host> 'cd ~/rt_paper && source cluster/aqua_env.sh && bash cluster/build.sh < /dev/null'

# 3. Sanity-check the binaries
ssh -p <port> <user>@<aqua-host> 'file ~/rt_paper/build/release/rt ~/rt_paper/build/release/rt_mpi'

# 4. Submit the master job
ssh -p <port> <user>@<aqua-host> 'cd ~/rt_paper && qsub cluster/job_paper.pbs'

# 5. (~14–18 h later, after the job emails you) pull results back
rsync -avzh --progress -e 'ssh -p <port>' <user>@<aqua-host>:~/rt_paper/results/ ./results/

# 6. Verify the run is clean
bash tools/verify_run.sh

# 7. Merge with prior runs (if any) and aggregate
bash tools/merge_results.sh   # only if you have multiple runs to combine
```

---

## 1. Pre-flight on AQUA

### 1a. Source the environment

```bash
ssh -p <port> <user>@<aqua-host>
cd ~/rt_paper
source cluster/aqua_env.sh
rt_env_check && echo "env OK"
```

What `aqua_env.sh` does (pitfalls noted):

| Component | Path | Why this exact name |
|---|---|---|
| GCC 9.2.0 | `/lfs/sware/gcc9.2.0/bin` | NOT `/lfs/sware/gcc920` — that's the module name; the directory has dots |
| OpenMPI 4.1.5 | `/lfs/sware/openmpi415/bin` | wraps gcc 9.2.0 cleanly |
| Anaconda3 | `/lfs/sware/anaconda3_2023/bin` | NOT `anaconda3_2023.09`; python385 lacks SSL so we can't use that |
| libLLVM-11 (Dr.Jit) | `/lfs/sware/anaconda3_2023/lib/libLLVM-11.so` | NOT `/opt/intel/...`; that path is login-node-only and invisible from compute nodes |
| Mitsuba venv | `~/local/mitsuba-venv` | created by `cluster/setup_mitsuba.sh` |

`rt_env_check` verifies:
- g++, mpicxx, python3 are on PATH
- g++ version ≥ 9.x (catches the 4.8.5 system-default failure mode)
- `DRJIT_LIBLLVM_PATH` resolves to an actual file on the current node

### 1b. Build the binaries

```bash
bash cluster/build.sh < /dev/null   # the < /dev/null prevents any rogue stdin prompt from hanging
```

Expected output:
```
Compiler: g++ (GCC) 9.2.0
MPI:      g++ (GCC) 9.2.0
g++ -std=c++17 ... -c src/main.cpp -o build/release/main.o
[...]
Built:
-rwxr-xr-x ... build/release/rt
-rwxr-xr-x ... build/release/rt_mpi
```

Verify both binaries are real Linux x86-64 ELF:
```bash
file build/release/rt build/release/rt_mpi
```

### 1c. Install Mitsuba 3 (one-time)

```bash
bash cluster/setup_mitsuba.sh 2>&1 | tee logs/setup_mitsuba.log
python3 -c "import mitsuba as mi; mi.set_variant('llvm_ad_rgb'); print('LLVM OK')"
```

The setup script pins `numpy==1.26.4` (numpy 2.x needs gcc ≥ 9.3 which AQUA doesn't have).

### 1d. (Optional) pbrt-v4

pbrt-v4 build is fragile on AQUA (upstream submodule SHAs are stale, libdeflate
CMake config mismatch). We have *deferred* this comparison from the paper's
Tier 3.1; the CSG-uniqueness claim holds without it (pbrt cannot represent CSG
geometry at all). To attempt anyway: `bash cluster/setup_pbrt.sh`. If `which pbrt`
resolves after that, the paper job picks it up automatically.

---

## 2. The master job

```bash
qsub cluster/job_paper.pbs
qstat -u $USER
```

Resource: `4 nodes × 40 cores × 48h walltime`. Realistic wall time ~14–18 hours
on a healthy queue.

### Defensive design (lessons from prior submissions)

The script writes everything to `/tmp/$USER_rtpaper_$JOBID/` on the head compute
node during the run, then atomically copies to `results/<TS>/` at the end.
Reason: `/lfs` showed coherence races on long-running jobs (file/dir vanishing
mid-write). `/tmp` is local disk and immune.

### Up-front correctness gate (~2 minutes)

The job aborts in <5 minutes — *before any tier runs* — if any of:
- `make test` fails
- `rt` binary doesn't produce identical Whitted output at T=1 vs T=40
- `rt_mpi` doesn't reproduce the serial Whitted output at np=4
- gcc on PATH is not ≥ 9.x
- `DRJIT_LIBLLVM_PATH` doesn't resolve on the compute node

### Tier inventory

| Tier | What | Output |
|---|---|---|
| 1.1 | Mitsuba 8192-SPP ground truth | `gt_mitsuba_8192spp.exr` (~2 MB) |
| 1.2 | OMP path-trace strong scaling, 7 thread counts × 5 reps | 35 rows |
| 1.3 | OMP headline (1024² × 1024 SPP) | 10 rows |
| 1.4 | Pure MPI strong scaling, np ∈ {1..160} × 5 reps | 45 rows |
| 1.5 | Adaptive vs static (OMP T=40 + MPI np=2 t=20) × 5 reps | 20 rows |
| 2.1 | Whitted strong scaling 1280×720, std + adaptive | 70 rows |
| 2.2 | Adaptive on 3 CSG scenes × 3 thread counts × 5 reps | 90 rows |
| 2.3 | Hybrid MPI+OMP, 9 splits × 5 reps | 45 rows |
| 2.4 | Weak scaling, 4 sizes | 12 rows |
| 2.5 | CSG-only at T=40, std + adaptive × 5 reps | 30 rows |
| 3.1 | pbrt-v4 sweep | skipped if not on PATH |
| 3.2 | Mitsuba scalar sweep, 5 SPPs × 3 reps | 15 rows |
| 3.3 | Mitsuba LLVM sweep, 5 SPPs × 3 reps | 15 rows |
| 3.4 | Time-to-quality sweep ours, 6 SPPs × 3 reps | 18 rows |

Expected total: ~410 rows.

---

## 3. After the job completes

The job emails you on finish (`#PBS -m abe` directive).

### Pull results

```bash
rsync -avzh --progress -e 'ssh -p <port>' <user>@<aqua-host>:~/rt_paper/results/        ./results/
rsync -avzh --progress -e 'ssh -p <port>' <user>@<aqua-host>:~/rt_paper/rt_paper.log    ./aqua_logs/
```

The `--exclude build/` from step 1 plus *no* `--delete` here means we never wipe
local data accidentally.

### Verify

```bash
bash tools/verify_run.sh
```

Pass criteria:

| Section | Pass |
|---|---|
| (a) rows + tiers | ≥400 rows; all 14 tier markers present |
| (b) failed runs | `0` |
| (c) Whitted bit-identity | `PASS` for whitted_showcase + each csg_* |
| (d) PT determinism | `PASS  N tuples each have exactly 1 hash` |
| (e) GT EXR | present, ~2 MB |
| (f) renderer coverage | `ours_omp ~250`, `ours_mpi ~95`, `mitsuba_scalar 15`, `mitsuba_llvm 16` |
| (g) MPI rank coverage | All 18 (rank, thread) configs with count `5` |
| (h) Super-linear | No `<-- SUPER-LINEAR!` flags; T=40 efficiency ≥80% |

### Aggregate (and merge with prior runs if needed)

```bash
# Single-run aggregate (already inside job_paper.pbs, but useful for re-aggregation)
python3 tools/aggregate.py results/<TS>/timings.csv -o results/<TS>/summary.csv

# OR — if you have multiple runs to combine (we did across this development cycle)
bash tools/merge_results.sh
```

The merged set lands in `results/merged/` with `timings.csv` (failed rows
filtered) and `summary.csv` (median + IQR per config).

---

## 4. Common pitfalls and what fixed them

| Symptom | Root cause | Fix |
|---|---|---|
| `mpirun: Error: unknown option "-ppn"` | `-ppn` is Intel-MPI / mpich syntax; AQUA's openmpi415 doesn't accept it | Drop `-ppn` entirely (default slot-fill is fine on single node); use `--map-by ppr:N:node` for explicit cross-node placement |
| `g++: error: unrecognized command line option '-std=c++17'` | system `/usr/bin/g++` is 4.8.5; gcc920 didn't make it onto PATH | `aqua_env.sh` prepends `/lfs/sware/gcc9.2.0/bin` *with the dots*; `rt_env_check` verifies version ≥ 9 |
| `libLLVM-11.so: cannot open shared object file` | `/opt/intel/...` exists only on login node; compute nodes don't see it | `aqua_env.sh` resolves `DRJIT_LIBLLVM_PATH` dynamically — tries `/lfs/sware/anaconda3_2023/lib/...` first (global FS) |
| `mpirun: ... could not access or execute Executable: build/release/rt_mpi` | rsync `--delete submission/` wiped `~/rt_paper/build/` because submission/ has no build/ subtree | Always rsync with `--exclude build/` AND `--exclude results/` |
| `tee: results/.../run.log: No such file or directory` mid-job | NFS coherence race on /lfs — directory vanished after `mkdir -p` succeeded | Job script writes to `/tmp/$USER_rtpaper_$JOBID/` and copies to `results/` only at the end |
| `rank 0 error: cannot open scene file` | mpirun on compute nodes launches workers with a different cwd than the launcher | `tools/bench.py` passes `mpirun --wdir "$(pwd)"` and resolves all paths to absolute |
| Job runs but no output for 30+ min | `bash cluster/build.sh \| tail -20` — tail buffers until upstream EOF, looks frozen | Drop the pipe: `bash cluster/build.sh < /dev/null` |
| `bash: ...: command not found` for what should be a quoted argument | zsh on macOS wraps long single-line ssh commands at ~200 chars | Use heredoc form: `ssh ... 'bash -s' <<'REMOTE'`...`REMOTE` |

---

## 5. Quick reference

| Item | Setting |
|---|---|
| Login modules (best-effort) | `anaconda3_2023.09` → `gcc920` → `openmpi415` |
| Real compute-node paths | `/lfs/sware/{anaconda3_2023, gcc9.2.0, openmpi415}` |
| Python venv | `~/local/mitsuba-venv` |
| Dr.Jit LLVM | `/lfs/sware/anaconda3_2023/lib/libLLVM-11.so` |
| ssh port | <port> |
| User on AQUA | <user> |
| Email for PBS notifications | <your-email> |
| Output PPM/EXR location during run | `output/_bench/` |
| Final results | `results/<TS>/` (after job completes) |
| Merged dataset | `results/merged/` |

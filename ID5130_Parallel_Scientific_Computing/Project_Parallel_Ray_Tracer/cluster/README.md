# AQUA Cluster — Run Guide

Cluster: IIT Madras AQUA | Intel Xeon Gold 6248 (2×20 cores, 40 physical / 80 logical)
Scheduler: PBS Professional | Interconnect: InfiniBand 100 Gbps

---

## Step 1 — Transfer code to AQUA

Run this on your Mac:
```bash
rsync -avz --exclude='output/' --exclude='build/' --exclude='*.ppm' \
    ./ \
    <user>@<aqua-host>:~/rt_project/
```

## Step 2 — Build on AQUA

SSH in, then:
```bash
cd ~/rt_project
bash cluster/build.sh
```

Expected output: two binaries `build/release/rt` and `build/release/rt_mpi`.

## Step 3 — Queue reference

| Queue    | Nodes | Use for                                      |
|----------|-------|----------------------------------------------|
| medium   | 80    | default — both jobs use this ← OMP + MPI     |
| long     | 80    | same as medium but longer walltime allowed   |
| verylong | 80    | longest walltime tier                        |
| large    | 46    | large-scale runs if medium is congested      |

Both .pbs files are set to `#PBS -q medium`.
Switch to `long` only if `medium` has a strict walltime that cuts off the 4-hour MPI job.

## Step 4 — Submit jobs

```bash
cd ~/rt_project

# Single-node OMP scaling (~3h)
qsub cluster/job_omp_scaling.pbs

# Multi-node MPI+hybrid scaling (~4h, uses 4 nodes)
qsub cluster/job_mpi_scaling.pbs
```

Check status:
```bash
qstat -a           # all jobs
qstat -u cs24s009  # your jobs only
```

Cancel a job:
```bash
qdel <jobid>
```

## Step 5 — Retrieve and parse results

After jobs complete, log files appear in `~/rt_project/`:
- `rt_omp_scaling.log`
- `rt_mpi_scaling.log`

Parse and print tables:
```bash
python3 cluster/collect_results.py rt_omp_scaling.log rt_mpi_scaling.log
```

Copy logs back to Mac:
```bash
scp <user>@<aqua-host>:~/rt_project/rt_omp_scaling.log \
    ./
scp <user>@<aqua-host>:~/rt_project/rt_mpi_scaling.log \
    ./
```


---

## Node topology (Xeon Gold 6248)

```
Node: 40 physical cores / 80 logical (HT)
  Socket 0: cores  0-19, HT threads 40-59   (NUMA node 0)
  Socket 1: cores 20-39, HT threads 60-79   (NUMA node 1)
L3: 27.5 MB per socket (28160 KB)
RAM: 188 GB per node
```

Optimal hybrid split for single node:
- 2 MPI ranks × 20 OMP threads = 1 rank per socket (NUMA-optimal)

For multi-node (N nodes):
- N×2 MPI ranks × 20 OMP threads each

---

## Troubleshooting

**"mpicxx not found" after module load**  
The default Intel MPI is already in PATH on AQUA. Just run `bash cluster/build.sh` directly.

**Job stuck in queue (state Q)**  
Check with `qstat -a`. May need a specific queue: add `#PBS -q <name>` to the .pbs files.

**OMP_NUM_THREADS is wrong in job**  
The scripts set it explicitly via shell variable before each run — no need for a global export.

**Output PPMs not found after job**  
PPMs go to `output/` in `$PBS_O_WORKDIR` (= the directory where qsub was run). Make sure you
submit from `~/rt_project/`.

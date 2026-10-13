// Author   :   Shubham Balaso Jadhav
// Date     :   20 April, 2025

// Header Files
#include "simulator.hpp"
#include <algorithm>
#include <vector>
#include <cmath>
#include <climits>

// Define thresholds
const double BASE_CPU_BOUND_THRESHOLD = 100; // ms average CPU burst
const double BASE_IO_BOUND_THRESHOLD = 0.6;  // fraction of IO bursts

// Dynamic thresholding
const double LOW_LOAD_THRESHOLD = 0.4;  // system load < 40% -> performance mode
const double HIGH_LOAD_THRESHOLD = 0.7; // system load > 70% -> energy-saving mode

// Helper enum for classification
enum ProcType { CPU_BOUND, IO_BOUND, MIXED };

// Attach classification to processes
static std::map<unsigned, ProcType> procTypeMap;

static std::map<int, int> idleEpochs; // core_id -> idle epoch count
const int GRACE_PERIOD = 1; // number of epochs to wait before powering off


void System::epoch(unsigned long currentTime) {

    if (schedulerScheme == SchedulerScheme::CLUSTERING) {
        static int nextCore = 0;  // Initialize only once
        
        // Determine which cluster is currently active
        bool littleActive = false;
        bool bigActive = false;
        
        for (int i = 0; i < 4; ++i) {
            if (cores[i]->getState() == CoreState::POWER_ON) {
                littleActive = true;
                break;
            }
        }
        
        for (int i = 4; i < 8; ++i) {
            if (cores[i]->getState() == CoreState::POWER_ON) {
                bigActive = true;
                break;
            }
        }
        
        // Activate Little cluster if no cores are initially powered on
        if (!littleActive && !bigActive) {
            for (int i = 0; i < 4; ++i) cores[i]->powerOnCore();
            littleActive = true;
            nextCore = 0;  // Reset to the first LITTLE core
        }
        
        // Calculate average load for the active cluster
        double totalLoad = 0.0;
        int totalCores = 0;
    
        if (littleActive) {
            for (int i = 0; i < 4; ++i) {
                if (cores[i]->getState() == CoreState::POWER_ON) {
                    totalCores++;
                    if (!cores[i]->getLoadHistory().empty()) {
                        totalLoad += cores[i]->getLoadHistory().back();
                    }
                }
            }
        } else if (bigActive) {
            for (int i = 4; i < 8; ++i) {
                if (cores[i]->getState() == CoreState::POWER_ON) {
                    totalCores++;
                    if (!cores[i]->getLoadHistory().empty()) {
                        totalLoad += cores[i]->getLoadHistory().back();
                    }
                }
            }
        }
    
        if (totalCores == 0) return;
        double avgLoad = (totalLoad / totalCores) * 100.0 / EPOCH_TIME;
    
        
        // Switch clusters based on load
        if (littleActive && avgLoad >= 50.0) {
            // Switch from Little to Big
            for (int i = 0; i < 4; ++i) {
                Core* src = cores[i];
                Core* dst = cores[i + 4];
                
                // Power on the destination core before migrating
                dst->powerOnCore();
                
                // Migrate all processes from src to dst
                auto processes = src->getScheduledProcesses();
                for (auto p : processes) {
                    migrateProcess(p, src, dst);
                }
                
                // Power off the source core after migration
                src->powerOffCore();
            }
            
            // Update cluster state and reset nextCore for round-robin
            littleActive = false;
            bigActive = true;
            nextCore = 4;  // Reset to the first BIG core
        } else if (bigActive && avgLoad < 50.0) {
            // Switch from Big to Little
            for (int i = 4; i < 8; ++i) {
                Core* src = cores[i];
                Core* dst = cores[i - 4];
                
                // Power on the destination core before migrating
                dst->powerOnCore();
                
                // Migrate all processes from src to dst
                auto processes = src->getScheduledProcesses();
                for (auto p : processes) {
                    migrateProcess(p, src, dst);
                }
                
                // Power off the source core after migration
                src->powerOffCore();
            }
            
            // Update cluster state and reset nextCore for round-robin
            littleActive = true;
            bigActive = false;
            nextCore = 0;  // Reset to the first LITTLE core
        }
        
        // Schedule newly arrived processes to the active cluster
        std::vector<Process*> arrived;
        for (auto& entry : processes) {
            Process* p = entry.second;
            if (p->getState() == ProcessState::ARRIVED)
                arrived.push_back(p);
        }
    
        // 2) True 4-core RR within the active cluster
        const int base      = littleActive ? 0 : 4;   // start of active cluster
        static int rr       = 0;                      // always 0…3
        const int CLUSTER_SZ = 4;
    
        for (Process* p : arrived) {
            int coreId = base + rr;                    // maps 0→base, 1→base+1, …, 3→base+3
            Core* core = cores[coreId];
            if (core->getState() == CoreState::POWER_OFF)
                core->powerOnCore();
            scheduleProcess(p, core);
            rr = (rr + 1) % CLUSTER_SZ;
        }
    
    } else if (schedulerScheme == SchedulerScheme::SWITCHER) {
    
        for (int i = 0; i < 4; ++i) {
            Core* little = cores[i];
            Core* big = cores[i + 4];
            bool littleOn = little->getState() == CoreState::POWER_ON;
            bool bigOn = big->getState() == CoreState::POWER_ON;
    
            Core* active = nullptr;
            if (littleOn) active = little;
            else if (bigOn) active = big;
            else {
                little->powerOnCore();
                active = little;
            }
    
            if (!active->getLoadHistory().empty()) {
                unsigned load = active->getLoadHistory().back();
                double loadPercent = (load * 100.0) / EPOCH_TIME;
    
                if (active == little && loadPercent >= 50.0) {
    
                    if (big->getState() == CoreState::POWER_OFF)
                        big->powerOnCore();
                    auto processes = active->getScheduledProcesses();
                    for (auto p : processes) migrateProcess(p, active, big);
                    active->powerOffCore();
                   
                } else if (active == big && loadPercent < 50.0) {
                    if (little->getState() == CoreState::POWER_OFF)
                        little->powerOnCore();
                    auto processes = active->getScheduledProcesses();
                    for (auto p : processes) migrateProcess(p, active, little);
                    active->powerOffCore();
                    
                }
            }
        }
    
        // Schedule arrived processes to active cores in strict Round-Robin
        std::vector<Process*> arrived;
        for (auto& entry : processes) {
            Process* p = entry.second;
            if (p->getState() == ProcessState::ARRIVED) {
                arrived.push_back(p);
            }
        }
    
        static int nextPair = 0;
        for (Process* p : arrived) {
            int pairIdx = nextPair % 4;
            Core* little = cores[pairIdx];
            Core* big = cores[pairIdx + 4];
            Core* active = nullptr;
    
            if (little->getState() == CoreState::POWER_ON) active = little;
            else if (big->getState() == CoreState::POWER_ON) active = big;
            else {
                little->powerOnCore();
                active = little;
            }
    
            if (active->getState() == CoreState::POWER_OFF)
                active->powerOnCore();
      
            scheduleProcess(p, active);
            nextPair = (nextPair + 1) % 4;
        }
    
    } else if (schedulerScheme == SchedulerScheme::GLOBAL_SCHEDULING) {
        
        // Ensure all 8 cores remain powered on per specification
        for(int i=0; i<8; i++) {
            if (cores[i]->getState() == CoreState::POWER_OFF) {
                cores[i]->powerOnCore();
            }
        }
    
        // Collect arrived processes without PID sorting
        std::vector<Process*> arrived;
        for (auto& entry : processes) {
            Process* p = entry.second;
            if (p->getState() == ProcessState::ARRIVED) {
                arrived.push_back(p);
            }
        }
    
        // Maintain round-robin state across epochs
        static int nextCore = 0;
        
        // Schedule processes in arrival order (input guarantees sorted arrival times)
        for (Process* p : arrived) {
            Core* targetCore = cores[nextCore];
            scheduleProcess(p, targetCore);
            nextCore = (nextCore + 1) % 8;
        }
    
    }
    else if (schedulerScheme == SchedulerScheme::BONUS) {

        // 1. Dynamic Threshold Adjustment based on system load (approximate using active cores)
        double activeCores = 0.0;
        for (auto& entry : cores) {
            if (entry.second->getState() == POWER_ON)
                activeCores += 1.0;
        }
        double systemLoad = activeCores / cores.size();

        double CPU_BOUND_THRESHOLD = BASE_CPU_BOUND_THRESHOLD;
        double IO_BOUND_THRESHOLD = BASE_IO_BOUND_THRESHOLD;

        if (systemLoad < LOW_LOAD_THRESHOLD) {
            CPU_BOUND_THRESHOLD *= 0.8; // be more aggressive
            IO_BOUND_THRESHOLD *= 0.8;
        } else if (systemLoad > HIGH_LOAD_THRESHOLD) {
            CPU_BOUND_THRESHOLD *= 1.2; // be more conservative
            IO_BOUND_THRESHOLD *= 1.2;
        }

        // 2. Classify all non-terminated processes
        for (auto& entry : processes) {
            Process* p = entry.second;
            if (p->getState() == TERMINATED) continue;

            auto pastBursts = p->getPastBursts();
            if (pastBursts.empty()) {
                procTypeMap[p->getId()] = CPU_BOUND; // optimistic
                continue;
            }
            double totalCpu = 0;
            int cpuCount = 0, ioCount = 0;
            for (auto& burst : pastBursts) {
                if (burst.first == BurstType::CPU) {
                    totalCpu += burst.second;
                    cpuCount++;
                } else if (burst.first == BurstType::IO) {
                    ioCount++;
                }
            }
            double avgCpu = cpuCount ? totalCpu / cpuCount : 0.0;
            double ioRatio = (cpuCount + ioCount) ? (double)ioCount / (cpuCount + ioCount) : 0.0;

            if (avgCpu > CPU_BOUND_THRESHOLD && ioRatio < IO_BOUND_THRESHOLD)
                procTypeMap[p->getId()] = CPU_BOUND;
            else if (ioRatio > IO_BOUND_THRESHOLD)
                procTypeMap[p->getId()] = IO_BOUND;
            else
                procTypeMap[p->getId()] = MIXED;
        }

        // 3. Schedule newly ARRIVED processes
        std::vector<Process*> arrived;
        for (auto& entry : processes) {
            Process* p = entry.second;
            if (p->getState() == ARRIVED)
                arrived.push_back(p);
        }

        static int rrLittle = 0; // round robin pointers
        static int rrBig = 0;

        for (Process* p : arrived) {
            ProcType type = procTypeMap.count(p->getId()) ? procTypeMap[p->getId()] : CPU_BOUND;
            if (type == CPU_BOUND) {
                // Prefer busy Big cores first
                int bestCore = -1, minLoad = INT_MAX;
                for (int i = 4; i < 8; ++i) {
                    if (cores[i]->getState() == POWER_OFF) continue;
                    int load = cores[i]->getScheduledProcesses().size();
                    if (load < minLoad) { minLoad = load; bestCore = i; }
                }
                if (bestCore == -1) bestCore = 4 + (rrBig++ % 4);
                if (cores[bestCore]->getState() == POWER_OFF)
                    cores[bestCore]->powerOnCore();
                scheduleProcess(p, cores[bestCore]);
            } else { // IO_BOUND or MIXED
                // Pack into as few Little cores as possible
                int bestCore = 0, minLoad = INT_MAX;
                for (int i = 0; i < 4; ++i) {
                    int load = cores[i]->getScheduledProcesses().size();
                    if (load < minLoad) { minLoad = load; bestCore = i; }
                }
                if (cores[bestCore]->getState() == POWER_OFF)
                    cores[bestCore]->powerOnCore();
                scheduleProcess(p, cores[bestCore]);
            }
        }

        // 4. Migrate processes aggressively (READY + WAITING_FOR_IO)
        for (auto& entry : processes) {
            Process* p = entry.second;
            if (p->getState() != READY && p->getState() != WAITING_FOR_IO) continue;

            Core* currentCore = nullptr;
            int cid = -1;
            for (auto& centry : cores) {
                auto sched = centry.second->getScheduledProcesses();
                if (std::find(sched.begin(), sched.end(), p) != sched.end()) {
                    currentCore = centry.second;
                    cid = currentCore->getId();
                    break;
                }
            }
            if (!currentCore) continue;

            ProcType type = procTypeMap.count(p->getId()) ? procTypeMap[p->getId()] : MIXED;

            if (type == CPU_BOUND && cid < 4) {
                int bestCore = 4, minLoad = INT_MAX;
                for (int i = 4; i < 8; ++i) {
                    int load = cores[i]->getScheduledProcesses().size();
                    if (load < minLoad) { minLoad = load; bestCore = i; }
                }
                if (bestCore != cid) {
                    if (cores[bestCore]->getState() == POWER_OFF)
                        cores[bestCore]->powerOnCore();
                    migrateProcess(p, currentCore, cores[bestCore]);
                }
            }
            else if (type == IO_BOUND && cid >= 4) {
                int bestCore = 0, minLoad = INT_MAX;
                for (int i = 0; i < 4; ++i) {
                    int load = cores[i]->getScheduledProcesses().size();
                    if (load < minLoad) { minLoad = load; bestCore = i; }
                }
                if (bestCore != cid) {
                    if (cores[bestCore]->getState() == POWER_OFF)
                        cores[bestCore]->powerOnCore();
                    migrateProcess(p, currentCore, cores[bestCore]);
                }
            }
        }

        // 5. Power off idle cores (both Big and Little) with grace period
        for (int i = 0; i < 8; ++i) {
            if (cores[i]->getScheduledProcesses().empty()) {
                idleEpochs[i]++;
                if (idleEpochs[i] >= GRACE_PERIOD)
                    if (cores[i]->getState() == POWER_ON)
                        cores[i]->powerOffCore();
            } else {
                idleEpochs[i] = 0; // reset counter if core has work
            }
        }
    }
}
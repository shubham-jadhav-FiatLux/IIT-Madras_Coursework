#include "simulator.hpp"
#include <algorithm>

void System::epoch(unsigned long currentTime) {

    if (schedulerScheme == SchedulerScheme::CLUSTERING) {
        static bool littleActive = true;

        static int nextLittleCore = 0;
        static int nextBigCore = 4;

        // Calculate average load for active cluster
        double totalLoad = 0.0;
        int activeCoresCount = 0;
        int clusterStart = littleActive ? 0 : 4;
        int clusterEnd = littleActive ? 4 : 8;

        for (int i = clusterStart; i < clusterEnd; ++i) {
            if (!cores[i]->getLoadHistory().empty()) {
                totalLoad += cores[i]->getLoadHistory().back();
                activeCoresCount++;
            }
        }

        double avgLoad = activeCoresCount > 0 ? 
            (totalLoad / activeCoresCount) * 100.0 / EPOCH_TIME : 0.0;

        // Cluster switching logic
        if (littleActive && avgLoad >= 50.0) {
            // Migrate from Little to Big cluster
            for (int i = 0; i < 4; ++i) {
                Core* src = cores[i];
                Core* dst = cores[i+4];
                dst->powerOnCore();
                auto processes = src->getScheduledProcesses();
                for (auto p : processes) migrateProcess(p, src, dst);
                src->powerOffCore();
            }
            littleActive = false;
            nextBigCore = 4; // Reset big cluster pointer
        }
        else if (!littleActive && avgLoad < 50.0) {
            // Migrate from Big to Little cluster
            for (int i = 4; i < 8; ++i) {
                Core* src = cores[i];
                Core* dst = cores[i-4];
                dst->powerOnCore();
                auto processes = src->getScheduledProcesses();
                for (auto p : processes) migrateProcess(p, src, dst);
                src->powerOffCore();
            }
            littleActive = true;
            nextLittleCore = 0; // Reset little cluster pointer
        }

        // Power on active cluster
        int activeStart = littleActive ? 0 : 4;
        int activeEnd = littleActive ? 4 : 8;
        for (int i = activeStart; i < activeEnd; ++i) {
            cores[i]->powerOnCore();
        }

        // Schedule new arrivals with proper round-robin
        std::vector<Process*> arrived;
        for (const auto& pair : processes) {
            Process* p = pair.second;
            if (p->getArrivalTime() == currentTime && 
                p->getState() != ProcessState::TERMINATED) {
                arrived.push_back(p);
            }
        }

        int& activeNextCore = littleActive ? nextLittleCore : nextBigCore;
        const int clusterSize = 4;
        bool scheduled = false;
        
        for (auto p : arrived) {
            // Find next idle core in active cluster
            for (int tries = 0; tries < (clusterEnd - clusterStart); tries++) {
                Core* target = cores[activeNextCore];
                if (target->getScheduledProcesses().empty()) {
                    scheduleProcess(p, target);
                    scheduled = true;
                    break;
                }
                activeNextCore++;
                if (activeNextCore >= clusterEnd) activeNextCore = clusterStart;
            }
            
            // Update round-robin pointer only if scheduled
            if (scheduled) {
                activeNextCore++;
                if (activeNextCore >= clusterEnd) activeNextCore = clusterStart;
            }
        }

        // Persist cluster-specific round-robin state
        if (littleActive) {
            nextLittleCore = activeNextCore;
        } else {
            nextBigCore = activeNextCore;
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

    } else if (schedulerScheme == SchedulerScheme::BONUS) {
        std::vector<Process*> arrived;
        for (auto& entry : processes) {
            Process* p = entry.second;
            if (p->getState() == ProcessState::ARRIVED) {
                arrived.push_back(p);
            }
        }

        for (Process* p : arrived) {
            unsigned totalCpu = 0;
            auto bursts = p->getPastBursts();
            for (auto& burst : bursts) {
                if (burst.first == BurstType::CPU) totalCpu += burst.second;
            }

            Core* target = nullptr;
            // New processes or light tasks go to Little cores
            if (bursts.empty() || totalCpu <= 20) {
                for (int i = 0; i < 4; ++i) {
                    Core* c = cores[i];
                    if (c->getState() == CoreState::POWER_OFF) c->powerOnCore();
                    if (!target || c->getScheduledProcesses().size() < target->getScheduledProcesses().size()) {
                        target = c;
                    }
                }
            } else {
                // Heavy tasks go to Big cores
                for (int i = 4; i < 8; ++i) {
                    Core* c = cores[i];
                    if (c->getState() == CoreState::POWER_OFF) c->powerOnCore();
                    if (!target || c->getScheduledProcesses().size() < target->getScheduledProcesses().size()) {
                        target = c;
                    }
                }
            }

            if (target) {
                target->powerOnCore();
                scheduleProcess(p, target);
            }
        }

        // Power off only truly idle cores
        for (auto& entry : cores) {
            Core* core = entry.second;
            if (core->getScheduledProcesses().empty() && core->getState() == CoreState::POWER_ON) {
                core->powerOffCore();
            }
        }
    }
}
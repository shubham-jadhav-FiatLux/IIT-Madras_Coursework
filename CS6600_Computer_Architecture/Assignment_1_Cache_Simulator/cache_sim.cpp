// CS6600 Assignment 1: cache and memory hierarchy simulator (L1 [+ victim cache] [+ L2], WBWA, LRU, non-inclusive L2)
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <algorithm>
#include "parse.h"

struct Block {
    uint32_t tag;      // for the victim cache: the full block address
    bool dirty;
};

// One level of the hierarchy. Every set (and the victim cache) is kept in MRU -> LRU order.
class Cache {
public:
    Cache(uint32_t size, uint32_t assoc, uint32_t blockSize, uint32_t vcBlocks, Cache* nextLevel)
        : assoc_(assoc), blockSize_(blockSize), vcBlocks_(vcBlocks), next_(nextLevel)
    {
        sets_ = size / (assoc * blockSize);
        offsetBits_ = log2u(blockSize);
        indexBits_ = log2u(sets_);
        set_.assign(sets_, std::vector<Block>());
    }

    // Serve a read ('r') or a write ('w') of the block containing addr.
    void access(char rw, uint32_t addr)
    {
        uint32_t blockAddr = addr >> offsetBits_;
        uint32_t index = blockAddr & (sets_ - 1);
        uint32_t tag = blockAddr >> indexBits_;
        std::vector<Block>& s = set_[index];
        if (rw == 'r') reads++; else writes++;

        for (size_t i = 0; i < s.size(); i++) {
            if (s[i].tag == tag) {                       // hit: move to MRU
                Block b = s[i];
                if (rw == 'w') b.dirty = true;
                s.erase(s.begin() + i);
                s.insert(s.begin(), b);
                return;
            }
        }
        if (rw == 'r') readMisses++; else writeMisses++;

        bool full = s.size() == assoc_;
        if (full && vcBlocks_ > 0) {
            swapRequests++;
            Block victim = s.back();                     // L1 LRU block V (its tag is rebuilt into a block address)
            uint32_t victimAddr = (victim.tag << indexBits_) | index;
            for (size_t j = 0; j < vc_.size(); j++) {
                if (vc_[j].tag == blockAddr) {           // swap X (in the VC) with V (in L1)
                    swaps++;
                    Block x = vc_[j];
                    vc_.erase(vc_.begin() + j);
                    vc_.insert(vc_.begin(), Block{victimAddr, victim.dirty});
                    s.pop_back();
                    if (rw == 'w') x.dirty = true;
                    s.insert(s.begin(), Block{tag, x.dirty});
                    return;
                }
            }
            // VC miss: V enters the VC, whose LRU block leaves (and is written back if dirty)
            s.pop_back();
            if (vc_.size() == vcBlocks_) {
                Block out = vc_.back();
                vc_.pop_back();
                if (out.dirty) writeBack(out.tag << offsetBits_);
            }
            vc_.insert(vc_.begin(), Block{victimAddr, victim.dirty});
        } else if (full) {
            Block victim = s.back();
            s.pop_back();
            if (victim.dirty) writeBack(((victim.tag << indexBits_) | index) << offsetBits_);
        }
        if (next_) next_->access('r', addr);             // fetch X from the next level
        s.insert(s.begin(), Block{tag, rw == 'w'});
    }

    void printContents(const char* title, bool victimCache) const
    {
        printf("\n===== %s contents =====\n", title);
        if (victimCache) {
            printf("  set\t0:\t");
            for (const Block& b : vc_) printf("%x %c\t", b.tag, b.dirty ? 'D' : ' ');
            printf("\n");
            return;
        }
        for (uint32_t i = 0; i < sets_; i++) {
            printf("  set\t%u:\t", i);
            for (const Block& b : set_[i]) printf("%x %c\t", b.tag, b.dirty ? 'D' : ' ');
            printf("\n");
        }
    }

    uint64_t reads = 0, readMisses = 0, writes = 0, writeMisses = 0;
    uint64_t swapRequests = 0, swaps = 0, writebacks = 0;

private:
    static uint32_t log2u(uint32_t x) { uint32_t r = 0; while ((1u << r) < x) r++; return r; }

    void writeBack(uint32_t addr)
    {
        writebacks++;
        if (next_) next_->access('w', addr);
    }

    uint32_t assoc_, blockSize_, vcBlocks_, sets_, offsetBits_, indexBits_;
    Cache* next_;
    std::vector<std::vector<Block>> set_;
    std::vector<Block> vc_;
};

int main(int argc, char* argv[])
{
    if (argc != 8) {
        fprintf(stderr, "Usage: %s <L1_SIZE> <L1_ASSOC> <L1_BLOCKSIZE> <VC_NUM_BLOCKS> <L2_SIZE> <L2_ASSOC> <trace_file>\n", argv[0]);
        return 1;
    }
    uint32_t l1Size = atoi(argv[1]), l1Assoc = atoi(argv[2]), blockSize = atoi(argv[3]);
    uint32_t vcBlocks = atoi(argv[4]), l2Size = atoi(argv[5]), l2Assoc = atoi(argv[6]);
    const char* traceFile = argv[7];

    Cache* l2 = l2Size > 0 ? new Cache(l2Size, l2Assoc, blockSize, 0, nullptr) : nullptr;
    Cache l1(l1Size, l1Assoc, blockSize, vcBlocks, l2);

    FILE* fp = fopen(traceFile, "r");
    if (!fp) { fprintf(stderr, "Error: Unable to open file %s\n", traceFile); return 1; }
    char rw; unsigned int addr;
    while (fscanf(fp, " %c %x", &rw, &addr) == 2) l1.access(rw, addr);
    fclose(fp);

    printf("===== Simulator configuration =====\n");
    printf("  L1_SIZE:\t\t%u\n  L1_ASSOC:\t\t%u\n  L1_BLOCKSIZE:\t\t%u\n  VC_NUM_BLOCKS:\t%u\n", l1Size, l1Assoc, blockSize, vcBlocks);
    printf("  L2_SIZE:\t\t%u\n  L2_ASSOC:\t\t%u\n  trace_file:\t\t%s\n", l2Size, l2Assoc, traceFile);
    l1.printContents("L1", false);
    if (vcBlocks > 0) l1.printContents("VC", true);
    if (l2) l2->printContents("L2", false);

    // ---- statistics
    double accesses = (double)(l1.reads + l1.writes);
    double srr = l1.swapRequests / accesses;
    double mrL1VC = (double)(l1.readMisses + l1.writeMisses - l1.swaps) / accesses;
    uint64_t l2Reads = l2 ? l2->reads : 0, l2ReadMisses = l2 ? l2->readMisses : 0;
    uint64_t l2Writes = l2 ? l2->writes : 0, l2WriteMisses = l2 ? l2->writeMisses : 0;
    uint64_t l2Writebacks = l2 ? l2->writebacks : 0;
    double mrL2 = l2Reads ? (double)l2ReadMisses / l2Reads : 0.0;
    uint64_t traffic = l2 ? l2ReadMisses + l2WriteMisses + l2Writebacks
                          : l1.readMisses + l1.writeMisses - l1.swaps + l1.writebacks;

    // ---- timing, energy and area from CACTI (a VC that CACTI cannot model uses HT = 0.2 ns)
    float ht1 = 0, e1 = 0, a1 = 0, ht2 = 0, e2 = 0, a2 = 0, htvc = 0, evc = 0, avc = 0;
    get_cacti_results(l1Size, blockSize, l1Assoc, &ht1, &e1, &a1);
    if (l2) get_cacti_results(l2Size, blockSize, l2Assoc, &ht2, &e2, &a2);
    if (vcBlocks > 0 && get_cacti_results(vcBlocks * blockSize, blockSize, vcBlocks, &htvc, &evc, &avc) > 0) {
        htvc = 0.2f; evc = 0; avc = 0;
    }
    // The reference simulator does this arithmetic in 32-bit floats (rates, AAT, energy and total time); the same
    // operation order is used here so that EDP matches the reference outputs digit for digit.
    const float memEnergy = 0.05f;                                   // nJ per block transfer
    float fAcc = (float)accesses;
    float missPenalty = 20.0f + (float)blockSize / 16.0f;            // 20 ns + block / (16 GB/s)
    float fSrr = (float)l1.swapRequests / fAcc;
    float fMr = (float)(l1.readMisses + l1.writeMisses - l1.swaps) / fAcc;
    float aat = ht1 + fSrr * htvc;
    float energy = fAcc * e1;
    energy = energy + (float)(l1.readMisses + l1.writeMisses) * e1;
    energy = energy + (float)(2 * l1.swapRequests) * evc;
    if (l2) {
        float fMr2 = l2ReadMisses ? (float)l2ReadMisses / (float)l2Reads : 0.0f;
        aat = aat + fMr * (ht2 + fMr2 * missPenalty);
        energy = energy + (float)(l2Reads + l2Writes) * e2;
        energy = energy + (float)(l2ReadMisses + l2WriteMisses) * e2;
        energy = energy + (float)(l2ReadMisses + l2WriteMisses + l2Writebacks) * memEnergy;
    } else {
        aat = aat + fMr * missPenalty;
        energy = energy + (float)(l1.readMisses + l1.writeMisses - l1.swaps + l1.writebacks) * memEnergy;
    }
    float totalTime = aat * fAcc;
    double edp = (double)energy * (double)totalTime;
    float area = a1 + a2 + avc;

    printf("\n===== Simulation results (raw) =====\n");
    printf("  a. number of L1 reads:\t\t\t\t%llu\n", (unsigned long long)l1.reads);
    printf("  b. number of L1 read misses:\t\t\t\t%llu\n", (unsigned long long)l1.readMisses);
    printf("  c. number of L1 writes:\t\t\t\t%llu\n", (unsigned long long)l1.writes);
    printf("  d. number of L1 write misses:\t\t\t\t%llu\n", (unsigned long long)l1.writeMisses);
    printf("  e. number of swap requests:\t\t\t\t%llu\n", (unsigned long long)l1.swapRequests);
    printf("  f. swap request rate:\t\t\t\t\t%.4f\n", srr);
    printf("  g. number of swaps:\t\t\t\t\t%llu\n", (unsigned long long)l1.swaps);
    printf("  h. combined L1+VC miss rate:\t\t\t\t%.4f\n", mrL1VC);
    printf("  i. number writebacks from L1/VC:\t\t\t%llu\n", (unsigned long long)l1.writebacks);
    printf("  j. number of L2 reads:\t\t\t\t%llu\n", (unsigned long long)l2Reads);
    printf("  k. number of L2 read misses:\t\t\t\t%llu\n", (unsigned long long)l2ReadMisses);
    printf("  l. number of L2 writes:\t\t\t\t%llu\n", (unsigned long long)l2Writes);
    printf("  m. number of L2 write misses:\t\t\t\t%llu\n", (unsigned long long)l2WriteMisses);
    printf("  n. L2 miss rate:\t\t\t\t\t%.4f\n", mrL2);
    printf("  o. number of writebacks from L2:\t\t\t%llu\n", (unsigned long long)l2Writebacks);
    printf("  p. total memory traffic:\t\t\t\t%llu\n", (unsigned long long)traffic);
    printf("\n===== Simulation results (performance) =====\n");
    printf("  1. average access time:\t\t\t%.4f\n", aat);
    printf("  2. energy-delay product:\t\t\t%.4f\n", edp);
    printf("  3. total area:\t\t\t\t%.4f\n", area);
    return 0;
}

#include "simulator.h"

void Cache::handleProcRequest(ProcRequest request, unsigned long long address) {

    unsigned long long blockAddress = address >> CACHE_OFFSET_BITS;

    CacheBlockState currentState = getState(blockAddress);
    bool hit = (currentState != CacheBlockState::Invalid);

    if (request == ProcRequest::ProcRd) {

        num_reads++;

        if (!hit) {

            num_read_misses++;

            bus->sendMessage(BusRequest::BusRd, blockAddress, id);
            
            bool sharingIsCaring = bus->getSharedLine();

            CacheBlockState newState;
            if (protocol == Protocol::MESI && !sharingIsCaring)

                newState = CacheBlockState::Exclusive;
            else
                newState = CacheBlockState::Shared;
            

            CacheBlock evicted_block = insertCacheBlock(blockAddress, newState);
            if (evicted_block.state == CacheBlockState::Modified) {

                bus->sendMessage(BusRequest::Flush, evicted_block.tag, id);
                num_writebacks++;
            }

        } else {

            moveToMRU(blockAddress);
        }

    } else if (request == ProcRequest::ProcWr) {

        num_writes++;

        if (!hit ){     

            num_write_misses++;

            bus->sendMessage(BusRequest::BusRdX, blockAddress, id);
              
            CacheBlock evicted_block = insertCacheBlock(blockAddress, CacheBlockState::Modified);
            if (evicted_block.state == CacheBlockState::Modified) {

                //bus->sendMessage(BusRequest::Flush, evicted_block.tag, id);
                bus->sendMessage(BusRequest::Flush, evicted_block.tag<<SET_BITS||(blockAddress&&(SET_BITS)-1), id);
                num_writebacks++;
            }
            
            
        } else if (currentState == CacheBlockState::Shared) {
            
            bus->sendMessage(BusRequest::BusRdX, blockAddress, id);
            setState(blockAddress, CacheBlockState::Modified);
            moveToMRU(blockAddress);

        } else if (currentState == CacheBlockState::Exclusive && protocol == Protocol::MESI) {
            
            setState(blockAddress, CacheBlockState::Modified);
            moveToMRU(blockAddress);

        } else if (currentState == CacheBlockState::Modified) {

            moveToMRU(blockAddress);
        }
    }
}

void Cache::handleBusRequest(BusRequest request, unsigned long long blockAddress) {

    CacheBlockState currentState = getState(blockAddress);

    switch (request) {

        case BusRequest::BusRd:
                                if (currentState == CacheBlockState::Modified) {

                                    setState(blockAddress, CacheBlockState::Shared);
                                    bus->setSharedLine();
                                    bus->sendMessage(BusRequest::Flush, blockAddress, id);
                                    num_writebacks++;

                                } else if (currentState == CacheBlockState::Exclusive && protocol == Protocol::MESI) {

                                    setState(blockAddress, CacheBlockState::Shared);
                                    bus->setSharedLine();

                                } else if (currentState == CacheBlockState::Shared) {

                                    bus->setSharedLine();
                                }
                                
                                break;

        case BusRequest::BusRdX:
                                if (currentState != CacheBlockState::Invalid) {

                                    setState(blockAddress, CacheBlockState::Invalid);
                                    num_invalidations++;

                                    if (currentState == CacheBlockState::Modified) {

                                        bus->sendMessage(BusRequest::Flush, blockAddress, id);
                                        num_writebacks++;

                                    } else if(protocol == Protocol::MESI && currentState == CacheBlockState::Exclusive){

                                            //bus->sendMessage(BusRequest::Flush, blockAddress, id);
                                            //num_writebacks++;

                                    } else if(protocol == Protocol::MESI && currentState == CacheBlockState::Shared){

                                            // No flush needed for Shared in MESI
                                    } else if(protocol == Protocol::MSI && currentState == CacheBlockState::Shared){

                                            // No flush needed for Shared in MSI
                                    }

                                }
                                
                                break;

        default:
                break;

    }

}

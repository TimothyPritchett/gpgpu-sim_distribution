#ifndef GPU_RFC_H
#define GPU_RFC_H

/*******************************************************************************
 *
 *	Includes
 *
 ******************************************************************************/

/* Standard Includes */
#include <stdio.h>
#include <stdlib.h>

/* GPGPU-sim Custom Includes */
#include "../abstract_hardware_model.h"



/*******************************************************************************
 *
 *	Constants
 *
 ******************************************************************************/

#define RFC_DEBUG_PRINTS (true)

/*******************************************************************************
 *
 *	Custom Classes and Types
 *
 ******************************************************************************/
typedef std::pair<unsigned,const warp_inst_t*> RFCRegEntry;
typedef std::list<RFCRegEntry> RFCRegList;

// Need to put stats in structs since we can add across them to get totals for all SMs
struct RFCStats{
  long long num_hits;
  long long num_read_hits;
  long long num_write_hits;
  long long num_misses;
  long long num_read_misses;
  long long num_write_misses;
  long long num_evictions;

  RFCStats(){
    clear();
  }

  void clear(){
    num_hits          = 0LL;
    num_read_hits     = 0LL;
    num_write_hits    = 0LL;
    num_misses        = 0LL;
    num_read_misses   = 0LL;
    num_write_misses  = 0LL;
    num_evictions     = 0LL;
  }

  RFCStats &operator+=(const RFCStats &rfc_stats){
    ///
    /// Overloading += operator to easily accumulate stats
    ///
    num_hits          += rfc_stats.num_hits;
    num_read_hits     += rfc_stats.num_read_hits;
    num_write_hits    += rfc_stats.num_write_hits;
    num_misses        += rfc_stats.num_misses;
    num_read_misses   += rfc_stats.num_read_misses;
    num_write_misses  += rfc_stats.num_write_misses;
    num_evictions     += rfc_stats.num_evictions;
    return *this;
  }

  RFCStats operator+(const RFCStats &rfc_stats){
    ///
    /// Overloading + operator to easily accumulate stats
    ///
    struct RFCStats result;
    result.num_hits          = num_hits         + rfc_stats.num_hits;
    result.num_read_hits     = num_read_hits    + rfc_stats.num_read_hits;
    result.num_write_hits    = num_write_hits   + rfc_stats.num_write_hits;
    result.num_misses        = num_misses       + rfc_stats.num_misses;
    result.num_read_misses   = num_read_misses  + rfc_stats.num_read_misses;
    result.num_write_misses  = num_write_misses + rfc_stats.num_write_misses;
    result.num_evictions     = num_evictions    + rfc_stats.num_evictions;
    return result;
  }

  void print_stats(FILE *fout){
    fprintf(fout, "\tTotal RFC Accesses     = %lld\n", (num_hits + num_misses));
    fprintf(fout, "\tTotal RFC Misses       = %lld\n", num_misses);
    fprintf(fout, "\tTotal RFC Read Misses  = %lld\n", num_read_misses);
    fprintf(fout, "\tTotal RFC Write Misses = %lld\n", num_write_misses);
    fprintf(fout, "\tTotal RFC Evictions    = %lld\n", num_evictions);
  }
};

class RegisterFileCache {
  // Define any/all private members and methods

  // Define any/all protected members and methods
  protected:
  unsigned m_num_reg_slots;
  // Keep tag and 'instruction info together' in the array
  std::map<unsigned,RFCRegList> m_internal_array; // Tag array is organized <warp id, array of reg tags/slots>
  
  // Stats field
  struct RFCStats m_stats;
  
  // Define any/all public members and methods
  public:

  // Define the constructor
  RegisterFileCache(unsigned num_reg_slots){
    m_num_reg_slots     = num_reg_slots;
    m_stats.clear();
  }

  // Define the destructor
  ~RegisterFileCache(){
    // Clear the tag array
    m_internal_array.clear();
  }

  // Define an accessor method to get this units stats
  void get_stats(struct RFCStats &ext_stats){
    // Add this instances stats to the external 'collector'
    ext_stats += m_stats;
  }

  // Method to check if a register currently exists in the cache
  // Does not update collected stats
  bool exists(unsigned warp_id, unsigned register_number){
    // Declare local variables
    std::map<unsigned,RFCRegList>::iterator  tmp_map_iter;
    RFCRegList::iterator                     tmp_list_iter;

    // Debug print
    if(RFC_DEBUG_PRINTS){
      printf("RFC Class: Exisits Method invoked\n");
    }

    // Search the map
    tmp_map_iter = m_internal_array.find(warp_id);
    if(m_internal_array.end() == tmp_map_iter){// No entries for this warp
      return false;
    }else{// This warp has a list -> search it for the reg number
      RFCRegList &tmp_warp_list = tmp_map_iter->second;
      for(tmp_list_iter = tmp_warp_list.begin(); tmp_list_iter != tmp_warp_list.end(); tmp_list_iter++){
        RFCRegEntry &tmp_pair = *tmp_list_iter;
        if(register_number == tmp_pair.first){// Found an entry for the register
          return true;
        }
      }
      
      // Can only get here if there was no match in the warp's list
      return false;
    }
  }

  // Method to handle read based lookups
  // Updates stats
  bool lookup_read(unsigned warp_id, unsigned register_number){

    // Debug print
    if(RFC_DEBUG_PRINTS){
      printf("RFC Class: Lookup Read Method invoked\n");
    }

    if(exists(warp_id, register_number)){// Found it!
      // Update the stats
      m_stats.num_hits      += 1LL;
      m_stats.num_read_hits += 1LL;

      // Done
      return true;
    }else{// Not here!
      // Update the stats
      m_stats.num_misses      += 1LL;
      m_stats.num_read_misses += 1LL;

      // Done
      return false;
    }
  }

  // Method to handle read based lookups
  // Updates stats
  bool lookup_write(unsigned warp_id, unsigned register_number){
    // Debug print
    if(RFC_DEBUG_PRINTS){
      printf("RFC Class: Lookup Write Method invoked\n");
    }
   
    if(exists(warp_id, register_number)){// Found it!
      // Update the stats
      m_stats.num_hits      += 1LL;
      m_stats.num_write_hits += 1LL;

      // Done
      return true;
    }else{// Not here!
      // Update the stats
      m_stats.num_misses        += 1LL;
      m_stats.num_write_misses  += 1LL;

      // Done
      return false;
    }
  }

  // Method to check if an eviction would be needed prior to insertion
  // Assumes that the item is not present (use exist or lookup first)
  // Returns True if eviction is required
  // Populates the supplied evictee ptr with the reg number (if needed)
  // Populates the supplied instruciton ptr with the reg's most recent instruction (if needed)
  bool check_for_eviction(unsigned warp_id, unsigned register_number, unsigned *evictee_reg, const warp_inst_t **evictee_inst){
    // Declare local variables
    std::map<unsigned,RFCRegList>::iterator  tmp_map_iter;
    RFCRegList::iterator                     tmp_list_iter;

    // Debug print
    if(RFC_DEBUG_PRINTS){
      printf("RFC Class: Check for Eviction Method invoked\n");
      pritnf("")
    }


    // Get the list for the warp if there is one
    tmp_map_iter = m_internal_array.find(warp_id);
    if(m_internal_array.end() == tmp_map_iter){// No current list for this warp
      // Plenty of space for this warp's new reg
      if(RFC_DEBUG_PRINTS){
        printf("RFC Class: Check for Eviction: Plenty of space\n");
      }
      return false;
    }else{// This warp has a list -> need to check for space
      RFCRegList &tmp_warp_list = tmp_map_iter->second;
      if(m_num_reg_slots > tmp_warp_list.size()){// Not all of the slots are used
        // Should be enough space for this new reg
        if(RFC_DEBUG_PRINTS){
          printf("RFC Class: Check for Eviction: Enough space\n");
        }
        return false;
      }else{// All spaces are currently in use -> need to evict one
        if(RFC_DEBUG_PRINTS){
          printf("RFC Class: Check for Eviction: Need to Evict\n");
        }

        // We are only following FIFO policy currently (push front, pop back)
        RFCRegEntry &tmp_evictee = *(tmp_warp_list.end());
        if(RFC_DEBUG_PRINTS){
          printf("RFC Class: Check for Eviction: Updating Evictee Info\n");
        }
        // Update the value at the supplied pointer for the register number
        *evictee_reg = tmp_evictee.first;
        // Update the value at the supplied pointer for the instruction
        *evictee_inst = tmp_evictee.second;
        // Done with populating evictee info items
        return true;
      }

    }
  }

  // Method to handle the insertion of a new register to the cache
  // Will remove 'evictee' if needed but does not handle write-back
  // Note: will pull warp id value from 'inst'
  void insert(unsigned register_number, const warp_inst_t *inst_ptr){
    // Declare local variables
    std::map<unsigned,RFCRegList>::iterator tmp_map_iter;
    RFCRegList::iterator                    tmp_list_iter;
    unsigned                                tmp_warp_id = inst_ptr->warp_id();

    // Debug print
    if(RFC_DEBUG_PRINTS){
      printf("RFC Class: Insert Method invoked\n");
    }

    if(0 == m_num_reg_slots){// Zero entry RFC -> never try to insert
      //std::cout >> "Error: RFC was initialized to have 0 entries per warp\n";
      return;
    }


    // Get the list for the warp if there is one
    tmp_map_iter = m_internal_array.find(tmp_warp_id);
    if(m_internal_array.end() == tmp_map_iter){// No current list for this warp
      // Need to add one
      RFCRegList tmp_new_list;
      m_internal_array.insert(std::make_pair(tmp_warp_id,tmp_new_list));

      // Fall through after creation of the list in order minimize duplicate code
    }

    // This warp has to have a list now -> Retrieve it
    RFCRegList &tmp_warp_list = m_internal_array[tmp_warp_id];

    
    // Handle eviction if needed
    if(m_num_reg_slots <= tmp_warp_list.size()){// List is full -> need to evict one
      // We are only following FIFO policy currently (push front, pop back)
      tmp_warp_list.pop_back();

      // Update the stats
      m_stats.num_evictions += 1LL;
    }
    
    // Handle insertion of the new reg
    // FIFO queue can have duplicates of same reg 
    // (but front to end scan for matches prevents any issues)
    // Handling position update/replacement of existing item is LRW
    tmp_warp_list.push_front(RFCRegEntry(register_number,inst_ptr));
    
    // Done
  }

  // Note Regifile Caches don't need to be flushed as long all warps have slots
  // Will need to flush if RFC size is shrunk to a subset of possible warps

  // Method to print the stats to a file
  // TODO
};

/*******************************************************************************
 *
 *	Global Variables
 *
 ******************************************************************************/

/*******************************************************************************
 *
 *	Exported variables
 *
 ******************************************************************************/

/*******************************************************************************
 *
 *	Exported Functions
 *
 ******************************************************************************/



#endif

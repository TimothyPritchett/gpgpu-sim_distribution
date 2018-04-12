###############################################################################
###############################################################################
#
# Makefile for automating the execution of the BFS benchmark
#
###############################################################################
###############################################################################

###############################################################################
# General Variables
###############################################################################
BENCHMARK_NAME				:= BFS
BENCHMARK_CMD 				:= ./$(BENCHMARK_NAME) graph4096.txt
RESULTS_BASE_DIR			:= ./Runtime_Results
RESULTS_FILE_BASE			:= runtime_log.txt
RFC_RESULTS_FILE_BASE	:= rfc_results.txt
ACTIVE_CONFIG_FILE		:= gpgpusim.config

RESULTS_REDUCTION_SCRIPT	:=

###############################################################################
# Configuration Related Variables
###############################################################################
RFC_SIZE := 
FIFO_CONFIG_PATTERN	= fifo_$(RFC_SIZE)_gpgpusim.config
FIFO_RESULTS_DIR		:= $(RESULTS_BASE_DIR)/FIFO
FIFO_RESULTS_FILE		= $(FIFO_RESULTS_DIR)/fifo_$(RFC_SIZE)_$(RESULTS_FILE_BASE)

LRW_CONFIG_PATTERN	= lrw_$(RFC_SIZE)_gpgpusim.config
LRW_RESULTS_DIR			:= $(RESULTS_BASE_DIR)/LRW
LRW_RESULTS_FILE		= $(LRW_RESULTS_DIR)/lrw_$(RFC_SIZE)_$(RESULTS_FILE_BASE)

LRU_CONFIG_PATTERN	= lru_$(RFC_SIZE)_gpgpusim.config
LRU_RESULTS_DIR			:= $(RESULTS_BASE_DIR)/LRU
LRU_RESULTS_FILE		= $(LRU_RESULTS_DIR)/lru_$(RFC_SIZE)_$(RESULTS_FILE_BASE)

##############################################################################
# Designate targets that do not correspond directly to files so that they are
# run every time they are called
##############################################################################
.PHONY: help clean veryclean

###############################################################################
#
# Admin Commands
#
###############################################################################

# Help target will also be the default target (first one) if none are specified
help:
	@echo "**********************************************************************"
	@echo "Local Makefile Help                                                   "
	@echo " Admin Rules:                                                         "
	@echo "   - clean:     Removes any temporary files (including logs)          "
	@echo "                                                                      "
	@echo "   - veryclean: Clean + Removes CU objects & Prior runtime results    "
	@echo "                                                                      "
	@echo "                                                                      "
	@echo "**********************************************************************"

# Remove temporary files for common cleaning
clean:
	@echo "Removing temporary files"
	@rm -f *.bak
	@rm -f *.*~
	@rm -f *.log
	@rm -f *stats.txt
	@rm -f *_log.txt

# Remove all generated files
veryclean:
	@$(MAKE) --no-print-directory clean
	@rm -f _cuobj*
	@rm -rf $(RESULTS_BASE_DIR)/*

###############################################################################
#
# Benchmark Run/Execution Commands
#
###############################################################################
RFC_SIZES := 1 2 3 4 5 6 7 8

run_fifo_sweep:
	@echo "Running FIFO style RFC size sweep for $(BENCHMARK_NAME)
	@$(MAKE) --no-print-directory $(addprefix run_benchmark_fifo_,$(RFC_SIZES))
	@echo "$(BENCHMARK_NAME) FIFO style RFC size sweep complete"

###############################################################################
#
# Pattern Rule targets
#
###############################################################################

# Pattern rule and base rule for running FIFO based RFC benchmark experiments
run_benchmark_fifo_%:
	@$(MAKE) --no-print-directory RFC_SIZE=$* run_benchmark_fifo

run_benchmark_fifo:
	@$(MAKE) --no-print-directory update_config_fifo_$(RFC_SIZE)
	@echo "Running $(BENCHMARK_NAME) w/ $(RFC_SIZE)-slot FIFO RFC"
	@mkdir -p $(FIFO_RESULTS_DIR)
	@$(BENCHMARK_CMD) > $(FIFO_RESULTS_FILE)
	@$(MAKE) --no-print-directory $(patsubst %_$(RESULTS_FILE_BASE),%_$(RFC_RESULTS_FILE_BASE),$(FIFO_RESULTS_FILE))

# Pattern rule and base rule for running LRW based RFC benchmark experiments
run_benchmark_lrw_%:
	@$(MAKE) --no-print-directory RFC_SIZE=$* run_benchmark_lrw

run_benchmark_lrw:
	@$(MAKE) --no-print-directory update_config_lrw_$(RFC_SIZE)
	@echo "Running $(BENCHMARK_NAME) w/ $(RFC_SIZE)-slot LRW RFC"
	@mkdir -p $(LRW_RESULTS_DIR)
	@$(BENCHMARK_CMD) > $(LRW_RESULTS_FILE)
	@$(MAKE) --no-print-directory $(patsubst %_$(RESULTS_FILE_BASE),%_$(RFC_RESULTS_FILE_BASE),$(LRW_RESULTS_FILE))

# Pattern rule and base rule for running LRW based RFC benchmark experiments
run_benchmark_lru_%:
	@$(MAKE) --no-print-directory RFC_SIZE=$* run_benchmark_lru

run_benchmark_lru:
	@$(MAKE) --no-print-directory update_config_lru_$(RFC_SIZE)
	@echo "Running $(BENCHMARK_NAME) w/ $(RFC_SIZE)-slot LRU RFC"
	@mkdir -p $(LRU_RESULTS_DIR)
	@$(BENCHMARK_CMD) > $(LRU_RESULTS_FILE)
	@$(MAKE) --no-print-directory $(patsubst %_$(RESULTS_FILE_BASE),%_$(RFC_RESULTS_FILE_BASE),$(LRU_RESULTS_FILE))

# Rule for updating/swapping the active config symbolic link
update_config_%:
	@echo "Updating active config to '$*_gpgpusim.config'"
	@rm -f $(ACTIVE_CONFIG_FILE)
	@ln -s $*_gpgpusim.config $(ACTIVE_CONFIG_FILE)

# Rule for extracting just the RFC's stats
%_$(RFC_RESULTS_FILE_BASE): %_$(RESULTS_FILE_BASE)
	@echo "Creating RFC focused results file '$@'"
	@grep "_rfc_" $^ > $@
	@grep " RFC " $^ >> $@

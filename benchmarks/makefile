###############################################################################
###############################################################################
#
# Makefile for automating the execution of the all benchmarks
#
###############################################################################
###############################################################################

###############################################################################
# General Variables
###############################################################################
# List of benchmarks (by folder) to run
BENCHMARKS := bfs bfs2 hotsp wp

##############################################################################
# Designate targets that do not correspond directly to files so that they are
# run every time they are called
##############################################################################
.PHONY: help clean veryclean
.PHONY: run_fifo_sweep

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
	@echo " Benchmark Runtime Rules:                                             "
	@echo "   - run_fifo_sweep: Runs the full size sweep of FIFO style RFC runs  "
	@echo "                       for all benchmarks                             "
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
	@$(MAKE) --no-print-directory $(addprefix clean_,$(BENCHMARKS))

# Pattern Rule for cleaning benchmark directories
clean_%:
	@$(MAKE) --no-print-directory -C $* clean

# Remove all generated files
veryclean:
	@$(MAKE) --no-print-directory clean
	@rm -f _cuobj*
	@rm -rf $(RESULTS_BASE_DIR)/*
	@$(MAKE) --no-print-directory $(addprefix veryclean_,$(BENCHMARKS))

# Pattern Rule for 'very clean'ing benchmark directories
veryclean_%:
	@$(MAKE) --no-print-directory -C $* veryclean

###############################################################################
#
# Benchmark Run/Execution Commands
#
###############################################################################

# Run the Full Size Sweep for the FIFO style RFC runs of each benchmark
run_fifo_sweep:
	@echo "Running the size sweeps for FIFO style RFC runs for all benchmarks"
	@$(MAKE) --no-print-directory $(addprefix run_fifo_sweep_,$(BENCHMARKS))
	@echo "All benchmarks have completed their FIFO style RFC sweep runs."

run_fifo_sweep_%:
	@$(MAKE) --no-print-directory -C $* run_fifo_sweep

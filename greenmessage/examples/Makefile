
###################
# test the examples
###################

ALL_EXAMPLES = $(wildcard */expected_output.txt)

test: $(ALL_EXAMPLES)

%/expected_output.txt : FORCE_RUN
	@echo ""
	@echo "####################################"
	@echo "### $(@D)"
	@rm -f $(@D)/system
	@$(MAKE) -C $(@D)
	@$(@D)/system | diff - $(@D)/expected_output.txt

FORCE_RUN:;



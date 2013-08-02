MODULES = generic mixed simple simple_speed_test platform

.PHONY: dummy modules modules-clean modules-install

all: modules

clean: modules-clean

install: modules-install

modules: $(patsubst %, _module_%, $(MODULES))
$(patsubst %, _module_%, $(MODULES)) : dummy
	$(MAKE) -C $(patsubst _module_%, %, $@) all

modules-clean: $(patsubst %, _module_clean_%, $(MODULES))
$(patsubst %, _module_clean_%, $(MODULES)) : dummy
	$(MAKE) -i -C $(patsubst _module_clean_%, %, $@) clean

modules-install: $(patsubst %, _module_install_%, $(MODULES))
$(patsubst %, _module_install_%, $(MODULES)) : dummy
	$(MAKE) -C $(patsubst _module_install_%, %, $@) install

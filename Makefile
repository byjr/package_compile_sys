all:
	./mk.sh

show:
	./mk.sh -s

distclean:
	rm -rf output

dlclean:
	rm -rf dl/*

clean:
	./mk.sh -c bclean

%-remake:
	@echo $@
	./mk.sh -nc remake $(@:-remake=)

%-resync:
	@echo $@
	./mk.sh -nc resync $(@:-resync=)

%-reconfigure:
	@echo $@
	./mk.sh -nc reconfig $(@:-reconfigure=)

%-rebuild:
	@echo $@
	./mk.sh -nc rebuild $(@:-rebuild=)

%-reinstall:
	@echo $@
	./mk.sh -nc reinstall $(@:-reinstall=)

%-sync:
	@echo $@
	./mk.sh -nc sync $(@:-sync=)

%-configure:
	@echo $@
	./mk.sh -nc config $(@:-configure=)

%-build:
	@echo $@
	./mk.sh -nc build $(@:-build=)

%-install:
	@echo $@
	./mk.sh -nc install $(@:-install=)

%-clean:
	@echo $@
	./mk.sh -nc clean $(@:-clean=)

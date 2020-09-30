TerminalLog=
ifeq ($(V),s)
	TerminalLog='--vbs'
endif

all:
	./mk.sh $(TerminalLog)

distclean:
	./mk.sh -c distclean $(TerminalLog)

clean:
	./mk.sh -c bclean $(TerminalLog)

hclean:
	./mk.sh -c hclean $(TerminalLog)
	
oclean:
	./mk.sh -c oclean $(TerminalLog)
	
%-remake:
	./mk.sh -nc remake $(@:-remake=) $(TerminalLog)

%-resync:
	./mk.sh -nc remake $(@:-resync=) $(TerminalLog)

%-reconfig:
	./mk.sh -nc reconfig $(@:-reconfig=) $(TerminalLog)

%-rebuild:
	./mk.sh -nc rebuild $(@:-rebuild=) $(TerminalLog)

%-reinstall:
	./mk.sh -nc reinstall $(@:-reinstall=) $(TerminalLog)

%-sync:
	./mk.sh -nc sync $(@:-sync=) $(TerminalLog)

%-configure:
	./mk.sh -nc config $(@:-configure=) $(TerminalLog)

%-build:
	./mk.sh -nc build $(@:-build=) $(TerminalLog)

%-install:
	./mk.sh -nc install $(@:-install=) $(TerminalLog)

%-clean:
	./mk.sh -nc clean $(@:-clean=) $(TerminalLog)

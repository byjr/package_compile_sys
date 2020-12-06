TerminalLog=
ifeq ($(V),1)
	TerminalLog :='--vbs'
endif

NeedCompileDeps :='--ndep'
ifeq ($(D),1)
	NeedCompileDeps :=
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
	./mk.sh $(NeedCompileDeps) -c remake $(@:-remake=) $(TerminalLog)
			 
%-resync:    
	./mk.sh $(NeedCompileDeps) -c remake $(@:-resync=) $(TerminalLog)
			  
%-reconfig:  
	./mk.sh $(NeedCompileDeps) -c reconfig $(@:-reconfig=) $(TerminalLog)
			 
%-rebuild:  
	./mk.sh $(NeedCompileDeps) -c rebuild $(@:-rebuild=) $(TerminalLog)
			  
%-reinstall: 
	./mk.sh $(NeedCompileDeps) -c reinstall $(@:-reinstall=) $(TerminalLog)
			 
%-sync:       
	./mk.sh $(NeedCompileDeps) -c sync $(@:-sync=) $(TerminalLog)
			 
%-configure: 
	./mk.sh $(NeedCompileDeps) -c config $(@:-configure=) $(TerminalLog)
			  
%-build:      
	./mk.sh $(NeedCompileDeps) -c build $(@:-build=) $(TerminalLog)
			 
%-install:   
	./mk.sh $(NeedCompileDeps) -c install $(@:-install=) $(TerminalLog)
			 
%-clean:     
	./mk.sh $(NeedCompileDeps) -c clean $(@:-clean=) $(TerminalLog)
	
help:
	@./mk.sh --help

target=main.exe
SOURCES=main.c


$(TARGET): $(SOURCES) 
	wcl -bt=dps -lr -d2 -fe=$@ $(SOURCES)

all: $(TARGET)

clean: .SYMBOLIC
	del *.obj
	del *.exe
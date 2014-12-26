CC		= g++
CFLAGS	= -g -std=c++11
OBJECTS	= TrieTree.o SourceCode.o SymbolParser.o plc.o Automaton.o SymbolManager.o Assembler.o
LIBS	= 

all: plc
plc: $(OBJECTS) 
	$(CC) -o plc $(OBJECTS) $(LIBS)

plc.o: Automaton.h Assembler.h SyntaxError.h SourceCode.h SymbolParser.h plc.cpp
	$(CC) $(CFLAGS) -c plc.cpp
Automaton.o: Automaton.h Automaton.cpp parser/rules.h
	$(CC) $(CFLAGS) -c Automaton.cpp
Assembler.o: Assembler.h Assembler.cpp
	$(CC) $(CFLAGS) -c Assembler.cpp
TrieTree.o: TrieTree.h TrieTree.cpp
	$(CC) $(CFLAGS) -c TrieTree.cpp
SourceCode.o: SourceCode.h SourceCode.cpp
	$(CC) $(CFLAGS) -c SourceCode.cpp
SymbolManager.o: SymbolManager.h SymbolManager.cpp
	$(CC) $(CFLAGS) -c SymbolManager.cpp
SymbolParser.o: SyntaxError.h TrieTree.h SourceCode.h Keyword.h SymbolParser.h SymbolParser.cpp
	$(CC) $(CFLAGS) -c SymbolParser.cpp

clean:
	rm -f plc $(OBJECTS)

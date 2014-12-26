PLC
===

PLC is a LALR-based [PL/0](http://en.wikipedia.org/wiki/PL/0) compiler assembling code for a special stack machine. Under the directory machine/, a simulator is provided for running the assembly.
This project is made for educational purposes only, stability of which is not guaranteed. However you may feel free to email me or fill in a bug report if any problem encountered.

Build Instruction
-----------------
* To build PLC, execute `make`. (LALR parser must be built and executed at least once in order to generate `parser/rule.h`)
* To build LALR parser, execute `make` in `parser/` directory.
* To build the unit test for LALR parser, execute `make test` in `parser/` directory.
* To build machine simulator, execute `make` in `machine/` directory.

Modules Description
-------------------
* `machine/machine.cpp`: A simulator for running the assembly designed for a stack machine.
* `parser/lalrparser.cpp`: A LALR parser whose rules are defined in `lalrparser.h` and output are written to `rules.h`.
* `parser/test.cpp`: A test for LALR parser that tries to reduce a simple PL/0 program.
* `Assembler.cpp`: Define semantic actions and assembly code.
* `Automaton.cpp`: Recognize input streams of symbols and execute semantic actions.
* `SourceCode.cpp`: Read source code and provide a simple buffering mechanism.
* `SymbolManager.cpp`: Store constants, variables and procedures in layered structure.
* `SymbolParser.cpp`: A lexical analyzer that produces symbol streams.
* `TrieTree.cpp`: A trie tree implementation that accelerates keyword and identifier locating.
* `plc.cpp`: The main entrance for PLC. Assemblied code are written to output.asm for default.

License
-------
This project is licensed under MIT License. Refer to LICENSE file for further information.
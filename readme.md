Cheshire
========

Why Cheshire?
-------------
Cheshire is a programming language designed by Michael Goulet, as a programming project to learn more about the design of compilers in general.
It is statically-typed and multi-paradigmatic (Object-Oriented and Imperative).

It is designed in three phases:
> Lexer/Parser

> Static Analysis (Type analysis, Scope bindings, etc.)

> Code Generation (Emitting to LLVM bytecode -- more targets to be added later)

How To Use
----------
To build the project, you may use the Makefile provided. There are a few different targets in the makefile. Other than "all", "build" and "clean", there are:

cleangen -- Removes generated Bison and Lex files.

generate -- Generate Bison and Lex files from "CheshireLexer.lex" and "CheshireParser.y".

todos -- Print out any "todo" or "fixme" comments in the files within the project.

Lexer/Parser
------------
Lexical analysis is done by an automatically generated scanner from Flex, defined in the file "CheshireLexer.lex". The parser is subsequently defined in the file "CheshireParser.y". 

More information to be added later.
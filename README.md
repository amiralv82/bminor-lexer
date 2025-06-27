# Bminor Lexer

A **stand-alone lexical analyzer** for the *Bminor* teaching language, implemented in clean, portable **C99**.

> Want to see how real lexers handle keywords, operators, comments, escape sequences, and symbol tables—without the ceremony of Flex or LLVM? This repo is for you.

---

## Features

| Category | Details |
|----------|---------|
| **Token support** | Identifiers, keywords, integers, strings (with `\n`, `\t`, `\"`, `\\`), operators, delimiters |
| **Comment handling** | Single-line `//` and block `/* … */` with unterminated-comment detection |
| **Accurate positions** | Tracks **line & column** numbers for precise error messages |
| **Symbol table** | Assigns unique integer codes to identifiers (starting at 100) and re-uses them |
| **Pretty output** | Prints a three-column token table: `Lexeme | Token Type | Token Value` |
| **No deps** | Pure C standard library — compiles with GCC, Clang, or MSVC |

---

## Build

```bash
# Clone
git clone https://github.com/<your-user>/bminor-lexer.git
cd bminor-lexer

# Compile
gcc -std=c99 -Wall -Wextra -o compiler lexer.c
```
(The executable name follows your usage note: ./compiler.)

---

Usage
```bash
./compiler input.txt
```
Sample output
```
Token               	Token Type     	Token Value
------------------------------------------------------------
print               	keywords       	print
"Hello, World!\n"   	STRING         	"Hello, World!\n"
;                   	delimiters     	;
```
Redirect the output to a file if you want to feed the token stream to later compiler stages.

---

Project layout
```
.
├── lexer.c            # The whole scanner
├── samples/           # Example Bminor programs
└── README.md
```

---

Limitations & roadmap

•	No floating-point or character literals yet

•	Hex / octal / binary integers not recognized

•	Only basic escape sequences (\n, \t, \", \\)

•	Not yet integrated with a parser — tokens only

Future ideas: add char literals, support floating numbers, integrate with the Bminor → C translator for a full compiler pipeline.

---

Acknowledgements

Inspired by the Bminor language used in University of Wisconsin–Madison compiler courses.
Escape-sequence logic follows the C99 specification.

---

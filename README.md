# 编译
```
lex Lexer.l
bison Parser.ypp -d
g++ AST.cpp Lexer.cpp Parser.tab.cpp -o output -ll
```
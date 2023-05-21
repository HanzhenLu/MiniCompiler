# 环境
* Ubuntu --20.04
* flex --2.6.4
* bison --3.5.1
# 编译
```
lex Lexer.l
bison Parser.ypp -d
g++ AST.cpp Lexer.cpp Parser.tab.cpp -o output -ll
```
# 运行
需要`input.txt`文件作为输入
# 环境
* Ubuntu --20.04
* flex --2.6.4
* bison --3.5.1
# 编译
```
lex Lexer.l
bison Parser.ypp -d
g++ AST.cpp Lexer.cpp Parser.tab.cpp -o output
```
# 运行
需要`input.txt`文件作为输入, 运行`a.out`
若要生成可视图，将`AST.cpp`中的`VISIBLE`设为$1$，在运行`a.out`后，执行如下指令即可生成可视图`AST.png`
```
dot -Tpng AST.dot -o AST.png
```

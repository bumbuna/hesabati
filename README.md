# HESABATI
Hesabati (swahili word for 'maths') is a library for evaluating mathematical expression.

## HOW IT WORKS
Hesabati is implemented as a virtual machine internally. A mathematical expression is first parsed into an AST(abstract syntax tree) then an interpreter kicks in and walks down the AST performing the necessary operations on the AST nodes.

## BUILDING
Hesabati uses CMAKE as its build system

### Requirements 
- cmake 3.0+
- any C compiler supported by cmake
- make or any other generator supported by cmake
- Linux/Unix OS

```bash
$ git clone https://github.com/devbumbuna/HESABATI.git
$ cd HESABATI
$ cmake -B build
$ cmake --build build
[optional]
$ cd build
$ sudo make install
$ ctest
```
## LICENSE
See <a href="license.txt">license</a>.

## CONTRIBUTION
All contributions are welcome.

## PROJECT PAGE
<a href="https://hesabati@devbumbuna.com"> Project Home Page</a>

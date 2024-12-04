@echo off
cmake -S build -B bin/debug -GNinja ^
	  -DCMAKE_BUILD_TYPE=Debug ^
	  -DCMAKE_CXX_COMPILER=Clang ^
	  -DCMAKE_C_COMPILER=Clang

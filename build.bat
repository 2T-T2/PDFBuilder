set DLL_NAME=PDFBuilder

@REM GCC を用いてDLLを作成
g++ -I. -c src\cpp\%DLL_NAME%.cpp -o out\%DLL_NAME%.o
g++ out\%DLL_NAME%.o -o out\%DLL_NAME%.dll -shared

@REM CSC を用いてDLLを作成
csc.exe /nologo /out:out\TPanda.%DLL_NAME%CS.dll /target:library src\cs\%DLL_NAME%.cs

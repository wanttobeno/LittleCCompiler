win_flex --yylineno --nounistd --header-file=src\scan.h -o src\scan.c src\scan.l
win_bison -d -o src\parse.c src\parse.y

rem msdev "lc.dsw" /MAKE "lc - Win32 Debug" /REBUILD

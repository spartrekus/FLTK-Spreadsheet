echo g++ $1 -I/usr/local/include -I/usr/local/include/FL/images -mwindows -DWIN32 -mno-cygwin -o sprsht sprsht.cxx -mwindows -mno-cygwin -L/usr/local/lib /usr/local/lib/libfltk.a -lole32 -luuid -lcomctl32 -lwsock32
g++ $1 -I/usr/local/include -I/usr/local/include/FL/images -mwindows -DWIN32 -mno-cygwin -o sprsht sprsht.cxx -mwindows -mno-cygwin -L/usr/local/lib /usr/local/lib/libfltk.a -lole32 -luuid -lcomctl32 -lwsock32
echo compiling finished at optimise level $1
echo strip -s sprsht.exe
strip -s sprsht.exe
echo finished strip run sprsht.exe
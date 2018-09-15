# Mmap_syscall_comparison
A performance comparison on reading a file using system calls or mapping it to memory and reading from there.

There are two main programs, one reads a file using the usual read functions, the other maps the file to virtual memory.
The files are read in chunks of bytes, first reading it 4 bytes at a time and going up all the way to the biggest power of 2 
that is smaller than the file size.

The syscall program performs an fread operation, asking the kernel to load the number of bytes wanted into a string in memory, called the "buffer", the mmap program copies the data from the mapping into the buffer using strncpy.

The file is read several times for each buffer size, in order to get a more accurate average time. All data is then output to 
another file in csv format to provide an easy way to compare the results.

The File generator program is used to create a file of the desired size to get an easy way to test the programs.

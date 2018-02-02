1. 	The finished project contains three Java files.

	> hCompress.java
		The main file for compressing. Buffer size is 1024 bytes.
	> hDecompress.java
		The main file for decompressing. Buffer size is 1024 bytes.
	> hoffmanNode.java
		The basic data structure.
		
2. 	You can use the "check.sh" file to test the application. However you may need to modify 
	the classpath in "compress.sh" and "decompress.sh" so the JVM can find the .class files.
	
	
3. 	Expected running time:
	In my own test:
		> Environment: VirtualBox + Ubuntu (4G virtual memory),host machine has 8GB memory, i7 dual-
	processor) 
		> Running time: 
			1G plain text file: about 40 seconds to compress 
								about 30 seconds to decompress
		> Have passed "diff" test.
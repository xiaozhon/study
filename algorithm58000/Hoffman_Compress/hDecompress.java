//package hoffman.compress;

import java.io.*;
import java.util.Comparator;
import java.util.PriorityQueue;

public class hDecompress {
	
	private static hoffmanNode[] hChar = new hoffmanNode[128];
	static InputStream inFile = null;
	static FileWriter outFile = null;
	private static int bufferSize = 10240;
	
	private static int i = 0;
	private static int totalChars = 0;	// the length of the contents
	static int offset = 0;

	private static int tableLength = 0;		
	private static int byteCount = 0;		// count every byte read in the file
	private static int tableSize = 0;
	private static hoffmanNode root = new hoffmanNode((char)0,0);
	
	static byte[] bytesBuffer = new byte[bufferSize];
	static char[] outChars = new char[bufferSize];
	
	static int inByteRead = 0;
	static int outCharCount = 0;

	public static void initialization() {
		// TODO Auto-generated method stub
		for(i = 0; i < 128; i++){
        	hChar[i] = new hoffmanNode((char)i, 0);		// initialize structure
        }
	}
	
	/*
	 * 	format of encoded file
	 * 		Magic number: <H F C D> 4 bytes
	 * 		Version: 				1 bytes
	 * 		table size:	 the number of items in the code table	1 bytes
	 * 		code table < char ch, int code>, the code = Integer.parseInt(hChar.code) + (codeLenth << 24);
	 * 										the first byte stores the length of the code
	 * 		content length: 	4 bytes
	 * 		encoded content: 	n bytes
	 * 		reserved bytes: 	4 bytes
	 * 
	 **/
	
	public static void rebuildCodeTable(String encodedFile) {
		// TODO Auto-generated method stub
		try {
			byte[] tmpBytes = new byte[4];
			
			inFile = new FileInputStream(encodedFile);//inFile.read(b, off, len)
			if((inByteRead = inFile.read(bytesBuffer)) != -1) {
			//	byteCount = byteCount + byteRead;
				
		// magic numbers
				if((char)bytesBuffer[0] != 'H' && (char)bytesBuffer[1] != 'F'
						&& (char)bytesBuffer[2] != 'C' && (char)bytesBuffer[3] != 'D') {
					System.out.println("wrong file! ");
					System.exit(0);
				}
				//System.out.println((char)bytesBuffer[0] + " " + (char)bytesBuffer[1] + " " 
				//		+ (char)bytesBuffer[2] + " " + (char)bytesBuffer[3]);
				// version number
				//System.out.println("version number: " + (char)bytesBuffer[4]);
				// table length
				//System.out.println("table length: " + bytesBuffer[5]);
				offset = 5;
				tableLength  = bytesBuffer[5];	// table length
				i = tableLength;
				//System.out.println("table length is: " + i);
				// store code to hChar table
				while (i > 0) {
					// char		
					char ch = (char)bytesBuffer[offset+1];
					hChar[ch].c = ch;
			
					// frequency
					tmpBytes[0] = bytesBuffer[offset+2];
					tmpBytes[1] = bytesBuffer[offset+3];
					tmpBytes[2] = bytesBuffer[offset+4];
					tmpBytes[3] = bytesBuffer[offset+5];
					hChar[ch].frequency = byte2int(tmpBytes);
					
					offset = offset + 5;
					
					i--;	
				} // while
				// total length	bytesBuffer[offset + 1~4]
				tmpBytes[0] = bytesBuffer[offset+1];
				tmpBytes[1] = bytesBuffer[offset+2];
				tmpBytes[2] = bytesBuffer[offset+3];
				tmpBytes[3] = bytesBuffer[offset+4];
				totalChars = byte2int(tmpBytes);
				
				offset = offset + 4+1;	// offset is the byte before current bytes to be processed
				
				//System.out.println("total chars is: " + totalChars);
            } // if
			
			
        } catch (IOException e1) {
            e1.printStackTrace();
        } 
		
	}
	
	public static void hDecoding(String encodedFile, String decodedFile) throws IOException {
		// TODO Auto-generated method stub
		//	for(i = 0; i < 128; i++) {
		//		if(hChar[i].frequency > 0) {
		//			System.out.println("get node: " + hChar[i].c);
		//		}
		//	}
		buildHoffmanTree();
		
		hoffmanNode currentNode = root;
		
		if(currentNode == null){
			System.out.println("No Hoffman Tree is build!");
			System.exit(0);
		}
		try {
			i = offset; //
			//System.out.println("offset of the content is: " + offset);
			
			int bitIdx = 0;
			inFile = new FileInputStream(encodedFile);
			
			outFile = new FileWriter(decodedFile);
			while((inByteRead = inFile.read(bytesBuffer)) != -1){
				
			// for the first loop, offset is located in the first byte of content
				while (i < inByteRead) {
			
					for(bitIdx = 7; bitIdx >= 0; bitIdx--) {	// for each byte
						if(((bytesBuffer[i] >> bitIdx) & 0x01) == 0) {
							currentNode = currentNode.lChild;
							//System.out.println("goes left.");
							
						} else if(((bytesBuffer[i] >> bitIdx) & 0x01) == 1) {
							currentNode = currentNode.rChild;
							//System.out.println("goes right");
							
						} 
						if(currentNode.lChild == null && currentNode.rChild == null) {
							if(outCharCount == bufferSize) {
								outFile.write(outChars);
								outCharCount = 0;
							}
							outChars[outCharCount] = currentNode.c;

							outCharCount++;
							byteCount++;
							if(byteCount == totalChars){
								break;
							}
							currentNode = root;		// a char has been checked, find the next
							
						}
						
					}
					if(byteCount == totalChars){
						break;
					}
					i++;
				}
				
				i = 0;
				
			}
			//System.out.println("-----------------------total convert bytes: " + byteCount);
			outFile.write(outChars, 0, outCharCount);
			
			//System.out.println("the last char is: " + outChars[outCharCount - 1]);
			
			outFile.close();
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}//inFile.read(b, off, len)
		
	}
	
	public static void buildHoffmanTree() {

		// in order to use priority queue, define new comparator for the queue
		Comparator<hoffmanNode> cmp = new Comparator<hoffmanNode>() {
			public int compare(hoffmanNode a, hoffmanNode b){
				return (a.frequency-b.frequency);	// less frequent node first
			}
		};
	
		PriorityQueue<hoffmanNode> nodeQueue = new PriorityQueue<hoffmanNode>(1, cmp);
		// use priorityqueue to sort the node according to their frequency.
		for(i = 0; i<128; i++){
			if(hChar[i].frequency > 0){
				//System.out.println("add char " + hChar[i].c + " to queue.");
				nodeQueue.add(hChar[i]);	// the node will be added in order
			}
		}
		//System.out.println("queue size: " + nodeQueue.size());
	
		tableSize  = nodeQueue.size();
	
		// now we can build the tree
		while(nodeQueue.size() > 1){
		 			
			hoffmanNode min1 = nodeQueue.poll();
			hoffmanNode min2 = nodeQueue.poll();
		
			hoffmanNode result = new hoffmanNode((char)0, min1.frequency+min2.frequency);
			result.lChild = min1;
			result.rChild = min2;
			nodeQueue.add(result);

		}
		root  = nodeQueue.peek();
	}
	
	public static int byte2int (byte[] b) {
		int value = 0;
		int shift = 0;
		for (int s = 0; s < 4; s++) {
			shift = (4 - 1 - s) * 8;
			value += (b[s] & 0x000000FF) << shift;
		}
		return value;
	}
	
	private static String bytesCode2string(byte[] b) {	// code table is in b[]
		// TODO Auto-generated method stub
		int codelen = b[0];
		int j = 0;
		int zeroOr1 = 0;
		String s = null;
		if(codelen > 8) {
			j = codelen - 8;	// the code in b[2]
			while (j > 0) {
				zeroOr1 = (b[2] >> (j-1)) & 0x01;
				if (zeroOr1 == 1) {	// 1
					if(j == codelen-8){
						s = "1";
					} else {
						s = s + '1';
					}
				}
				else {
					if(j == codelen-8){
						s = "0";
					} else {
						s = s + '0';
					}
				}
				j--;
			}
			codelen = 8;		// the code length in b[3]
		}
		j = codelen;
		while (j > 0) {			// the code in b[3]
			zeroOr1 = (b[3] >> (j-1)) & 0x01;
			if (zeroOr1 == 1) {	// 1
				if(j == codelen){
					s = "1";
				} else {
					s = s + '1';
				}
			}
			else {
				if(j == codelen){
					s = "0";
				} else {
					s = s + '0';
				}
			}
			j--;
		}
		return s;
	}
	
	/**
	 * @param args
	 * @throws IOException 
	 */
	public static void main(String[] args) throws IOException {
		// TODO Auto-generated method stub
		long begin = System.currentTimeMillis();
		//String encodedFile = "/opt/encoded_file1.txt";
		//String decodedFile = "/opt/decoded_file.txt";
		String encodedFile = args[0];	// while sh compress.sh is used
		String decodedFile = args[1];
		initialization();
		rebuildCodeTable(encodedFile);		// rebuild code Table
		hDecoding(encodedFile, decodedFile);
		long end = System.currentTimeMillis();
		System.out.println("Decompressing time cost: " + (end - begin) + " milli seconds");
	}

}

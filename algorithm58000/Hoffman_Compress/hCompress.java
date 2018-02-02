/*
 * Compressing use Hoffman Coding
 *
 * @author: 	Xiaoyang Zhong
 * @date:	March 22th, 2013
 **/
//package hoffman.compress;

import java.io.*;
import java.util.*;

public class hCompress {
	// each node for each possible char
	private static hoffmanNode[] hChar = new hoffmanNode[128];
	
	// the root of the hoffman tree
	private static hoffmanNode root = new hoffmanNode((char)0, 0);
	
	private static int tableSize = 0;
	private static int totalChars = 0;
	
	private static int bufferSize = 10240;
	private static int i = 0;
	
	/**
	 * compute the frequency of the chars in the file 
	 **/
	public static void fileFrequency(String fileName) throws IOException {
		File file = new File(fileName);
        Reader reader = null;
		int index = 0;
        
        for(i = 0; i < 128; i++){
        	hChar[i] = new hoffmanNode((char)i, 0);		// initialize structure
        }
        
        try{    
            char[] charBuffer = new char[bufferSize];		// read in file by chars, buffer length 1024
            int charRead = 0;		// the length of chars have been read
            
			//reader = new InputStreamReader(new FileInputStream(fileName));
            reader = new FileReader(file);
            while ((charRead = reader.read(charBuffer)) != -1) {
            	totalChars += charRead;
            	for (i = 0; i < charRead; i++) {
            		index = charBuffer[i];
            	//	hChar[index].increaseFreq(1);	// increase the frequency of the char
            		hChar[index].frequency++;
            	}
            }
			//System.out.println("total chars is: "+ totalChars);
        } catch (IOException e){
        	e.printStackTrace();
        }
		//for(i = 0; i < 128; i++)
		//{
		//	if(hChar[i].frequency > 0)
		//	System.out.println((char)i + " frequency:  " + hChar[i].frequency);
		//}
        
	}
	
	/**
	 * create Hoffman Tree
	 * 
	 **/
	
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
			//	System.out.println("add char " + hChar[i].c + " to queue.");
				nodeQueue.add(hChar[i]);	// the node will be added in order
			}
		}
		//System.out.println("queue size: " + nodeQueue.size());
		
		tableSize = nodeQueue.size();
		
		// now we can build the tree
		while(nodeQueue.size() > 1){
						
			hoffmanNode min1 = nodeQueue.poll();
			hoffmanNode min2 = nodeQueue.poll();
			
			hoffmanNode result = new hoffmanNode((char)0, min1.frequency+min2.frequency);
			result.lChild = min1;
			result.rChild = min2;
			nodeQueue.add(result);

		}
		
		root = nodeQueue.peek();

	}
	
	/**
	 *	get hoffman code, both String code and byte code
	 *
	 **/
	public static void getHoffmanCode(hoffmanNode treeRoot, String s, int len){
		if(treeRoot == null) {
			return;
		}
		if((treeRoot.lChild == null) && (treeRoot.rChild == null)){
			treeRoot.code = s;
			treeRoot.codeLength = len;
			//System.out.println("node " + treeRoot.c + " code is: " 
			//			+ treeRoot.code + " length: " + treeRoot.codeLength);
		}
		if(treeRoot.lChild != null) {
			getHoffmanCode(treeRoot.lChild, s + '0', len + 1);
		}
		if(treeRoot.rChild != null) {
			getHoffmanCode(treeRoot.rChild, s + '1', len + 1);
		}
	}
	
	public static void buildCodeTable() {
		if(root != null){
			getHoffmanCode(root.lChild, "0", 1);
			getHoffmanCode(root.rChild, "1", 1);
		}
		
		// check whether the code is stored in hChar, the globle variable
		for (i = 0; i < 128; i++){
			if(hChar[i].frequency > 0){
				// get the byteCode for each char
				hChar[i].byteCode = string2byteCode(hChar[i].code);
				//System.out.println("hChar " + hChar[i].c + ", frequency: " + hChar[i].frequency
				//		+ ", code is: " + hChar[i].code);			
			}
		}

	}
	
	/*
	 * 	write out encoded file
	 * 	Contents to be written:
	 * 		Magic number: <H F C D> 4 bytes
	 * 		Version: 				1 bytes
	 * 		table size:	 the number of items in the code table	1 bytes
	 * 		code table <char ch, int frequency>, store the char and its frequency
	 * 										in decoding process, use this frequency to build a tree
	 * 										can do fast comparasion to find the char.
	 * 		content length: 	4 bytes
	 * 		encoded content: 
	 * 
	 **/
	public static void hEncoding(String fileName, String outFile) throws IOException {
		// the code is string
		// String.charAt(idx) can return the char at position idx; idx starts at 0

		File inFile = new File(fileName);
        	Reader reader = null;
        
		FileOutputStream outf = new FileOutputStream(outFile);

		// write Magic number
		
		outf.write('H');
		outf.write('F');
		outf.write('C');
		outf.write('D');
		
		// version number
		outf.write('1');
		// table size
		byte[] tSize = int2byte(tableSize);
		outf.write(tSize[3]);	
		
		// write code table
		int intCode = 0;
		for (i = 0; i < 128; i++) {		// successfully
			if (hChar[i].frequency > 0){
				outf.write(hChar[i].c);
				
				// <char, frequency>
				outf.write(int2byte(hChar[i].frequency));
			}
		}
		
		// write content length
		outf.write(int2byte(totalChars));
		
		// write content     
        try{    
        	
            char[] inBuffer = new char[bufferSize];		// read in file by chars, buffer length 1024
            byte[] outBuffer = new byte[bufferSize];
			
			/*
			 * The follwing variables are used in bytes Code
			**/   
           
            int charRead = 0;
            int availableBitsInByte = 8;
            int bytesInOutBuffer = 0;
          
            int index = 0;
            int restCodeInHigh = 0;
            int restCodeInMid = 0;
            int restCodeInLow = 0;
            int codeMask = 0;
            byte xorByte = 0;
          
            reader = new FileReader(fileName);
            
            while ((charRead = reader.read(inBuffer)) != -1) {
			 /*
			  * the following part is for byte code
			  * */           	
 
				for (i = 0; i < charRead; i++) {
            		index = inBuffer[i];
            		
					//	The code in the byte array is: 
					//	hChar[].byteCode[0],hChar[].byteCode[1], hChar[].byteCode[2]
        		   
            		if(hChar[index].codeLength > 16) {
            			restCodeInHigh = hChar[index].codeLength - 16;
            			restCodeInMid = 8;
            			restCodeInLow = 8;
            		} else if(hChar[index].codeLength > 8) {
            			restCodeInHigh = 0;
            			restCodeInMid = hChar[index].codeLength - 8;
            			restCodeInLow = 8;
            		} else {
            			restCodeInHigh = 0;
            			restCodeInMid = 0;
            			restCodeInLow = hChar[index].codeLength;
            		}	
        			// first encod the high byte
        			   
           			while(restCodeInHigh > 0) {		// encoded the high byte
           				if(availableBitsInByte >= restCodeInHigh) {
           					           	
           					xorByte = (byte) ((hChar[index].byteCode[0] << (8 - restCodeInHigh))>> (8 - restCodeInHigh));
               				
           					for(int j = 0; j< restCodeInHigh; j++) {
           							codeMask = (codeMask << 1) + 1;
           					}
           					xorByte = (byte) (xorByte & codeMask);
           					
           					outBuffer[bytesInOutBuffer] = (byte) (outBuffer[bytesInOutBuffer] << restCodeInHigh ^ xorByte);
           					
           					
           					availableBitsInByte = availableBitsInByte - restCodeInHigh;
           					restCodeInHigh = 0;
           					
           				} else if(availableBitsInByte > 0){	
							// available Bits in Byte is less than rest code in high byte
							// '+' is the avliable bits: ******++
							// between '|' is the code in high, '#' is the rest code --|--cccc|
							// we want to put the first 'c' in the byte	
           					
           					xorByte = (byte) ((hChar[index].byteCode[0] << (8 - restCodeInHigh))>> (8 - availableBitsInByte));
               				
           					for(int j = 0; j< availableBitsInByte; j++) {
           							codeMask = (codeMask << 1) + 1;
           					}
           					xorByte = (byte) (xorByte & codeMask);
           					
           					outBuffer[bytesInOutBuffer] = (byte) (outBuffer[bytesInOutBuffer] << availableBitsInByte ^ xorByte);
           					
           					restCodeInHigh = restCodeInHigh - availableBitsInByte;
           					availableBitsInByte = 0;	// one byte is filled
           								
           				} 
           				else {	
							// available bits in byte is 0
						   //System.out.println("code Length > 16: " + "build byte: " + 
						   //outBuffer[bytesInOutBuffer] + " bytesInOutBuffer is: " + (bytesInOutBuffer+1) );

           					availableBitsInByte = 8;
           					bytesInOutBuffer++;
           						
           					if(bytesInOutBuffer == bufferSize) {
           						outf.write(outBuffer);
           						bytesInOutBuffer = 0;
           						
           						for(int j = 0; j< bufferSize; j++) {
               						outBuffer[j] = 0;
               					}
           					}
           				}
           			} // while restCodeInhigh
        			   // if available bits is longer than the rest code in mid byte
            			
           			while(restCodeInMid > 0) {		// encoded the mid byte
           				if(availableBitsInByte >= restCodeInMid) {
           					
           					xorByte = (byte) ((hChar[index].byteCode[1] << (8 - restCodeInMid))>> (8 - restCodeInMid));
               				
           					for(int j = 0; j< restCodeInMid; j++) {
           							codeMask = (codeMask << 1) + 1;
           					}
           					xorByte = (byte) (xorByte & codeMask);
           					
           					outBuffer[bytesInOutBuffer] = (byte) (outBuffer[bytesInOutBuffer] << restCodeInMid ^ xorByte);
           					
           					availableBitsInByte = availableBitsInByte - restCodeInMid;
           					restCodeInMid = 0;
           					codeMask = 0;
           					
           				} else if(availableBitsInByte > 0){	
							// available Bits in Byte is less than rest code in high byte
							// '+' is the avliable bits: ******++
							// between '|' is the code in high, '#' is the rest code --|--cccc|
							// we want to put the first 'c' in the byte	
           					xorByte = (byte) ((hChar[index].byteCode[1] << (8 - restCodeInMid))>> (8 - availableBitsInByte));
               				
           					for(int j = 0; j< availableBitsInByte; j++) {
           							codeMask = (codeMask << 1) + 1;
           					}
           					xorByte = (byte) (xorByte & codeMask);
           					
           					outBuffer[bytesInOutBuffer] = (byte) (outBuffer[bytesInOutBuffer] << availableBitsInByte ^ xorByte);
           					
           					
           					restCodeInMid = restCodeInMid - availableBitsInByte;
           					availableBitsInByte = 0;	// one byte is filled	
           					codeMask = 0;
           				} 
           				else {	// available bits in byte is 0
							//System.out.println("code Length > 8: " + "build byte: " + 
							//		outBuffer[bytesInOutBuffer] + " bytesInOutBuffer is: " + (bytesInOutBuffer+1) );

           					availableBitsInByte = 8;
           					bytesInOutBuffer++;	
           					if(bytesInOutBuffer == bufferSize) {
           						outf.write(outBuffer);
           						bytesInOutBuffer = 0;
           						
           						for(int j = 0; j< bufferSize; j++) {
               						outBuffer[j] = 0;
               					}
           					}      		
           				}
           			}	// while rest code in mid
            			
           			while(restCodeInLow > 0) {		// encoded the low byte
          				
           				if(availableBitsInByte >= restCodeInLow) {
 	
           					xorByte = (byte) ((hChar[index].byteCode[2] << (8 - restCodeInLow))>> (8 - restCodeInLow));
               				
           					for(int j = 0; j< restCodeInLow; j++) {
           							codeMask = (codeMask << 1) + 1;
           					}
           					xorByte = (byte) (xorByte & codeMask);
           					
           					outBuffer[bytesInOutBuffer] = (byte) (outBuffer[bytesInOutBuffer] << restCodeInLow ^ xorByte);
           					
           					availableBitsInByte = availableBitsInByte - restCodeInLow;
           					restCodeInLow = 0;
           					codeMask = 0;
           					
           				} else if(availableBitsInByte > 0){	
							// available Bits in Byte is less than rest code in high byte
							// '+' is the avliable bits: ******++
							// between '|' is the code in high, '#' is the rest code --|--cccc|
							// we want to put the first 'c' in the byte	
							////////DONE	
           					xorByte = (byte) ((hChar[index].byteCode[2] << (8 - restCodeInLow))>> (8 - availableBitsInByte));
           				
           					for(int j = 0; j< availableBitsInByte; j++) {
           							codeMask = (codeMask << 1) + 1;
           					}
           					xorByte = (byte) (xorByte & codeMask);
           					
           					outBuffer[bytesInOutBuffer] = (byte) (outBuffer[bytesInOutBuffer] << availableBitsInByte ^ xorByte);
				
           					restCodeInLow = restCodeInLow - availableBitsInByte;
           					availableBitsInByte = 0;	// one byte is filled
           					codeMask = 0;
           								
           				} 
           				else {	// available bits in byte is 0, a byte is filled

           					availableBitsInByte = 8;
           					bytesInOutBuffer++;				
           					
           					if(bytesInOutBuffer == bufferSize) {
           						outf.write(outBuffer);
           						bytesInOutBuffer = 0;
           						
           						for(int j = 0; j< bufferSize; j++) {
               						outBuffer[j] = 0;
               					}
           					}
           		
           				}
           			}	// while rest code in low

        		   
        	   }// for
			// byte code end           	
			/**********************************************************************************/
            	

			/******************************************************************************************/ 
			}	// while
			
            // for byte code
            if(availableBitsInByte > 0){	// in the last run, one byte is built
            	outBuffer[bytesInOutBuffer] = (byte) (outBuffer[bytesInOutBuffer] << availableBitsInByte);
            }
           outf.write(outBuffer, 0, bytesInOutBuffer + 1);		
            
        } catch (Exception e1) {
            e1.printStackTrace();
        } 
		outf.close();
		
	}
	
	public static byte[] int2byte (int i){		
		// int to 4 bytes, used to convert table size
		
		return new byte[] {
				(byte) ((i>>24) & 0xFF),
				(byte) ((i>>16) & 0xFF),
				(byte) ((i>>8) & 0xFF),
				(byte) (i & 0xFF)
		};
	}
	
	public static char[] int2char(int i){
		return new char[] {
				(char) ((i>>24) & 0xFF),
				(char) ((i>>16) & 0xFF),
				(char) ((i>>8) & 0xFF),
				(char) (i & 0xFF)
		};
	}
	
	// String to byte code, use three byte to store the string code 
	public static byte[] string2byteCode (String s){
		// code is a "01" string, convert into 3 bytes
		byte[] byteCode = new byte[3];
		byte[] tmp = new byte[4];
		int t = 0;
		int highLen = 0;
		int intCode = 0;
		
		for(t = 0; t < s.length(); t++) {
			if(s.charAt(t) == '0'){
				intCode = intCode << 1;
			} else {
				intCode = (intCode << 1) ^ 0x01;
			}
		}
		
		tmp = int2byte(intCode);
		
		byteCode[0] = tmp[1];
		byteCode[1] = tmp[2];
		byteCode[2] = tmp[3];
	
		return byteCode;
	}
	
	public static String bytesCode2string(byte[] b) {
		// used to the first byte is the length of the code
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
	 * Simple start compression
	 * @param args
	 * @throws IOException 
	 */
	public static void main(String[] args) throws IOException {
		// TODO Auto-generated method stub
		long begin = System.currentTimeMillis();
		//String fileName = "/opt/hoffman_test.txt";
		//String fileName = "/opt/book_example.txt";
		//String fileName = "/opt/test_data.txt";
		//String fileName = "/opt/compress_test.txt";
		//String outFile = "/opt/encoded_file1.txt";
		//String outFile = "/opt/test_data_encoded.txt";
		String fileName = args[0];	// while sh compress.sh is used
		String outFile = args[1];
		fileFrequency(fileName);		// step 1: read in file and compute freq
		buildHoffmanTree();				// step 2: build hoffman tree
		buildCodeTable();				// step 3: generate hoffman code
		hEncoding(fileName, outFile);			// step 4: encode file
		long end = System.currentTimeMillis();
		System.out.println("Compressing time cost: " + (end - begin) + " milli seconds");
		
	
	//  read in file time test
	
	/*	long begin = System.currentTimeMillis();
		
		String fileName = args[0];
		File file = new File(fileName);
        Reader reader = null;  
        
		char[] charBuffer = new char[bufferSize];		// read in file by chars, buffer length bufferSize
        int charRead = 0;		// the length of chars have been read
        
		reader = new FileReader(file);
        while ((charRead = reader.read(charBuffer)) != -1) {
            for (i = 0; i < charRead; i++) {
				int index = charBuffer[i];
            	//	hChar[index].increaseFreq(1);	// increase the frequency of the char
            }
        }
		long end = System.currentTimeMillis();
		System.out.println("read in file time cost: " + (end - begin) + " milli seconds");
		
	*/	
	}

}

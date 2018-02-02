//package hoffman.compress;

public class hoffmanNode {
	char c;
	int frequency;
	
	hoffmanNode lChild;
	hoffmanNode rChild;
	
	String code;	// store code
	int codeLength;
	
	byte[] byteCode;	// use 3 bytes to store the hoffman code. Must faster than String presentation
	
	public hoffmanNode(){
		
	}
	
	public hoffmanNode(char ch, int freq){
		c = ch;
		frequency = freq;
		lChild = null;
		rChild = null;
		code = null;
		codeLength = 0;
		byteCode = new byte[3];
	}
	public void increaseFreq(int amount) {
		frequency = frequency + amount;
	}

	public void initialization() {
		// TODO Auto-generated method stub
		frequency = 0;
	}
	
}

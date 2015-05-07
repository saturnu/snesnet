package mod;

import java.io.DataOutputStream;

public class TestModule extends Module{


	public TestModule(DataOutputStream dos) {
		super(dos);
	}

	@Override
	public byte getMagicByte() {
		return 0x01;
	}

	@Override
	public void setCommand(String cmd) {


		  if(cmd.equals("74")){ 
	  			
        	  
  			  //0b1111 11 111111111
  			  //0=bt pressed, 1=bt released
  			  //FFFE = B
  			byte[] data = new byte[2];
  			data[0]=(byte) 0xFF;
  			data[1]=(byte) 0xFE; //B
  			
  			sendBytes(data);
  			
  			
		  		  try {
					Thread.sleep(35);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
  			
  			  
	  			data[0]=(byte) 0xFF;
	  			data[1]=(byte) 0xFF;
	  			  
	  		sendBytes(data);
  			  
  		  }
		
	}
	

}

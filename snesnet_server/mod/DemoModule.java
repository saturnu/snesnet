
package mod;

import java.io.DataOutputStream;

public class DemoModule extends Module{

	private long str_length;
	private String string0;
	

	public DemoModule(DataOutputStream dos) {
		super(dos);
		str_length=0;
		string0="";
	}

	@Override
	public byte getMagicByte() {
		return 0x02;
	}

	@Override
	public void setCommand(String cmd) {
		
		if(str_length==0){
		str_length = Long.parseLong(cmd, 16);
		System.out.println("str_length="+str_length);
		}else{
			
			
			if(str_length>0){
				string0=string0+((char)Long.parseLong(cmd, 16));
				
			}
			
			if(string0.length()==str_length && str_length!=0){
				System.out.println("received string: "+string0);
				str_length=0;
				string0="";
			}
		}
		/*
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
  		  */
		
	}
	

}

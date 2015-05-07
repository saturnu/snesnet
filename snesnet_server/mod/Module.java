package mod;

import java.io.DataOutputStream;
import java.io.IOException;


public class Module {

	String status;
	DataOutputStream dos;
	
	


	public void setDos(DataOutputStream dos) {
		this.dos = dos;
	}


	public Module(DataOutputStream dos) {
		this.dos=dos;
    	status=null;
	}
	  

	public void setStatus(String string) {
		status=string;
	}
		
	
	public void setCommand(String cmd) {
	}
	  
	  public byte getMagicByte () {
		return 0x00;
	}
	  
	public String getStatus() {
		return status;
	}

	  
	public int sendBytes(byte data[]){
	    	  try {
	    		  
	    		  if(dos==null)
	    			  System.out.println("dos is null");
	    		  else
	  			dos.write(data, 0, 2);
	  		} catch (IOException e) {
	  			// TODO Auto-generated catch block
	  			e.printStackTrace();
	  		} //.print(a+""+b);

	    	  try {
	    		  if(dos==null)
	    			  System.out.println("dos is null");
	    		  else
	  			dos.flush();
	  		} catch (IOException e) {
	  			// TODO Auto-generated catch block
	  			e.printStackTrace();
	  		}
	    	//  System.out.println("data: "+data);
	    	  return 0;
	      }
	  
	  
}


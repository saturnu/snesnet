import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.LinkedList;
import java.util.Random;

import mod.DemoModule;
import mod.DummyModule;
import mod.TestModule;
import net.MySQLAccess;
import net.Session;

public class SnesNETsrv {

      // the socket used by the server
      private ServerSocket serverSocket;
      private LinkedList<Session> session_list;
      final protected static char[] hexArray = "0123456789ABCDEF".toCharArray();
      
      //debug;
      long startTime=0;
      long pkt_cnt=0;
      
      
      // server constructor
      SnesNETsrv (int port) {
    	  
    	  session_list = new LinkedList<Session>();
      //	modulcontainerlist = new LinkedList();
              /* create socket server and wait for connection requests */
              try
              {
                      serverSocket = new ServerSocket(port);
                      System.out.println("Server waiting for client on port " + serverSocket.getLocalPort());

                      while(true)
                      {
                              Socket socket = serverSocket.accept();  // accept connection
                            //  System.out.println("New client asked for a connection");
                             
                              TcpThread t = new TcpThread(socket);    // make a thread of it
                              System.out.println("Starting a thread for a new Client");
                              t.start();
                      }
              }
              catch (IOException e) {
                      System.out.println("Exception on new ServerSocket: " + e);
              }
      }               

//      you must "run" server to have the server run as a console application
      public static void main(String[] arg) {
              // start server on port 1500
              new SnesNETsrv(13375);
      }
      
      /*
      public int sendBytes(DataOutputStream output, byte data[]){
    	  try {
			output.write(data, 0, 2);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} //.print(a+""+b);

    	  try {
			output.flush();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    	//  System.out.println("data: "+data);
    	  return 0;
      }
     */
     
      private int addSession(String user, int id){
      	
    	  int remove_flag=-1;
    	  
    	  for(int i=0;i<session_list.size();i++){
    		  if( ((Session)session_list.get(i)).getId()==id ){
    			  System.out.println("user "+id+" already on list");
    			  	//is too old - remove?
    			  long logout_time = ((Session)session_list.get(i)).getLogout_time();
    			  long current_time = System.currentTimeMillis();
    			  
    			  if(current_time-logout_time > 3000){
    				  System.out.println("session of user "+id+" expired");
    				  remove_flag=i;
    			  }else
    			   return -1; //active session - reauth needed

    		  }
    	  }
    	  
    	  //delete old session
    	  if(remove_flag!=-1)
    		  session_list.remove(remove_flag);
    	  
		  Session s0 = new Session(id, user); //not on list... add new session
	//	  s0.setModul(new DummyModule(null));
		  session_list.add(s0);
  	
		  return 0;
      }
      
      private Session getSession(int id){
        	
    	  for(int i=0;i<session_list.size();i++){
    		  if( ((Session)session_list.get(i)).getId()==id ){  
    			  return ((Session)session_list.get(i)); //active session
    		  }
    	  }
		  return null;
      }
      
  	public static String bytesToHex(byte[] bytes) {
	    char[] hexChars = new char[bytes.length * 2];
	    for ( int j = 0; j < bytes.length; j++ ) {
	        int v = bytes[j] & 0xFF;
	        hexChars[j * 2] = hexArray[v >>> 4];
	        hexChars[j * 2 + 1] = hexArray[v & 0x0F];
	    }
	    return new String(hexChars);
	}
      
      /*
      private void clearList(PrintWriter Soutput, BufferedReader Sinput){
      
 
      	for(int i=0;i<modulcontainerlist.size();i++){
      	Modul dummy = ((ModulContainer)modulcontainerlist.get(i)).getModul();
      		
      		if(dummy.getStatus().equals("stopped") || dummy.getStatus().equals("finished")  || dummy.getStatus().equals("faild")){
      			((ModulContainer)modulcontainerlist.get(i)).getTr().stop();
      			*/
      			/*
      			try {
						((ModulContainer)Modulcontainerlist.get(i)).getTr().sleep( 2000 );
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
					*/
    	  /*
      			((ModulContainer)modulcontainerlist.get(i)).getTr().interrupt();
      			modulcontainerlist.remove(i);
      			System.gc(); //fire garbage collector :>
      		}
      	}
      
      }
      */
      
      private void login(PrintWriter o, BufferedReader i){
    	  System.out.println("login");
      }
      
      
      //10 random bit for trivium
      //only harmless chars used for the lulz
      private String generateIV(){
    	  String iv = "";
    	  
    	    Random randomGenerator = new Random();
    	    for (int idx = 1; idx <= 10; ++idx){
    	      int randomInt = randomGenerator.nextInt(72);
    	      if(randomInt == '\\') randomInt='c';
    	      iv = iv + "" + (char)(0x30+randomInt);
    	    }
    	  
    	  return iv;
      }
      
     
      /** One instance of this thread will run for each client */
      class TcpThread extends Thread {
              // the socket where to listen/talk
              Socket socket;
              BufferedReader input;
              PrintWriter output;
              
              OutputStream out_stream;
              DataOutputStream dos;
              
              MySQLAccess dao = new MySQLAccess("jdbc:mysql://10.0.0.1/snesnet", "snesnet", "asdasd");
              
              String username;
              String password;
              Session session0;
              int auth_id=0;
              int reauth=0;
              int id;
              String iv="";
             
              TcpThread(Socket socket) {
                      this.socket = socket;
              }
              public void run() {
                      /* Creating both Data Stream */
                      System.out.println("Thread trying to create Object Input/Output Streams");
                      try
                      {
                    	  out_stream = socket.getOutputStream(); 
                    	  dos = new DataOutputStream(out_stream);
                    	  output = new PrintWriter(socket.getOutputStream(), true);
                    	  
                          input = new BufferedReader( new InputStreamReader(socket.getInputStream()));
                           
                             
                      }
                      catch (IOException e) {
                              System.out.println("Exception creating new Input/output Streams: " + e);
                              return;
                      }
                      System.out.println("Thread waiting for a String from the Client");

      
                      boolean active = true;
                      
                      //loop connection
                      do{
                      String cmd = null;
                    
                      if(socket.isConnected())
						try {
							//line based input
							cmd = input.readLine(); 
						} catch (IOException e) {
							auth_id=0;
                            output.close();
                            
                            if(session0!=null)
                            	session0.setLogout_time(System.currentTimeMillis());
                            
                            try {
								input.close();
							} catch (IOException e1) {
								// TODO Auto-generated catch block
								if(session0!=null)
	                            	session0.setLogout_time(System.currentTimeMillis());
								e1.printStackTrace();
							}
						   // Handle the error
						}


						
                      if(cmd != null){
                    //  System.out.println("*"+cmd+"*");
                      switch(cmd.charAt(0)){
                      /*
                        username = client0
						password = oisjoseoisiusde
						key = 0AnL]d`50m
                       */
                      
                      
                      case 'U' : username = cmd.substring(1);
                      				System.out.println("#"+username+"#");
                      				//1. check if valid user in db
                      				
                      				//todo:
                      				//some security and length checks here
                      				
                      				String statement = "select * from snesnet.user where username like '"+ username +"'";
                      			    int db_userid=0;
									try {
										db_userid = dao.getIntFromqueryDB(statement, "userid");
									} catch (Exception e1) {
										// TODO Auto-generated catch block
										e1.printStackTrace();
									}
                      				
									if(db_userid!=0){
											if(addSession(username,db_userid)==0){ //2. is valid username - add session
	                      						System.out.println("new session added: auth needed");
	                      					}else{
	                      						System.out.println("old session: reauth needed");
	                      						reauth=1;
	                      					}
											id=db_userid;
									
									// reference Session
									session0 = getSession(db_userid);
											
									//send iv
										iv = generateIV();
										System.out.println("iv test: "+iv);
										output.println("IV "+iv);
											
									}else{
									//send wrong username
										output.println("ERROR");
									}
										
                      				
                      				//maybe add with unique db-id instead 1-333
                      			
                      			
                      				break;
					                      case 'P' : password = cmd.substring(1);
					                      System.out.println("#"+password+"#");
					                      
					                   
					                      
					                        String key="";
					                       // System.out.println("id: "+id);
		                      				statement = "select * from snesnet.user where userid = '"+ id +"'";
		                      			  
											try {
												
												key = dao.getStringFromqueryDB(statement, "key");
											} catch (Exception e1) {
												// TODO Auto-generated catch block
												e1.printStackTrace();
											}
					                     
					                        String db_pw="";
					                       // System.out.println("id: "+id);
		                      				statement = "select * from snesnet.user where userid = '"+ id +"'";
		                      			  
											try {
												
												db_pw = dao.getStringFromqueryDB(statement, "password");
											} catch (Exception e1) {
												// TODO Auto-generated catch block
												e1.printStackTrace();
											}
					                      
					                    
					                      
					              		String iv_hex = bytesToHex(iv.getBytes());
					            		String key_hex = bytesToHex(key.getBytes());
					            		String pass_hex_crypt = password; //bytesToHex(password.getBytes());

					                  
					             
					                    
					                         
					                        // using the Runtime exec method:
					                    	String exec = "/home/tt/snesnet/trivium_cmd/trivium "+key_hex+" "+iv_hex+" "+pass_hex_crypt;
											Process p = null;
											try {
												p = Runtime.getRuntime().exec(exec);
											} catch (IOException e1) {
												// TODO Auto-generated catch block
												e1.printStackTrace();
											}
										    
										                        BufferedReader stdInput = new BufferedReader(new
										                        InputStreamReader(p.getInputStream()));
										             
										                        // read the output from the command
										  
										    String hex_pw = "";                  
											try {
												String tmp = null;
												while ((tmp = stdInput.readLine()) != null) {
													hex_pw = tmp.substring(5);
												}
											} catch (IOException e1) {
												// TODO Auto-generated catch block
												e1.printStackTrace();
											}
										  

										   // System.out.println("enc2: "+hex_pw); //pw in hex
											  StringBuilder decoded_pw = new StringBuilder();
											    for (int i = 0; i < hex_pw.length(); i+=2) {
											        String str = hex_pw.substring(i, i+2);
											        decoded_pw.append((char)Integer.parseInt(str, 16));
											    }
											    System.out.println( "decoded: "+ decoded_pw);
											
											//sqlquery :> if plain pw is the right one
											
												String user_esc = username; //todo escape me first
												String pass_esc = decoded_pw.toString(); //todo escape me first

												String auth_query = "SELECT userid FROM snesnet.user WHERE username = '"+user_esc+"' AND password = (SHA1(CONCAT_WS(':','"+user_esc+"','"+pass_esc+"')))"; 
												
				                  		
												
												try {
													auth_id = dao.getIntFromqueryDB(auth_query, "userid");
													//not found -> null -> 0
												} catch (Exception e1) {
													// TODO Auto-generated catch block
													e1.printStackTrace();
												}
						/*					    
						try {
							sleep(1000);
						} catch (InterruptedException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
						*/					
												if(auth_id!=0){
													 System.out.println( "auth: access granted");
													 output.println("OK");
												}else{
													 System.out.println( "auth: access denied");
													 output.println("ERROR");
												}
											   
					                      
					                      break;
                      
                      
                      case 'C' : //command
                    	  		  if(auth_id!=0){
                    	  			    String value=cmd.substring(1);
                    	  			    System.out.println("cmd: ["+value+"]");
                    	  			    
                    	  			  		//check if reauth
                    	  			  		if(reauth==0){
                    	  			  			//first byte is module id
                    	  			  			System.out.println("setting module:"+value);
                    	  			  			switch(value){
                    	  			  			 case "01": session0.setModule(new TestModule(dos));
                    	  			  			 			break;
                    	  			  			 case "02": session0.setModule(new DemoModule(dos));
         	  			  			 						break;                    	  			  			 			
                    	  			  			
                    	  			  			 default: session0.setModule(new DummyModule(dos));
                    	  			  			}
                    	  			  			
                    	  			  			//setting in reauth mode not to add the next cmd as module
                    	  			  			reauth=2;
                    	  			  		
                    	  			  		}else{
                    	  			  			if(reauth==1){ //real_reauth mode - set new dos
                    	  			  			System.out.println("setting new dos to module");
	                    	  			  			session0.getModule().setDos(dos);
	                    	  			  			reauth=2;
                    	  			  			}
                    	  			  			
                    	  			  			//std module command
                    	  			  			session0.getModule().setCommand(value);
                    	  			  		}
		                    	  		
		                    	  		  
		                    	  			/*if(value.equals("ff")){
		                    	  			startTime = System.currentTimeMillis();
		                    	  			}else if (value.equals("00")){
		                    	  			  long stopTime = System.currentTimeMillis();
		                    	  		      long elapsedTime = stopTime - startTime;
		                    	  		      System.out.println("benchmark - 100 bytes: "+elapsedTime+" ms");
		                    	  			}else{
		                    	  			  System.out.println("pkt counter: "+ (++pkt_cnt));
		                    	  			}
		                    	  			*/
		                    	  			
                    	  			
                    	  		  }
                    	  		 //send command to modul inside session
                    	  			break;
                      case 'Q' :  //quit
                      		//log out
                      	       try {
                      	    	      auth_id=0;
                      	    	      session0.setLogout_time(System.currentTimeMillis());
                      	    	   	  active=false;
                                      output.close();
                                      input.close();
                                      System.out.println("Client quit");
                              }
                              catch (Exception e) {                                       
                              }
                      	
                      	break;
                      case 'X' : //clearList(output, input); 
                      				break; //clear finished stopped
                      	default : break;
                      }
                      }else{
                    	  //nullstring -> quit client
                 	       try {
                 	    	  auth_id=0;
                 	    	 session0.setLogout_time(System.currentTimeMillis());
               	    	   	  active=false;
                               output.close();
                               input.close();
                               System.out.println("Client quit");
                       }
                       catch (Exception e) {                                       
                       }
                      }
                      
                      //still active?
                      }while(active);
                              
                              
                      }
              }
      }


package net;

import java.io.DataOutputStream;

import mod.Module;

public class Session {

	private int id;
	private String username;
	private Module module0;
	private long logout_time;



	public Session(int id_num, String name){

		id=id_num;
		setUsername(name);
		setLogout_time(0);
	}
	
	
	public int getId() {
		return id;
	}
	public void setId(int id) {
		this.id = id;
	}


	public String getUsername() {
		return username;
	}


	public void setUsername(String username) {
		this.username = username;
	}


	public Module getModule() {
		return module0;
	}


	public void setModule(Module module0) {
		this.module0 = module0;
	}


	public long getLogout_time() {
		return logout_time;
	}


	public void setLogout_time(long logout_time) {
		this.logout_time = logout_time;
	}
	
}

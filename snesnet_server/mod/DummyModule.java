package mod;

import java.io.DataOutputStream;

public class DummyModule extends Module {



	public DummyModule(DataOutputStream dos) {
		super(dos);

	}

	@Override
	public void setStatus(String string) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void setCommand(String cmd) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public byte getMagicByte() {
		return 0x00;
	}

	@Override
	public String getStatus() {
		// TODO Auto-generated method stub
		return null;
	}

}

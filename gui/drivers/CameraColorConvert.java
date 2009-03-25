package rack.gui.drivers;

public class CameraColorConvert {

	public CameraColorConvert() {
		super();
	}
	
	public int calcMono8(int color) {
		color = color & 0xff;
		return 0xFF000000 | (color << 16) | (color << 8) | (color << 0);
	}
	
	public int calcMono12(int color) {
		color = (color >> 4) & 0xff;
		return 0xFF000000 | (color << 16) | (color << 8) | (color << 0);
	}
	
	public int calcMono16(int color) {
		color = (color >> 8) & 0xff;
		return 0xFF000000 | (color << 16) | (color << 8) | (color << 0);
	}
	
	public int calcMono24(int color) {
		color = (color >> 16) & 0xff;
		return 0xff000000 | (color << 16) | (color << 8) | (color << 0);
	}
	
}

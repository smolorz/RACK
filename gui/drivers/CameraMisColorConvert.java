package rack.gui.drivers;

import java.awt.Color;
import java.util.Iterator;
import java.util.Vector;


public class CameraMisColorConvert extends CameraColorConvert {

	private int[] lockuptable;
	
	private Vector<Color> col;
	private Vector<Integer> pos;
	
	public CameraMisColorConvert() {
		super();
		lockuptable = new int[0x10000];
		col = new Vector<Color>();
		pos = new Vector<Integer>();
	}
	
	public void initTable() {
		col.clear();
		pos.clear();
	}
	
	public void addColor(int pos, Color col) {
		if ((pos >= 0) && (pos < this.lockuptable.length)) {
			int i = 0;
			while ((i < this.pos.size()) && 
					(pos < this.pos.get(i).intValue())) {
				i++;
			}
			if ((i < this.pos.size()) &&
				(pos == this.pos.get(i).intValue())) {
				this.col.set(i, new Color(col.getRed(), col.getGreen(), col.getBlue(), col.getAlpha()));
				return;
			}
			this.col.insertElementAt(new Color(col.getRed(), col.getGreen(), col.getBlue(), col.getAlpha()), i );
			this.pos.insertElementAt(new Integer(pos), i);
		}
	}
	
	public void removeColor(int pos) {
		for(int i = 0; i < this.pos.size(); i++) {
			if (this.pos.get(i).intValue() == pos) {
				this.pos.remove(i);
				this.col.remove(i);
				return;
			}
		}
	}
	
	public boolean calcTable() {
		for (int i = 0; i < this.lockuptable.length; i++) {
			this.lockuptable[i] = 0x00000000;				
		}	

		if ((this.pos.size() == 0) || (this.pos.size() != this.col.size()))
			return false;
		Iterator<Integer> iter = this.pos.iterator();
		while (iter.hasNext()) {
			Integer i = iter.next();
			if ((i.intValue() < 0) || (i.intValue() >= this.lockuptable.length))
				return false;
		}
			
		if (this.pos.size() == 1) {
			this.lockuptable[this.pos.get(0)] = this.col.get(0).getRGB();
			return true;
		}
			
		Iterator<Integer> iterpos = this.pos.iterator();
		Iterator<Color> itercol = this.col.iterator();
			
		Color gc0 = null;
		Color gc1 = itercol.next(); 
			
	    int gpos0 = 0;
		int gpos1 = iterpos.next().intValue();
		
		while (iterpos.hasNext() && itercol.hasNext()) {
			gc0 = gc1;
			gpos0 = gpos1;
			
			gc1 = itercol.next();
			gpos1 = iterpos.next().intValue();
			
			Color c0;
			Color c1;
			int pos0;
			int pos1;
			
			if (gpos0 <= gpos1) {
				c0 = gc0;
				c1 = gc1;
				pos0 = gpos0;
				pos1 = gpos1;
			} else {
				c0 = gc1;
				c1 = gc0;
				pos0 = gpos1;
				pos1 = gpos0;
			}
			
			

			int colora = c0.getAlpha();
			int colorr = c0.getRed();
			int colorg = c0.getGreen();
			int colorb = c0.getBlue();
			
			
			int dx = pos1 - pos0;
			int da = c1.getAlpha() - c0.getAlpha();
			int dr = c1.getRed() - c0.getRed();
			int dg = c1.getGreen() - c0.getGreen();
			int db = c1.getBlue() - c0.getBlue();
			
			int stepa = (da >= 0)?1:-1;
			int stepr = (dr >= 0)?1:-1;
			int stepg = (dg >= 0)?1:-1;
			int stepb = (db >= 0)?1:-1;

			int sa = stepa * da;
			int sr = stepr * dr;
			int sg = stepg * dg;
			int sb = stepb * db;
			
			dx *= 2;
			da *= stepa * 2;
			dr *= stepr * 2;
			dg *= stepg * 2;
			db *= stepb * 2;

			for (int x = pos0; x <= pos1; x++) {
				this.lockuptable[x] = (colora << 24) | (colorr << 16) | (colorg << 8) | colorb; 				
				
				if (dx != 0) {
					sa += da;
					while (sa >= dx) {
						sa -= dx;
						colora+= stepa;
					}
					sr += dr;
					while (sr >= dx) {
						sr -= dx;
						colorr+= stepr;
					}
					sg += dg;
					while (sg >= dx) {
						sg -= dx;
						colorg+= stepg;
					}
					sb += db;
					while (sb >= dx) {
						sb -= dx;
						colorb+= stepb;
					}
				}
			}					
		}
	    return true;	
	}
	

	@Override
	public int calcMono8(int color) {
		return this.lockuptable[(color) & 0x00ff];
//		return this.lockuptable[(color << 8) & 0xffff];
	}

	@Override
	public int calcMono12(int color) {
		return this.lockuptable[(color) & 0x0fff];
	}

	@Override
	public int calcMono16(int color) {
		return this.lockuptable[(color) & 0xffff];
	}

/*	@Override
	public int calcMono24(int color) {
		//not jet implemented
		return 0;
	}
*/
	public int calcMono17(int color) {
		// Use only 17-Bit of the 24-Bit gray values
		if (color > 0x1ffff)
			return this.lockuptable[0xffff];
		return this.lockuptable[(color >> 1) & 0xffff];
	}
}

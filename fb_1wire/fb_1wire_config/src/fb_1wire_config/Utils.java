/*
 * Version 1.0
 * 14.06.2005
 * 
 * eichlger
 */
package fb_1wire_config;

import java.io.File;

public class Utils {
    public final static String hex = "hex";
    public final static String all = "*";

    /*
     * Get the extension of a file.
     */
    public static String getExtension(File f) {
        String ext = null;
        String s = f.getName();
        int i = s.lastIndexOf('.');

        if (i > 0 &&  i < s.length() - 1) {
            ext = s.substring(i+1).toLowerCase();
        }
        return ext;
    }
	public static int makeGrpAdrStoI(String s1){
		int grpAdr = 0;
		String s[] = s1.split("/");
		if (s.length == 3){
			//  aaaallll mmmmmmmm
			// area line member
			//area
			grpAdr += (Integer.parseInt(s[0]) << 11);
			//line
			grpAdr += (Integer.parseInt(s[1]) << 8);
			//member
			grpAdr += Integer.parseInt(s[2]);
			
		}
		return grpAdr;
	}
	
	public static String makeGrpAdrItoS(int i1){
		String s1 = "";
		if (i1 > 0){
			s1 = String.valueOf(i1 >> 11) + "/" + String.valueOf((i1 >> 8) & 0x07) + "/" + String.valueOf(i1 & 0xFF);
		}else{
			s1 = "0/0/0";
		}
		return s1;
	}
	public static int makePhysAdrStoI(String s1){
		int grpAdr = 0;
		String s[] = s1.split(".");
		if (s.length == 3){
			//  aaaallll mmmmmmmm
			// area line member
			//area
			grpAdr += (Integer.parseInt(s[0]) << 12);
			//line
			grpAdr += (Integer.parseInt(s[1]) << 8);
			//member
			grpAdr += Integer.parseInt(s[2]);
			
		}
		return grpAdr;
	}
	
	public static String makePhysAdrItoS(int i1){
		String s1 = "";
		if (i1 > 0){
			s1 = String.valueOf(i1 >> 12) + "." + String.valueOf((i1 >> 8) & 0x0F) + "." + String.valueOf(i1 & 0xFF);
		}else{
			s1 = "0.0.0";
		}
		return s1;
	}
	public static int boolenToInt(boolean b1){
		if (b1){
			return 1;
		}else{
			return 0;
		}
	}
	public static String getDefaultString(String s1,String s2){
		if ((s1 == null) || (s1 == "")){
			return s2;
		}else{
			return s1;
		}
	}
    public static byte[] intToByte(int i1){
		byte b[] = new byte[2];
		
        b[0] = (byte)(i1 & 0x00FF);
		b[1] = (byte)(i1 >> 8);
		return b;
	}
    public static int byteToInt(byte[] b){
		int i1 = 0;
    	if (b.length == 2){
			i1 = ((int)(b[0]) << 8) + ((int)(b[1]) & 0x00FF) ;
		}
    	return i1;
	}
	public static byte[] floatToByte(float f1){
		byte b[] = new byte[4];
		
		int i1 = Float.floatToIntBits(f1);
		
        b[3] = (byte)((i1 >> 24) & 0xFF);
		b[2] = (byte)((i1 >> 16) & 0xFF);
		b[1] = (byte)((i1 >> 8) & 0xFF);
		b[0] = (byte)(i1 & 0xFF);
		return b;
	}
	public static byte[] sensorIdToBytearray(String s){
		byte[] b1 = new byte[8];
		String[] s1 = s.split(":");
		if (s1.length == 8){
			for (int i = 0; i < s1.length;i++){
				int v = Integer.parseInt(s1[i], 16);
				b1[i] = (byte) v;
			}
		}
		return b1;
	}
	public static String sensorIdToString(byte[] b){
		String s = "";
		if (b.length == 8){
			for (int i = 0; i < b.length;i++){
				if (i>0){
					s+=":";
				}
				s += String.format("%02x", b[i]).toUpperCase();
				
			}
		}
		
		return s;
	}

}

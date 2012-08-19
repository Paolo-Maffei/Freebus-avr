package fb_1wire_config;
/*
 * Version 1.0
 * 14.06.2005
 */

public class TreeData {
	
	public String name = null;
	public int Number = 0;
	public int type = 0;
	public static final int NULL = 0;
	public static final int SETTINGS = 1;
	public static final int SENSOR = 2;
	public TreeData(){
		
	}
	public TreeData(String inName, int inNumber, int inType){
		
		name = inName;
		Number = inNumber;
		type = inType;
	}
	public String toString(){
		return name;
	}
}

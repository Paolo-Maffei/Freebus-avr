/*
 * Version 1.0
 * 14.06.2005
 * 
 * eichlger
 */
package fb_1wire_config;

import java.io.File;
import javax.swing.filechooser.*;

public class HexFilter extends FileFilter{

	
	//Accept all directories and all gif, jpg, tiff, or png files.
    public boolean accept(File f) {
        if (f.isDirectory()) {
            return true;
        }

        String extension = Utils.getExtension(f);
        if (extension != null) {
            if (extension.equals(Utils.hex)){
                    return true;
            } else {
                return false;
            }
        }

        return false;
    }

    //The description of this filter
    public String getDescription() {
        return "hex-Files *.hex";
    }

}

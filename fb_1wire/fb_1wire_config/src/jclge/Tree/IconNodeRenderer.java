/*
 * Version 1.0
 * 14.06.2005
 */
package jclge.Tree;

import java.util.*;
import java.awt.*;
import javax.swing.*;
import javax.swing.tree.*;


/**
 * @author eichlger
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class IconNodeRenderer extends DefaultTreeCellRenderer {

	  /**
	 * 
	 */
	private static final long serialVersionUID = -2968763720942534259L;

	public Component getTreeCellRendererComponent(JTree tree, Object value,
	      boolean sel, boolean expanded, boolean leaf,
	      int row, boolean hasFocus) {
	      
	    super.getTreeCellRendererComponent(tree, value,
	       sel, expanded, leaf, row, hasFocus);
	       
	    Icon icon = ((IconNode)value).getIcon();
	    if (icon == null) {
	      Hashtable<?, ?> icons = (Hashtable<?, ?>)tree.getClientProperty("JTree.icons");
	      String name = ((IconNode)value).getIconName();
	      if ((icons != null) && (name != null)) {
	        icon = (Icon)icons.get(name);
	        if (icon != null) {
	          setIcon(icon);
	        }
	      }
	    } else {
	      setIcon(icon);
	    }
	    return this;
	  }
	}

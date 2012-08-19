/*
 * Version 1.0
 * 14.06.2005
 */
package jclge.Tree;

/**
 * @author eichlger
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
import javax.swing.*;
import javax.swing.tree.*;

public class IconNode extends DefaultMutableTreeNode  {

  /**
	 * 
	 */
	private static final long serialVersionUID = -8340548063958726837L;
protected Icon   icon = null;
  protected String iconName = null;

  public IconNode() {
    this(null);
  }

  public IconNode(Object userObject) {
    this(userObject, true, null);
  }

  public IconNode(Object userObject, boolean allowsChildren
		       , Icon icon) {
    super(userObject, allowsChildren);
    this.icon = icon;
  }



  public void setIcon(Icon icon) {
    this.icon = icon;
  }
  
  public Icon getIcon() {
    return icon;
  }
  
  public String getIconName() {
    String str = userObject.toString();
    int index = str.lastIndexOf(".");
    if (index != -1) {
      return str.substring(++index);
    } else {
      return iconName;
    }  
  }
  
  /*
  public String getIconName() {
    return iconName;
  }
  */
  
  public void setIconName(String name) {
    iconName = name;
  }

}
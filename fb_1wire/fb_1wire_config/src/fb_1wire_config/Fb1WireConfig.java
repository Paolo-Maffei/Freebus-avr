package fb_1wire_config;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.InvalidPropertiesFormatException;
import java.util.Properties;
import java.util.Vector;

import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JToolBar;
import javax.swing.JTree;
import javax.swing.KeyStroke;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeNode;
import javax.swing.tree.TreePath;

import fb_1wire_config.TreeData;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import jclge.Tree.IconNode;
import jclge.Tree.IconNodeRenderer;

public class Fb1WireConfig extends JPanel implements ActionListener{

	/**
	 * 
	 */
	private static final long serialVersionUID = 2943335573953379478L;
	private JPanel jContentPane = null;
	private JFrame jFrame = null;
	private JMenuBar jJMenuBar = null;
	private JMenu jMenu = null;
	private JMenuItem jMenuItem = null;
	private JMenuItem jMenuItem2 = null;
	private JMenuItem jMenuItem3 = null;
	private JToolBar jToolBar = null;
	private JButton jButtonNew = null;
	private JButton jButtonOpen = null;
	private JButton jButtonSave = null;
	private JButton jButtonSaveAs = null;
	private JButton jButtonExit = null;
	private Properties properties = new Properties();
	private JScrollPane jScrollPane1 = null;
	private JPanel panelCenter = null;
	private JTree jTree = null;
	private IconNode root;
	private DefaultTreeModel treeModel;
	private String frameTitle = "Freebus 1-Wire konfigurator V1.0 BY GE";
	private String hexFile = "";
	//settings
	private JTextField txtPysAddr = new JTextField();
	private JTextField txtNrSensors = new JTextField();
	private JComboBox cmbSendBasis = new JComboBox();
	//Sensor
	private JTextField txtSensorId = new JTextField();
	private JTextField txtSensorGrpNr = new JTextField();
	private JComboBox cmbSendCyclic = new JComboBox();
	private JTextField txtCyclicFactor = new JTextField();
	private JComboBox cmbSendChange = new JComboBox();
	private JTextField txtSendChangeValue = new JTextField();
	private Vector<SensorCfg> vSensor = new Vector<SensorCfg>();
	
	
	public Fb1WireConfig(){
		getJFrame();
	}
	private JPanel getJContentPane() {
		if (jContentPane == null) {
			BorderLayout borderLayout1 = new BorderLayout();
			jContentPane = new JPanel();
			jContentPane.setLayout(borderLayout1);
			borderLayout1.setHgap(5);
			borderLayout1.setVgap(5);
			jContentPane.add(getJToolBar(), java.awt.BorderLayout.NORTH);
			jContentPane.add(getJScrollPane1(),java.awt.BorderLayout.WEST);
			//jContentPane.add(getJPanelCenter(), java.awt.BorderLayout.CENTER);
			txtPysAddr.setText("0/0/0");
			txtNrSensors.setText("0");
			setTreeItems(Integer.parseInt(txtNrSensors.getText()));
			
			txtSensorId.setText("28:FA:3B:AF:03:00:00:80");
			txtSensorGrpNr.setText("0/3/0");
			
			
			cmbSendBasis.addItem("130ms");
			cmbSendBasis.addItem("260ms");
			cmbSendBasis.addItem("520ms");
			cmbSendBasis.addItem("1s");
			cmbSendBasis.addItem("2,1s");
			cmbSendBasis.addItem("4,2s");
			cmbSendBasis.addItem("8,4s");
			cmbSendBasis.addItem("17s");
			cmbSendBasis.addItem("34s");
			cmbSendBasis.addItem("1,1min");
			cmbSendBasis.addItem("2,2min");
			cmbSendBasis.addItem("4,5min");
			cmbSendBasis.addItem("9min");
			cmbSendBasis.addItem("18min");
			cmbSendBasis.addItem("35min");
			cmbSendBasis.addItem("1,2h");
			cmbSendBasis.setSelectedIndex(9);
			
			cmbSendCyclic.addItem("no");
			cmbSendCyclic.addItem("yes");
			cmbSendCyclic.setSelectedIndex(1);
			txtCyclicFactor.setText("1");
			cmbSendChange.addItem("no");
			cmbSendChange.addItem("yes");
			cmbSendChange.setSelectedIndex(0);
			txtSendChangeValue.setText("0.1");
			
		}
		return jContentPane;
	}
	private JFrame getJFrame (){
		if (jFrame == null) {
			jFrame = new JFrame();
			properties.clear();
			try {
				//properties.loadFromXML(new FileInputStream("main.properties"));
				properties.load(new FileInputStream("main.properties"));
			} catch (InvalidPropertiesFormatException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (FileNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			ImageIcon icon = new ImageIcon(getClass().getResource("/images/fb_logo_klein.gif"));
			jFrame.setIconImage(icon.getImage());
			jFrame.setContentPane(getJContentPane());
			jFrame.setJMenuBar(getJJMenuBar());
			jFrame.setTitle(frameTitle);
			jFrame.setSize(Integer.parseInt(properties.getProperty("windowWidth","800")) , Integer.parseInt(properties.getProperty("windowHeight","600")));
			jFrame.setLocation(Integer.parseInt(properties.getProperty("windowPosX","0")) , Integer.parseInt(properties.getProperty("windowPosY","0")));
			ExitWindow exit = new ExitWindow();
			jFrame.addWindowListener(exit);
			openLastProject();
		}
		return jFrame;
	}
	private void getPanelCenter(){
		if (panelCenter != null){
			this.jContentPane.remove(panelCenter);
			this.jContentPane.validate();
			panelCenter = null;
		}
		if (panelCenter == null){
			panelCenter = new JPanel();
		}
		if (jTree.getSelectionPath() != null){
			int yIndex = 0;
			boolean showOk = false;
			IconNode n1 = (IconNode) jTree.getSelectionPath().getLastPathComponent();
			TreeData td = (TreeData)n1.getUserObject();
			panelCenter.setLayout(new GridBagLayout());
			JButton btnOk = new JButton("OK");
			GridBagConstraints c = new GridBagConstraints();
			c.insets = new Insets(5,5,5,5);
			c.fill = GridBagConstraints.NONE;
			c.weighty = 0;
			c.weightx = 20;
			if (td.type == TreeData.SETTINGS){
				showOk = true;
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.EAST;
				c.gridx = 0;
				c.gridy = yIndex;
				panelCenter.add(new JLabel("physical Address"),c);
				c.fill = GridBagConstraints.HORIZONTAL;
				c.anchor = GridBagConstraints.WEST;
				c.gridx = 1;
				c.gridy = yIndex;
				panelCenter.add(txtPysAddr,c);
				yIndex++;
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.EAST;
				c.gridx = 0;
				c.gridy = yIndex;
				panelCenter.add(new JLabel("cyclic send basis"),c);
				c.fill = GridBagConstraints.HORIZONTAL;
				c.anchor = GridBagConstraints.WEST;
				c.gridx = 1;
				c.gridy = yIndex;
				panelCenter.add(cmbSendBasis,c);
				yIndex++;
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.EAST;
				c.gridx = 0;
				c.gridy = yIndex;
				panelCenter.add(new JLabel("Number of Sensors"),c);
				c.fill = GridBagConstraints.HORIZONTAL;
				c.anchor = GridBagConstraints.WEST;
				c.gridx = 1;
				c.gridy = yIndex;
				panelCenter.add(txtNrSensors,c);
				yIndex++;
				
				
			}else if (td.type == TreeData.SENSOR){
				SensorCfg sens = vSensor.get(td.Number);
				showOk = true;
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.EAST;
				c.gridx = 0;
				c.gridy = yIndex;
				panelCenter.add(new JLabel("SensorId"),c);
				c.fill = GridBagConstraints.HORIZONTAL;
				c.anchor = GridBagConstraints.WEST;
				c.gridx = 1;
				c.gridy = yIndex;
				panelCenter.add(txtSensorId,c);
				txtSensorId.setText(sens.id);
				yIndex++;
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.EAST;
				c.gridx = 0;
				c.gridy = yIndex;
				panelCenter.add(new JLabel("Grp-Address for Sensor"),c);
				c.fill = GridBagConstraints.HORIZONTAL;
				c.anchor = GridBagConstraints.WEST;
				c.gridx = 1;
				c.gridy = yIndex;
				panelCenter.add(txtSensorGrpNr,c);
				txtSensorGrpNr.setText(Utils.makeGrpAdrItoS(sens.grpNr));
				yIndex++;
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.EAST;
				c.gridx = 0;
				c.gridy = yIndex;
				panelCenter.add(new JLabel("send cyclic"),c);
				c.fill = GridBagConstraints.HORIZONTAL;
				c.anchor = GridBagConstraints.WEST;
				c.gridx = 1;
				c.gridy = yIndex;
				panelCenter.add(cmbSendCyclic,c);
				if (sens.sendCyclic){
					cmbSendCyclic.setSelectedIndex(1);
				}else{
					cmbSendCyclic.setSelectedIndex(0);
				}
				yIndex++;
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.EAST;
				c.gridx = 0;
				c.gridy = yIndex;
				panelCenter.add(new JLabel("factor for cyclic send (0..127)"),c);
				c.fill = GridBagConstraints.HORIZONTAL;
				c.anchor = GridBagConstraints.WEST;
				c.gridx = 1;
				c.gridy = yIndex;
				panelCenter.add(txtCyclicFactor,c);
				txtCyclicFactor.setText(String.valueOf(sens.cyclicFactor));
				yIndex++;
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.EAST;
				c.gridx = 0;
				c.gridy = yIndex;
				panelCenter.add(new JLabel("send by change"),c);
				c.fill = GridBagConstraints.HORIZONTAL;
				c.anchor = GridBagConstraints.WEST;
				c.gridx = 1;
				c.gridy = yIndex;
				panelCenter.add(cmbSendChange,c);
				if (sens.sendByChange){
					cmbSendChange.setSelectedIndex(1);
				}else{
					cmbSendChange.setSelectedIndex(0);
				}

				yIndex++;
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.EAST;
				c.gridx = 0;
				c.gridy = yIndex;
				panelCenter.add(new JLabel("diff for send by change [1/10°C]"),c);
				c.fill = GridBagConstraints.HORIZONTAL;
				c.anchor = GridBagConstraints.WEST;
				c.gridx = 1;
				c.gridy = yIndex;
				panelCenter.add(txtSendChangeValue,c);
				txtSendChangeValue.setText(String.valueOf(sens.sendByChangeFactor));
				yIndex++;
				
				
				
				
			}
			if (showOk){
				c.fill = GridBagConstraints.NONE;
				c.anchor = GridBagConstraints.CENTER;
				c.weightx = 0;
				c.weighty = 0;
				c.gridx = 0;
				c.gridy = yIndex;
				c.gridwidth = 2;
				btnOk.addActionListener( new ActionListener() {
			          public void actionPerformed(ActionEvent e) {
			  			IconNode n1 = (IconNode) jTree.getSelectionPath().getLastPathComponent();
						TreeData td = (TreeData)n1.getUserObject();
						if (td.type == TreeData.SETTINGS){
							setTreeItems(Integer.parseInt(txtNrSensors.getText()));
						}else if (td.type == TreeData.SENSOR){
							SensorCfg sens = vSensor.get(td.Number);
							sens.id = txtSensorId.getText();
							sens.grpNr = Utils.makeGrpAdrStoI(txtSensorGrpNr.getText());
							if (cmbSendCyclic.getSelectedIndex() == 0){
								sens.sendCyclic = false;
							}else{
								sens.sendCyclic = true;
							}
							sens.cyclicFactor = (byte)Integer.parseInt(txtCyclicFactor.getText());
							if (cmbSendChange.getSelectedIndex() == 0){
								sens.sendByChange = false;
							}else{
								sens.sendByChange = true;
							}
							sens.sendByChangeFactor = Integer.parseInt(txtSendChangeValue.getText());
						}
			          }
		        } );		
				panelCenter.add(btnOk,c);
			}
			this.jContentPane.add(panelCenter,BorderLayout.CENTER);		
			this.jContentPane.validate();
			yIndex++;
			
			
		}		
	}
	
	private JScrollPane getJScrollPane1() {
		if (jScrollPane1 == null) {
			jScrollPane1 = new JScrollPane();
			jScrollPane1.setPreferredSize(new Dimension(120, 100));
			jScrollPane1.setViewportView(getJTree());
		}
		return jScrollPane1;
	}
	private JTree getJTree() {
		if (jTree == null) {
			root = new IconNode("Root");
			treeModel = new DefaultTreeModel(root);
		    jTree = new JTree(treeModel);
			//jTree = new JTree(root);
			jTree.setRootVisible(false);	
			jTree.setShowsRootHandles(true);
			jTree.setVisible(true);
			jTree.setCellRenderer(new IconNodeRenderer());
			jTree.setEditable(false);
			jTree.addKeyListener(new java.awt.event.KeyListener(){
			    public void keyTyped(KeyEvent e) {
			    }
			    public void keyPressed(KeyEvent e) {
			    	if (e.getKeyCode() == KeyEvent.VK_ENTER){
			    		if (jTree.getSelectionPath().getLastPathComponent() != null){
		    				//showPicture(jTree.getSelectionPath().getLastPathComponent().toString());
			    			//System.out.println("Tree pressed " + jTree.getSelectionPath().getLastPathComponent().toString());
				    		
			    		}
			    	}
			    }
			    public void keyReleased(KeyEvent e) {
			    }
			});
			jTree.addMouseListener(new java.awt.event.MouseListener(){
				public void mousePressed(MouseEvent e) {
					//System.out.println("mousePressed");
					//getPanelCenter();
			            if (e.isPopupTrigger()) {
			            	//createPopupMenu(e);
		            	}
		            }

				    public void mouseReleased(MouseEvent e) {
						//System.out.println("mouseReleased");
			            if (e.isPopupTrigger()) {
			            	//createPopupMenu(e);
		            	}
				    }

				    public void mouseEntered(MouseEvent e) {
				    }

				    public void mouseExited(MouseEvent e) {
				    }

				    public void mouseClicked(MouseEvent e) {
				    	//if (e.getClickCount() == 2){
				    		if (jTree.getSelectionPath() != null){
					    		if (jTree.getSelectionPath().getLastPathComponent() != null){
					    			//System.out.println("Tree pressed " + jTree.getSelectionPath().getLastPathComponent().toString());
				    				//showPicture(jTree.getSelectionPath().getLastPathComponent().toString());
					    		}
				    		}
				    	//}
				    }

			});
			jTree.addTreeSelectionListener(new javax.swing.event.TreeSelectionListener() {
				public void valueChanged(javax.swing.event.TreeSelectionEvent e) {
					//System.out.println("valueChanged()"); // TODO Auto-generated Event stub valueChanged()
					getPanelCenter();
				}
			});
		}
		return jTree;
	}
	
	// If expand is true, expands all nodes in the tree.
	// Otherwise, collapses all nodes in the tree.
	public void expandAll(JTree tree, boolean expand) {
	    TreeNode root = (TreeNode) tree.getModel().getRoot();
	    // Traverse tree from root
	    expandAll(tree, new TreePath(root), expand);
	}

	private void expandAll(JTree tree, TreePath parent, boolean expand) {
	    // Traverse children
	    TreeNode node = (TreeNode) parent.getLastPathComponent();
	    if (node.getChildCount() >= 0) {
	        for (Enumeration<?> e = node.children(); e.hasMoreElements();) {
	            TreeNode n = (TreeNode) e.nextElement();
	            TreePath path = parent.pathByAddingChild(n);
	            expandAll(tree, path, expand);
	        }
	    }
	    // Expansion or collapse must be done bottom-up
	    if (expand) {
	        tree.expandPath(parent);
	    } else {
	        tree.collapsePath(parent);
	    }
	}	

	private JMenuBar getJJMenuBar() {
		if (jJMenuBar == null) {
			jJMenuBar = new JMenuBar();
			jJMenuBar.add(getJMenu());
		}
		return jJMenuBar;
	}
	private JMenu getJMenu() {
		if (jMenu == null) {
			jMenu = new JMenu();
			jMenu.setText("Datei");
			jMenu.setMnemonic(java.awt.event.KeyEvent.VK_D);
			//jMenu.setMnemonic(java.awt.event.KeyEvent.VK_F1);
			jMenu.setIcon(new ImageIcon(getClass().getResource("/images/exit.gif")));
			jMenu.add(getJMenuItem2());
			jMenu.add(getJMenuItem3());
			jMenu.add(getJMenuItem());
		}
		return jMenu;
	}
	private JMenuItem getJMenuItem() {
		if (jMenuItem == null) {
			jMenuItem = new JMenuItem();
			jMenuItem.setText("Beenden");
			jMenuItem.setMnemonic(KeyEvent.VK_B);
			jMenuItem.setAccelerator(KeyStroke.getKeyStroke("F1"));
			jMenuItem.setIcon(new ImageIcon(getClass().getResource("/images/exit.gif")));
			jMenuItem.addActionListener(new ActionListener() { 
				public void actionPerformed(ActionEvent e) {   
					jFrame.getToolkit().getSystemEventQueue().postEvent(new WindowEvent(jFrame, WindowEvent.WINDOW_CLOSING));
				}
			});
		}
		return jMenuItem;
	}
	private JMenuItem getJMenuItem2() {
		if (jMenuItem2 == null) {
			jMenuItem2 = new JMenuItem();
			jMenuItem2.setText("neues Projekt");
			jMenuItem2.setIcon(new ImageIcon(getClass().getResource("/images/new.gif")));
			jMenuItem2.setMnemonic(java.awt.event.KeyEvent.VK_N);
			jMenuItem2.addActionListener(new java.awt.event.ActionListener() { 
				public void actionPerformed(java.awt.event.ActionEvent e) {    
					newProject();
				}
			});
		}
		return jMenuItem2;
	}
	private JMenuItem getJMenuItem3() {
		if (jMenuItem3 == null) {
			jMenuItem3 = new JMenuItem();
			jMenuItem3.setText("Projekt öffnen");
			jMenuItem3.setIcon(new ImageIcon(getClass().getResource("/images/open.gif")));
			jMenuItem3.setMnemonic(java.awt.event.KeyEvent.VK_O);
			jMenuItem3.addActionListener(new java.awt.event.ActionListener() { 
				public void actionPerformed(java.awt.event.ActionEvent e) {    
					openProject();
				}
			});
		}
		return jMenuItem3;
	}
	private JToolBar getJToolBar() {
		if (jToolBar == null) {
			jToolBar = new JToolBar();
			jToolBar.add(getJButtonNew());
			jToolBar.add(getJButtonOpen());
			jToolBar.add(getJButtonSave());
			jToolBar.add(getJButtonSaveAs());
			jToolBar.add(getJButtonExit());
		}
		return jToolBar;
	}
	private JButton getJButtonNew() {
		if (jButtonNew == null) {
			jButtonNew = new JButton();
			jButtonNew.setToolTipText("create new config");
			jButtonNew.setIcon(new ImageIcon(getClass().getResource("/images/new.gif")));
			jButtonNew.addActionListener(new java.awt.event.ActionListener() { 
				public void actionPerformed(java.awt.event.ActionEvent e) {    
					newProject();
				}
			});
		}
		return jButtonNew;
	}
	private JButton getJButtonOpen() {
		if (jButtonOpen == null) {
			jButtonOpen = new JButton();
			jButtonOpen.setToolTipText("open existing config");
			jButtonOpen.setIcon(new ImageIcon(getClass().getResource("/images/open.gif")));
			jButtonOpen.addActionListener(new java.awt.event.ActionListener() { 
				public void actionPerformed(java.awt.event.ActionEvent e) {    
					openProject();
				}
			});
		}
		return jButtonOpen;
	}
	private JButton getJButtonSave() {
		if (jButtonSave == null) {
			jButtonSave = new JButton();
			jButtonSave.setToolTipText("save config to file");
			jButtonSave.setIcon(new ImageIcon(getClass().getResource("/images/save.gif")));
			jButtonSave.addActionListener(new java.awt.event.ActionListener() { 
				public void actionPerformed(java.awt.event.ActionEvent e) {    
					//TODO Save to File
					if (hexFile == ""){
						selectSaveFile();
					}
					if (hexFile != ""){
						saveHexFile();
					}
				}
			});
		}
		return jButtonSave;
	}
	
	private JButton getJButtonSaveAs() {
		if (jButtonSaveAs == null) {
			jButtonSaveAs = new JButton();
			jButtonSaveAs.setToolTipText("save config to file");
			jButtonSaveAs.setIcon(new ImageIcon(getClass().getResource("/images/saveas.gif")));
			jButtonSaveAs.addActionListener(new java.awt.event.ActionListener() { 
				public void actionPerformed(java.awt.event.ActionEvent e) {    
					//TODO Save to File
					if (selectSaveFile() == 0){
						saveHexFile();
					}
				}
			});
		}
		return jButtonSaveAs;
	}

	private JButton getJButtonExit() {
		if (jButtonExit == null) {
			jButtonExit = new JButton();
			jButtonExit.setToolTipText("close Window");
			jButtonExit.setIcon(new ImageIcon(getClass().getResource("/images/exit.gif")));
			jButtonExit.addActionListener(new java.awt.event.ActionListener() { 
				public void actionPerformed(java.awt.event.ActionEvent e) {    
					jFrame.getToolkit().getSystemEventQueue().postEvent(new WindowEvent(jFrame, WindowEvent.WINDOW_CLOSING));
				}
			});
		}
		return jButtonExit;
	}
	
	


	public static void main(String[] args) {
		Fb1WireConfig terminal = new Fb1WireConfig();
		terminal.jFrame.setVisible(true);
	}	
   	public void actionPerformed(ActionEvent evt) {

   	}
   	private void newProject(){
		//TODO new Project   		
   	}
   	private void openLastProject(){
   		String fileName = properties.getProperty("lastFile","");
   		if (!fileName.equals("")){
   			hexFile = fileName;
	        File f1 = new File(hexFile);
			try {
		        if (f1.exists()){
		        	loadHexFile();
		        	//TODO openDb(fileName,false);
		        }
			} catch (Exception e) {
				e.printStackTrace();
			}
   		}
   	}
   	private void openProject(){
		String fileName = "";
		JFileChooser chooser = new JFileChooser();
		chooser.setDialogType(JFileChooser.OPEN_DIALOG);
		chooser.setMultiSelectionEnabled(false);
		chooser.setAcceptAllFileFilterUsed(false);
		chooser.setDialogTitle("select hex File");
		chooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
		chooser.setCurrentDirectory(new File(properties.getProperty("lastPath","")));
		chooser.addChoosableFileFilter(new HexFilter());
		chooser.setFileFilter(new HexFilter());		
		int retval = chooser.showOpenDialog(this.jFrame);
		if(retval == JFileChooser.APPROVE_OPTION) {
			hexFile = chooser.getSelectedFile().getAbsoluteFile().toString(); 
			System.out.println("You chose to open this file: " + fileName);
			properties.setProperty("lastFile", hexFile);
			properties.setProperty("lastPath", chooser.getSelectedFile().getAbsolutePath().toString());
			loadHexFile();
		}
   	}
   	public class ExitWindow extends WindowAdapter {
   		public void windowClosing(WindowEvent e){
   			System.out.println("window closing");
			properties.setProperty("windowWidth", String.valueOf(jFrame.getWidth()));
			properties.setProperty("windowHeight", String.valueOf(jFrame.getHeight() ));
			properties.setProperty("windowPosX", String.valueOf(jFrame.getX() ));
			properties.setProperty("windowPosY", String.valueOf(jFrame.getY()));
			try {
				properties.store(new FileOutputStream("main.properties"), "FB_1_WIRE MAIN-CONFIG-File");
		    } catch (IOException e1) {
		    }
			jFrame.setSize(Integer.parseInt(properties.getProperty("windowWidth","800")) , Integer.parseInt(properties.getProperty("windowHeight","600")));
			jFrame.setLocation(Integer.parseInt(properties.getProperty("windowPosX","800")) , Integer.parseInt(properties.getProperty("windowPosY","0")));
   			System.exit(0);
   		}
   	}
   	private void setTreeItems(int sensorNumbers){
   		if (root != null){
   			root.removeAllChildren();
   			TreeData td = new TreeData("Settings",0,TreeData.SETTINGS);
   			IconNode node1 = null;
   			IconNode node2 = null;
   			node1 = new IconNode(td);
   			root.add(node1);
   			td = new TreeData("Sensors",0,TreeData.NULL);
   			node1 = new IconNode(td);
   			root.add(node1);
   			for (int i = 0;i < sensorNumbers;i++){
   	   			td = new TreeData("Sensor " + (i+1),i,TreeData.SENSOR);
   	   			node2 = new IconNode(td);
   	   			node1.add(node2);   				
   			}
   			jTree.setRootVisible(true);
   			expandAll(jTree,true);
   			treeModel.reload();
   			jTree.setRootVisible(false);
			int anz = sensorNumbers - vSensor.size();
			SensorCfg sens = null;
			if (anz < 0){
				anz *= -1;
				for (int i = 0;i < anz;i++){
					vSensor.remove(vSensor.size()-1);			
				}				
   			}else if (anz > 0){
				for (int i = 0;i < anz;i++){
	   				sens = new SensorCfg();
					vSensor.add(sens);			
				}
   			}
   		}
   	}
   	private int selectSaveFile(){
		JFileChooser chooser = new JFileChooser();
		chooser.setDialogType(JFileChooser.SAVE_DIALOG);
		chooser.setMultiSelectionEnabled(false);
		chooser.setAcceptAllFileFilterUsed(false);
		chooser.setDialogTitle("select hex File");
		chooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
		chooser.setCurrentDirectory(new File(properties.getProperty("lastPath","")));
		chooser.addChoosableFileFilter(new HexFilter());
		chooser.setFileFilter(new HexFilter());		
		int retval = chooser.showSaveDialog(this.jFrame);
		if(retval == JFileChooser.APPROVE_OPTION) {
			hexFile = chooser.getSelectedFile().getAbsoluteFile().toString();
			if (!hexFile.toLowerCase().endsWith(".hex")){
				hexFile += ".hex";
			}
			jFrame.setTitle(frameTitle + " " + hexFile);
			properties.setProperty("lastFile", hexFile);
			properties.setProperty("lastPath", chooser.getSelectedFile().getAbsolutePath().toString());
			System.out.println("You chose to open this file: " + hexFile);
			try {
				//TODO open hex-file
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			return 0;
		}else{
			return -1;
		}
   	}
   	private int loadHexFile(){
   		if (hexFile != ""){
			jFrame.setTitle(frameTitle + " " + hexFile);
			File f = new File( hexFile );
			if (f.length() != 512){
				return -1;
			}
			byte[] buffer = new byte[ (int) f.length() ];
			InputStream in;
			try {
				in = new FileInputStream( f );
				try {
					in.read(buffer);
					byte[] b1 = new byte[2];
					//physikal address
					b1[0] = buffer[23];
					b1[1] = buffer[24];
					txtPysAddr.setText(Utils.makeGrpAdrItoS(Utils.byteToInt(b1)));
					//cyclic send basis
					cmbSendBasis.setSelectedIndex((int)buffer[96] & 0x0F);
					//number of Group-adresses
					SensorCfg sens = null;
					int offset = 25;
					int anz = (int)buffer[97];
					if (anz == -1){
						anz = 0;
					}
					for (int i = 0;i < anz;i++){
						b1[0] = buffer[offset];
						b1[1] = buffer[offset+1];
						sens = new SensorCfg();
						sens.grpNr = Utils.byteToInt(b1);
						vSensor.add(sens);						
						offset+=2;
					}
					txtNrSensors.setText(String.valueOf(anz));
					setTreeItems(anz);
					int offs = 98;
					byte b2[] = new byte[8];
					for (int i = 0;i < vSensor.size();i++){
						sens = vSensor.elementAt(i);
						for (int j= 0;j < 8;j++){
							b2[j] = buffer[offs+j];
						}
						sens.id = Utils.sensorIdToString(b2);
						offs += 8;
						//cyclic send
						if (((int)buffer[offs] & 0x80) > 0){
							sens.sendCyclic = true;
						}else{
							sens.sendCyclic = false;
						}
						sens.cyclicFactor = (byte) (buffer[offs] & 0x7F); 
						offs += 8;
					}
					
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				try {
					in.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			} catch (FileNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}   
			return 0;
   		}else{
   			return -2;
   		}
   	}
   	
   	private void saveHexFile(){
   		if (hexFile != ""){
   	   		try {
   	   			byte[] buffer = new byte[ 512 ];
   	   			Arrays.fill(buffer, (byte)0xFF);
   	   			byte[] b1 = new byte[2];
				//Software Version
   	   			buffer[7] = 0x01;
   	   			
   	   			
   	   			//number of Group-adresses
				buffer[22] = (byte)(Integer.parseInt(txtNrSensors.getText()) + 1);
				//physikal address
   	   			b1 = Utils.intToByte(Utils.makeGrpAdrStoI(txtPysAddr.getText()));
				buffer[23] = b1[1];
				buffer[24] = b1[0];
				int offs = 25;
				txtPysAddr.setText(Utils.makeGrpAdrItoS(Utils.byteToInt(b1)));
				SensorCfg sens = null;
				for (int i = 0;i < vSensor.size();i++){
					sens = vSensor.elementAt(i);
					b1 = Utils.intToByte(sens.grpNr);
					buffer[offs]   = b1[1];
					buffer[offs+1] = b1[0];
					offs += 2;
				}
				//cyclic send basis
				buffer[96] = (byte) cmbSendBasis.getSelectedIndex();
				//count of Sensors
				buffer[97] = (byte)Integer.parseInt(txtNrSensors.getText());
				offs = 98;
				byte b2[] = new byte[8];
				for (int i = 0;i < vSensor.size();i++){
					sens = vSensor.elementAt(i);
					b2 = Utils.sensorIdToBytearray(sens.id);
					for (int j= 0;j < 8;j++){
						buffer[offs+j] = b2[j];
					}
					offs += 8;
					//cyclic send
					if (sens.sendCyclic){
						buffer[offs] = (byte)((byte)0x80 + sens.cyclicFactor);
					}else{
						buffer[offs] =  sens.cyclicFactor;
					}
					
					offs += 8;
				}

				System.out.println("test");
				FileOutputStream fos = new FileOutputStream(hexFile);
				fos.write(buffer);
   				fos.close();
   	   			
   	    	} catch (FileNotFoundException fnfe){
   	    		fnfe.printStackTrace();
   	    	} catch (IOException e) {
   				// TODO Auto-generated catch block
   				e.printStackTrace();
   			}
   			
   		}
   	}
	
}

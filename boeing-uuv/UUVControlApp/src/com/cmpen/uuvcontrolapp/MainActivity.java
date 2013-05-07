package com.cmpen.uuvcontrolapp;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.UnknownHostException;
import java.util.Timer;
import java.util.TimerTask;

import android.net.wifi.WifiConfiguration;
import android.os.Bundle;
import android.app.Activity;
import android.graphics.Color;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.support.v4.app.NavUtils;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.Toast;
import android.widget.Button;
import android.widget.TextView;
import android.widget.ToggleButton;



public class MainActivity extends Activity {
	
	private final String ipAddress = "192.168.1.136"; // Arduino IP
	private final int port = 1000;
	
	private final char OFF		= '0';
	private final char ON		= '1';
	private final char RISE	    = '2';
	private final char FALL	    = '3';
	private final char FORWARD  = '4';
	private final char BACKWARD = '5';
	private final char nRISE	 = '6';
	private final char nFALL	 = '7';
	private final char nFORWARD  = '8';
	private final char nBACKWARD = '9';
	
	private final int rDepth = 0;
	private final int rGyrX = 1;
	private final int rGyrY = 2;
	private final int rGyrZ = 3;
	private final int rAccX = 4;
	private final int rAccY = 5;
	private final int rAccZ = 6;
	
	
	ToggleButton onBtn;
	Button up, down, left, right;
	Button go, connect;
	EditText depth;
	TextView depthm, GXm, GYm, GZm, AXm, AYm, AZm;
	SeekBar seek;
	
	Socket kkSocket = null;
	PrintWriter out = null;
	BufferedReader in = null;
	
	boolean isOn = false, isConnected = false;
	Toast toast;
	char[] outputFromArduino = new char[30];
	String[] data = new String[7];
	Timer inputTimer;
	int delay = 0; // delay for 1 sec. 
    long period = 1000; // repeat every 10 sec. 
	

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        onBtn = (ToggleButton) findViewById(R.id.toggleButton1);
        onBtn.setOnClickListener(tbListener());
        
        up = (Button) findViewById(R.id.button1);
        up.setOnTouchListener(dPad());
        
        down = (Button) findViewById(R.id.button4);
        down.setOnTouchListener(dPad());
        
        left = (Button) findViewById(R.id.button2);
        left.setOnTouchListener(dPad());
        
        right = (Button) findViewById(R.id.button3);
        right.setOnTouchListener(dPad());
        
        go = (Button) findViewById(R.id.button5);
        go.setOnClickListener(goBtn());
        
        connect = (Button) findViewById(R.id.button6);
        connect.setOnClickListener(goBtn());
        
        depth = (EditText) findViewById(R.id.editText1);
        
        seek = (SeekBar)findViewById(R.id.seekBar1); 
        seek.setOnSeekBarChangeListener(slider());
        
        depthm = (TextView) findViewById(R.id.textView5);
        GXm = (TextView) findViewById(R.id.TextView02);
        GYm = (TextView) findViewById(R.id.TextView04);
        GZm = (TextView) findViewById(R.id.TextView05);
        AXm = (TextView) findViewById(R.id.textView4);
        AYm = (TextView) findViewById(R.id.textView6);
        AZm = (TextView) findViewById(R.id.textView8);
        
        
              
      
        inputTimer = new Timer();        
      
        
    }
    
    private void errorMessage(String msg) {
    	System.err.println("Error:" + msg);
    }
    

    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_main, menu);
        return true;
    }
    
    public void updateData() {
    	depthm.setText("Depth = " + data[rDepth] + " in.");
    	GXm.setText("Yaw   = " + data[rGyrX] + "\u00B0");
    	GYm.setText("Pitch = " + data[rGyrY] + "\u00B0");
    	GZm.setText("Roll  = " + data[rGyrZ] + "\u00B0");
    	AXm.setText("X = " + data[rAccX]);
    	AYm.setText("Y = " + data[rAccY]);
    	AZm.setText("Z = " + data[rAccZ]);
    }
    
    private void arduinoOutput(){
    	if (isConnected) {
    		try {
    			BufferedReader in = new BufferedReader( new InputStreamReader( kkSocket.getInputStream() ));
    			if (in.ready()) {
    				in.read(outputFromArduino);
    				System.out.println("DATA: " + String.valueOf(outputFromArduino));
    				data = String.valueOf(outputFromArduino).split(",");
    				//data[0] = String.copyValueOf(outputFromArduino, 0, 4);
    			}
    			else {
    				System.out.println("NO DATA");
    			}
    		} catch (IOException e) {
    			System.out.println("IOException");
    			e.printStackTrace();
    		}
    		
    	}
    	
    }
    
    private OnSeekBarChangeListener slider() {
    	return new OnSeekBarChangeListener() {
    		
    		@Override
			public void onStartTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}    	

			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				if (isConnected == true) {
    				try {
    					out = new PrintWriter(kkSocket.getOutputStream(), true);
    					out.println("s"+seekBar.getProgress());
    					 toast = Toast.makeText(getApplicationContext(),"Speed changed to " + seekBar.getProgress(), Toast.LENGTH_SHORT);
					     toast.show();
    					
    				} catch (IOException e) {
    					e.printStackTrace();
    				}
    			}
				
			}


			@Override
			public void onProgressChanged(SeekBar seekBar, int progress,
					boolean fromUser) {
				// TODO Auto-generated method stub
				
			}
    	};
    	
    }
    
    private OnTouchListener dPad() {
    	return new OnTouchListener() {

			@Override
			public boolean onTouch(View v, MotionEvent event) {
				if(v.getId() == left.getId() && isConnected == true) {
					if (event.getAction() == MotionEvent.ACTION_DOWN) {
						System.out.println("press left");
						try {
						out = new PrintWriter(kkSocket.getOutputStream(), true);
						out.println(BACKWARD);
					} catch (IOException e) {
						e.printStackTrace();
					}
					}
					else if (event.getAction() == MotionEvent.ACTION_UP) {
						System.out.println("release left");
						try {
						out = new PrintWriter(kkSocket.getOutputStream(), true);
						out.println(nBACKWARD);
					} catch (IOException e) {
						e.printStackTrace();
					}
					}
				}
				if(v.getId() == right.getId() && isConnected == true) {
					if (event.getAction() == MotionEvent.ACTION_DOWN) {
						System.out.println("press right");
						try {
						out = new PrintWriter(kkSocket.getOutputStream(), true);
						out.println(FORWARD);
					} catch (IOException e) {
						e.printStackTrace();
					}
					}
					else if (event.getAction() == MotionEvent.ACTION_UP) {
						System.out.println("release right");
						try {
						out = new PrintWriter(kkSocket.getOutputStream(), true);
						out.println(nFORWARD);
					} catch (IOException e) {
						e.printStackTrace();
					}
					}
				}
				if(v.getId() == up.getId() && isConnected == true) {
					if (event.getAction() == MotionEvent.ACTION_DOWN) {
						System.out.println("press rise");
						try {
						out = new PrintWriter(kkSocket.getOutputStream(), true);
						out.println(RISE);
					} catch (IOException e) {
						e.printStackTrace();
					}					
					
					}
					else if (event.getAction() == MotionEvent.ACTION_UP) {
						System.out.println("release rise");
						try {
						out = new PrintWriter(kkSocket.getOutputStream(), true);
						out.println(nRISE);
					} catch (IOException e) {
						e.printStackTrace();
					}
					}
				}
				else if(v.getId() == down.getId() && isConnected == true) {
					if (event.getAction() == MotionEvent.ACTION_DOWN) {
						System.out.println("press dive");
						try {
						out = new PrintWriter(kkSocket.getOutputStream(), true);
						out.println(FALL);
					} catch (IOException e) {
						e.printStackTrace();
					}
					}
					else if (event.getAction() == MotionEvent.ACTION_UP) {
						System.out.println("release dive");
						try {
						out = new PrintWriter(kkSocket.getOutputStream(), true);
						out.println(nFALL);
					} catch (IOException e) {
						e.printStackTrace();
					}
					}
				}
				return false;
			}};
    }
    
    private OnClickListener goBtn() {
    	return new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (v.getId() == connect.getId()) {
					if(isConnected == true) {  
						try {
							kkSocket.close();
							connect.setText("Disconnected");
						    connect.setTextColor(Color.RED);
						    isConnected = false;
							
						} catch (IOException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
						
					}
					
					else if(isConnected == false) {  
						try {
							 InetAddress addr = InetAddress.getByName(ipAddress);
							 SocketAddress sockaddr = new InetSocketAddress(addr, port);
							 kkSocket = new Socket();
					    	   kkSocket.connect(sockaddr, 2000);
					    	   //kkSocket = new Socket(ipAddress, port);
					    	   connect.setText("Connected");
						       connect.setTextColor(Color.GREEN);
						       toast = Toast.makeText(getApplicationContext(),"Connection established to UUV", Toast.LENGTH_SHORT);
						       toast.show();
						       isConnected = true;
						       
						       final Runnable runnableUpdateData = new Runnable() {
						           public void run() {
						               updateData();
						           }
						       };
						       
						       inputTimer.scheduleAtFixedRate(new TimerTask() 
					            { 						    	   
						    	   		public void run() 
						                { 
						                	arduinoOutput();  // display the data
						                	runOnUiThread(runnableUpdateData);
						                }					                
					            }, delay, period); 
					    	 
					       } catch (UnknownHostException e) {
					    	 errorMessage("Unknown host" + ipAddress);
					       } catch (IOException e) {
					    	errorMessage("Couldn't get I/O for the connection to: " + ipAddress);
					    	toast = Toast.makeText(getApplicationContext(),"Couldn't get I/O for the connection to UUV", Toast.LENGTH_LONG);
					    	toast.show();
					    	connect.setTextColor(Color.RED);
					       	}
					}
					
				      
				       
				}
				
				if (v.getId() == go.getId() && isConnected == true) {
					int depthInput = Integer.parseInt(depth.getText().toString());
					if (depthInput < 36 && depthInput >= 0) {
						try {
							out = new PrintWriter(kkSocket.getOutputStream(), true);
							out.println("d"+depthInput);						
						} catch (IOException e) {
							e.printStackTrace();						
						}
					}
					else {
						toast = Toast.makeText(getApplicationContext(),"Warning: Depth must be an integer between 0 and 36 inches", Toast.LENGTH_LONG);
					    toast.show();
					}
				}
				
			}};
    }
    
    private OnClickListener tbListener() {
    	return new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (v.getId() == onBtn.getId() && isOn == false  && isConnected == true) {
					isOn = true;
						        				
	        		try {
						out = new PrintWriter(kkSocket.getOutputStream(), true);
						out.println(ON);						
					} catch (IOException e) {
						e.printStackTrace();						
					}
				}
				else if (v.getId() == onBtn.getId() && isOn == true  && isConnected == true) {
					isOn = false;					
					try {
						out = new PrintWriter(kkSocket.getOutputStream(), true);
						out.println(OFF);						
					} catch (IOException e) {
						e.printStackTrace();
						}
				}
				
			}};
    }
    
}

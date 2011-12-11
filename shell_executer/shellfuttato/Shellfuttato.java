/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package shellfuttato;

import java.awt.GridLayout;
import java.awt.event.InputMethodEvent;
import java.awt.event.InputMethodListener;
import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.util.Scanner;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

/**
 *
 * @author adam
 */
public class Shellfuttato extends JFrame implements DocumentListener {
    JTextArea inputtext;
    JTextArea command;
    JTextArea output;
public Shellfuttato(){
    super();
    setBounds(0, 0, 1000, 600);
    setLayout(new GridLayout(1, 3));
    add(new JScrollPane(inputtext=new JTextArea()) );
    add(new JScrollPane(command=new JTextArea()) );
    add(new JScrollPane(output=new JTextArea()) );
  //  inputtext.addInputMethodListener(this);
  //  command.addInputMethodListener(this);
    inputtext.getDocument().addDocumentListener(this);
    command.getDocument().addDocumentListener(this);
    show();
    
}
    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        // TODO code application logic here
    new Shellfuttato();
    
    }

public static String readInputStreamAsString(InputStream in) 
    throws IOException {

    BufferedInputStream bis = new BufferedInputStream(in);
    ByteArrayOutputStream buf = new ByteArrayOutputStream();
    int result = bis.read();
    while(result != -1) {
      byte b = (byte)result;
      buf.write(b);
      result = bis.read();
    }        
    return buf.toString();
}
    
public void updateoutput()
{
    Thread th=new Thread(){
    public void run(){
        try {

            PrintWriter b=new PrintWriter(new FileWriter("progi"));
            b.print(command.getText());
            b.close();

            new File("progi").setExecutable(true);
            
            Process p=Runtime.getRuntime().exec("./progi");
       // p.getOutputStream().write(command.getText().getBytes());
       
        p.getOutputStream().write(inputtext.getText().getBytes());
      p.getOutputStream().close();
        p.waitFor();
         //System.err.println(63); 
         output.setText( readInputStreamAsString( p.getInputStream() ) );
        output.setText(output.getText()+readInputStreamAsString( p.getErrorStream() ) );
        
        } catch (InterruptedException ex) {
            Logger.getLogger(Shellfuttato.class.getName()).log(Level.SEVERE, null, ex);
        } catch (IOException ex) {
            Logger.getLogger(Shellfuttato.class.getName()).log(Level.SEVERE, null, ex);
        }
    } };
th.start();
}
    @Override
    public void insertUpdate(DocumentEvent de) {
    updateoutput();}
    @Override
    public void removeUpdate(DocumentEvent de) {
    updateoutput();}

    @Override
    public void changedUpdate(DocumentEvent de) {
    updateoutput();}

}

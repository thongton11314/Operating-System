import java.net.*;
import java.io.*;

public class DateServerMT {    
    public static void main(String[] args) {
        ServerSocket serSocket = null;
        System.out.println("Accessing into Server, MT");
        try {

            // create new server socket
            serSocket = new ServerSocket(Integer.parseInt(args[0]));

            // tracking requests time
            int count = 0;

            // runrun
            while (true)
            {
                
                // get request
                Socket client = serSocket.accept();
                
                // create track
                WorkerMT worker = new WorkerMT(client);
                System.out.println("Client request frequency:" + count);
                System.out.println(" - Local port:" + client.getLocalPort());
                System.out.println(" - Remote Sock Adress:" + client.getRemoteSocketAddress());
                count++;
                
                // put it in pool
                Thread thread = new Thread(worker);
                thread.start();              
            }
        }
        catch (IOException ioe) {
            System.err.println(ioe);
        }
        finally {
            try {

                // close socket
                serSocket.close();
                System.out.println("Closing Server");
            }
            catch (IOException ioe) {
                System.err.println(ioe);
            }
        }
    }
}

class WorkerMT implements Runnable {
    Socket socket;

    public WorkerMT(Socket socket) {
        this.socket = socket;
    }

    public void run() {
        try {

            // write the Date to the socket 
            PrintWriter pout = new PrintWriter(socket.getOutputStream(), true);
            
            // listening for connections 
            pout.println(new java.util.Date().toString());

            // close the socket and resume 
            socket.close();
        } catch (Exception ex) {
            System.err.println("Error: " + ex);
        }
    }
}
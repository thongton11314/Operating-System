import java.net.*;
import java.io.*;
import java.util.concurrent.*;

public class DateServerMTP
{ 
    private static final int SIZE = 50;
    public static void main(String[] args) {
        ServerSocket serSocket = null;
        System.out.println("Accessing into Server, MTP");
        try {

            // create new server socket 6013
            serSocket = new ServerSocket(Integer.parseInt(args[0]));

            // create a pool, Maximum 50 thread will be active to process task
            ExecutorService executorService = Executors.newFixedThreadPool(SIZE);

            // tracking requests time
            int count = 0;

            // run
            while(true) {

                // get request
                Socket client = serSocket.accept();     

                // create track
                WorkerMTP worker = new WorkerMTP(client);
                System.out.println("Client request frequency:" + count);
                System.out.println(" - Local port:" + client.getLocalPort());
                System.out.println(" - Remote Sock Adress:" + client.getRemoteSocketAddress());
                count++;

                // put it in pool
                executorService.execute(worker);
            }
        } 
        catch(IOException ex) {
            System.err.println("Error:" + ex);
        }
        finally {

            try {

                // close socket
                serSocket.close();
                System.out.println("Closing Server");
            }
            catch(IOException ex) {
                System.err.println("Error:" + ex);
            }
        }
    }
}

class WorkerMTP implements Runnable {
    Socket socket;

    public WorkerMTP(Socket socket) {
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
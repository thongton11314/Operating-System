/**
 * Created by rtdimpsey on 4/13/17.
 */
import java.net.*;
import java.io.*;

public class DateServer430
{
    public static void main(String[] args)
    {
        try
        {
            ServerSocket sock = new ServerSocket(6013);
            while (true)
            {
                /* now listen for connections */
                Socket client = sock.accept();

                /* write the Date to the socket */
                PrintWriter pout = new PrintWriter(client.getOutputStream(), true);

                /* close the socket and resume */
                /* listening for connections */
                pout.println(new java.util.Date().toString());
                client.close();
            }
        }
        catch (IOException ie) {
            System.err.println(ie);
        }
    }
}

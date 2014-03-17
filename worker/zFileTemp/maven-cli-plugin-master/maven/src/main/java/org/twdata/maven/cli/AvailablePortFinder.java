package org.twdata.maven.cli;

import java.io.IOException;
import java.net.ServerSocket;

/**
 * @since version
 */
public class AvailablePortFinder
{
    public static final int MIN_PORT = 1100;
    public static final int MAX_PORT = 49151;

    public static boolean available(int port) throws IllegalArgumentException
    {
        if (port < MIN_PORT || port > MAX_PORT)
        {
            throw new IllegalArgumentException("Invalid port: " + port);
        }

        ServerSocket ss = null;
        try
        {
            ss = new ServerSocket(port);
            ss.setReuseAddress(true);
            return true;
        }
        catch (IOException e)
        {
            // Do nothing
        }
        finally
        {

            if (ss != null)
            {
                try
                {
                    ss.close();
                }
                catch (IOException e)
                {
                    /* should not be thrown */
                }
            }
        }

        return false;
    }

    public static synchronized int getPortOrNextAvailable(int oldPort, int fromPort) throws Exception
    {
        if(available(oldPort))
        {
            return oldPort;
        }

        for (int i = fromPort; i <= MAX_PORT; i++) {
            if (available(i)) {
                return i;
            }
        }

        ServerSocket ss = null;
        try
        {
            ss = new ServerSocket(0);
            ss.setReuseAddress(true);
            return ss.getLocalPort();

        }catch (IOException e)
        {
            // Do nothing
        }
        finally
        {

            if (ss != null)
            {
                try
                {
                    ss.close();
                }
                catch (IOException e)
                {
                    /* should not be thrown */
                }
            }
        }

        throw new Exception("Could not find an available port above " + fromPort);
    }

}

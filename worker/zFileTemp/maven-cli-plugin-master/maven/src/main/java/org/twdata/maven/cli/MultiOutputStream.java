package org.twdata.maven.cli;

import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;


/**
 * @since version
 */
public class MultiOutputStream extends OutputStream
{
    List<OutputStream> outputStreams;

    public MultiOutputStream(OutputStream... outputStreams)
    {
        this.outputStreams = new ArrayList<OutputStream>(Arrays.asList(outputStreams));
    }

    @Override
    public void write(int b) throws IOException
    {
        for (int i=0; i<outputStreams.size(); i++)
        {
            OutputStream out = outputStreams.get(i);
            if(out.getClass().isAssignableFrom(PrintStream.class))
            {
                if(!checkAndWrite((PrintStream) out, b))
                {
                    outputStreams.remove(out);
                }
            }
            else
            {
                out.write(b);
            }
        }
    }

    @Override
    public void write(byte[] b) throws IOException
    {
        for (int i=0; i<outputStreams.size(); i++)
        {
            OutputStream out = outputStreams.get(i);
            if(out.getClass().isAssignableFrom(PrintStream.class))
            {
                if(!checkAndWrite((PrintStream) out, b))
                {
                    outputStreams.remove(out);
                }
            }
            else
            {
                out.write(b);
            }
        }
    }

    @Override
    public void write(byte[] b, int off, int len) throws IOException
    {
        for (int i=0; i<outputStreams.size(); i++)
        {
            OutputStream out = outputStreams.get(i);
            if(out.getClass().isAssignableFrom(PrintStream.class))
            {
                if(!checkAndWrite((PrintStream) out, b, off, len))
                {
                    outputStreams.remove(out);
                }
            }
            else
            {
                out.write(b, off, len);
            }
        }
    }
    
    @Override
    public void flush() throws IOException
    {
        for (OutputStream out: outputStreams)
        {
            out.flush();
        }
    }

    @Override
    public void close() throws IOException
    {
        for (OutputStream out: outputStreams)
        {
            out.close();
        }
    }
    
    private boolean checkAndWrite(PrintStream stream, int b)
    {
        if(stream.checkError())
        {
            return false;
        }
        else
        {
            stream.write(b);
            return true;
        }
    }

    private boolean checkAndWrite(PrintStream stream, byte[] b) throws IOException
    {
        if(stream.checkError())
        {
            return false;
        }
        else
        {
            stream.write(b);
            return true;
        }
    }

    private boolean checkAndWrite(PrintStream stream, byte[] b, int off, int len) throws IOException
    {
        if(stream.checkError())
        {
            return false;
        }
        else
        {
            stream.write(b,off,len);
            return true;
        }
    }
    
    public void addStream(OutputStream stream)
    {
        outputStreams.add(stream);
    }
    
    public void removeStream(OutputStream stream)
    {
        if(outputStreams.contains(stream))
        {
            outputStreams.remove(stream);
        }
    }
}

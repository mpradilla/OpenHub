package org.twdata.maven.cli;

import java.lang.reflect.Proxy;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;

/**
 * Ctrl-C signal handler that has no binary dependence on undocumented sun misc classes
 *
 * This handler conditionally supervises the thread passed to it, interrupting the thread if Ctrl-C is pressed.  If
 * not in the supervising state, it will call System.exit(1).
 *
 * If sun.misc.Signal doesn't exist, this class does nothing.
 */
public class CtrlCSignalHandler
{
    private volatile boolean supervising;
    private final Thread supervisedThread;

    public CtrlCSignalHandler()
    {
        this(Thread.currentThread());
    }

    public CtrlCSignalHandler(Thread supervisedThread)
    {
        this.supervisedThread = supervisedThread;
        createAndRegisterSignalHandler();
    }

    private Object createAndRegisterSignalHandler()
    {
        // Check that sun.misc.SignalHandler and sun.misc.Signal exists
        try
        {
            Class<?> signalClass = Class.forName("sun.misc.Signal");
            Class<?> signalHandlerClass = Class.forName("sun.misc.SignalHandler");
            // Implement signal handler
            Object signalHandler = Proxy.newProxyInstance(getClass().getClassLoader(),
                new Class<?>[]{signalHandlerClass}, new InvocationHandler()
                {
                    public Object invoke(Object proxy, Method method, Object[] args) throws Throwable
                    {
                        // only method we are proxying is handle()
                        if (supervising && !supervisedThread.isInterrupted())
                        {
                            supervisedThread.interrupt();
                        }
                        else
                        {
                            System.exit(1);
                        }
                        return null;
                    }
                });
            // Register the signal handler, this code is equivalent to:
            // Signal.handle(new Signal("INT"), signalHandler);
            signalClass.getMethod("handle", signalClass, signalHandlerClass).invoke(null, signalClass.getConstructor(
                String.class).newInstance("INT"), signalHandler);

        }
        catch (ClassNotFoundException cnfe)
        {
            // sun.misc Signal handler classes don't exist
        }
        catch (Exception e)
        {
            // Ignore this one too, if the above failed, the signal API is incompatible with what we're expecting
        }
        return null;
    }

    /**
     * Start supervising the thread
     */
    public void startSupervising()
    {
        supervising = true;
    }

    /**
     * Stop supervising the theard
     */
    public void stopSupervising()
    {
        supervising = false;
    }
}

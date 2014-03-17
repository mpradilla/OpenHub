package org.twdata.maven.cli.console;

import java.io.IOException;
import java.io.InputStream;
import java.io.PrintStream;
import java.io.OutputStreamWriter;
import java.net.SocketException;
import jline.Completor;
import jline.ConsoleReader;
import org.apache.maven.plugin.logging.Log;

public class JLineCliConsole implements CliConsole {
    private final ConsoleReader consoleReader;
    private final Log logger;

    public JLineCliConsole(InputStream in, PrintStream out, Log logger, Completor completor,
            String prompt) {
        try {
            consoleReader = new ConsoleReader(in, new OutputStreamWriter(out));
            consoleReader.setBellEnabled(false);
            consoleReader.setDefaultPrompt(prompt + "> ");
            this.logger = logger;
            consoleReader.addCompletor(completor);
        } catch (IOException ex) {
            throw new RuntimeException("Unable to create reader to read commands.", ex);
        }
    }

    public String readLine() {
        try {
            return consoleReader.readLine();
        } catch (SocketException ex) {
            return null;
        } catch (IOException ex) {
            throw new RuntimeException("Unable to read command.", ex);
        }
    }

    public void writeInfo(String info) {
        logger.info(info);
    }

    public void writeError(String error) {
        logger.error(error);
    }

    public void writeDebug(String debug) {
        logger.debug(debug);
    }
}

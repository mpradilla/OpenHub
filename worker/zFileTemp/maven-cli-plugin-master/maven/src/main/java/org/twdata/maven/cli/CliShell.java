package org.twdata.maven.cli;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.List;
import org.codehaus.plexus.util.StringUtils;
import org.twdata.maven.cli.commands.Command;
import org.twdata.maven.cli.console.CliConsole;

public class CliShell {
    private final List<Command> commands;
    private final CliConsole console;

    public CliShell(List<Command> commands, CliConsole console) {
        this.commands = commands;
        this.console = console;
    }

    public void run() {
        console.writeInfo("Waiting for commands...");
        CtrlCSignalHandler ctrlCSignalHandler = new CtrlCSignalHandler();

        String line;
        while ((line = console.readLine()) != null) {
            ctrlCSignalHandler.startSupervising();
            try {
                if (StringUtils.isEmpty(line)) {
                    continue;
                } else if (interpretCommand(line.trim().replaceAll(" {2,}", " ")) == false) {
                    break;
                }
            } catch (Exception ex) {
                StringWriter exMsg = new StringWriter();
                ex.printStackTrace(new PrintWriter(exMsg));
                console.writeError("Unable to complete running command: " + line + "\n"
                        + exMsg.toString());
            }
            finally {
                ctrlCSignalHandler.stopSupervising();
            }
        }
    }

    private boolean interpretCommand(String request) {
        for (Command command : commands) {
            if (command.matchesRequest(request)) {

                return command.run(request, console);
            }
        }

        console.writeError("Invalid command: " + request);
        return true;
    }
}

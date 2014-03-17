package org.twdata.maven.cli.commands;

import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import org.twdata.maven.cli.CommandTokenCollector;
import org.twdata.maven.cli.console.CliConsole;

public class HelpCommand implements Command {
    private final Set<String> helpCommands;
    private final List<Command> availableCommands;

    public HelpCommand(List<Command> availableCommands) {
        this.availableCommands = availableCommands;

        HashSet<String> helpCommandTokens = new HashSet<String>();
        helpCommandTokens.add("help");
        helpCommandTokens.add("?");
        helpCommands = Collections.unmodifiableSet(helpCommandTokens);
    }

    public void describe(CommandDescription description) {
        description.describeCommandName("Help commands")
                .describeCommandToken("help, ?", null);
    }

    public void collectCommandTokens(CommandTokenCollector collector) {
        collector.addCommandTokens(helpCommands);
    }

    public boolean matchesRequest(String request) {
        return helpCommands.contains(request);
    }

    public boolean run(String request, CliConsole console) {
        CliConsoleCommandDescription description = new CliConsoleCommandDescription(console);

        for (Command command : availableCommands) {
            command.describe(description);
        }

        description.outputDescription();
        return true;
    }
}

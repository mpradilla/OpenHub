package org.twdata.maven.cli.commands;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import org.twdata.maven.cli.CommandTokenCollector;
import org.twdata.maven.cli.console.CliConsole;

public class ListProjectsCommand implements Command {
    private final Set<String> listCommands;

    private final Set<String> projectNames;

    public ListProjectsCommand(Set<String> projectNames) {
        this.projectNames = projectNames;

        Set<String> commands = new HashSet<String>();
        commands.add("list");
        commands.add("ls");
        listCommands = Collections.unmodifiableSet(commands);
    }

    public void describe(CommandDescription description) {
        description.describeCommandName("List module commands")
                .describeCommandToken("list, ls", null);
    }

    public void collectCommandTokens(CommandTokenCollector collector) {
        collector.addCommandTokens(listCommands);
    }

    public boolean matchesRequest(String request) {
        return listCommands.contains(request);
    }

    public boolean run(String request, CliConsole console) {
        console.writeInfo("Listing available projects: ");

        for (String projectName : projectNames) {
            console.writeInfo("* " + projectName);
        }

        return true;
    }
}

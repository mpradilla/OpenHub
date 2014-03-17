package org.twdata.maven.cli.commands;

public interface CommandDescription {
    CommandDescription describeCommandName(String commandName);
    CommandDescription describeCommandToken(String token, String description);
}

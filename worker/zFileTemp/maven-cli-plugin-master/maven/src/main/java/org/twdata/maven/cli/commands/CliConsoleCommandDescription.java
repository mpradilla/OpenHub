package org.twdata.maven.cli.commands;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.TreeMap;
import org.twdata.maven.cli.console.CliConsole;

public class CliConsoleCommandDescription implements CommandDescription {
    private final CliConsole console;
    private final StringBuilder description = new StringBuilder();
    private final TreeMap<String, String> tokenDescriptions = new TreeMap<String, String>();

    public CliConsoleCommandDescription(CliConsole console) {
        this.console = console;
    }

    public CommandDescription describeCommandName(String commandName) {
        if (!tokenDescriptions.isEmpty()) {
            flushTokenDescriptions();
        }
        description.append("\n").append(commandName).append(":\n");
        return this;
    }

    private void flushTokenDescriptions() {
        int maxTokenLength = determineLongestTokenLength();
        String formattedDescription = formatTokenDescriptions(maxTokenLength);
        description.append(formattedDescription);
        tokenDescriptions.clear();
    }

    private int determineLongestTokenLength() {
        int longestTokenLength = 0;
        for (String token : tokenDescriptions.keySet()) {
            longestTokenLength = Math.max(longestTokenLength, token.length());
        }

        return longestTokenLength;
    }

    private String formatTokenDescriptions(int maxTokenLength) {
        StringWriter stringWriter = new StringWriter();
        PrintWriter out = new PrintWriter(stringWriter);

        for (String token : tokenDescriptions.keySet()) {
            out.printf(" %-" + maxTokenLength + "s  %s%n", token, tokenDescriptions.get(token));
        }

        out.flush();
        out.close();
        return stringWriter.toString();
    }

    public CommandDescription describeCommandToken(String token, String description) {
        if (description == null) {
            tokenDescriptions.put(token, "");
        } else {
            tokenDescriptions.put(token, description);
        }

        return this;
    }

    public void outputDescription() {
        flushTokenDescriptions();
        console.writeInfo(description.toString());
    }
}

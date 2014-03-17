package org.twdata.maven.cli;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

public class CommandTokenCollector {
    private final HashSet<String> validTokens = new HashSet<String>();

    public void addCommandTokens(Set<String> tokens) {
        validTokens.addAll(tokens);
    }

    public Set<String> getCollectedTokens() {
        for (Iterator<String> i = validTokens.iterator(); i.hasNext(); ) {
            if (i.next().startsWith("-")) {
                i.remove();
            }
        }

        return validTokens;
    }
}

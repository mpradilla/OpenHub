package org.twdata.maven.cli.commands;

import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import org.apache.maven.project.MavenProject;
import org.codehaus.plexus.util.StringUtils;
import org.twdata.maven.cli.PhaseCall;

public class PhaseCallBuilder {
    private final MavenProject defaultProject;
    private final Map<String, MavenProject> modules;
    private final Map<String, String> userAliases;
    private final boolean ignoreFailures;

    public PhaseCallBuilder(MavenProject project, Map<String, MavenProject> modules,
        Map<String, String> userAliases, boolean ignoreFailures) {
        defaultProject = project;
        this.modules = modules;
        this.userAliases = userAliases;
        this.ignoreFailures = ignoreFailures;
    }

    public List<PhaseCall> parseCommand(String text) {
        List<PhaseCall> phases = new ArrayList<PhaseCall>();
        PhaseCall currentPhaseCall = null;

        for (String token : resolveUserAliases(text)) {
            if (modules.containsKey(token)) {
                currentPhaseCall = addProject(phases, currentPhaseCall,
                        modules.get(token));
            } else if (token.startsWith("-D")) {
                addProperty(phases, currentPhaseCall, token);
            } else if (token.contains("*")) {
                String regexToken = token.replaceAll("\\*", ".*");
                for (String moduleName : modules.keySet()) {
                    if (Pattern.matches(regexToken, moduleName)) {
                        currentPhaseCall = addProject(phases,
                                currentPhaseCall, modules.get(moduleName));
                    }
                }
            } else if (token.equals("-o")) {
                goOffline(phases, currentPhaseCall);
            } else if (token.equals("-N")) {
                disableRecursive(phases, currentPhaseCall);
            } else if (token.equals("-S")) {
                addProperty(phases, currentPhaseCall, "-Dmaven.test.skip=true");
            } else if (token.startsWith("-P")) {
                addProfile(phases, currentPhaseCall, token);
            } else {
                currentPhaseCall = addPhase(phases, currentPhaseCall,
                        token);
            }
        }

        return phases;
    }

    private List<String> resolveUserAliases(String text) {
        List<String> result = new ArrayList<String>();

        for (String token : splitByWhitespaceEscapingQuotes(text)) {
            if ("".equals(token)) {
                continue;
            }
            if (userAliases.containsKey(token)) {
                result.addAll(resolveUserAliases(userAliases.get(token)));
            } else if (!StringUtils.isEmpty(token)) {
                result.add(token);
            }
        }

        return result;
    }

    private Iterable<String> splitByWhitespaceEscapingQuotes(String text) {
        return new CommandTokenizer(text);
    }

    private PhaseCall addProject(List<PhaseCall> phases, PhaseCall currentPhaseCall, MavenProject project) {
        if (currentPhaseCall == null || !currentPhaseCall.getPhases().isEmpty()) {
            currentPhaseCall = new PhaseCall(ignoreFailures);
            phases.add(currentPhaseCall);
        }
        currentPhaseCall.addProject(project);
        return currentPhaseCall;
    }

    private PhaseCall addPhase(List<PhaseCall> phases, PhaseCall currentPhaseCall, String phase) {
        if (currentPhaseCall == null) {
            currentPhaseCall = new PhaseCall(ignoreFailures);
            currentPhaseCall.addProject(defaultProject);
            phases.add(currentPhaseCall);
        }
        currentPhaseCall.addPhase(phase);
        return currentPhaseCall;
    }

    private void disableRecursive(List<PhaseCall> phases, PhaseCall currentPhaseCall) {
        if (currentPhaseCall == null) {
            currentPhaseCall = new PhaseCall(ignoreFailures);
            phases.add(currentPhaseCall);
        }
        currentPhaseCall.doNotRecurse();
    }

    private void goOffline(List<PhaseCall> phases, PhaseCall currentPhaseCall) {
        if (currentPhaseCall == null) {
            currentPhaseCall = new PhaseCall(ignoreFailures);
            phases.add(currentPhaseCall);
        }
        currentPhaseCall.goOffline();
    }

    private void addProfile(List<PhaseCall> phases, PhaseCall currentPhaseCall, String profile) {
        if (profile.length() < 3) {
            return;
        }

        if (currentPhaseCall == null) {
            currentPhaseCall = new PhaseCall(ignoreFailures);
            phases.add(currentPhaseCall);
        }

        profile = profile.substring(2);
        currentPhaseCall.addProfile(profile);
    }

    private void addProperty(List<PhaseCall> phases, PhaseCall currentPhaseCall, String property) {
        if (property.length() < 3 || hasNoKeyValue(property)) {
            return;
        }

        if (currentPhaseCall == null) {
            currentPhaseCall = new PhaseCall(ignoreFailures);
            phases.add(currentPhaseCall);
        }
        property = property.substring(2);
        String key = property;
        String value = "1";
        if (property.indexOf("=") >= 0) {
            String[] propertyTokens = property.split("=", 2);
            key = propertyTokens[0];
            if (propertyTokens.length > 1) {
                value = propertyTokens[1];
            }
        }
        currentPhaseCall.addProperty(key, value);
    }

    private boolean hasNoKeyValue(String property) {
        return property.charAt(2) == '=' || !property.contains("=")
                || property.endsWith("=");
    }
}

package org.twdata.maven.cli;

import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import org.apache.maven.project.MavenProject;
import org.twdata.maven.cli.console.CliConsole;

public class PhaseCall {
    private final boolean ignoreFailures;

    private final List<String> phases = new ArrayList<String>();
    private final List<String> profiles = new ArrayList<String>();
    private final List<MavenProject> projects = new ArrayList<MavenProject>();
    private final Properties properties = new Properties();
    private boolean offline = false;
    private boolean recursive = true;

    public PhaseCall(boolean ignoreFailures)
    {
        this.ignoreFailures = ignoreFailures;
    }

    public void addProject(MavenProject project) {
        projects.add(project);
    }

    public List<String> getPhases() {
        return phases;
    }

    public void addPhase(String phase) {
        phases.add(phase);
    }

    public List<String> getProfiles() {
        return profiles;
    }

    public void addProfile(String profile) {
        profiles.add(profile);
    }

    public Properties getProperties() {
        return properties;
    }

    public void addProperty(String property, String value) {
        properties.put(property, value);
    }

    public boolean isOffline() {
        return offline;
    }

    public void goOffline() {
        offline = true;
    }

    public boolean isRecursive() {
        return recursive;
    }

    public void doNotRecurse() {
        recursive = false;
    }

    public boolean run(PhaseCallRunner runner, CliConsole console) {
        for (MavenProject project : projects) {
            boolean success = runner.run(project, this, console);
            if (!(ignoreFailures || success))
            {
                return false;
            }
        }
        return true;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (MavenProject project : projects) {
            sb.append("project: ").append(project.getArtifactId()).append(" ");
        }
        for (String phase : phases) {
            sb.append("phase: ").append(phase).append(" ");
        }
        for (String profile : profiles) {
            sb.append("profile: ").append(profile).append(" ");
        }
        for (Object propName : properties.keySet()) {
            sb.append("property: ").append(propName).append("=").append(properties.get(propName)).append(" ");
        }
        return sb.toString();
    }
}

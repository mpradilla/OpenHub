package org.twdata.maven.cli;

import java.util.List;
import org.apache.maven.execution.MavenSession;
import org.apache.maven.model.Plugin;
import org.apache.maven.plugin.MojoExecutionException;
import org.apache.maven.plugin.PluginManager;
import org.apache.maven.project.MavenProject;
import org.twdata.maven.mojoexecutor.MojoExecutor;

import static org.twdata.maven.mojoexecutor.MojoExecutor.artifactId;
import static org.twdata.maven.mojoexecutor.MojoExecutor.configuration;
import static org.twdata.maven.mojoexecutor.MojoExecutor.executeMojo;
import static org.twdata.maven.mojoexecutor.MojoExecutor.executionEnvironment;
import static org.twdata.maven.mojoexecutor.MojoExecutor.goal;
import static org.twdata.maven.mojoexecutor.MojoExecutor.groupId;
import static org.twdata.maven.mojoexecutor.MojoExecutor.plugin;
import static org.twdata.maven.mojoexecutor.MojoExecutor.version;

public class MojoCall {
    private final String groupId;
    private final String artifactId;
    private final String goal;

    public MojoCall(String groupId, String artifactId, String goal) {
        this.groupId = groupId;
        this.artifactId = artifactId;
        this.goal = goal;
    }

    public void run(MavenProject project, MavenSession session, MojoExecutor.ExecutionEnvironment executionEnvironment)
            throws MojoExecutionException {
        executeMojo(
                plugin(groupId(groupId), artifactId(artifactId), version(getVersion(project))),
                goal(goal),
                configuration(),
                executionEnvironment);
    }

    /**
     * Tries to determine what version of the plugin has been already
     * configured for this project. If unknown, "RELEASE" is used.
     *
     * @param project The maven project
     * @return The discovered plugin version
     */
    private String getVersion(MavenProject project) {
        String version = null;

        @SuppressWarnings("unchecked")
        List<Plugin> plugins = project.getBuildPlugins();

        for (Plugin plugin : plugins) {
            if (groupId.equals(plugin.getGroupId())
                    && artifactId.equals(plugin.getArtifactId())) {
                version = plugin.getVersion();
                break;
            }
        }

        if (version == null) {
            plugins = project.getPluginManagement().getPlugins();
            for (Plugin plugin : plugins) {
                if (groupId.equals(plugin.getGroupId())
                        && artifactId.equals(plugin.getArtifactId())) {
                    version = plugin.getVersion();
                    break;
                }
            }
        }

        if (version == null) {
            version = "RELEASE";
        }
        return version;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(groupId).append(":").append(artifactId);
        sb.append(" [").append(goal).append("]");
        return sb.toString();
    }
}

package org.twdata.maven.cli;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;
import org.apache.maven.plugin.AbstractMojo;
import org.apache.maven.plugin.MojoExecutionException;

/**
 * Installs an IDEA plugin that sends commands to a listening CLI port
 *
 * @requiresDependencyResolution execute
 * @aggregator true
 * @goal idea
 * @requiresProject false
 */
public class IdeaMojo extends AbstractMojo {
    private static final String MAVEN_CLI_JAR = "maven-cli-idea-plugin.jar";

    public void execute() throws MojoExecutionException {
        File pluginsDir = determinePluginsDirViaIdeaPlugins();
        if (pluginsDir == null) {
            pluginsDir = determinePluginsDirViaIdeaHome();
            if (pluginsDir == null) {
                pluginsDir = determinePluginsViaDefaults();
            }
        }

        File pluginFile = new File(pluginsDir, MAVEN_CLI_JAR);
        if (pluginFile.exists()) {
            pluginFile.delete();
        }

        InputStream source = null;
        OutputStream dest = null;
        try {
            dest = new FileOutputStream(pluginFile);
            source = getClass().getResourceAsStream("/"+MAVEN_CLI_JAR);

            byte[] buffer = new byte[1024];
            int len;
            while ((len = source.read(buffer)) > 0) {
                dest.write(buffer, 0, len);
            }
        } catch (IOException e) {
            throw new MojoExecutionException("Unable to copy IDEA plugin", e);
        }
        finally {
            if (source != null) {
                try {
                    source.close();
                } catch (IOException e) {
                    e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                }
            }
            if (dest != null) {
                try {
                    dest.close();
                } catch (IOException e) {
                    e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
                }
            }
        }
        getLog().info("IDEA plugin installed");
    }

    private File determinePluginsDirViaIdeaHome() throws MojoExecutionException {
        String path = System.getProperty("idea.home");
        if (path == null) {
            return null;
        }
        File ideaHome = new File(path);
        if (!ideaHome.exists()) {
            throw new MojoExecutionException("The IDEA home directory doesn't exist");
        }

        File pluginsDir = new File(ideaHome, "plugins");
        if (!pluginsDir.exists()) {
            File configDir = new File(ideaHome, "config");
            pluginsDir = new File(configDir, "plugins");
            if (!pluginsDir.exists()) {
                throw new MojoExecutionException("The IDEA plugins directory cannot be found at "+pluginsDir.getAbsolutePath());
            }
        }
        return pluginsDir;
    }

    private File determinePluginsDirViaIdeaPlugins() throws MojoExecutionException {
        String path = System.getProperty("idea.plugins");
        if (path == null) {
            return null;
        }
        File pluginsDir = new File(path);
        if (!pluginsDir.exists()) {
            throw new MojoExecutionException("The IDEA plugins directory cannot be found at "+pluginsDir.getAbsolutePath());
        }
        return pluginsDir;
    }

    private File determinePluginsViaDefaults() {
        String[] paths = new String[] {
                System.getProperty("user.home") + "/.IntelliJIdea80/config/plugins",
                System.getProperty("user.home") + "/Library/Application Support/IntelliJIDEA80"
        };

        for (String path : paths) {
            String conv = path.replace('/', File.separatorChar);
            File pluginsDir = new File(conv);
            if (pluginsDir.exists()) {
                return pluginsDir;
            }
        }
        return null;
    }
}

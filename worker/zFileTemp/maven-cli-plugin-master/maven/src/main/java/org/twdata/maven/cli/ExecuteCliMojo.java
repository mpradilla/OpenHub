package org.twdata.maven.cli;

import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.maven.plugin.BuildPluginManager;
import org.apache.maven.plugin.PluginManager;
import org.codehaus.plexus.component.repository.exception.ComponentLookupException;
import org.twdata.maven.cli.commands.Command;
import org.twdata.maven.cli.commands.ExecuteGoalCommand;
import org.twdata.maven.mojoexecutor.MojoExecutor;

/**
 * Provides an interactive command line interface for Maven plugins, allowing
 * users to execute plugins directly.
 *
 * @requiresDependencyResolution execute
 * @aggregator true
 * @goal execute
 */
public class ExecuteCliMojo extends AbstractCliMojo {
    /**
     * Command aliases. Commands should be in the form GROUP_ID:ARTIFACT_ID:GOAL
     *
     * @parameter
     */
    private Map<String, String> commands = new HashMap<String, String>();

    /**
     * The Maven PluginManager Object
     *
     * @component
     */
    protected PluginManager pluginManager;

    @Override
    protected void beforeExecute() {
        // do nothing
    }

    @Override
    protected Command getSpecializedCliMojoCommand() {
        return new ExecuteGoalCommand(project, session, getExecutionEnvironment(), commands);
    }

    protected MojoExecutor.ExecutionEnvironment getExecutionEnvironment()
    {
        try
        {
            Class bpmClass = Class.forName("org.apache.maven.plugin.BuildPluginManager");
            Object buildPluginManager = session.lookup("org.apache.maven.plugin.BuildPluginManager");

            Class[] params = new Class[] {project.getClass(),session.getClass(),bpmClass};
            Method execEnvMethod = MojoExecutor.class.getMethod("executionEnvironment",params);
            Object[] args = new Object[] {project, session, buildPluginManager};

            MojoExecutor.ExecutionEnvironment execEnv = (MojoExecutor.ExecutionEnvironment) execEnvMethod.invoke(null,args);
            if(null != execEnv)
            {
                return execEnv;
            }
        }
        catch (Exception e)
        {
           // e.printStackTrace();
        }

        return MojoExecutor.executionEnvironment(project, session, pluginManager);
    }
}

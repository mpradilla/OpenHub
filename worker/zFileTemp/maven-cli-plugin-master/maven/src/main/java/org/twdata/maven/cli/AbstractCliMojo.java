package org.twdata.maven.cli;

import java.io.InputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.apache.maven.execution.MavenSession;
import org.apache.maven.plugin.AbstractMojo;
import org.apache.maven.plugin.MojoExecutionException;
import org.apache.maven.project.MavenProject;
import org.twdata.maven.cli.commands.Command;
import org.twdata.maven.cli.commands.ExitCommand;
import org.twdata.maven.cli.commands.HelpCommand;
import org.twdata.maven.cli.commands.ListProjectsCommand;
import org.twdata.maven.cli.console.JLineCliConsole;

public abstract class AbstractCliMojo extends AbstractMojo {
    /**
     * The Maven Project Object
     *
     * @parameter expression="${project}"
     * @required
     * @readonly
     */
    protected MavenProject project;

    /**
     * The Maven Session Object
     *
     * @parameter expression="${session}"
     * @required
     * @readonly
     */
    protected MavenSession session;

    /**
     * The reactor projects.
     *
     * @parameter expression="${reactorProjects}"
     * @readonly
     */
    private List reactorProjects;

    /**
     * Command prompt text.
     *
     * @parameter
     */
    private String prompt = "maven2";

    /**
     * TCP port to listen to for shell access
     *
     * @parameter expression="${cli.port}"
     */
    private String port = null;

    private boolean acceptSocket = true;
    private ServerSocket server = null;
    private CommandsCompletor commandsCompletor;
    private MultiOutputStream mos;

    protected Map<String, MavenProject> modules = new HashMap<String, MavenProject>();
    protected List<Command> cliCommands = new ArrayList<Command>();

    protected abstract void beforeExecute();
    protected abstract Command getSpecializedCliMojoCommand();

    public final void execute() throws MojoExecutionException {
        
        if(null == mos)
        {
            mos = new MultiOutputStream(System.out);
            System.setOut(new PrintStream(mos));
        }
        beforeExecute();
        resolveModulesInProject();
        buildCommands();
        buildCommandsCompletor();

        Thread consoleShell = displayConsoleCliShell();
        displaySocketCliShell();
        try {
            consoleShell.join();
        } catch (InterruptedException e) {
            // ignore
        }
    }

    private void resolveModulesInProject() {
        for (Object reactorProject : reactorProjects) {
            modules.put(((MavenProject) reactorProject).getArtifactId(),
                    (MavenProject) reactorProject);
        }
    }

    private void buildCommands() {
        cliCommands.add(getSpecializedCliMojoCommand());
        cliCommands.add(new ExitCommand());
        cliCommands.add(new ListProjectsCommand(modules.keySet()));
        cliCommands.add(new HelpCommand(cliCommands));
    }

    private void buildCommandsCompletor() {
        CommandTokenCollector collector = new CommandTokenCollector();
        for (Command command : cliCommands) {
            command.collectCommandTokens(collector);
        }

        commandsCompletor = new CommandsCompletor(collector.getCollectedTokens());
    }

    private Thread displayConsoleCliShell() {
        Thread shell = new Thread() {
            @Override
            public void run() {
                try {
                    displayShell(System.in, System.out);
                    acceptSocket = false;
                    if (server != null) {
                        server.close();
                    }
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            }
        };
        shell.start();

        return shell;
    }

    private void displaySocketCliShell() {
        String foundPort = "";
        if (port != null) {
            try {
                int requestedPort = Integer.parseInt(port);
                int plusOne = requestedPort + 1;
                
                int finalPort = AvailablePortFinder.getPortOrNextAvailable(requestedPort,plusOne);
                foundPort = Integer.toString(finalPort);
                
                if(requestedPort != finalPort)
                {
                    getLog().warn("request port " + requestedPort + " was taken. Using new port: " + finalPort);    
                }
                
                server = new ServerSocket(finalPort);
                openSocket(server, finalPort);
                
            } catch (IOException e) {
                getLog().error("Cannot open port " + foundPort + " for cli server: " + e);
                return;
            }
            catch (Exception e)
            {
                getLog().error(e.getMessage());
                return;
            }
        }
    }

    private void displayShell(InputStream in, PrintStream out) {
        JLineCliConsole console = new JLineCliConsole(in, out, getLog(), commandsCompletor, prompt);
        new CliShell(cliCommands, console).run();
    }

    private void openSocket(ServerSocket server, int port) {
        getLog().info("Opening port " + port + " for socket cli access");
        while (acceptSocket) {
            Socket connection = null;
            try {
                connection = server.accept();
                
                PrintStream socketStream = new PrintStream(connection.getOutputStream());
                mos.addStream(socketStream);
                displayShell(connection.getInputStream(), System.out);
            } catch (IOException ex) {
                getLog().error("Server quit unexpectedly");
                ex.printStackTrace();
            }
            finally {
                if (connection != null) {
                    try {
                        connection.close();
                    } catch (IOException e) {
                        // we really don't care
                    }
                }
            }
        }
    }
}
